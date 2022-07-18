/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Orange SA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "ns3/log.h"

#include "traffic-control-utils.h"

#include <ortools/linear_solver/linear_solver.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TrafficControlUtils");

void
TrafficControlUtils::OptimizeDutyCycleMaxMin (const devices_t &devs, const double limit,
                                              output_t &output)
{
  NS_LOG_FUNCTION ("Num devs: " + std::to_string (devs.size ())
                   << "Traffic bound: " + std::to_string (limit));

  using namespace operations_research;

  // Prepare inputs
  datamodel_t dm;
  dm.limit = limit;
  dm.bound = (int) devs.size ();
  for (auto const &d : devs)
    dm.deltas.push_back (d.second);

  // Create the ip solver with the CBC backend.
  std::unique_ptr<MPSolver> solver (MPSolver::CreateSolver ("CBC"));
  if (!solver)
    {
      NS_LOG_WARN ("CBC solver unavailable.");
      return;
    }
  //if (GetLogComponent ("TrafficControlUtils").IsEnabled (LogLevel::LOG_DEBUG))
  if (false)
    solver->EnableOutput ();
  solver->SetTimeLimit (absl::Seconds (30));

  static int nsettings = 1 + m_dutycycles.size ();
  const double infinity = solver->infinity ();
  static double alpha = 1e-5;
  static double c = 1e15;

  // Create the variables.
  std::vector<std::vector<MPVariable *>> x (dm.bound);
  for (int i = 0; i < dm.bound; ++i)
    solver->MakeBoolVarArray (
        nsettings, "Which duty-cycle value is assigned to device " + std::to_string (i), &(x[i]));

  MPVariable *theta =
      solver->MakeNumVar (0.0, infinity, "Linearization variable for the max min objective.");

  // Create the constraints.
  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          1.0, 1.0, "One and only one duty-cycle setting must be used by device " + std::to_string (i));
      for (int l = 0; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], 1);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0.0, dm.deltas[i],
          "Duty-cycle setting must not be greater than current offered traffic for device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i]);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1]);
    }

  MPConstraint *constraint = solver->MakeRowConstraint (
      0.0, dm.limit,
      "Total offerd traffic must not be grater than the limit imposed by PDR requirements");
  for (int i = 0; i < dm.bound; ++i)
    {
      constraint->SetCoefficient (x[i][0], dm.deltas[i]);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1]);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0.0, infinity, "Linearize objective for device " + std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i]);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1]);
      constraint->SetCoefficient (theta, -1.0);
    }

  // Create the objective function.
  MPObjective *const objective = solver->MutableObjective ();
  // Maximize offered traffic (max min duty-cycle)
  objective->SetCoefficient (theta, c);
  for (int i = 0; i < dm.bound; ++i)
    {
      objective->SetCoefficient (x[i][0], dm.deltas[i] / dm.bound * c * alpha);
      for (int l = 1; l < nsettings; ++l)
        objective->SetCoefficient (x[i][l], m_dutycycles[l - 1] / dm.bound * c * alpha);
    }
  objective->SetMaximization ();

  const MPSolver::ResultStatus result_status = solver->Solve ();

  // Check that the problem is feasible.
  if (result_status == MPSolver::INFEASIBLE)
    NS_LOG_ERROR ("The problem is INFEASIBLE.");

  // Check that the problem has an optimal solution.
  if (result_status != MPSolver::OPTIMAL)
    NS_LOG_DEBUG ("The problem does not have an optimal solution.");

  int countDisabled = 0;
  double objValue = 0.0;
  for (int i = 0; i < dm.bound; ++i)
    {
      if (x[i][0]->solution_value ())
        {
          output[devs[i].first] = 0;
          objValue += dm.deltas[i];
          continue;
        }
      for (int l = 1; l < nsettings - 1; ++l)
        if (x[i][l]->solution_value ())
          {
            output[devs[i].first] = l + 6;
            objValue += m_dutycycles[l - 1];
            continue;
          }
      if (x[i][nsettings - 1]->solution_value ())
        {
          output[devs[i].first] = 255;
          countDisabled++;
          continue;
        }
    }

  NS_LOG_DEBUG ("Bound = " + std::to_string (limit) + ", Objective value = " +
                std::to_string (objValue) + ", Num disabled = " + std::to_string (countDisabled));
}

void
TrafficControlUtils::OptimizeDutyCycleMax (const devices_t &devs, const double limit,
                                           output_t &output)
{
  NS_LOG_FUNCTION ("Num devs: " + std::to_string (devs.size ())
                   << "Traffic bound: " + std::to_string (limit));

  using namespace operations_research;

  // Prepare inputs
  datamodel_t dm;
  dm.limit = limit;
  dm.bound = (int) devs.size ();
  for (auto const &d : devs)
    dm.deltas.push_back (d.second);

  // Create the ip solver with the CBC backend.
  std::unique_ptr<MPSolver> solver (MPSolver::CreateSolver ("CBC"));
  if (!solver)
    {
      NS_LOG_WARN ("CBC solver unavailable.");
      return;
    }
  if (false)
    solver->EnableOutput ();
  solver->SetTimeLimit (absl::Seconds (30));

  static int nsettings = 1 + m_dutycycles.size ();
  static double c = 1e9;

  // Create the variables.
  std::vector<std::vector<MPVariable *>> x (dm.bound);
  for (int i = 0; i < dm.bound; ++i)
    solver->MakeBoolVarArray (
        nsettings, "Which duty-cycle value is assigned to device " + std::to_string (i), &(x[i]));

  // Create the constraints.
  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          1, 1, "One and only one duty-cycle setting must be used by device " + std::to_string (i));
      for (int l = 0; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], 1);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, dm.deltas[i],
          "Duty-cycle setting must not be greater than current offered traffic for device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i]);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1]);
    }

  MPConstraint *constraint = solver->MakeRowConstraint (
      0, dm.limit,
      "Total offerd traffic must not be grater than the limit imposed by PDR requirements");
  for (int i = 0; i < dm.bound; ++i)
    {
      constraint->SetCoefficient (x[i][0], dm.deltas[i]);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1]);
    }

  // Create the objective function.
  MPObjective *const objective = solver->MutableObjective ();
  // Maximize offered traffic
  for (int i = 0; i < dm.bound; ++i)
    {
      objective->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        objective->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
    }
  objective->SetMaximization ();

  const MPSolver::ResultStatus result_status = solver->Solve ();

  // Check that the problem is feasible.
  if (result_status == MPSolver::INFEASIBLE)
    NS_LOG_ERROR ("The problem is INFEASIBLE.");

  // Check that the problem has an optimal solution.
  if (result_status != MPSolver::OPTIMAL)
    NS_LOG_DEBUG ("The problem does not have an optimal solution.");

  int countDisabled = 0;
  for (int i = 0; i < dm.bound; ++i)
    {
      if (x[i][0]->solution_value ())
        {
          output[devs[i].first] = 0;
          continue;
        }
      for (int l = 1; l < nsettings - 1; ++l)
        if (x[i][l]->solution_value ())
          output[devs[i].first] = l + 6;
      if (x[i][nsettings - 1]->solution_value ())
        {
          output[devs[i].first] = 255;
          countDisabled++;
          continue;
        }
    }

  NS_LOG_DEBUG ("Bound = " + std::to_string (limit) +
                ", Objective value = " + std::to_string (objective->Value ()) +
                ", Num disabled = " + std::to_string (countDisabled));
}

const std::vector<double> TrafficControlUtils::m_dutycycles =
    std::vector<double>{1.0 / std::pow (2.0, 7.0),  1.0 / std::pow (2.0, 8.0),
                        1.0 / std::pow (2.0, 9.0),  1.0 / std::pow (2.0, 10.0),
                        1.0 / std::pow (2.0, 11.0), 1.0 / std::pow (2.0, 12.0),
                        1.0 / std::pow (2.0, 13.0), 1.0 / std::pow (2.0, 14.0),
                        1.0 / std::pow (2.0, 15.0), 0.0};

} // namespace ns3

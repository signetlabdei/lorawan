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
TrafficControlUtils::OptimizeDutyCycleMaxMin (const devices_t &devs, const double limit, output_t &output)
{
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
  static double c = 1e9;

  // Create the variables.
  std::vector<std::vector<MPVariable *>> x (dm.bound);
  for (int i = 0; i < dm.bound; ++i)
    solver->MakeBoolVarArray (
        nsettings, "Which duty-cycle value is assigned to device " + std::to_string (i), &(x[i]));

  std::vector<MPVariable *> d;
  solver->MakeBoolVarArray (dm.bound, "Whether to exclude a device", &d);

  MPVariable *theta = solver->MakeNumVar (
      0, infinity, "Linearization variable for the max min objective.");

  // Create the constraints.
  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, 1 * c,
          "Allow exclusion only if lowest duty-cycle would be used on device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][nsettings - 1], 1.0 * c);
      constraint->SetCoefficient (d[i], -1.0 * c);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          1 * c, 1 * c,
          "One and only one duty-cycle setting must be used by device " + std::to_string (i));
      for (int l = 0; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], 1.0 * c);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, dm.deltas[i] * c,
          "Duty-cycle setting must not be greater than current offered traffic for device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
    }

  MPConstraint *constraint = solver->MakeRowConstraint (
      0, dm.limit * c,
      "Total offerd traffic must not be grater than the limit imposed by PDR requirements");
  for (int i = 0; i < dm.bound; ++i)
    {
      constraint->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
      constraint->SetCoefficient (d[i], -m_dutycycles.back () * c);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, infinity, "Linearize objective for device " + std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
      constraint->SetCoefficient (d[i], -1.0 * c);
      constraint->SetCoefficient (theta, -1.0);
    }

  // Create the objective function.
  MPObjective *const objective = solver->MutableObjective ();
  // Maximize offered traffic (max min duty-cycle)
  objective->SetCoefficient (theta, 1.0);
  objective->SetMaximization ();

  const MPSolver::ResultStatus result_status = solver->Solve ();

  // Check that the problem is feasible.
  if (result_status == MPSolver::INFEASIBLE)
    NS_LOG_ERROR ("The problem is INFEASIBLE.");

  // Check that the problem has an optimal solution.
  if (result_status != MPSolver::OPTIMAL)
    NS_LOG_DEBUG ("The problem does not have an optimal solution.");

  for (int i = 0; i < dm.bound; ++i)
    {
      if (d[i]->solution_value ())
        {
          output[devs[i].first] = 255;
          continue;
        }
      if (x[i][0]->solution_value ())
        output[devs[i].first] = 0;
      for (int l = 1; l < nsettings; ++l)
        if (x[i][l]->solution_value ())
          output[devs[i].first] = l + 6;
    }
}

void
TrafficControlUtils::OptimizeDutyCycleMax (const devices_t &devs, const double limit, output_t &output)
{
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

  std::vector<MPVariable *> d;
  solver->MakeBoolVarArray (dm.bound, "Whether to exclude a device", &d);

  // Create the constraints.
  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, 1 * c,
          "Allow exclusion only if lowest duty-cycle would be used on device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][nsettings - 1], 1.0 * c);
      constraint->SetCoefficient (d[i], -1.0 * c);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          1 * c, 1 * c,
          "One and only one duty-cycle setting must be used by device " + std::to_string (i));
      for (int l = 0; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], 1.0 * c);
    }

  for (int i = 0; i < dm.bound; ++i)
    {
      MPConstraint *constraint = solver->MakeRowConstraint (
          0, dm.deltas[i] * c,
          "Duty-cycle setting must not be greater than current offered traffic for device " +
              std::to_string (i));
      constraint->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
    }

  MPConstraint *constraint = solver->MakeRowConstraint (
      0, dm.limit * c,
      "Total offerd traffic must not be grater than the limit imposed by PDR requirements");
  for (int i = 0; i < dm.bound; ++i)
    {
      constraint->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        constraint->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
      constraint->SetCoefficient (d[i], -m_dutycycles.back () * c);
    }

  // Create the objective function.
  MPObjective *const objective = solver->MutableObjective ();
  // Maximize offered traffic
  for (int i = 0; i < dm.bound; ++i)
    {
      objective->SetCoefficient (x[i][0], dm.deltas[i] * c);
      for (int l = 1; l < nsettings; ++l)
        objective->SetCoefficient (x[i][l], m_dutycycles[l - 1] * c);
      objective->SetCoefficient (d[i], -1.0 * c);
    }
  objective->SetMaximization ();

  const MPSolver::ResultStatus result_status = solver->Solve ();

  // Check that the problem is feasible.
  if (result_status == MPSolver::INFEASIBLE)
    NS_LOG_ERROR ("The problem is INFEASIBLE.");

  // Check that the problem has an optimal solution.
  if (result_status != MPSolver::OPTIMAL)
    NS_LOG_DEBUG ("The problem does not have an optimal solution.");

  for (int i = 0; i < dm.bound; ++i)
    {
      if (d[i]->solution_value ())
        {
          output[devs[i].first] = 255;
          continue;
        }
      if (x[i][0]->solution_value ())
        output[devs[i].first] = 0;
      for (int l = 1; l < nsettings; ++l)
        if (x[i][l]->solution_value ())
          output[devs[i].first] = l + 6;
    }
}

const std::vector<double> TrafficControlUtils::m_dutycycles = std::vector<double>{
    1.0 / std::pow (2.0, 7.0),  1.0 / std::pow (2.0, 8.0),  1.0 / std::pow (2.0, 9.0),
    1.0 / std::pow (2.0, 10.0), 1.0 / std::pow (2.0, 11.0), 1.0 / std::pow (2.0, 12.0),
    1.0 / std::pow (2.0, 13.0), 1.0 / std::pow (2.0, 14.0), 1.0 / std::pow (2.0, 15.0)};

} // namespace ns3

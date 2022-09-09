/*
 * Set of utilities to keep main example files more clean.
 */

#include "ns3/log.h"

#include "ns3/lora-interference-helper.h"

#include <unordered_map>
#include <iomanip>
#include <regex>

using namespace ns3;
using namespace lorawan;

/**
 * Clusters info (% devices, PDR required)
 */
using cluster_t = std::vector<std::pair<double, double>>;

/**
 * Parse clusters' info from string
 */
cluster_t
ParseClusterInfo (std::string s)
{
  s.erase (std::remove (s.begin (), s.end (), ' '), s.end ());
  std::regex rx ("\\{\\{[0-9]+(\\.[0-9]+)?,0*(1(\\.0+)?|0|\\.[0-9]+)\\}"
                 "(,\\{[0-9]+(\\.[0-9]+)?,0*(1(\\.0+)?|0|\\.[0-9]+)\\})*\\}");
  NS_ASSERT_MSG (std::regex_match (s, rx), "Cluster vector ill formatted. "
                                           "Syntax: \"{{double > 0, double [0,1]},...}\"");

  s.erase (std::remove (s.begin (), s.end (), '{'), s.end ());
  s.erase (std::remove (s.begin (), s.end (), '}'), s.end ());

  cluster_t clusterInfo;
  double share = 0;
  double pdr = 0;

  std::string d = ",";
  size_t pos = 0;
  double tot = 0;
  while ((pos = s.find (d)) != std::string::npos)
    {
      share = std::stod (s.substr (0, pos));
      s.erase (0, pos + d.length ());
      tot += share;

      pos = s.find (d);
      pdr = std::stod (s.substr (0, pos));
      s.erase (0, pos + d.length ());

      clusterInfo.push_back ({share, pdr});
    }
  NS_ASSERT_MSG (tot != 100.0, "Total share among clusters must be 100%.");
  return clusterInfo;
}

/**
 * \brief Computes total deployment area 
 * 
 * Computes total deployment area in range of gateways placed with
 * complete radial hexagonal tiling. This assumes that the maximum 
 * range devices are placed from a gateway is the side of hexagons. 
 * 
 * \param range Maximum device range from center of a gateway [m]
 * \param rings Number of rings of hexagons (central gateway = first ring)
 * 
 * \return Deployment area [km^2]
 */
double
ComputeArea (double range, int rings)
{
  if (rings == 1)
    return pow (range / 1000, 2) * M_PI;

  double radius = range * std::cos (M_PI / 6);
  int ngateways = 3 * rings * rings - 3 * rings + 1;

  double hexag = range / 1000 * radius / 1000 * 3;
  double disc = pow (range / 1000, 2) * M_PI;
  return (ngateways - 6 * (rings - 1)) * hexag // Internal hexagons
         + 3 * (hexag + disc) // Vertices
         + 2 * (rings - 2) * (2 * hexag + disc); // Sides
}

/**
 * Possible interference matrices
 */
const std::unordered_map<std::string, LoraInterferenceHelper::CollisionMatrix> sirMap = {
    {"CROCE", LoraInterferenceHelper::CROCE},
    {"GOURSAUD", LoraInterferenceHelper::GOURSAUD},
    {"ALOHA", LoraInterferenceHelper::ALOHA}};

/**
 * Print initial configuration
 */
void
PrintConfigSetup (int nDevs, double range, int rings, std::vector<int> &devPerSF)
{
  double area = ComputeArea (range, rings);
  std::stringstream ss;
  //ss << std::setprecision (10);
  ss << "Area: " << area << " km^2, Density: " << nDevs / area << " devs/km^2\n";
  ss << "\n|- SF distribution:    ";
  for (int j = (int) devPerSF.size () - 1; j >= 0; --j)
    ss << "SF" << 12 - j << ":" << devPerSF[j] << ", ";
  ss << "\n";
  ss << "\nAll configurations terminated. Starting simulation...\n\n"
     << "--------------------------------------------------------------------------------\n";
  std::cout << ss.str ();
}
//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_BUILDING_H
#define EPIDEMIC_SIM_BUILDING_H

// STL
#include <string>
#include <unordered_map>

// Epidemic
#include "Agent.h"
#include "Distributions.h"

namespace Epidemic {

struct BuildingConfiguration
{
   std::string m_name;
   double m_minX;
   double m_minY;
   double m_maxX;
   double m_maxY;
   std::vector<std::pair<std::string, Statistics::Distribution>> m_agentNameAndDistribution;
};

class Building
{
public:
   using Id = int;
   using Type = int;

   struct Position
   {
      double m_x;
      double m_y;
   };

   static Type get_building_type_by_name(const std::string& name);

   explicit Building(const BuildingConfiguration& buildingConfiguration,
      const std::vector<AgentConfiguration>& agentConfigurations,
      std::unordered_map<Agent::Id, Agent>& agentContainer);

   constexpr Id get_id() noexcept { return m_id; }
   constexpr const Position& get_max_position() & noexcept { return m_maxPosition; }
   constexpr Position get_max_position() && noexcept { return m_maxPosition; }

private:
   Id m_id;
   Type m_type;
   Position m_maxPosition;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_BUILDING_H
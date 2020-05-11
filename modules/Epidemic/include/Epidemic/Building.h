//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_BUILDING_H
#define EPIDEMIC_SIM_BUILDING_H

// STL
#include <string>
#include <unordered_map>

// Epidemic
#include "Types.h"
#include "Distributions.h"

namespace Epidemic {

class Building
{
public:
   using Distance = double;
   struct Position
   {
      Distance m_x = 0.0;
      Distance m_y = 0.0;
   };

   static BuildingType get_building_type_by_name(const std::string& name);
   static BuildingId get_next_building_id();

   explicit Building(const std::string& buildingName, const Position& maxPosition);

   [[nodiscard]] constexpr BuildingId get_id() const noexcept { return m_id; }
   [[nodiscard]] constexpr const Position& get_max_position() const & noexcept { return m_maxPosition; }
   [[nodiscard]] constexpr Position get_max_position() && noexcept { return m_maxPosition; }

   Position agent_enters_building(AgentId agent);
   void update_agent_position(AgentId agent, const Position& newPosition);
   void agent_leaves_building(AgentId agent);

   std::vector<std::pair<AgentId, Distance>> get_nearby_agents(Distance distance, AgentId agentId) const;

private:
   BuildingId m_id;
   BuildingType m_type;
   Position m_maxPosition;
   std::uniform_real_distribution<double> m_xDist;
   std::uniform_real_distribution<double> m_yDist;
   std::unordered_map<AgentId, Position> m_curAgents;

   static std::unordered_map<std::string, BuildingType> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_BUILDING_H
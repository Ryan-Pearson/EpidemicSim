#include "Epidemic/Building.h"

// Epidemic
#include "Epidemic/Distributions.h"

namespace Epidemic {

static BuildingId uniqueBuildingId = -1;
static BuildingType uniqueBuildingType = -1;
std::unordered_map<std::string, BuildingType> Building::s_typeByName;

Building::Building(const std::string& buildingName, const Position& maxPosition) :
   m_id(++uniqueBuildingId),
   m_type(get_building_type_by_name(buildingName)),
   m_maxPosition(maxPosition),
   m_entrancePosition(Position {m_maxPosition.m_x / 2.0, 0.0})
{
}

BuildingId get_next_building_id()
{
   return uniqueBuildingId + 1;
}

BuildingType Building::get_building_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueBuildingType);
      it = emplace_ret.first;
   }
   return it->second;
}

Building::Position Building::agent_enters_building(AgentId agent)
{
   const auto empRet = m_curAgents.emplace(agent, m_entrancePosition);

   if (empRet.second)
   {
      return empRet.first->second;
   }
   else
   {
      throw std::runtime_error("Agent attempted to enter building it was already in.");
   }
}

void Building::update_agent_position(AgentId agent, const Position& newPosition)
{
   m_curAgents.at(agent) = newPosition;
}

void Building::agent_leaves_building(AgentId agent)
{
   const auto ret = m_curAgents.erase(agent);

   if (ret == 0)
   {
      throw std::runtime_error("Agent attempting to leave building it was not in!");
   }
}

std::vector<std::pair<AgentId, Building::Distance>> Building::get_nearby_agents(
   const Distance distance, const AgentId agentId) const
{
   std::vector<std::pair<AgentId, Building::Distance>> ret;

   const auto norm2 = [](const Position x, const Position y) {
      const Distance xDist = x.m_x - y.m_x;
      const Distance yDist = x.m_y - y.m_y;
      return std::sqrt(xDist * xDist + yDist * yDist);
   };

   const auto agentPosition = m_curAgents.at(agentId);
   for (const auto& agentAndPositionToCheck : m_curAgents)
   {
      const AgentId agentIdToCheck = agentAndPositionToCheck.first;
      const Position agentPositionToCheck = agentAndPositionToCheck.second;
      const Distance distanceBetween = norm2(agentPosition, agentPositionToCheck);

      if ((agentIdToCheck != agentId) && (distanceBetween < distance))
      {
         ret.emplace_back(std::make_pair(agentIdToCheck, distanceBetween));
      }
   }

   return ret;
}

} // namespace Epidemic
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

} // namespace Epidemic
#include "Epidemic/Building.h"

// Epidemic
#include "Epidemic/Distributions.h"

namespace Epidemic {

static Building::Id uniqueBuildingId = -1;
static Building::Type uniqueBuildingType = -1;
std::unordered_map<std::string, Building::Type> Building::s_typeByName;

Building::Building(const BuildingConfiguration& buildingConfiguration,
   const std::vector<AgentConfiguration>& agentConfigurations,
   std::unordered_map<Agent::Id, Agent>& agentContainer) :
   m_id(++uniqueBuildingId), m_type(get_building_type_by_name(buildingConfiguration.m_name))
{
   std::uniform_int_distribution xDist(buildingConfiguration.m_minX, buildingConfiguration.m_maxX);
   std::uniform_int_distribution yDist(buildingConfiguration.m_minX, buildingConfiguration.m_maxY);

   m_maxPosition = Position {xDist(Statistics::GLOBAL_RANDOM_ENGINE), yDist(Statistics::GLOBAL_RANDOM_ENGINE)};

   for (const auto& curAgentAndDist : buildingConfiguration.m_agentNameAndDistribution)
   {
      const auto numAgents = Statistics::sample_distribution(curAgentAndDist.second);
   }
}

Building::Type Building::get_building_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueBuildingType);
      it = emplace_ret.first;
   }
   return it->second;
}

} // namespace Epidemic
//
// Created by Ryan S. Pearson on 5/6/20.
//

#include "Epidemic/WorldBuilder.h"

namespace Epidemic {

World build_world(const WorldConfiguration& worldConfiguration)
{
   std::unordered_map<CommunityId, Community> communities;
   std::unordered_map<AgentId, Agent> agents;

   for (const auto& communityTypeAndDistribution : worldConfiguration.m_communities)
   {
      const CommunityId communityType = communityTypeAndDistribution.first;
      const auto& communityDistribution = communityTypeAndDistribution.second;

      const auto& communityConfig = worldConfiguration.m_communityConfigs.at(communityType);

      const int numCommunitiesToSpawn = std::max(0, Statistics::sample_distribution(communityDistribution));
      for (int curCommunityCount = 0; curCommunityCount < numCommunitiesToSpawn; ++curCommunityCount)
      {
         Community::BuildingByTypeMap buildings;
         for (const auto& buildingTypeAndDistribution : communityConfig.m_buildings)
         {
            const BuildingId buildingType = buildingTypeAndDistribution.first;
            const auto& buildingDistribution = buildingTypeAndDistribution.second;

            const auto& buildingConfig = worldConfiguration.m_buildingConfigs.at(buildingType);
            const auto& buildingName = buildingConfig.m_name;
            auto buildingsEmplaceRet = buildings.emplace(buildingType, std::vector<Building> {});

            if (buildingsEmplaceRet.second)
            {
               throw std::runtime_error("Invalid building configuration for a community");
            }

            auto& containerOfBuildingsOfThisType = buildingsEmplaceRet.first->second;

            std::uniform_int_distribution xDist(buildingConfig.m_minX, buildingConfig.m_maxX);
            std::uniform_int_distribution yDist(buildingConfig.m_minX, buildingConfig.m_maxY);

            const int numBuildingsToSpawn = std::max(0, Statistics::sample_distribution(buildingDistribution));

            // Must reserve for stable building pointers
            containerOfBuildingsOfThisType.reserve(static_cast<size_t>(numBuildingsToSpawn));

            for (int curBuildingCount = 0; curBuildingCount < numBuildingsToSpawn; ++curBuildingCount)
            {
               const Building::Position maxPosition {
                  xDist(Statistics::GLOBAL_RANDOM_ENGINE), yDist(Statistics::GLOBAL_RANDOM_ENGINE)};

               containerOfBuildingsOfThisType.emplace_back(buildingName, maxPosition);
               auto& curBuilding = containerOfBuildingsOfThisType.back();

               std::uniform_int_distribution curBuildingXDist(0.0, maxPosition.m_x);
               std::uniform_int_distribution curBuildingYDist(0.0, maxPosition.m_y);

               for (const auto& agentTypeAndDistribution : buildingConfig.m_agents)
               {
                  const AgentId agentType = agentTypeAndDistribution.first;
                  const auto& agentDistribution = agentTypeAndDistribution.second;

                  const auto& agentConfig = worldConfiguration.m_agentConfigs.at(agentType);

                  const int numAgentsToSpawn = std::max(0, Statistics::sample_distribution(agentDistribution));
                  for (int curAgentCount = 0; curAgentCount < numAgentsToSpawn; ++curAgentCount)
                  {
                     const Building::Position agentStartingPosition {curBuildingXDist(Statistics::GLOBAL_RANDOM_ENGINE),
                        curBuildingYDist(Statistics::GLOBAL_RANDOM_ENGINE)};
                     Agent newAgent(agentConfig.m_name, &curBuilding, agentStartingPosition);
                     const AgentId agentId = newAgent.get_id();
                     curBuilding.agent_enters_building(newAgent.get_id());
                     curBuilding.update_agent_position(newAgent.get_id(), agentStartingPosition);
                     agents.emplace(agentId, newAgent);
                  }
               }
            }
         }

         Community newCommunity(std::move(buildings));
         communities.emplace(newCommunity.get_id(), std::move(newCommunity));
      }
   }

   return World(std::move(communities), std::move(agents));
}

} // namespace Epidemic
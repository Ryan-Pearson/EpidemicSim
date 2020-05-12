//
// Created by Ryan S. Pearson on 5/6/20.
//

#include "Epidemic/WorldBuilder.h"

// STL
#include <unordered_set>

namespace Epidemic {

World build_world(const WorldConfiguration& worldConfiguration)
{
   struct BuildingSpawnInfo
   {
      CommunityId m_community;
      Building* m_building;
      const std::unordered_map<AgentType, Statistics::Distribution>* m_agents;
   };

   std::vector<Community> communities;
   std::vector<Agent> agents;
   std::vector<BuildingSpawnInfo> agentsInBuildingToSpawn;

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

            if (!buildingsEmplaceRet.second)
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
                  xDist(Statistics::get_global_random_engine()), yDist(Statistics::get_global_random_engine())};

               containerOfBuildingsOfThisType.emplace_back(buildingName, maxPosition);
               auto& curBuilding = containerOfBuildingsOfThisType.back();

               if (!buildingConfig.m_agents.empty())
               {
                  agentsInBuildingToSpawn.emplace_back(
                     BuildingSpawnInfo {Community::get_next_community_id(), &curBuilding, &buildingConfig.m_agents});
               }
            }
         }

         Community newCommunity(std::move(buildings));
         assert(newCommunity.get_id() == static_cast<int>(communities.size()));
         communities.emplace_back(std::move(newCommunity));
      }
   }

   for (const auto& buildingSpawnInfo : agentsInBuildingToSpawn)
   {
      const auto& maxPosition = buildingSpawnInfo.m_building->get_max_position();
      std::uniform_int_distribution curBuildingXDist(0.0, maxPosition.m_x);
      std::uniform_int_distribution curBuildingYDist(0.0, maxPosition.m_y);

      const auto& community = communities[buildingSpawnInfo.m_community];

      for (const auto& agentSpawnInfo : *buildingSpawnInfo.m_agents)
      {
         const AgentId agentType = agentSpawnInfo.first;
         const auto& agentDistribution = agentSpawnInfo.second;

         const auto& agentConfig = worldConfiguration.m_agentConfigs.at(agentType);
         const auto& locationConfig = agentConfig.m_locations;

         const int numAgentsToSpawn = std::max(0, Statistics::sample_distribution(agentDistribution));
         for (int curAgentCount = 0; curAgentCount < numAgentsToSpawn; ++curAgentCount)
         {
            const auto movementConfigMapToInfoMap = [&](const auto& mapPairIn) {
               const auto movementConfigToInfo = [&](const AgentMovementConfiguration& config) {
                  const auto sampleRandomBuilding = [&](const std::string& buildingName) -> Building* {
                     if (buildingName == "SpawnLocation")
                     {
                        return buildingSpawnInfo.m_building;
                     }
                     else
                     {
                        const auto& buildingList =
                           community.get_buildings_of_type(Building::get_building_type_by_name(buildingName));
                        std::uniform_int_distribution<> dis(0, buildingList.size() - 1);
                        const auto buildingIndex = dis(Statistics::get_global_random_engine());
                        // TODO: Const cast
                        return const_cast<Building*>(&buildingList[buildingIndex]);
                     }
                  };

                  boost::container::small_vector<Building*, 10> buildingPtrs;
                  std::transform(config.m_locations.cbegin(), config.m_locations.cend(),
                     std::inserter(buildingPtrs, buildingPtrs.end()), sampleRandomBuilding);
                  return AgentMovementInfo {config.m_locationIdx, std::move(buildingPtrs), config.m_lambda};
               };

               return std::make_pair(mapPairIn.first, movementConfigToInfo(mapPairIn.second));
            };

            boost::container::flat_map<Timestep, AgentMovementInfo> locations;
            std::transform(locationConfig.cbegin(), locationConfig.cend(), std::inserter(locations, locations.end()),
               movementConfigMapToInfoMap);

            agents.emplace_back(Agent(buildingSpawnInfo.m_building, agentConfig.m_name, std::move(locations),
               worldConfiguration.m_infectionSymptomsDist, worldConfiguration.m_infectionDurationDist,
               agentConfig.m_mortalityRate));
         }
      }
   }

   int numToInfect = std::min(worldConfiguration.m_initialInfectionSize, static_cast<int>(agents.size()));
   std::uniform_int_distribution<> agentPicker(0, static_cast<int>(agents.size()));
   std::unordered_set<int> toInfect;
   while (static_cast<int>(toInfect.size()) < numToInfect)
   {
      toInfect.insert(agentPicker(Statistics::get_global_random_engine()));
   }

   Timestep numberOfInfectionTimestepsRemaining = 0;
   for (const AgentId agentToInfect : toInfect)
   {
      const auto daysRemaining = agents[agentToInfect].attempt_infection(0);
      numberOfInfectionTimestepsRemaining += daysRemaining.value_or(0);
   }

   return World(std::move(communities), std::move(agents), numberOfInfectionTimestepsRemaining);
}

} // namespace Epidemic
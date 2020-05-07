//
// Created by Ryan S. Pearson on 5/6/20.
//

#include "Epidemic/WorldBuilder.h"

namespace Epidemic {

World build_world(const WorldConfiguration& worldConfiguration)
{
   for (const auto& communityConfigAndId : worldConfiguration.m_communities)
   {
      const Community::Id communityId = communityConfigAndId.first;
      const auto& communityConfig = worldConfiguration.m_communityConfigs.at(communityId);

      for (const auto& buildingConfigAndId : communityConfig.m_buildings)
      {
         const Building::Id buildingId = buildingConfigAndId.first;
         const auto& buildingConfig = worldConfiguration.m_buildingConfigs.at(buildingId);

         std::uniform_int_distribution xDist(buildingConfig.m_minX, buildingConfig.m_maxX);
         std::uniform_int_distribution yDist(buildingConfig.m_minX, buildingConfig.m_maxY);

         m_maxPosition = Position {xDist(Statistics::GLOBAL_RANDOM_ENGINE), yDist(Statistics::GLOBAL_RANDOM_ENGINE)};

         for (const auto& agentConfigAndId : buildingConfig.m_agents)
         {
            const Agent::Id agentId = agentConfigAndId.first;
            const auto& agentConfig = worldConfiguration.m_agentConfigs.at(agentId);
         }
      }
   }
}

} // namespace Epidemic
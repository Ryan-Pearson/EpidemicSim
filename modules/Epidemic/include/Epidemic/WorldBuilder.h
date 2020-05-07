//
// Created by Ryan S. Pearson on 5/6/20.
//

#ifndef EPIDEMIC_SIM_WORLDBUILDER_H
#define EPIDEMIC_SIM_WORLDBUILDER_H

#include "World.h"

namespace Epidemic {

struct AgentConfiguration
{
   std::string m_name;
};

struct BuildingConfiguration
{
   std::string m_name;
   double m_minX;
   double m_minY;
   double m_maxX;
   double m_maxY;
   std::unordered_map<Agent::Id, Statistics::Distribution> m_agents;
};

struct CommunityConfiguration
{
   std::string m_name;
   std::unordered_map<int, Statistics::Distribution> m_buildings;
};

struct WorldConfiguration
{
   std::unordered_map<int, AgentConfiguration> m_agentConfigs;
   std::unordered_map<int, BuildingConfiguration> m_buildingConfigs;
   std::unordered_map<int, CommunityConfiguration> m_communityConfigs;

   std::unordered_map<int, Statistics::Distribution> m_communities;
};

World build_world(const WorldConfiguration& worldConfiguration);

} // namespace Epidemic

#endif //EPIDEMIC_SIM_WORLDBUILDER_H

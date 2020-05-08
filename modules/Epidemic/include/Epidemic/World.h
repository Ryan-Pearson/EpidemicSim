//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_WORLD_H
#define EPIDEMIC_SIM_WORLD_H

// STL
#include <unordered_map>

// Epidemic
#include "Agent.h"

namespace Epidemic {

class World
{
public:
   explicit World(
      std::unordered_map<CommunityId, Community> communities, std::unordered_map<AgentId, Agent> agents);

private:
   std::unordered_map<CommunityId, Community> m_communities;
   std::unordered_map<AgentId, Agent> m_agents;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_WORLD_H
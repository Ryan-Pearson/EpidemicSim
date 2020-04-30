//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_WORLD_H
#define EPIDEMIC_SIM_WORLD_H

// STL
#include <unordered_map>

// Epidemic
#include "Agent.h"
#include "Building.h"
#include "Community.h"
#include "WorldConfiguration.h"

namespace Epidemic {

class World
{
public:
   explicit World(const WorldConfiguration& config);

private:
   std::unordered_map<Community::Id, Community> m_communities;
   std::unordered_map<Agent::Id, Agent> m_agents;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_WORLD_H
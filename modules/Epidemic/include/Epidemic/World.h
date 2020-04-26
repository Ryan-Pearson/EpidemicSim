//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_WORLD_H
#define EPIDEMIC_SIM_WORLD_H

// STL
#include <unordered_set>

// Epidemic
#include "Agent.h"

namespace Epidemic {

class World
{
public:

private:
   std::unordered_set<Agent> m_agents;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_WORLD_H
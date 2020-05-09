//
// Created by Ryan S. Pearson on 5/7/20.
//

#ifndef EPIDEMIC_SIM_TYPES_H
#define EPIDEMIC_SIM_TYPES_H

namespace Epidemic {

using Timestep = int;

using AgentId = int;
using AgentType = int;

using BuildingId = int;
using BuildingType = int;

using CommunityId = int;
using CommunityType = int;

// Globals
// TODO: Make better / remove
constexpr double TIMESTEP_TO_TIME_IN_SEC = 60.0;
constexpr double AGENT_SPEED = 2.93333; // 2.93333 ft/s ~= 2 mph
constexpr Timestep INFECTION_LENGTH =
   static_cast<Timestep>(1e-6 + (2.0 * 7.0 * 24.0 * 60.0 * 60.0 / TIMESTEP_TO_TIME_IN_SEC));

} // namespace Epidemic

#endif //EPIDEMIC_SIM_TYPES_H

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
constexpr Timestep TIMESTEP_PER_HOUR = 60 * 60;
constexpr Timestep TIMESTEP_PER_DAY = TIMESTEP_PER_HOUR * 24;
constexpr double TIMESTEP_TO_TIME_IN_SEC = 1.0 / (static_cast<double>(TIMESTEP_PER_HOUR) / (60.0 * 60.0));
constexpr double TIMESTEP_TO_TIME_IN_HOUR = TIMESTEP_TO_TIME_IN_SEC / (60.0 * 60.0);
constexpr double TIME_IN_HOUR_TO_TIMESTEP = 1.0 / TIMESTEP_TO_TIME_IN_HOUR;
constexpr double AGENT_SPEED = 2.93333; // 2.93333 ft/s ~= 2 mph

} // namespace Epidemic

#endif //EPIDEMIC_SIM_TYPES_H

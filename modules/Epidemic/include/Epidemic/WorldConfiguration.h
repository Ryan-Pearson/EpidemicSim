//
// Created by Ryan S. Pearson on 4/26/20.
//

#ifndef EPIDEMIC_SIM_WORLDCONFIGURATION_H
#define EPIDEMIC_SIM_WORLDCONFIGURATION_H

// STL
#include <vector>

// Epidemic
#include "CommunityConfiguration.h"

namespace Epidemic {

struct WorldConfiguration
{
   std::vector<CommunityConfiguration> communities;
};

} // namespace Epidemic

#endif //EPIDEMIC_SIM_WORLDCONFIGURATION_H

//
// Created by Ryan S. Pearson on 4/27/20.
//

#ifndef EPIDEMIC_SIM_COMMUNITYCONFIGURATION_H
#define EPIDEMIC_SIM_COMMUNITYCONFIGURATION_H

// STL
#include <string>
#include <unordered_map>

// Epidemic
#include "CommonConfiguration.h"

namespace Epidemic {

struct CommunityConfiguration
{
   CommunityConfiguration(std::string name) : m_name(std::move(name)) { }

   std::string m_name;
   std::unordered_map<std::string, Statistics::Distribution> m_buildings;
};

} // namespace Epidemic

#endif //EPIDEMIC_SIM_COMMUNITYCONFIGURATION_H

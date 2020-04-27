//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_COMMUNITY_H
#define EPIDEMIC_SIM_COMMUNITY_H

// STL
#include <unordered_map>
#include <vector>

// Epidemic
#include "Building.h"

namespace Epidemic {

using CommunityID = int;

class Community
{
public:
   using BuildingByTypeMap = std::unordered_map<Building::Type, std::vector<Building>>;

   explicit Community() noexcept;

   constexpr CommunityID getId() noexcept { return m_id; }
   const std::vector<Building>& getBuildingsOfType(Building::Type type) const &;

private:
   CommunityID m_id;
   BuildingByTypeMap m_buildings;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_COMMUNITY_H
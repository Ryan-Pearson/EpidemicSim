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
#include "Distributions.h"
#include "Types.h"

namespace Epidemic {

class Community
{
public:
   using BuildingByTypeMap = std::unordered_map<CommunityType, std::vector<Building>>;

   static CommunityType get_community_type_by_name(const std::string& name);

   explicit Community(BuildingByTypeMap buildings);

   [[nodiscard]] constexpr CommunityId get_id() const noexcept { return m_id; }
   [[nodiscard]] std::vector<Building> get_buildings_of_type(BuildingType type) && { return m_buildings[type]; }
   [[nodiscard]] const std::vector<Building>& get_buildings_of_type(BuildingType type) & { return m_buildings[type]; }

private:
   CommunityId m_id;
   BuildingByTypeMap m_buildings;

   static std::unordered_map<std::string, CommunityType> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_COMMUNITY_H
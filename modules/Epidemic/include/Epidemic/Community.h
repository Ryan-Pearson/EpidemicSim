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

namespace Epidemic {

struct CommunityConfiguration
{
   std::string m_name;
   std::unordered_map<std::string, Statistics::Distribution> m_buildings;
};

class Community
{
   using BuildingByTypeMap = std::unordered_map<Building::Type, std::vector<Building>>;

public:
   using Id = int;
   using Type = int;

   explicit Community(const CommunityConfiguration& configuration) noexcept;

   static Type get_community_type_by_name(const std::string& name);

   constexpr Id get_id() noexcept { return m_id; }
   const std::vector<Building>& get_buildings_of_type(Building::Type type) const&;

private:
   Id m_id;
   BuildingByTypeMap m_buildings;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_COMMUNITY_H
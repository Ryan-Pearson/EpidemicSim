#include "Epidemic/Community.h"

namespace Epidemic {

thread_local CommunityId uniqueCommunityId = -1;
thread_local CommunityType uniqueCommunityType = -1;
thread_local std::unordered_map<std::string, CommunityType> Community::s_typeByName;

Community::Community(BuildingByTypeMap buildings) : m_id(++uniqueCommunityId), m_buildings(std::move(buildings))
{
}

void Community::reset_static_counters()
{
   uniqueCommunityId = -1;
   uniqueCommunityType = -1;
}

CommunityId Community::get_next_community_id()
{
   return uniqueCommunityId + 1;
}

CommunityType Community::get_community_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueCommunityType);
      it = emplace_ret.first;
   }
   return it->second;
}

} // namespace Epidemic
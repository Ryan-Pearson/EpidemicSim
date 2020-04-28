#include "Epidemic/Community.h"

namespace Epidemic {

static Community::Id uniqueCommunityId = -1;
static Community::Type uniqueCommunityType = -1;
std::unordered_map<std::string, Community::Type> Community::s_typeByName;

Community::Community(const CommunityConfiguration& configuration) noexcept : m_id(++uniqueCommunityId) { }

Community::Type Community::get_community_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueCommunityType);
      it = emplace_ret.first;
   }
   return it->second;
}

const std::vector<Building>& Community::get_buildings_of_type(const Building::Type type) const&
{
   return m_buildings.at(type);
}

} // namespace Epidemic
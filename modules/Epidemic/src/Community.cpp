#include "Epidemic/Community.h"

namespace Epidemic {

static int uniqueCommunityId = 0;

Community::Community(const CommunityConfiguration& configuration) noexcept : m_id(++uniqueCommunityId) { }

const std::vector<Building>& Community::getBuildingsOfType(const Building::Type type) const&
{
   return m_buildings.at(type);
}

} // namespace Epidemic
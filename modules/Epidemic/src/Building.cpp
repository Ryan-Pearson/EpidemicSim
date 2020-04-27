#include "Epidemic/Building.h"

namespace Epidemic {

static int uniqueBuildingId = -1;

Building::Building(const Type type, const Position& maxPosition) noexcept :
   m_id(++uniqueBuildingId), m_type(type), m_maxPosition(maxPosition)
{
}

} // namespace Epidemic
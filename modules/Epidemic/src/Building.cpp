#include "Epidemic/Building.h"

namespace Epidemic {

static Building::Id uniqueBuildingId = -1;
static Building::Type uniqueBuildingType = -1;
std::unordered_map<std::string, Building::Type> Building::s_typeByName;

Building::Building(const Type type, const Position& maxPosition) noexcept :
   m_id(++uniqueBuildingId), m_type(type), m_maxPosition(maxPosition)
{
   (void)m_id;
   (void)m_type;
   (void)m_maxPosition;
}

Building::Type Building::get_building_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueBuildingType);
      it = emplace_ret.first;
   }
   return it->second;
}

} // namespace Epidemic
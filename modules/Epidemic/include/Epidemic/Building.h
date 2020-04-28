//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_BUILDING_H
#define EPIDEMIC_SIM_BUILDING_H

// STL
#include <string>
#include <unordered_map>

namespace Epidemic {

class Building
{
public:
   using Id = int;
   using Type = int;

   struct Position
   {
      double x;
      double y;
   };

   static Type get_building_type_by_name(const std::string& name);

   explicit Building(Type type, const Position& maxPosition) noexcept;

   constexpr Id getId() noexcept { return m_id; }
   constexpr const Position& getMaxPosition() & noexcept { return m_maxPosition; }
   constexpr Position getMaxPosition() && noexcept { return m_maxPosition; }

private:
   Id m_id;
   Type m_type;
   Position m_maxPosition;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_BUILDING_H
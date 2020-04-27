//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_BUILDING_H
#define EPIDEMIC_SIM_BUILDING_H

namespace Epidemic {

using BuildingId = int;

class Building
{
public:
   enum class Type
   {
      ESSENTIAL,
      HOUSE,
      NON_ESSENTIAL,
      OFFICE,
      SCHOOL
   };

   struct Position
   {
      double x;
      double y;
   };

   explicit Building(Type type, const Position& maxPosition) noexcept;

   constexpr BuildingId getId() noexcept { return m_id; }
   constexpr const Position& getMaxPosition() & noexcept { return m_maxPosition; }
   constexpr Position getMaxPosition() && noexcept { return m_maxPosition; }

private:
   BuildingId m_id;
   Type m_type;
   Position m_maxPosition;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_BUILDING_H
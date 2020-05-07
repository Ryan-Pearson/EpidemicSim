//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_BUILDING_H
#define EPIDEMIC_SIM_BUILDING_H

// STL
#include <string>
#include <unordered_map>

// Epidemic
#include "Agent.h"
#include "Distributions.h"

namespace Epidemic {

class Building
{
public:
   using Id = int;
   using Type = int;

   struct Position
   {
      double m_x;
      double m_y;
   };

   static Type get_building_type_by_name(const std::string& name);

   explicit Building(const std::string& buildingName, const Position& maxPosition);

   [[nodiscard]] constexpr Id get_id() const noexcept { return m_id; }
   [[nodiscard]] constexpr const Position& get_max_position() const & noexcept { return m_maxPosition; }
   [[nodiscard]] constexpr Position get_max_position() && noexcept { return m_maxPosition; }

private:
   Id m_id;
   Type m_type;
   Position m_maxPosition;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_BUILDING_H
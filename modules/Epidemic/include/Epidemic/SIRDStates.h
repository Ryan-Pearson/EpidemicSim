//
// Created by Ryan S. Pearson on 5/8/20.
//

#ifndef EPIDEMIC_SIM_SIRDSTATES_H
#define EPIDEMIC_SIM_SIRDSTATES_H

// STL
#include <variant>

// Epidemic
#include "Types.h"

namespace Epidemic {
namespace SirdState {

   struct Susceptible
   {
   };

   struct Infectious
   {
      Timestep m_symptomsShow;
      Timestep m_infectionEndTime;
   };

   struct Recovered
   {
      Timestep m_timeRecovered;
   };

   struct Deceased
   {
      Timestep m_timeDeceased;
   };

} // namespace SirdState

using SIRD = std::variant<SirdState::Susceptible, SirdState::Infectious, SirdState::Recovered, SirdState::Deceased>;

} // namespace Epidemic

#endif //EPIDEMIC_SIM_SIRDSTATES_H

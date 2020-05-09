//
// Created by Ryan S. Pearson on 5/8/20.
//

#ifndef EPIDEMIC_SIM_SIRDSTATES_H
#define EPIDEMIC_SIM_SIRDSTATES_H

// STL
#include <variant>

namespace Epidemic {
namespace SirdState {

   struct Susceptible
   {
   };

   struct Infectious
   {
   };

   struct Recovered
   {
   };

   struct Deceased
   {
   };
} // namespace SirdState

using SIRD = std::variant<SirdState::Susceptible, SirdState::Infectious, SirdState::Recovered, SirdState::Deceased>;

} // namespace Epidemic

#endif //EPIDEMIC_SIM_SIRDSTATES_H

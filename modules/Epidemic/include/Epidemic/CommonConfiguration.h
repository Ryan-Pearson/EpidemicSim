//
// Created by Ryan S. Pearson on 4/29/20.
//

#ifndef EPIDEMIC_SIM_COMMONCONFIGURATION_H
#define EPIDEMIC_SIM_COMMONCONFIGURATION_H

// STL
#include <random>
#include <vector>
#include <variant>

// Boost
#include <boost/container/flat_map.hpp>

namespace Epidemic {
namespace Statistics {

   struct PDF
   {
      PDF(const boost::container::flat_map<int, int>& numToWeight);

      std::vector<int> numByIdx;
      std::discrete_distribution<int> dist;
   };

   struct Gaussian
   {
      Gaussian(const double mean, const double stdDev) noexcept : dist(mean, stdDev) { }

      std::normal_distribution<double> dist;
   };

   struct Fixed
   {
      constexpr Fixed(const int value) noexcept : m_value(value) { }

      int m_value;
   };

   using Distribution = std::variant<PDF, Gaussian, Fixed>;

} // namespace Statistics
} // namespace Epidemic

#endif //EPIDEMIC_SIM_COMMONCONFIGURATION_H

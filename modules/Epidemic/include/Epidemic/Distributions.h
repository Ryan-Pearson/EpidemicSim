//
// Created by Ryan S. Pearson on 4/29/20.
//

#ifndef EPIDEMIC_SIM_DISTRIBUTIONS_H
#define EPIDEMIC_SIM_DISTRIBUTIONS_H

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

      std::vector<int> m_numByIdx;
      std::discrete_distribution<int> m_dist;
   };

   struct Gaussian
   {
      Gaussian(const double mean, const double stdDev) noexcept : m_dist(mean, stdDev) { }

      std::normal_distribution<double> m_dist;
   };

   struct Fixed
   {
      constexpr Fixed(const int value) noexcept : m_value(value) { }

      int m_value;
   };

   using Distribution = std::variant<PDF, Gaussian, Fixed>;

   int sample_distribution(const Distribution& dist) noexcept;

} // namespace Statistics
} // namespace Epidemic

#endif //EPIDEMIC_SIM_DISTRIBUTIONS_H

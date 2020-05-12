//
// Created by Ryan S. Pearson on 4/29/20.
//

#ifndef EPIDEMIC_SIM_DISTRIBUTIONS_H
#define EPIDEMIC_SIM_DISTRIBUTIONS_H

// STL
#include <random>
#include <variant>
#include <vector>

// Boost
#include <boost/container/flat_map.hpp>

namespace Epidemic {
namespace Statistics {

   std::mt19937& get_global_random_engine();

   struct PDF
   {
      explicit PDF(const boost::container::flat_map<int, int>& numToWeight);

      std::vector<int> m_numByIdx;
      std::discrete_distribution<int> m_dist;
   };

   struct Gaussian
   {
      explicit Gaussian(const double mean, const double stdDev) noexcept : m_dist(mean, stdDev) { }

      std::normal_distribution<double> m_dist;
   };

   struct Fixed
   {
      int m_value;
   };

   using Distribution = std::variant<Fixed, PDF, Gaussian>;

   int sample_distribution(const Distribution& dist) noexcept;

} // namespace Statistics
} // namespace Epidemic

#endif //EPIDEMIC_SIM_DISTRIBUTIONS_H

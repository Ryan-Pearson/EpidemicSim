//
// Created by Ryan S. Pearson on 4/29/20.
//

#include "Epidemic/Distributions.h"

namespace Epidemic {
namespace Statistics {

   // clang-format off
   // Taken from https://en.cppreference.com/w/cpp/utility/variant/visit, not needed in C++20
   template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
   template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
   // clang-format on

   std::mt19937 seeded_random_engine()
   {
      std::mt19937 rng(std::random_device {}());
      rng.discard(700000);
      return rng;
   }

   static std::mt19937 randomEngine = seeded_random_engine();

   PDF::PDF(const boost::container::flat_map<int, int>& numToWeight)
   {
      std::vector<int> weights;

      const auto getKey = [](const auto& pair) { return pair.first; };
      const auto getValue = [](const auto& pair) { return pair.second; };

      m_numByIdx.reserve(numToWeight.size());
      weights.reserve(numToWeight.size());

      std::transform(numToWeight.cbegin(), numToWeight.cend(), std::back_inserter(m_numByIdx), getKey);
      std::transform(numToWeight.cbegin(), numToWeight.cend(), std::back_inserter(weights), getValue);

      m_dist = std::discrete_distribution<int>(weights.cbegin(), weights.cend());
   }

   int sample_distribution(Distribution& dist) noexcept
   {
      return std::visit(overloaded {[](PDF& pdf) { return pdf.m_numByIdx.at(pdf.m_dist(randomEngine)); },
                           [](Gaussian& gaussian) { return static_cast<int>(gaussian.m_dist(randomEngine)); },
                           [](const Fixed& fixed) { return fixed.m_value; }},
         dist);
   }

} // namespace Statistics
} // namespace Epidemic
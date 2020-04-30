//
// Created by Ryan S. Pearson on 4/29/20.
//

#include "Epidemic/CommonConfiguration.h"

namespace Epidemic {
namespace Statistics {

   PDF::PDF(const boost::container::flat_map<int, int>& numToWeight)
   {
      std::vector<int> weights;

      const auto getKey = [](const auto& pair) { return pair.first; };
      const auto getValue = [](const auto& pair) { return pair.second; };

      numByIdx.reserve(numToWeight.size());
      weights.reserve(numToWeight.size());

      std::transform(numToWeight.cbegin(), numToWeight.cend(), std::back_inserter(numByIdx), getKey);
      std::transform(numToWeight.cbegin(), numToWeight.cend(), std::back_inserter(weights), getValue);

      dist = std::discrete_distribution<int>(weights.cbegin(), weights.cend());
   }

} // namespace Statistics
} // namespace Epidemic
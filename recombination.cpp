
#include <algorithm>
#include <random>

#include "functions.hpp"
#include "recombination.hpp"

namespace recombination {
   std::vector<individual> crossover(
      const std::vector<std::vector<individual>::iterator>& parents,
      KusiakLayoutEvaluator& kle) {
      // the return value
      std::vector<individual> children;
      
      // the rng
      std::random_device device;
      std::mt19937 rng(device());
      
      // combine each parent with the one next to it
      for (auto it = parents.begin(); it + 1 < parents.end(); ++it)
      {
         auto& layout_a = (*(it))->layout;
         auto& layout_b = (*(it + 1))->layout;
         
         // determine a random cutoff from [0, size]
         std::size_t size = std::min(layout_a.size(), layout_b.size());
         std::uniform_int_distribution<std::size_t> dist_size(size);
         std::size_t cutoff = dist_size(rng);
         
         individual child;
         // copy [0, cutoff] turbines from layout a
         for (std::size_t i = 0; i < cutoff; ++i)
         {
            child.layout.push_back(layout_a[i]);
         }
         // copy [cutoff, size] turbines from layout b
         for (std::size_t i = cutoff; i < size; ++i)
         {
            child.layout.push_back(layout_b[i]);
         }
         
         // determine the fitness
         auto mat_layout = functions::individual_to_matrix<double>(child.layout);
         child.fitness = kle.evaluate_2014(&mat_layout);

         children.push_back(child);
      }

      return children;
   }
}
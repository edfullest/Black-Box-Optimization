#include <algorithm>
#include <random>

#include "functions.hpp"
#include "recombination.hpp"

namespace recombination {
   std::vector<individual> none(const std::vector<std::vector<individual>::iterator>& parents,
                                Scenario& scenario) {      
      std::vector<individual> children;
      // just copy all the parents
      for (auto& parent : parents) {
         children.push_back(*parent);
      }
      return children;
   }
   
   std::vector<individual> crossover(const std::vector<std::vector<individual>::iterator>& parents,
                                     Scenario& scenario) {      
      // the return value
      std::vector<individual> children;

      // the rng
      std::random_device device;
      std::mt19937 rng(device());

      // combine each parent with the one next to it
      for (auto it = parents.begin(); it != parents.end(); ++it) {
         auto& layout_a = (*(it))->layout;
         auto& layout_b = it + 1 == parents.end() ?
	    (*(parents.begin()))->layout : (*(it + 1))->layout;

         // sort the layouts so that the crossover is geometric
         auto coord_compare = [](const coordinate& a, const coordinate& b) {
            return a.y == b.y ? a.x < b.x : a.y < b.y;
         };
         std::sort(layout_a.begin(), layout_a.end(), coord_compare);
         std::sort(layout_b.begin(), layout_b.end(), coord_compare);

         // determine which layout has less turbines. the cutoff will be drawn from this
         bool a_min = layout_a.size() < layout_b.size();
         auto& min_layout = a_min ? layout_a : layout_b;
         auto& max_layout = a_min ? layout_b : layout_a;
         std::size_t min_size = min_layout.size();
         std::size_t max_size = max_layout.size();

         // determine a random cutoff from [0, min_size]
         // this is the number of turbines to copy from the first parent
         //std::size_t size = std::min(layout_a.size(), layout_b.size());
         std::uniform_int_distribution<std::size_t> dist_min(0, min_size);
         std::size_t cutoff = dist_min(rng);
         // if the size is below the maximum, add some additional turbines
         // this prevents that we will eventually converge to low turbine individuals
         std::uniform_int_distribution<std::size_t> dist_over(min_size, scenario.max_turbines);
         std::size_t over = dist_over(rng);
         std::size_t size = max_size < over ? max_size : over;
         std::uniform_real_distribution<double> wdist(0.0, scenario.width);
         std::uniform_real_distribution<double> hdist(0.0, scenario.height); 

         individual child;
         // copy [0, cutoff] turbines from min_layout
         for (std::size_t i = 0; i < cutoff; ++i) {
            child.layout.push_back(min_layout[i]);
         }
         // copy [cutoff, size] turbines from max_layout
         for (std::size_t i = cutoff; i < size; ++i) {
            auto& coord = max_layout[i];
            if (!functions::turbine_collides(coord.x, coord.y, scenario, child.layout)) {
               child.layout.push_back(coord);
            }
         }
         
         // create [size, over] new turbines
         std::size_t child_size = child.layout.size();
         for (std::size_t i = child_size; i < over; ++i) {
            double x = wdist(rng);
            double y = hdist(rng);
            if (!functions::turbine_collides(x, y, scenario, child.layout)) {
               child.layout.push_back({ x, y });
            }
         }

         children.push_back(child);
      }

      return children;
   }
}

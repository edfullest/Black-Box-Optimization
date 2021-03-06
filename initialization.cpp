#include <random>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "random.hpp"
#include "initialization.hpp"
#include "API/Matrix.hpp"

namespace initialization {

    /* initialization_1
     *
     * This function takes  the WindScenario as parameter and produces
     * a population / vector of those turbine layouts.
     * The returned layouts are completely random based and
     * will be corrected by the "replaceviolations" function
     *
     * !! srand(time(0)); needs to be in the main function !!
     * !! currently no evaluation of the population implemented !
     !
     * parameters: WindScenario swscenario, int pop_size
     * return: std::vector<individual> population
     *
     */
    std::vector<individual> initialization_1(WindFarmLayoutEvaluator &evaluator,
                                             Scenario &scenario,
                                             int pop_size) {

        std::vector<individual> population;
        individual indiv;
        coordinate coord;

        for (int j = 0; j < pop_size;j++){
            //initiate layout (coordinates)
            for (int i = 0;i < scenario.max_turbines; i++) {
                //creates coordinates and puts them into the individual
                coord.x = scenario.width * (double) rand() / (double) RAND_MAX;
                coord.y = scenario.height * (double) rand() / (double) RAND_MAX;
                indiv.layout.push_back(coord);
            }

            population.push_back(indiv);
            indiv.layout.clear();
        }
        initialization::replace_violations(population,scenario);
        initialization::evaluate_population(evaluator,population);
        return population;

    }

   double turbine_distance(const coordinate& a, const coordinate& b) {
      double dx = std::abs(a.x - b.x);
      double dy = std::abs(a.y - b.y);
      return std::sqrt(dx * dx + dy * dy);
   }

    /* replace_violations
     *
     * This functions takes a population reference and the layout
     * constraints from the WindScenario as parameters, and automatically corrects
     * all turbine positions within the given population by new
     * random positions. No return value.
     *
     * parameters: std::vector<individual> &population
     * 			   double radius, int width, int height
     *
     * return: void
     */
   void replace_violations(std::vector<individual> &population, Scenario &scenario) {
        double radius = scenario.R * 8.0001; //distance must be radius*8
        //population.at(i).layout.at(j).x
        //thirst loop to pick an individual, second to pick a coordinate to be checked
        //third loop to compare the chosen coordinate with all other coordinates
        //within the individuals layout
        for (int i = 0; i < population.size(); i++) {
            for (int j = 0; j < population.at(i).layout.size(); j++){
                for (int coord = 0; coord < population.at(i).layout.size();) {
                    //preventing the comparison to the coordinate itself
                    if (j == coord) {
                        coord++;
                        continue;
                    }
                    // checks if the x and y values are NOT within the violation distance
                    // off the corresponding coordinate
                    // Both values, x and y, must be within that range
                    // either under or above the given coordinate
                    // Each "successfull" if branch proves that no violation exists
                    // for this pair, and thereby continues to the next coordinate /
                    // iteration
                    if (turbine_distance(population[i].layout[j],
                                         population[i].layout[coord]) > radius) {
                       coord++;
                       continue;
                    }

                    // violation occured:
                    // The violation is proven if no if branch above is triggered
                    // the violating coordinate will be randomly replaced, and all
                    // other coordinates within the individual will be checked against
                    // the new one (coord = 0)
                    population[i].layout[j].x = scenario.width * (double) rand()
                       / (double) RAND_MAX;
                    population[i].layout[j].y = scenario.height * (double) rand()
                       / (double) RAND_MAX;
                    coord = 0;
                }

            }
        }
    }
   
    /* evaluate_population
     *
     * This functions takes the population and the evaluator
     * as parameter, casts the population's individuals
     * to matrixes and evaluates them, storing the new fitnesses
     * in the population structures, no return value;
     *
     * parameters: std::vector<individual> &population, WindScenario &wscenario
     *
     * return: void
     *
    */
    void evaluate_population(WindFarmLayoutEvaluator &evaluator,
                             std::vector<individual> &population) {
        //cast to Matrix
        Matrix<double> test_matrix(population.at(0).layout.size(),2);
        
        for(int j = 0; j < population.size(); j++) {       
            for(int i = 0; i < population.at(j).layout.size(); i++) {
                test_matrix.set(i,0,population.at(j).layout.at(i).x);
                test_matrix.set(i,1,population.at(j).layout.at(i).y);
            }
            population.at(j).fitness = evaluator.evaluate(&test_matrix);        
        }                
    }
   
    individual create_individual_2(WindFarmLayoutEvaluator &evaluator,
                                   Scenario &scenario){
        // The class Random is used for wrapping a random number generator and a
        // uniform real distribution
        Random rand;
        
        // There is a security constraint between turbines, they can't be
        // less than 8R near, which means that they have to be more than
        // 8R far away from each other
        double min_distance = 8.0 * scenario.R;
        double factor = min_distance * 1.000001;
        // These are the farm's constraints
        double width = scenario.width / factor;
        double height = scenario.height / factor;
        std::vector<coordinate> layout;
        
        int count = 0;
        int max_n_turbines = scenario.max_turbines;
        
        // We check how many turbines we can stack up in this scenario, and check how
        // it compares with the number of turbines given in it
        /*for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                double xpos = x * factor;
                double ypos = y * factor;
                bool valid = true;
                Matrix<double> scenario_obstacles = wscenario.obstacles;
                // We check for obstacle constraints
                if (!functions::coordinateCollidesWithObstacles(x, y, evaluator)){
                    max_n_turbines++;
                }
            }
        }
        // If the max n turbines is more than the scenario's ones,
        // we take the latter one
        max_n_turbines = max_n_turbines <= wscenario.nturbines ? max_n_turbines : wscenario.nturbines;*/
        
        int n_turbines = rand.DrawNumber<int>(3.0 / 4.0 * max_n_turbines,
                                              max_n_turbines);
                
        // We create all the turbines
        individual indiv;
        while (count < n_turbines) {
            double x = rand.DrawNumber<double>(0, width) * factor;
            double y = rand.DrawNumber<double>(0, height) * factor;
            // Checks if the coordinate generated collides with any other
            // turbine
            if(!functions::turbine_collides(x, y, scenario, indiv.layout)) {
                struct coordinate coord = {
                    x, //x coordinate
                    y, //y coordinate
                };

                indiv.layout.push_back(coord);
                count++;
            }
        };
        // We calculate the fitness of the individual, or the layout
        Matrix<double> mat_layout = functions::individual_to_matrix<double>(indiv.layout);
        indiv.fitness = evaluator.evaluate(&mat_layout);
        return indiv;
    }

    std::vector<individual> initialization_2(WindFarmLayoutEvaluator &evaluator,
                                             Scenario &scenario,
                                             int num_population){
            std::vector<individual> population;
        
            // We start populating
            for(int i = 0; i < num_population; ++i) {
                struct individual indiv =
                        initialization::create_individual_2(evaluator, scenario);
                population.push_back(indiv);
            }

            return population;
    }
}

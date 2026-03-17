#pragma once
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Eigen>
/**
 * Typedef for the fitness function return type.
 *
 * This is an example, change it for your specific problem.
 *
 */
typedef float tFitness;

/**
 * Domain of each element
 */
template<typename tDomain>
using tSolution = Eigen::VectorX<tDomain>;

/**
 * Represent the new option to select the heuristic
 */
typedef tFitness tHeuristic;

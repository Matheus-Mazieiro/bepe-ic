#ifndef REDUCEDINSTANCESOLVER_H
#define REDUCEDINSTANCESOLVER_H

#include "ortools/sat/cp_model.h"

#include "Input.hpp"
#include "Settings.hpp"
#include <vector>

#include "TourEnhancement.hpp"

using namespace operations_research;
using namespace operations_research::sat;

class ReducedInstanceSolver
{
private:
public:
    Input &input;
    CpSolverResponse response;
    ReducedInstanceSolver(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour);
    CpSolverResponse Solve(int seed, std::vector<std::vector<bool>> &tour);
    static void dfs_find_cycles(int current, int start, const std::vector<std::vector<BoolVar>> &matrix, int sub_begin, int sub_end, std::vector<bool> &visited, std::vector<int> &path, std::vector<std::vector<std::pair<int, int>>> &result, const CpSolverResponse response);
    std::vector<std::vector<std::pair<int, int>>> Subciclos(std::vector<std::vector<BoolVar>> &matrix, int submatrix_begin, int submatrix_end, const CpSolverResponse response);
    void PrintSolution(CpSolverResponse &response, std::vector<std::vector<BoolVar>> &x, std::vector<std::vector<std::vector<std::vector<BoolVar>>>> &y);
};

#endif
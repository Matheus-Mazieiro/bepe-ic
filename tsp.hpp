#ifndef TSP_H
#define TSP_H

#include "ortools/sat/cp_model.h"

#include "Input.hpp"
#include "Settings.hpp"
#include <vector>

using namespace operations_research;
using namespace operations_research::sat;

class TSP
{
private:
    std::vector<std::vector<int>> Subcycles(std::vector<int> &sol);
    std::vector<int> SolutionTour(const CpSolverResponse &solver, const std::vector<std::vector<BoolVar>> &sol);

public:
    Input &input;
    Settings &settings;
    std::vector<int> tour; // Tour no formato v[i]=j, se existe a aresta (i, j)
    CpSolverResponse response;

    TSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour);
    void SolveTSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour);
    void ExactTSP(Input &input, std::vector<std::vector<bool>> &tour);
    void ExactTSP(Input &input, std::vector<std::vector<bool>> &tour, int begin, int end);
};

#endif
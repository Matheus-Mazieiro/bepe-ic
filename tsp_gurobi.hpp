#ifndef TSPGUROBI_H
#define TSPGUROBI_H

// #include "ortools/sat/cp_model.h"
#include "gurobi_c++.h"

#include "Input.hpp"
#include "Settings.hpp"
#include <vector>

class TSP
{
private:
public:
    Input &input;
    Settings &settings;
    std::vector<int> tour; // Tour no formato v[i]=j, se existe a aresta (i, j)

    TSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour);
    void SolveTSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour);
    void ExactTSP(Input &input, std::vector<std::vector<bool>> &tour);
    // void ExactTSP(Input &input, std::vector<std::vector<bool>> &tour, int begin, int end);
};

class SubtourEliminationCallback : public GRBCallback
{
public:
    SubtourEliminationCallback(std::vector<std::vector<GRBVar>> &x_, int n_);

protected:
    void callback() override;

private:
    std::vector<std::vector<GRBVar>> &x;
    int n;

    std::vector<int> findSubtour(const std::vector<std::vector<double>> &sol);
};

#endif
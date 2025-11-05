#ifndef REDUCEDINSTANCESOLVER_H
#define REDUCEDINSTANCESOLVER_H

#include "gurobi_c++.h"

#include "Input.hpp"
#include "Settings.hpp"
#include <algorithm>
#include <set>
#include <vector>

#include "TourEnhancement.hpp"
#include "Printer.hpp"

class ReducedInstanceSolver
{
private:
public:
    Input input;
    Settings &settings;
    // CpSolverResponse response;
    ReducedInstanceSolver(Input input, Settings &settings, std::vector<std::vector<bool>> &tour);
    // CpSolverResponse Solve(int seed, std::vector<std::vector<bool>> &tour);
    void Solve(int seed, std::vector<std::vector<bool>> &tour);
    // static void dfs_find_cycles(int current, int start, const std::vector<std::vector<BoolVar>> &matrix, int sub_begin, int sub_end, std::vector<bool> &visited, std::vector<int> &path, std::vector<std::vector<std::pair<int, int>>> &result, const CpSolverResponse response);
    // std::vector<std::vector<std::pair<int, int>>> Subciclos(std::vector<std::vector<BoolVar>> &matrix, int submatrix_begin, int submatrix_end, const CpSolverResponse response);
    //  void PrintSolution(CpSolverResponse &response, std::vector<std::vector<BoolVar>> &x, std::vector<std::vector<std::vector<std::vector<BoolVar>>>> &y);
};

class SubPathEliminationCallback : public GRBCallback
{
private:
    std::vector<std::vector<int>> findDroneSubtours(int truck_i, int truck_j);

protected:
    void callback() override;

public:
    std::vector<std::vector<std::vector<std::vector<GRBVar>>>> &y_vars;
    int num_truck_nodes;
    int num_drone_nodes; // Número de nós que o drone pode visitar (Vd)

    // Construtor para inicializar as referências às variáveis
    SubPathEliminationCallback(std::vector<std::vector<std::vector<std::vector<GRBVar>>>> &vars, int n_truck, int n_drone);
};

#endif
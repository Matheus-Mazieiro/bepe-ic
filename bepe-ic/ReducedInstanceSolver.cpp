#include "ReducedInstanceSolver.hpp"

ReducedInstanceSolver::ReducedInstanceSolver(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour) : input(input)
{
    response = Solve(settings.seed, tour);
}

CpSolverResponse ReducedInstanceSolver::Solve(int seed, std::vector<std::vector<bool>> &tour)
{
    int n_truck = input.num_truck_nodes;
    int n_drone = input.num_drone_nodes;
    int n_truck_l = n_truck + 1;
    int n_total = n_truck + n_drone + 1;

    // Modelo
    CpModelBuilder cp_model;

    auto name = [](const std::string &base, int a, int b = -1, int c = -1, int d = -1)
    {
        std::string result = base + "_" + std::to_string(a);
        if (b != -1)
            result += "_" + std::to_string(b);
        if (c != -1)
            result += "_" + std::to_string(c);
        if (d != -1)
            result += "_" + std::to_string(d);
        return result;
    };

    // Variáveis
    std::vector<std::vector<BoolVar>> x(n_truck_l, std::vector<BoolVar>(n_truck_l));
    std::vector<std::vector<std::vector<std::vector<BoolVar>>>> y(n_truck_l,
                                                                  std::vector<std::vector<std::vector<BoolVar>>>(n_truck_l,
                                                                                                                 std::vector<std::vector<BoolVar>>(n_total,
                                                                                                                                                   std::vector<BoolVar>(n_total))));
    std::vector<BoolVar> z(n_drone);
    std::vector<std::vector<BoolVar>> h(n_total, std::vector<BoolVar>(n_total));
    std::vector<IntVar> u(n_truck);
    std::vector<IntVar> v(n_total);
    for (int i = 0; i < n_truck_l; i++)
        for (int j = 0; j < n_truck_l; j++)
        {
            x[i][j] = cp_model.NewBoolVar();
            for (int m = 0; m < n_total; m++)
                for (int n = 0; n < n_total; n++)
                    y[i][j][m][n] = cp_model.NewBoolVar();
        }
    for (int i = 0; i < n_drone; i++)
        z[i] = cp_model.NewBoolVar();
    for (int m = 0; m < n_total; m++)
    {
        v[m] = cp_model.NewIntVar(Domain(1, n_total));
        for (int n = 0; n < n_total; n++)
            h[m][n] = cp_model.NewBoolVar();
    }
    for (int i = 0; i < n_truck; i++)
        u[i] = cp_model.NewIntVar(Domain(1, n_truck));

    // Objetivo
    {
        LinearExpr obj;
        for (int m = 0; m < n_drone; m++)
            obj += input.drones_nodes_profits[m + n_truck_l] * z[m];
        for (int m = 0; m < n_total; m++)
            for (int n = 0; n < n_total; n++)
                obj += input.drone_profits_graph[m][n] * h[m][n];
        cp_model.Maximize(obj);
    }

    // Restrições
    // 2
    {
        for (int j = 1; j < n_truck + 1; j++)
        {
            LinearExpr left_side;
            LinearExpr right_side;
            for (int i = 0; i < n_truck_l; i++)
            {
                if (i != j)
                {
                    left_side += x[i][j];
                    right_side += x[j][i];
                }
            }
            cp_model.AddEquality(left_side, right_side);
            cp_model.AddLessOrEqual(right_side, 1);
        }
    }

    // 3
    {
        LinearExpr left_side;
        LinearExpr right_side;
        for (int i = 1; i < n_truck + 1; i++)
        {
            left_side += x[i][0];
            right_side += x[0][i];
        }
        cp_model.AddEquality(left_side, right_side);
        cp_model.AddEquality(right_side, 1);
    }

    // 4
    {
        int M = n_truck;
        for (int i = 1; i < n_truck + 1; i++)
        {
            for (int j = 1; j < n_truck + 1; j++)
            {
                if (i != j)
                {
                    cp_model.AddLessOrEqual(u[i - 1] - u[j - 1] + 1, M * (1 - x[i][j]));
                }
            }
        }
    }

    // 5 - Mudei de \in V_d para \in V
    {
        for (int i = 0; i < n_truck_l; i++)
        {
            for (int j = 0; j < n_truck_l; j++)
            {
                for (int n = 0; n < n_total; n++)
                {
                    LinearExpr left_side;  // Quantidade de arestas que chegam
                    LinearExpr right_side; // Quantidade de arestas que saem
                    for (int m = 0; m < n_total; m++)
                    {
                        left_side += y[i][j][m][n];
                        right_side += y[i][j][n][m];
                    }

                    if (n != i && n != j) // Preserva fluxo em vertices intermediarios
                        cp_model.AddEquality(left_side, right_side);
                    if (n == i) // Uma aresta a mais sai de i
                        cp_model.AddEquality(x[i][j], right_side - left_side);
                    if (n == j) // Uma aresta a mais chega em j
                        cp_model.AddEquality(x[i][j], left_side - right_side);

                    // cp_model.AddLessOrEqual(right_side, 1); // Caso eu queira que vértices nao possam ser repetidos
                }
            }
        }
    }

    // 6 - Corrigi de \in V_d para \in V, o jumento do cara não sabe que a porra do drone pode visitar truck nodes
    {
        for (int i = 0; i < n_truck_l; i++)
        {
            for (int j = 0; j < n_truck_l; j++)
            {
                LinearExpr left_side;
                for (int m = 0; m < n_total; m++)
                {
                    for (int n = 0; n < n_total; n++)
                        left_side += input.drone_graph[m][n] * y[i][j][m][n];
                    // left_side += input.drone_graph[m][j] * y[i][j][m][j]; isso deveria estar no codigo???
                }

                cp_model.AddLessOrEqual(left_side, input.t_max);
            }
        }
    }

    // 7 - Removido, nao faz sentido, ver restrições 5
    {
    }

    // 8 - Eliminação de subtours feita de forma análoga à restrição 4
    {
    }

    // 9
    {
        for (int m = 0; m < n_drone; m++)
        {
            LinearExpr left_side;

            for (int i = 0; i < n_truck_l; i++)
            {
                for (int j = 0; j < n_truck_l; j++)
                {
                    for (int n = 0; n < n_total; n++)
                    {
                        left_side += y[i][j][m + n_truck_l][n];
                    }
                    // left_side += y[i][j][m + n_truck_l][j]; isso deveria estar no codigo???
                }
            }

            cp_model.AddGreaterOrEqual(left_side, z[m]);
        }
    }

    // 10
    {
        for (int m = 0; m < n_total; m++)
        {
            for (int n = 0; n < n_total; n++)
            {
                LinearExpr left_side;

                for (int i = 0; i < n_truck_l; i++)
                {
                    for (int j = 0; j < n_truck_l; j++)
                    {
                        left_side += y[i][j][m][n];
                    }
                }
                cp_model.AddGreaterOrEqual(left_side, h[m][n]);
            }
        }
    }

    // 11
    {
        int M = (n_total + 1) * (n_total + 1);
        for (int i = 0; i < n_truck_l; i++)
        {
            for (int j = 0; j < n_truck_l; j++)
            {
                LinearExpr left_side;

                for (int m = 0; m < n_total; m++)
                {
                    for (int n = 0; n < n_total; n++)
                        left_side += y[i][j][m][n];
                    // left_side += y[i][j][m][j];
                }
                // for (int n = 0; n < n_total; n++)
                //     left_side += y[i][j][i][n];
                // left_side += y[i][j][i][j];

                cp_model.AddLessOrEqual(left_side, M * x[i][j]);
            }
        }
    }

    // 12
    {
        int M = (n_total + 1) * (n_total + 1);
        for (int i = 0; i < n_truck_l; i++)
        {
            LinearExpr left_side;
            LinearExpr right_side;
            for (int m = 0; m < n_total; m++)
            {
                for (int n = 0; n < n_total; n++)
                    left_side += y[i][i][m][n];
                // left_side += y[i][i][m + n_truck_l][i];
            }
            // for (int n = 0; n < n_drone; n++)
            //     left_side += y[i][i][i][n + n_truck_l];
            // left_side += y[i][i][i][i];

            for (int j = 0; j < n_truck_l; j++)
            {
                if (i != j)
                    right_side += x[i][j];
            }
            cp_model.AddLessOrEqual(left_side, M * right_side);
        }
    }

    // 13 - selfedge (Faz sentido??)
    {
        // for (int i = 0; i < n_truck_l; i++)
        //{
        //     cp_model.AddEquality(x[i][i], false);
        //     for (int m = 0; m < n_total; m++)
        //     {
        //         for (int n = 0; n < n_total; n++)
        //         {
        //             cp_model.AddEquality(y[i][i][m][n], false);
        //         }
        //     }
        //     for (int j = 0; j < n_truck_l; j++)
        //     {
        //         for (int m = 0; m < n_total; m++)
        //         {
        //             cp_model.AddEquality(y[i][j][m][m], false);
        //         }
        //     }
        // }
    }

    // 14 - Tour de entrada
    {
        if (tour.size() == n_total && tour[0].size() == n_total)
        {
            std::cout << "TSP constraint: " << std::endl;
            for (int m = 0; m < n_total; m++) // Should be from 0 to n_total? Or from n_truck_l to n_total?
            {
                for (int n = 0; n < n_total; n++)
                {
                    if ((n < n_truck_l && m >= n_truck_l) || (n >= n_truck_l && m < n_truck_l)) // Should not create constraint between Vd and Vt'?
                    {
                        std::cout << "? ";
                        continue;
                    }
                    if (tour[m][n] == false)
                    {
                        std::cout << "x ";
                        for (int i = 0; i < n_truck_l; i++)
                        {
                            for (int j = 0; j < n_truck_l; j++)
                            {
                                cp_model.AddEquality(y[i][j][m][n], false);
                            }
                        }
                    }
                    else
                        std::cout << "? ";
                }
                std::cout << std::endl;
            }
        }
    }

    // Parametros
    SatParameters parameters;
    parameters.set_random_seed(seed);

    // Solver
    CpSolverResponse response;
    std::vector<int> solutionTour;
    std::vector<std::vector<int>> subtours;
    bool subcycle = true;
    while (subcycle)
    {
        subcycle = false;

        Model model;
        model.Add(NewSatParameters(parameters));
        response = SolveCpModel(cp_model.Build(), &model);

        // std::cout << "Solver status: " << CpSolverStatus_Name(response.status()) << std::endl;

        if (response.status() == CpSolverStatus::MODEL_INVALID)
            exit(1);

        if (response.status() == CpSolverStatus::OPTIMAL || response.status() == CpSolverStatus::FEASIBLE)
        {
            // std::cout << "(" << response.objective_value() << ")Found Solution: ";

            for (int i = 0; i < n_truck_l; i++)
            {
                for (int j = 0; j < n_truck_l; j++)
                {
                    if (SolutionBooleanValue(response, x[i][j]))
                    {
                        // Verificar se tem um subciclo no drone vertices

                        std::vector<std::vector<std::pair<int, int>>> ciclyes = Subciclos(y[i][j], n_truck_l, n_total - 1, response);
                        if (ciclyes.size() > 0)
                        {
                            subcycle = true;
                            for (std::vector<std::pair<int, int>> c : ciclyes)
                            {
                                LinearExpr left_side;
                                for (std::pair<int, int> p : c)
                                {
                                    left_side += y[i][j][p.first][p.second];
                                }
                                cp_model.AddLessOrEqual(left_side, c.size() - 1);
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "Total: " << response.objective_value() << std::endl;
    // PrintSolution(response, x, y);

    std::cout << "Enhancing tour: " << std::endl;
    std::vector<std::vector<bool>> x_bool(x.size(), std::vector<bool>(x.size()));
    std::vector<std::vector<std::vector<std::vector<bool>>>> y_bool(y.size(), std::vector<std::vector<std::vector<bool>>>(y[0].size(), std::vector<std::vector<bool>>(y[0][0].size(), std::vector<bool>(y[0][0][0].size()))));
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            x_bool[i][j] = SolutionBooleanValue(response, x[i][j]);
            for (int m = 0; m < y[i][j].size(); m++)
            {
                for (int n = 0; n < y[i][j][m].size(); n++)
                {
                    y_bool[i][j][m][n] = SolutionBooleanValue(response, y[i][j][m][n]);
                }
            }
        }
    }

    //TourEnhancement::Enhance(input, x_bool, y_bool);
    std::cout << std::endl;
    std::cout << std::endl;
    return response;
}

void ReducedInstanceSolver::dfs_find_cycles(
    int current,
    int start,
    const std::vector<std::vector<BoolVar>> &matrix,
    int sub_begin,
    int sub_end,
    std::vector<bool> &visited,
    std::vector<int> &path,
    std::vector<std::vector<std::pair<int, int>>> &result,
    const CpSolverResponse response)
{
    int n = (int)matrix.size();
    // Explore all possible neighbors in the sub-range
    for (int w = sub_begin; w <= sub_end; ++w)
    {
        if (!SolutionBooleanValue(response, matrix[current][w]))
            continue; // no edge

        if (w == start)
        {
            // Potential cycle closure
            if (path.size() >= 2)
            {
                // Record the cycle edges along the current path + (current->start)
                std::vector<std::pair<int, int>> cycleEdges;
                for (size_t i = 0; i + 1 < path.size(); ++i)
                {
                    int u = path[i];
                    int v = path[i + 1];
                    cycleEdges.emplace_back(u, v);
                }
                // closing edge
                cycleEdges.emplace_back(current, start);
                result.push_back(std::move(cycleEdges));
            }
            // Do not recurse further on w == start
        }
        else if (w > start && !visited[w])
        {
            // Continue DFS deeper, only if w > start (ensuring start is min in cycle)
            visited[w] = true;
            path.push_back(w);
            dfs_find_cycles(w, start, matrix, sub_begin, sub_end, visited, path, result, response);
            path.pop_back();
            visited[w] = false;
        }
        // else: either w < start (skip to avoid duplicates), or visited[w]==true (would form non-simple path)
    }
}

std::vector<std::vector<std::pair<int, int>>> ReducedInstanceSolver::Subciclos(
    std::vector<std::vector<BoolVar>> &matrix,
    int submatrix_begin,
    int submatrix_end,
    const CpSolverResponse response)
{
    int n = (int)matrix.size();
    if (submatrix_begin < 0 || submatrix_end < submatrix_begin || submatrix_end >= n)
    {
        throw std::out_of_range("Invalid submatrix_begin/end indices");
    }
    // Optional: check square matrix
    for (const auto &row : matrix)
    {
        if ((int)row.size() != n)
        {
            throw std::invalid_argument("Adjacency matrix must be square");
        }
    }

    std::vector<std::vector<std::pair<int, int>>> result;
    std::vector<bool> visited(n, false);
    std::vector<int> path;
    // For each start vertex in the sub-range
    for (int s = submatrix_begin; s <= submatrix_end; ++s)
    {
        visited[s] = true;
        path.clear();
        path.push_back(s);
        dfs_find_cycles(s, s, matrix, submatrix_begin, submatrix_end, visited, path, result, response);
        path.pop_back();
        visited[s] = false;
    }
    return result;
}

void ReducedInstanceSolver::PrintSolution(CpSolverResponse &response, std::vector<std::vector<BoolVar>> &x, std::vector<std::vector<std::vector<std::vector<BoolVar>>>> &y)
{
    if (response.status() == CpSolverStatus::INFEASIBLE)
    {
        std::cout << CpSolverStatus_Name(response.status()) << std::endl;
        return;
    }

    std::cout << "Truck tour:" << std::endl;
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            if (SolutionBooleanValue(response, x[i][j]))
                std::cout << "1 ";
            else
                std::cout << "- ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Drone tour:" << std::endl;
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            if (SolutionBooleanValue(response, x[i][j]))
            {
                double cost = 0;
                std::cout << "i == " << i << "    j == " << j << std::endl;
                for (int m = 0; m < y[i][j].size(); m++)
                {
                    for (int n = 0; n < y[i][j][m].size(); n++)
                    {
                        if (SolutionBooleanValue(response, y[i][j][m][n]))
                        {
                            cost += input.drone_graph[m][n];
                            std::cout << "1 ";
                        }
                        else
                            std::cout << "- ";
                    }
                    std::cout << std::endl;
                }
                std::cout << "cost == " << cost << "/" << input.t_max << std::endl;
                std::cout << std::endl;
            }
        }
    }
}

#include "tsp_gurobi.hpp"

TSP::TSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour) : input(input), settings(settings)
{
    SolveTSP(input, settings, tour);
}

void TSP::SolveTSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour)
{
    if (settings.instance_reduction_method == "exact")
        return ExactTSP(input, tour);
    // if (settings.instance_reduction_method == "partial")
    //     return ExactTSP(input, tour, input.num_truck_nodes, input.num_nodes);
    if (settings.instance_reduction_method == "memetic")
        std::cout << "To be implemented" << std::endl;
    if (settings.instance_reduction_method == "memetic_variation")
        std::cout << "To be implemented" << std::endl;
    if (settings.instance_reduction_method == "nearest_neighbor")
        std::cout << "To be implemented" << std::endl;
    if (settings.instance_reduction_method == "nearest_insertion")
        std::cout << "To be implemented" << std::endl;
    if (settings.instance_reduction_method == "farthest_insertion")
        std::cout << "To be implemented" << std::endl;
}

void TSP::ExactTSP(Input &input, std::vector<std::vector<bool>> &tour)
{
    int n = input.num_nodes;

    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "tsp.log");
    env.start();

    // Modelo
    GRBModel model = GRBModel(env);

    // Variáveis
    std::vector<std::vector<GRBVar>> x(n, std::vector<GRBVar>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + i + j);

    // Restricoes
    for (int j = 0; j < n; j++)
    {
        GRBLinExpr c1;
        GRBLinExpr c2;
        for (int i = 0; i < n; i++)
        {
            if (i == j)
                continue;
            c1 += x[i][j];
            c2 += x[j][i];
        }
        model.addConstr(c1 == 1);
        model.addConstr(c2 == 1);
    }

    // Lazy Constraint
    model.set(GRB_IntParam_LazyConstraints, 1);
    SubtourEliminationCallback cb(x, n);
    model.setCallback(&cb);

    // Objetivo
    GRBLinExpr obj;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j)
                obj += input.drone_profits_graph[i][j] * x[i][j];
    model.setObjective(obj, GRB_MAXIMIZE);

    model.optimize();

    std::cout << "TSP solution with profit: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;

    tour = std::vector<std::vector<bool>>(x.size(), std::vector<bool>(x[0].size()));
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            tour[i][j] = x[i][j].get(GRB_DoubleAttr_X) > 0.5;
        }
    }
}

SubtourEliminationCallback::SubtourEliminationCallback(std::vector<std::vector<GRBVar>> &x, int n) : x(x), n(n) {}

void SubtourEliminationCallback::callback()
{
    if (where == GRB_CB_MIPSOL)
    {
        std::vector<std::vector<double>> sol(n, std::vector<double>(n));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                sol[i][j] = getSolution(x[i][j]);
            }
        }

        std::vector<int> tour = findSubtour(sol);

        if (tour.size() < n)
        {
            GRBLinExpr expr = 0;
            for (int i = 0; i < tour.size(); i++)
            {
                for (int j = 0; j < tour.size(); j++)
                {
                    if (i != j)
                        expr += x[tour[i]][tour[j]];
                }
            }
            addLazy(expr <= static_cast<int>(tour.size()) - 1);
        }
    }
}

std::vector<int> SubtourEliminationCallback::findSubtour(const std::vector<std::vector<double>> &sol)
{
    std::vector<bool> visited(n, false);
    std::vector<int> best_tour;
    for (int start = 0; start < n; start++)
    {
        if (visited[start])
            continue;
        std::vector<int> tour;
        int current = start;

        while (!visited[current])
        {
            visited[current] = true;
            tour.push_back(current);

            for (int j = 0; j < n; j++)
            {
                if (sol[current][j] > 0.5)
                {
                    current = j;
                    break;
                }
            }
        }
        if (tour.size() < best_tour.size() || best_tour.empty())
            best_tour = tour;
    }
    return best_tour;
}
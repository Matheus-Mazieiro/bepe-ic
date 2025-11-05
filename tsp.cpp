#include "tsp.hpp"

TSP::TSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour) : input(input), settings(settings)
{
    SolveTSP(input, settings, tour);
}

void TSP::SolveTSP(Input &input, Settings &settings, std::vector<std::vector<bool>> &tour)
{
    if (settings.instance_reduction_method == "exact")
        return ExactTSP(input, tour);
    if (settings.instance_reduction_method == "partial")
        return ExactTSP(input, tour, input.num_truck_nodes, input.num_nodes);
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

    // Modelo
    CpModelBuilder cp_model;

    // Variáveis
    std::vector<std::vector<BoolVar>> x(n, std::vector<BoolVar>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            x[i][j] = cp_model.NewBoolVar().WithName("x_" + i + j);

    // Restricoes
    for (int j = 0; j < n; j++)
    {
        LinearExpr constraint1;
        LinearExpr constraint2;
        for (int i = 0; i < n; i++)
        {
            if (i == j)
                continue;
            constraint1 += x[i][j];
            constraint2 += x[j][i];
        }
        cp_model.AddEquality(constraint1, 1);
        cp_model.AddEquality(constraint2, 1);
    }

    // Objetivo
    LinearExpr obj;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j)
                obj += input.drone_profits_graph[i][j] * x[i][j];
    cp_model.Maximize(obj);

    // Solver (lazy constraint)
    std::vector<int> solutionTour;
    std::vector<std::vector<int>> subtours;
    CpSolverResponse response;
    while (true)
    {
        response = Solve(cp_model.Build());

        // Resultado
        std::cout << "Solver status: " << CpSolverStatus_Name(response.status()) << std::endl;

        // Se encontrou uma candidata
        if (response.status() == CpSolverStatus::OPTIMAL || response.status() == CpSolverStatus::FEASIBLE)
        {
            std::cout << "Found Solution: ";

            // Adiciona restrições dinamicamente (caso exista subtour)
            solutionTour = SolutionTour(response, x);
            subtours = Subcycles(solutionTour);
            if (subtours.size() != 1)
            {
                std::cout << "Invalid (Have " << subtours.size() << " subtours)" << std::endl;
                for (std::vector<int> v : subtours)
                {
                    LinearExpr subtour_lhs;

                    for (int u_node : v)
                        for (int v_node : v)
                            if (u_node != v_node)
                                subtour_lhs += x[u_node][v_node];

                    cp_model.AddLessOrEqual(subtour_lhs, static_cast<int>(v.size()) - 1);
                }
            }
            else
                break;
        }
    }
    for (auto s : solutionTour)
        std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "TSP solution with profit: " << response.objective_value() << std::endl;

    tour = std::vector<std::vector<bool>>(x.size(), std::vector<bool>(x[0].size()));
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            tour[i][j] = SolutionBooleanValue(response, x[i][j]);
        }
    }
}

std::vector<std::vector<int>> TSP::Subcycles(std::vector<int> &sol)
{
    int n = sol.size();
    std::vector<bool> visited(n, false);
    std::vector<std::vector<int>> subcycles;
    for (int i = 0; i < n; i++)
    {
        std::vector<int> subcycle;

        int cur = i;
        while (!visited[cur])
        {
            visited[cur] = true;
            subcycle.push_back(cur);
            cur = sol[cur];
        }

        if (subcycle.size() > 0)
            subcycles.push_back(subcycle);
    }
    return subcycles;
}

std::vector<int> TSP::SolutionTour(const CpSolverResponse &solver, const std::vector<std::vector<BoolVar>> &sol)
{
    int n = sol.size();
    std::vector<int> tour(n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (SolutionBooleanValue(solver, sol[i][j]))
            {
                tour[i] = j;
                continue;
            }
        }
    }

    return tour;
}

void TSP::ExactTSP(Input &input, std::vector<std::vector<bool>> &tour, int begin, int end)
{
    int n = end - begin;

    // Modelo
    CpModelBuilder cp_model;

    // Variáveis
    std::vector<std::vector<BoolVar>> x(n, std::vector<BoolVar>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            x[i][j] = cp_model.NewBoolVar().WithName("x_" + i + j);

    // Restricoes
    for (int j = 0; j < n; j++)
    {
        LinearExpr constraint1;
        LinearExpr constraint2;
        for (int i = 0; i < n; i++)
        {
            if (i == j)
                continue;
            constraint1 += x[i][j];
            constraint2 += x[j][i];
        }
        cp_model.AddEquality(constraint1, 1);
        cp_model.AddEquality(constraint2, 1);
    }

    // Objetivo
    LinearExpr obj;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j)
                obj += input.drone_profits_graph[i + begin][j + begin] * x[i][j];
    cp_model.Maximize(obj);

    // Solver (lazy constraint)
    std::vector<int> solutionTour;
    std::vector<std::vector<int>> subtours;
    CpSolverResponse response;
    while (true)
    {
        response = Solve(cp_model.Build());

        // Resultado
        std::cout << "Solver status: " << CpSolverStatus_Name(response.status()) << std::endl;

        // Se encontrou uma candidata
        if (response.status() == CpSolverStatus::OPTIMAL || response.status() == CpSolverStatus::FEASIBLE)
        {
            std::cout << "Found Solution: ";

            // Adiciona restrições dinamicamente (caso exista subtour)
            solutionTour = SolutionTour(response, x);
            subtours = Subcycles(solutionTour);
            if (subtours.size() != 1)
            {
                std::cout << "Invalid (Have " << subtours.size() << " subtours)" << std::endl;
                for (std::vector<int> v : subtours)
                {
                    LinearExpr subtour_lhs;

                    for (int u_node : v)
                        for (int v_node : v)
                            if (u_node != v_node)
                                subtour_lhs += x[u_node][v_node];

                    cp_model.AddLessOrEqual(subtour_lhs, static_cast<int>(v.size()) - 1);
                }
            }
            else
                break;
        }
    }
    for (auto s : solutionTour)
        std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "TSP solution with profit: " << response.objective_value() << std::endl;

    tour = std::vector<std::vector<bool>>(input.num_nodes, std::vector<bool>(input.num_nodes));
    for (int i = 0; i < input.num_nodes; i++)
    {
        for (int j = 0; j < input.num_nodes; j++)
        {
            if ((i >= begin && i < end) && (j >= begin && j < end))
                tour[i][j] = SolutionBooleanValue(response, x[i - begin][j - begin]);
            else
                tour[i][j] = false;
        }
    }
}
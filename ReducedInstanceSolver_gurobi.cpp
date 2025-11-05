#include "ReducedInstanceSolver_gurobi.hpp"

ReducedInstanceSolver::ReducedInstanceSolver(Input input, Settings &settings, std::vector<std::vector<bool>> &tour) : input(input), settings(settings)
{
    std::cout << "Solving" << std::endl;
    Solve(settings.seed, tour);
}

void ReducedInstanceSolver::Solve(int seed, std::vector<std::vector<bool>> &tour)
{

    int n_truck = input.num_truck_nodes;
    int n_drone = input.num_drone_nodes;
    int n_truck_l = n_truck + 1;
    int n_total = n_truck + n_drone + 1;

    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "cdop.log");
    env.start();

    // Modelo
    GRBModel model = GRBModel(env);

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
    std::vector<std::vector<GRBVar>> x(n_truck_l, std::vector<GRBVar>(n_truck_l));
    std::vector<std::vector<std::vector<std::vector<GRBVar>>>> y(n_truck_l,
                                                                 std::vector<std::vector<std::vector<GRBVar>>>(n_truck_l,
                                                                                                               std::vector<std::vector<GRBVar>>(n_total,
                                                                                                                                                std::vector<GRBVar>(n_total))));
    std::vector<GRBVar> z(n_drone);
    std::vector<std::vector<GRBVar>> h(n_total, std::vector<GRBVar>(n_total));
    std::vector<GRBVar> u(n_truck);
    for (int i = 0; i < n_truck_l; i++)
        for (int j = 0; j < n_truck_l; j++)
        {
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
            for (int m = 0; m < n_total; m++)
                for (int n = 0; n < n_total; n++)
                    y[i][j][m][n] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }
    for (int i = 0; i < n_drone; i++)
        z[i] = model.addVar(0.0, 1.0, input.drones_nodes_profits[i + n_truck_l], GRB_BINARY);
    for (int m = 0; m < n_total; m++)
    {
        for (int n = 0; n < n_total; n++)
            h[m][n] = model.addVar(0.0, 1.0, input.drone_profits_graph[m][n], GRB_BINARY);
    }
    for (int i = 0; i < n_truck; i++)
        u[i] = model.addVar(1.0, n_truck, 0.0, GRB_INTEGER);

    // Objetivo
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // Restrições
    // 2
    {
        for (int j = 1; j < n_truck + 1; j++)
        {
            GRBLinExpr left_side;
            GRBLinExpr right_side;
            for (int i = 0; i < n_truck_l; i++)
            {
                if (i != j)
                {
                    left_side += x[i][j];
                    right_side += x[j][i];
                }
            }
            model.addConstr(left_side == right_side);
            model.addConstr(right_side <= 1);
        }
    }

    // 3
    {
        GRBLinExpr left_side;
        GRBLinExpr right_side;
        for (int i = 1; i < n_truck + 1; i++)
        {
            left_side += x[i][0];
            right_side += x[0][i];
        }
        model.addConstr(left_side == right_side);
        model.addConstr(right_side == 1);
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
                    model.addConstr(u[i - 1] - u[j - 1] + 1 <= M * (1 - x[i][j]));
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
                    GRBLinExpr left_side;  // Quantidade de arestas que chegam
                    GRBLinExpr right_side; // Quantidade de arestas que saem
                    for (int m = 0; m < n_total; m++)
                    {
                        left_side += y[i][j][m][n];
                        right_side += y[i][j][n][m];
                    }

                    if (n != i && n != j) // Preserva fluxo em vertices intermediarios
                        model.addConstr(left_side == right_side);
                    if (n == i) // Uma aresta a mais sai de i
                        model.addConstr(x[i][j] == right_side - left_side);
                    if (n == j) // Uma aresta a mais chega em j
                        model.addConstr(x[i][j] == left_side - right_side);

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
                GRBLinExpr left_side;
                for (int m = 0; m < n_total; m++)
                {
                    for (int n = 0; n < n_total; n++)
                        left_side += input.drone_graph[m][n] * y[i][j][m][n];
                    // left_side += input.drone_graph[m][j] * y[i][j][m][j]; isso deveria estar no codigo???
                }

                model.addConstr(left_side <= input.t_max);
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
            GRBLinExpr left_side;

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
            model.addConstr(left_side >= z[m]);
        }
    }

    // 10
    {
        for (int m = 0; m < n_total; m++)
        {
            for (int n = 0; n < n_total; n++)
            {
                GRBLinExpr left_side;

                for (int i = 0; i < n_truck_l; i++)
                {
                    for (int j = 0; j < n_truck_l; j++)
                    {
                        left_side += y[i][j][m][n];
                    }
                }
                model.addConstr(left_side >= h[m][n]);
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
                GRBLinExpr left_side;

                for (int m = 0; m < n_total; m++)
                {
                    for (int n = 0; n < n_total; n++)
                        left_side += y[i][j][m][n];
                }

                model.addConstr(left_side <= M * x[i][j]);
            }
        }
    }

    // 12
    {
        int M = (n_total + 1) * (n_total + 1);
        for (int i = 0; i < n_truck_l; i++)
        {
            GRBLinExpr left_side;
            GRBLinExpr right_side;
            for (int m = 0; m < n_total; m++)
            {
                for (int n = 0; n < n_total; n++)
                    left_side += y[i][i][m][n];
            }

            for (int j = 0; j < n_truck_l; j++)
            {
                if (i != j)
                    right_side += x[i][j];
            }
            model.addConstr(left_side <= M * right_side);
        }
    }

    // 13
    {
        std::vector<std::vector<GRBVar>> tau(n_truck_l, std::vector<GRBVar>(n_truck_l));
        for (int i = 0; i < n_truck_l; ++i)
        {
            for (int j = 0; j < n_truck_l; ++j)
            {
                tau[i][j] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "tau_" + std::to_string(i) + "_" + std::to_string(j));
            }
        }

        for (int i = 0; i < n_truck_l; ++i)
        {
            for (int j = 0; j < n_truck_l; ++j)
            {
                // Restrição 1: tau_ij >= t_ij * x_ij
                model.addConstr(tau[i][j] >= input.truck_graph[i][j] * x[i][j]);

                // Restrição 2: tau_ij >= sum_m sum_n t'_mn * y_ij_mn
                GRBLinExpr sum = 0;
                for (int m = 0; m < n_total; ++m)
                {
                    for (int n = 0; n < n_total; ++n)
                    {
                        sum += input.drone_graph[m][n] * y[i][j][m][n];
                    }
                }
                model.addConstr(tau[i][j] >= sum);

                model.addConstr(tau[i][j] <= input.d);
            }
        }
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
                                model.addConstr(y[i][j][m][n] == 0);
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

    // Lazy Constraint
    model.set(GRB_IntParam_LazyConstraints, 1);
    SubPathEliminationCallback cb(y, n_truck_l, n_total);
    model.setCallback(&cb);

    // Solver
    model.set(GRB_DoubleParam_TimeLimit, settings.max_iteration/1000); // Tempo em segundos
    model.set(GRB_IntParam_Seed, settings.seed);
    model.optimize();

    // std::cout << "Total: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
    //
    // for (int i = 0; i < n_truck_l; i++)
    // {
    // for (int j = 0; j < n_truck_l; j++)
    // {
    // if (x[i][j].get(GRB_DoubleAttr_X) > 0.5)
    // {
    // std::cout << "i = " << i << "      j = " << j << std::endl;
    // for (int m = 0; m < n_total; m++)
    // {
    // for (int n = 0; n < n_total; n++)
    // {
    // if (y[i][j][m][n].get(GRB_DoubleAttr_X) > 0.5)
    // std::cout << "1 ";
    // else
    // std::cout << "- ";
    // }
    // std::cout << std::endl;
    // }
    // }
    // }
    // }

    // std::cout << "Enhancing tour: " << std::endl;
    std::vector<std::vector<bool>> x_bool(x.size(), std::vector<bool>(x.size()));
    std::vector<std::vector<std::vector<std::vector<bool>>>> y_bool(y.size(), std::vector<std::vector<std::vector<bool>>>(y[0].size(), std::vector<std::vector<bool>>(y[0][0].size(), std::vector<bool>(y[0][0][0].size()))));
    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < x[i].size(); j++)
        {
            x_bool[i][j] = x[i][j].get(GRB_DoubleAttr_X) > 0.5;
            for (int m = 0; m < y[i][j].size(); m++)
            {
                for (int n = 0; n < y[i][j][m].size(); n++)
                {
                    y_bool[i][j][m][n] = y[i][j][m][n].get(GRB_DoubleAttr_X) > 0.5;
                }
            }
        }
    }

    Printer::UpdateStatus(x_bool, y_bool,
                          model.get(GRB_DoubleAttr_ObjVal),
                          -1,
                          std::chrono::high_resolution_clock::now(),
                          -1);
    Printer::opt = (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL);

    // TourEnhancement::Enhance(input, x_bool, y_bool);
    // std::cout << std::endl;
    // std::cout << std::endl;
}

SubPathEliminationCallback::SubPathEliminationCallback(std::vector<std::vector<std::vector<std::vector<GRBVar>>>> &vars, int n_truck, int n_drone)
    : y_vars(vars), num_truck_nodes(n_truck), num_drone_nodes(n_drone) {}

void SubPathEliminationCallback::callback()
{
    // Verificamos se estamos em um ponto onde o Gurobi encontrou uma nova solução inteira (MIPSOL)
    if (where == GRB_CB_MIPSOL)
    {
        try
        {
            // Iteramos sobre cada possível trecho de caminhão (i, j)
            for (int i = 0; i < num_truck_nodes; ++i)
            {
                for (int j = 0; j < num_truck_nodes; ++j)
                {
                    if (i == j)
                        continue;

                    // Função para encontrar sub-rotas de drones associadas ao trecho (i, j)
                    // Esta função usa getSolution() para obter os valores das variáveis y na solução atual
                    std::vector<std::vector<int>> cycles = findDroneSubtours(i, j);

                    // Se encontrarmos sub-rotas...
                    if (!cycles.empty())
                    {
                        // Adicionamos uma lazy constraint para cada sub-rota encontrada
                        for (const auto &cycle : cycles)
                        {
                            GRBLinExpr violated_constraint = 0;
                            // A restrição de corte soma todas as variáveis y dentro da sub-rota
                            for (size_t k = 0; k < cycle.size(); ++k)
                            {
                                int u = cycle[k];
                                int v = cycle[(k + 1) % cycle.size()]; // O próximo nó no ciclo
                                violated_constraint += y_vars[i][j][u][v];
                            }
                            // Adiciona a restrição: sum(y_uv) <= |S| - 1, onde S é o conjunto de nós na sub-rota
                            addLazy(violated_constraint <= (int)cycle.size() - 1);
                            // std::cout << "INFO: Lazy constraint adicionada para o trecho (" << i << "," << j << ") com " << cycle.size() << " nos." << std::endl;
                        }
                    }
                }
            }
        }
        catch (const GRBException &e)
        {
            std::cout << "Erro no callback: " << e.getMessage() << std::endl;
        }
    }
}

std::vector<std::vector<int>> SubPathEliminationCallback::findDroneSubtours(int truck_i, int truck_j)
{
    std::vector<std::vector<int>> subtours;
    std::vector<bool> visited(num_drone_nodes, false);

    // Assumimos que o nó '0' é o depot/origem para as rotas do drone
    // Nós visitáveis pelo drone são de 0 a num_drone_nodes-1
    for (int start_node = 0; start_node < num_drone_nodes; ++start_node)
    {
        if (!visited[start_node])
        {
            std::vector<int> current_tour;
            std::vector<int> stack;
            stack.push_back(start_node);

            while (!stack.empty())
            {
                int u = stack.back();
                stack.pop_back();

                if (!visited[u])
                {
                    visited[u] = true;
                    current_tour.push_back(u);

                    // Encontra o sucessor de 'u' na solução atual
                    for (int v = 0; v < num_drone_nodes; ++v)
                    {
                        if (u == v)
                            continue;
                        // getSolution() obtém o valor da variável na solução inteira atual
                        if (getSolution(y_vars[truck_i][truck_j][u][v]) > 0.5)
                        {
                            stack.push_back(v);
                            break; // Assumimos que cada nó tem no máximo um sucessor
                        }
                    }
                }
            }

            // Agora `current_tour` contém uma rota completa ou uma sub-rota.
            // Verificamos se é uma sub-rota válida (não contém o depot e tem mais de um nó)
            // O depot/nó de partida do caminhão é 'truck_i'. O nó de chegada é 'truck_j'
            // A rota do drone sai de 'truck_i' e volta para 'truck_j'.
            // No contexto da matriz y[i][j], os nós do drone podem ter uma origem e um destino locais.
            // A definição exata de sub-rota depende de como os nós do drone são mapeados.
            // Aqui, vamos assumir que uma sub-rota é um ciclo que não envolve a origem principal (ex: nó 0).
            bool is_a_subtour = std::find(current_tour.begin(), current_tour.end(), 0) == current_tour.end();
            if (current_tour.size() > 1 && is_a_subtour)
            {
                subtours.push_back(current_tour);
            }
        }
    }
    return subtours;
}

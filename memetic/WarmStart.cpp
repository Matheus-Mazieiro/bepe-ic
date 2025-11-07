#include "WarmStart.hpp"

WarmStart::WarmStart(Input &input, Settings &settings) : input(input), settings(settings)
{
}

/// @brief Insert drone nodes into the end of solution
/// @param solution The solution that drone nodes will be added
/// @return return True if a drone node can be added without violate any constraint
bool WarmStart::InsertDrone(std::vector<std::pair<int, bool>> &solution)
{
    // Colocar isso aqui numa funcao envelope para evitar recalculo...
    // Estoy computando a quantidade de visitas em cada vértice/aresta e vendo o impacto de cada um
    std::vector<std::vector<int>> drone_visiting_edges(input.num_nodes, std::vector<int>(input.num_nodes, 0));
    std::vector<int> drone_visiting_nodes(input.num_nodes, 0);
    double solution_time = 0;
    double solution_profit = 0;
    double last_op_drone_time = 0;
    int last_truck = 0;
    for (int i = 0; i < solution.size() - 1; i++)
    {
        drone_visiting_nodes[solution[i].first]++;
        drone_visiting_edges[solution[i].first][solution[i + 1].first]++;

        last_op_drone_time += input.drone_graph[solution[i].first][solution[i + 1].first];
        if (drone_visiting_edges[solution[i].first][solution[i + 1].first] == 1)
            solution_profit += input.drone_profits_graph[solution[i].first][solution[i + 1].first];
        if (drone_visiting_nodes[solution[i].first] == 1)
            solution_profit += input.drones_nodes_profits[solution[i].first];
        if (solution[i + 1].second)
        {
            solution_time += std::max(last_op_drone_time, input.truck_graph[last_truck][solution[i + 1].first]);
            last_op_drone_time = 0;
            last_truck = solution[i + 1].first;
        }
    }
    drone_visiting_nodes[solution.back().first]++;
    drone_visiting_edges[solution.back().first][solution[0].first]++;

    last_op_drone_time += input.drone_graph[solution.back().first][solution[0].first];
    if (drone_visiting_edges[solution.back().first][solution[0].first] == 1)
        solution_profit += input.drone_profits_graph[solution.back().first][solution[0].first];
    if (drone_visiting_nodes[solution.back().first] == 1)
        solution_profit += input.drones_nodes_profits[solution.back().first];
    solution_time += std::max(last_op_drone_time, input.truck_graph[last_truck][solution[0].first]);

    // Aqui eu insiro um drone node entre solution.back() e solution[0]
    // A idéia é ser um 'nearest neighbor'-like
    // Não ficou claro se eu quero minimizar o tempo ou maximizar o lucro na ideia de NN
    // Implementei um que maximiza o lucro
    // O argumento para usar 'minimiza tempo': memetico pode tirar vantagem
    // Outra abordagem que pode ser considerada: densidade lucro/tempo
    // Foi implementado um 'maximiza lucro'
    drone_visiting_edges[solution.back().first][solution[0].first]--;
    double profit_gain = drone_visiting_edges[solution.back().first][solution[0].first] == 0
                             ? -input.drone_profits_graph[solution.back().first][solution[0].first]
                             : 0;

    int best_node = -1;
    double best_gain = 0;
    double best_solution_time = 0;
    double best_last_op_drone_time = 0;
    for (int i = 0; i < input.num_nodes; i++)
    {
        double node_gain = profit_gain;
        if (drone_visiting_edges[solution.back().first][i] == 0)
            node_gain += input.drone_profits_graph[solution.back().first][i];
        if (drone_visiting_edges[i][solution[0].first] == 0)
            node_gain += input.drone_profits_graph[i][solution[0].first];

        double new_solution_time = solution_time;
        new_solution_time += -std::max(last_op_drone_time, input.truck_graph[last_truck][solution[0].first]);
        double new_last_op_drone_time = last_op_drone_time;
        new_last_op_drone_time += -input.drone_graph[solution.back().first][solution[0].first] +
                                  input.drone_graph[solution.back().first][i] +
                                  input.drone_graph[i][solution[0].first];
        new_solution_time += std::max(new_last_op_drone_time, input.truck_graph[last_truck][solution[0].first]);

        // Esse nó não 'cabe' na solução
        if (new_last_op_drone_time > input.t_max || new_solution_time > input.d)
            continue;

        if (node_gain > best_gain)
        {
            best_node = i;
            best_gain = node_gain;
            best_solution_time = new_solution_time;
            best_last_op_drone_time = new_last_op_drone_time;
        }
    }

    if (best_node < 0)
    {
        drone_visiting_edges[solution.back().first][solution[0].first]++;
        return false;
    }

    //std::cout << "Adding " << best_node << " improving sol by " << best_gain << std::endl;
    //std::cout << "best_last_op_drone_time = " << best_last_op_drone_time << " | best_solution_time = " << best_solution_time << " | old_solution_time = " << solution_time << std::endl;
    //std::cout << "Truck time: " << input.truck_graph[last_truck][solution[0].first]
    //          << " | Old drone time: " << last_op_drone_time
    //          << " | New drone time: " << best_last_op_drone_time << std::endl;

    drone_visiting_edges[solution.back().first][best_node]++;
    drone_visiting_edges[best_node][solution[0].first]++;
    solution.push_back(std::make_pair(best_node, false));
    return true;
}

/// @brief Incrementa solution ao adicionar um vertice de caminhão com a lógica chepest insertion
/// @param solution
/// @return return True if a drone node can be added without violate any constraint
bool WarmStart::InsertTruck(std::vector<std::pair<int, bool>> &solution)
{
    //std::cout << "Inserting truck" << std::endl;
    std::vector<int> truck_tour;
    std::vector<bool> in_tour(input.num_truck_nodes, false);
    double truck_time = 0;
    for (auto p : solution)
        if (p.second)
        {
            if (p.first != 0)
                truck_time += input.truck_graph[truck_tour.back()][p.first];
            truck_tour.push_back(p.first);
            in_tour[p.first] = true;
        }
    truck_time += input.truck_graph[truck_tour.back()][truck_tour.front()];

    while (truck_tour.size() < in_tour.size())
    {
        double cheapest = std::numeric_limits<double>::infinity();
        int best_vert = -1;
        int best_pos = -1; // Entre j e j+1
        for (int i = 0; i < in_tour.size(); i++)
        {
            if (in_tour[i] == false)
            {
                for (int j = 0; j < truck_tour.size(); j++)
                {
                    int u = truck_tour[j];
                    int v = truck_tour[(j + 1) % truck_tour.size()];
                    double insertion_cost = input.truck_graph[u][i] + input.truck_graph[i][v] - input.truck_graph[u][v];
                    if (insertion_cost < cheapest)
                    {
                        cheapest = insertion_cost;
                        best_vert = i;
                        best_pos = j;
                    }
                }
            }
        }
        if (truck_time + cheapest > input.d || best_pos == -1)
        {
            break;
        }
        else
        {
            //std::cout << "Inserting " << best_vert << " at " << best_pos + 1 << std::endl;
            truck_tour.insert(truck_tour.begin() + best_pos + 1, best_vert);
            in_tour[best_vert] = true;
        }
    }

    for (auto t : truck_tour)
    {
        //std::cout << t << " ";
        if (t == 0)
            continue;
        solution.push_back(std::make_pair(t, true));
    }
    //std::cout << std::endl;

    return false;
}

std::vector<int> WarmStart::CheapestInsertionWithInitialTour(const std::vector<std::vector<double>> &dist, std::vector<int> initialTour, const std::vector<int> &remainingVertices)
{
    int n = dist.size();
    std::vector<bool> inTour(n, false);
    for (int v : initialTour)
        inTour[v] = true;

    std::vector<int> tour = initialTour; // já começa com tour inicial

    // Estruturas auxiliares
    std::vector<double> bestCost(n, 1e18);
    std::vector<int> bestPos(n, -1);
    std::priority_queue<Insertion, std::vector<Insertion>, std::greater<Insertion>> pq;

    // Inicializa os custos de inserção para os vértices fora do tour
    for (int k : remainingVertices)
    {
        if (inTour[k])
            continue;
        double minC = 1e18;
        int pos = -1;
        for (int i = 0; i < (int)tour.size() - 1; i++)
        {
            int a = tour[i], b = tour[i + 1];
            double c = dist[a][k] + dist[k][b] - dist[a][b];
            if (c < minC)
            {
                minC = c;
                pos = i + 1;
            }
        }
        bestCost[k] = minC;
        bestPos[k] = pos;
        pq.push({minC, k, pos});
    }

    int inserted = 0;
    int totalToInsert = remainingVertices.size();

    // Loop de inserção
    while (inserted < totalToInsert)
    {
        // Pega o vértice com menor custo de inserção
        Insertion current;
        do
        {
            if (pq.empty())
            {
                std::cout << "ERRO: heap vazia antes de terminar as inserções!\n";
                return tour;
            }
            current = pq.top();
            pq.pop();
        } while (inTour[current.vertex]);

        int k = current.vertex;
        int pos = current.position;

        // Insere no tour
        tour.insert(tour.begin() + pos, k);
        inTour[k] = true;
        inserted++;

        // Atualiza custos dos vértices restantes
        for (int l : remainingVertices)
        {
            if (inTour[l])
                continue;
            double minC = bestCost[l];
            int newPos = bestPos[l];

            for (int i = 0; i < (int)tour.size() - 1; i++)
            {
                int a = tour[i], b = tour[i + 1];
                double c = dist[a][l] + dist[l][b] - dist[a][b];
                if (c < minC)
                {
                    minC = c;
                    newPos = i + 1;
                }
            }

            if (minC < bestCost[l])
            {
                bestCost[l] = minC;
                bestPos[l] = newPos;
                pq.push({minC, l, newPos});
            }
        }
    }

    return tour;
}
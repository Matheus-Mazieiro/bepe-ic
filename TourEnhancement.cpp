#include "TourEnhancement.hpp"

std::vector<std::vector<std::vector<std::vector<bool>>>> TourEnhancement::Enhance(Input &input, std::vector<std::vector<bool>> truck_tour, std::vector<std::vector<std::vector<std::vector<bool>>>> drone_tour)
{
    bool improved = true;
    std::vector<std::vector<int>> edge_count(drone_tour[0][0].size(), std::vector<int>(drone_tour[0][0][0].size()));
    std::vector<std::vector<double>> time_duration(truck_tour.size(), std::vector<double>(truck_tour[0].size()));
    double total_time = 0;
    for (int i = 0; i < drone_tour.size(); i++)
    {
        for (int j = 0; j < drone_tour[i].size(); j++)
        {
            if (!truck_tour[i][j])
                continue;

            for (int m = 0; m < drone_tour[i][j].size(); m++)
            {
                for (int n = 0; n < drone_tour[i][j][m].size(); n++)
                {
                    if (!drone_tour[i][j][m][n])
                        continue;
                    edge_count[m][n]++;
                    time_duration[i][j] += input.drone_graph[m][n];
                }
            }
            total_time += time_duration[i][j];
        }
    }
    // Add vertices
    while (improved)
    {
        improved = false;

        for (int i = 0; i < drone_tour.size() && !improved; i++)
        {
            for (int j = 0; j < drone_tour[i].size() && !improved; j++)
            {
                if (truck_tour[i][j] == false)
                    continue;
                for (int m = 0; m < drone_tour[i][j].size(); m++)
                {
                    for (int n = 0; n < drone_tour[i][j][m].size(); n++)
                    {
                        if (!drone_tour[i][j][m][n] || m == n)
                            continue;
                        for (int k = 0; k < drone_tour[i][j][m].size(); k++)
                        {
                            if (drone_tour[i][j][m][k] || drone_tour[i][j][k][n] || m == k || n == k)
                                continue;
                            double delta_profit = 0;
                            double delta_time = 0;
                            if (edge_count[m][n] == 1)
                                delta_profit -= input.drone_profits_graph[m][n];
                            if (edge_count[m][k] == 0)
                                delta_profit += input.drone_profits_graph[m][k];
                            if (edge_count[k][n] == 0)
                                delta_profit += input.drone_profits_graph[k][n];
                            delta_time -= input.drone_graph[m][n];
                            delta_time += input.drone_graph[m][k];
                            delta_time += input.drone_graph[k][n];
                            if (delta_profit > 0 && time_duration[i][j] + delta_time <= input.t_max)
                            {
                                time_duration[i][j] += delta_time;
                                edge_count[m][n]--;
                                edge_count[m][k]++;
                                edge_count[k][n]++;
                                drone_tour[i][j][m][n] = false;
                                drone_tour[i][j][m][k] = true;
                                drone_tour[i][j][k][n] = true;
                                improved = true;
                            }
                        }
                    }
                }
            }
        }
    }

    double total_profit = 0;
    std::vector<bool> h(drone_tour.size());
    for (int m = 0; m < edge_count.size(); m++)
    {
        for (int n = 0; n < edge_count[m].size(); n++)
        {
            if (edge_count[m][n] > 0)
            {
                total_profit += input.drone_profits_graph[m][n];
                h[m] = true;
            }
        }
        total_profit += h[m] * input.drones_nodes_profits[m];
    }

    std::cout << "Objective changed to " << total_profit << std::endl;
    return drone_tour;
}
/*
#include <algorithm>
double TourEnhancement::Enhance(Input &input, std::vector<std::pair<int, bool>> &tour, bool add_vertex)
{

    // Estruturas auxiliares
    std::vector<std::vector<int>> edge_count(input.num_nodes, std::vector<int>(input.num_nodes));
    std::vector<int> node_count(input.num_nodes, 0);
    std::vector<double> time_duration;
    std::vector<std::pair<double, int>> truck_durations;
    double truck_sum = 0;
    double total_time = 0;

    int start = 0;
    double current_fly_duration = 0;
    do
    {
        edge_count = std::vector<std::vector<int>>(input.num_nodes, std::vector<int>(input.num_nodes));
        node_count = std::vector<int>(input.num_nodes, 0);
        time_duration = std::vector<double>(0);
        truck_durations = std::vector<std::pair<double, int>>(0);
        truck_sum = 0;
        total_time = 0;

        for (int i = 0; i < tour.size(); i++)
        {
            std::pair<int, bool> u = tour[i];
            std::pair<int, bool> v = tour[(i + 1) % tour.size()];
            edge_count[u.first][v.first]++;

            current_fly_duration += input.drone_graph[u.first][v.first];
            if (v.second)
            {
                time_duration.push_back(current_fly_duration);
                total_time += current_fly_duration;
                current_fly_duration = 0;
                truck_durations.push_back({input.truck_graph[start][v.first], v.first});
                truck_sum += input.truck_graph[start][v.first];
                start = v.first;
            }
            node_count[u.first]++;
        }

        if (truck_sum > input.d)
        {
            std::sort(truck_durations.begin(), truck_durations.end(), std::greater<>());
            int to_remove = truck_durations[0].second != 0 ? truck_durations[0].second : truck_durations[1].second;
            tour.erase(tour.begin() + truck_durations[0].second);
        }
    } while (truck_sum > input.d);

    for (auto p : tour)
    {
        if (p.second)
            std::cout << "*";
        std::cout << p.first << " ";
    }
    std::cout << std::endl;
    std::cout << "{";
    for (auto t : time_duration)
    {
        std::cout << t << ", ";
        // total_time += t;
    }
    std::cout << "}" << std::endl
              << total_time << "/" << input.d << std::endl;

    // Loop principal da busca --- Adding vertices
    bool improved = add_vertex;
    while (improved)
    {
        improved = false;

        int time_index = 0;
        for (int i = 0; i < tour.size() && !improved; i++)
        {
            std::pair<int, bool> u = tour[i];
            std::pair<int, bool> v = tour[(i + 1) % tour.size()];

            for (int k = 0; k < input.num_nodes && !improved; k++)
            {
                double delta_profit = 0;
                double delta_time = 0;

                if (edge_count[u.first][v.first] == 1)
                    delta_profit -= input.drone_profits_graph[u.first][v.first];
                if (edge_count[u.first][k] == 0)
                    delta_profit += input.drone_profits_graph[u.first][k];
                if (edge_count[k][v.first] == 0)
                    delta_profit += input.drone_profits_graph[k][v.first];
                if (node_count[k] == 0)
                    delta_profit += input.drones_nodes_profits[k];
                delta_time -= input.drone_graph[u.first][v.first];
                delta_time += input.drone_graph[u.first][k];
                delta_time += input.drone_graph[k][v.first];

                if (delta_profit > 0 &&
                    total_time + delta_time <= input.d &&
                    time_duration[time_index] + delta_time <= input.t_max)
                {
                    total_time += delta_time;
                    time_duration[time_index] += delta_time;
                    edge_count[u.first][v.first]--;
                    edge_count[u.first][k]++;
                    edge_count[k][v.first]++;
                    node_count[k]++;
                    tour.insert(tour.begin() + i + 1, {k, false});
                    improved = true;

                    std::cout << "Added " << k << std::endl;
                }
            }

            time_index += v.second;
        }
    }

    std::cout << "After adding: ";
    for (auto p : tour)
    {
        if (p.second)
            std::cout << "*";
        std::cout << p.first << " ";
    }
    std::cout << std::endl;

    std::cout << "{";
    for (auto t : time_duration)
    {
        std::cout << t << ", ";
        // total_time += t;
    }
    std::cout << "} / " << input.t_max << std::endl
              << total_time << "/" << input.d << std::endl;

    // Fixing T_max problems
    for (int i = 0; i < time_duration.size(); i++)
    {
        int count = 0;
        int j;
        for (j = 1; count != i && j < tour.size(); j++)
            count += tour[j].second;
        while (time_duration[i] > input.t_max)
        {
            // Remover aresta para corrigir T_max

            double best_delta_profit = 0;
            double best_delta_time = 0;
            int best_removal = -1;

            int prev_node = (j) % tour.size();
            int node_to_remove = (j + 1) % tour.size();
            int next_node = (j + 2) % tour.size();
            if (tour[next_node].first == tour[prev_node].first)
                next_node = (j + 3) % tour.size();
            for (int k = j + 1; !tour[next_node].second; k++)
            {
                prev_node = (k - 1) % tour.size();
                node_to_remove = (k) % tour.size();
                next_node = (k + 1) % tour.size();
                if (tour[next_node].first == tour[prev_node].first)
                    continue;

                int delta_profit = 0;
                int delta_time = 0;
                delta_time += input.drone_graph[tour[prev_node].first][tour[next_node].first];
                if (edge_count[tour[prev_node].first][tour[next_node].first] == 0)
                    delta_profit += input.drone_profits_graph[tour[prev_node].first][tour[next_node].first];
                delta_time -= input.drone_graph[tour[prev_node].first][tour[node_to_remove].first];
                if (edge_count[tour[prev_node].first][tour[node_to_remove].first] == 1)
                    delta_profit -= input.drone_profits_graph[tour[prev_node].first][tour[node_to_remove].first];
                delta_time -= input.drone_graph[tour[node_to_remove].first][tour[next_node].first];
                if (edge_count[tour[node_to_remove].first][tour[next_node].first] == 1)
                    delta_profit -= input.drone_profits_graph[tour[node_to_remove].first][tour[next_node].first];
                if (node_count[tour[node_to_remove].first] == 1)
                    delta_profit -= input.drones_nodes_profits[tour[node_to_remove].first];

                if (time_duration[i] + best_delta_time > input.t_max)
                {
                    if (delta_time < best_delta_time)
                    {
                        best_delta_time = delta_time;
                        best_delta_profit = delta_profit;
                        best_removal = node_to_remove;
                    }
                }
                else
                {
                    if (time_duration[i] + delta_time <= input.t_max)
                    {
                        if (delta_profit > best_delta_profit)
                        {
                            best_delta_time = delta_time;
                            best_delta_profit = delta_profit;
                            best_removal = node_to_remove;
                        }
                    }
                }
            }
            if (best_removal == -1)
            {
                std::cout << "Error on removing vertices" << std::endl;
                break;
            }

            // Remove aresta
            time_duration[i] += best_delta_time;
            edge_count[tour[best_removal - 1].first][tour[best_removal].first]--;
            edge_count[tour[best_removal].first][tour[best_removal + 1].first]--;
            edge_count[tour[best_removal - 1].first][tour[best_removal + 1].first]++;
            node_count[tour[best_removal].first]--;
            std::cout << "Removed " << tour[best_removal].first << " Between " << tour[best_removal - 1].first << " and " << tour[best_removal + 1].first << std::endl;
            tour.erase(tour.begin() + best_removal);
            for (auto p : tour)
            {
                if (p.second)
                    std::cout << "*";
                std::cout << p.first << " ";
            }
            std::cout << std::endl;
        }
    }

    std::cout << "After fixing T_max:";
    for (auto p : tour)
    {
        if (p.second)
            std::cout << "*";
        std::cout << p.first << " ";
    }
    std::cout << std::endl;

    total_time = 0;
    std::cout << "{";
    for (int i = 0; i < time_duration.size(); i++)
    {
        double t = std::max(time_duration[i], truck_durations[i].first);
        std::cout << t << ", ";
        total_time += t;
    }
    std::cout << "} / " << input.t_max << std::endl
              << total_time << "/" << input.d << std::endl;

    // Fixing D problems
    while (total_time > input.d)
    {
        double best_delta_profit = 0;
        double best_delta_time = 0;
        int best_removal = -1;

        int prev_node = (0) % tour.size();
        int node_to_remove = (1) % tour.size();
        int next_node = (2) % tour.size();
        if (tour[next_node].first == tour[prev_node].first)
            next_node = (3) % tour.size();
        int part_count = 0;
        for (int k = 1; k < tour.size(); k++)
        {
            prev_node = k - 1;
            node_to_remove = k;
            next_node = (k + 1) % tour.size();
            if (tour[next_node].first == tour[prev_node].first)
                next_node = (k + 2) % tour.size();

            if (tour[node_to_remove].second)
            {
                part_count++;
                continue;
            }
            if (time_duration[part_count] <= truck_durations[part_count].first)
                continue;

            int delta_profit = 0;
            int delta_time = 0;
            delta_time += input.drone_graph[tour[prev_node].first][tour[next_node].first];
            if (edge_count[tour[prev_node].first][tour[next_node].first] == 0)
                delta_profit += input.drone_profits_graph[tour[prev_node].first][tour[next_node].first];
            delta_time -= input.drone_graph[tour[prev_node].first][tour[node_to_remove].first];
            if (edge_count[tour[prev_node].first][tour[node_to_remove].first] == 1)
                delta_profit -= input.drone_profits_graph[tour[prev_node].first][tour[node_to_remove].first];
            delta_time -= input.drone_graph[tour[node_to_remove].first][tour[next_node].first];
            if (edge_count[tour[node_to_remove].first][tour[next_node].first] == 1)
                delta_profit -= input.drone_profits_graph[tour[node_to_remove].first][tour[next_node].first];
            if (node_count[tour[node_to_remove].first] == 1)
                delta_profit -= input.drones_nodes_profits[tour[node_to_remove].first];

            if (total_time + best_delta_time > input.d)
            {
                if (delta_time < best_delta_time)
                {
                    best_delta_time = delta_time;
                    best_delta_profit = delta_profit;
                    best_removal = node_to_remove;
                }
            }
            else
            {
                if (total_time + delta_time <= input.d)
                {
                    if (delta_profit > best_delta_profit)
                    {
                        best_delta_time = delta_time;
                        best_delta_profit = delta_profit;
                        best_removal = node_to_remove;
                    }
                }
            }
        }

        std::cout << "best_removal: " << best_removal << std::endl;
        if (best_removal == -1)
        {
            std::cout << "Error on removing vertices on tour: ";
            for (auto p : tour)
            {
                if (p.second)
                    std::cout << "*";
                std::cout << p.first << " ";
            }
            std::cout << std::endl;
            break;
        }

        // Remove aresta
        std::cout << "best_delta_time: " << best_delta_time << std::endl;
        total_time += best_delta_time;
        edge_count[tour[prev_node].first][tour[best_removal].first]--;
        edge_count[tour[best_removal].first][tour[next_node].first]--;
        edge_count[tour[prev_node].first][tour[next_node].first]++;
        node_count[tour[best_removal].first]--;
        tour.erase(tour.begin() + best_removal);
    }

    std::cout << "Final: ";
    for (auto p : tour)
    {
        if (p.second)
            std::cout << "*";
        std::cout << p.first << " ";
    }
    std::cout << std::endl;
    std::cout << total_time << "/" << input.d << std::endl;

    { // Recalculando profit
        double total_profit = 0;
        std::vector<bool> h(edge_count.size());
        for (int m = 0; m < edge_count.size(); m++)
        {
            for (int n = 0; n < edge_count[m].size(); n++)
            {
                if (edge_count[m][n] > 0)
                {
                    total_profit += input.drone_profits_graph[m][n];
                    h[m] = true;
                }
            }
            total_profit += h[m] * input.drones_nodes_profits[m];
        }
        // std::cout << "Objective changed to " << total_profit << std::endl;
        return total_profit;
    }
}
    */
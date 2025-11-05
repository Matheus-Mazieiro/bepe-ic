#include "TourEnhancement.hpp"
#include <algorithm>

#define DEBUG 0

std::vector<std::pair<int, bool>> TourEnhancement::Enhance(Input &input, std::vector<std::pair<int, bool>> tour, bool add_vertex)
{
    if (DEBUG)
    {
        std::cout << "Starting tour: ";
        for (auto p : tour)
        {
            if (p.second)
                std::cout << "*";
            std::cout << p.first << " ";
        }
        std::cout << std::endl;
    }

    // Remove truck visiting that is bigger than d
    {
        double truck_total = 0;
        int last_truck = 0;
        std::vector<int> truck_tour;
        for (int i = 0; i < tour.size(); i++)
        {
            int next = (i + 1) % tour.size();
            if (!tour.at(next).second)
                continue;

            truck_total += input.truck_graph.at(last_truck).at(tour.at(next).first);
            truck_tour.push_back(last_truck);
            last_truck = tour.at(next).first;
        }
        if (DEBUG)
            std::cout << "Truck_total = " << truck_total << "/" << input.d << std::endl;

        while (truck_total > input.d)
        {
            if (DEBUG)
                std::cout << "Need to remove" << std::endl;
            double best_delta_time = 0;
            int best_to_remove = 1;
            for (int i = 1; i < truck_tour.size(); i++)
            {
                int prev = i - 1;
                int cur = i;
                int next = (i + 1) % truck_tour.size();
                double delta_time = input.truck_graph.at(truck_tour.at(prev)).at(truck_tour.at(next));
                delta_time -= input.truck_graph.at(truck_tour.at(prev)).at(truck_tour.at(cur));
                delta_time -= input.truck_graph.at(truck_tour.at(cur)).at(truck_tour.at(next));
                if (delta_time < best_delta_time)
                {
                    best_delta_time = delta_time;
                    best_to_remove = cur;
                }
            }
            truck_total += best_delta_time;
            if (DEBUG)
                std::cout << "Removing " << truck_tour.at(best_to_remove) << std::endl;
            auto it = std::find_if(tour.begin(), tour.end(),
                                   [&](const std::pair<int, bool> &p)
                                   {
                                       return p.first == truck_tour.at(best_to_remove) && p.second == true;
                                   });

            if (it != tour.end())
            {
                tour.erase(it);
            }
            truck_tour.erase(truck_tour.begin() + best_to_remove);
        }

        if (DEBUG)
        {
            std::cout << "After removing truck nodes only: ";
            for (auto p : tour)
            {
                if (p.second)
                    std::cout << "*";
                std::cout << p.first << " ";
            }
            std::cout << std::endl;
            std::cout << "Truck_total = " << truck_total << std::endl
                      << std::endl;
        }
    }

    // Estruturas auxiliares
    std::vector<std::vector<int>> edge_count(input.num_nodes, std::vector<int>(input.num_nodes));
    std::vector<int> node_count(input.num_nodes, 0);
    std::vector<double> flight_duration;
    std::vector<int> truck_indices(1, 0);
    double current_fly = 0;
    int last_truck = 0;
    for (int i = 0; i < tour.size(); i++)
    {
        auto cur = tour.at(i);
        auto next = tour.at((i + 1) % tour.size());
        node_count.at(cur.first)++;
        edge_count.at(cur.first).at(next.first)++;
        current_fly += input.drone_graph.at(cur.first).at(next.first);
        if (next.second)
        {
            truck_indices.push_back(i + 1);
            flight_duration.push_back(current_fly);
            current_fly = 0;
        }
    }
    truck_indices.erase(truck_indices.end() - 1);

    // Remove drone visiting that is bigger than t_max
    {
        if (DEBUG)
        {
            std::cout << "Duracao dos voos: {";
            for (int i = 0; i < flight_duration.size(); i++)
            {
                std::cout << flight_duration.at(i);
                if (i < flight_duration.size() - 1)
                    std::cout << ", ";
            }
            std::cout << "} / " << input.t_max << std::endl;
        }

        for (int i = 0; i < flight_duration.size(); i++)
        {
            while (flight_duration.at(i) > input.t_max)
            {
                double best_delta_profit = 0;
                double best_delta_time = 0;
                int best_removal = -1;

                int j = truck_indices.at(i);

                int prev_node = (j) % tour.size();
                int node_to_remove = (j + 1) % tour.size();
                int next_node = (j + 2) % tour.size();
                if (tour.at(next_node).first == tour.at(prev_node).first)
                    next_node = (j + 3) % tour.size();

                for (int k = j + 1; !tour.at(next_node).second; k++)
                {
                    prev_node = (k - 1) % tour.size();
                    node_to_remove = (k) % tour.size();
                    next_node = (k + 1) % tour.size();
                    if (tour.at(next_node).first == tour.at(prev_node).first)
                        continue;

                    int delta_profit = 0;
                    int delta_time = 0;
                    delta_time += input.drone_graph.at(tour.at(prev_node).first).at(tour.at(next_node).first);
                    if (edge_count.at(tour.at(prev_node).first).at(tour.at(next_node).first) == 0)
                        delta_profit += input.drone_profits_graph.at(tour.at(prev_node).first).at(tour.at(next_node).first);
                    delta_time -= input.drone_graph.at(tour.at(prev_node).first).at(tour.at(node_to_remove).first);
                    if (edge_count.at(tour.at(prev_node).first).at(tour.at(node_to_remove).first) == 1)
                        delta_profit -= input.drone_profits_graph.at(tour.at(prev_node).first).at(tour.at(node_to_remove).first);
                    delta_time -= input.drone_graph.at(tour.at(node_to_remove).first).at(tour.at(next_node).first);
                    if (edge_count.at(tour.at(node_to_remove).first).at(tour.at(next_node).first) == 1)
                        delta_profit -= input.drone_profits_graph.at(tour.at(node_to_remove).first).at(tour.at(next_node).first);
                    if (node_count.at(tour.at(node_to_remove).first) == 1)
                        delta_profit -= input.drones_nodes_profits.at(tour.at(node_to_remove).first);

                    if (flight_duration.at(i) + best_delta_time > input.t_max)
                    {
                        if (delta_time <= best_delta_time)
                        {
                            best_delta_time = delta_time;
                            best_delta_profit = delta_profit;
                            best_removal = node_to_remove;
                        }
                    }
                    else
                    {
                        if (flight_duration.at(i) + delta_time <= input.t_max)
                        {
                            if (delta_profit >= best_delta_profit)
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
                //                std::cout << "Removendo " << tour.at(best_removal).first << std::endl;
                flight_duration.at(i) += best_delta_time;
                edge_count.at(tour.at((best_removal - 1 + tour.size()) % tour.size()).first).at(tour.at(best_removal).first)--;
                edge_count.at(tour.at(best_removal).first).at(tour.at((best_removal + 1) % tour.size()).first)--;
                edge_count.at(tour.at((best_removal - 1 + tour.size()) % tour.size()).first).at(tour.at((best_removal + 1) % tour.size()).first)++;
                node_count.at(tour.at(best_removal).first)--;
                // if (DEBUG)
                //     std::cout << "Removed " << tour.at(best_removal).first << " Between " << tour.at(best_removal - 1).first << " and " << tour.at(best_removal + 1).first << std::endl;
                tour.erase(tour.begin() + best_removal);
            }
        }
        if (DEBUG)
        {
            std::cout << "After fixing T_max:";
            for (auto p : tour)
            {
                if (p.second)
                    std::cout << "*";
                std::cout << p.first << " ";
            }
            std::cout << std::endl;
            std::cout << "Duracao dos voos: {";
            for (int i = 0; i < flight_duration.size(); i++)
            {
                std::cout << flight_duration.at(i);
                if (i < flight_duration.size() - 1)
                    std::cout << ", ";
            }
            std::cout << "} / " << input.t_max << std::endl
                      << std::endl;
        }
    }
    // Remove drone visiting that is bigger than d
    double total_time = 0;
    {
        int last_truck = 0;
        double current_drone_time = 0;
        for (int i = 0; i < tour.size(); i++)
        {
            int cur = i;
            int next = (i + 1) % tour.size();

            current_drone_time += input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
            if (tour.at(next).second)
            {
                total_time += std::max(current_drone_time, input.truck_graph.at(last_truck).at(tour.at(next).first));
                last_truck = tour.at(next).first;
            }
        }
        // Recalcula total_time
        total_time = 0;
        last_truck = 0;
        current_drone_time = 0;
        for (int i = 0; i < tour.size(); i++)
        {
            int cur = i;
            int next = (i + 1) % tour.size();

            current_drone_time += input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
            if (tour.at(next).second)
            {
                total_time += std::max(current_drone_time, input.truck_graph.at(last_truck).at(tour.at(next).first));
                last_truck = tour.at(next).first;
                current_drone_time = 0;
            }
        }

        while (total_time > input.d)
        {
            double best_delta_time = 0;
            double best_delta_profit = 0;
            int best_removal = -1;

            double new_total_time = total_time - best_delta_time; // será atualizado no laço
            int sync = 0;
            for (int i = 1; i + 1 < tour.size(); i++)
            {
                if (tour.at(i).second)
                {
                    sync++;
                    continue;
                }

                int prev = i - 1;
                int next = i + 1;

                if (tour.at(prev).first == tour.at(next).first)
                    continue;

                double delta_time = 0;
                double delta_profit = 0;

                // Tempo
                delta_time += input.drone_graph.at(tour.at(prev).first).at(tour.at(next).first);
                delta_time -= input.drone_graph.at(tour.at(prev).first).at(tour.at(i).first);
                delta_time -= input.drone_graph.at(tour.at(i).first).at(tour.at(next).first);

                // Lucro
                if (edge_count.at(tour.at(prev).first).at(tour.at(next).first) == 0)
                    delta_profit += input.drone_profits_graph.at(tour.at(prev).first).at(tour.at(next).first);
                if (edge_count.at(tour.at(prev).first).at(tour.at(i).first) == 1)
                    delta_profit -= input.drone_profits_graph.at(tour.at(prev).first).at(tour.at(i).first);
                if (edge_count.at(tour.at(i).first).at(tour.at(next).first) == 1)
                    delta_profit -= input.drone_profits_graph.at(tour.at(i).first).at(tour.at(next).first);
                if (node_count.at(tour.at(i).first) == 1)
                    delta_profit -= input.drones_nodes_profits.at(tour.at(i).first);

                // new_total está sendo calculado errado, ele dever ser sum{max{caminhao[i][j], caminhdo_do_drone(i, j)}}
                // Recalcula total_time hipotético como se o nó tour[i] fosse removido
                std::vector<std::pair<int, bool>> temp_tour = tour;
                temp_tour.erase(temp_tour.begin() + i);

                double hypothetical_total_time = 0;
                int hypothetical_last_truck = 0;
                double hypothetical_drone_time = 0;
                for (int h = 0; h < temp_tour.size(); h++)
                {
                    int cur = h;
                    int next = (h + 1) % temp_tour.size();

                    hypothetical_drone_time += input.drone_graph.at(temp_tour.at(cur).first).at(temp_tour.at(next).first);
                    if (temp_tour.at(next).second)
                    {
                        hypothetical_total_time += std::max(hypothetical_drone_time, input.truck_graph.at(hypothetical_last_truck).at(temp_tour.at(next).first));
                        hypothetical_last_truck = temp_tour.at(next).first;
                        hypothetical_drone_time = 0;
                    }
                }

                if (best_removal == -1)
                {
                    best_delta_time = delta_time;
                    best_delta_profit = delta_profit;
                    best_removal = i;
                }

                if (hypothetical_total_time > input.d)
                {
                    // Ainda insuficiente: prioriza maior delta_time
                    if (best_removal == -1 || delta_time > best_delta_time)
                    {
                        best_delta_time = delta_time;
                        best_delta_profit = delta_profit;
                        best_removal = i;
                    }
                }
                else
                {
                    // Já suficiente: prioriza menor perda de lucro
                    if (best_removal == -1 || delta_profit > best_delta_profit)
                    {
                        best_delta_time = delta_time;
                        best_delta_profit = delta_profit;
                        best_removal = i;
                    }
                }
            }

            if (best_removal == -1)
            {
                if (DEBUG)
                    std::cout << "Não foi possível reduzir mais o tempo total sem remover caminhão" << std::endl;
                break;
            }

            // std::cout << total_time << " / " << input.d << std::endl;
            if (DEBUG)
                std::cout << "Removendo " << tour.at(best_removal).first << " (apenas drone)" << std::endl;

            // Atualiza estruturas auxiliares
            edge_count.at(tour.at(best_removal - 1).first).at(tour.at(best_removal).first)--;
            edge_count.at(tour.at(best_removal).first).at(tour.at(best_removal + 1).first)--;
            edge_count.at(tour.at(best_removal - 1).first).at(tour.at(best_removal + 1).first)++;
            node_count.at(tour.at(best_removal).first)--;

            tour.erase(tour.begin() + best_removal);

            // Recalcula total_time
            total_time = 0;
            last_truck = 0;
            double current_drone_time = 0;
            for (int i = 0; i < tour.size(); i++)
            {
                int cur = i;
                int next = (i + 1) % tour.size();

                current_drone_time += input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
                if (tour.at(next).second)
                {
                    total_time += std::max(current_drone_time, input.truck_graph.at(last_truck).at(tour.at(next).first));
                    last_truck = tour.at(next).first;
                    current_drone_time = 0;
                }
            }
        }
        if (DEBUG)
        {
            std::cout << "After fixing D:";
            for (auto p : tour)
            {
                if (p.second)
                    std::cout << "*";
                std::cout << p.first << " ";
            }
            std::cout << total_time << " / " << input.d << std::endl;
        }
    }

    // Add vertex
    {

        std::vector<double> truck_times;
        double last_truck = 0;
        for (int i = 1; i < tour.size(); i++)
            if (tour.at(i).second)
            {
                truck_times.push_back(input.truck_graph.at(last_truck).at(tour.at(i).first));
                last_truck = tour.at(i).first;
            }
        truck_times.push_back(input.truck_graph.at(last_truck).at(0));

        if (DEBUG)
        {
            std::cout << "Take a look at FLYIGHT_DURATION: ";
            for (auto a : flight_duration)
                std::cout << a << " ";
            std::cout << std::endl;
            std::cout << "Take a look at TRUCK_TIMES: ";
            for (auto a : truck_times)
                std::cout << a << " ";
            std::cout << std::endl;
        }

        bool improved = add_vertex;
        while (improved)
        {
            improved = false;
            double best_delta_time = 0;
            double flying_delta_time = 0;
            double best_delta_profit = 0;
            int node_to_insert = 0;
            int position_to_insert = -1;
            int sync = 0;

            for (int i = 0; i < input.num_nodes; i++) // i == vertice que sera inserido
            {
                int sync_count = 0;
                for (int j = 0; j < tour.size(); j++) // (j, j+1) == aonde o vertice sera inserido
                {
                    int cur = j;
                    int next = (j + 1) % tour.size();

                    if (tour.at(cur).first == i || i == tour.at(next).first)
                        continue;

                    // sync_count = (sync_count + tour.at(next).second) % flight_duration.size();
                    size_t fd_sz = flight_duration.size(); // use size_t
                    size_t sync_count = 0;                 // use size_t, não int
                    sync_count = (sync_count + static_cast<size_t>(tour.at(next).second)) % fd_sz;

                    double delta_time = input.drone_graph.at(tour.at(cur).first).at(i);
                    delta_time += input.drone_graph.at(i).at(tour.at(next).first);
                    delta_time -= input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
                    double flying_time = delta_time;
                    if (flight_duration.at(sync_count) + flying_time > input.t_max)
                        continue;
                    if (flight_duration.at(sync_count) > truck_times.at(sync_count))
                        delta_time = flying_time;
                    else
                        delta_time = std::max(0.0, flight_duration.at(sync_count) + flying_time - truck_times.at(sync_count));
                    if (total_time + delta_time > input.d)
                        continue;

                    double delta_profit = 0;
                    if (edge_count.at(tour.at(cur).first).at(i) == 0)
                        delta_profit += input.drone_profits_graph.at(tour.at(cur).first).at(i);
                    if (edge_count.at(i).at(tour.at(next).first) == 0)
                        delta_profit += input.drone_profits_graph.at(i).at(tour.at(next).first);
                    if (edge_count.at(tour.at(cur).first).at(tour.at(next).first) == 1)
                        delta_profit -= input.drone_profits_graph.at(tour.at(cur).first).at(tour.at(next).first);
                    if (node_count.at(i) == 0)
                        delta_profit += input.drones_nodes_profits.at(i);

                    if (delta_profit > best_delta_profit)
                    {
                        best_delta_profit = delta_profit;
                        node_to_insert = i;
                        if (next == 0)
                            next = tour.size();
                        position_to_insert = next;
                        sync = sync_count;
                        best_delta_time = delta_time;
                        flying_delta_time = flying_time;
                    }
                }
            }

            if (position_to_insert >= 0)
            {
                if (DEBUG)
                {
                    for (auto p : tour)
                    {
                        if (p.second)
                            std::cout << "*";
                        std::cout << p.first << " ";
                    }
                    std::cout << std::endl;
                }
                tour.insert(tour.begin() + position_to_insert, {node_to_insert, false});
                edge_count.at(tour.at(position_to_insert - 1).first).at(tour.at(position_to_insert).first)++;
                edge_count.at(tour.at(position_to_insert).first).at(tour.at((position_to_insert + 1) % tour.size()).first)++;
                edge_count.at(tour.at(position_to_insert - 1).first).at(tour.at((position_to_insert + 1) % tour.size()).first)--;
                node_count.at(tour.at(position_to_insert).first)++;
                flight_duration.at(sync) += flying_delta_time;
                total_time += best_delta_time;
                if (DEBUG)
                    std::cout << "Trying to insert " << node_to_insert << " into position " << position_to_insert << " (Total time = " << total_time << "/" << input.d << ")" << std::endl;
                improved = true;

                total_time = 0;
                last_truck = 0;
                double current_drone_time = 0;
                for (int i = 0; i < tour.size(); i++)
                {
                    int cur = i;
                    int next = (i + 1) % tour.size();

                    current_drone_time += input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
                    if (tour.at(next).second)
                    {
                        total_time += std::max(current_drone_time, input.truck_graph.at(last_truck).at(tour.at(next).first));
                        last_truck = tour.at(next).first;
                        current_drone_time = 0;
                    }
                }
                if (DEBUG)
                    std::cout << "Total time recalculado: " << total_time << std::endl;
                if (total_time > input.d)
                {
                    tour.erase(tour.begin() + position_to_insert);
                    break;
                }
            }
        }
    }

    total_time = 0;
    last_truck = 0;
    double current_drone_time = 0;
    for (int i = 0; i < tour.size(); i++)
    {
        int cur = i;
        int next = (i + 1) % tour.size();

        current_drone_time += input.drone_graph.at(tour.at(cur).first).at(tour.at(next).first);
        if (tour.at(next).second)
        {
            total_time += std::max(current_drone_time, input.truck_graph.at(last_truck).at(tour.at(next).first));
            last_truck = tour.at(next).first;
            current_drone_time = 0;
        }
    }
    if (DEBUG)
    {
        std::cout << "Total time recalculado: " << total_time << std::endl;
        std::cout << "No final ficou: ";
        for (auto p : tour)
        {
            if (p.second)
                std::cout << "*";
            std::cout << p.first << " ";
        }
        std::cout << std::endl;
    }
    return tour;
}
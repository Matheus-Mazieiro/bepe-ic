#include "Individual.hpp"

Individual::Individual(const Individual &other) : input(other.input), settings(other.settings)
{
    this->current = other.current;
    this->pocket = other.pocket;
}

Individual::Individual(Input &input, Settings &settings) : input(input), settings(settings)
{
    for (int i = 0; i < input.num_nodes; i++)
    {
        current.tour.push_back({i, (i < input.num_truck_nodes + 1)});
        pocket.tour.push_back({i, (i < input.num_truck_nodes + 1)});
    }

    std::shuffle(current.tour.begin() + 1, current.tour.end(), settings.rng);
    std::shuffle(pocket.tour.begin() + 1, pocket.tour.end(), settings.rng);

    current.tour = TourEnhancement::Enhance(input, current.tour, settings.local_search == "LS");
    pocket.tour = TourEnhancement::Enhance(input, pocket.tour, settings.local_search == "LS");

    current.profit = EvaluateTour(input, current.tour, current.time, current.objective);
    pocket.profit = EvaluateTour(input, pocket.tour, current.time, current.objective);
}

void Individual::PrintIndividual(Solution &sol)
{
    for (auto i : sol.tour)
    {
        if (i.second)
            std::cout << "*";
        std::cout << i.first << " ";
    }
    std::cout << " --> " << sol.profit << std::endl;
}

double Individual::EvaluateTour(Input &input, std::vector<std::pair<int, bool>> &tour, double &total_time, double &obj)
{
    if (tour.size() < 2)
    {
        return 0.0;
    }

    std::vector<std::vector<int>> edge_count(input.drone_graph.size(), std::vector<int>(input.drone_graph.size(), 0));
    std::vector<int> node_count(input.num_nodes, 0);

    double total_profit = 0.0;
    total_time = 0;
    obj = 0;

    // --- PENALIDADE POR REVISITA DE PONTO DE SINCRONIA ---
    std::map<int, int> sync_point_counts;
    for (const auto &node_pair : tour)
    {
        if (node_pair.second)
        {
            sync_point_counts[node_pair.first]++;
        }
    }

    double total_revisit_penalty = 0.0;
    double revisit_penalty = input.t_max * input.t_max;
    for (const auto &pair : sync_point_counts)
    {
        int node = pair.first;
        int count = pair.second;
        int allowed_visits = 1;

        if (count > allowed_visits)
        {
            int extra_visits = count - allowed_visits;
            double penalty = extra_visits * revisit_penalty;
            total_revisit_penalty += penalty;
        }
    }
    total_profit -= total_revisit_penalty;

    // --- CÁLCULO DE LUCRO E PENALIDADE POR TEMPO (t_max) ---
    double current_drone_segment_time = 0.0;
    for (size_t i = 0; i < tour.size(); ++i)
    {
        int u = tour.at(i).first;
        size_t next_index = (i + 1) % tour.size();
        int v = tour.at(next_index).first;

        edge_count.at(u).at(v)++;
        node_count.at(u)++;
        current_drone_segment_time += input.drone_graph.at(u).at(v);

        if (tour.at(next_index).second)
        {
            if (current_drone_segment_time > input.t_max)
            {
                double penalty_cost = current_drone_segment_time;

                total_profit -= (penalty_cost - input.t_max);
            }
            total_time += current_drone_segment_time;
            current_drone_segment_time = 0.0;
        }
    }

    // Penalidade por tempo total D
    total_profit -= std::max(total_time - input.d, 0.0);

    for (size_t i = 0; i < edge_count.size(); i++)
    {
        if (node_count.at(i) > 0)
        {
            obj += input.drones_nodes_profits.at(i);
            total_profit += input.drones_nodes_profits.at(i);
        }
        for (size_t j = 0; j < edge_count.at(i).size(); j++)
        {
            if (edge_count.at(i).at(j) > 0)
            {
                obj += input.drone_profits_graph.at(i).at(j);
                total_profit += input.drone_profits_graph.at(i).at(j);
            }
        }
    }

    return std::max(0.0, total_profit);
}
void Individual::Optimize(bool enhance)
{
    std::vector<std::pair<int, bool>> new_tour;
    new_tour.push_back(current.tour[0]);
    for (int i = 1; i < current.tour.size(); i++)
    {
        if (current.tour[i].first == new_tour.back().first)
        {
            current.tour[i].second = new_tour.back().second || current.tour[i].second;
            continue;
        }

        new_tour.push_back(current.tour[i]);
    }
    current.tour = new_tour;

    current.tour = TourEnhancement::Enhance(input, current.tour, settings.local_search == "LS");
    current.profit = EvaluateTour(input, current.tour, current.time, current.objective);
}

void Individual::Mutate()
{
    double mutation_rate = 0.1;
    double rand_val = (double)settings.rng() / settings.rng.max();

    if (rand_val < mutation_rate && current.tour.size() >= 2)
    {
        // Troca dois nós aleatórios (exceto o primeiro, se for fixo)
        int idx1 = 1 + (settings.rng() % (current.tour.size() - 1));
        int idx2 = 1 + (settings.rng() % (current.tour.size() - 1));

        if (current.tour[idx1].first < input.num_truck_nodes + 1)
            current.tour[idx1].second |= (((double)settings.rng() / settings.rng.max()) >= 0.5);

        if (current.tour[idx2].first < input.num_truck_nodes + 1)
            current.tour[idx2].second |= (((double)settings.rng() / settings.rng.max()) >= 0.5);

        current.tour.insert(current.tour.begin() + 1 + (settings.rng() % (current.tour.size() - 1)), {input.num_nodes * settings.rng() / settings.rng.max(), false});

        std::swap(current.tour[idx1], current.tour[idx2]);
    }
}

void Individual::StructureIndividual()
{
    if (current.profit > pocket.profit)
        std::swap(current, pocket);
}

std::vector<std::pair<int, bool>> Individual::MaximizeProfitTour(Input &input, Settings &settings)
{
    int n = input.num_nodes;
    std::vector<int> C(n);
    std::vector<double> D(n, INT_MAX);
    std::vector<int> T(0);
    int s = 0; // Nó inicial
    for (int j = 0; j < n; j++)
        C[j] = j;

    T.push_back(s);
    C.erase(find(C.begin(), C.end(), s));
    int i = s;
    while (C.size() > 0)
    {
        for (int j = 0; j < C.size(); j++)
        {
            if (-input.drone_profits_graph[i][C.at(j)] < D.at(C.at(j)))
                D.at(C.at(j)) = -input.drone_profits_graph[i][C.at(j)];
        }
        int c = 0;
        for (int j = 1; j < C.size(); j++)
        {
            if (D.at(C.at(j)) < D.at(C.at(c)))
                c = j;
        }

        double d = INT_MAX;
        int t = -1;
        for (int tl = 0; tl < T.size(); tl++)
        {
            double dl = -(-input.drone_profits_graph[T.at(tl)][T.at((tl + 1) % T.size())]) +
                        -input.drone_profits_graph[T.at(tl)][C.at(c)] +
                        -input.drone_profits_graph[C.at(c)][T.at((tl + 1) % T.size())];
            if (dl < d)
            {
                d = dl;
                t = tl;
            }
        }
        i = C.at(c);
        T.insert(T.begin() + t + 1, C.at(c));
        C.erase(C.begin() + c);
    }
    std::vector<std::pair<int, bool>> cdop_tour;
    for (auto cdop : T)
        cdop_tour.push_back(std::make_pair(cdop, cdop <= input.num_truck_nodes));
    return cdop_tour;
}

std::vector<std::pair<int, bool>> Individual::MinimizeDroneTimeTour(Input &input, Settings &settings)
{
    int n = input.num_nodes;
    std::vector<int> C(n);
    std::vector<double> D(n, INT_MAX);
    std::vector<int> T(0);
    int s = 0; // Nó inicial
    for (int j = 0; j < n; j++)
        C[j] = j;

    T.push_back(s);
    C.erase(find(C.begin(), C.end(), s));
    int i = s;
    while (C.size() > 0)
    {
        for (int j = 0; j < C.size(); j++)
        {
            if (input.drone_graph[i][C.at(j)] < D.at(C.at(j)))
                D.at(C.at(j)) = input.drone_graph[i][C.at(j)];
        }
        int c = 0;
        for (int j = 1; j < C.size(); j++)
        {
            if (D.at(C.at(j)) < D.at(C.at(c)))
                c = j;
        }

        double d = INT_MAX;
        int t = -1;
        for (int tl = 0; tl < T.size(); tl++)
        {
            double dl = -input.drone_graph[T.at(tl)][T.at((tl + 1) % T.size())] +
                        input.drone_graph[T.at(tl)][C.at(c)] +
                        input.drone_graph[C.at(c)][T.at((tl + 1) % T.size())];
            if (dl < d)
            {
                d = dl;
                t = tl;
            }
        }
        i = C.at(c);
        T.insert(T.begin() + t + 1, C.at(c));
        C.erase(C.begin() + c);
    }
    std::vector<std::pair<int, bool>> cdop_tour;
    for (auto cdop : T)
        cdop_tour.push_back(std::make_pair(cdop, cdop <= input.num_truck_nodes));
    return cdop_tour;
}

std::vector<std::pair<int, bool>> Individual::GreedyStart(Input &input, Settings &settings)
{
    int n = input.num_drone_nodes;
    std::vector<int> C(n);
    std::vector<double> D(input.num_nodes, INT_MAX);
    std::vector<int> T(0);
    int s = 0; // Nó inicial
    for (int j = 1; j < n; j++)
        C[j] = j + input.num_truck_nodes + 1;
    C[0] = 0;

    T.push_back(s);
    C.erase(find(C.begin(), C.end(), s));
    int i = s;
    while (C.size() > 0)
    {
        for (int j = 0; j < C.size(); j++)
        {
            if (-input.drone_profits_graph[i][C.at(j)] < D.at(C.at(j)))
                D.at(C.at(j)) = -input.drone_profits_graph[i][C.at(j)];
        }
        int c = 0;
        for (int j = 1; j < C.size(); j++)
        {
            if (D.at(C.at(j)) < D.at(C.at(c)))
                c = j;
        }

        double d = INT_MAX;
        int t = -1;
        for (int tl = 0; tl < T.size(); tl++)
        {
            double dl = -(-input.drone_profits_graph[T.at(tl)][T.at((tl + 1) % T.size())]) +
                        -input.drone_profits_graph[T.at(tl)][C.at(c)] +
                        -input.drone_profits_graph[C.at(c)][T.at((tl + 1) % T.size())];
            if (dl < d)
            {
                d = dl;
                t = tl;
            }
        }
        i = C.at(c);
        T.insert(T.begin() + t + 1, C.at(c));
        C.erase(C.begin() + c);
    }
    std::vector<std::pair<int, bool>> cdop_tour;
    for (auto cdop : T)
        cdop_tour.push_back(std::make_pair(cdop, false));
    cdop_tour[0].second = true;

    std::vector<bool> truck_nodes_inserted(input.num_truck_nodes, false);
    double cur_time = 0;
    for (int i = 0; i < cdop_tour.size(); i++)
    {
        int cur = cdop_tour[i].first;
        int next = cdop_tour[(i + 1) % cdop_tour.size()].first;
        if (cur_time <= input.t_max)
        {
            cur_time += input.drone_graph[cur][next];
        }

        if (cur_time > input.t_max)
        {
            bool dealing = true;
            while (dealing)
            {
                cur_time -= input.drone_graph[cur][next];
                for (int t = 1; t < input.num_truck_nodes; t++)
                {
                    if (truck_nodes_inserted[t])
                        continue;
                    if (cur_time + input.drone_graph[cur][t] < input.t_max)
                    {
                        truck_nodes_inserted[t] = true;
                        cur_time += input.drone_graph[cur][t];
                        cdop_tour.insert(cdop_tour.begin() + i + 1, std::make_pair(t, true));
                        dealing = false;
                        cur_time = 0;
                        break;
                    }
                }
                if (dealing)
                {
                    next = cur;
                    cur = cdop_tour[--i].first;
                }
                else
                {
                }
            }
        }
    }

    return cdop_tour;
}

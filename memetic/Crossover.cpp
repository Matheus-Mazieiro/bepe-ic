#include "Crossover.hpp"

#include <queue>

Solution Crossover::MPX(Individual *p1, Individual *p2)
{
    std::vector<int> p1_truck;
    std::vector<int> p2_truck;
    for (auto p : p1->pocket.tour)
        if (p.second)
            p1_truck.push_back(p.first);
    for (auto p : p2->pocket.tour)
        if (p.second)
            p2_truck.push_back(p.first);

    std::uniform_int_distribution<> dis(0, p1_truck.size() - 1);
    int a = dis(p1->settings.rng);
    int b = dis(p1->settings.rng);
    while (b == a)
        b = dis(p1->settings.rng);
    if (a > b)
        std::swap(a, b);

    std::vector<int> from_p1;
    for (int i = a; i <= b; i++)
        from_p1.push_back(p1_truck[i]);

    std::vector<int> indices_at_p2;
    int first = -1;
    for (int j = 0; j < p2_truck.size(); j++)
    {
        for (int i = 0; i < from_p1.size(); i++)
        {
            if (p2_truck[j] == from_p1[i])
            {
                indices_at_p2.push_back(j);
                if (i == 0)
                    first = j;
                break;
            }
        }
    }

    std::vector<std::pair<int, bool>> left_tour;  // .second == true iff that node came from p1
    std::vector<std::pair<int, bool>> right_tour; // .second == true iff that node came from p1
    std::sort(indices_at_p2.begin(), indices_at_p2.end());
    for (int i = 0, j = 0; i < p2_truck.size(); i++)
    {
        if (j < indices_at_p2.size() && i == indices_at_p2[j])
        {
            left_tour.push_back({-1, true});
            right_tour.push_back({-1, true});
            j++;
        }
        else
        {
            left_tour.push_back({p2_truck[i], false});
            right_tour.push_back({p2_truck[i], false});
        }
    }

    int j = 0;
    for (int i = p2_truck.size(); i > 0; i--)
    {
        if (left_tour[(first + i) % p2_truck.size()].first == -1)
        {
            left_tour[(first + i) % p2_truck.size()].first = from_p1[j++];
        }
    }

    j = 0;
    for (int i = 0; i < p2_truck.size(); i++)
    {
        if (right_tour[(first + i) % p2_truck.size()].first == -1)
        {
            right_tour[(first + i) % p2_truck.size()].first = from_p1[j++];
        }
    }

    Solution left;
    left.tour = std::vector<std::pair<int, bool>>();
    for (int i = 0; i < left_tour.size(); i++)
    {
        std::pair<int, bool> target = {left_tour[i].first, true};
        // copia do p1
        if (left_tour[i].second)
        {
            auto it = find(p1->pocket.tour.begin(), p1->pocket.tour.end(), target);
            if (it != p1->pocket.tour.end())
            {
                int index = std::distance(p1->pocket.tour.begin(), it);
                do
                {
                    left.tour.push_back(p1->pocket.tour[index++]);
                } while (index < p1->pocket.tour.size() && p1->pocket.tour[index].second == false);
            }
        }
        // copia do p2
        else
        {
            auto it = find(p2->pocket.tour.begin(), p2->pocket.tour.end(), target);
            if (it != p2->pocket.tour.end())
            {
                int index = std::distance(p2->pocket.tour.begin(), it);
                do
                {
                    left.tour.push_back(p2->pocket.tour[index++]);
                } while (index < p2->pocket.tour.size() && p2->pocket.tour[index].second == false);
            }
        }
    }
    left.profit = Individual::EvaluateTour(p1->input, left.tour, left.time, left.objective);

    Solution right;
    right.tour = std::vector<std::pair<int, bool>>();
    for (int i = 0; i < right_tour.size(); i++)
    {
        std::pair<int, bool> target = {right_tour[i].first, true};
        // copia do p1
        if (right_tour[i].second)
        {
            auto it = find(p1->pocket.tour.begin(), p1->pocket.tour.end(), target);
            if (it != p1->pocket.tour.end())
            {
                int index = std::distance(p1->pocket.tour.begin(), it);
                do
                {
                    right.tour.push_back(p1->pocket.tour[index++]);
                } while (index < p1->pocket.tour.size() && p1->pocket.tour[index].second == false);
            }
        }
        // copia do p2
        else
        {
            auto it = find(p2->pocket.tour.begin(), p2->pocket.tour.end(), target);
            if (it != p2->pocket.tour.end())
            {
                int index = std::distance(p2->pocket.tour.begin(), it);
                do
                {
                    right.tour.push_back(p2->pocket.tour[index++]);
                } while (index < p2->pocket.tour.size() && p2->pocket.tour[index].second == false);
            }
        }
    }

    right.profit = Individual::EvaluateTour(p1->input, right.tour, right.time, right.objective);

    if (left.profit > right.profit)
        return left;
    return right;
}
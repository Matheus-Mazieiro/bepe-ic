#ifndef WARMSTART_H
#define WARMSTART_H

#include "../Input.hpp"
#include "../Settings.hpp"
#include "Individual.hpp"
#include <vector>
#include <queue>

struct Insertion
{
    double cost;
    int vertex;
    int position; // posição no tour onde deve ser inserido
    bool operator>(const Insertion &other) const { return cost > other.cost; }
};

class WarmStart
{
private:
    std::vector<int> CheapestInsertionWithInitialTour(
        const std::vector<std::vector<double>> &dist,
        std::vector<int> initialTour,
        const std::vector<int> &remainingVertices);

public:
    Input &input;
    Settings &settings;

    std::vector<std::pair<int, bool>> tour;

    WarmStart(Input &input, Settings &settings);

    bool InsertDrone(std::vector<std::pair<int, bool>> &solution);
    bool InsertTruck(std::vector<std::pair<int, bool>> &solution);
};

#endif
#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#define DEBUG 1
#include <limits.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include "../Input.hpp"
#include "../Settings.hpp"
#include "../TourEnhancement.hpp"

typedef struct
{
    std::vector<std::pair<int, bool>> tour; // {indice, sincroniza}
    double profit;                          // Sucetivel a pressão evolutiva
    double time;                            // Tempo total do tour
    double objective;                       // Valor objetivo
} Solution;

class Individual
{
private:
public:
    Input &input;
    Settings &settings;
    Solution current;
    Solution pocket;
    Individual(const Individual &other);
    Individual(Input &input, Settings &settings);
    static double EvaluateTour(Input &input, std::vector<std::pair<int, bool>> &tour, double &total_time, double &obj);
    void PrintIndividual(Solution &sol);
    void Optimize(bool enhance = false);
    void Mutate();
    void StructureIndividual();

    std::vector<std::pair<int, bool>> MaximizeProfitTour(Input &input, Settings &settings);
    std::vector<std::pair<int, bool>> MinimizeDroneTimeTour(Input &input, Settings &settings);
    std::vector<std::pair<int, bool>> GreedyStart(Input &input, Settings &settings);
};

#endif
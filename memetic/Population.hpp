#ifndef POPULATION_H
#define POPULATION_H

#include "../Input.hpp"
#include "../Settings.hpp"
#include "Individual.hpp"
#include "Crossover.hpp"
#include <vector>

class Population
{
private:
public:
    Input &input;
    Settings &settings;
    std::vector<Individual *> pop;
    Population(Input &input, Settings &settings);
    void InitPop();
    void StructurePop();
    void Evolve();
    void OptimizePop();
    std::pair<Individual *, Individual *> Crossover(Individual *p1, Individual *p2);
    Individual *SelectParentTournament();
    void Shake();
};

#endif
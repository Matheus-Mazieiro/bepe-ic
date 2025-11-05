#ifndef CROSSOVER_H
#define CROSSOVER_H

#include "Individual.hpp"
#include <vector>
#include <algorithm>

class Crossover
{
private:
public:
    static Solution MPX(Individual *p1, Individual *p2);
};

#endif
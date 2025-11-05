#ifndef MEMETIC_H
#define MEMETIC_H

#include <vector>
#include <iostream>
#include "../Input.hpp"
#include "../Settings.hpp"
#include "Population.hpp"
#include "Individual.hpp"
#include "../Printer.hpp"
#include <chrono>

class Memetic
{
private:
public:
    Input &input;
    Settings &settings;
    Population pop;

    Memetic(Input &input, Settings &settings);
    void Solve();
};

#endif
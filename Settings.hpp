#ifndef SETTINGS_H
#define SETTINGS_H

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <random>

class Settings
{
private:
public:
    int seed;
    int max_iteration;
    int population_size;
    std::string settings_file;
    std::string instance_reduction_method; // 'exact', 'partial', etc...
    std::string solving_method;            // 'exact', 'tsp_exact', 'all', etc...
    std::mt19937 rng;
    std::string crossover;
    std::string local_search;

    Settings(const std::string &filename);
    void PrintSettings(std::ostream &file_out) const;
};

#endif
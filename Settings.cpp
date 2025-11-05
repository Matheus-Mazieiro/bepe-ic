#include "Settings.hpp"

Settings::Settings(const std::string &filename) : settings_file(filename)
{
    std::ifstream file(filename, std::ios::in);

    if (!file)
    {
        std::cout << filename;
        throw std::runtime_error("Cannot open instance file.");
    }

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file >> max_iteration;
        file >> population_size;
        file >> seed;
        rng = std::mt19937(seed);
        file >> instance_reduction_method;
        file >> solving_method;
        file >> crossover;
        file >> local_search;
    }
    catch (std::ifstream::failure &e)
    {
        throw std::fstream::failure("Error reading instance file.");
    }
}

void Settings::PrintSettings(std::ostream &file_out) const
{
    file_out << "========== Settings ==========" << std::endl;
    file_out << "Settings file:        " << settings_file << std::endl;
    file_out << "Seed:                 " << seed << std::endl;
    file_out << "Reduction method:     " << instance_reduction_method << std::endl;
    file_out << "Max iter:             " << max_iteration << std::endl;
    file_out << "Population size:      " << population_size << std::endl;
    file_out << "Crossover:            " << crossover << std::endl;
    file_out << "Local Search:         " << local_search << std::endl;
    file_out << "==============================" << std::endl;
    file_out << std::endl;
}
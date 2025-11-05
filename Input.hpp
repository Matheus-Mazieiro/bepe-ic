#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

class Input
{
private:
public:
    std::string file_name;
    int num_truck_nodes;
    int num_drone_nodes;
    int num_nodes;
    double t_max;
    double d;
    std::vector<std::vector<double>> truck_graph;
    std::vector<std::vector<double>> drone_graph;

    std::vector<int> drones_nodes_profits;
    std::vector<std::vector<int>> drone_profits_graph;

    Input(const std::string &filename);
    void PrintInputData(std::ostream &file_out) const;
    void PrintInput(std::ostream &file_out) const;
};

#endif
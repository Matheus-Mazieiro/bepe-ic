#include "Input.hpp"

Input::Input(const std::string &filename) : file_name(filename)
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
        char c;
        std::string s;

        // Lê m
        file >> c;
        file >> num_truck_nodes;

        // Lê n
        file >> c;
        file >> num_drone_nodes;

        // Lê nodes e profits
        int n = 1 + num_truck_nodes + num_drone_nodes;
        num_nodes = n;
        drones_nodes_profits = std::vector<int>(n, 0);
        for (int i = 0; i < n; i++)
        {
            int p;
            file >> c;
            file >> p;
            file >> p;
            if (c == 'd')
            {
                file >> p;
                drones_nodes_profits[i] = p;
            }
        }

        // Lê truck graph
        std::getline(file, s); // Precisa de 2 para consumir o \n que o 'file >> p' deixou
        std::getline(file, s);
        truck_graph = std::vector<std::vector<double>>(1 + num_truck_nodes, std::vector<double>(1 + num_truck_nodes, 0));
        for (int i = 0; i < 1 + num_truck_nodes; i++)
            for (int j = 0; j < 1 + num_truck_nodes; j++)
                file >> truck_graph[i][j];

        // Lê T_max
        file >> c;
        file >> t_max;

        // Lê D
        file >> c;
        file >> d;

        // Lê drone distance graph
        std::getline(file, s);
        std::getline(file, s);
        drone_graph = std::vector<std::vector<double>>(n, std::vector<double>(n, 0));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                file >> drone_graph[i][j];

        // Lê drone profit graph
        std::getline(file, s);
        std::getline(file, s);
        drone_profits_graph = std::vector<std::vector<int>>(n, std::vector<int>(n, 0));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                file >> drone_profits_graph[i][j];
    }
    catch (std::ifstream::failure &e)
    {
        throw std::fstream::failure("Error reading instance file.");
    }
}

void Input::PrintInputData(std::ostream &file_out) const
{
    int n = 1 + num_truck_nodes + num_drone_nodes;

    file_out << "(" << num_truck_nodes << ", " << num_drone_nodes << ")" << std::endl;

    file_out << "T_max: " << t_max << std::endl;
    file_out << "D: " << d << std::endl;

    file_out << "Profits: " << std::endl;
    for (int i = 0; i < n; i++)
        file_out << i << ": " << drones_nodes_profits[i] << std::endl;

    file_out << "Truck graph:" << std::endl;
    for (auto v : truck_graph)
    {
        for (auto e : v)
            file_out << e << " ";
        file_out << std::endl;
    }
    file_out << "Drone distance graph:" << std::endl;
    for (auto v : drone_graph)
    {
        for (auto e : v)
            file_out << e << " ";
        file_out << std::endl;
    }

    file_out << "Drone profit graph:" << std::endl;
    for (auto v : drone_profits_graph)
    {
        for (auto e : v)
            file_out << e << " ";
        file_out << std::endl;
    }
}

void Input::PrintInput(std::ostream &file_out) const
{
    int n = 1 + num_truck_nodes + num_drone_nodes;

    file_out << "=========== Input ===========" << std::endl;
    file_out << "File name:                " << file_name << std::endl;
    file_out << "Total node count:         " << n << std::endl;
    file_out << "Truck node count:         " << num_truck_nodes << std::endl;
    file_out << "Drone node count:         " << num_drone_nodes << std::endl;
    file_out << "=============================" << std::endl;
    file_out << std::endl;
}
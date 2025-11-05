#ifndef TOURENHANCEMENT_H
#define TOURENHANCEMENT_H

#include "Input.hpp"
#include <vector>
#include <iostream>

class TourEnhancement
{
private:
public:
    static std::vector<std::vector<std::vector<std::vector<bool>>>> Enhance(Input &input, std::vector<std::vector<bool>> truck_tour, std::vector<std::vector<std::vector<std::vector<bool>>>> drone_tour);
    static std::vector<std::pair<int, bool>> Enhance(Input &input, std::vector<std::pair<int, bool>> tour, bool add_vertex);
};

#endif
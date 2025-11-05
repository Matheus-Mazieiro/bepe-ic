#ifndef PRINTER_H
#define PRINTER_H

#include <vector>
#include <iostream>
#include <chrono>

class Printer
{
private:
public:
    static std::chrono::_V2::system_clock::time_point starting_time;
    static std::vector<std::pair<int, bool>> solution;
    static std::vector<std::vector<bool>> truck;
    static std::vector<std::vector<std::vector<std::vector<bool>>>> drone;
    static double solution_value;
    static int iter_count;
    static std::chrono::_V2::system_clock::time_point current_time;
    static int best_improved_iter;
    static int improve_count;
    static std::ostream *file_out;
    static int time_to_print;
    static bool opt;
    static void StartPrinter(std::chrono::_V2::system_clock::time_point starting_time);
    static bool UpdateStatus(std::vector<std::pair<int, bool>> &solution,
                             double solution_value,
                             int iter_count,
                             std::chrono::_V2::system_clock::time_point current_time,
                             int best_improved_iter);

    static bool UpdateStatus(
        std::vector<std::vector<bool>> &truck,
        std::vector<std::vector<std::vector<std::vector<bool>>>> &drone,
        double solution_value,
        int iter_count,
        std::chrono::_V2::system_clock::time_point current_time,
        int best_improved_iter);

    static void PrintStatus(std::ostream &file_out);
};

#endif
#include "Printer.hpp"

std::chrono::_V2::system_clock::time_point Printer::starting_time;
std::vector<std::pair<int, bool>> Printer::solution;
std::vector<std::vector<bool>> Printer::truck;
std::vector<std::vector<std::vector<std::vector<bool>>>> Printer::drone;
double Printer::solution_value;
int Printer::iter_count;
std::chrono::_V2::system_clock::time_point Printer::current_time;
int Printer::best_improved_iter;
int Printer::improve_count;
std::ostream *Printer::file_out;
int Printer::time_to_print = 250;
bool Printer::opt = false;

void Printer::StartPrinter(std::chrono::_V2::system_clock::time_point starting_time)
{
    Printer::starting_time = starting_time;
    Printer::solution_value = -1;
    Printer::current_time = starting_time;
    Printer::improve_count = -1;
}

bool Printer::UpdateStatus(std::vector<std::pair<int, bool>> &solution,
                           double solution_value,
                           int iter_count,
                           std::chrono::_V2::system_clock::time_point current_time,
                           int best_improved_iter)
{
    Printer::iter_count = iter_count;
    Printer::current_time = current_time;
    //if (solution_value > Printer::solution_value)
    {
        Printer::solution_value = solution_value;
        Printer::solution = solution;
        Printer::best_improved_iter = best_improved_iter;
        Printer::improve_count++;
        return true;
    }
    std::cout << "Not updated due to worst solution" << std::endl;
    return false;
}

bool Printer::UpdateStatus(
    std::vector<std::vector<bool>> &truck,
    std::vector<std::vector<std::vector<std::vector<bool>>>> &drone,
    double solution_value,
    int iter_count,
    std::chrono::_V2::system_clock::time_point current_time,
    int best_improved_iter)
{
    Printer::iter_count = iter_count;
    Printer::current_time = current_time;
    if (solution_value > Printer::solution_value)
    {
        Printer::solution_value = solution_value;
        Printer::truck = truck;
        Printer::drone = drone;
        Printer::best_improved_iter = best_improved_iter;
        Printer::improve_count++;
        return true;
    }
    return false;
}

void Printer::PrintStatus(std::ostream &file_out)
{
    file_out << "====== Algorithm Status ======" << std::endl;
    file_out << "Valor:                " << solution_value << std::endl;
    file_out << "Time:                 " << std::chrono::duration_cast<std::chrono::milliseconds>(current_time - starting_time).count() << std::endl;
    file_out << "nIter:                " << iter_count << std::endl;
    file_out << "Best Iter:            " << best_improved_iter << std::endl;
    file_out << "Improvement Count:    " << improve_count << std::endl;
    file_out << "==============================" << std::endl;

    if (solution.size() != 0)
    {
        file_out << "Solution tour: ";
        for (auto p : solution)
        {
            if (p.second)
                file_out << std::endl;
            file_out << p.first << " ";
        }
        file_out << std::endl;
    }
    else
    {
        file_out << "Optimal:              " << opt << std::endl;
        file_out << "Truck:" << std::endl;
        for (auto xi : truck)
        {
            for (auto xij : xi)
                file_out << xij << " ";
            file_out << std::endl;
        }

        file_out << "Drone:" << std::endl;
        for (int i = 0; i < truck.size(); i++)
        {
            for (int j = 0; j < truck[i].size(); j++)
            {
                if (truck[i][j] == false)
                    continue;
                file_out << "i = " << i << "      j = " << j << std::endl;
                for (int m = 0; m < drone[i][j].size(); m++)
                {
                    for (int n = 0; n < drone[i][j][m].size(); n++)
                    {
                        file_out << drone[i][j][m][n] << " ";
                    }
                    file_out << std::endl;
                }
            }
        }
    }
    file_out << "==============================" << std::endl;
    file_out << std::endl;
}
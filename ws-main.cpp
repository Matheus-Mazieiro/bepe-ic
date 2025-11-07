#include <chrono>
#include <sys/stat.h>

#include "Input.hpp"
#include "Settings.hpp"
#include "Printer.hpp"
#include "memetic/WarmStart.hpp"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Wrong number of parameters." << std::endl;
        return 0;
    }
    std::string diretory = "Output/";
    mkdir(diretory.c_str(), 0777);
    std::ostream *out = new std::ofstream((diretory + argv[3]), std::ios::out | std::ios::trunc);

    // Read input
    std::string input_file = argv[1];
    Input input(input_file);
    input.PrintInput(*out);

    // Read settings
    std::string setting_file = argv[2];
    Settings settings(setting_file);
    settings.PrintSettings(*out);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Write file
    Printer::file_out = out;

    // Testando Warm Start
    {

        auto start_time = std::chrono::high_resolution_clock::now();
        Individual *newIdividual = new Individual(input, settings);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::ostream &file_out = *Printer::file_out;
        file_out << "====== Random Pocket ======" << std::endl;
        file_out << "Valor Pocket:         " << newIdividual->pocket.profit;
        for (auto t : newIdividual->pocket.tour)
        {
            if (t.second)
                file_out << std::endl;
            file_out << t.first << " ";
        }
        file_out << std::endl;
        file_out << "====== Random Current =====" << std::endl;
        file_out << "Valor Current:        " << newIdividual->current.profit;
        for (auto t : newIdividual->current.tour)
        {
            if (t.second)
                file_out << std::endl;
            file_out << t.first << " ";
        }
        file_out << std::endl;

        start_time = std::chrono::high_resolution_clock::now();
        WarmStart ws(input, settings);
        std::vector<std::pair<int, bool>> solution(1, {0, true});
        ws.InsertTruck(solution);
        solution = TourEnhancement::Enhance(input, solution, true);
        newIdividual->current.tour = solution;
        newIdividual->current.profit = newIdividual->EvaluateTour(input, newIdividual->current.tour, newIdividual->current.time, newIdividual->current.objective);
        end_time = std::chrono::high_resolution_clock::now();
        file_out << "====== Warm Start ======" << std::endl;
        file_out << "Valor:                " << newIdividual->current.profit << std::endl;
        file_out << "Time(µs):             " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        for (auto t : newIdividual->pocket.tour)
        {
            if (t.second)
                file_out << std::endl;
            file_out << t.first << " ";
        }
        file_out << std::endl;
        file_out << "==============================" << std::endl;

        // newIdividual->PrintIndividual(newIdividual->current);
    }
}

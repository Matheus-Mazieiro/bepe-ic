#include <chrono>
#include <sys/stat.h>

#include "Input.hpp"
#include "memetic/Memetic.hpp"
#include "ReducedInstanceSolver_gurobi.hpp"
#include "Settings.hpp"
#include "tsp_gurobi.hpp"
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
    Printer::StartPrinter(start_time);
    Printer::file_out = out;

    // Testando Warm Start
    //{
    //    std::cout << "Testando Warm Start!" << std::endl;
    //    WarmStart ws(input, settings);
    //    std::vector<std::pair<int, bool>> solution(1, {0, true});
    //    for (auto p : solution)
    //    {
    //        if (p.second)
    //            std::cout << "*";
    //        std::cout << p.first << " ";
    //    }
    //    std::cout << std::endl;
//
    //    ws.InsertTruck(solution);
    //    for (auto p : solution)
    //    {
    //        if (p.second)
    //            std::cout << "*";
    //        std::cout << p.first << " ";
    //    }
    //    std::cout << std::endl;
//
    //    solution = TourEnhancement::Enhance(input, solution, true);
    //    for (auto p : solution)
    //    {
    //        if (p.second)
    //            std::cout << "*";
    //        std::cout << p.first << " ";
    //    }
    //    std::cout << std::endl;
    //}

    // Although good idea, this doesnt necesserly helps, considering that it stills remaing NP-hard(?)
    if (settings.solving_method == "tsp_exact")
    {
        std::vector<std::vector<bool>> tsp_tour;
        // Create reduced instance
        std::cout << "Solving TSP" << std::endl;
        TSP tsp(input, settings, tsp_tour);
        std::cout << "TSP took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "ms" << std::endl;

        start_time = std::chrono::high_resolution_clock::now();
        // Solve reduced instance
        std::cout << "Solving reduced considering OPT_TSP:" << std::endl;
        ReducedInstanceSolver reduced(input, settings, tsp_tour);
        std::cout << "Reduced took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "ms" << std::endl;
    }

    if (settings.solving_method == "TSP+LS")
    {
        std::vector<std::vector<bool>> tsp_tour;
        // Create reduced instance
        std::cout << "Solving TSP" << std::endl;
        TSP tsp(input, settings, tsp_tour);
        std::vector<std::pair<int, bool>> tsp_sequence = {{0, true}};
        while (true)
        {
            int next;
            bool isSync;
            for (int i = 0; i < tsp_tour.size(); i++)
            {
                if (tsp_tour[tsp_sequence.back().first][i])
                {
                    next = i;
                    isSync = i <= input.num_truck_nodes;
                    break;
                }
            }
            if (next == 0)
                break;
            tsp_sequence.push_back({next, isSync});
        }

        // double after_enhancement = TourEnhancement::Enhance(input, tsp_sequence);
        // std::cout << "TSP took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "ms (" << after_enhancement << ")" << std::endl;
        // Printer::UpdateStatus(tsp_sequence, after_enhancement, -1, std::chrono::high_resolution_clock::now(), -1);
    }

    if (settings.solving_method == "memetic")
    {
        std::cout << "Running Memetic..." << std::endl;
        Memetic memetic(input, settings);
        memetic.Solve();
        std::cout << "Memetic took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "ms" << std::endl;

        start_time = std::chrono::high_resolution_clock::now();
    }

    if (settings.solving_method == "exact")
    {
        std::vector<std::vector<bool>> empty_tour(0);
        start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Solving exact:" << std::endl;
        ReducedInstanceSolver exact(input, settings, empty_tour);
        std::cout << "Exact took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "ms" << std::endl;
    }

    Printer::PrintStatus(*out);
}

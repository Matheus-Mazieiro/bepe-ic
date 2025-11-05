#include "Memetic.hpp"

Memetic::Memetic(Input &input, Settings &settings) : input(input), settings(settings), pop(input, settings)
{
}

void Memetic::Solve()
{
    // Population pop(input, settings);
    pop.InitPop();
    pop.StructurePop();

    int iters_without_improovement = 0;
    double best = pop.pop[0]->current.profit;
    int delta_time;
    int iter_count = 0;
    do
    {
        iter_count++;
        pop.Evolve();

        if (pop.pop[0]->current.profit > best)
        {
            best = pop.pop[0]->current.profit;
            iters_without_improovement = 0;
        }
        iters_without_improovement++;

        if (iters_without_improovement >= 15)
        {
            pop.Shake();
            iters_without_improovement = 0;
            best = pop.pop[0]->current.profit;
        }

        auto current_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - Printer::starting_time).count();

        Printer::UpdateStatus(pop.pop[0]->pocket.tour,
                              pop.pop[0]->pocket.objective,
                              iter_count,
                              current_time,
                              iter_count);

        if (delta_time >= Printer::time_to_print && Printer::time_to_print < settings.max_iteration)
        {
            Printer::PrintStatus(*Printer::file_out);
            Printer::time_to_print *= 2;
        }

    } while (delta_time <= settings.max_iteration);

    //    for (auto ind : pop.pop)
    //    {
    //        ind->current.tour = TourEnhancement::Enhance(input, ind->current.tour, false);
    //        ind->current.profit = Individual::EvaluateTour(input, ind->current.tour, ind->current.time, ind->current.objective);
    //        ind->pocket.tour = TourEnhancement::Enhance(input, ind->pocket.tour, false);
    //        ind->pocket.profit = Individual::EvaluateTour(input, ind->pocket.tour, ind->pocket.time, ind->pocket.objective);
    //    }

    pop.StructurePop();

    std::cout << "\n--- Melhor Solução Encontrada ---" << std::endl;
    std::cout << "Pocket: ";
    pop.pop[0]->PrintIndividual(pop.pop[0]->pocket);
    std::cout << "Current: ";
    pop.pop[0]->PrintIndividual(pop.pop[0]->current);

    Solution aaa;
    auto current_time = std::chrono::high_resolution_clock::now();
    aaa.tour = TourEnhancement::Enhance(input, pop.pop[0]->pocket.tour, false);
        Printer::UpdateStatus(aaa.tour,
                          aaa.objective,
                          iter_count,
                          current_time,
                          iter_count);
    Printer::PrintStatus(std::cout);

    aaa.tour = TourEnhancement::Enhance(input, aaa.tour, false);
    aaa.profit = Individual::EvaluateTour(input, aaa.tour, aaa.time, aaa.objective);
    
    Printer::UpdateStatus(aaa.tour,
                          aaa.objective,
                          iter_count,
                          current_time,
                          iter_count);
    Printer::PrintStatus(*Printer::file_out);
    Printer::PrintStatus(std::cout);

    for (int i = 1; i < pop.pop.size(); ++i)
        delete pop.pop[i];
}

#include "Population.hpp"

#include "WarmStart.hpp"
#define TWO_INDIVIDUAL_WS 0
#define GREEDY_WS 0
#define MARIO_WS 0

Population::Population(Input &input, Settings &settings) : input(input), settings(settings)
{
}

void Population::InitPop()
{
    for (int i = 0; i < settings.population_size; i++)
    {
        Individual *newIdividual = new Individual(input, settings);
        pop.push_back(newIdividual);
    }
    // return;
    if (MARIO_WS)
    {
        std::vector<std::pair<int, bool>> solution(1, {0, true});
        WarmStart ws(input, settings);
        ws.InsertTruck(solution);
        solution = TourEnhancement::Enhance(input, solution, true);
        pop[0]->current.tour = solution;
        pop[0]->current.profit = pop[0]->EvaluateTour(input, pop[0]->current.tour, pop[0]->current.time, pop[0]->current.objective);
        return;
    }

    // Warm start com 2 individuos: 1 que maximiza lucro; outro minimiza tempo
    if (TWO_INDIVIDUAL_WS)
    {
        //      Minimiza tempo
        pop[0]->current.tour = pop[0]->MinimizeDroneTimeTour(input, settings);
        std::reverse(pop[0]->current.tour.begin() + 1, pop[0]->current.tour.end());
        pop[0]->pocket.tour = pop[0]->MinimizeDroneTimeTour(input, settings);
        pop[0]->current.tour = TourEnhancement::Enhance(input, pop[0]->current.tour, settings.local_search == "LS");
        pop[0]->pocket.tour = TourEnhancement::Enhance(input, pop[0]->pocket.tour, settings.local_search == "LS");
        //       Maximiza lucro
        auto bestTour = pop[1]->MaximizeProfitTour(input, settings);
        pop[1]->current.tour = bestTour;
        std::reverse(pop[1]->current.tour.begin() + 1, pop[1]->current.tour.end());
        pop[1]->pocket.tour = bestTour;
        pop[1]->current.tour = TourEnhancement::Enhance(input, pop[1]->current.tour, settings.local_search == "LS");
        pop[1]->pocket.tour = TourEnhancement::Enhance(input, pop[1]->pocket.tour, settings.local_search == "LS");

        pop[0]->current.profit = pop[0]->EvaluateTour(input, pop[0]->current.tour, pop[0]->current.time, pop[0]->current.objective);
        pop[0]->pocket.profit = pop[0]->EvaluateTour(input, pop[0]->pocket.tour, pop[0]->current.time, pop[0]->current.objective);
        pop[1]->current.profit = pop[1]->EvaluateTour(input, pop[1]->current.tour, pop[1]->current.time, pop[1]->current.objective);
        pop[1]->pocket.profit = pop[1]->EvaluateTour(input, pop[1]->pocket.tour, pop[1]->current.time, pop[1]->current.objective);
    }
    //   End 2 ind Warm Start

    if (GREEDY_WS)
    {
        auto bestTour = pop[0]->GreedyStart(input, settings);
        pop[0]->current.tour = bestTour;
        std::reverse(pop[0]->current.tour.begin() + 1, pop[0]->current.tour.end());
        pop[0]->pocket.tour = bestTour;
        pop[0]->current.profit = pop[0]->EvaluateTour(input, pop[0]->current.tour, pop[0]->current.time, pop[0]->current.objective);
        pop[0]->pocket.profit = pop[0]->EvaluateTour(input, pop[0]->pocket.tour, pop[0]->current.time, pop[0]->current.objective);
        pop[0]->current.tour = TourEnhancement::Enhance(input, pop[0]->current.tour, settings.local_search == "LS");
        pop[0]->pocket.tour = TourEnhancement::Enhance(input, pop[0]->pocket.tour, settings.local_search == "LS");

        std::cout << "current ws: ";
        for (auto e : pop[0]->current.tour)
        {
            if (e.second)
                std::cout << "*";
            std::cout << e.first << " ";
        }
        std::cout << std::endl;
        std::cout << "pocket ws: ";
        for (auto e : pop[0]->pocket.tour)
        {
            if (e.second)
                std::cout << "*";
            std::cout << e.first << " ";
        }
        std::cout << std::endl;
    }
}

void Population::StructurePop()
{
    // Update Pocket
    for (auto ind : pop)
        ind->StructureIndividual();

    // Pocket Propagation
    for (int i = 1; i < pop.size(); ++i)
    {
        int child = i;
        while (child > 0)
        {
            int parent = (child - 1) / 3;
            if (pop[child]->pocket.profit > pop[parent]->pocket.profit)
            {
                std::swap(pop[child], pop[parent]);
                child = parent;
            }
            else
                break;
        }
    }
}

void Population::Evolve()
{
    //    std::cout << settings.crossover << std::endl;
    if (settings.crossover == "MA-MPX")
    {
        for (int i = 1; i < pop.size(); i++)
        {
            Individual *p1 = pop[(i - 1) / 3];
            Individual *p2 = pop[i];

            p2->current = Crossover::MPX(p1, p2);

            p2->Mutate();
            p2->Optimize();
            p2->current.profit = Individual::EvaluateTour(input, p2->current.tour, p2->current.time, p2->current.objective);
        }
        StructurePop();
        return;
    }

    // Atualiza os individuos conforme o resultado de crossover
    if (settings.crossover == "MA-MMX")
    {
        for (int i = 1; i < pop.size(); i++)
        {
            Individual *p1 = pop[(i - 1) / 3];
            Individual *p2 = pop[i];

            std::pair<Individual *, Individual *> offspring_raw = Crossover(p1, p2);
            if (offspring_raw.first->current.profit > offspring_raw.second->current.profit)
                p2->current = offspring_raw.first->current;
            else
                p2->current = offspring_raw.second->current;

            delete offspring_raw.first;
            delete offspring_raw.second;

            p2->Mutate();
            p2->Optimize();
            p2->current.profit = Individual::EvaluateTour(input, p2->current.tour, p2->current.time, p2->current.objective);
        }
        StructurePop();
        return;
    }

    // Substitui toda a populacao
    if (settings.crossover == "GA-like")
    {
        std::vector<Individual *> new_pop;
        new_pop.reserve(settings.population_size);

        StructurePop();
        if (!pop.empty())
        {
            for (int i = 0; i < settings.population_size / 5; i++)
            {
                new_pop.push_back(pop[i]);
            }
        }

        while (new_pop.size() < settings.population_size)
        {
            Individual *p1 = SelectParentTournament();
            Individual *p2 = SelectParentTournament();

            std::pair<Individual *, Individual *> offspring_raw = Crossover(p1, p2);

            auto child1 = offspring_raw.first;
            auto child2 = offspring_raw.second;

            child1->Mutate();
            child2->Mutate();

            child1->Optimize();
            child2->Optimize();

            child1->current.profit = Individual::EvaluateTour(input, child1->current.tour, child1->current.time, child1->current.objective);
            child2->current.profit = Individual::EvaluateTour(input, child2->current.tour, child2->current.time, child2->current.objective);

            new_pop.push_back(std::move(child1));
            if (new_pop.size() < settings.population_size)
                new_pop.push_back(std::move(child2));
        }

        pop = std::move(new_pop);
        StructurePop();
    }
}

Individual *Population::SelectParentTournament()
{
    int tournament_size = 2;
    int best_index = -1;
    double best_profit = -1e9;

    for (int i = 0; i < tournament_size; ++i)
    {
        int rand_index = settings.rng() % pop.size();
        if (pop[rand_index]->pocket.profit > best_profit)
        {
            best_profit = pop[rand_index]->pocket.profit;
            best_index = rand_index;
        }
    }
    return pop[best_index];
}

std::pair<Individual *, Individual *> Population::Crossover(Individual *p1, Individual *p2)
{
    int a = 1 + (p1->pocket.tour.size() - 1) * settings.rng() / settings.rng.max();
    int b = 1 + (p2->pocket.tour.size() - 1) * settings.rng() / settings.rng.max();
    Individual *new_a = new Individual(*p1);
    Individual *new_b = new Individual(*p2);

    new_a->current.tour = std::vector<std::pair<int, bool>>();
    new_b->current.tour = std::vector<std::pair<int, bool>>();
    for (int i = 0; i < a; i++)
        new_a->current.tour.push_back(p1->pocket.tour[i]);
    for (int i = b; i < p2->pocket.tour.size(); i++)
        new_a->current.tour.push_back(p2->pocket.tour[i]);

    for (int i = 0; i < b; i++)
        new_b->current.tour.push_back(p2->pocket.tour[i]);
    for (int i = a; i < p1->pocket.tour.size(); i++)
        new_b->current.tour.push_back(p1->pocket.tour[i]);

    new_a->current.profit = Individual::EvaluateTour(input, new_a->current.tour, new_a->current.time, new_a->current.objective);
    new_b->current.profit = Individual::EvaluateTour(input, new_b->current.tour, new_b->current.time, new_b->current.objective);

    return {new_a, new_b};
}

void Population::OptimizePop()
{
    for (auto ind : pop)
        ind->Optimize(true);
}

void Population::Shake()
{
    for (int i = 1; i < pop.size(); ++i)
        delete pop[i];
    pop.resize(1);
    while (pop.size() <= settings.population_size)
    {
        Individual *newIdividual = new Individual(input, settings);
        pop.push_back(newIdividual);
    }
}
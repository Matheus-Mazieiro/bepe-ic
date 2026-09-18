#include <queue>
#include <vector>
#include <ilcplex/ilocplex.h>
#include <vector>
#include <iostream>

#include "../Input.hpp"

ILOSTLBEGIN

typedef IloArray<IloBoolVarArray> BoolVarMatrix2D;
typedef IloArray<BoolVarMatrix2D> BoolVarMatrix3D;
typedef IloArray<BoolVarMatrix3D> BoolVarMatrix4D;

void printCplex(IloCplex cplex)
{
    IloCplex::CplexStatus status = cplex.getCplexStatus();
    std::cout << "========================================\n";
    std::cout << "STATUS DA SOLUCAO: " << status << "\n";
    std::cout << "========================================\n";

    if (status != IloCplex::Optimal && status != IloCplex::Feasible)
    {
        std::cout << "Nao ha solucao viavel disponivel para impressao.\n";
        return;
    }

    std::cout << "Objetivo Calculado = " << cplex.getObjValue() << "\n\n";

    std::cout << "--- ESTATISTICAS DE DESEMPENHO ---\n";
    std::cout << "Tempo de Execucao  : " << cplex.getTime() << " segundos\n";
    std::cout << "Iteracoes Simplex  : " << cplex.getNiterations() << "\n";
    if (cplex.isMIP())
    {
        std::cout << "Nos da Arvore (MIP): " << cplex.getNnodes() << "\n";
        std::cout << "MIP Gap Atual      : " << (cplex.getMIPRelativeGap() * 100.0) << "%\n";
    }
    std::cout << "\n";

    IloEnv env = cplex.getEnv();
    IloModel model = cplex.getModel();

    IloNumVarArray vars(env);
    IloRangeArray ranges(env);

    IloModel::Iterator it(model);
    while (it.ok())
    {
        IloExtractable ext = *it;

        if (ext.isVariable())
        {
            // Conversao correta usando o metodo nativo da classe base
            vars.add(ext.asVariable());
        }
        else if (ext.isConstraint())
        {
            // No Concert, IloRange herda de IloConstraint.
            // O cast seguro de ponteiro/referencia implícita resolve o tipo.
            IloConstraint con = ext.asConstraint();
            if (con.getImpl())
            {
                ranges.add((IloRange &)con);
            }
        }
        ++it;
    }

    if (vars.getSize() > 0)
    {
        std::cout << "--- VARIAVEIS DE DECISAO ---\n";
        IloNumArray varValues(env);
        cplex.getValues(varValues, vars);

        IloNumArray reducedCosts(env);
        bool hasReducedCosts = !cplex.isMIP();
        if (hasReducedCosts)
        {
            try
            {
                cplex.getReducedCosts(reducedCosts, vars);
            }
            catch (...)
            {
                hasReducedCosts = false;
            }
        }

        for (IloInt i = 0; i < vars.getSize(); ++i)
        {
            if (vars[i].getName())
            {
                std::cout << "  " << vars[i].getName() << " = " << varValues[i];
            }
            else
            {
                std::cout << "  Var_" << i << " = " << varValues[i];
            }

            if (hasReducedCosts)
            {
                std::cout << " | Custo Reduzido = " << reducedCosts[i];
            }
            std::cout << "\n";
        }
        varValues.end();
        reducedCosts.end();
        std::cout << "\n";
    }

    if (ranges.getSize() > 0)
    {
        std::cout << "--- RESTRICOES ---\n";
        IloNumArray slacks(env);
        cplex.getSlacks(slacks, ranges);

        IloNumArray dualPrices(env);
        bool hasDualPrices = !cplex.isMIP();
        if (hasDualPrices)
        {
            try
            {
                cplex.getDuals(dualPrices, ranges);
            }
            catch (...)
            {
                hasDualPrices = false;
            }
        }

        for (IloInt i = 0; i < ranges.getSize(); ++i)
        {
            if (ranges[i].getName())
            {
                std::cout << "  " << ranges[i].getName() << " -> Folga = " << slacks[i];
            }
            else
            {
                std::cout << "  Restricao_" << i << " -> Folga = " << slacks[i];
            }

            if (hasDualPrices)
            {
                std::cout << " | Preco Sombra = " << dualPrices[i];
            }
            std::cout << "\n";
        }
        slacks.end();
        dualPrices.end();
    }
    std::cout << "========================================\n";

    vars.end();
    ranges.end();
}

ILOLAZYCONSTRAINTCALLBACK2(ConnectivityCallback, BoolVarMatrix4D, y, const Input &, input)
{
    IloEnv env = getEnv();

    int N = input.num_nodes;

    // para cada arco do caminhão
    for (int i = 0; i <= input.num_truck_nodes; i++)
    {
        for (int j = 0; j <= input.num_truck_nodes; j++)
        {
            //----------------------------------------------------------
            // Monta o grafo escolhido
            //----------------------------------------------------------

            std::vector<std::vector<int>> G(N);

            for (int v = 0; v < N; v++)
            {
                for (int w = 0; w < N; w++)
                {
                    if (input.drone_graph[v][w] <= 0)
                        continue;

                    if (getValue(y[i][j][v][w]) > 0.5)
                        G[v].push_back(w);
                }
            }

            //----------------------------------------------------------
            // procura componentes
            //----------------------------------------------------------

            std::vector<int> component(N, -1);

            int comp = 0;

            for (int s = 0; s < N; s++)
            {
                if (component[s] != -1)
                    continue;

                std::queue<int> Q;

                Q.push(s);

                component[s] = comp;

                while (!Q.empty())
                {
                    int u = Q.front();
                    Q.pop();

                    for (int v : G[u])
                    {
                        if (component[v] == -1)
                        {
                            component[v] = comp;
                            Q.push(v);
                        }
                    }
                }

                comp++;
            }

            if (comp == 1)
                continue;

            //----------------------------------------------------------
            // existe mais de uma componente
            //----------------------------------------------------------

            for (int c = 1; c < comp; c++)
            {
                std::vector<bool> inS(N, false);

                for (int v = 0; v < N; v++)
                    if (component[v] == c)
                        inS[v] = true;

                IloExpr lhs(env);

                for (int v = 0; v < N; v++)
                {
                    if (inS[v])
                        continue;

                    for (int w = 0; w < N; w++)
                    {
                        if (!inS[w])
                            continue;

                        if (input.drone_graph[v][w] > 0)
                            lhs += y[i][j][v][w];
                    }
                }

                //------------------------------------------------------
                // para cada arco interno de S
                //------------------------------------------------------

                for (int vp = 0; vp < N; vp++)
                {
                    if (!inS[vp])
                        continue;

                    for (int wp = 0; wp < N; wp++)
                    {
                        if (!inS[wp])
                            continue;

                        if (getValue(y[i][j][vp][wp]) < 0.5)
                            continue;

                        add(lhs >= y[i][j][vp][wp]);
                    }
                }

                lhs.end();
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Wrong number of parameters." << std::endl;
        std::cout << "Expected: \n <executavel> <inputFile> <outputFile>" << std::endl;
        return 0;
    }
    Input input(argv[1]);
    std::ofstream arquivo(argv[2]);
    std::streambuf *bufferOriginal = std::cout.rdbuf();
    std::cout.rdbuf(arquivo.rdbuf());

    input.PrintInput(std::cout);

    int M = input.num_nodes;

    IloEnv env;

    try
    {
        IloModel model(env);

        // Vars
        IloBoolVarArray z(env, input.num_drone_nodes);     // z[0..n-1]
        IloArray<IloBoolVarArray> h(env, input.num_nodes); // h[0..n-1][0..m-1]
        for (int i = 0; i < input.num_nodes; i++)
            h[i] = IloBoolVarArray(env, input.num_nodes);
        IloArray<IloBoolVarArray> x(env, input.num_truck_nodes + 1); // x[0..n-1][0..m-1]
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
            x[i] = IloBoolVarArray(env, input.num_truck_nodes + 1);
        BoolVarMatrix4D y(env, input.num_truck_nodes + 1); // y[0..n-1][0..m-1][0..u-1][0..v-1]
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            y[i] = BoolVarMatrix3D(env, input.num_truck_nodes + 1);
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                y[i][j] = BoolVarMatrix2D(env, input.num_nodes);
                for (int k = 0; k < input.num_nodes; k++)
                    y[i][j][k] = IloBoolVarArray(env, input.num_nodes);
            }
        }
        IloNumVarArray u(env, input.num_truck_nodes + 1); // u[0..n-1][0..m-1]
        for (int i = 0; i <= input.num_truck_nodes; i++)
            u[i] = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);

        IloArray<IloNumVarArray> D(env, input.num_truck_nodes + 1);
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            D[i] = IloNumVarArray(env, input.num_truck_nodes + 1);
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                D[i][j] = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);
            }
        }

        // Objetivo
        IloExpr obj(env);
        for (int i = 0; i < input.num_drone_nodes; i++)
            obj += z[i] * input.drones_nodes_profits[1 + input.num_truck_nodes + i]; // Colocar parametro aqui
        for (int v = 0; v < input.num_nodes; v++)
            for (int w = 0; w < input.num_nodes; w++)
                obj += h[v][w] * input.drone_profits_graph[v][w]; // Colocar parametro aqui
        model.add(IloMaximize(env, obj));

        // Restricoes
        // 2.
        for (int j = 1; j < input.num_truck_nodes + 1; j++)
        {
            IloExpr r2_l(env);
            IloExpr r2_r(env);
            for (int i = 0; i < input.num_truck_nodes + 1; i++)
            {
                r2_l += x[i][j];
                r2_r += x[j][i];
            }
            model.add(r2_l == r2_r);
            model.add(r2_r == 1);
        }

        // 3.
        IloExpr r3_r(env);
        IloExpr r3_l(env);
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            r3_l += x[i][0];
            r3_r += x[0][i];
        }
        model.add(r3_l == r3_r);
        model.add(r3_r == 1);

        // 4.
        for (int i = 1; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 1; j < input.num_truck_nodes + 1; j++)
            {
                if (i == j)
                    continue;

                model.add(u[i] - u[j] + 1 <= M * (1 - x[i][j]));
            }
        }

        // 5.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                for (int w = 0; w < input.num_nodes; w++)
                {
                    if (w == i || w == j)
                        continue;
                    IloExpr r5_l(env);
                    for (int v = 0; v < input.num_nodes; v++)
                        r5_l += y[i][j][v][w] - y[i][j][w][v];
                    model.add(r5_l == 0);
                }
            }
        }

        // 6.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                IloExpr r6_l(env);
                for (int v = 0; v < input.num_nodes; v++)
                {
                    r6_l += y[i][j][i][v] - y[i][j][v][i];
                }
                model.add(r6_l == x[i][j]);
            }
        }

        // 7.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                IloExpr r7_l(env);
                for (int v = 0; v < input.num_nodes; v++)
                {
                    r7_l += y[i][j][v][j] - y[i][j][j][v];
                }
                model.add(r7_l == x[i][j]);
            }
        }

        // 8.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                IloExpr r8_l(env);
                for (int v = 0; v < input.num_nodes; v++)
                {
                    for (int w = 0; w < input.num_nodes; w++)
                    {
                        r8_l += y[i][j][v][w] * input.drone_graph[v][w];
                    }
                }
                model.add(r8_l <= input.t_max);
            }
        }

        // 10.
        for (int v = 0; v < input.num_drone_nodes; v++)
        {
            IloExpr r10_l(env);
            for (int i = 0; i < input.num_truck_nodes + 1; i++)
            {
                for (int j = 0; j < input.num_truck_nodes + 1; j++)
                {
                    for (int w = 0; w < input.num_nodes; w++)
                    {
                        r10_l += y[i][j][1 + input.num_truck_nodes + v][w];
                    }
                }
            }
            model.add(r10_l >= z[v]);
        }

        // 11.
        for (int v = 0; v < input.num_nodes; v++)
        {
            for (int w = 0; w < input.num_nodes; w++)
            {
                IloExpr r11_l(env);
                for (int i = 0; i < input.num_truck_nodes + 1; i++)
                {
                    for (int j = 0; j < input.num_truck_nodes + 1; j++)
                    {
                        r11_l += y[i][j][v][w];
                    }
                }
                model.add(r11_l >= h[v][w]);
            }
        }

        // 12.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                model.add(input.truck_graph[i][j] * x[i][j] <= D[i][j]);
            }
        }

        // 13.
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                IloExpr r13_l(env);
                for (int v = 0; v < input.num_nodes; v++)
                {
                    for (int w = 0; w < input.num_nodes; w++)
                    {
                        r13_l += input.drone_graph[v][w] * y[i][j][v][w];
                    }
                }
                model.add(r13_l <= D[i][j]);
            }
        }

        // 14.
        IloExpr r14_l(env);
        for (int i = 0; i < input.num_truck_nodes + 1; i++)
        {
            for (int j = 0; j < input.num_truck_nodes + 1; j++)
            {
                r14_l += D[i][j];
            }
        }
        model.add(r14_l <= input.d);

        // Modelo
        IloCplex cplex(model);

        // 9. TODO: Ver como adiciona lazy constraints
        cplex.use(ConnectivityCallback(env, y, input));

        cplex.setParam(IloCplex::Param::TimeLimit, 1024.0);
        cplex.setParam(IloCplex::Param::Threads, 6);

        cplex.solve();
        printCplex(cplex);
    }
    catch (IloException &e)
    {
        std::cerr << "Rolou um erro: ";
        std::cerr << e << std::endl;
    }

    env.end();
    std::cout.rdbuf(bufferOriginal);
    return 0;
}

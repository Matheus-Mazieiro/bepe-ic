import csv
import os

def mochila_fracionaria(g_drone, g_profit, t):
    profitable_edges = []  # (densidade, custo, valor)

    for i, _ in enumerate(g_drone):
        for j, _ in enumerate(g_drone[i]):
            valor = float(g_profit[i][j])
            custo = float(g_drone[i][j])
            if valor > 0 and custo > 0:
                densidade = valor / custo
                profitable_edges.append([densidade, custo, valor])

    # Ordena por densidade decrescente
    profitable_edges.sort(key=lambda x: x[0], reverse=True)

    total_valor = 0.0
    total_custo = 0.0

    for densidade, custo, valor in profitable_edges:
        if total_custo + custo <= float(t):
            # Cabe inteiro
            total_custo += custo
            total_valor += valor
        else:
            # Cabe apenas fração
            restante = float(t) - total_custo
            if restante > 0:
                frac = restante / custo
                total_valor += valor * frac
                total_custo += custo * frac
            break  # atingiu capacidade total

    #print(f"Valor total: {total_valor:.2f}")
    #print(f"Custo total usado: {total_custo:.2f} / {int(t):.2f}")

    return total_valor



if __name__ == "__main__":
    input_path = input(f'pasta de arquivos: ')
    files = os.listdir(input_path)

    with open('instances/instance-infos.csv', 'w') as f:
        f.write('instancia, m, n, t, d, E_profit, V_profit, E_proftible_cost, FKP\n')

        for file in files:
            with open(f'{input_path}/{file}', 'r') as entrada:
                linhas = entrada.readlines()
            
            instance = dict()
            instance['name'] = file
            instance['m'] = linhas[0].strip().split(' ')[1]
            instance['n'] = linhas[1].strip().split(' ')[1]
            instance['depot'] = linhas[2].strip().split(' ')[1:]
            instance['t_nodes'] = []
            instance['d_nodes'] = []
            for l in linhas[3:3 + int(instance['m'])]:
                instance['t_nodes'].append(l.strip().split(' ')[1:])
            for l in linhas[3 + int(instance['m']) : 3 + int(instance['m']) + int(instance['n'])]:
                instance['d_nodes'].append(l.strip().split(' ')[1:])
            instance['t_graph'] = []
            for l in linhas[3 + int(instance['m']) + int(instance['n']) + 1 : 3 + 2 * int(instance['m']) + int(instance['n']) + 1]:
                instance['t_graph'].append(l.strip().split())

            instance['t'] = linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 1].split(' ')[1].strip()
            instance['d'] = linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 2].split(' ')[1].strip()


            instance['d_graph'] = []
            for l in linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 4 : 3 + 3 * int(instance['m']) + 2 * int(instance['n']) + 1 + 5]:
                instance['d_graph'].append(l.strip().split())

            instance['profit_graph'] = []
            for l in linhas[ 3 + 3 * int(instance['m']) + 2 * int(instance['n']) + 1 + 6 :  3 + 4 * int(instance['m']) + 3 * int(instance['n']) + 1 + 7]:
                instance['profit_graph'].append(l.strip().split())

            total_nodes_profit = 0
            for x, y, p in instance['d_nodes']:
                #print(p)
                total_nodes_profit += int(p)

            total_edges_profit = 0
            total_profit_edges_cost = 0
            for i, val_i in enumerate(instance['profit_graph']):
                for j, val_ij in enumerate(val_i):
                    total_edges_profit += int(val_ij)
                    if int(val_ij) > 0:
                        total_profit_edges_cost += float(instance['d_graph'][i][j])

            print(f'Instancia: {instance["name"]}')
            print(f'm: {instance["m"]}')
            print(f'n: {instance["n"]}')
            print(f'Profit das arestas: {total_edges_profit}')
            print(f'Profit dos vértices: {total_nodes_profit}')
            print(f'Custo de passar por todas as arestas com profit: {total_profit_edges_cost}')
            profit_frac = mochila_fracionaria(instance['d_graph'], instance['profit_graph'], instance['d'])
            print(f'Profit Mochila fracionária: {profit_frac}')
            f.write(f'{instance["name"]}, {instance["m"]}, {instance["n"]}, {instance["t"]}, {instance["d"]}, {total_edges_profit}, {total_nodes_profit}, {total_profit_edges_cost}, {profit_frac}\n')

#./instances/created-instances
#./instances/realistic_instances
import csv
import os

def mochila_fracionaria(g_drone, g_profit, t):
    #TODO
    print('NOT YET IMPLEMENTED')

if __name__ == "__main__":
    input_path = input(f'pasta de arquivos: ')
    files = os.listdir(input_path)

    with open('instances/instance-infos.csv', 'w') as f:
        f.write('instancia, m, n, t, d, E_profit, V_profit, E_proftible_cost\n')

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

            instance['t'] = linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 1].split(' ')[1]
            instance['d'] = linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 2].split(' ')[1]


            instance['d_graph'] = []
            for l in linhas[3 + 2 * int(instance['m']) + int(instance['n']) + 1 + 4 : 3 + 3 * int(instance['m']) + 2 * int(instance['n']) + 1 + 5]:
                instance['d_graph'].append(l.strip().split())

            instance['profit_graph'] = []
            for l in linhas[ 3 + 3 * int(instance['m']) + 2 * int(instance['n']) + 1 + 6 :  3 + 4 * int(instance['m']) + 3 * int(instance['n']) + 1 + 7]:
                instance['profit_graph'].append(l.strip().split())

            total_nodes_profit = 0
            for x, y, p in instance['d_nodes']:
                print(p)
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
            f.write(f'{instance["name"]}, {instance["m"]}, {instance["n"]}, {instance["t"]}, {instance["d"]}, {total_edges_profit}, {total_nodes_profit}, {total_profit_edges_cost}\n')



#./instances/created-instances
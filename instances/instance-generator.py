import os
import random

def EuclideanDistance(i, j):
    return ((i[0] - j[0])**2 + (i[1] - j[1])**2)**0.5


m = int(input('Number of truck only vertices: '))
n = int(input('Number of drone only vertices: '))
alpha = float(input('Detour alpha value: ')) # Valor do detour feito pelo caminhao
beta = float(input('Probability of type-2 drone link: ')) # Probabilidade de um drone link ser do tipo 2 (ter road)
gamma = float(input('Bendig ratio: '))
c = int(input('T_max Coefficient: '))
seed = int(input('Seed: '))

random.seed(seed)

instancePath = f'./small-instances/mm-{m}-{n}-{seed}.cdop'

os.makedirs(os.path.dirname(instancePath), exist_ok=True)

print(f'\nGenerating instance and saving on {instancePath}')


nodes = []
with open(instancePath, 'w') as file:
    file.write(f'm {m}\n')
    file.write(f'n {n}\n')
    for i in range(m + n + 1):
        x = random.randint(-25, 25) # pedi diametro de brisbane pro chat gp (em km)
        y = random.randint(-20, 20) 
        nodes.append([x, y])
        if i == 0:
            file.write(f'{i} {x} {y}\n')
        elif i <= m:
            file.write(f't {x} {y}\n')
        else:
            profit = random.randint(0, 100)
            file.write(f'd {x} {y} {profit}\n')

    truck_graph = [[0 for _ in range(m + 1)] for _ in range(m + 1)]
    for i in range(m + 1):
        for j in range(i + 1, m + 1):
            truck_graph[i][j] = EuclideanDistance(nodes[i], nodes[j]) * alpha
            truck_graph[j][i] = truck_graph[i][j]

    file.write(f'Truck Graph:\n')
    for row in truck_graph:
        file.write(' '.join(f'{val:7.2f}' for val in row) + '\n')

    X_min = nodes[0][0]
    X_max = nodes[0][0]
    Y_min = nodes[0][0]
    Y_max = nodes[0][0]
    for x, y in nodes:
        X_min = min(x, X_min)
        X_max = max(x, X_max)
        Y_min = min(y, Y_min)
        Y_max = max(y, Y_max)
    T_max = c * (X_max - X_min + Y_max - Y_min)
    #T_max = random.randint(12, 15) # Autonomia de um drone médio conforme chatgpt
    file.write(f'T {T_max}\n')
    
    D = 2 * c * (X_max - X_min + Y_max - Y_min)
    file.write(f'D {D}\n')

    drone_distance_graph = [[0 for _ in range(n + m + 1)] for _ in range(n + m + 1)]
    drone_profit_graph = [[0 for _ in range(n + m + 1)] for _ in range(n + m + 1)]
    for i in range(n + m + 1):
        for j in range(i + 1, n + m + 1):
            type2_link = random.uniform(0, 1)
            if type2_link <= beta:
                drone_distance_graph[i][j] = EuclideanDistance(nodes[i], nodes[j]) * gamma
                drone_distance_graph[j][i] = drone_distance_graph[i][j]

                drone_profit_graph[i][j] = random.randint(0, 100)
                drone_profit_graph[j][i] = drone_profit_graph[i][j]
            else:
                drone_distance_graph[i][j] = EuclideanDistance(nodes[i], nodes[j])
                drone_distance_graph[j][i] = drone_distance_graph[i][j]


    file.write(f'Drone Distance Graph:\n')
    for row in drone_distance_graph:
        file.write(' '.join(f'{val:7.2f}' for val in row) + '\n')
    file.write(f'Drone Profit Graph:\n')
    for row in drone_profit_graph:
        file.write(' '.join(f'{val:2d}' for val in row) + '\n')



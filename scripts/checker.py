import os

VERBOSE = 0

def get_input(input_file):
    # print(input_file)
    with open(input_file, 'r') as f:
        linhas = f.readlines()

    truck_graph = []
    drone_graph = []
    drone_profits_graph = []
    for i, l in enumerate(linhas):
        if l[0] == 'm':
            m = int(l.split(' ')[1].strip())
        if l[0] == 'n':
            n = int(l.split(' ')[1].strip())
            drone_profits_nodes = [0] * (m + n + 1)
        if l[0] == 'd':
            drone_profits_nodes[i - 2] = int(l.split(' ')[3].strip())
        if 'Truck Graph:' in l:
            for j in range(i + 1, i + m + 2):
                truck_graph.append([float(v.strip()) for v in linhas[j].split()])
        if l.split(' ')[0].strip() == 'T':
            t_max = int(l.split(' ')[1].strip())
        if l.split(' ')[0].strip() == 'D':
            D = int(l.split(' ')[1].strip())
        if 'Drone Distance Graph:' in l:
            for j in range(i + 1, i + m + n + 2):
                drone_graph.append([float(v.strip()) for v in linhas[j].split()])
        if 'Drone Profit Graph:' in l:
            for j in range(i + 1, i + m + n + 2):
                drone_profits_graph.append([float(v.strip()) for v in linhas[j].split()])
    
    input_object = {
        'm': m,
        'n': n,
        't_max': t_max,
        'D': D,
        'truck_graph': truck_graph,
        'drone_graph': drone_graph,
        'drone_profits_graph': drone_profits_graph,
        'drone_profits_nodes': drone_profits_nodes
    }
    return input_object

def check_solution(sol_file):

    tour = []
    with open(sol_file, 'r') as f:
        linhas = f.readlines()
    
    if len(linhas) == 0:
        print(f'Arquivo vazio: {sol_file}')
        quit(0)

    tour_start = -1
    value = 0
    time = 0
    for i, linha in enumerate(linhas):
        if 'Solution tour: ' in linha:
            tour_start = i + 1
        if 'File name:' in linha:
            input_obj = get_input(linha.split(':')[1].strip())
        if 'Valor:' in linha:
            value = float(linha.split(':')[1].strip())
        if 'Time:' in linha:
            time = int(linha.split(':')[1].strip())
    m = input_obj['m']
    n = input_obj['n']

    # print(input_obj)

    index = tour_start
    while('=' not in linhas[index]):
        for j, c in enumerate(linhas[index].split()):
            if c == '\n' or c == ' ':
                continue
            tour.append([int(c), j == 0])
        index += 1

    adj_mat = [[0 for _ in range(m + n + 1)] for _ in range(m + n + 1)] # mudar para ser do tamanho da entrada
    adj_array = [0 for _ in range(m + n + 1)]
    for idx, [node, sync] in enumerate(tour):
        adj_mat[tour[idx][0]][tour[(idx + 1) % len(tour)][0]] += 1
        adj_array[tour[idx][0]] += 1

    for t in tour:
        if t[0] >= m + 1 and t[1] == True:
            print('Drone vertice sincronizando')
            return 1

    truck_vertices_count = [0] * (m + 1)
    for t in tour:
        if t[1]:
            truck_vertices_count[t[0]] += 1
            if truck_vertices_count[t[0]] > 1:
                print('Sync on repeated node')
                return 1

    objective_value = 0
    for i in range(len(adj_mat)):
        for j in range(len(adj_mat[i])):
            if adj_mat[i][j] > 0:
                objective_value += input_obj['drone_profits_graph'][i][j]
    for i, t in enumerate(adj_array):
        if t > 0:
            objective_value += input_obj['drone_profits_nodes'][i]

    if value - objective_value >= 0.01:
        print(f'Different value on tour ({sol_file}) ({value} != {objective_value})')
        return 1
    elif VERBOSE: 
        print(f'Comparing objective vaule ({sol_file}) ({value} =?= {objective_value})')

    drone_flight_duration = 0
    total_time_operation = 0
    last_sync_node = 0
    for i, t in enumerate(tour):
        u = tour[i]
        v = tour[(i + 1) % len(tour)]
        drone_flight_duration += input_obj['drone_graph'][u[0]][v[0]]
        if v[1]:
            if drone_flight_duration > input_obj['t_max']:
                print(f'Impossible drone flight ({sol_file} )')
                return 1
            elif VERBOSE: 
                print(f'Comparing drone flight duration ({sol_file}) ({drone_flight_duration} =?= {input_obj["t_max"]})')
            total_time_operation += max(drone_flight_duration, input_obj['truck_graph'][last_sync_node][v[0]])
            # print(f'drone operations lengths {max(drone_flight_duration, input_obj["truck_graph"][last_sync_node][v[0]])}', end='')
            #if drone_flight_duration > input_obj["truck_graph"][last_sync_node][v[0]]:
            #    print(f'drone')
            #else:
            #    print(f'truck')
            drone_flight_duration = 0
            last_sync_node = v[0]
    
    if total_time_operation > input_obj['D']:
        print(f"Violated max time operation {total_time_operation}/{input_obj['D']} ({sol_file} )")
        return 1
    elif VERBOSE:
        print(f"Comparing max time operation ({sol_file}) ({total_time_operation} =?= {input_obj['D']})")

    if abs(time - 1024000) >= 1000:
        print(f"{sol_file} runned for {time/1000}s, not for 1024s")
    return 0

if __name__ == "__main__":
    input_path = 'Output/'
    #mm-5-10-1917698_Memetic-LS_MPX_1881515.out'
    files = os.listdir(input_path)
    sum = 0
    total = 0
    for f in files:
        #if '-LS' not in f:
        sum += check_solution(f'{input_path}{f}')
        total += 1
    print(f'{sum} wrong answers on {total} solutions')
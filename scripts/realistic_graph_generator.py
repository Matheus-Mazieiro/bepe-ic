import random
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
import time
import sys
from collections import defaultdict


mn = [(5, 10), (5, 25), (5, 40), 
      (10, 10), (10, 25), (10, 40),
      (15, 10), (15, 25), (15, 40),
      (20, 10), (20, 25),
      (25, 10), (25, 25)
      ]
seeds = [1110588, 1762875, 1767920, 1891798, 1917698]

def dist(u, v):
    return ((u[2]-v[2])**2 + (u[1]-v[1])**2)**0.5

def mst(pos, dist_func):
    n = len(pos)
    
    in_mst = [False] * n
    key = [float("inf")] * n   # menor custo para conectar cada vértice
    parent = [-1] * n          # pai na MST
    
    key[0] = 0  # começa pelo vértice 0
    
    for _ in range(n):
        # escolhe o vértice fora da MST com menor key
        u = -1
        best = float("inf")
        for i in range(n):
            if not in_mst[i] and key[i] < best:
                best = key[i]
                u = i
        
        in_mst[u] = True
        
        # relaxa as arestas (u, v) para todos os v fora da MST
        for v in range(n):
            if not in_mst[v]:
                w = dist_func(pos[u], pos[v])
                if w < key[v]:
                    key[v] = w
                    parent[v] = u
    
    # monta a matriz de adjacência da MST
    adj = [[0.0] * n for _ in range(n)]
    
    for v in range(1, n):
        u = parent[v]
        w = dist_func(pos[u], pos[v])
        adj[u][v] = w
        adj[v][u] = w
    
    return adj

def has_path(edges, start, target):
    """
    Verifica se existe caminho de start até target usando DFS
    edges: lista de arestas (u, v, w)
    """
    # Construir lista de adjacência
    adj = defaultdict(list)
    for u, v, w in edges:
        adj[u].append(v)
        adj[v].append(u)  # grafo não-direcionado

    # DFS
    stack = [start]
    visited = set()

    while stack:
        node = stack.pop()
        if node == target:
            return True
        if node not in visited:
            visited.add(node)
            stack.extend(adj[node])

    return False

def BombProposal1(vertices, edges, m, n, prob):
    truck_tree = [] 

    stack = [(0, 0)]
    visited = set()

    while stack and len(truck_tree) <= m:
        node_orig, node_dest = stack.pop(random.randint(0, len(stack) - 1))

        if node_dest not in visited:
            truck_tree.append((node_orig, node_dest))
            visited.add(node_dest)
            for i in range(m+n+1):
                if edges[(node_dest, i)] > 0:
                    stack.append((node_dest, i))


    vert = []
    for i in range(len(truck_tree)):
        u, v = truck_tree[i]
        vert.append(vertices[v])
    for v in vertices:
        if v not in vert:
            vert.append(v)
    
    vertices[:] = vert
    truck_tree_remapped = []
    for tt in truck_tree:
        u, v = tt
        u_l = -1
        v_l = -1
        for i, vertice in enumerate(vertices):
            if vertice[0] == u:
                u_l = i
            if vertice[0] == v:
                v_l = i
        truck_tree_remapped.append((u_l, v_l))

    old_new_map = dict()
    for i, vertice in enumerate(vert):
        old_new_map[vertice[0]] = i

    new_edges = edges.copy()
    for i in range(len(edges)):
        for j in range(len(edges)):
            new_edges[old_new_map[i], old_new_map[j]] = edges[i, j]
    edges[:] = new_edges

    for i in range(len(vertices)):
        vertices[i] = (i, vertices[i][1], vertices[i][2])

    graph = np.zeros((m+n+1, m+n+1))
    for u, v in truck_tree_remapped:
        graph[u, v] = 1
        graph[v, u] = 1

    for i in range(m + 1):
        for j in range(m+1):
            graph[i, j] = graph[i, j] or ((edges[i, j] != 0) * (random.uniform(0, 1) < prob))

    for i in range(m + 1, len(edges)):
        for j in range(m+1, len(edges)):
            graph[i, j] = (edges[i, j] != 0) * (random.uniform(0, 1) < prob)
    return graph

# alpha quao reta é a rua
def truck_network(vertices, edges, m, n, alpha):
    size = m+n+1
    tn = edges.copy()

    for i in range(size):
        for j in range(size):
            if tn[i, j]:
                tn[i, j] = alpha * dist(vertices[i], vertices[j])

    for i in range(size):
        for j in range(size):
            if tn[i, j] == 0:
                tn[i, j] = float('inf')
        tn[i,i] = 0

    # Ffloyd-Warshall
    for k in range(size):
        for i in range(size):
            for j in range(size):
                tn[i][j] = min(tn[i, j], tn[i, k] + tn[k, j])

    return tn[:m+1, :m+1]

# gamma é o quanto de volta aquela rua dá
def drone_network(vertices, edges_city, edges_survived, m, n, gamma):
    size = m + n + 1
    dn = np.zeros((size, size), dtype=float)
    dp = np.zeros((size, size), dtype=int)

    # drone network
    for i in range(size):
        for j in range(size):
            # existe road
            if edges_city[i, j]:
                dn[i][j] = gamma * dist(vertices[i], vertices[j])
            # nao existe road
            else:
                dn[i][j] = dist(vertices[i], vertices[j])

    # drone profit
    for i in range(size):
        for j in range(i + 1, size):
            dp[i, j] = 0
            dp[j, i] = 0
            # existe road
            if edges_city[i, j] and not edges_survived[i, j]:
                    road_profit = int(random.uniform(0, 1) * dn[i,j])
                    dp[i, j] = road_profit
                    dp[j, i] = road_profit


    return dn, dp

def print_network(network, t):
    for i in range(network.shape[0]):
        for j in range(network.shape[1]):
            if t is float:
                print(f'{network[i, j]:6.2f}', end=' ')
            else:
                print(f'{network[i, j]:4d}', end=' ')
        print()

if __name__ == '__main__':
    for seed in seeds:
        random.seed(seed)

        for m, n in mn:
            instance = f'instances/realistic-instances/{m}-{n}-{seed}.cdop'
            with open(instance, "w") as f:
                print(f"generating {instance}")
                old_stdout = sys.stdout
                sys.stdout = f

                pos = []
                for i in range(m+n+1):
                    x = random.randint(0, 100)
                    y = random.randint(0, 100)
                    pos.append((i, x, y))

                g = mst(pos, dist)
                g = np.array(g).astype(float)

                g = np.matrix(g)


                edge_list = []

                for u in range(len(pos)):
                    for v in range(u+1, len(pos)):
                        edge_list.append((dist(pos[u], pos[v]), u, v))

                edge_list = sorted(edge_list)

                edge_count = y+x
                max_edge_count = 2 #random.uniform(1.5, 2.5)
                max_degree = [0] * (m+n+1)
                for i in range(len(max_degree)):
                    max_degree[i] = random.randint(2, 6)

                for d, u, v in edge_list:
                    if g[u, v] == 0 and edge_count < max_edge_count*(y+x+1) and ((g[u, :] != 0).sum() < max_degree[u]) and ((g[:, v] != 0).sum() < max_degree[v]):
                        g[u, v] = d
                        g[v, u] = d
                        edge_count+=1
                # Printar isso aqui no fim
                #print(f'|edge_list| == {len(edge_list)}')
                #print(f'max number of edges == {max_edge_count*(y+x+1)}')
                #print(f'edge_count == {edge_count}')
                #print(f'max_degree == {max_degree}')



                #print(f'==============================================')
                g = np.matrix(g)
                g_bin = (g != 0).astype(int)

                prop1 = BombProposal1(pos, g_bin, m, n, 0.8)



                # Só imprimir e partir para o abraço
                print(f'm {m}')
                print(f'n {n}')
                print(f'0 {pos[0][1]} {pos[0][2]}')
                max_x = pos[0][1]
                min_x = pos[0][1]
                max_y = pos[0][2]
                min_y = pos[0][2]
                for i in range(1, m+1):
                    print(f't {pos[i][1]} {pos[i][2]}')
                    if pos[i][1] < min_x:
                        min_x = pos[i][1]
                    if pos[i][1] > max_x:
                        max_x = pos[i][1]
                    if pos[i][2] < min_y:
                        min_y = pos[i][2]
                    if pos[i][2] > max_y:
                        max_y = pos[i][2]

                for i in range(m + 1, m + n + 1):
                    vert_profit = random.randint(1, 100)
                    print(f'd {pos[i][1]} {pos[i][2]} {vert_profit}')
                    if pos[i][1] < min_x:
                        min_x = pos[i][1]
                    if pos[i][1] > max_x:
                        max_x = pos[i][1]
                    if pos[i][2] < min_y:
                        min_y = pos[i][2]
                    if pos[i][2] > max_y:
                        max_y = pos[i][2]
                print("Truck Graph:")
                tn = truck_network(pos, prop1, m, n, 1)
                print_network(tn, float)
                T_max = 5 * (max_x - min_x + max_y - min_y)
                print(f'T {T_max}')
                print(f'D {2 * T_max}')
                dn, dp = drone_network(pos, g, prop1, m, n, 1.5)
                print("Drone Distance Graph:")
                print_network(dn, float)
                print("Drone Profit Graph:")
                print_network(dp, int)
                
                sys.stdout = old_stdout

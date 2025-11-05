import csv
import os
from collections import defaultdict

# ====== Algorithm Status ======
# Valor:                5528
# Time:                 1024020
# nIter:                -1
# Best Iter:            -1
# Improvement Count:    0
# ==============================

solving_tipe = 'Memetic'#Memetic, Exact

def read_data(sol_file):
    print(sol_file)

    instance = sol_file.split('/')[1].split(f'_{solving_tipe}')[0]
    settings = solving_tipe + sol_file.split(f'_{solving_tipe}')[1].split('.')[0]
    m = instance.split('-')[1]
    n = instance.split('-')[2]   
    value = -1         

    with open(sol_file, 'r') as f:
        linhas = f.readlines()

    for i, linha in enumerate(linhas):
        if 'Algorithm Status' in linha:
            value = linhas[i+1].split(':')[1].strip()
            time = linhas[i+2].split(':')[1].strip()
            nIter = linhas[i+3].split(':')[1].strip()
            best_iter = linhas[i+4].split(':')[1].strip()
            improv_count = linhas[i+5].split(':')[1].strip()

    if value != -1:
        obj = {
            'instance': instance,
            'settings': settings,
            'm': int(m),
            'n': int(n),
            'value': float(value),
            'time': float(time),
            'nIter': int(nIter),
            'best_iter': int(best_iter),
            'improv_count': int(improv_count)
        }
    else:
        obj = {
            'instance': instance,
            'settings': settings,
            'm': int(m),
            'n': int(n),
            'value': float(-1),
            'time': float(-1),
            'nIter': int(-1),
            'best_iter': int(-1),
            'improv_count': int(-1)
        }
    return obj

def add_to_csv(csv_file, obj):
    with open(csv_file, 'a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=obj.keys())
        if not os.path.exists(csv_file) or os.stat(csv_file).st_size == 0:
            writer.writeheader()
        writer.writerow(obj)

def agrupar_por_instancia_e_prefixo(objetos):
    grupos = defaultdict(list)

    for obj in objetos:
        settings = obj['settings']
        prefixo = settings.rsplit('_', 1)[0] if '_' in settings else settings
        chave = (obj['instance'], prefixo)
        grupos[chave].append(obj)

    resultados = []
    for (instance, prefixo), grupo in grupos.items():
        media = lambda k: sum(o[k] for o in grupo) / len(grupo)
        resultados.append({
            'instance': instance,
            'settings_prefix': prefixo,
            'm': media('m'),
            'n': media('n'),
            'value': media('value'),
            'time': media('time'),
            'nIter': media('nIter'),
            'best_iter': media('best_iter'),
            'improv_count': media('improv_count')
        })

    return resultados

def agrupar_por_prefixos(objetos):
    grupos = defaultdict(list)

    for obj in objetos:
        prefix_instance = obj['instance'].rsplit('-', 1)[0] if '-' in obj['instance'] else obj['instance']
        prefix_settings = obj['settings'].rsplit('_', 1)[0] if '_' in obj['settings'] else obj['settings']
        chave = (prefix_instance, prefix_settings)
        if obj['value'] >= 0:
            grupos[chave].append(obj)
        else: 
            print(f"unsolved at {obj['instance']}")

    resultados = []
    for (prefix_instance, prefix_settings), grupo in grupos.items():
        media = lambda k: sum(o[k] for o in grupo) / len(grupo)
        resultados.append({
            'instance_prefix': prefix_instance,
            'settings_prefix': prefix_settings,
            'm': media('m'),
            'n': media('n'),
            'value': media('value'),
            'time': media('time'),
            'nIter': media('nIter'),
            'best_iter': media('best_iter'),
            'improv_count': media('improv_count'),
            'solved': len(grupo)
        })

    return resultados

if __name__ == "__main__":
    input_path = input(f'pasta de arquivos: ')
    files = os.listdir(input_path)
    objs = []
    for file in files:
        print(f'{input_path}/{file}')
        objs.append(read_data(f'{input_path}/{file}'))

#    for obj in objs:
#        add_to_csv(f'csv-ma.csv', obj)

    media = agrupar_por_instancia_e_prefixo(objs)
    for m in media:
        add_to_csv('csv-ma-media.csv', m)

    media = agrupar_por_prefixos(objs)
    for m in media:
        add_to_csv('csv-ma-media-final.csv', m)


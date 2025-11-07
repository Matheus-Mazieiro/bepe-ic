#!/usr/bin/env python3
import os
import re
import csv
import sys
import glob
from collections import defaultdict
from statistics import mean

def find_section(text, section_name):
    """Retorna substring do início da seção até a próxima linha com muitos '=' ou EOF."""
    # tenta encontrar 'Random Pocket', 'Random Current', 'Warm Start' etc.
    idx = text.find(section_name)
    if idx == -1:
        return ""
    # procurar próximo header de ===== ou próximo section marker
    m = re.search(r'\n=+\s*[\w ]+?=+\n', text[idx+1:])
    if m:
        end = idx + 1 + m.start()
        return text[idx:end]
    else:
        return text[idx:]

def parse_file(path):
    """Retorna um dicionário com os campos extraídos do arquivo path."""
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        text = f.read()

    # Instance (do campo File name:)
    m = re.search(r'File name:\s*(.+)', text)
    instance = None
    if m:
        fname = m.group(1).strip()
        instance = os.path.splitext(os.path.basename(fname))[0]
    else:
        # fallback: tentar extrair do nome do arquivo após WS_
        bn = os.path.basename(path)
        m2 = re.match(r'WS_(.+?)_', bn)
        if m2:
            instance = m2.group(1)
        else:
            instance = os.path.splitext(bn)[0]

    # Settings (do campo Settings file:)
    m = re.search(r'Settings file:\s*(.+)', text)
    settings = None
    if m:
        settings_fname = os.path.basename(m.group(1).strip())
        settings_noext = os.path.splitext(settings_fname)[0]
        # remover suffix numérico (seed) se houver, ex: Memetic_MMX_1099666 -> Memetic_MMX
        settings = re.sub(r'_(\d+)$', '', settings_noext)
    else:
        # fallback: tenta extrair do nome do arquivo (entre underscores, depois da instância)
        bn = os.path.basename(path)
        rest = bn[len("WS_"):] if bn.startswith("WS_") else bn
        parts = rest.split('_')
        if len(parts) >= 2:
            # junta partes do meio menos o último que pode ser seed/out
            settings_candidate = '_'.join(parts[1:-1]) or parts[1]
            settings = re.sub(r'\.out$', '', settings_candidate)
        else:
            settings = 'UNKNOWN'

    # Truck / Drone nodes
    truck = None
    drone = None
    m = re.search(r'Truck node count:\s*(\d+)', text)
    if m:
        truck = int(m.group(1))
    m = re.search(r'Drone node count:\s*(\d+)', text)
    if m:
        drone = int(m.group(1))

    # RANDOM POCKET
    pocket_section = find_section(text, 'Random Pocket')
    pocket_val = None
    m = re.search(r'Valor Pocket:\s*([+-]?\d+(\.\d+)?)', pocket_section)
    if m:
        pocket_val = float(m.group(1))

    # RANDOM CURRENT
    current_section = find_section(text, 'Random Current')
    current_val = None
    m = re.search(r'Valor Current:\s*([+-]?\d+(\.\d+)?)', current_section)
    if m:
        current_val = float(m.group(1))

    # WARM START (preciso pegar o Valor dentro da seção Warm Start e o Time(µs))
    warm_section = find_section(text, 'Warm Start')
    warm_val = None
    warm_time = None
    # Value sometimes shown as 'Valor:' inside the warm section
    m = re.search(r'Valor:\s*([+-]?\d+(\.\d+)?)', warm_section)
    if m:
        warm_val = float(m.group(1))
    # Time may be "Time(µs):" or "Time(us):"
    m = re.search(r'Time\(?µs\)?:\s*([+-]?\d+(\.\d+)?)', warm_section) or re.search(r'Time\(?us\)?:\s*([+-]?\d+(\.\d+)?)', warm_section)
    if m:
        warm_time = float(m.group(1))

    return {
        'file_path': path,
        'instance': instance,
        'settings': settings,
        'truck_nodes': truck,
        'drone_nodes': drone,
        'pocket_val': pocket_val,
        'current_val': current_val,
        'warm_val': warm_val,
        'warm_time_us': warm_time
    }

def aggregate(rows):
    """Agrupa por (instance, settings) e calcula médias conforme solicitado."""
    groups = defaultdict(list)
    for r in rows:
        key = (r['instance'], r['settings'])
        groups[key].append(r)

    results = []
    for (instance, settings), items in groups.items():
        # truck/drone: pode haver variação, pegamos o valor modal/mais comum ou o primeiro não-nulo
        truck_vals = [it['truck_nodes'] for it in items if it['truck_nodes'] is not None]
        drone_vals = [it['drone_nodes'] for it in items if it['drone_nodes'] is not None]
        truck = truck_vals[0] if truck_vals else None
        drone = drone_vals[0] if drone_vals else None

        # médias ignorando None
        pocket_vals = [it['pocket_val'] for it in items if it['pocket_val'] is not None]
        current_vals = [it['current_val'] for it in items if it['current_val'] is not None]
        warm_vals = [it['warm_val'] for it in items if it['warm_val'] is not None]
        warm_times = [it['warm_time_us'] for it in items if it['warm_time_us'] is not None]

        result = {
            'instance': instance,
            'settings': settings,
            'drone_nodes': drone if drone is not None else '',
            'truck_nodes': truck if truck is not None else '',
            'avg_random_pocket': mean(pocket_vals) if pocket_vals else '',
            'avg_random_current': mean(current_vals) if current_vals else '',
            'avg_warmstart_value': mean(warm_vals) if warm_vals else '',
            'avg_warmstart_time_us': mean(warm_times) if warm_times else '',
            'samples': len(items)
        }
        results.append(result)
    return results

def write_csv(results, out_csv):
    fieldnames = [
        'instance', 'settings', 'drone_nodes', 'truck_nodes',
        'avg_random_pocket', 'avg_random_current',
        'avg_warmstart_value', 'avg_warmstart_time_us', 'samples'
    ]
    with open(out_csv, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for r in results:
            writer.writerow(r)

def main():
    if len(sys.argv) >= 2:
        input_dir = sys.argv[1]
    else:
        input_dir = input("Pasta com arquivos (ex: ./results): ").strip()
    if not input_dir:
        print("Diretório inválido.")
        return

    # procurar recursivamente por arquivos WS_*.out
    pattern = os.path.join(input_dir, '**', 'WS_*.out')
    files = glob.glob(pattern, recursive=True)
    if not files:
        print("Nenhum arquivo WS_*.out encontrado em", input_dir)
        return

    rows = []
    for fpath in sorted(files):
        try:
            parsed = parse_file(fpath)
            rows.append(parsed)
        except Exception as e:
            print("Erro ao parsear", fpath, ":", e)

    aggregated = aggregate(rows)

    out_csv = os.path.join(input_dir, 'ws_summary_by_instance_settings.csv')
    write_csv(aggregated, out_csv)
    print("CSV gerado em:", out_csv)
    print(f"{len(aggregated)} grupos (instance,settings) escritos. {len(rows)} arquivos processados.")

if __name__ == "__main__":
    main()

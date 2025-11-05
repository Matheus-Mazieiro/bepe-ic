import os
import csv

path = 'Output/'
out_file = 'evolution_through_time'

# Step 1: Collect best final value for each instance
best_value_per_instance = {}

for file in os.listdir(path):
    with open(os.path.join(path, file), 'r') as f:
        lines = f.readlines()
        instance = ''
        values = []
        for line in lines:
            if 'File name:' in line:
                instance = line.split(':')[1].strip()
            if 'Valor:' in line:
                values.append(float(line.split(':')[1].strip()))
        if instance and values:
            best_val = values[-1]
            if instance not in best_value_per_instance or best_val > best_value_per_instance[instance]:
                best_value_per_instance[instance] = best_val

# Step 2: Now parse again and generate both raw and normalized CSVs
def add_to_csv(csv_file, obj, fieldnames):
    file_exists = os.path.exists(csv_file)
    with open(csv_file, 'a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        if not file_exists or os.stat(csv_file).st_size == 0:
            writer.writeheader()
        writer.writerow(obj)

for file in os.listdir(path):
    obj = {
        'instance': '',
        'setting': '',
        'm': 0,
        'n': 0,
        'values': []
    }

    with open(os.path.join(path, file), 'r') as f:
        lines = f.readlines()
        for line in lines:
            if 'Truck node count:' in line:
                obj['m'] = line.split(':')[1].strip()
            if 'Drone node count:' in line:
                obj['n'] = line.split(':')[1].strip()
            if 'File name:' in line:
                obj['instance'] = line.split(':')[1].strip()
            if 'Settings file:' in line:
                obj['setting'] = line.split(':')[1].strip()
            if 'Valor:' in line:
                obj['values'].append(float(line.split(':')[1].strip()))

    best_val = best_value_per_instance.get(obj['instance'], 1.0)

    normalized_values = [x / best_val for x in obj['values']]
    num_values = len(obj['values'])
    time_columns = [f'{0.25 * (2 ** i)}s' for i in range(num_values)]
    fieldnames = ['instance', 'setting', 'm', 'n'] + time_columns

    row_raw = {
        'instance': obj['instance'],
        'setting': obj['setting'],
        'm': obj['m'],
        'n': obj['n'],
    }
    row_raw.update({t: v for t, v in zip(time_columns, obj['values'])})

    row_norm = {
        'instance': obj['instance'],
        'setting': obj['setting'],
        'm': obj['m'],
        'n': obj['n'],
    }
    row_norm.update({t: v for t, v in zip(time_columns, normalized_values)})

    add_to_csv(f'{out_file}.csv', row_raw, fieldnames)
    add_to_csv(f'{out_file}_norm.csv', row_norm, fieldnames)

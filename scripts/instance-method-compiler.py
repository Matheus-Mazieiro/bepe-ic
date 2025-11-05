import csv
import os

def instance_method(csv_media, csv_exact):
    objs = []

    with open(csv_media, 'r') as f:
        linhas = f.readlines()
        for linha in linhas[1:]:
            l = linha.split(',')
            resultado = next((obj for obj in objs if obj.get('instance') == l[0]), None)
            if(resultado == None):
                resultado = {
                    'instance': l[0],
                    'm': l[2],
                    'n': l[3],
                }
                objs.append(resultado)
            #else
            resultado[l[1]] = l[4]

    with open(csv_exact) as f:
        linhas = f.readlines()
        for linha in linhas[1:]:
            l = linha.split(',')
            resultado = next((obj for obj in objs if obj.get('instance') == l[0]), None)
            if(resultado == None):
                resultado = {
                    'instance': l[0],
                    'm': l[2],
                    'n': l[3],
                }
                objs.append(resultado)
            #else
            resultado[l[1]] = l[4]

    return objs

def write_to_csv(objs, csv_out):
    print(f'wrinting in {csv_out}')
    with open(csv_out, 'a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=objs[0].keys())
        if not os.path.exists(csv_out) or os.stat(csv_out).st_size == 0:
            writer.writeheader()
        for obj in objs:
            writer.writerow(obj)




if __name__ == "__main__":
    csv_in = f'csv-ma-media.csv'
    csv_exact = f'csv-exact.csv'
    csv_out = f'csv-instance-method.csv'
    objs = instance_method(csv_in, csv_exact)
    write_to_csv(objs, csv_out)
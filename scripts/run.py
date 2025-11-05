import os
import subprocess
from joblib import Parallel, delayed

input_path = './instances/created-instances'
settings_path = './settings'
output_path = ''  # pode colocar './outputs' se quiser redirecionar para uma pasta

input_files = os.listdir(input_path)
setting_files = os.listdir(settings_path)

commands = []
i = 0
for input_file in input_files:
    for setting_file in setting_files:
        output = f'{input_file.split(".")[0]}_{setting_file.split(".")[0]}.out'
        commands.append([i, f'./bepe {input_path}/{input_file} {settings_path}/{setting_file} {output_path}/{output}'])
        #commands.append([i, f'{input_path}/{input_file} {settings_path}/{setting_file} {output_path}/{output}'])
        i += 1

def run(exec_command):
    index, command = exec_command
    print(f'[{index}] Executando: {command}')
    #command = 'make -f Makefile.mak run args=\"' + command + '\"'
    try:
        result = subprocess.run(
            command,
            shell=True,
            capture_output=True,
            text=True,
        )
    except Exception as e:
        return {
            "index": index,
            "command": command,
            "error": f"Exceção ao executar: {e}",
            "stdout": "",
            "stderr": ""
        }

    if result.returncode != 0:
        return {
            "index": index,
            "command": command,
            "error": f"Exit code {result.returncode}",
            "stdout": result.stdout,
            "stderr": result.stderr
        }

    # Se você quiser guardar a saída dos comandos de sucesso também:
    return {
        "index": index,
        "command": command,
        "error": None,
        "stdout": result.stdout,
        "stderr": result.stderr
    }

# Executa os comandos em paralelo
results = Parallel(n_jobs=10)(delayed(run)(cmd) for cmd in commands)

# Separando erros
errors = [r for r in results if r["error"] is not None]

if len(errors) > 0:
    print("\nCOMANDOS COM ERRO:\n")
    for err in errors:
        print(f"[{err['index']}] Comando: {err['command']}")
        print(f"Erro: {err['error']}")
        print("STDERR:")
        print(err["stderr"])
        print("-" * 60)

# (opcional) Mostrar a saída de comandos que funcionaram
successes = [r for r in results if r["error"] is None]
print(f"\n{len(successes)}/{len(results)} comandos executados com sucesso.")

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Estilo limpo
sns.set(style='whitegrid', palette='colorblind')

# Carrega os dados
df = pd.read_csv('evolution_through_time_norm.csv')

# Limpeza
df = df.dropna(axis=1, how='all')
colunas_tempo = [col for col in df.columns if col.endswith('s')]
tempos = [float(col.replace('s', '')) for col in colunas_tempo]

# Agrupamento por prefixo
def extrair_prefixo(path):
    nome = path.split('/')[-1].replace('.config', '')
    return ''.join(nome.split('_')[:-1])

df['prefix'] = df['setting'].apply(extrair_prefixo)
rename_map = {
    "MemeticMMX": r"$\mathrm{MMX_{+LS}}$",
    "Memetic-LSMPX": r"$\mathrm{MPX}$",
    "MemeticMPX": r"$\mathrm{MPX_{+LS}}$",
    "Memetic-LSMMX": r"$\mathrm{MMX}$",
    "Exact": r"$\mathrm{Gurobi}$"
}
df['prefix'] = df['prefix'].replace(rename_map)

df_mean = df.groupby('prefix')[colunas_tempo].mean()
df_std = df.groupby('prefix')[colunas_tempo].std()

# Plot
plt.figure(figsize=(14, 7))

cores = sns.color_palette('colorblind', n_colors=len(df_mean.index))

for i, prefix in enumerate(df_mean.index):
    mean_values = df_mean.loc[prefix].values
    std_values = df_std.loc[prefix].values
    cor = cores[i]

    # Curva com barras de erro verticais
    plt.errorbar(
        tempos, mean_values, yerr=std_values,
        label=prefix,
        color=cor,
        linewidth=2,
        capsize=3,
        fmt='-o',  # linha sólida com marcadores circulares
        markersize=4
    )

    # Anotar o nome ao final da curva
    plt.text(tempos[-1]*1.05, mean_values[-1], prefix, color=cor,
             fontsize=9, verticalalignment='center')

# Ajustes visuais
plt.xscale('log', base=2)
plt.xlabel('Time (s)')
plt.ylabel('Average quality (%)')
plt.title('Average performance over time by configuration group')
plt.grid(True, axis='y')
plt.tight_layout()
plt.xlim(min(tempos), max(tempos)*1.3)
plt.ylim(0, 1.05)

# Salva e exibe
plt.savefig('evolution.pdf')
print('saved at evolution.pdf')

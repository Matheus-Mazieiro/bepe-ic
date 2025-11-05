import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import tikzplotlib  # Importa o tikzplotlib

# Carrega a planilha (ajuste o nome conforme necessário)
df = pd.read_csv('csv-instance-method.csv')

# Seleciona apenas as colunas dos algoritmos
alg_cols = df.columns[3:]  # Ignora 'instance', 'm', 'n'

# Calcula os ranks por linha (instância)
rank_df = -df[alg_cols]
rank_df = rank_df[alg_cols].rank(axis=1, method="min")

# Adiciona a coluna da instância para identificação
rank_df["instance"] = df["instance"]

# Converte para formato longo
long_df = pd.melt(rank_df, id_vars="instance", var_name="Algoritmo", value_name="Rank")

rename_map = {
    "Memetic_MMX": r"$\mathrm{MMX_{+LS}}$",
    "Memetic-LS_MPX": r"$\mathrm{MPX}$",
    "Memetic_MPX": r"$\mathrm{MPX_{+LS}}$",
    "Memetic-LS_MMX": r"$\mathrm{MMX}$",
    "Exact": r"$\mathrm{Gurobi}$"
}
long_df["Algoritmo"] = long_df["Algoritmo"].replace(rename_map)


# Cria o gráfico
#plt.figure(figsize=(5, 4))
sns.violinplot(x="Algoritmo", y="Rank", data=long_df, inner="box", cut=0)
plt.title("Ranking Distribution by Algorithm")
plt.ylabel("Ranking (1 = best)")
plt.xlabel("Algorithm")
plt.xticks(rotation=30)
plt.grid(True, linestyle="--", alpha=1)
plt.tight_layout()
plt.yticks(ticks=range(1, 6))

# Exporta para TikZ
tikzplotlib.save("violin.tex")
plt.savefig("violin_ranks.pdf", format="pdf")
print("Arquivo TikZ salvo como 'violin_ranks.tex'")

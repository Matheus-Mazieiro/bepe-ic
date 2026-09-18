import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from scipy.stats import friedmanchisquare
import scikit_posthocs as sp
import numpy as np

# Função para desenhar Critical Difference plot manualmente
def plot_cd_diagram(mean_ranks, cd, title="Critical Difference Diagram", filename="cd_diagram.pdf"):
    sorted_ranks = mean_ranks.sort_values()
    names = sorted_ranks.index.tolist()
    ranks = sorted_ranks.values

    fig, ax = plt.subplots(figsize=(10, 2))
    ax.set_xlim(0.5, len(names) + 0.5)
    ax.set_ylim(0, 1)
    ax.set_axis_off()

    # Desenha os rankings
    for i, (name, rank) in enumerate(zip(names, ranks), 1):
        ax.plot([rank], [0.5], 'o', color='black')
        ax.text(rank, 0.6, name, rotation=90, verticalalignment='bottom', horizontalalignment='center', fontsize=10)

    # Desenha a escala de ranks
    min_r, max_r = int(np.floor(min(ranks))), int(np.ceil(max(ranks)))
    for r in range(min_r, max_r + 1):
        ax.plot([r, r], [0.4, 0.45], color='black')
        ax.text(r, 0.35, f'{r:.1f}', ha='center', fontsize=8)

    ax.plot([min_r, max_r], [0.45, 0.45], color='black')

    # Desenha a Critical Difference (CD)
    mid_point = (max_r + min_r) / 2
    ax.plot([mid_point - cd / 2, mid_point + cd / 2], [0.85, 0.85], color='red', linewidth=2)
    ax.plot([mid_point - cd / 2, mid_point - cd / 2], [0.83, 0.87], color='red', linewidth=2)
    ax.plot([mid_point + cd / 2, mid_point + cd / 2], [0.83, 0.87], color='red', linewidth=2)
    ax.text(mid_point, 0.9, f"CD = {cd:.2f}", ha='center', fontsize=9)

    plt.title(title)
    plt.tight_layout()
    plt.savefig(filename)
    plt.close()


# Carrega os dados
df = pd.read_csv('nonRealistic-csv-instance-method.csv')
algoritmos = df.columns[3:]
dados = df[algoritmos]

print(dados.columns)
rename_map = {
    "WS_Memetic_MMX": r"$\mathrm{_{WS}MMX_{+LS}}$",
    "WS_Memetic-LS_MPX": r"$\mathrm{_{WS}MPX}$",
    "WS_Memetic_MPX": r"$\mathrm{_{WS}MPX_{+LS}}$",
    "WS_Memetic-LS_MMX": r"$\mathrm{_{WS}MMX}$",

    "Memetic_MMX": r"$\mathrm{MMX_{+LS}}$",
    "Memetic-LS_MPX": r"$\mathrm{MPX}$",
    "Memetic_MPX": r"$\mathrm{MPX_{+LS}}$",
    "Memetic-LS_MMX": r"$\mathrm{MMX}$",

    "Exact": r"$\mathrm{Gurobi}$"
}
dados = dados.rename(columns=rename_map)

# Teste de Friedman
stat, p = friedmanchisquare(*[dados[col] for col in dados.columns])
print(f"Friedman test: estatística={stat:.3f}, p-valor={p:.5f}")

if p < 0.05:
    print("\nDiferenças significativas detectadas! Rodando teste de Nemenyi:")

    # Teste de Nemenyi
    nemenyi = sp.posthoc_nemenyi_friedman(dados)

    # PDF 1: Tabela de p-valores
    fig1, ax1 = plt.subplots(figsize=(10, 8))
    ax1.axis('off')
    table = ax1.table(cellText=nemenyi.round(3).values,
                      rowLabels=nemenyi.index,
                      colLabels=nemenyi.columns,
                      loc='center',
                      cellLoc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1.2, 1.2)
    fig1.tight_layout()
    fig1.savefig('nemenyi_pvalues_table.pdf')
    plt.close()

    # PDF 2: Heatmap
    fig2, ax2 = plt.subplots(figsize=(10, 8))
    sns.heatmap(nemenyi, annot=True, fmt=".3f", cmap="coolwarm", cbar=True, ax=ax2)
    ax2.set_title("Heatmap dos p-valores - Teste de Nemenyi")
    fig2.tight_layout()
    fig2.savefig('nemenyi_pvalues_heatmap.pdf')
    plt.close()

    # PDF 3: Critical Difference Plot
    mean_ranks = dados.rank(axis=1, method='average', ascending=False).mean()
    k = len(mean_ranks)   # número de algoritmos
    N = len(dados)        # número de instâncias
    q_alpha = 2.728       # para alpha = 0.05 e k até 10 (veja tabela de valores críticos de Nemenyi)
    cd = q_alpha * np.sqrt(k * (k + 1) / (6 * N))
    plot_cd_diagram(mean_ranks, cd)

    # PDF 4: Gráfico de Rankings Médios
    fig3, ax3 = plt.subplots(figsize=(10, 6))
    sorted_ranks = mean_ranks.sort_values()
    sns.barplot(x=sorted_ranks.values, y=sorted_ranks.index, palette="viridis", ax=ax3)
    ax3.set_xlabel("Average Ranking (lower is better)")
    ax3.set_ylabel("Algorithm")
    ax3.set_title("Average Ranking of Algorithms (Friedman)")
    for i, v in enumerate(sorted_ranks.values):
        ax3.text(v + 0.05, i, f"{v:.2f}", va='center')
    fig3.tight_layout()
    fig3.savefig("mean_rankings_barplot.pdf")
    plt.close()


    print("\nArquivos salvos com sucesso:")
    print("- nemenyi_pvalues_table.pdf")
    print("- nemenyi_pvalues_heatmap.pdf")
    print("- cd_diagram.pdf")
    print("- mean_rankings_barplot.pdf")


else:
    print("\nSem diferenças estatisticamente significativas.")

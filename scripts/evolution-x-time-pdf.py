import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Estilo limpo
sns.set(style='whitegrid', palette='colorblind')

# Carrega os dados
df = pd.read_csv('evolution_through_time_realistic_norm.csv')

# Limpeza
df = df.dropna(axis=1, how='all')
colunas_tempo = [col for col in df.columns if col.endswith('s')]
tempos = [float(col.replace('s', '')) for col in colunas_tempo]

# Agrupamento por prefixo
def extrair_prefixo(path):
    nome = path.split('/')[-1].replace('.config', '')
    # remove trailing id (everything after last underscore)
    nome_no_id = nome.rsplit('_', 1)[0]
    if path.startswith('WS'):
        return 'WS_' + nome_no_id
    return nome_no_id

df['prefix'] = df['setting'].apply(extrair_prefixo)
rename_map = {
    "MemeticMMX": r"$\mathrm{MMX_{+LS}}$",
    "Memetic-LSMPX": r"$\mathrm{MPX}$",
    "MemeticMPX": r"$\mathrm{MPX_{+LS}}$",
    "Memetic-LSMMX": r"$\mathrm{MMX}$",
    "WSMemeticMMX": r"$\mathrm{_{WS}MMX_{+LS}}$",
    "WSMemetic-LSMPX": r"$\mathrm{_{WS}MPX}$",
    "WSMemeticMPX": r"$\mathrm{_{WS}MPX_{+LS}}$",
    "WSMemetic-LSMMX": r"$\mathrm{_{WS}MMX}$",
    "Exact": r"$\mathrm{Gurobi}$"
}
def map_label(prefix):
    # try direct match
    if prefix in rename_map:
        return rename_map[prefix]
    # try normalized match (ignore underscores and hyphens)
    norm = prefix.replace('_', '').replace('-', '')
    for k, v in rename_map.items():
        if k.replace('_', '').replace('-', '') == norm:
            return v
    return prefix

df_mean = df.groupby('prefix')[colunas_tempo].mean()
df_std = df.groupby('prefix')[colunas_tempo].std()

# Plot
plt.figure(figsize=(14, 7))

cores = sns.color_palette('colorblind', n_colors=len(df_mean.index))

for i, prefix in enumerate(df_mean.index):
    mean_values = df_mean.loc[prefix].values
    std_values = df_std.loc[prefix].values
    cor = cores[i]

    # Curva sem barras de erro (linha com marcadores)
    label_display = map_label(prefix)
    plt.plot(
        tempos, mean_values,
        label=label_display,
        color=cor,
        linewidth=2,
        marker='o',
        markersize=4,
        linestyle='-'
    )
    
    # (remoção de anotações inline: usaremos legenda abaixo)

# Exibe legenda no canto inferior direito com caixa ao redor (estilo performance profile)
legend = plt.legend(title='', fontsize=9, loc='lower right', frameon=True, fancybox=False)
frame = legend.get_frame()
frame.set_edgecolor('black')
frame.set_linewidth(0.6)
frame.set_facecolor('white')
frame.set_alpha(1)

# Ajustes visuais
plt.xscale('log', base=2)
plt.xlabel('Time (s)')
plt.ylabel('Average quality (%)')
plt.title('Average performance over time by configuration group')
plt.grid(True, axis='y')
plt.tight_layout()
plt.xlim(min(tempos), max(tempos)*1.3)
plt.ylim(0, 1.05)

# Salva PDF
out_pdf = 'evolution.pdf'
plt.savefig(out_pdf, bbox_inches='tight')
print(f'saved PDF at {out_pdf}')

# Gera arquivo .tex com TikZ/PGFPlots (usa tikzplotlib se disponível, senão grava um fallback)
out_tex = 'evolution_realistic.tex'
fig = plt.gcf()
try:
    import tikzplotlib
    tikzplotlib.save(out_tex, figure=fig)
    print(f'saved TikZ LaTeX at {out_tex}')
except Exception:
    print('tikzplotlib não disponível - escrevendo fallback .tex com TikZ/PGFPlots')

    def _tex_escape(s):
        if s is None:
            return ''
        s = str(s)
        if s.startswith('$') and s.endswith('$'):
            return s
        repl = {
            '\\': r'\\textbackslash{}',
            '_': r'\\_',
            '%': r'\\%',
            '&': r'\\&',
            '#': r'\\#',
            '{': r'\\{',
            '}': r'\\}',
            '~': r'\\textasciitilde{}',
            '^': r'\\textasciicircum{}'
        }
        for k, v in repl.items():
            s = s.replace(k, v)
        return s

    with open(out_tex, 'w') as f:
        f.write('\\documentclass[tikz,border=10pt]{standalone}\n')
        f.write('\\usepackage{pgfplots}\n')
        f.write('\\usepackage{xcolor}\n')
        f.write('\\pgfplotsset{compat=1.17}\n')
        f.write('\\begin{document}\n')
        f.write('\\begin{tikzpicture}\n')
        f.write('\\begin{axis}[\n')
        f.write('    width=14cm,\n')
        f.write('    height=7.5cm,\n')
        f.write('    xlabel={Time (s)},\n')
        f.write('    ylabel={Average quality (\\%)},\n')
        f.write('    xmode=log,\n')
        f.write('    log basis x=2,\n')
        f.write(f'    xmin={min(tempos)}, xmax={max(tempos)*1.3},\n')
        f.write('    ymin=0, ymax=1.05,\n')
        f.write("    legend style={at={(0.98,0.02)},anchor=south east},\n")
        f.write('    ymajorgrids=true,\n')
        f.write("    grid style={dashed,gray!30},\n")
        f.write(']\n')

        # Define colors to avoid fragile color model syntax
        for i, (r, g, b) in enumerate(cores):
            f.write('\\definecolor{col%d}{rgb}{%.6f,%.6f,%.6f}\n' % (i, r, g, b))

        for i, prefix in enumerate(df_mean.index):
            mean_values = df_mean.loc[prefix].values
            label_display = map_label(prefix)
            coords = ' '.join(f'({t},{v})' for t, v in zip(tempos, mean_values))
            # usa a cor definida acima
            f.write('\\addplot[line width=1.2pt, mark=o, color=col%d] coordinates { %s };\n' % (i, coords))
            f.write('\\addlegendentry{%s}\n' % (_tex_escape(label_display)))

        f.write('\\end{axis}\n')
        f.write('\\end{tikzpicture}\n')
        f.write('\\end{document}\n')

    print(f'Wrote fallback TikZ LaTeX to {out_tex}')

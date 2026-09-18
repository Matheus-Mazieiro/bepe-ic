import csv
import math

#Choose if it is a minimization or maximization problem
#probType = 'minimization'
probType = 'maximization'

#Choose the data file in csv format (see example dados.csv)
file = open('realistic-csv-instance-method.csv')

#Choose the .tex file name, you will need to compile it after 
#(e.g. pdflatex performance_profile.tex) this will generate a ppp.txt file 
#you may see a error if you try to compile again without removing ppp.txt
fileOutput = open('realistic_performance_profile.tex', "w")


csvreader = csv.reader(file)
header = next(csvreader)
numMethods = len(header)-3

data = []

for row in csvreader:
	row[3:] = [float(i) for i in row[3:]]
	data.append(row)
	
file.close()
	
numInstances = len(data)

setX = set()

totals = [[] for i in range(numMethods)]

for l in data:
	opt = 1
	if probType == 'minimization':
		opt = min(l[3:])
	if probType == 'maximization':
		opt = max(l[3:])
	for i in range(numMethods):
		l[i+3] = l[i+3]/opt 
		setX.add(l[i+3])
		totals[i].append(l[i+3])
listaX = list(setX)
listaX.sort() 

fileOutput.write("\\documentclass[crop,tikz]{standalone}\n")
fileOutput.write("\\usepackage{tikz}\n")
fileOutput.write("\\usepackage{pgfplots}\n")
fileOutput.write("\\begin{document}\n")
fileOutput.write("\\begin{filecontents*}{ppp.txt}\n")

fileOutput.write("x")
for i in range(numMethods):
	fileOutput.write(" m" + str(i))
fileOutput.write("\n")

for x in listaX:
	fileOutput.write(str(x))
	for m in totals:
		if probType == 'minimization':
			fileOutput.write(" " + str((sum(i <= x for i in m)/numInstances) * 100))
		if probType == 'maximization':
			fileOutput.write(" " + str((sum(i >= x for i in m)/numInstances) * 100))
			
	fileOutput.write("\n")
		
fileOutput.write("\\end{filecontents*}\n")


minY = 100
for m in totals:
	minAux = (sum(i == 1 for i in m)/numInstances) * 100
	if minAux < minY:
		minY = minAux


fileOutput.write("\\begin{tikzpicture}\n")
fileOutput.write("  \\tiny\n")
fileOutput.write("	\\begin{axis}[\n")
fileOutput.write("		height=7cm,\n")
fileOutput.write("		width=12cm,\n")
fileOutput.write("		xmin=" + str(listaX[0]) + ",\n")
fileOutput.write("		xmax=" + str(listaX[-1]) + ",\n")
if probType == 'maximization': 
	fileOutput.write("		x dir=reverse,\n")
r = math.floor(minY/10) * 10
fileOutput.write("		ytick={" + str(r) + "," + str(r + 10) + ",...,100},\n")
fileOutput.write("		%xtick={1.00,1.01,...," + str(listaX[-1]) + "},\n")
fileOutput.write("		grid=major,\n")
fileOutput.write("		no markers,\n")
fileOutput.write("		xlabel=Performance gap,\n")
fileOutput.write("            ylabel=\\% of instances,\n")
fileOutput.write("		legend pos=south east,\n")
fileOutput.write("		x tick label style={/pgf/number format/.cd, \n")
fileOutput.write("		                                fixed, \n")
fileOutput.write("                                    fixed zerofill,\n")
fileOutput.write("                                    precision=2},  \n")
fileOutput.write("        cycle list={green!90!black,style=thick\\\\\n")
fileOutput.write("                    magenta,style=thick\\\\\n")
fileOutput.write("                    cyan,style=thick\\\\\n")
fileOutput.write("                    blue,style=thick\\\\\n")
fileOutput.write("                    red,style=thick\\\\\n")
fileOutput.write("                    yellow!90!black,style=thick\\\\\n")
fileOutput.write("                    black,style=thick\\\\}\n")
fileOutput.write("	]\n")
fileOutput.write("	\\pgfplotstableread[col sep=space]{ppp.txt}\\mydata;\n")

for m in range(numMethods):
	fileOutput.write("    \\addplot table[y = m" + str(m) + "] from \\mydata ;\n")
	fileOutput.write("    \\addlegendentry{" + header[m+3] + "}\n")
	
	
fileOutput.write("	 	\end{axis}\n")
fileOutput.write("\end{tikzpicture}\n")
fileOutput.write("\end{document}\n")
fileOutput.close()

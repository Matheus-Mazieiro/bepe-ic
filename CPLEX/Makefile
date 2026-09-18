compile:
	g++ pl.cpp ../Input.cpp \
	-I/opt/ibm/ILOG/CPLEX_Studio222/concert/include \
	-I/opt/ibm/ILOG/CPLEX_Studio222/cplex/include \
	-L/opt/ibm/ILOG/CPLEX_Studio222/concert/lib/x86-64_linux/static_pic \
	-L/opt/ibm/ILOG/CPLEX_Studio222/cplex/lib/x86-64_linux/static_pic \
	-lconcert -lilocplex -lcplex \
	-pthread -ldl -lm \
	-o pl 

execute:
	./pl

run:
	./run.sh
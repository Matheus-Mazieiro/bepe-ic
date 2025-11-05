# Definição de variáveis
CC = g++
CFLAGS = -O3 -fno-omit-frame-pointer -MMD -g 
TARGET = bepe

SRCS = memetic/WarmStart.cpp  main.cpp Input.cpp memetic/Crossover.cpp memetic/Individual.cpp memetic/Memetic.cpp memetic/Population.cpp Printer.cpp ReducedInstanceSolver_gurobi.cpp Settings.cpp TourEnhancement.cpp TourEnhancement2.cpp tsp_gurobi.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
ORTOOLS_BINARY_PATH = /home/matheus-ubunto/constraint_programming/or-tools_x86_64_Ubuntu-22.04_cpp_v9.10.4067
ORTOOLS_INC_FLAGS = -I$(ORTOOLS_BINARY_PATH)/include 	-I$(GUROBI_HOME)/include

ORTOOLS_LIB_FLAGS = \
	-lpthread \
	-lm \
	-ldl \
	-L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi120
#	-L$(ORTOOLS_BINARY_PATH)/lib \
	-lortools \
	-lprotobuf \
	-lre2 \
	-lz \
	-lClp \
	-lCbc \
	-lscip \

#export GUROBI_HOME=/home/aloc/matheus/gurobi1202/linux64
PATH := $(GUROBI_HOME)/bin:$(PATH)
#export PATH
LD_LIBRARY_PATH := $(GUROBI_HOME)/lib:$(LD_LIBRARY_PATH)
#export LD_LIBRARY_PATH
#export GRB_LICENSE_FILE=/home/aloc/matheus/gurobi.lic

# Regras para gerar o executável
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(ORTOOLS_INC_FLAGS) -o $(TARGET) $(OBJS) $(ORTOOLS_LIB_FLAGS)

-include $(DEPS)

# Regra para compilar arquivos .c em .o e gerar dependências
%.o: %.cpp
	$(CC) $(CFLAGS) $(ORTOOLS_INC_FLAGS) -c $< -o $@

# Limpeza dos arquivos objeto, dependências e executável
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)
	rm -f *.log

rebuild: clean all

#run: $(TARGET)
#	time ./$(TARGET) $(args)

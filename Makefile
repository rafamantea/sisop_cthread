#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "fila2.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
TST_DIR=./testes/
CFLAGS=-m32 -c

all: directory lib

directory:
	mkdir lib -p -v

lib: clean cthread.o cdata.o
	ar crs $(LIB_DIR)libcthread.a $(BIN_DIR)support.o $(BIN_DIR)cdata.o $(BIN_DIR)cthread.o

cthread.o: $(SRC_DIR)cthread.c
	$(CC) $(CFLAGS) $(SRC_DIR)cthread.c -Wall -o $(BIN_DIR)cthread.o

cdata.o: $(SRC_DIR)cdata.c
	$(CC) $(CFLAGS) $(SRC_DIR)cdata.c -Wall -o $(BIN_DIR)cdata.o

# regran: #dependências para a regran
#  	$(CC) -o $(BIN_DIR)regran $(SRC_DIR)regran.c -Wall

clean:
	find $(BIN_DIR) $(LIB_DIR) $(TST_DIR) -type f ! -name 'support.o' ! -name '*.c' ! -name 'Makefile' ! -name "TestsDocumentation.txt" -delete

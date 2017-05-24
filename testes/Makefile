###############################################
# Makefile para programas de testes unitarios #
# #############################################

CC=gcc
LIB_DIR=../lib
FLAGS=-m32 -o
LIB=-lcthread

all: ccreate cyield cjoin csem cidentify

ccreate:
	$(CC) $(FLAGS) ccreate testCreate.c -L$(LIB_DIR) $(LIB)

cyield:
	$(CC) $(FLAGS) cyield testYield.c -L$(LIB_DIR) $(LIB)

cjoin:
	$(CC) $(FLAGS) cjoin testJoin.c -L$(LIB_DIR) $(LIB)

csem:
	$(CC) $(FLAGS) csem testSemaforo.c -L$(LIB_DIR) $(LIB)

cidentify:
	$(CC) $(FLAGS) cidentify testIdentify.c -L$(LIB_DIR) $(LIB)


clean:
	find -type f ! -name '*.c' ! -name 'Makefile' ! -name "TestsDocumentation.txt" -delete
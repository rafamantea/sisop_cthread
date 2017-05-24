#
# Makefile para programas exemplo
# 

CC=gcc
FLAGS=-m32 -o
LIB_DIR=../lib

all: exemplo mandel teste_vetor barbeiro filosofos prodcons series

exemplo: exemplo.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) exemplo exemplo.c -L$(LIB_DIR) -lcthread -Wall

teste_vetor: teste_vetor.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) teste_vetor teste_vetor.c -L$(LIB_DIR) -lcthread -Wall

mandel: mandel.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) mandel mandel.c -L$(LIB_DIR) -lcthread -lm -Wall

barbeiro: barbeiro.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) barbeiro barbeiro.c -L$(LIB_DIR) -lcthread -lm -Wall

filosofos: filosofos.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) filosofos filosofos.c -L$(LIB_DIR) -lcthread -lm -Wall

prodcons: prodcons.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) prodcons prodcons.c -L$(LIB_DIR) -lcthread -lm -Wall

series: series.c $(LIB_DIR)/libcthread.a
	$(CC) $(FLAGS) series series.c -L$(LIB_DIR) -lcthread -lm -Wall

clean:
	rm -rf exemplo mandel teste_vetor barbeiro filosofos prodcons series *.o *~





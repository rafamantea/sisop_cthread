#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

void* func0(void *arg) {
	printf("Eu sou uma THREAD imprimindo %d\n", *((int *)arg));
	cyield();
	return;
}

int main(int argc, char *argv[]) {

	int	id0, id1, id2, id3, id4;
	int j0, j1, j2, j3, j4, j5;
	int i = 1;


	id0 = ccreate(func0, (void *)&i, 1);
	id1 = ccreate(func0, (void *)&i, 2);
	id2 = ccreate(func0, (void *)&i, 3);
	id3 = ccreate(func0, (void *)&i, 0);
	id4 = ccreate(func0, (void *)&i, 3);

	j0 = cyield();
	puts("Voltou do primeiro Cyield() MAIN\n");
	printf("Valor de retorno de cyield eh: %d\n", j0);
	j1 = cyield();
	puts("Voltou do segundo Cyield() MAIN\n");
	printf("Valor de retorno de cyield eh: %d\n", j1);
	j2 = cyield();
	puts("Voltou do terceiro Cyield() MAIN\n");
	printf("Valor de retorno de cyield eh: %d\n", j2);
	j3 = cyield();
	puts("Voltou do quato Cyield() MAIN\n");
	printf("Valor de retorno de cyield eh: %d\n", j3);
	j4 = cyield();
	puts("Voltou do quinto Cyield() MAIN para finalizar o programa.\n");
	printf("Valor de retorno de cyield eh: %d\n", j4);
	j5 = cyield();
	puts("Voltou do sexto Cyield() MAIN para finalizar o programa.\n");
	printf("Valor de retorno de cyield eh: %d\n", j5);

	printf("Eu sou a main voltando para terminar o programa\n");
}

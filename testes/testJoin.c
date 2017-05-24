#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
}

int main(int argc, char *argv[]) {

	int	id0, id1;
	int j0, j1;
	int i;

	id0 = ccreate(func0, (void *)&i);
	printf("Eu sou a thread de TID: %d\n", id0);
	id1 = ccreate(func1, (void *)&i);
	printf("Eu sou a thread de TID: %d\n", id1);

	printf("Eu sou a main após a criação de ID0 e ID1\n");

	printf("Main solicitou join para thread de TID: 1\n");
	j0 = cjoin(id0);
	printf("Retorno de cjoin(1) é: %d\n",j0);
	printf("Main solicitou join para thread de TID: 2\n");
	j1 = cjoin(id1);
	printf("Retorno de cjoin(2) é: %d\n",j1);


	printf("Eu sou a main após todos os cjoins terem sido efetuados\n");
}
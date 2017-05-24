/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));

	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
	return;
}

int main(int argc, char *argv[]) {

	int	id0, id1, id2, id3, id4, id5, id6;
	int i;


	id0 = ccreate(func0, (void *)&i, 1);
	printf("Eu sou a thread de TID: %d\n", id0);
	id1 = ccreate(func1, (void *)&i, 2);
	printf("Eu sou a thread de TID: %d\n", id1);
	id2 = ccreate(func1, (void *)&i, 3);
	printf("Eu sou a thread de TID: %d\n", id2);
	id3 = ccreate(func1, (void *)&i, 0);
	printf("Eu sou a thread de TID: %d\n", id3);
	id4 = ccreate(func1, (void *)&i, 2);
	printf("Eu sou a thread de TID: %d\n", id4);

	printf("Eu sou a main apos a criacao de threads\n");

	return 0;
}

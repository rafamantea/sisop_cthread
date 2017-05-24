/**
** Teste da função cidentify
**/

#include <stdio.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"



int main(int argc, char *argv[]){
	char c[105] = "";
	int i = 1;
	i = cidentify(c, sizeof(c));
	printf("cidentify = %d | Ocorreu erro com ponteiro ou size. \n", i);

	i = cidentify(c, -1);
	printf("cidentify = %d | Ocorreu erro com ponteiro ou size. \n", i);

	char d[104] = "";
	i = cidentify(d, sizeof(d));
	if(i == -1){
		printf("cidentify = %d | Ocorreu erro com ponteiro ou size. \n", i);
		
	}	
	else{
		printf("cidentify = %d | A funcao cidentify funcionou conforme o esperado\n", i);
	}

	printf("Ponteiro agora contém valores: \n:");
	while(i<sizeof(d)){
		printf("%c", d[i]);
		i++;
	}	


	return 0;
}

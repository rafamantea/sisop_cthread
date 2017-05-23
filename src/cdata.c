#define _XOPEN_SOURCE 600


#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"

#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdint.h> // Usada para gerar ticket com uint8_t

#define SUCCESS 0
#define ERROR -1

#define CRIACAO 0
#define APTO  1
#define EXEC  2
#define BLOQ  3
#define TERMINO 4

#define stackSize SIGSTKSZ


TCB_t *unjoin;
/******************
* FUNÇÕES AUXILIARES
*******************/
int createQueue(PFILA2 fila)
{
  //Inicializa fila de bloqueados
  int initializedQueue;
  initializedQueue = CreateFila2(fila);

  if (initializedQueue == ERROR) {
    return ERROR;
  }
  else {
    return SUCCESS;
  }
}


int generateTicket()
{
  unsigned int random = Random2();
  int ticket = random % 256;
  return ticket;
}

int searchForBestTicket(PFILA2 fila, int loteryTicket)
{
  // Procura tid de melhor TICKET da fila de aptos
  TCB_t *tcb;
  int bestTID;
  int bestValue;

  int first = FirstFila2(fila);

  if (first == SUCCESS) {
    tcb = (TCB_t*) GetAtIteratorFila2(fila);
    bestValue = abs(tcb->ticket - loteryTicket);
    bestTID = tcb->tid;
    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(fila);
      tcb = (TCB_t*) GetAtIteratorFila2(fila);

      if (GetAtIteratorFila2(fila) == NULL) {
        return bestTID;
      }
      if (abs(tcb->ticket - loteryTicket) < bestValue) {
        bestValue = abs(tcb->ticket - loteryTicket);
        bestTID = tcb->tid;
      }
    }
    return bestTID;
  }
  else {
    return ERROR;
  }
}

int searchForTid(PFILA2 fila, int tid)
{
  int first;
  first = FirstFila2(fila);
  if (first == SUCCESS) {
    void *tcb;
    TCB_t *wanted;
    wanted = (TCB_t*) GetAtIteratorFila2(fila);
    if (wanted->tid == tid) {
      return SUCCESS;
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fila);
        tcb = GetAtIteratorFila2(fila);
        if (tcb == NULL) {
          return ERROR;
        }
        else {
          wanted = (TCB_t*) tcb;
          if (wanted->tid == tid) {
            return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  }
  else {
    return ERROR;
  }

}

int searchInFilaJoin(PFILA2 filaJoin, int tid) {
  int first;
  first = FirstFila2(filaJoin);
  if (first == 0) {
    BLOCK_join *ptr;
    ptr = (BLOCK_join *)GetAtIteratorFila2(filaJoin);
    if (ptr->tid == tid) {
      return ERROR;
    }
    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(filaJoin);
      ptr = (BLOCK_join *)GetAtIteratorFila2(filaJoin);
      if(!GetAtIteratorFila2(filaJoin)){
        return SUCCESS;
      }
      else if (ptr->tid == tid) {
        return ERROR;
      }
    }
  }

  return SUCCESS;
}

void deleteFromBlockedQueue(PFILA2 filaBloqueados, int tid){
  int first;

  first = FirstFila2(filaBloqueados);
  if(first == SUCCESS){
    TCB_t *ptr;
    ptr = (TCB_t *) GetAtIteratorFila2(filaBloqueados);
    if(ptr->tid == tid){
      DeleteAtIteratorFila2(filaBloqueados);
    }
    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(filaBloqueados);
      ptr = (TCB_t *)GetAtIteratorFila2(filaBloqueados);
      if (ptr == NULL) {
        return;
      }
      if(ptr->tid == tid){
        DeleteAtIteratorFila2(filaBloqueados);
      }      
    }
  }
}


void runsThroughQueue(PFILA2 fila)
{
  int work;
  work = FirstFila2(fila);
  if (work == 0) {
    void *tcb;
    tcb = GetAtIteratorFila2(fila);
    TCB_t *teste = (TCB_t*) malloc(sizeof(TCB_t));
    teste = (TCB_t*) tcb;
    printf("%d\n", teste->tid);
    int work2 = 0;
    while (work2 == 0) {
      work2 = NextFila2(fila);
      tcb = GetAtIteratorFila2(fila);
      if (tcb == NULL) {
        return;
      }
      teste = (TCB_t*) tcb;
      printf("%d\n", teste->tid);
    }

  }
}

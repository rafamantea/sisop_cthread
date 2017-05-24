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

#define stackSize SIGSTKSZ


TCB_t *unjoin;
/******************
* FUNÇÕES AUXILIARES
*******************/

TCB_t* criarTCB(int tid, ucontext_t contexto) {
  TCB_t* tcb = malloc(sizeof(TCB_t));
    tcb->tid = tid; 
    tcb->state = PROCST_CRIACAO;
    // salvar contexto no TCB
    tcb->context = contexto;
  
  return tcb;
}

int adicionarNaFila(PFILA2 fila, PNODE2 pnodo) {
  return AppendFila2(fila, pnodo);
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

int changePriority(PFILA2 pfila, int prio) {
  TCB_t *tcb;
  tcb = (TCB_t *) GetAtIteratorFila2(pfila);
  tcb->ticket = prio;
  return 0;
}

/*int searchInFilaJoin(PFILA2 filaJoin, int tid) {
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
}*/

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


// *****************************************************************




/**
int queue_has_tcb(PFILA2 queue, int tid){
    TCB_t* tcb;

    if (queue == NULL){
        return 1;
    }
    if(FirstFila2(queue) == 0){

        tcb = (TCB_t*)GetAtIteratorFila2(queue);
        while(tcb != NULL && tcb->tid != tid){
            if(NextFila2(queue) == 0){
                tcb = (TCB_t*)GetAtIteratorFila2(queue);
                if (tcb == NULL){
                    return 1;
                }
            }
            else{
                return 1;
            }
        }
        if (tcb->tid == tid){
            return 0;
        }
    }
    return 1;
}*/
/*
TCB_t * get_tcb(PFILA2 queue, int tid) {
  TCB_t* tcb;

  if(FirstFila2(queue) == 0){
    tcb = (TCB_t*)GetAtIteratorFila2(queue);
    while(tcb != NULL && tcb->tid != tid){
      if(NextFila2(queue) == 0){
        tcb = (TCB_t*)GetAtIteratorFila2(queue);
      }
    }
    if (tcb->tid == tid){
      return tcb;
    }
  }
  return NULL;
}*/

/*TCB_t * get_tcb_by_tid(int tid) {
  if(queue_has_tcb(ready_very_high, tid) {
    return get_tcb(ready_very_high, tid);
  
  } else if(queue_has_tcb(ready_high, tid)) {
    return get_tcb(ready_high, tid);
    
  } else if(queue_has_tcb(ready_medium, tid)) {
    return get_tcb(ready_medium, tid);
    
  } else if(queue_has_tcb(ready_low, tid)) {
    return get_tcb(ready_low, tid);
    
  } else if(queue_has_tcb(blocked, tid)) {
    return get_tcb(blocked, tid);
  }
  
  return NULL;
}
*/

/*int tid_exists(int tid) {
  if (searchForTid(it_ready_very_high, tid) == ERROR && searchForTid(it_ready_high, tid) == ERROR &&
  searchForTid(it_ready_medium, tid) == ERROR && searchForTid(it_ready_low, tid) == ERROR &&
  searchForTid(it_blocked, tid) == ERROR) {
    
    return ERROR;
  }
  
  return SUCCESS;
}*/


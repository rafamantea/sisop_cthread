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

int insertTCB_at_queue(PFILA2 fila, TCB_t* tcb) {
  PNODE2 pnodo = malloc(sizeof(NODE2)); // alocar espaço para um novo nodo na fila
  pnodo->node = tcb;

  if ( AppendFila2(fila, pnodo) ){
    printf( "Erro ao inserir TCB na fila");
    return 1;
  }
  return 0;
}

/*int createQueue(PFILA2 fila)
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
}*/

/*int generateTicket()
{
  unsigned int random = Random2();
  int ticket = random % 256;
  return ticket;
}*/

/*int searchForBestTicket(PFILA2 fila, int loteryTicket)
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
}*/

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
  tcb->tid = prio;
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


// *****************************************************************




int add_ready_by_priority(int prio, TCB_t* tcb) {
  switch(prio) {
    case VERY_HIGH:
      return insertTCB_at_queue(ready_very_high, tcb);
    case HIGH:
      return insertTCB_at_queue(ready_high, tcb);
    case MEDIUM:
      return insertTCB_at_queue(ready_medium, tcb);
    case LOW:
      return insertTCB_at_queue(ready_low, tcb);
    default:
      return 1;
  }
}

int add_ready_by_priority(TCB_t* tcb) {
  return add_ready_by_priority(tcb->ticket, tcb);
}

int blocked_to_ready(int tid) {
    TCB_t* tcb;
    TCB_t* tcb_aux = (TCB_t*) malloc(sizeof(TCB_t));

    if (FirstFila2(blocked) == 0){
        tcb = (TCB_t*) GetAtIteratorFila2(blocked);
        while(tcb != NULL){
            if(tcb->tid == tid){
                *tcb_aux = *tcb;
                if (DeleteAtIteratorFila2(blocked) == 0){
                    tcb_aux->state = PROCST_APTO;
                    add_ready_by_priority(tcb_aux);
                    return 0;
                }
                else{
                    return -1;
                }
            }
            NextFila2(blocked);
            tcb = (TCB_t*) GetAtIteratorFila2(blocked);
        }
    }
    return -1;  
}

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
}

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
}

TCB_t * get_tcb_by_tid(int tid) {
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

#define _XOPEN_SOURCE 600


#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"

#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
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
#define MAINTID 0

/************
* VARIÁVEIS GLOBAIS
************/

// Contextos para execução de funções de escalonador e de finalizador de threads
ucontext_t contextDispatcher, contextTerminator;

int tid = 1; //Mantém tid global para enumerar threads


BLOCK_join *joinPtr; //Ponteiro criado para iterar sobre fila de bloqueados por Join
TCB_t *CPU; //Ponteiro criada para "simular" a CPU;
TCB_t mainThread;
FILA2 filaAptos;
FILA2 filaBloqueados;
FILA2 filaJoin;
int uninitializedDependencies = 1;



/*******************************************************************************
********************************************************************************
* ******************************     FUNÇÕES ESCALONADOR
********************************************************************************
*******************************************************************************/
int clearCPU()
{
  if (CPU->tid != MAINTID) {
    free(CPU->context.uc_stack.ss_sp);
    free(CPU);
    CPU = NULL;
  }
  return SUCCESS;

}

void removeThreadFromBlockedQueue(threadID)
{
  // Procura na filaBloqueados por threadID
  // Retira ela da filaBloqueados e adiciona na filaAptos
  TCB_t *ptr;

  if(FirstFila2(&filaBloqueados) == SUCCESS){
    ptr = (TCB_t *) GetAtIteratorFila2(&filaBloqueados);
    if(ptr->tid == threadID){
      AppendFila2(&filaAptos, (void *) ptr);
      DeleteAtIteratorFila2(&filaBloqueados);
      printf("DESBLOQUEOU TID %d \n", ptr->tid);
      return;
    }
    else{
      int iterator = 0;
      while(iterator == 0){
        iterator = NextFila2(&filaBloqueados);
        ptr = (TCB_t *) GetAtIteratorFila2(&filaBloqueados);
        if(!ptr){
          return;
        }
        else{
          if(ptr->tid == threadID){
            AppendFila2(&filaAptos, (void *) ptr);
            DeleteAtIteratorFila2(&filaBloqueados);
            printf("DESBLOQUEOU TID %d \n", ptr->tid);
          }
        }
      }
      return;
    }

  }
  else{
    return;
  }

}

void verifyJoinedProcesses(int tidThreadTerminated)
{
  // Verificar se EM TODA A filaJoin existe um joinPtr->tid com valor de CPU->tid
  // Se existir, retira o processo de tid = joinPtr->threadWaiting da filaBloqueados
  // E dá free em joinPtr malloc BLOCK_join
  // se não existir retorna;
 

  if (FirstFila2(&filaJoin) == SUCCESS) {
    joinPtr = (BLOCK_join *) GetAtIteratorFila2(&filaJoin);
    if(joinPtr->tid == tidThreadTerminated){
      removeThreadFromBlockedQueue(joinPtr->threadWaiting);
      DeleteAtIteratorFila2(&filaJoin);
      free(joinPtr);
      joinPtr = NULL;
    }
    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(&filaJoin);
      joinPtr = (BLOCK_join *) GetAtIteratorFila2(&filaJoin);
      if(!joinPtr){
        return;
      }
      else{
        if(joinPtr->tid == tidThreadTerminated){
          removeThreadFromBlockedQueue(joinPtr->threadWaiting);
          DeleteAtIteratorFila2(&filaJoin);
          free(joinPtr);
          joinPtr = NULL;
        }
      }
    }
    return;
  }
  else{
    return;
  }
}



void terminate()
{
  //VERIFICAR FILA DE BLOQUEADOS -> SE ALGUM ESTIVER BLOQUEADO,
  //                                DESBLOQUEIA O PROCESSO;
  // VERIFICAR FILA DE SEMÁFORO -> CWAIT() / CSIGNAL()
  // RETIRA PROCESSO DE ESTADO EXECUTANDO
  verifyJoinedProcesses(CPU->tid);
  clearCPU();
  setcontext(&contextDispatcher);
}

void selectProcess(int bestTID){
  int first = FirstFila2(&filaAptos);
  if ( first == SUCCESS){
    CPU = (TCB_t *) GetAtIteratorFila2(&filaAptos);
    if(CPU->tid == bestTID){
      DeleteAtIteratorFila2(&filaAptos);
      return;
    }
    else{
      int iterator=0;
      while(iterator == 0){
        iterator = NextFila2(&filaAptos);
        CPU = (TCB_t *) GetAtIteratorFila2(&filaAptos);
        if(CPU->tid == bestTID){
          DeleteAtIteratorFila2(&filaAptos);
          return;
        }
      }
    }
  }
  return;
}


void dispatch()
{
  // Gera bilhete de loteria  ++
  // Percorre fila para achar os que mais se aproximam ++
  // Seleciona a thread a ser executada ++
  // retira ela da fila de aptos ++
  // faz swap para o contexto selecionado ++
  int loteryTicket = generateTicket();
  int bestTID;
  bestTID = searchForBestTicket(&filaAptos, loteryTicket);

  selectProcess(bestTID);
  CPU->state = EXEC;

  setcontext(&CPU->context);
}


/*******************************************************************************
********************************************************************************
* *************  FUNÇÕES AUXILIARES E DE INICIALIZAÇÃO
********************************************************************************
*******************************************************************************/

int createDispatcherContext()
{
  getcontext(&contextDispatcher);
  contextDispatcher.uc_link = 0;
  contextDispatcher.uc_stack.ss_sp = (char*) malloc(stackSize);
  if (contextDispatcher.uc_stack.ss_sp == NULL) {
    return ERROR;
  }
  contextDispatcher.uc_stack.ss_size = stackSize;
  makecontext(&contextDispatcher, (void(*)(void))dispatch, 0);
  return SUCCESS;

}

int createTerminatorContext()
{
  getcontext(&contextTerminator);
  contextTerminator.uc_link = 0;
  contextTerminator.uc_stack.ss_sp = (char*) malloc(stackSize);
  if (contextTerminator.uc_stack.ss_sp == NULL) {
    return ERROR;
  }
  contextTerminator.uc_stack.ss_size = stackSize;
  makecontext(&contextTerminator, (void(*)(void))terminate, 0);
  return SUCCESS;

}

int createBlockedQueue()
{
  //Inicializa fila de bloqueados
  return createQueue(&filaBloqueados);

}

int createJoinQueue() {
  return createQueue(&filaJoin);
}

int createMainContext() {
  //gera Contexto da main
  mainThread.tid = MAINTID;
  mainThread.state = EXEC;
  mainThread.ticket = generateTicket(); // Valor dummie

  getcontext(&mainThread.context);

  CPU = &mainThread;
  if (CPU) {
    printf("Adicionou a CPU!\n");
    return SUCCESS;
  }
  else {
    return ERROR;
  }

}


int createReadyQueue()
{
  //Inicializa fila de aptos
  return createQueue(&filaAptos);
}

int initialize()
{
  // Criar MainContext
  // Criar fila de bloqueados
  // Criar fila de aptos
  // Criar threads de dispatcher e terminate
  // Fila de semáforos irá ser criada apenas quando for necessária

  int dispatcherContextCreated;
  int terminateContextCreated;

  int blockedQueueinitilized;
  int joinQueueinitilized;
  int readyQueueinitilized;
  int mainContextCreated;



  blockedQueueinitilized = createBlockedQueue();
  joinQueueinitilized = createJoinQueue();
  readyQueueinitilized = createReadyQueue();
  mainContextCreated = createMainContext();
  dispatcherContextCreated = createDispatcherContext();
  terminateContextCreated = createTerminatorContext();

  if (mainContextCreated == ERROR ||
          blockedQueueinitilized == ERROR ||
          joinQueueinitilized == ERROR ||
          readyQueueinitilized == ERROR ||
          dispatcherContextCreated == ERROR ||
          terminateContextCreated == ERROR) {
    return ERROR;
  }
  else {
    return SUCCESS;
  }

}





/*******************************************************************************
********************************************************************************
* **************  FUNÇÕES QUE DEVEM SER NECESSARIAMENTE IMPLEMENTADAS
********************************************************************************
*******************************************************************************/
int ccreate(void* (*start)(void*), void *arg)
{

  if (uninitializedDependencies == 1) {
    uninitializedDependencies = initialize();
    if (uninitializedDependencies == ERROR) {
      return ERROR;
    }
  }

  TCB_t *newThread = (TCB_t*) malloc(sizeof(TCB_t));
  newThread->tid = tid;
  newThread->state = APTO;
  newThread->ticket = generateTicket(); // Valor dummie

  getcontext(&newThread->context);

  newThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);

  if (newThread->context.uc_stack.ss_sp == NULL) {
    return ERROR; // Erro ao alocar espaço para thread
  }
  newThread->context.uc_stack.ss_size = stackSize;
  newThread->context.uc_link = &contextTerminator;

  makecontext(&newThread->context, (void(*)(void))start, 1, arg);


  tid++;

  int addedToReadyQueue;
  addedToReadyQueue = AppendFila2(&filaAptos, (void *) newThread);
  if (addedToReadyQueue == SUCCESS) {
    // printf("Adicionou na fila de aptos!\n");
    return newThread->tid;
  }
  else {
    return ERROR;
  }
}

int cjoin(int tid)
{
  // VERIFICA DE tid existe na filaAptos ou filaBloqueados ++
  // VERIFICA SE tid JÁ EXISTE NA filaJoin, ou seja, 
  //      se uma thread já está esperando por esse tid
  // ADD THREAD NA filaBloqueados e PAR tid, threadWaiting na filaJoin
  // SAIR DE EXECUÇÃO
  // SALVAR CONTEXTO ATUAL
  // SETAR CONTEXTO PARA DISPATCHER


  if (searchForTid(&filaAptos, tid) == ERROR &&
      searchForTid(&filaBloqueados, tid) == ERROR) {
    return ERROR;
  }

  if(searchInFilaJoin(&filaJoin, tid) == ERROR){
    return ERROR;
  }
  else {
    CPU->state = BLOQ;
    BLOCK_join *newPair = (BLOCK_join*) malloc(sizeof(BLOCK_join));
    newPair->tid = tid;
    newPair->threadWaiting = CPU->tid;
    if(AppendFila2(&filaJoin, (void *) newPair) == SUCCESS &&
      AppendFila2(&filaBloqueados, (void *) CPU) == SUCCESS){
    }
    swapcontext(&CPU->context, &contextDispatcher);
    return SUCCESS;
  }   
}

int cyield(void)
{
  //MUDA ESTADO PARA APTO
  //RETIRA DA CPU
  //FAZ SWAP CONTEXT COM DISPATCHER

  if (uninitializedDependencies == 1) {
    uninitializedDependencies = initialize();
    if (uninitializedDependencies == ERROR) {
      return ERROR;
    }
  }

  CPU->state = APTO;
  if( FirstFila2(&filaAptos) == SUCCESS &&
      AppendFila2(&filaAptos, (void *) CPU) == SUCCESS){
      swapcontext(&CPU->context, &contextDispatcher);
      return SUCCESS;
  }
  return ERROR;

}

int csem_init(csem_t *sem, int count)
{
  if (uninitializedDependencies == 1) {
    uninitializedDependencies = initialize();
    if (uninitializedDependencies == ERROR) {
      return ERROR;
    }
  }

  if(!sem || count < 0){
    return ERROR;
  }

  sem->count = count;
  sem->fila = 0;

  return SUCCESS;
}

int cwait(csem_t *sem)
{
  if (uninitializedDependencies == 1) {
    uninitializedDependencies = initialize();
    if (uninitializedDependencies == ERROR) {
      return ERROR;
    }
  }

  if(!sem){
    return ERROR;
  }

  // decrementa 1 do counter ++
  // Verifica se counter < 0 ++
  //  verifica se fila do semáforo já está criada ++
  //    se fila não está criada, cria ++
  //  adiciona tcb à fila de espera ++
  //  muda estado da thread de Exec para Bloq ++
  //  muda contexto para dipatcher (swapcontext) ++

  sem->count--;

  if(sem->count < 0){
    if(!sem->fila){
      sem->fila = (FILA2 *) malloc(sizeof(FILA2));
      CreateFila2(sem->fila);
    }
    CPU->state = BLOQ;
    if(AppendFila2(sem->fila, (void *) CPU) != SUCCESS || 
       AppendFila2(&filaBloqueados, (void *) CPU) != SUCCESS){
      return ERROR;
    }
    swapcontext(&CPU->context, &contextDispatcher);
  }

  return SUCCESS;
}

int csignal(csem_t *sem)
{
  if (uninitializedDependencies == 1) {
    uninitializedDependencies = initialize();
    if (uninitializedDependencies == ERROR) {
      return ERROR;
    }
  }

  // Adiciona 1 ao counter ++
  // Verifica se existe fila de bloqueados de semáforo++
  //    se existir remove primeiro TCB da fila semaforo++
  //    Adiciona para fila de aptos++
  //    
  // Retorna SUCCESS++

  if(!sem){
    return ERROR;
  }

  sem->count++;

  if(sem->fila){

    FirstFila2(sem->fila);
    TCB_t *unlockedThread = (TCB_t *) GetAtIteratorFila2(sem->fila);
    unlockedThread->state = APTO;
    AppendFila2(&filaAptos, (void *) unlockedThread);
    deleteFromBlockedQueue(&filaBloqueados, unlockedThread->tid);
    DeleteAtIteratorFila2(sem->fila);
    if(FirstFila2(sem->fila)!= SUCCESS){
      free(sem->fila);
      sem->fila = NULL;
    }

  }


  return SUCCESS;
}

int cidentify(char *name, int size)
{
  char groupName[] = "Nome: Filipe Joner Cartao: 208840\nNome: Jean Andrade Cartao: 252846\nNome: Rodrigo Dal Ri Cartao: 244936";
  int realSize = sizeof(groupName);
  int i = 0;
  if (size <= 0 || size > realSize) {
    return ERROR;
  }

  while (i < size) {
    *name = groupName[i];
    name++;
    i++;
  }
  return SUCCESS;
}



/*******************************************************************************
********************************************************************************
* ******************************     FUNÇÕES DE TESTESS
********************************************************************************
*******************************************************************************/
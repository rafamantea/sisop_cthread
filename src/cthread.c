#define _XOPEN_SOURCE 600


#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"

#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR -1

#define STACK_SIZE SIGSTKSZ
#define MAIN_TID 0

/***************************************
*
* VARIÁVEIS GLOBAIS
*
****************************************/

// Contextos para execução de funções de escalonador e de finalizador de threads
//ucontext_t contextDispatcher, contextTerminator;

int tid_count = 1; //Mantém tid global para enumerar threads


//BLOCK_join *joinPtr; //Ponteiro criado para iterar sobre fila de bloqueados por Join
//TCB_t *CPU; //Ponteiro criada para "simular" a CPU;
//FILA2 filaAptos;
//FILA2 filaBloqueados;
//FILA2 filaJoin;
int initialized = 0;

/*
*** NOSSAS VARIÁVEIS
*/

//Filas 
FILA2 ready_very_high;
FILA2 ready_high;
FILA2 ready_medium;
FILA2 ready_low;

FILA2 blocked;

//Iteradores das filas
PFILA2 it_ready_very_high;
PFILA2 it_ready_high;
PFILA2 it_ready_medium;
PFILA2 it_ready_low;

PFILA2 it_blocked;

//Simboliza o Processador
TCB_t *cpu_tcb;

//main thread
TCB_t main_thread;

//context do dispatcher
ucontext_t context_dispatcher;
ucontext_t context_finish;



/***************************************
*
* FUNÇÕES ESCALONADOR
*
****************************************/
int clearCPU()
{
  if (CPU->tid != MAIN_TID) {
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

void verifyJoinedProcesses(int tidThreadTerminated){
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

void terminate(){
  //VERIFICAR FILA DE BLOQUEADOS -> SE ALGUM ESTIVER BLOQUEADO,
  //                                DESBLOQUEIA O PROCESSO;
  // VERIFICAR FILA DE SEMÁFORO -> CWAIT() / CSIGNAL()
  // RETIRA PROCESSO DE ESTADO EXECUTANDO
  verifyJoinedProcesses(cpu_tcb->tid);
  clearCPU();
  setcontext(&context_dispatcher);
}

/*void selectProcess(int bestTID){
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
}*/

void dispatch() {
  // Gera bilhete de loteria  ++
  // Percorre fila para achar os que mais se aproximam ++
  // Seleciona a thread a ser executada ++
  // retira ela da fila de aptos ++
  // faz swap para o contexto selecionado ++
  //int loteryTicket = generateTicket();
  //int bestTID;
  

  //bestTID = searchForBestTicket(&filaAptos, loteryTicket);

  //selectProcess(bestTID);
  cpu_tcb->state = PROCST_EXEC;

  setcontext(&cpu_tcb->context);
}


/***************************************
*
* FUNÇÕES INICIALIZAÇÃO
*
****************************************/
 /*int createDispatcherContext()
{
  getcontext(&contextDispatcher);
  contextDispatcher.uc_link = 0;
  contextDispatcher.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);
  if (contextDispatcher.uc_stack.ss_sp == NULL) {
    return ERROR;
  }
  contextDispatcher.uc_stack.ss_size = STACK_SIZE;
  makecontext(&contextDispatcher, (void(*)(void))dispatch, 0);
  return SUCCESS;

}*/

/*int createTerminatorContext()
{
  getcontext(&contextTerminator);
  contextTerminator.uc_link = 0;
  contextTerminator.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);
  if (contextTerminator.uc_stack.ss_sp == NULL) {
    return ERROR;
  }
  contextTerminator.uc_stack.ss_size = STACK_SIZE;
  makecontext(&contextTerminator, (void(*)(void))terminate, 0);
  return SUCCESS;

}*/

/*int createBlockedQueue()
{
  //Inicializa fila de bloqueados
  return createQueue(&filaBloqueados);
}*/

/*int createJoinQueue() {
  return createQueue(&filaJoin);
}*/

/*int createMainContext() {
  //gera Contexto da main
  main_thread.tid = MAIN_TID;
  main_thread.state = EXEC;
  main_thread.ticket = generateTicket(); // Valor dummie

  getcontext(&main_thread.context);

  CPU = &main_thread;
  if (CPU) {
    printf("Adicionou a CPU!\n");
    return SUCCESS;
  }
  else {
    return ERROR;
  }

}*/

/*int createReadyQueue()
{
  //Inicializa fila de aptos
  return createQueue(&filaAptos);
}*/

/*int createMainContext() {
  //gera Contexto da main
  main_thread.tid = MAIN_TID;
  main_thread.state = EXEC;
  main_thread.ticket = generateTicket(); // Valor dummie

  getcontext(&main_thread.context);

  CPU = &main_thread;
  if (CPU) {
    printf("Adicionou a CPU!\n");
    return SUCCESS;
  }
  else {
    return ERROR;
  }
}*/

int init_queues() {
  it_ready_very_high = &ready_very_high;
  it_ready_high = &ready_high;
  it_ready_medium = &ready_medium;
  it_ready_low = &ready_low;
  it_blocked = &blocked;
  
  if( CreateFila2( it_ready_very_high ) ) {
    printf("Erro ao criar fila apto - VERY HIGH\n");
    return 1; //erro
  } 
  if( CreateFila2( it_ready_high ) ) {
    printf("Erro ao criar fila apto - HIGH\n");
    return 1; //erro
  } 
  if( CreateFila2( it_ready_medium ) ) {
    printf("Erro ao criar fila apto - MEDIUM\n");
    return 1; //erro
  } 
  if( CreateFila2( it_ready_low ) ) {
    printf("Erro ao criar fila apto - LOW\n");
    return 1; //erro
  }
  if( CreateFila2( it_blocked ) ) {
    printf("Erro ao criar fila bloqueados\n");
    return 1; //erro
  }
  
  return 0;
}

int init_main_thread_context() { 
  //current_thread_context = (TCB_t*)criarTCB(0, current_thread_context);
  main_thread.id = 0;
  main_thread.state = PROCST_EXEC;
  main_thread.ticket = 0;  // nao se usa a prioridade nessa caso

  getcontext(&main_thread.context);
  cpu_tcb = &main_thread;

  if (Escalonador == NULL) {
    return 1; //erro
  }
  return 0;
}

int create_context_dispacher() {
  getcontext(&context_dispatcher);
  
  context_dispatcher.uc_link = 0;
  context_dispatcher.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);

  if (context_dispatcher.uc_stack.ss_sp == NULL) {
    return 1; //erro
  }
  context_dispatcher.uc_stack.ss_size = STACK_SIZE;
  makecontext(&context_dispatcher, (void(*)(void))dispatch, 0); // contexto para funcao dispatch()
  return 0;

}

int create_context_finish() {
  getcontext(&context_finish);

  context_finish.uc_link = 0;
  context_finish.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);
  if (context_finish.uc_stack.ss_sp == NULL) {
    return 1; //erro
  }
  context_finish.uc_stack.ss_size = STACK_SIZE;
  makecontext(&context_finish, (void(*)(void))terminate, 0);
  return 0;
}

int initialize() {
  // Criar MainContext
  // Criar fila de bloqueados
  // Criar fila de aptos
  // Criar threads de dispatcher e terminate
  // Fila de semáforos irá ser criada apenas quando for necessária

  //int dispatcherContextCreated;
  //int terminateContextCreated;

  //int blockedQueueinitilized;
  //int joinQueueinitilized;
  //int readyQueueinitilized;
  //int mainContextCreated;
  //blockedQueueinitilized = createBlockedQueue();
  //joinQueueinitilized = createJoinQueue();
  //readyQueueinitilized = createReadyQueue();

  // inicialização das filas
  if ( init_queues() ){
    printf("Erro ao iniciar as filas\n");
    return 1;
  }

  // inicialização do context da main_thread
  if( init_main_thread_context() ) {
    printf("Erro ao iniciar o main context\n");
    return 1;
  }

  // criar contexto para o dispacher
  if ( create_context_dispacher() ) {
    printf("Erro ao criar um contexto para o dispatcher\n");
    return 1;
  }

  // criar contexto para finalização
  if( create_context_finish() ) {
    printf("Erro ao criar um contexto para a funcao finish\n");
    return 1;
  }

  //mainContextCreated = createMainContext();
  //dispatcherContextCreated = createDispatcherContext();
  //terminateContextCreated = createTerminatorContext();

  /*if (mainContextCreated == ERROR ||
          blockedQueueinitilized == ERROR ||
          joinQueueinitilized == ERROR ||
          readyQueueinitilized == ERROR ||
          dispatcherContextCreated == ERROR ||
          terminateContextCreated == ERROR) {
    return ERROR;
  }
  else {
    return SUCCESS;
  }*/

  return 0;

}

void check_initialized() {
	if (!initialized) {
		initialized = initialize();
		if (initialized == ERROR) {
			return ERROR;
		}
	}
	return SUCCESS;
}

/***************************************
*
* PRIMITIVAS CTHREAD
*
****************************************/

int ccreate (void *(*start)(void *), void *arg, int prio){

  if (check_initialized() == ERROR) {
	  return ERROR;
  }

  TCB_t *new_thread = (TCB_t*) malloc(sizeof(TCB_t));
  new_thread->tid = tid_count;
  new_thread->state = PROCST_APTO;
  new_thread->ticket = prio; 

  getcontext(&new_thread->context);

  new_thread->context.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);

  if (new_thread->context.uc_stack.ss_sp == NULL) {
    return ERROR; // Erro ao alocar espaço para thread
  }
  new_thread->context.uc_stack.ss_size = STACK_SIZE;
  new_thread->context.uc_link = &contextTerminator;

  makecontext(&new_thread->context, (void(*)(void))start, 1, arg);

  tid_count++;

  int added_to_ready;
  added_to_ready = add_ready_by_priority(tcb, prio);

  if (added_to_ready != SUCCESS) {
    return ERROR;
  }
  
    return new_thread->tid;
}

int csetprio(int tid, int prio){
  // procurar em cada uma das filas de apto

  if( !searchForTid(it_ready_very_high, tid) ) { // se a thread estiver na fila VH
    changePriority(it_ready_very_high, prio);
    return SUCCESS;
  }

  if( !searchForTid(it_ready_high, tid) ) { // se a thread estiver na fila H
    changePriority(it_ready_high, prio);
    return SUCCESS;
  }

  if( !searchForTid(it_ready_medium, tid) ) { // se a thread estiver na fila M
    changePriority(it_ready_medium, prio);
    return SUCCESS;
  }

  if( !searchForTid(it_ready_low, tid) ) { // se a thread estiver na fila L
    changePriority(it_ready_low, prio);
    return SUCCESS;
  }
  
  if( !searchForTid(it_blocked, tid) ) { // se a thread estiver na fila Blocked
    changePriority(it_blocked, prio);
    return SUCCESS;
  }  

  return ERROR;
}


int cjoin(int tid){
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

int cyield(void){
  //MUDA ESTADO PARA APTO
  //RETIRA DA CPU
  //FAZ SWAP CONTEXT COM DISPATCHER

  if (check_initialized() == ERROR) {
	  return ERROR;
  }

  CPU->state = APTO;
  if( FirstFila2(&filaAptos) == SUCCESS &&
      AppendFila2(&filaAptos, (void *) CPU) == SUCCESS){
      swapcontext(&CPU->context, &contextDispatcher);
      return SUCCESS;
  }
  return ERROR;

}

int csem_init(csem_t *sem, int count){

  if (check_initialized() == ERROR) {
	  return ERROR;
  }

  if(!sem || count < 0){
    return ERROR;
  }

  sem->count = count;
  sem->fila = 0;

  return SUCCESS;
}

int cwait(csem_t *sem){
  if (!initialized) {
    initialized = initialize();
    if (initialized == ERROR) {
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

int csignal(csem_t *sem){

  if (check_initialized() == ERROR) {
	  return ERROR;
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

int cidentify(char *name, int size){
  char students[] = "Rafael Amantea - 228433\n Mauricio Carmelo - xxxxxx";
  int realSize = sizeof(students);
  int i = 0;
  if (size <= 0 || size > realSize) {
    return ERROR;
  }

  while (i < size) {
    *name = students[i];
    name++;
    i++;
  }
  return SUCCESS;
}


/***************************************
*
* FUNÇÕES AUXILIARES
*
****************************************/



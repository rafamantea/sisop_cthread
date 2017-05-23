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

FILA2 join;

FILA2 blocked;

//Iteradores das filas
PFILA2 it_ready_very_high;
PFILA2 it_ready_high;
PFILA2 it_ready_medium;
PFILA2 it_ready_low;

PFILA2 it_join;

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
  it_join = &join;
  
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
  if( CreateFila2( it_join ) ) {
    printf("Erro ao criar fila join\n");
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

/**
1. Verifica se TID existe nas filas de prioridade APTOS ou Bloqueados.
2. Verifica se TID já existe na fila de join, ou seja, se alguma thread já está esperando por esse tid.
3. Adiciona thread na fila Bloqueados, e par Tid, thread waiting na fila Join
4. Sai de execução
5. Salva contexto atual
6. Seta contexto p/ dispatcher
**/
int cjoin(int tid){

	if (tid_exists(tid) == ERROR) {

		return ERROR;
	}

	if(searchInFilaJoin(it_join, tid) == ERROR){
		return ERROR;
	} 

	cpu_tcb->state = PROCST_BLOCK;
	BLOCK_join *new_pair = (BLOCK_join*) malloc(sizeof(BLOCK_join));
	new_pair->tid = tid;
	new_pair->threadWaiting = cpu_tcb->tid;
	
	if(AppendFila2(it_join, (void *) new_pair) == SUCCESS && AppendFila2(it_blocked, (void *) cpu_tcb) == SUCCESS){}
	swapcontext(&cpu_tcb->context, &contextDispatcher);
	return SUCCESS;  
}

/**
1. Muda estado para apto de acordo com a prioridade
2. Retira da CPU
3. Faz swap context com o dispatcher
**/
int cyield(void){
	
  if (check_initialized() == ERROR) {
	  return ERROR;
  }

  cpu->state = PROCST_APTO;
  if( FirstFila2(&filaAptos) == SUCCESS && AppendFila2(&filaAptos, (void *) CPU) == SUCCESS){//TODO: reinserir na fila de acordo com a prioridade
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

  if (check_initialized() == ERROR) {
	  return ERROR;
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
    cpu_tcb->state = BLOQ;
    if(AppendFila2(sem->fila, (void *) cpu_tcb) != SUCCESS || 
       AppendFila2(it_blocked, (void *) cpu_tcb) != SUCCESS){
      return ERROR;
    }
    swapcontext(&cpu_tcb->context, &contextDispatcher);
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

int tid_exists(int tid) {
	if (searchForTid(it_ready_very_high, tid) == ERROR && searchForTid(it_ready_high, tid) == ERROR &&
	searchForTid(it_ready_medium, tid) == ERROR && searchForTid(it_ready_low, tid) == ERROR &&
	searchForTid(it_blocked, tid) == ERROR) {
	  
		return ERROR;
  }
  
  return SUCCESS;
}

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
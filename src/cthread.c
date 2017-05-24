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

#define VERY_HIGH 0
#define HIGH 1
#define MEDIUM 2
#define LOW 3

/***************************************
*
* VARIÁVEIS GLOBAIS
*
****************************************/

int tid_count = 1; //Mantém tid global para enumerar threads


//BLOCK_join *joinPtr; //Ponteiro criado para iterar sobre fila de bloqueados por Join

int initialized = 0;

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
*	DECLARAÇÃO DE FUNÇÕES AUXILIARES
*
***************************************/
int has_thread_in_ready_queue();
int insertTCB_at_queue(PFILA2 fila, TCB_t* tcb);
int add_ready_by_priority2(int prio, TCB_t* tcb);
int add_ready_by_priority(TCB_t* tcb);
void remove_thread_from_blocked_queue(int threadID);


/***************************************
*
* FUNÇÕES ESCALONADOR
*
****************************************/
int clear_cpu() {
  if (cpu_tcb->tid != MAIN_TID) {
    free(cpu_tcb->context.uc_stack.ss_sp);
    free(cpu_tcb);
    cpu_tcb = NULL;
  }
  return SUCCESS;
}

/**
1.Verificar se EM TODA A filaJoin existe um joinPtr->tid com valor de CPU->tid
2.Se existir, retira o processo de tid = joinPtr->threadWaiting da filaBloqueados
3.E dá free em joinPtr malloc BLOCK_join
4.se não existir retorna;
**/
/*void check_joined_processes(int tidThreadTerminated){
  if (FirstFila2(&filaJoin) == SUCCESS) {
    joinPtr = (BLOCK_join *) GetAtIteratorFila2(&filaJoin);
    if(joinPtr->tid == tidThreadTerminated){
      remove_thread_from_blocked_queue(joinPtr->threadWaiting);
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
          remove_thread_from_blocked_queue(joinPtr->threadWaiting);
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
}*/

 /**
 1. Verifica fila de bloqueados: se alguem estiver bloqueado, desbloqueia o processo;
 2. Verifica fila de semáforo: cwait() / csignal()
 3. Retira processo do estado executando
 4. check_joined_processes(cpu_tcb->tid);
 **/
void terminate(){
  clear_cpu();
  setcontext(&context_dispatcher);
}

void dispatch() {

  if(FirstFila2(it_ready_very_high) == SUCCESS) {
	  FirstFila2(it_ready_very_high);
	  cpu_tcb = (TCB_t *) GetAtIteratorFila2(it_ready_very_high);
    //FirstFila2(it_ready_very_high);
	  DeleteAtIteratorFila2(it_ready_very_high);
	  
  } else if(FirstFila2(it_ready_high) == SUCCESS) {
	  FirstFila2(it_ready_high);
	  cpu_tcb = (TCB_t *) GetAtIteratorFila2(it_ready_high);
	  DeleteAtIteratorFila2(it_ready_high);
	  
  } else if(FirstFila2(it_ready_medium) == SUCCESS) {
	  FirstFila2(it_ready_medium);
	  cpu_tcb = (TCB_t *) GetAtIteratorFila2(it_ready_medium);
	  DeleteAtIteratorFila2(it_ready_medium);
	  
  } else if(FirstFila2(it_ready_low) == SUCCESS) {
	  FirstFila2(it_ready_low);
	  cpu_tcb = (TCB_t *) GetAtIteratorFila2(it_ready_low);
	  DeleteAtIteratorFila2(it_ready_low);
  }

  cpu_tcb->state = PROCST_EXEC;

  printf("\n%d\n", &cpu_tcb->tid);

  setcontext(&cpu_tcb->context);
}


/***************************************
*
* FUNÇÕES INICIALIZAÇÃO
*
****************************************/
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
  main_thread.tid = 0;
  main_thread.state = PROCST_EXEC;
  main_thread.ticket = 0;  // nao se usa a prioridade nessa caso

  getcontext(&main_thread.context);
  cpu_tcb = &main_thread;

  if (cpu_tcb == NULL) {
    return ERROR; //erro
  }
  return SUCCESS;
}

int create_context_dispacher() {
  getcontext(&context_dispatcher);
  
  context_dispatcher.uc_link = 0;
  context_dispatcher.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);

  if (context_dispatcher.uc_stack.ss_sp == NULL) {
    return ERROR; //erro
  }
  context_dispatcher.uc_stack.ss_size = STACK_SIZE;
  makecontext(&context_dispatcher, (void(*)(void))dispatch, 0); // contexto para funcao dispatch()
  return SUCCESS;

}

int create_context_finish() {
  getcontext(&context_finish);

  context_finish.uc_link = 0;
  context_finish.uc_stack.ss_sp = (char*) malloc(STACK_SIZE);
  if (context_finish.uc_stack.ss_sp == NULL) {
    return ERROR; //erro
  }
  context_finish.uc_stack.ss_size = STACK_SIZE;
  makecontext(&context_finish, (void(*)(void))terminate, 0);
  return SUCCESS;
}

int initialize() {
  // inicialização das filas
  if ( init_queues() ){
    printf("Erro ao iniciar as filas\n");
    return -1; // erro
  }

  // inicialização do context da main_thread
  if( init_main_thread_context() ) {
    printf("Erro ao iniciar o main context\n");
    return -1; //erro
  }

  // criar contexto para o dispacher
  if ( create_context_dispacher() ) {
    printf("Erro ao criar um contexto para o dispatcher\n");
    return -1; //erro
  }

  // criar contexto para finalização
  if( create_context_finish() ) {
    printf("Erro ao criar um contexto para a funcao finish\n");
    return -1; //erro
  }
  return 1;
}

/*int check_initialized() {
	if (!initialized) {
		initialized = initialize();
		if (initialized == ERROR) {
			return ERROR;
		}
	}
	return SUCCESS;
}*/

/***************************************
*
* PRIMITIVAS CTHREAD
*
****************************************/

int ccreate (void *(*start)(void *), void *arg, int prio){

  /*if (check_initialized() == ERROR) {
	  return ERROR;
  }*/
  if (initialized == 0) {
    initialized = initialize();
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
  new_thread->context.uc_link = &context_finish;

  makecontext(&new_thread->context, (void(*)(void))start, 1, arg);

  tid_count++;

  int added_to_ready;
  added_to_ready = add_ready_by_priority(new_thread);

  if (added_to_ready != SUCCESS) {
    return ERROR;
  }
  
    return new_thread->tid;
}

/**
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

}*/

/**
1. Verifica se TID existe nas filas de prioridade APTOS ou Bloqueados.
2. Verifica se TID já existe na fila de join, ou seja, se alguma thread já está esperando por esse tid.
3. Adiciona thread na fila Bloqueados, e par Tid, thread waiting na fila Join
4. Sai de execução
5. Salva contexto atual
6. Seta contexto p/ dispatcher
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
	swapcontext(&cpu_tcb->context, &context_dispatcher);
	return SUCCESS;  
}
*/

/**
1. Muda estado para apto de acordo com a prioridade
2. Retira da CPU
3. Faz swap context com o dispatcher
*/
int cyield(void){

  /*if (check_initialized() == ERROR) {
	  return ERROR;
  }*/
  if (initialized == 0) { // se ainda nao foi inicializado
    printf("\nEscalonador ainda não foi inicializado.\n");
    return ERROR;
  }

  cpu_tcb->state = PROCST_APTO;

  if( (has_thread_in_ready_queue() == SUCCESS) && (add_ready_by_priority(cpu_tcb) == SUCCESS) ){
      swapcontext(&cpu_tcb->context, &context_dispatcher);
      return SUCCESS;
  }
  return ERROR;

}

/*
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
*/
/*
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
*/
/*
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
    TCB_t *unlocked_thread = (TCB_t *) GetAtIteratorFila2(sem->fila);
    unlocked_thread->state = PROCST_APTO;
	add_ready_by_priority(unlocked_thread);
    deleteFromBlockedQueue(it_blocked, unlocked_thread->tid);
    DeleteAtIteratorFila2(sem->fila);
    if(FirstFila2(sem->fila)!= SUCCESS){
      free(sem->fila);
      sem->fila = NULL;
    }
  }

  return SUCCESS;
}

*/
/*
int cidentify(char *name, int size){
  char students[] = "Rafael Amantea - 228433\n Mauricio Carmelo - 273165";
  int real_size = sizeof(students);
  int i = 0;
  if (size <= 0 || size > real_size) {
    return ERROR;
  }

  while (i < size) {
    *name = students[i];
    name++;
    i++;
  }
  return SUCCESS;
}
*/
/***************************************
*
* FUNÇÕES AUXILIARES
*
****************************************/

int has_thread_in_ready_queue() {

  //printf("\nFirstFila2() VH: %d\n", FirstFila2(it_ready_very_high));
  //printf("NextFila2() VH: %d\n", NextFila2(it_ready_very_high));

	if( !FirstFila2(it_ready_very_high) ) {
		return SUCCESS;
	} 
  if( !FirstFila2(it_ready_high) ) {
		return SUCCESS;
	} 
  if( !FirstFila2(it_ready_medium) ) {
		return SUCCESS;
	}
  if( !FirstFila2(it_ready_low) ) {
		return SUCCESS;
	}



	return ERROR;
}

int insertTCB_at_queue(PFILA2 fila, TCB_t* tcb) {
  PNODE2 pnodo = malloc(sizeof(NODE2)); // alocar espaço para um novo nodo na fila
  pnodo->node = tcb;

  if ( AppendFila2(fila, pnodo)){
    return 1;
  }
  return 0;
}

int add_ready_by_priority2(int prio, TCB_t* tcb) {
  switch(prio) {
    case VERY_HIGH:
      //printf("\nprioridade VH\n");
      return insertTCB_at_queue(it_ready_very_high, tcb);
    case HIGH:
      //printf("\nprioridade H\n");
      return insertTCB_at_queue(it_ready_high, tcb);
    case MEDIUM:
      //printf("\nprioridade M\n");
      return insertTCB_at_queue(it_ready_medium, tcb);
    case LOW:
        //printf("\nprioridade L\n");
      return insertTCB_at_queue(it_ready_low, tcb);
    default:
      return 1;
  }
}

int add_ready_by_priority(TCB_t* tcb) {
  return add_ready_by_priority2(tcb->ticket, tcb);
}


/**
1.Procura na fila de Bloqueados por threadID
2.Retira da fila de Bloqueados e adiciona na fila de aptos de acordo com a prioridade
 **/
void remove_thread_from_blocked_queue(int threadID) {
  TCB_t *ptr;

  if(FirstFila2(it_blocked) == SUCCESS){
    ptr = (TCB_t *) GetAtIteratorFila2(it_blocked);
    if(ptr->tid == threadID){
	  add_ready_by_priority(ptr);
      DeleteAtIteratorFila2(it_blocked);
      printf("DESBLOQUEOU TID %d \n", ptr->tid);
      return;
    }
    else{
      int iterator = 0;
      while(iterator == 0){
        iterator = NextFila2(it_blocked);
        ptr = (TCB_t *) GetAtIteratorFila2(it_blocked);
        if(!ptr){
          return;
        }
        else{
          if(ptr->tid == threadID){
            add_ready_by_priority(ptr);
            DeleteAtIteratorFila2(it_blocked);
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
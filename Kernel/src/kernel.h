#ifndef kernel_H
#define kernel_H
#include "shared_utils.h"
#include "server_utils.h"

int conexion_a_memoria;
int conexion_a_cpu;
int conexion_a_cpu_interrupt;

bool esFIFO;
bool init = true;

uint32_t estimacionInicial;
double alfa;
uint32_t gradoMultiprogramacion;
uint32_t tiempoMaximoBloqueado;

//TODO - MUTEX - tic - toc
clock_t tic;
clock_t toc;

u_int32_t nextPID;
pthread_mutex_t mutexNextPID;

typedef struct PID_FD {
	u_int32_t pid;
	int* fd;
} pid_fd;
t_list* pidWithFd;
pthread_mutex_t mutexListaPidWithFd;

//TODO - MUTEX - running
t_pcb* running;

t_queue* colaNew;
pthread_mutex_t mutexColaNew;

t_queue* colaReady;
pthread_mutex_t mutexColaReady;
sem_t ready;

sem_t semGradoMultiprogramacion;
sem_t semFinalizoSusoendido;

sem_t replanificar;

t_queue* colaBloqueados;
pthread_mutex_t mutexColaBloqueados;

sem_t bloqueado;

t_queue* colaReadySuspendidos;
pthread_mutex_t mutexColaReadySuspendidos;

t_log *logger;

#endif

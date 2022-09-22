#ifndef cpu_H
#define cpu_H
#include "math.h"
#include "shared_utils.h"
#include "server_utils.h"
#include "client_utils.h"
#endif

int conexion_a_memoria;
int retardoNoOp;

t_pcb* pcbActual;
//pthread_mutex_t mutexPcbActual;

bool checkInterrupt;
pthread_mutex_t mutexCheckInterrupt;

// TLB
typedef struct {
	uint32_t pagina;
	void* marco;
	clock_t instanteDeCarga;
	clock_t tiempoDeUltimaReferencia;
} t_tlb;
int entradasTLB;
int proximoIndiceTLBLibre;
char* algortimoTLB;
t_tlb* tlb;

uint32_t tamanioPagina;
uint32_t cantEntradasPorTabla;

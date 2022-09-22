#ifndef memoria_H
#define memoria_H
#include "shared_utils.h"
#include "server_utils.h"
#include "client_utils.h"
#endif

int tamMemoria;
int tamPagina;
int entradasPorTabla;
int retardoMemoria;
int retardoSWAP;
char* algoritmoReemplazo;
int marcosPorProceso;

pthread_mutex_t mutexColaPaginasASwapear;
sem_t swapear;
sem_t  suspenderProcesoOK;
sem_t  swapAMemOK;
sem_t  memASwapOK;


void* memoriaUsuarioInicio;

typedef struct {
	void* marco;
	bool presente;
	bool uso;
	bool modificado;
} t_entrada2doNivel;

t_queue* colaPaginasASwapear;


typedef struct {
	uint32_t nroTabla1erNivel;
	uint32_t  nroTabla2doNivel;
	uint32_t indiceEntrada2doNivel;
	uint32_t nroDePagina;
	void* marco;
	bool presente;
	bool modificado;
	bool uso;
} t_pagina_en_memoria;

typedef struct {
	char* nombreArchivoSWAP;
	uint32_t tamanioPrograma;
	uint32_t marcosAsignados;
	t_list* tablaDe1erNivel;
	t_list* tablaDeMarcosAsignados;
} t_mem_proceso;

typedef enum {
	SWAP_A_MEM,
	MEM_A_SWAP,
	SUSPENDER
} opSwap;

typedef struct {
	opSwap opSwap;
	uint32_t nroPagina;
	uint32_t nroTabla1erNivel;
	uint32_t nroTabla2doNivel;
	uint32_t indiceEntrada2doNivel;
	void* marco;
} t_solicitud_swap;

//TODO - MUTEX - tablas1er y 2do nivel
t_list* tablasDe1erNivel;
t_list* tablasDe2doNivel;

t_list* listaDeMarcos;
typedef struct {
	bool disponible;
	void* marco;
	uint32_t nroTabla1erNivel;
} t_info_marco;

uint32_t gradoDeMultiprogramacion;

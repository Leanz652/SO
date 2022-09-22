#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <pthread.h>

typedef enum
{
	//Kernel
	NEW,
	TERMINATE,
	BLOCKED,
	DISPATCHED,
	//CPU
	EXEC,
	INTERRUPT,
	END_IO,
	SET_MEM_CONFIG,
	//MEM
	GET_MEM_CONFIG,
	INIT_PROCESS,
	SUSPEND_PROCESS,
	END_PROCESS,
	GET_TABLA_2DO_NIVEL,
	GET_FRAME,
	GET_DATA,
	SAVE_DATA,
	MEM_OK
} op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum
{
	NOP,
	IO,
    READ,
    COPY,
    WRITE,
    EXIT
} t_instCode;
typedef struct
{
	uint32_t programSize;
	t_list* instrucciones;
} __attribute__((packed)) t_program;
typedef struct
{
	t_instCode instCode;
	uint32_t operators[2];
} __attribute__((packed)) t_inst;

typedef struct
{
	uint32_t pid;
	uint32_t tamanio;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	double estimacion_rafaga;
	uint32_t suspendido;
	t_list* instrucciones;
} __attribute__((packed)) t_pcb;

typedef struct {
  int* first;
  void* second;
} thread_args;


#endif

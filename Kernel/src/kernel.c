#include "kernel.h"

void notificarMemoriaInitProcess(uint32_t pid, uint32_t tamanioPrograma, op_code opCode) {
	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = opCode;
	paquete->buffer = buffer;
	buffer->size = 2 * sizeof(uint32_t);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tamanioPrograma, sizeof(uint32_t));
	buffer->stream = stream;
	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);
}


void notificarMemoria(uint32_t pid, op_code opCode) {
	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = opCode;
	paquete->buffer = buffer;
	buffer->size = sizeof(uint32_t);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &pid, sizeof(uint32_t));
	buffer->stream = stream;
	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);
}

t_pcb* siguientePCBporSJF(t_list* lista) {
	t_pcb* pcbActual = list_get(lista, 0);
	t_pcb* pcbBuscado = list_get(lista, 0);
	int minimo = pcbActual->estimacion_rafaga;
	int indexBuscado = 0;
	if (list_size(lista) > 1) {
		int i = 1;
		while (i < list_size(lista)) {
			pcbActual = list_get(lista, i);
			if (pcbActual->estimacion_rafaga < minimo) {
				pcbBuscado = pcbActual;
				minimo = pcbBuscado->estimacion_rafaga;
				indexBuscado = i;
			}
			i++;
		}
	}
	list_remove(colaReady->elements, indexBuscado);
	return pcbBuscado;
}

t_paquete* crearPaquetePCB(t_pcb* pcb, op_code opCode) {
	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = 6 * sizeof(uint32_t)
			+ list_size(pcb->instrucciones) * sizeof(t_inst);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &(pcb->pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(pcb->tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(pcb->tabla_paginas), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(pcb->program_counter), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(pcb->estimacion_rafaga), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(pcb->suspendido), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	//serializarInstrucciones;
	int i = 0;
	while (i < list_size(pcb->instrucciones)) {
		memcpy(stream + offset, list_get(pcb->instrucciones, i),
				sizeof(t_inst));
		offset += sizeof(t_inst);
		i++;
	}
	buffer->stream = stream;
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = opCode;
	paquete->buffer = buffer;
	return paquete;
}

t_pcb* crearPCB(t_list *instrucciones) {
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pthread_mutex_lock(&mutexNextPID);
	pcb->pid = nextPID;
	pthread_mutex_unlock(&mutexNextPID);
	pcb->instrucciones = instrucciones;
	pcb->tabla_paginas = NULL;
	pcb->program_counter = 0;
	pcb->estimacion_rafaga = estimacionInicial;
	pcb->tamanio = list_size(instrucciones);
	pcb->suspendido = 0;
	return pcb;
}

void recibirPCBdesdeSocket(t_pcb* pcbActual, uint32_t* ioBloquedTime,
		int* cliente_fd) {
	int size;
	void *buffer = recibir_buffer(&size, cliente_fd);
	int offset = 0;
	memcpy(&(pcbActual->pid), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(pcbActual->tamanio), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(pcbActual->tabla_paginas), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(pcbActual->program_counter), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(pcbActual->estimacion_rafaga), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(pcbActual->suspendido), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	int cantidadInstr = (size - 6 * sizeof(uint32_t)) / sizeof(t_inst);
	pcbActual->instrucciones = list_create();
	t_inst *instr;
	int j = 0;
	while (j < cantidadInstr) {
		instr = malloc(sizeof(t_inst));
		memcpy(instr, buffer + offset, sizeof(t_inst));
		offset += sizeof(t_inst);
		list_add(pcbActual->instrucciones, instr);
		j++;
	}

	memcpy(ioBloquedTime, buffer + offset, sizeof(uint32_t));

	free(buffer);
}

void notificarConsola(uint32_t pid) {
	int tieneMismoPid(pid_fd* pid_fd) {
		return (pid_fd->pid) == pid;
	}
	pthread_mutex_lock(&mutexListaPidWithFd);
	pid_fd* respFd = list_find(pidWithFd, (void*) tieneMismoPid);
	pthread_mutex_unlock(&mutexListaPidWithFd);
	int opCode = TERMINATE;
	send(respFd->fd, &opCode, sizeof(int), 0);
}

void atenderBlocked() {
	while (1) {
		sem_wait(&bloqueado);
		log_info(logger, "Bloqueando.");
		running = NULL;
		sem_post(&replanificar);

		pthread_mutex_lock(&mutexColaBloqueados);
		t_pcb* bloqued = queue_pop(colaBloqueados);
		uint32_t* bloquedTimeInMili = queue_pop(colaBloqueados);
		pthread_mutex_unlock(&mutexColaBloqueados);

	log_info(logger, "Proceso N°: %d, Bloqueado por %dms. A dormir", bloqued->pid,*bloquedTimeInMili);
	usleep(*bloquedTimeInMili * 1000);
	log_info(logger, "Proceso N°: %d, Se desperto con estado: %d", bloqued->pid, bloqued->suspendido);
		

		if (bloqued->suspendido == 2) {
			log_info(logger,
			"Proceso deja de estar Suspendido-Blocked  (N°: %d).",
			bloqued->pid);
			bloqued->suspendido = 0;
			pthread_mutex_lock(&mutexColaReadySuspendidos);
			queue_push(colaReadySuspendidos, bloqued);
			pthread_mutex_unlock(&mutexColaReadySuspendidos);
		} else{
			bloqued->suspendido = 0;
			//pasarlo a ready
			log_info(logger,"Proceso para ready  (N°: %d).",
			bloqued->pid);
			int a = queue_is_empty(colaReady);
			log_info(logger," Esta vacia?. %d", a );
			pthread_mutex_lock(&mutexColaReady);
			queue_push(colaReady, bloqued);
			a = queue_is_empty(colaReady);
			log_info(logger," Esta vacia?. %d", a);
			pthread_mutex_unlock(&mutexColaReady);
		} 
		log_info(logger, "Proceso Desbloqueado - Senial a Ready");
		sem_post(&ready);
	}
}


void* timer(t_pcb* running){
	t_pcb* pcbAChequear = running; 
	printf("Timer del proceso N°: %d, Con Suspendido N°: %d \n", pcbAChequear->pid, pcbAChequear->suspendido);
	usleep(tiempoMaximoBloqueado*1000);

	if(pcbAChequear->suspendido == 1){
		pcbAChequear->suspendido = 2;
		sem_post(&semGradoMultiprogramacion);
			//TODO - TEST - llamarMemoria - avisar a memoria suspenderProceso
			/*******************************************************************/
			log_info(logger, "Solicitando suspenderProceso N°: %d a Memoria.", pcbAChequear->pid);
			notificarMemoria(pcbAChequear->tabla_paginas, SUSPEND_PROCESS);
			log_info(logger, "Solicitando suspenderProceso a Memoria. OK.");

			log_info(logger, "Esperando respuesta suspenderProceso a Memoria.");
			recibir_operacion(conexion_a_memoria);
			log_info(logger, "Esperando respuesta suspenderProceso a Memoria. OK.");
			/*******************************************************************/

			log_info(logger, "Proceso Suspendido-Blocked (N°: %d).",
					pcbAChequear->pid);
	}

	return NULL;
} 


void atenderCPU() {
	double rafagaEnSegundos;
	double rafagaEnMiliSegundos;
	while (1) {
		int cod_op = recibir_operacion(conexion_a_cpu);
		log_info(logger, "Llego PCB desde CPU - ProceosN°: %d", running->pid);
		switch (cod_op) {
		case TERMINATE:
			;
			uint32_t bloquedTime;
			recibirPCBdesdeSocket(running, &bloquedTime, conexion_a_cpu);

			log_info(logger, "Proceso N°: %d  - Terminado", running->pid);

			//TODO - TEST - llamarMemoria - finalizarProceso
			/***********************************************************************/
			log_info(logger, "Solicitando finalizarProceso a Memoria.");
			notificarMemoria(running->tabla_paginas, END_PROCESS);
			log_info(logger, "Solicitando finalizarProceso a Memoria. OK.");

			log_info(logger, "Esperando respuesta finalizarProceso a Memoria.");
			recibir_operacion(conexion_a_memoria);
			log_info(logger, "Esperando respuesta finalizarProceso a Memoria. OK.");
			/*******************************************************************/

			//Avisar A Consola
			notificarConsola(running->pid);

			//liberar_pcb(pcb);
			free(running);
			running = NULL;

			sem_post(&semGradoMultiprogramacion);
			sem_post(&replanificar);
			break;
		case BLOCKED:
			;
			uint32_t* bloquedTimeInMiliseconds = malloc(sizeof(uint32_t));
			recibirPCBdesdeSocket(running, bloquedTimeInMiliseconds,
					conexion_a_cpu);
			log_info(logger, "BLOQUEO POR IO ms: %d  | Proceso N°: %d",
					*bloquedTimeInMiliseconds, running->pid);


			pthread_t manejarSusp;
			pthread_create(&manejarSusp,NULL,timer,running);


			//Actualizar rafaga
			toc = clock();
			rafagaEnSegundos = (double) (toc - tic) / CLOCKS_PER_SEC;
			rafagaEnMiliSegundos = rafagaEnSegundos * 1000;
			printf("Rafaga: %f miliSegundos\n", rafagaEnMiliSegundos);
			running->estimacion_rafaga = running->estimacion_rafaga * alfa
					+ rafagaEnMiliSegundos * (1 - alfa);

			pthread_mutex_lock(&mutexColaBloqueados);
			// en realidad sería bloqueado = 1
			running->suspendido=1;
			queue_push(colaBloqueados, running);
			queue_push(colaBloqueados, bloquedTimeInMiliseconds);
			pthread_mutex_unlock(&mutexColaBloqueados);

			sem_post(&bloqueado);
			break;
		case DISPATCHED:
			;
			//aca el bloqued time NO se usa
			int bloquedTimeFoo = 0;
			recibirPCBdesdeSocket(running, &bloquedTimeFoo, conexion_a_cpu);

			//Actualizar rafaga
			toc = clock();
			rafagaEnSegundos = (double) (toc - tic) / CLOCKS_PER_SEC;
			rafagaEnMiliSegundos = rafagaEnSegundos * 1000;
			printf("Rafaga: %f miliSegundos\n", rafagaEnMiliSegundos);
			running->estimacion_rafaga = running->estimacion_rafaga * alfa
					+ rafagaEnMiliSegundos * (1 - alfa);

			//CPU ---> dispatch(PCB) ---> Kernel (ponerlo en ready)
			log_info(logger, "....LockReady");
			pthread_mutex_lock(&mutexColaReady);
			log_info(logger, "Tamaño cola ready: %d", queue_size(colaReady));
			queue_push(colaReady, running);
			log_info(logger, "Tamaño cola ready: %d", queue_size(colaReady));
			pthread_mutex_unlock(&mutexColaReady);
			log_info(logger, "....LockReady.OK");

			running = NULL;
			sem_post(&replanificar);

			break;
		case -1:
			log_error(logger, "El cliente se desconecto. Terminando servidor");
			return;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
	}
}

void gestionarColaReadys() {
	while (1) {
		log_info(logger, "Esperando señal Ready.");
		sem_wait(&ready);
		//sem_wait(&semGradoMultiprogramacion);

		t_pcb* toReady;

		pthread_mutex_lock(&mutexColaReadySuspendidos);
		if (!queue_is_empty(colaReadySuspendidos)) {
			log_info(logger, "Busco en ready suspendios --> OK.");
			toReady = queue_pop(colaReadySuspendidos);
			pthread_mutex_unlock(&mutexColaReadySuspendidos);

			sem_wait(&semGradoMultiprogramacion);
			log_info(logger, "....LockReady");
			pthread_mutex_lock(&mutexColaReady);
			queue_push(colaReady, toReady);
			pthread_mutex_unlock(&mutexColaReady);
			log_info(logger, "....LockReady.OK");
			log_info(logger, "PUSH colaReady.OK");
		} else {
			pthread_mutex_unlock(&mutexColaReadySuspendidos);
			pthread_mutex_lock(&mutexColaNew);
			if (!queue_is_empty(colaNew)) {
				log_info(logger, "Busco en New --> OK.");
				//puedePasarAReady
				toReady = queue_pop(colaNew);
				pthread_mutex_unlock(&mutexColaNew);

				sem_wait(&semGradoMultiprogramacion);
				// TODO - TEST - llamarMemoria - inicializar proceso en momoria / guardar resp
				/**************************************************************************/
				log_info(logger, "Solicitando inicializarProceso a Memoria.");
				notificarMemoriaInitProcess(toReady->pid, toReady->tamanio, INIT_PROCESS);
				log_info(logger, "Solicitando inicializarProceso a Memoria. OK.");
				log_info(logger, "Proceso N°: %d", toReady->pid);

				log_info(logger, "Esperando respuesta inicializarProceso a Memoria.");
				recibir_operacion(conexion_a_memoria);
				int size;
				void *buffer = recibir_buffer(&size, conexion_a_memoria);
				int offset = 0;
				memcpy(&(toReady->tabla_paginas), buffer + offset, sizeof(uint32_t));
				free(buffer);
				log_info(logger, "toReady->tabla_paginas: %d", toReady->tabla_paginas);
				log_info(logger, "Esperando respuesta inicializarProceso a Memoria. OK.");
				/**************************************************************************/

				log_info(logger, "....LockReady");
				pthread_mutex_lock(&mutexColaReady);
				queue_push(colaReady, toReady);
				pthread_mutex_unlock(&mutexColaReady);
				log_info(logger, "....LockReady.OK");
			} else {
				pthread_mutex_unlock(&mutexColaNew);
				//FIXME
				//sem_post(&semGradoMultiprogramacion);
			}

			/*
			if (esFIFO && init == true) {
				init = false;
				//sem_wait(&semGradoMultiprogramacion);
				sem_post(&replanificar);
			} else if (esFIFO && running == NULL) {
				//sem_wait(&semGradoMultiprogramacion);
				//FIXME
				//sem_post(&replanificar);
			} else if (!esFIFO) {
				//sem_wait(&semGradoMultiprogramacion);
				sem_post(&replanificar);
			}
			*/
		}

		log_info(logger, "REPLANIFICA");

		sem_post(&replanificar);

	}
}

void planificadorCortoPlazo() {
	while (1) {
		log_info(logger, "Esperando señal replanificadora");
		sem_wait(&replanificar);
		log_info(logger, "Replanificando.OK");

		t_pcb* toRun;

		bool encontro = false;
		if (esFIFO) {
			log_info(logger, "pase por aca");
			if (running == NULL) {
				log_info(logger, "FIFO.");
				log_info(logger, "Busco en ready.");
				log_info(logger, "....LockReady");
				pthread_mutex_lock(&mutexColaReady);
				if (!queue_is_empty(colaReady)) {
					log_info(logger, "Busco en ready Ok.");
					encontro = true;
					log_info(logger, "Tamaño cola ready: %d",
							queue_size(colaReady));
					toRun = queue_pop(colaReady);
					log_info(logger, "Tamaño cola ready: %d",
							queue_size(colaReady));
				}
				pthread_mutex_unlock(&mutexColaReady);
				log_info(logger, "....LockReady.OK");
				if (encontro) {
					log_info(logger, "Corriendo: PID: %d", toRun->pid);
					t_paquete *paquete = crearPaquetePCB(toRun, EXEC);
					tic = clock();
					enviar_paquete(paquete, conexion_a_cpu);
					eliminar_paquete(paquete);
					running = toRun;
					log_info(logger, "Paquete PCB enviado a CPU.");
				} else {
					//running = NULL;
					log_info(logger, "Nada para poner a correr.");
				}
			}
		} else {
			log_info(logger, "SJF.");

			//kernel ---> interrpt --> CPU
			log_info(logger, "....LockReady");
			pthread_mutex_lock(&mutexColaReady);
			if (running == NULL) {
				if (!queue_is_empty(colaReady)) {
					log_info(logger, "Se usa funcion SJF y se envia a CPU");
					running = siguientePCBporSJF(colaReady->elements);
					t_paquete *paquete = crearPaquetePCB(running, EXEC);
					tic = clock();
					enviar_paquete(paquete, conexion_a_cpu);
					eliminar_paquete(paquete);
				} else {
					log_info(logger, "OCEOSO.");
				}
			} else {
				if (!queue_is_empty(colaReady)) {

					log_info(logger, "INTERRUPIR al CPU.");

					int opCode = INTERRUPT;
					send(conexion_a_cpu_interrupt, &opCode, sizeof(int), 0);

					log_info(logger, "Tamaño cola ready: %d",
							queue_size(colaReady));
				}
			}
			pthread_mutex_unlock(&mutexColaReady);
			log_info(logger, "....LockReady.OK");

		}
	}
}

void atenderCliente(int socket) {
	int cliente_fd = socket;
	int cod_op = recibir_operacion(cliente_fd);
	switch (cod_op) {
	case NEW:
		;
		log_info(logger, "Llegó un Programa");
		int size;
		void* buffer = recibir_buffer(&size, cliente_fd);
		t_program programa;
		int offset = 0;
		memcpy(&programa.programSize, buffer + offset, sizeof(uint32_t));

		int cantidadInstr = (size - sizeof(uint32_t)) / sizeof(t_inst);
		offset += sizeof(uint32_t);

		programa.instrucciones = list_create();
		t_inst* instr;
		int j = 0;
		while (j < cantidadInstr) {
			instr = malloc(sizeof(t_inst));
			memcpy(instr, buffer + offset, sizeof(t_inst));
			offset += sizeof(t_inst);
			list_add(programa.instrucciones, instr);
			j++;
		}
		free(buffer);

		t_pcb* pcb = crearPCB(programa.instrucciones);
		pthread_mutex_lock(&mutexColaNew);
		queue_push(colaNew, pcb);
		pthread_mutex_unlock(&mutexColaNew);
		log_info(logger, "Agregada la PCB a la cola NEW");

		//guardo el socket
		pid_fd* elem = malloc(sizeof(pid_fd));
		elem->fd = cliente_fd;
		pthread_mutex_lock(&mutexNextPID);
		elem->pid = nextPID;
		pthread_mutex_unlock(&mutexNextPID);

		//guardo el PID
		pthread_mutex_lock(&mutexListaPidWithFd);
		list_add(pidWithFd, elem);
		pthread_mutex_unlock(&mutexListaPidWithFd);

		pthread_mutex_lock(&mutexNextPID);
		nextPID++;
		pthread_mutex_unlock(&mutexNextPID);

		sem_post(&ready);
		break;
	case -1:
		log_error(logger, "El cliente se desconecto. Terminando servidor");
		return;
	default:
		log_warning(logger, "Operacion desconocida.");
		break;
	}
}

int main(int argc, char **argv) {
	logger = log_create("./kernel.log", "kernel", true, LOG_LEVEL_INFO);

	t_config *config = config_create(argv[1]);
	if (config == NULL) {
		printf(
				"No se pudo iniciar la consola, revise el path del archivo de configuración.");
		exit(2);
	}

	char *algoritmoPlanificacion = config_get_string_value(config,
			"ALGORITMO_PLANIFICACION");
	esFIFO = false;
	if (strcmp(algoritmoPlanificacion, "FIFO") == 0) {
		esFIFO = true;
	}
	estimacionInicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	alfa = config_get_double_value(config, "ALFA");
	gradoMultiprogramacion = config_get_int_value(config,
			"GRADO_MULTIPROGRAMACION");
	tiempoMaximoBloqueado = config_get_int_value(config,
			"TIEMPO_MAXIMO_BLOQUEADO");

	running = NULL;

	nextPID = 1;
	pthread_mutex_init(&mutexNextPID, NULL);

	pidWithFd = list_create();
	pthread_mutex_init(&mutexListaPidWithFd, NULL);

	colaNew = queue_create();
	pthread_mutex_init(&mutexColaNew, NULL);

	colaReady = queue_create();
	pthread_mutex_init(&mutexColaReady, NULL);
	sem_init(&semGradoMultiprogramacion, 0, gradoMultiprogramacion);
	sem_init(&semFinalizoSusoendido, 0, 0);
	sem_init(&ready, 0, 0);

	sem_init(&replanificar, 0, 0);

	sem_init(&bloqueado, 0, 0);

	colaBloqueados = queue_create();
	pthread_mutex_init(&mutexColaBloqueados, NULL);

	colaReadySuspendidos = queue_create();
	pthread_mutex_init(&mutexColaReadySuspendidos, NULL);

	char *ip_conexion_memoria = config_get_string_value(config, "IP_MEMORIA");
	char *puerto_conexion_memoria = config_get_string_value(config,
			"PUERTO_MEMORIA");

	char *ip_server_kernel = config_get_string_value(config, "IP_ESCUCHA");
	char *port_server_kernel = config_get_string_value(config,
			"PUERTO_ESCUCHA");
	char *ip_conexion_cpu = config_get_string_value(config, "IP_CPU");
	char *puerto_conexion_cpu_dispatch = config_get_string_value(config,
			"PUERTO_CPU_DISPATCH");
	char *puerto_conexion_cpu_interrupt = config_get_string_value(config,
			"PUERTO_CPU_INTERRUPT");

	conexion_a_memoria = crear_conexion(ip_conexion_memoria,
			puerto_conexion_memoria);
	conexion_a_cpu_interrupt = crear_conexion(ip_conexion_cpu,
			puerto_conexion_cpu_interrupt);
	conexion_a_cpu = crear_conexion(ip_conexion_cpu,
			puerto_conexion_cpu_dispatch);

	int server_fd = iniciar_servidor(ip_server_kernel, port_server_kernel);
	log_info(logger, "Servidor listo para recibir al cliente");

	pthread_t hiloCortoPlazo;
	pthread_create(&hiloCortoPlazo, NULL, (void*) planificadorCortoPlazo,
	NULL);
	pthread_detach(hiloCortoPlazo);

	pthread_t hiloCPU;
	pthread_create(&hiloCPU, NULL, (void*) atenderCPU, NULL);
	pthread_detach(hiloCPU);

	pthread_t hiloReady;
	pthread_create(&hiloReady, NULL, (void*) gestionarColaReadys, NULL);
	pthread_detach(hiloReady);

	pthread_t hiloBlocked;
	pthread_create(&hiloBlocked, NULL, (void*) atenderBlocked, NULL);
	pthread_detach(hiloBlocked);

	while (1) {
		int cliente_fd = esperar_cliente(server_fd);
		pthread_t hiloCliente;
		pthread_create(&hiloCliente, NULL, (void*) atenderCliente,
				(int*) cliente_fd);
		/*
		 FIXME acá hay que ver cómo liberar la memoria de cada thread. En teorìa no hay nada que hacer:
		 Valgrind simply reports what it "sees" (something was allocated but not de-allocated). But it's not a real leak, so you don't need to worry about this.
		 Ver aca: https://stackoverflow.com/questions/57016280/memory-leaks-in-pthread-even-if-the-state-is-detached
		 */
		pthread_detach(hiloCliente);
	}

	pthread_mutex_destroy(&mutexColaNew);
	pthread_mutex_destroy(&mutexColaReady);
	pthread_mutex_destroy(&mutexColaBloqueados);
	pthread_mutex_destroy(&mutexColaReadySuspendidos);
	pthread_mutex_destroy(&mutexListaPidWithFd);
	pthread_mutex_destroy(&mutexNextPID);

	sem_destroy(&semGradoMultiprogramacion);
	sem_destroy(&semFinalizoSusoendido);
	sem_destroy(&replanificar);
	sem_destroy(&ready);

	liberar_conexion(conexion_a_memoria);
	liberar_conexion(conexion_a_cpu);
	liberar_conexion(conexion_a_cpu_interrupt);

	close(server_fd);
	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

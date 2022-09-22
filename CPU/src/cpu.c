#include "cpu.h"

// la TLB no tiene columna de proceso,
// la misma se tiene que limpiar ante cada cambio de proceso para que no pase eso
// cuando el Kernel cambia qué proceso está en EXEC
void limpiarTLB() {
	int i = 0;
	proximoIndiceTLBLibre = 0;
	while (i < entradasTLB) {
		tlb[i].marco = NULL;
		tlb[i].pagina = NULL;
		tlb[i].instanteDeCarga = NULL;
		tlb[i].tiempoDeUltimaReferencia = NULL;
		i++;
	}
}

void actualizarTLB(uint32_t pagina, void* marco) {
	if (proximoIndiceTLBLibre < entradasTLB) {
		uint32_t proximaPosicionTLB = proximoIndiceTLBLibre;
		tlb[proximaPosicionTLB].pagina = pagina;
		tlb[proximaPosicionTLB].marco = marco;
		tlb[proximaPosicionTLB].instanteDeCarga = clock();
		tlb[proximaPosicionTLB].tiempoDeUltimaReferencia = clock();
		proximoIndiceTLBLibre++;
	} else {
		uint32_t indiceVictima = 0;
		int i = 0;
		while (i < entradasTLB) {
			if (strcmp(algortimoTLB, "FIFO")) {
				if (tlb[i].instanteDeCarga > tlb[indiceVictima].instanteDeCarga) {
					indiceVictima = i;
				}
			} else {
				if (tlb[i].tiempoDeUltimaReferencia > tlb[indiceVictima].tiempoDeUltimaReferencia) {
					indiceVictima = i;
				}
			}
			i++;
		}
		tlb[indiceVictima].pagina = pagina;
		tlb[indiceVictima].marco = marco;
		tlb[indiceVictima].instanteDeCarga = clock();
		tlb[indiceVictima].tiempoDeUltimaReferencia = clock();
	}
}

uint32_t bucarPaginaEnTLB(uint32_t pagina) {
	int i = 0;
	while (i < entradasTLB) {
		log_info(logger, "tlb[i].pagina %d == pagina %d", tlb[i].pagina, pagina);
		if (tlb[i].pagina == pagina) {
			tlb[i].tiempoDeUltimaReferencia = clock();
			return tlb[i].marco;
		}
		i++;
	}
	return NULL;
}

//TODO - TEST - llamarMemoria -  buscarTabla2doNivel
uint32_t buscarTabla2doNivel(uint32_t entradaTabla1erNivel) {

	log_info(logger, "Solicitando buscarTabla2doNivel.");

	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_TABLA_2DO_NIVEL;
	paquete->buffer = buffer;

	buffer->size = 2 * sizeof(uint32_t);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &pcbActual->tabla_paginas, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &entradaTabla1erNivel, sizeof(uint32_t));

	buffer->stream = stream;
	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);

	log_info(logger, "Esperando respuesta buscarTabla2doNivel.");
	uint32_t nroTabla2doNivel;
	int size;
	int cod_op = recibir_operacion(conexion_a_memoria);
	void *bufferRecv = recibir_buffer(&size, conexion_a_memoria);
	offset = 0;
	memcpy(&nroTabla2doNivel, bufferRecv + offset, sizeof(uint32_t));
	log_info(logger, "nroTabla2doNivel: %d\n", nroTabla2doNivel);
	free(bufferRecv);

	return nroTabla2doNivel;
}

//TODO - TEST - llamarMemoria - buscarMarco
uint32_t buscarMarco(uint32_t nroPagina, uint32_t tabla2doNivel, uint32_t entradaTabla2doNivel) {
	log_info(logger, "Solicitando buscarMarco.");

	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_FRAME;
	paquete->buffer = buffer;
	buffer->size = 4 * sizeof(uint32_t);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &(pcbActual->tabla_paginas), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tabla2doNivel, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &entradaTabla2doNivel, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &nroPagina, sizeof(uint32_t));
	buffer->stream = stream;

	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);

	uint32_t marco;
	int size;
	int cod_op = recibir_operacion(conexion_a_memoria);
	void *bufferRecv = recibir_buffer(&size, conexion_a_memoria);
	offset = 0;
	memcpy(&marco, bufferRecv + offset, sizeof(uint32_t));
	free(bufferRecv);
	return marco;
}

void copiarDato(void* direccionFisica, uint32_t dato) {
	log_info(logger, "Solicitando copiarDato.");
	log_info(logger, "Dato a copiar: %d", dato);
	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = SAVE_DATA;
	paquete->buffer = buffer;
	buffer->size = 2 * sizeof(void*);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &direccionFisica, sizeof(void*));
	offset += sizeof(void*);
	memcpy(stream + offset, &dato, sizeof(void*));
	buffer->stream = stream;
	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);
	recibir_operacion(conexion_a_memoria);

}
//TODO - TEST - llamarMemoria - bucarDato
uint32_t bucarDato(void* direccionFisica) {
	//Finalmente acceder a la porción de memoria correspondiente (la dirección física).
	//desplazamiento = direccion_logica - numero_pagina * tamanio_pagina
	log_info(logger, "Solicitando bucarDato.");
	t_buffer *buffer = malloc(sizeof(t_buffer));
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_DATA;
	paquete->buffer = buffer;
	buffer->size = 1 * sizeof(void*);
	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &direccionFisica, sizeof(void*));
	buffer->stream = stream;
	enviar_paquete(paquete, conexion_a_memoria);
	eliminar_paquete(paquete);

	log_info(logger, "Esperando respuesta bucarDato.");
	uint32_t dato;
	int size;
	int cod_op = recibir_operacion(conexion_a_memoria);
	void *bufferRecv = recibir_buffer(&size, conexion_a_memoria);
	offset = 0;
	memcpy(&dato, bufferRecv + offset, sizeof(uint32_t));
	log_info(logger, "Dato: %d\n", dato);
	free(bufferRecv);

	return dato;
}

uint32_t traducirDireccion(uint32_t direccionLogica) {
	log_info(logger, "traducirDireccion");
	log_info(logger, "direccionLogica: %d", direccionLogica);
	uint32_t nroPagina = floor(direccionLogica / tamanioPagina);
	log_info(logger, "nroPagina = %d / %d = %d", direccionLogica, tamanioPagina, nroPagina);
	uint32_t entradaTabla1erNivel = floor(nroPagina / cantEntradasPorTabla);
	log_info(logger, "entradaTabla1erNivel: %d", entradaTabla1erNivel);
	uint32_t entradaTabla2doNivel = nroPagina % cantEntradasPorTabla;
	log_info(logger, "entradaTabla2doNivel: %d", entradaTabla2doNivel);
	uint32_t desplazamiento = direccionLogica - nroPagina * tamanioPagina;
	log_info(logger, "desplazamiento: %d", desplazamiento);

	uint32_t direccionMarco = bucarPaginaEnTLB(nroPagina);
	if (direccionMarco == NULL) {
		log_info(logger, "NO ESTA EN TLB");
		uint32_t nroTabla2doNivel = buscarTabla2doNivel(entradaTabla1erNivel);
		direccionMarco = buscarMarco(nroPagina, nroTabla2doNivel, entradaTabla2doNivel);
		actualizarTLB(nroPagina, direccionMarco);
	} else {
		log_info(logger, "SI ESTA EN TLB");
	}
	log_info(logger, "Marco: %d\n", direccionMarco);

	return direccionMarco + desplazamiento;
}

t_paquete* crearPaquetePCB(t_pcb* pcb, uint32_t* bloquedTime, op_code opCode) {
	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = 6 * sizeof(uint32_t)
			+ list_size(pcb->instrucciones) * sizeof(t_inst)
			+ 1 * sizeof(uint32_t);
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

	memcpy(stream + offset, bloquedTime, sizeof(uint32_t)), buffer->stream =
			stream;

	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = opCode;
	paquete->buffer = buffer;
	return paquete;
}

void cicloInstruccion(int* cliente_fd) {
	uint32_t direccionLogica, direccionFisica, dato;
	t_paquete *paquete;
	t_list* instrucciones = pcbActual->instrucciones;
	while (pcbActual->program_counter < pcbActual->tamanio) {
		log_info(logger, "Program counter: %d Tamanio total: %d  N°Proceso: %d \n", pcbActual->program_counter, pcbActual->tamanio, pcbActual->pid);

		t_inst * instruccionAEjecutar = list_get(instrucciones,
				pcbActual->program_counter);
		switch (instruccionAEjecutar->instCode) {
		case NOP:
			log_info(logger, "NOP. | Del proceso N°: %d", pcbActual->pid );
			usleep(retardoNoOp * 1000);
			break;
		case IO:
			log_info(logger, "IO. | Del proceso N°: %d", pcbActual->pid );
			pcbActual->program_counter = pcbActual->program_counter + 1;

			paquete = crearPaquetePCB(pcbActual,
					&(instruccionAEjecutar->operators[0]), BLOCKED);
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);

			log_info(logger, "ENVIADO a Kernel x IO | Del proceso N°: %d", pcbActual->pid );
			return;
		case READ:
			log_info(logger, "READ. | Del proceso N°: %d", pcbActual->pid );
			direccionLogica = instruccionAEjecutar->operators[0];
			direccionFisica = traducirDireccion(direccionLogica);
			dato = bucarDato(direccionFisica);
			break;
		case COPY:
			log_info(logger, "COPY. | Del proceso N°: %d", pcbActual->pid );

			direccionLogica = instruccionAEjecutar->operators[1];
			direccionFisica = traducirDireccion(direccionLogica);
			dato = bucarDato(direccionFisica);

			direccionLogica = instruccionAEjecutar->operators[0];
			direccionFisica = traducirDireccion(direccionLogica);
			copiarDato(direccionFisica, dato);

			break;
		case WRITE:
			log_info(logger, "WRITE. | Del proceso N°: %d", pcbActual->pid );
			direccionLogica = instruccionAEjecutar->operators[0];
			direccionFisica = traducirDireccion(direccionLogica);
			dato = instruccionAEjecutar->operators[1];
			copiarDato(direccionFisica, dato);
			break;
		case EXIT:
			log_info(logger, "EXIT. | Del proceso N°: %d", pcbActual->pid );
			paquete = malloc(sizeof(t_paquete));

			paquete = crearPaquetePCB(pcbActual,
					&(instruccionAEjecutar->operators[0]), TERMINATE);
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			return;
		default:
			log_info(logger, "INSTRUCCION DESCONOCIDA.");
			break;
		}
		pcbActual->program_counter = pcbActual->program_counter + 1;

		pthread_mutex_lock(&mutexCheckInterrupt);
		if (checkInterrupt == true && instruccionAEjecutar->instCode != IO
				&& instruccionAEjecutar->instCode != EXIT) {
			checkInterrupt = false;
			pthread_mutex_unlock(&mutexCheckInterrupt);

			log_info(logger, "Enviando a Kernel x Interrupcion | N°Proceso: %d .", pcbActual->pid);

			paquete = crearPaquetePCB(pcbActual,
					&(instruccionAEjecutar->operators[0]), DISPATCHED);
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			return;
		} else {
			pthread_mutex_unlock(&mutexCheckInterrupt);
		}
	}
}

void atenderInterrupt(int socket) {
	int cliente_fd = socket;
	while (1) {
//		int cod_op;
//		recv(cliente_fd, &cod_op, sizeof(int), MSG_WAITALL);
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case INTERRUPT:
			log_info(logger, "Llegó INTERRUPT.");
			pthread_mutex_lock(&mutexCheckInterrupt);
			checkInterrupt = true;
			pthread_mutex_unlock(&mutexCheckInterrupt);
			break;
		case -1:
			log_error(logger, "El cliente se desconecto.");
			return;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
	}
}

void atenderDispatch(int socket) {
	int cliente_fd = socket;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case EXEC:
			limpiarTLB();
			recibirPCBdesdeSocket(pcbActual, cliente_fd);
			log_info(logger, "Llego PCB al CPU | N°Proceso: %d ", pcbActual->pid);

			pthread_t hiloCicloInstruccion;
			pthread_create(&hiloCicloInstruccion, NULL,
					(void*) cicloInstruccion, cliente_fd);
			pthread_detach(hiloCicloInstruccion);

			break;
		case -1:
			log_error(logger, "El cliente se desconecto.");
			return;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
	}
}

void server_controller(void* _args) {
	thread_args *args = (thread_args *) _args;
	while (1) {
		log_info(logger, "Esperando cliente....");
		int cliente_fd = esperar_cliente(args->first);
		pthread_t hiloCliente;
		pthread_create(&hiloCliente, NULL, (void *) args->second,
				(int *) cliente_fd);
		pthread_detach(hiloCliente);
	}
}

int main(int argc, char **argv) {
	logger = log_create("./cpu.log", "CPU", true, LOG_LEVEL_INFO);

	t_config *config = config_create(argv[1]);
	if (config == NULL) {
		log_info(logger,
				"No se pudo iniciar el CPU, revise el path del archivo de configuración.");
		exit(2);
	}

	pthread_mutex_init(&mutexCheckInterrupt, NULL);
	checkInterrupt = false;

	retardoNoOp = config_get_int_value(config, "RETARDO_NOOP");

	char *ip_conexion_memoria = config_get_string_value(config, "IP_MEMORIA");
	char *puerto_conexion_memoria = config_get_string_value(config,
			"PUERTO_MEMORIA");

	char *ip_server_cpu = config_get_string_value(config, "IP_ESCUCHA");
	char *port_dispatch_server_cpu = config_get_string_value(config,
			"PUERTO_ESCUCHA_DISPATCH");
	char *port_interrupt_server_cpu = config_get_string_value(config,
			"PUERTO_ESCUCHA_INTERRUPT");

	entradasTLB = config_get_int_value(config,
					"ENTRADAS_TLB");
	tlb = (t_tlb*) malloc(entradasTLB * sizeof(t_tlb));

	algortimoTLB = config_get_string_value(config,
				"REEMPLAZO_TLB");

	conexion_a_memoria = crear_conexion(ip_conexion_memoria,
			puerto_conexion_memoria);

	int opCode = GET_MEM_CONFIG;
	log_info(logger, "Pidiendo config a memoria.");
	send(conexion_a_memoria, &opCode, sizeof(int), 0);

	log_info(logger, "Esperando respuesta.");
	int size;
	int cod_op = recibir_operacion(conexion_a_memoria);
	void *bufferRecv = recibir_buffer(&size, conexion_a_memoria);
	int offset = 0;
	memcpy(&tamanioPagina, bufferRecv + offset, sizeof(uint32_t));
	log_info(logger, "Tamaño pagina: %d\n", tamanioPagina);
	offset += sizeof(uint32_t);
	memcpy(&cantEntradasPorTabla, bufferRecv + offset, sizeof(uint32_t));
	log_info(logger, "Entradas por Tabla: %d\n", cantEntradasPorTabla);
	free(bufferRecv);


//	pthread_mutex_init(&mutexPcbActual, NULL);
	pcbActual = malloc(sizeof(t_pcb));

	int server_dispatch = iniciar_servidor(ip_server_cpu,
			port_dispatch_server_cpu);
	int server_interrupt = iniciar_servidor(ip_server_cpu,
			port_interrupt_server_cpu);

	thread_args *argsDispatch = malloc(sizeof(thread_args));
	argsDispatch->first = server_dispatch;
	argsDispatch->second = atenderDispatch;

	pthread_t hiloDispatch;
	pthread_create(&hiloDispatch, NULL, (void *) server_controller,
			argsDispatch);
	pthread_detach(hiloDispatch);

	thread_args *argsInterrupt = malloc(sizeof(thread_args));
	argsInterrupt->first = server_interrupt;
	argsInterrupt->second = atenderInterrupt;

	pthread_t hiloInterrupt;
	pthread_create(&hiloInterrupt, NULL, (void *) server_controller,
			argsInterrupt);
	pthread_detach(hiloInterrupt);

	while (1) {
		sleep(1);
	}

	pthread_mutex_destroy(&mutexCheckInterrupt);
	//pthread_mutex_destroy(&mutexPcbActual);

	liberar_conexion(conexion_a_memoria);
	close(server_dispatch);
	close(server_interrupt);
	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

void recibirPCBdesdeSocket(t_pcb* pcbActual, int* cliente_fd) {
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
	free(buffer);
}

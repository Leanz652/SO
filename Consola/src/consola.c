#include "consola.h"

void liberar_programa(t_program* programa) {
	list_destroy_and_destroy_elements(programa->instrucciones, free);
	free(programa);
}

t_paquete* crear_paquete_programa(t_program* programa) {
	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t)
			+ list_size(programa->instrucciones) * sizeof(t_inst);

	void *stream = malloc(buffer->size);
	int offset = 0;
	memcpy(stream + offset, &(programa->programSize), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//serializarInstrucciones();
	int i = 0;
	while (i < list_size(programa->instrucciones)) {
		memcpy(stream + offset, list_get(programa->instrucciones, i),
				sizeof(t_inst));
		offset += sizeof(t_inst);
		i++;
	}
	buffer->stream = stream;

	//lleno el paquete
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW;
	paquete->buffer = buffer;
	return paquete;
}

t_program* crearPrograma(int programSize) {
	t_program *programa = malloc(sizeof(t_program));
	programa->programSize = programSize;
	programa->instrucciones = list_create();
	return programa;
}

FILE* abirArchivo(char *filename) {
	if (filename == NULL) {
		log_error(logger, "Error: debe informar un archivo de instrucciones.");
		exit(1);
	}
	return fopen(filename, "r");
}

void agregarInstruccionesDesdeArchivo(FILE* instructionsFile,
		t_list *instrucciones) {
	if (instructionsFile == NULL) {
		log_error(logger,
				"Error: no se pudo abrir el archivo de instrucciones.");
		exit(1);
	}
	const unsigned MAX_LENGTH = 256;
	char buffer[MAX_LENGTH];
	while (fgets(buffer, 256, instructionsFile) != NULL) {
		int i = 0;
		char **palabra = string_split(buffer, " ");
		t_inst *instr;
		if (strcmp(palabra[0], "NO_OP") == 0) {
			u_int32_t cantidadDeNOP = atoi(palabra[1]);
			while (cantidadDeNOP > 0) {
				instr = malloc(sizeof(t_inst));
				instr->instCode = NOP;
				instr->operators[0] = NULL;
				instr->operators[1] = NULL;
				cantidadDeNOP--;
				list_add(instrucciones, instr);
			}
			free(palabra[0]);
			free(palabra[1]);
		} else {
			t_inst *instr = malloc(sizeof(t_inst));
			if (strcmp(palabra[0], "I/O") == 0) {
				instr->instCode = IO;
				instr->operators[0] = atoi(palabra[1]);
				instr->operators[1] = NULL;
				free(palabra[0]);
				free(palabra[1]);
			} else if (strcmp(palabra[0], "READ") == 0) {
				instr->instCode = READ;
				instr->operators[0] = atoi(palabra[1]);
				instr->operators[1] = NULL;
				free(palabra[0]);
				free(palabra[1]);
			} else if (strcmp(palabra[0], "COPY") == 0) {
				instr->instCode = COPY;
				instr->operators[0] = atoi(palabra[1]);
				instr->operators[1] = atoi(palabra[2]);
				free(palabra[0]);
				free(palabra[1]);
				free(palabra[2]);
			} else if (strcmp(palabra[0], "WRITE") == 0) {
				instr->instCode = WRITE;
				instr->operators[0] = atoi(palabra[1]);
				instr->operators[1] = atoi(palabra[2]);
				free(palabra[0]);
				free(palabra[1]);
				free(palabra[2]);
			} else if (strcmp(palabra[0], "EXIT") == 0) {
				instr->instCode = EXIT;
				instr->operators[0] = NULL;
				instr->operators[1] = NULL;
				free(palabra[0]);
			}
			list_add(instrucciones, instr);
		}
		free(palabra);
	}
	fclose(instructionsFile);
	log_info(logger, "Se parsearon #Instrucciones: %d",
			list_size(instrucciones));
}

int main(int argc, char **argv) {
	char *ip;
	char *puerto;
	int conexion;

	logger = log_create("./consola.log", "consola", true, LOG_LEVEL_INFO);

	t_config *config = config_create(argv[1]);
	if (config == NULL) {
		log_info(logger,
				"No se pudo iniciar la consola, revise el path del archivo de configuración.");
		exit(2);
	}
	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");
	log_info(logger, "Se cargó la url del Kernel: %s:%s", ip, puerto);

	conexion = crear_conexion(ip, puerto);
	if (conexion == -1) {
		log_info(logger, "No se pudo conectar al Kernel.");
		log_destroy(logger);
		config_destroy(config);
		exit(2);
	}

	FILE *instructionsFile = abirArchivo(argv[3]);

	t_program *programa = crearPrograma(atoi(argv[2]));
	agregarInstruccionesDesdeArchivo(instructionsFile, programa->instrucciones);

	t_paquete *paquete = crear_paquete_programa(programa);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
	liberar_programa(programa);

	//esperar la respuesta del Kernel.
	int size;
	log_info(logger, "Esperando Kernel");
	recv(conexion, &size, sizeof(int), MSG_WAITALL);
	log_info(logger, "Esperando Kernel OK");

	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(config);

}

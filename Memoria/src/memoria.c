#include "memoria.h"
#include "stdlib.h"

t_paquete* crearPaquete(op_code codOp, uint32_t tamanio) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codOp;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = tamanio;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	return paquete;
}

void disponibilizarMarco(void* marco) {
	uint32_t i = 0;
	while (i < list_size(listaDeMarcos)) {
		t_info_marco* infoMarco = list_get(listaDeMarcos, i);
		if (infoMarco->marco == marco) {
			infoMarco->disponible = true;
			infoMarco->nroTabla1erNivel = NULL;
			break;
		}
		i++;
	}
}

void* obtenerNuevoMarcoMemoriaUsuario(uint32_t nroTabla1erNivel) {
	uint32_t i = 0;
	while (i < list_size(listaDeMarcos)) {
		t_info_marco* infoMarco = list_get(listaDeMarcos, i);
		if (infoMarco->disponible == true) {
			infoMarco->disponible = false;
			infoMarco->nroTabla1erNivel = nroTabla1erNivel;
			return infoMarco->marco;
		}
		i++;
	}
	return NULL;
}

void marcarPaginaComoModificada(void* direccionFisica) {
	void* marco = ((int) direccionFisica) - ((int) direccionFisica % tamPagina);
	uint32_t i = 0;
	while (i < list_size(listaDeMarcos)) {
		t_info_marco* infoMarco = list_get(listaDeMarcos, i);
		if (infoMarco->marco == marco) {
			t_mem_proceso* memProceso = list_get(tablasDe1erNivel,
					infoMarco->nroTabla1erNivel);

			uint32_t j = 0;
			while (j < list_size(memProceso->tablaDeMarcosAsignados)) {
				t_pagina_en_memoria* paginaEnMemoria = list_get(
						memProceso->tablaDeMarcosAsignados, i);
				if (paginaEnMemoria->marco == marco) {
					paginaEnMemoria->modificado = true;

					uint32_t indiceTabla1erNivel = paginaEnMemoria->nroDePagina
							/ entradasPorTabla;
					uint32_t indiceTabla2doNivel = paginaEnMemoria->nroDePagina
							% entradasPorTabla;
					uint32_t* nroTabla2doNivel = list_get(
							memProceso->tablaDe1erNivel, indiceTabla1erNivel);
					t_list* tabla2doNivel = list_get(tablasDe2doNivel,
							*nroTabla2doNivel);
					t_entrada2doNivel* entrada2doNivel = list_get(tabla2doNivel,
							indiceTabla2doNivel);
					entrada2doNivel->modificado = true;
					break;
				}
				j++;
			}
			break;
		}
		i++;
	}
}

int indexClock = 0;
//TODO - 00 - implementar algortimos clock y clockModificado
void elegirPaginaVictima(t_list* listaDeMarcosEnMemoria,
		uint32_t* nroTabla2doNivelVictima, uint32_t* nroPaginaASalvar,
		uint32_t* indiceEntrada2doNivelVictima) {

	/*
	 int i = 0;
	 while (i < list_size(listaDeMarcosEnMemoria)) {
	 t_pagina_en_memoria* paginaEnMemoria = list_get(listaDeMarcosEnMemoria,
	 i);
	 if (paginaEnMemoria->presente = true) {
	 *nroPaginaASalvar = paginaEnMemoria->nroDePagina;
	 *indiceEntrada2doNivelVictima =
	 paginaEnMemoria->indiceEntrada2doNivel;
	 *nroTabla2doNivelVictima = paginaEnMemoria->nroTabla2doNivel;
	 break;
	 }
	 i++;
	 }
	 */

	bool found = false;
	int cantMarcos = list_size(listaDeMarcosEnMemoria) - 1;

	//CLOCK

	if (strcmp(algoritmoReemplazo, "CLOCK") == 0) {
		log_info(logger, "-----------CLOCK----------------");

		while (!found) {
			if (indexClock > cantMarcos) {
				indexClock = 0;
			}
			t_pagina_en_memoria* paginaActual = list_get(listaDeMarcosEnMemoria,
					indexClock);
			if (paginaActual->uso) {
				paginaActual->uso = false;
			} else {
				//return
				found = true;
				*nroPaginaASalvar = paginaActual->nroDePagina;
				*indiceEntrada2doNivelVictima =
						paginaActual->indiceEntrada2doNivel;
				*nroTabla2doNivelVictima = paginaActual->nroTabla2doNivel;

			}
			indexClock++;
		}
	} else {
		log_info(logger, "-----------CLOCK-M----------------");
		//CLOCK MODIFICADO
		int vueltas = 1;
		int indexVuelta = indexClock % (cantMarcos + 1);

		while (!found) {
			t_pagina_en_memoria* paginaActual = list_get(listaDeMarcosEnMemoria,
					indexClock % (cantMarcos + 1));

			if (vueltas % 2 == 1) {
				if (!paginaActual->uso && !paginaActual->modificado) {
					//return
					found = true;
					*nroPaginaASalvar = paginaActual->nroDePagina;
					*indiceEntrada2doNivelVictima =
							paginaActual->indiceEntrada2doNivel;
					*nroTabla2doNivelVictima = paginaActual->nroTabla2doNivel;
				}
			} else {
				if (!paginaActual->uso && paginaActual->modificado) {
					//return
					found = true;
					*nroPaginaASalvar = paginaActual->nroDePagina;
					*indiceEntrada2doNivelVictima =
							paginaActual->indiceEntrada2doNivel;
					*nroTabla2doNivelVictima = paginaActual->nroTabla2doNivel;
				} else {
					paginaActual->uso = false;
				}
			}
			indexClock++;
			if (indexClock % (cantMarcos + 1) == indexVuelta) {
				vueltas++;
			}
		}
	}

}

uint32_t dameIndiceEntradaTabla1erNivel(t_mem_proceso* memProceso,
		uint32_t nroTabla2doNivel) {
	uint32_t indiceEntradaTabla1erNivel;
	int cant = list_size(memProceso->tablaDe1erNivel);
	int i = 0;
	while (i < cant) {
		uint32_t* entrada1erNivel = list_get(memProceso->tablaDe1erNivel, i);
		if (*entrada1erNivel == nroTabla2doNivel) {
			indiceEntradaTabla1erNivel = i;
			break;
		}
		i++;
	}
	return indiceEntradaTabla1erNivel;
}

char* crearSWAP(uint32_t tamanioPrograma, uint32_t pid) {
	char *extension = ".swap";
	char *pID = malloc(20);
	sprintf(pID, "%d", pid);
	strcat(pID, extension);
	FILE *fp = fopen(pID, "w");
	fseek(fp, tamanioPrograma, SEEK_SET);
	fputc('\0', fp);
	fclose(fp);
	return pID;
}

void moverDeMemoriaASWAP(uint32_t numeroPagina, uint32_t nroTabla1erNivel,
		uint32_t nroTabla2doNivel, uint32_t indiceEntrada2doNivel) {
	usleep(retardoSWAP);
	t_mem_proceso* memProceso = list_get(tablasDe1erNivel, nroTabla1erNivel);
//	uint32_t indiceEntradaTabla1erNivel = dameIndiceEntradaTabla1erNivel(
//			memProceso, nroTabla2doNivel);
	t_list* tablaDe2doNivel = list_get(tablasDe2doNivel, nroTabla2doNivel);
	t_entrada2doNivel* entrada2doNivel = list_get(tablaDe2doNivel,
			indiceEntrada2doNivel);

	uint32_t direccionPagina = numeroPagina / tamPagina;

	FILE *fp = fopen(memProceso->nombreArchivoSWAP, "r+");

	uint32_t cantidadDePaginasMemoria = memProceso->tamanioPrograma / tamPagina;
	long swapOffsets[cantidadDePaginasMemoria];

	int i = 0;
	while (i < cantidadDePaginasMemoria) {
		fseek(fp, tamPagina * i, SEEK_SET);
		swapOffsets[i] = ftell(fp);
		i++;
	}

	fseek(fp, swapOffsets[direccionPagina], SEEK_SET);
	fwrite(entrada2doNivel->marco, tamPagina, 1, fp);
	fclose(fp);
}

void moverDeSWAPaMemoria(uint32_t numeroPagina, uint32_t nroTabla1erNivel,
		uint32_t nroTabla2doNivel, uint32_t indiceEntrada2doNivel) {
	usleep(retardoSWAP);
	t_mem_proceso* memProceso = list_get(tablasDe1erNivel, nroTabla1erNivel);
	uint32_t indiceEntradaTabla1erNivel = dameIndiceEntradaTabla1erNivel(
			memProceso, nroTabla2doNivel);
	t_list* tablaDe2doNivel = list_get(tablasDe2doNivel, nroTabla2doNivel);
	t_entrada2doNivel* entrada2doNivel = list_get(tablaDe2doNivel,
			indiceEntrada2doNivel);

	uint32_t direccionPagina = memProceso->tamanioPrograma / tamPagina;

	FILE *fp = fopen(memProceso->nombreArchivoSWAP, "r");

	uint32_t cantidadDePaginasDelPrograma = tamMemoria / tamPagina;
	long swapOffsets[cantidadDePaginasDelPrograma];

	int i = 0;
	while (i < cantidadDePaginasDelPrograma) {
		fseek(fp, tamPagina * i, SEEK_SET);
		swapOffsets[i] = ftell(fp);
		i++;
	}

	fseek(fp, swapOffsets[direccionPagina], SEEK_SET);
	fread(entrada2doNivel->marco, tamPagina, 1, fp);
	fclose(fp);
}

void cargarPaginaEnMemoria(uint32_t nroPagina, uint32_t nroTabla1erNivel,
		uint32_t nroTabla2doNivel, uint32_t indiceEntrada2doNivel) {

	t_mem_proceso* memProceso = list_get(tablasDe1erNivel, nroTabla1erNivel);

	uint32_t indiceEntradaTabla1erNivel = dameIndiceEntradaTabla1erNivel(
			memProceso, nroTabla2doNivel);

	t_list* tablaDe2doNivel = list_get(tablasDe2doNivel, nroTabla2doNivel);

	t_entrada2doNivel* entrada2doNivel = list_get(tablaDe2doNivel,
			indiceEntrada2doNivel);

	uint32_t nroPagina2 = indiceEntradaTabla1erNivel * entradasPorTabla;

	if (memProceso->marcosAsignados < marcosPorProceso) {
		void* nuevoMarco = obtenerNuevoMarcoMemoriaUsuario(nroTabla1erNivel);
		entrada2doNivel->marco = nuevoMarco;

		//voy a buscar la pagina a swap
		t_solicitud_swap* solicitudSwap = malloc(sizeof(t_solicitud_swap));
		solicitudSwap->opSwap = SWAP_A_MEM;
		solicitudSwap->nroTabla1erNivel = nroTabla1erNivel;
		solicitudSwap->nroTabla2doNivel = nroTabla2doNivel;
		solicitudSwap->indiceEntrada2doNivel = indiceEntrada2doNivel;
		solicitudSwap->nroPagina = nroPagina;
		solicitudSwap->marco = nuevoMarco;
		pthread_mutex_lock(&mutexColaPaginasASwapear);
		queue_push(colaPaginasASwapear, solicitudSwap);
		pthread_mutex_unlock(&mutexColaPaginasASwapear);
		sem_post(&swapear);
		sem_wait(&swapAMemOK);

		entrada2doNivel->presente = true;
		entrada2doNivel->modificado = false;

		t_pagina_en_memoria* paginaEnMemoria = malloc(
				sizeof(t_pagina_en_memoria));
		paginaEnMemoria->nroTabla1erNivel = nroTabla1erNivel;
		paginaEnMemoria->nroTabla2doNivel = nroTabla2doNivel;
		paginaEnMemoria->indiceEntrada2doNivel = indiceEntrada2doNivel;
		paginaEnMemoria->presente = true;
		paginaEnMemoria->uso = true;
		paginaEnMemoria->modificado = false;
		paginaEnMemoria->marco = nuevoMarco;
		paginaEnMemoria->nroDePagina = nroPagina;
		list_add(memProceso->tablaDeMarcosAsignados, paginaEnMemoria);

		memProceso->marcosAsignados++;

	} else {
		//sem_wait(&reemplazarPagina)
		uint32_t nroTabla2doNivelVictima, indiceEntrada2doNivelVictima,
				nroPaginaASalvar;
		elegirPaginaVictima(memProceso->tablaDeMarcosAsignados,
				&nroTabla2doNivelVictima, &nroPaginaASalvar,
				&indiceEntrada2doNivelVictima);

		t_entrada2doNivel* entrada2doNivelVictima = list_get(tablaDe2doNivel,
				indiceEntrada2doNivelVictima);
		/*
		 t_list* tablaDe2doNivelVictima = list_get(tablasDe2doNivel, nroTabla2doNivelVictima);
		 uint32_t indiceEntradaTabla1erNivelVictima = dameIndiceEntradaTabla1erNivel(memProceso, nroTabla2doNivelVictima);
		 uint32_t nroPaginaVictima = indiceEntradaTabla1erNivelVictima * tamPagina;
		 */
		uint32_t nroPaginaVictima = nroPaginaASalvar;

		t_solicitud_swap* solicitudSwap = malloc(sizeof(t_solicitud_swap));
		solicitudSwap->opSwap = MEM_A_SWAP;
		solicitudSwap->nroPagina = nroPaginaVictima;
		solicitudSwap->nroTabla1erNivel = nroTabla1erNivel;
		solicitudSwap->nroTabla2doNivel = nroTabla2doNivelVictima;
		solicitudSwap->indiceEntrada2doNivel = indiceEntrada2doNivelVictima;
		pthread_mutex_lock(&mutexColaPaginasASwapear);
		queue_push(colaPaginasASwapear, solicitudSwap);
		pthread_mutex_unlock(&mutexColaPaginasASwapear);
		sem_post(&swapear);
		sem_wait(&memASwapOK);

		entrada2doNivelVictima->presente = false;
		entrada2doNivelVictima->modificado = false;
		uint32_t i = 0;
		while (i < list_size(memProceso->tablaDeMarcosAsignados)) {
			t_pagina_en_memoria* paginaEnMemoria = list_get(
					memProceso->tablaDeMarcosAsignados, i);
			if (paginaEnMemoria->nroDePagina == nroPaginaVictima) {
				paginaEnMemoria->presente = false;
				paginaEnMemoria->modificado = false;
				paginaEnMemoria->uso = false;
				disponibilizarMarco(paginaEnMemoria->marco);
				break;
			}
			i++;
		}

		void* nuevoMarco = obtenerNuevoMarcoMemoriaUsuario(nroTabla1erNivel);
		entrada2doNivel->marco = nuevoMarco;

		pthread_mutex_lock(&mutexColaPaginasASwapear);
		solicitudSwap = malloc(sizeof(t_solicitud_swap));
		solicitudSwap->opSwap = SWAP_A_MEM;
		solicitudSwap->nroPagina = nroPagina;
		solicitudSwap->nroTabla1erNivel = nroTabla1erNivel;
		solicitudSwap->nroTabla2doNivel = nroTabla2doNivel;
		solicitudSwap->indiceEntrada2doNivel = indiceEntrada2doNivel;
		solicitudSwap->marco = nuevoMarco;
		queue_push(colaPaginasASwapear, solicitudSwap);
		pthread_mutex_unlock(&mutexColaPaginasASwapear);
		sem_post(&swapear);
		sem_wait(&swapAMemOK);

		//reemplazarLaVictima
		entrada2doNivel->presente = true;
		entrada2doNivel->modificado = false;
		while (i < list_size(memProceso->tablaDeMarcosAsignados)) {
			t_pagina_en_memoria* paginaEnMemoria = list_get(
					memProceso->tablaDeMarcosAsignados, i);
			if (paginaEnMemoria->nroDePagina == nroPaginaVictima) {
				paginaEnMemoria->nroDePagina = nroPagina;
				paginaEnMemoria->presente = true;
				paginaEnMemoria->uso = true;
				paginaEnMemoria->modificado = false;
				paginaEnMemoria->marco = nuevoMarco;
				break;
			}
			i++;
		}

		//sem_post(&reemplazarPagina)
	}
}

uint32_t* dameNumeroTabla2doNivel(uint32_t nroTabla1erNivel,
		uint32_t entrada1erNivel) {
	t_mem_proceso* memProceso = list_get(tablasDe1erNivel, nroTabla1erNivel);
	return list_get(memProceso->tablaDe1erNivel, entrada1erNivel);
}

void* dameMarco(uint32_t nroPagina, uint32_t nroTabla1erNivel,
		uint32_t nroTabla2doNivel, uint32_t indiceEntrada2doNivel) {
	t_list* tablaDe2doNivel = list_get(tablasDe2doNivel, nroTabla2doNivel);
	t_entrada2doNivel* entrada2doNivel = list_get(tablaDe2doNivel,
			indiceEntrada2doNivel);

	if (entrada2doNivel->presente) {
		return entrada2doNivel->marco;
	} else {
		cargarPaginaEnMemoria(nroPagina, nroTabla1erNivel, nroTabla2doNivel,
				indiceEntrada2doNivel);
		return entrada2doNivel->marco;
	}
}

uint32_t inicializarMemoriaProceso(uint32_t pid, uint32_t tamanioPrograma) {
	t_mem_proceso* memoriaProceso = malloc(sizeof(t_mem_proceso));

	char* nombreArchivoSWAP = crearSWAP(tamanioPrograma, pid);
	memoriaProceso->nombreArchivoSWAP = nombreArchivoSWAP;
	memoriaProceso->tamanioPrograma = tamanioPrograma;

	t_list* tablaDe1erNivel = list_create();
	t_list* tablaDe2doNivel;

	t_entrada2doNivel* entrada2doNivel;
	uint32_t* indiceTabla2doNivel;
	uint32_t i = 0;
	uint32_t j = 0;
	while (i < entradasPorTabla) {
		tablaDe2doNivel = list_create();
		j = 0;
		while (j < entradasPorTabla) {
			entrada2doNivel = malloc(sizeof(t_entrada2doNivel));
			entrada2doNivel->marco = NULL;
			entrada2doNivel->modificado = false;
			entrada2doNivel->uso = false;
			entrada2doNivel->presente = false;
			list_add(tablaDe2doNivel, entrada2doNivel);
			j++;
		}
		list_add(tablasDe2doNivel, tablaDe2doNivel);
		indiceTabla2doNivel = malloc(sizeof(uint32_t));
		*indiceTabla2doNivel = i;
		list_add(tablaDe1erNivel, indiceTabla2doNivel);
		i++;
	}
	memoriaProceso->tablaDe1erNivel = tablaDe1erNivel;
	memoriaProceso->marcosAsignados = 0;
	memoriaProceso->tablaDeMarcosAsignados = list_create();
	list_add(tablasDe1erNivel, memoriaProceso);
	return list_size(tablasDe1erNivel) - 1;
}

void suspenderProceso(uint32_t nroTabla1erNivel) {
	t_mem_proceso* memProc = list_get(tablasDe1erNivel, nroTabla1erNivel);
	t_list* tablaDeMarcosAsignados = memProc->tablaDeMarcosAsignados;
	uint32_t i = 0;
	log_info(logger, "Marcos Asignados: %d", list_size(tablaDeMarcosAsignados));
	while (i < list_size(tablaDeMarcosAsignados)) {
		t_pagina_en_memoria* paginaEnMemoria = list_get(tablaDeMarcosAsignados,
				i);
		if (paginaEnMemoria->presente) {
			t_mem_proceso* memProce = list_get(tablasDe1erNivel,
					paginaEnMemoria->nroTabla1erNivel);
			uint32_t indiceEntrada1erNivel = floor(
					paginaEnMemoria->nroDePagina / entradasPorTabla);
			uint32_t indiceEntrada2doNivel = paginaEnMemoria->nroDePagina
					% entradasPorTabla;
			uint32_t* nroTabla2doNivel = list_get(memProce->tablaDe1erNivel,
					indiceEntrada1erNivel);

			pthread_mutex_lock(&mutexColaPaginasASwapear);
			t_solicitud_swap* solicitudSwap = malloc(sizeof(t_solicitud_swap));
			solicitudSwap->opSwap = SUSPENDER;
			solicitudSwap->nroPagina = paginaEnMemoria->nroDePagina;
			solicitudSwap->nroTabla1erNivel = paginaEnMemoria->nroTabla1erNivel;
			solicitudSwap->nroTabla2doNivel = paginaEnMemoria->nroTabla2doNivel;
			solicitudSwap->indiceEntrada2doNivel =
					paginaEnMemoria->indiceEntrada2doNivel;
			queue_push(colaPaginasASwapear, solicitudSwap);
			pthread_mutex_unlock(&mutexColaPaginasASwapear);
			sem_post(&swapear);
			sem_wait(&suspenderProcesoOK);

			t_list* tablaDe2doNivel = list_get(tablasDe2doNivel,
					*nroTabla2doNivel);
			t_entrada2doNivel* entrada2doNivel = list_get(tablaDe2doNivel,
					indiceEntrada2doNivel);
			entrada2doNivel->presente = false;
			entrada2doNivel->modificado = false;

			paginaEnMemoria->presente = false;
			paginaEnMemoria->uso = false;
			paginaEnMemoria->modificado = false;
		}
		i++;
	}
}

void finalizarProceso(uint32_t nroTabla1erNivel) {

	t_mem_proceso* memProc = list_get(tablasDe1erNivel, nroTabla1erNivel);

	t_list* tablaDe1erNivel = memProc->tablaDe1erNivel;

	uint32_t nroEntradasTabla1erNivel = list_size(tablaDe1erNivel);
	uint32_t i = 0;
	while (i < nroEntradasTabla1erNivel) {
		uint32_t* nroTabla2doNivel = list_get(tablaDe1erNivel, i);
		t_list* tabla2doNivel = list_get(tablasDe2doNivel, *nroTabla2doNivel);
		uint32_t nroEntradasTabla2doNivel = list_size(tabla2doNivel);
		uint32_t j = 0;
		while (j < nroEntradasTabla2doNivel) {
			t_entrada2doNivel* entrada2donivel = list_get(tabla2doNivel, j);
//			free(entrada2donivel);
			j++;
		}
//		list_destroy(tabla2doNivel);
		i++;
	}
	list_destroy(tablaDe1erNivel);

	list_destroy_and_destroy_elements(memProc->tablaDeMarcosAsignados, free);

	memProc->marcosAsignados = 0;
	memProc->tamanioPrograma = 0;
	remove(memProc->nombreArchivoSWAP);
	free(memProc->nombreArchivoSWAP);

}

uint32_t leer(void* direccion) {
	uint32_t valor;
	memcpy(&valor, direccion, sizeof(uint32_t));
	return valor;
}

void grabar(void* direccion, uint32_t dato) {
	log_info(logger, "Copiando aca: %d esto: %d", direccion, dato);
	memcpy(direccion, &dato, sizeof(uint32_t));
	log_info(logger, "Copiando aca: %d esto: %d --- OK", direccion, dato);
	log_info(logger, "marcarPaginaComoModificada.");
	marcarPaginaComoModificada(direccion);
	log_info(logger, "marcarPaginaComoModificada.OK");
}

void copiar(void* direccionDesde, void* direccionHasta) {
	uint32_t aCopiar = leer(direccionDesde);
	grabar(direccionHasta, aCopiar);
	marcarPaginaComoModificada(direccionHasta);
}

void atenderSolicitudSWAP() {
	while (1) {
		log_info(logger, "Espeando solicitud SWAP.");
		sem_wait(&swapear);
		log_info(logger, "Swapeando. Size: %d",
				queue_size(colaPaginasASwapear));
		t_solicitud_swap* solicitudSWAP = queue_pop(colaPaginasASwapear);
		log_info(logger, "solicitudSWAP.OK");
		switch (solicitudSWAP->opSwap) {
		case SWAP_A_MEM:
			;
			log_info(logger, "SWAP_A_MEM.");
			moverDeSWAPaMemoria(solicitudSWAP->nroPagina,
					solicitudSWAP->nroTabla1erNivel,
					solicitudSWAP->nroTabla2doNivel,
					solicitudSWAP->indiceEntrada2doNivel);
			log_info(logger, "SWAP_A_MEM.OK");
			sem_post(&swapAMemOK);
			break;
		case MEM_A_SWAP:
			;
			log_info(logger, "MEM_A_SWAP.");
			moverDeMemoriaASWAP(solicitudSWAP->nroPagina,
					solicitudSWAP->nroTabla1erNivel,
					solicitudSWAP->nroTabla2doNivel,
					solicitudSWAP->indiceEntrada2doNivel);
			log_info(logger, "MEM_A_SWAP.OK");
			sem_post(&memASwapOK);
			break;
		case SUSPENDER:
			;
			log_info(logger, "SUSPENDER");
			moverDeMemoriaASWAP(solicitudSWAP->nroPagina,
					solicitudSWAP->nroTabla1erNivel,
					solicitudSWAP->nroTabla2doNivel,
					solicitudSWAP->indiceEntrada2doNivel);
			log_info(logger, "SUSPENDER.OK");
			sem_post(&suspenderProcesoOK);
			break;
		default:
			;
			log_warning(logger, "Operacion desconocida.");
			break;
		}
	}
}

void atenderCliente(int socket) {
	int cliente_fd = socket;
	int offset;
	t_buffer* buffer;
	t_paquete* paquete;
	void* stream;
	void* bufferRecv;
	int opCode;
	int size;
	void* direccionFisica;
	uint32_t dato;
	uint32_t nroTabla1erNivel;

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case SAVE_DATA:
			;
			log_info(logger, "SAVE_DATA");
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&direccionFisica, bufferRecv + offset, sizeof(void*));
			offset += sizeof(void*);
			memcpy(&dato, bufferRecv + offset, sizeof(uint32_t));
			free(bufferRecv);

			usleep(retardoMemoria);
			grabar(direccionFisica, dato);

			opCode = MEM_OK;
			send(cliente_fd, &opCode, sizeof(int), 0);
			log_info(logger, "SAVE_DATA.OK");
			break;

		case GET_DATA:
			;
			log_info(logger, "GET_DATA");
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&direccionFisica, bufferRecv + offset, sizeof(void*));
			offset += sizeof(uint32_t);
			free(bufferRecv);

			usleep(retardoMemoria);
			dato = leer(direccionFisica);

			paquete = crearPaquete(MEM_OK, sizeof(uint32_t));
			offset = 0;
			memcpy(paquete->buffer->stream + offset, &dato, sizeof(uint32_t));
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			log_info(logger, "GET_DATA.OK");
			break;

		case INIT_PROCESS:
			;
			log_info(logger, "INIT_PROCESS");
			uint32_t tamanioProceso;
			uint32_t pID;
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&pID, bufferRecv + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&tamanioProceso, bufferRecv + offset, sizeof(uint32_t));
			free(bufferRecv);

			uint32_t nroTablaPagina1erNivel = inicializarMemoriaProceso(pID,
					tamanioProceso);
			log_info(logger, "nroTablaPagina1erNivel: %d",
					nroTablaPagina1erNivel);
			paquete = crearPaquete(MEM_OK, sizeof(uint32_t));
			offset = 0;
			memcpy(paquete->buffer->stream + offset, &nroTablaPagina1erNivel,
					sizeof(uint32_t));
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			log_info(logger, "INIT_PROCESS.OK");
			break;

		case GET_FRAME:
			;
			log_info(logger, "GET_FRAME");
			uint32_t nroTabla2doNivel;
			uint32_t entradaTabla2doNivel;
			uint32_t nroPagina;
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&nroTabla1erNivel, bufferRecv + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&nroTabla2doNivel, bufferRecv + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&entradaTabla2doNivel, bufferRecv + offset,
					sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&nroPagina, bufferRecv + offset, sizeof(uint32_t));
			free(bufferRecv);

			usleep(retardoMemoria);
			void* marco = dameMarco(nroPagina, nroTabla1erNivel,
					nroTabla2doNivel, entradaTabla2doNivel);

			paquete = crearPaquete(MEM_OK, sizeof(uint32_t));
			offset = 0;
			memcpy(paquete->buffer->stream + offset, &marco, sizeof(uint32_t));
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			log_info(logger, "GET_FRAME.OK");
			break;

		case GET_TABLA_2DO_NIVEL:
			;
			uint32_t entradaTabla1erNivel;
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&nroTabla1erNivel, bufferRecv + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&entradaTabla1erNivel, bufferRecv + offset,
					sizeof(uint32_t));
			free(bufferRecv);

			usleep(retardoMemoria);
			uint32_t* numeroTabla2doNivel = dameNumeroTabla2doNivel(
					nroTabla1erNivel, entradaTabla1erNivel);

			paquete = crearPaquete(MEM_OK, sizeof(uint32_t));
			offset = 0;
			memcpy(paquete->buffer->stream + offset, numeroTabla2doNivel,
					sizeof(uint32_t));
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			break;

		case END_PROCESS:
			;
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&nroTabla1erNivel, bufferRecv + offset, sizeof(uint32_t));
			free(bufferRecv);

			finalizarProceso(nroTabla1erNivel);

			opCode = MEM_OK;
			send(cliente_fd, &opCode, sizeof(int), 0);
			break;

		case SUSPEND_PROCESS:
			;
			bufferRecv = recibir_buffer(&size, cliente_fd);
			offset = 0;
			memcpy(&nroTabla1erNivel, bufferRecv + offset, sizeof(uint32_t));
			free(bufferRecv);
			log_info(logger,"Suspendiendo un proceso");
			suspenderProceso(nroTabla1erNivel);

			opCode = MEM_OK;
			send(cliente_fd, &opCode, sizeof(int), 0);
			break;

		case GET_MEM_CONFIG:
			;
			paquete = crearPaquete(MEM_OK, 2 * sizeof(uint32_t));
			offset = 0;
			memcpy(paquete->buffer->stream + offset, &tamPagina,
					sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(paquete->buffer->stream + offset, &entradasPorTabla,
					sizeof(uint32_t));
			enviar_paquete(paquete, cliente_fd);
			eliminar_paquete(paquete);
			break;

		case -1:
			;
			log_error(logger, "El cliente se desconecto.");
			return;

		default:
			;
			log_warning(logger, "Operacion desconocida.");
			break;
		}

	}
}

int main(int argc, char **argv) {
	logger = log_create("./memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);

	t_config *config = config_create(argv[1]);
	if (config == NULL) {
		printf(
				"No se pudo iniciar la consola, revise el path del archivo de configuración.");
		exit(2);
	}
	char *ip_server_memoria = config_get_string_value(config, "IP_ESCUCHA");
	char *port_server_memoria = config_get_string_value(config,
			"PUERTO_ESCUCHA");

	int server_fd = iniciar_servidor(ip_server_memoria, port_server_memoria);
	log_info(logger, "Servidor de memoria listo para recibir al cliente");

	tamMemoria = config_get_int_value(config, "TAM_MEMORIA");
	memoriaUsuarioInicio = calloc(1, tamMemoria);

	listaDeMarcos = list_create();
	tamPagina = config_get_int_value(config, "TAM_PAGINA");

	void* memoriaUsuarioIt = memoriaUsuarioInicio;
	uint32_t i = 0, cantPaginas = tamMemoria / tamPagina;
	while (i < cantPaginas) {
		t_info_marco* infoMarco = malloc(sizeof(t_info_marco));
		infoMarco->disponible = true;
		infoMarco->marco = memoriaUsuarioIt;
		infoMarco->nroTabla1erNivel = NULL;
		list_add(listaDeMarcos, infoMarco);
		memoriaUsuarioIt += tamPagina;
		i++;
	}

	entradasPorTabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
	retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA") * 1000;
	retardoSWAP = config_get_int_value(config, "RETARDO_SWAP") * 1000;
	algoritmoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	marcosPorProceso = config_get_int_value(config, "MARCOS_POR_PROCESO");

	tablasDe1erNivel = list_create();
	tablasDe2doNivel = list_create();
	gradoDeMultiprogramacion = tamMemoria / marcosPorProceso;

	colaPaginasASwapear = queue_create();
	pthread_mutex_init(&mutexColaPaginasASwapear, NULL);
	sem_init(&swapear, 0, 0);
	sem_init(&suspenderProcesoOK, 0, 0);
	sem_init(&swapAMemOK, 0, 0);
	sem_init(&memASwapOK, 0, 0);

	pthread_t hiloSuspender;
	pthread_create(&hiloSuspender, NULL, (void*) atenderSolicitudSWAP,
	NULL);
	pthread_detach(hiloSuspender);

	// TESTs
	/*
	 inicializarMemoriaProceso(0, 128);
	 uint32_t a = 12;
	 void* ab = memoriaUsuarioInicio;
	 grabar(memoriaUsuarioInicio, a);
	 a = leer(memoriaUsuarioInicio);
	 printf("Leido: %d", a);
	 memoriaUsuarioInicio++;
	 copiar(ab, memoriaUsuarioInicio);
	 a = leer(memoriaUsuarioInicio);
	 printf("Leido: %d", a);
	 */
//	dameMarco(0, 0, 0);
//	dameMarco(0, 1, 1);
//	dameMarco(0, 2, 1);
//	dameMarco(0, 2, 2);
//	dameMarco(0, 2, 3);
//	dameMarco(0, 3, 3);
//	dameMarco(0, 2, 1);
	while (1) {
		int cliente_fd = esperar_cliente(server_fd);
		pthread_t hiloCliente;
		pthread_create(&hiloCliente, NULL, (void*) atenderCliente,
				(int*) cliente_fd);
		pthread_detach(hiloCliente);
	}

	pthread_mutex_destroy(&mutexColaPaginasASwapear);
	sem_destroy(&swapear);
	sem_destroy(&suspenderProcesoOK);
	sem_destroy(&swapAMemOK);
	sem_destroy(&memASwapOK);

	close(server_fd);
	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

PROCESS_NAME=Kernel
IDIR =./src
SHARED_IDIR = ../Shared
CC=gcc  # Vamos a usar gcc como compilador
CFLAGS=-I$(IDIR) -I$(SHARED_IDIR) -g -Wall # Compilamos con los flags -g (con simbolos de debug) y -Wall (que nos muestre todos los warnings)

ODIR=./obj
SRCDIR =./src
LIBS=-lcommons -lpthread -lreadline -lcunit -lrt -lm # Incluimos las bibliotecas que necesitamos, las commons y readline

SHARED_SRCDIR = ../Shared
SHARED_ODIR = ../Shared

DEPS = $(wildcard $(IDIR)/*.h)

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(ODIR)/%.o,$(SRC))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) $(SHARED_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SHARED_ODIR)/%.o: $(SHARED_SRCDIR)/%.c $(SHARED_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

SHARED_DEPS = $(wildcard $(SHARED_IDIR)/*.h)

SHARED_SRC = $(wildcard $(SHARED_SRCDIR)/*.c)
SHARED_OBJ = $(patsubst $(SHARED_SRCDIR)/%.c,$(SHARED_ODIR)/%.o,$(SHARED_SRC))

$(PROCESS_NAME): $(OBJ) $(SHARED_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
# Esta regla .PHONY es para decirle a make que si hay archivos con nombres similares
# a alguna regla, que entonces no asuma que el argumento es el archivo, sino que es la regla
# dado que va a tratar de tomar el archivo y no la regla por defecto

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(PROCESS_NAME) $(SHARED_ODIR)/*.o
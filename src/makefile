IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -g -std=c11 -O3

ODIR=../obj
LDIR =../lib

LIBS=-lm

_DEPS = renderer.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = renderer.o viewport.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

renderer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 

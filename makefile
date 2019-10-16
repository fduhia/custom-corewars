CC =     gcc -std=c99
WARNINGS = -Wall -pedantic
CFLAGS = $(WARNINGS) -Iobj -I.
LFLAGS = $(WARNINGS)

all: sim

clean:
	rm -rf obj
	rm -f sim

sim: sim.c 68defs.h stermp.h
	$(CC) $(CFLAGS) sim.c stermp.c -o sim

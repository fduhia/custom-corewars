CC =     gcc -std=c99
WARNINGS = -Wall -pedantic
CFLAGS = $(WARNINGS) -c -Iobj -I.
LFLAGS = $(WARNINGS)

all: obj sim.exe

clean:
	rm -rf obj
	rm -f sim.exe

obj:
	mkdir obj

sim.exe: obj/sim.o obj/parser.o 
	$(CC) $(LFLAGS) obj/sim.o obj/parser.o -o sim.exe

obj/sim.o: sim.c 68defs.h parser.h
	$(CC) $(CFLAGS) sim.c -o obj/sim.o

obj/parser.o: parser.c parser.h
	$(CC) $(CFLAGS) parser.c -o obj/parser.o


TARGETS: .cc. o
OBJS=bman.o level.o bomb.o font.o ai.o queue.o BFSearch.o list.o player.o compat.o sound_dummy.o menu.o
FLAGS=-g

%.o: %.cc
	g++ ${FLAGS} -c -o $@ $<

all: bman ${OBJS}

bman: ${OBJS}
	g++ ${FLAGS} -o $@ ${OBJS} -lSDL2

TARGETS: .cc. o
OBJS=bman.o level.o bomb.o font.o ai.o queue.o BFSearch.o list.o player.o compat.o sound_dummy.o menu.o
FLAGS=-g

%.o: %.cc
	g++ ${FLAGS} -c -o $@ $<

all: bman ${OBJS}

bman: ${OBJS}
	g++ ${FLAGS} -o $@ ${OBJS} -lSDL2

bman.html:
	emcc -O2 -o bman.html -sASYNCIFY -sSDL2_IMAGE_FORMATS='["bmp"]' -s USE_SDL_IMAGE=2 -sUSE_SDL=2 --embed-file fonts --embed-file data  bman.cc level.cc bomb.cc font.cc ai.cc queue.cc BFSearch.cc list.cc player.cc compat.cc sound_dummy.cc menu.cc

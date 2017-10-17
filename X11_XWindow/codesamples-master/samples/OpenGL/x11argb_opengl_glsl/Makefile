.PHONY: all

OPT = -O3

CPU_OPT = -march=native

all: main

main: main.c
	g++ $(OPT) $(CPU_OPT) -g3 -o main.out main.c -lX11 -lXrender -lGLEW -lm -lGL

clean:
	rm -f *.o*

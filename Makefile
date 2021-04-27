# MAKEFLAGS=-s
export CFLAGS=-std=c11 -O3

all:
	make -C vendored/makeheaders
	make -C vendored/lemon
	make -C vendored/logc

clean:
	make -C vendored/makeheaders clean
	make -C vendored/lemon clean
	make -C vendored/logc clean
	make -C src clean

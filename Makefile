# MAKEFLAGS=-s
export CFLAGS=-std=c11 -O3
export DEBUG=
export DEBUG_TRACE=

all:
	make -C src opam_bootstrap
	mkdir -p ${HOME}/.local/bin
	ls -l src
	ls -l ${HOME}/.local/bin
	cp -v src/opam_bootstrap ${HOME}/.local/bin

# x:
# 	make -C vendored/makeheaders
# 	make -C vendored/lemon
# 	make -C vendored/logc

clean:
	make -C vendored/makeheaders clean
	make -C vendored/lemon clean
	make -C vendored/logc clean
	make -C src clean

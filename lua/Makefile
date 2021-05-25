# hideous, but it works

CFLAGS+=-std=c11 -I../vendored/logc -I../vendored/uthash -I../src/meta_parser -I../vendored/lua

ifdef $(DEBUG)
CFLAGS+=-DDEBUG -g -O0
else
CFLAGS+=-O3
endif

ifdef $(DEBUG_TRACE)
CFLAGS+=-DDEBUG_TRACE
endif

SRCS=obazl.c obazl_dir.c

META_OBJS=../src/meta_parser/driver.o \
../src/meta_parser/meta_entries.o \
../src/meta_parser/meta_flags.o \
../src/meta_parser/meta_packages.o \
../src/meta_parser/meta_properties.o \
../src/meta_parser/meta_settings.o \
../src/meta_parser/meta_values.o \
../src/meta_parser/meta_fs.o


LOG_OBJS=../vendored/logc/log.o

BAZEL_OBJS=../src/emit_build_bazel.o ../src/opam_bootstrap.o

PARSER_OBJS=../src/meta_parser/metalexer.o ../src/meta_parser/metaparser.o

ALL_OBJS=${META_OBJS} ${BAZEL_OBJS} ${LOG_OBJS} ${PARSER_OBJS}

all: obazl.o obazl_dir.o opam_bootstrap lua
	$(CC) -shared -o obazl.so ${ALL_OBJS} obazl.o obazl_dir.o -L../vendored/lua -llua

obazl.o: obazl.c obazl_h logc
obazl_dir.o: obazl_dir.c obazl_dir.h

OBJS=obazl.o obazl_dir.o

.PHONY: opam_bootstrap
opam_bootstrap:
	make -C ../src opam_bootstrap

.PHONY: obazl_h
obazl_h:
	make -C ../src/meta_parser obazl_h

.PHONY: logc
logc:
	make -C ../vendored/logc

.PHONY: lua
lua:
	make -C ../vendored/lua

clean:
	rm -f *.o *.so

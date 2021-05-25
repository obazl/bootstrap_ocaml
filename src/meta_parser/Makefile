CFLAGS=-std=c11 -I../../vendored/logc -I../../vendored/uthash

ifdef $(DEBUG)
CFLAGS+=-DDEBUG -g -O0
else
CFLAGS+=-O3
endif

ifdef $(DEBUG_TRACE)
CFLAGS+=-DDEBUG_TRACE
endif

SRCS=driver.c \
meta_entries.c \
meta_flags.c \
meta_fs.c \
meta_packages.c \
meta_properties.c \
meta_settings.c \
meta_values.c

hdrs := $(patsubst %.c,%.h,$(wildcard *.c))

META_OBJS=driver.o meta_entries.o meta_flags.o meta_packages.o meta_properties.o meta_settings.o meta_values.o meta_fs.o

# metaparser_impl.o

LOG_OBJS=../../vendored/logc/log.o

PARSER_OBJS=metalexer.o metaparser.o

OBJS=${META_OBJS} ${LOG_OBJS} ${PARSER_OBJS}

all: ${OBJS}

driver.o: driver.c driver.h
meta_entries.o: meta_entries.c meta_entries.h
meta_flags.o: meta_flags.c meta_flags.h
meta_fs.o: meta_fs.c meta_fs.h
meta_packages.o: meta_packages.c meta_packages.h
meta_properties.o: meta_properties.c meta_properties.h
meta_settings.o: meta_settings.c meta_settings.h
meta_values.o: meta_values.c meta_values.h
opam_bootstrap.o: opam_bootstrap.c opam_bootstrap.h

metalexer.o: metalexer.c metalexer.h
metaparser.o: metaparser.c metaparser.h

meta: ${META_OBJS}

parser: ${PARSER_OBJS}

$(hdrs): mkhdrs

metalexer.h: metalexer.c mkhdrs

## Assumption is that re2c has already been built...
metalexer.c:
	re2c metalexer.re -o metalexer.c

metaparser.c: metaparser.y lemon
	../../vendored/lemon/lemon -m metaparser.y -T../../vendored/lemon/lempar.c

mkhdrs: makeheaders metaparser.c metalexer.c *.c
	../../vendored/makeheaders/makeheaders ${SRCS} metalexer.c metaparser.c

obazl_h: makeheaders metaparser.c metalexer.c *.c
	../../vendored/makeheaders/makeheaders -h ${SRCS} metalexer.c metaparser.c > obazl.h


.PHONY: lemon
lemon:
	$(MAKE) -C ../../vendored/lemon

.PHONY: logc
logc:
	$(MAKE) -C ../../vendored/logc

.PHONY: makeheaders
makeheaders:
	$(MAKE) -C ../../vendored/makeheaders

clean:
	rm -f metaparser.c metaparser.out
	rm -f metalexer.c
	rm -f *.h *.o
	rm -f opam_bootstrap

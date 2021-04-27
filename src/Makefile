CFLAGS+=-std=c11 -I../vendored/logc -I../vendored/uthash
CFLAGS+=-O3
# CFLAGS+=-DDEBUG -g

SRCS=api.c \
driver.c \
emit_build_bazel.c \
meta_entries.c \
meta_flags.c \
meta_packages.c \
meta_properties.c \
meta_settings.c \
meta_values.c \
metaparser_impl.c \
obazl_fs.c \
opam_bootstrap.c \
utils.c

api.o: api.c api.h
driver.o: driver.c driver.h
emit_build_bazel.o: emit_build_bazel.c emit_build_bazel.h
meta_entries.o: meta_entries.c meta_entries.h
meta_flags.o: meta_flags.c meta_flags.h
meta_packages.o: meta_packages.c meta_packages.h
meta_properties.o: meta_properties.c meta_properties.h
meta_settings.o: meta_settings.c meta_settings.h
meta_values.o: meta_values.c meta_values.h
metaparser_impl.o: metaparser_impl.c metaparser_impl.h
obazl_fs.o: obazl_fs.c obazl_fs.h
opam_bootstrap.o: opam_bootstrap.c opam_bootstrap.h
metalexer.o: metalexer.c metalexer.h
metaparser.o: metaparser.c metaparser.h

hdrs := $(patsubst %.c,%.h,$(wildcard *.c))

OBJS=api.o driver.o emit_build_bazel.o meta_entries.o meta_flags.o meta_packages.o meta_properties.o meta_settings.o meta_values.o metaparser_impl.o obazl_fs.o opam_bootstrap.o metalexer.o metaparser.o ../vendored/logc/log.o

opam_bootstrap: ${OBJS}
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(hdrs): mkhdrs

mkhdrs: makeheaders metaparser.c metalexer.c *.c
	../vendored/makeheaders/makeheaders ${SRCS} metalexer.c metaparser.c

metaparser.c: metaparser.y lemon
	../vendored/lemon/lemon -m metaparser.y -T../vendored/lemon/lempar.c

## Assumption is that re2c has already been built...
metalexer.c:
	re2c metalexer.re -o metalexer.c

.PHONY: lemon
lemon:
	make -C ../vendored/lemon

.PHONY: logc
logc:
	make -C ../vendored/logc

.PHONY: makeheaders
makeheaders:
	make -C ../vendored/makeheaders

clean:
	rm -f metaparser.c metaparser.out
	rm -f metalexer.c
	rm -f *.h *.o
	rm -f opam_bootstrap

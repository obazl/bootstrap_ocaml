#include <stdbool.h>
#include <stdio.h>
#include "utarray.h"
#include "uthash.h"

#include "log.h"
#include "meta_flags.h"

static int indent = 2;
static int delta = 2;
static char *sp = " ";

/* **************************************************************** */
EXPORT int obzl_meta_flags_count(obzl_meta_flags *_flags)
{
    return utarray_len(_flags->list);
}

EXPORT obzl_meta_flag *obzl_meta_flags_nth(obzl_meta_flags *_flags, int _i)
{
    return utarray_eltptr(_flags->list, _i);
}

/* **************************************************************** */
EXPORT char *obzl_meta_flag_name(obzl_meta_flag *flag)
{
    return flag->s;
}

EXPORT bool obzl_meta_flag_polarity(obzl_meta_flag *flag)
{
    return flag->polarity;
}
/* ****************************************************************
   struct obzl_meta_flag_s
   **************************************************************** */
#if INTERFACE
struct obzl_meta_flag {
    bool polarity;
    char *s;
};

struct obzl_meta_flags {
    UT_array *list;
};
#endif

UT_icd flag_icd = {sizeof(struct obzl_meta_flag), NULL, flag_copy, flag_dtor};

void flag_copy(void *_dst, const void *_src) {
#if DEBUG_TRACE
    log_trace("flag_copy");
#endif
    struct obzl_meta_flag *dst = (struct obzl_meta_flag*)_dst;
    struct obzl_meta_flag *src = (struct obzl_meta_flag*)_src;
    dst->polarity = src->polarity;
    dst->s = src->s ? strdup(src->s) : NULL;
}

void flag_dtor(void *_elt) {
    struct obzl_meta_flag *elt = (struct obzl_meta_flag*)_elt;
    if (elt->s) free(elt->s);
}

/* ****************
   obzl_meta_flags (was: UT_array flags)
   **************** */
obzl_meta_flags *obzl_meta_flags_new(void)
{
#if DEBUG_TRACE
    log_trace("obzl_meta_flags_new");
#endif
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    return new_flags;
}

obzl_meta_flags *obzl_meta_flags_new_copy(obzl_meta_flags *old_flags)
{
#if DEBUG_TRACE
    /* log_trace("obzl_meta_flags_new_copy %p", old_flags); */
#endif
    if (old_flags == NULL) {
        return NULL;
    }

    /* UT_array *new_flags; */
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    struct obzl_meta_flag *old_flag = NULL;
    /* struct obzl_meta_flag *new_flag; */
    while ( (old_flag=(struct obzl_meta_flag*)utarray_next(old_flags->list, old_flag))) {
        /* log_trace("old_flag: %s", old_flag->s); */
        /* new_flag = (struct obzl_meta_flag*)malloc(sizeof(struct obzl_meta_flag)); */
        /* flag_copy(new_flag, old_flag); */
        /* log_trace("new_flag: %s", new_flag->s); */
        utarray_push_back(new_flags->list, old_flag);
    }
    return new_flags;
}

/* EXPORT UT_array *obzl_meta_flags_new_tokenized(char *flags) */
EXPORT obzl_meta_flags *obzl_meta_flags_new_tokenized(char *flags)
{
#if DEBUG_TRACE
    log_trace("obzl_meta_flags_new_tokenized(%s)", flags);
#endif
    if (flags == NULL) {
        return NULL;
    }
    /* UT_array *new_flags; */
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    char *token, *sep = " ,\n";
    token = strtok(flags, sep);
    struct obzl_meta_flag *new_flag;
    while( token != NULL ) {
        /* printf("Flag token: %s", token); */
        new_flag = (struct obzl_meta_flag*)malloc(sizeof(struct obzl_meta_flag));
        new_flag->polarity = token[0] == '-'? false : true;
        new_flag->s = token[0] == '-'? strdup(++token) : strdup(token);
        /* printf("new flag: %s", new_flag->s); */
        utarray_push_back(new_flags->list, new_flag);
        token = strtok(NULL, sep);
    }
    /* dump_flags(8, new_flags); */
    return new_flags;
}

/* void flags_dtor(UT_array *old_flags) { */
void flags_dtor(obzl_meta_flags *old_flags) {
    struct obzl_meta_flag *old_flag = NULL;
    while ( (old_flag=(struct obzl_meta_flag*)utarray_next(old_flags->list, old_flag))) {
        free(old_flag->s);
    }
    utarray_free(old_flags->list);
    free(old_flags);
}

char *obzl_meta_flags_to_string(obzl_meta_flags *flags)
{
#ifdef DEBUG_TRACE
    log_trace("%*obzl_meta_flags_to_string", indent, sp);
#endif
    char *buf = (char*)calloc(512, 1);

    if (flags == NULL) {
        /* printf("%*sflags: none\n", indent, sp); */
        return buf;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                /* printf("%*sflags ct: 0\n", indent, sp); */
                return buf;
            } else {
                /* log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list)); */
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    /* printf("%*s%s (polarity: %d)\n", delta+indent, sp, a_flag->s, a_flag->polarity); */
                    mystrcat(buf, a_flag->s);
                    mystrcat(buf, ", ");
                }
                /* printf("buf: %s\n", buf); */
                return buf;
            }
        } else {
            /* printf("%*sflags none: 0\n", indent, sp); */
            /* log_debug("%*sflags: none", indent, sp); */
            return buf;
        }
    }
}

char *obzl_meta_flags_to_condition_name(obzl_meta_flags *flags)
{
#ifdef DEBUG_TRACE
    log_trace("%*obzl_meta_flags_to_condition_name", indent, sp);
#endif
    char *buf = (char*)calloc(512, 1);

    if (flags == NULL) {
        /* printf("%*sflags: none\n", indent, sp); */
        return buf;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                /* printf("%*sflags ct: 0\n", indent, sp); */
                return buf;
            } else {
                if (utarray_len(flags->list) == 1) {
                    struct obzl_meta_flag *a_flag = obzl_meta_flags_nth(flags, 0);

                    // "standard predicates have predefined select labels:
                    // byte, native:  @ocaml//mode:byte
                    // toploop, create_toploop
                    // mt, mt_posix, mt_vm: @opam//cfg/mt:std, @opam/cfg/mt:posix, @opam/cfg/mt:vm
                    //  cmd line:  --@opam//cfg/mt=posix
                    // gprof, autolink, preprocessor,
                    // syntax, plugin (legacy)

                    // other "well-known" predicates:
                    // ppx_driver, custom_ppx: @ppx//cfg:driver, @ppx//cfg:custom
                    // ??? should not be global?

                    // if 'byte' or 'native', use @ocaml//mode:bytecode, @ocaml//mode:native
                    // otherwise, use @opam//cfg:foo

                    mystrcat(buf, "@opam//cfg:");
                    mystrcat(buf, a_flag->s);
                    return buf;
                } else {
                    /* log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list)); */
                    struct obzl_meta_flag *a_flag = NULL;
                    while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                        /* printf("%*s%s (polarity: %d)\n", delta+indent, sp, a_flag->s, a_flag->polarity); */
                        mystrcat(buf, a_flag->s);
                        mystrcat(buf, "_");
                    }
                    /* printf("buf: %s\n", buf); */
                    return buf;
                }
            }
        } else {
            /* printf("%*sflags none: 0\n", indent, sp); */
            /* log_debug("%*sflags: none", indent, sp); */
            return buf;
        }
    }
}

/* **************************************************************** */
#if DEBUG_TRACE
void dump_flags(int indent, obzl_meta_flags *flags)
{
    indent++;            /* account for width of log label */
    /* log_trace("%*sdump_flags", indent, sp); */
    if (flags == NULL) {
        log_trace("%*sflags: none", indent, sp);
        return;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                log_trace("%*sflags: none.", indent, sp);
                return;
            } else {
                log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list));
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity);
                }
            }
        } else {
            log_debug("%*sflags: none", indent, sp);
        }
    }
}
#endif

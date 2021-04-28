#if EXPORT_INTERFACE
#include <stdio.h>
#endif

#include "log.h"
#include "utarray.h"
#include "utstring.h"

#include "emit_build_bazel.h"

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

static int indent = 2;
static int delta = 2;


void emit_bazel_hdr(FILE* ostream, int level, char *repo, char *pkg_prefix, obzl_meta_package *_pkg)
{
    char *pkg_name = obzl_meta_package_name(_pkg);
    char *pname = "requires";
    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, pname);
    if ( deps_prop == NULL ) {
        log_error("Prop '%s' not found.", pname);
        return;
    /* } else { */
    /*     /\* log_debug("Got prop: %s", obzl_meta_property_name(deps_prop)); *\/ */
    }

    fprintf(ostream, "load(\n");
    fprintf(ostream, "%*s\"@obazl_rules_ocaml//ocaml:rules.bzl\",\n", 4, sp);
    fprintf(ostream, "%*s\"ocaml_import\"\n", 4, sp);
    fprintf(ostream, ")\n");
}

void emit_bazel_imports(FILE* ostream, int level,
                        char *repo, char *pkg, obzl_meta_package *_pkg,
                        char *property)
{
    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, property);
    if ( deps_prop == NULL ) {
        char *pkg_name = obzl_meta_package_name(_pkg);
        log_warn("Prop '%s' not found: %s.", property, pkg_name);
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    if (obzl_meta_settings_count(settings) == 0) {
        log_info("No deps for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    /* log_info("settings count: %d", settings_ct); */

    if (settings_ct > 1) {
        fprintf(ostream, "%*s%s = select({\n", level*spfactor, sp, property);
    } else {
        fprintf(ostream, "%*s%s = [\n", level*spfactor, sp, property);
    }

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting %d", i+1); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        char *condition;
        if (flags == NULL)
            condition =  "//conditions:default";
        else
            condition =  "condition";

        char *condition_comment = obzl_meta_flags_to_string(flags);
        /* log_debug("condition_comment: %s", condition_comment); */

        char *condition_name = obzl_meta_flags_to_condition_name(flags);

        /* FIXME: multiple settings means multiple flags; decide how to handle for deps */
        // construct a select expression on the flags
        if (settings_ct > 1) {
            fprintf(ostream, "        \"%s\"%-4s",
                    condition_name, ":");
                    /* condition_comment, /\* FIXME: name the condition *\/ */
        }
        vals = obzl_meta_setting_values(setting);
        /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            /* char *s = (char*)*v; */
            /* while (*s) { */
            /*     /\* printf("s: %s\n", s); *\/ */
            /*     if(s[0] == '.') { */
            /*         /\* log_info("Hit"); *\/ */
            /*         s[0] = '/'; */
            /*     } */
            /*     s++; */
            /* } */
            if (settings_ct > 1) {
                fprintf(ostream, "\"//:%s/%s\"",
                        /* (2+level)*spfactor, sp, */
                        pkg, *v);
            } else {
                /* fprintf(ostream, "%*s\"%s//%s/%s\"\n", (1+level)*spfactor, sp, repo, pkg, *v); */
                fprintf(ostream, "%*s\"//:%s/%s\"\n", (1+level)*spfactor, sp, pkg, *v);
            }
        }
        if (settings_ct > 1) {
            fprintf(ostream, "\",\n"); // , (1+level)*spfactor, sp);
            /* fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp); */
        }
        free(condition_comment);
    }
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_archive_imports(FILE* ostream, int level, char *repo, char *pkg, obzl_meta_package *_pkg)
{
    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"archive\",\n");
    emit_bazel_imports(ostream, 1, "@opam", ".lib", _pkg, "archive");
    emit_bazel_deps(ostream, 1, "@opam", "lib", _pkg);
    fprintf(ostream, ")\n");
}

void emit_bazel_plugin_imports(FILE* ostream, int level, char *repo, char *pkg, obzl_meta_package *_pkg)
{
    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"plugin\",\n");
    emit_bazel_imports(ostream, 1, "@opam", ".lib", _pkg, "plugin");
    emit_bazel_deps(ostream, 1, "@opam", "lib", _pkg);
    fprintf(ostream, ")\n");
}

// deps: construct bazel pkg path, in @opam
// the pkgs in the deps list are opam/ocamlfind pkg names;
// as paths, they are relative to the opam lib dir.
// dotted dep strings must be converted to filesys paths

// what about non-opam META files? we always need a repo name ('@foo') and a pkg path.

void emit_bazel_deps(FILE* ostream, int level, char *repo, char *pkg, obzl_meta_package *_pkg)
{

    //FIXME: skip if no 'requires'

    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    char *pname = "requires";
    struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, pname);
    if ( deps_prop == NULL ) {
        char *pkg_name = obzl_meta_package_name(_pkg);
        /* log_warn("Prop '%s' not found: %s.", pname, pkg_name); */
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    if (obzl_meta_settings_count(settings) == 0) {
        log_info("No deps for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    /* log_info("settings count: %d", settings_ct); */

    if (settings_ct > 1) {
        fprintf(ostream, "%*sdeps = select({\n", level*spfactor, sp);
    } else {
        fprintf(ostream, "%*sdeps = [\n", level*spfactor, sp);
    }

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting %d", i+1); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        char *condition;
        if (flags == NULL)
            condition =  "//conditions:default";
        else
            condition =  "condition";

        char *condition_comment = obzl_meta_flags_to_string(flags);
        /* log_debug("condition_comment: %s", condition_comment); */

        /* FIXME: multiple settings means multiple flags; decide how to handle for deps */
        // construct a select expression on the flags
        if (settings_ct > 1) {
            fprintf(ostream, "%*s\"%s\": [ ## %s\n",
                    (1+level)*spfactor, sp,
                    condition, /* FIXME: name the condition */
                    condition_comment);
        }
        vals = obzl_meta_setting_values(setting);
        /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            char *s = (char*)*v;
            while (*s) {
                /* printf("s: %s\n", s); */
                if(s[0] == '.') {
                    /* log_info("Hit"); */
                    s[0] = '/';
                }
                s++;
            }
            if (settings_ct > 1) {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n",
                        (2+level)*spfactor, sp,
                        repo, pkg, *v);
            } else {
                fprintf(ostream, "%*s\"%s//%s/%s\"\n", (1+level)*spfactor, sp, repo, pkg, *v);
            }
        }
        if (settings_ct > 1) {
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        free(condition_comment);
    }
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_subpackages(char *_tgtroot,
                            char *_repo,
                            char *_pkg_prefix,
                            struct obzl_meta_package *_pkg)

{
    log_debug("emit_bazel_subpackages, pfx: %s", _pkg_prefix);
    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *e = NULL;

    char pfx[256];

    for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PACKAGE) {
            log_debug("Package entry: %s", e->package->name);
            /* pfx[0] = '\0'; */
            /* mystrcat(pfx, _pkg_prefix); */
            /* mystrcat(pfx, "/"); */
            /* mystrcat(pfx, e->package->name); */
            /* log_debug("RECUR PFX: %s", pfx); */
            emit_build_bazel(_tgtroot, _repo, _pkg_prefix, e->package);
        }
    }
}

EXPORT void emit_build_bazel(char *_tgtroot,
                             char *_repo,
                             char *_pkg_prefix,
                             struct obzl_meta_package *_pkg)
{
#ifdef DEBUG
    log_info("emit_build_bazel");
    log_info("\t@%s//%s/%s", _repo, _pkg_prefix, obzl_meta_package_name(_pkg));
    /* log_set_quiet(false); */
    log_debug("%*sparsed name: %s", indent, sp, obzl_meta_package_name(_pkg));
    log_debug("%*sparsed dir:  %s", indent, sp, obzl_meta_package_dir(_pkg));
    log_debug("%*sparsed src:  %s", indent, sp, obzl_meta_package_src(_pkg));
#endif

    UT_string *build_bazel_file;
    utstring_new(build_bazel_file);

    utstring_clear(build_bazel_file);
    utstring_printf(build_bazel_file, "%s/%s/%s", _tgtroot, _pkg_prefix, obzl_meta_package_name(_pkg));
    mkdir_r(utstring_body(build_bazel_file), "");
    utstring_printf(build_bazel_file, "/%s", "BUILD.bazel");
    log_debug("writing: %s", utstring_body(build_bazel_file));

    FILE *ostream;
    ostream = fopen(utstring_body(build_bazel_file), "w");
    if (ostream == NULL) {
        perror(utstring_body(build_bazel_file));
        /* log_error("fopen failure for %s", utstring_body(build_bazel_file)); */
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "## original: %s\n\n", obzl_meta_package_src(_pkg));

    emit_bazel_hdr(ostream, 1, "@opam", "lib", _pkg);

    if (obzl_meta_package_has_archives(_pkg)) {
        emit_bazel_archive_imports(ostream, 1, "@opam", ".lib", _pkg);
    }

    if (obzl_meta_package_has_plugins(_pkg)) {
        emit_bazel_plugin_imports(ostream, 1, "@opam", ".lib", _pkg);
    }

    fclose(ostream);

    char pfx[256];
    pfx[0] = '\0';
    mystrcat(pfx, _pkg_prefix);
    mystrcat(pfx, "/");
    mystrcat(pfx, obzl_meta_package_name(_pkg));
    log_debug("new pfx: %s", pfx);
    emit_bazel_subpackages(_tgtroot, _repo, pfx, _pkg);

    utstring_free(build_bazel_file);
}

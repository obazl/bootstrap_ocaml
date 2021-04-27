#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>


#include "utarray.h"
#include "uthash.h"

#include "log.h"

/* #include "obazl.h" */
#include "opam_bootstrap.h"

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

static int indent = 2;
static int delta = 2;

/* **************************************************************** */

static int verbosity = 0;
int errnum;
int rc;

/* #define BUFSIZE 1024 */

char work_buf[PATH_MAX];

char outdir[PATH_MAX];

UT_array *opam_packages;

char basedir[PATH_MAX];
char coqlib[PATH_MAX];

char tgtroot_bin[PATH_MAX];
char tgtroot_lib[PATH_MAX];

char symlink_src[PATH_MAX];
char symlink_tgt[PATH_MAX];

struct buildfile_s {
    char *name;                 /* set from strndup optarg; must be freed */
    char *path;                 /* set from getenv; do not free */
    UT_hash_handle hh;
} ;
struct buildfile_s *buildfiles = NULL;

struct fileset_s *filesets = NULL;

/* struct package_s *packages = NULL; */

/* **************************************************************** */
char *run_cmd(char *cmd)
{
    static char buf[PATH_MAX];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }

    while (fgets(buf, sizeof buf, fp) != NULL) {
        /* printf("SWITCH: %s\n", buf); */
        buf[strcspn(buf, "\n")] = 0;
    }

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return NULL;
    }
    return buf;
}

void opam_config(char *_opam_switch, char *outdir)
{
    /*
      1. discover switch
         a. check env var OPAMSWITCH
         b. use -s option
         c. run 'opam var switch'
      2. discover lib dir: 'opam var lib'
     */

    char *opam_switch;
    char *srcroot_bin;
    char *srcroot_lib;

    char *tgt_bin = "bin";
    char *tgt_bin_hidden = "/.bin";
    char *tgt_lib = "lib";
    char *tgt_lib_hidden = "/.lib";

    char workbuf[PATH_MAX];

    tgtroot_bin[0] = '\0';
    mystrcat(tgtroot_bin, outdir);
    mystrcat(tgtroot_bin, "/bin");
    tgtroot_lib[0] = '\0';
    mystrcat(tgtroot_lib, outdir);
    mystrcat(tgtroot_lib, "/lib");

    /* FIXME: handle switch arg */
    char *cmd, *result;
    if (_opam_switch == NULL) {
        /* log_info("using current switch"); */
        cmd = "opam var switch";

        result = run_cmd(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(%s)\n", cmd);
        } else
            opam_switch = strndup(result, PATH_MAX);
    }

    cmd = "opam var bin";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        srcroot_bin = strndup(result, PATH_MAX);

    cmd = "opam var lib";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        srcroot_lib = strndup(result, PATH_MAX);

    // STEP 0: install root WORKSPACE, BUILD files

    // STEP 1: link opam dirs
    // NOTE: root BUILD.bazel must contain 'exports_files([".bin/**"], [".lib/**"])'

    // FIXME: these dir symlinks can be done in starlark code; which way is better?

    mkdir_r(outdir, "");
    workbuf[0] = '\0';
    mystrcat(workbuf, outdir);
    mystrcat(workbuf, tgt_bin_hidden);
    rc = symlink(srcroot_bin, workbuf); // tgtroot_bin);
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(symlink_tgt);
            log_error("symlink failure for %s -> %s\n", srcroot_bin, tgtroot_bin);
            exit(EXIT_FAILURE);
        }
    }
    /* mkdir_r(tgtroot_lib, ""); */
    workbuf[0] = '\0';
    mystrcat(workbuf, outdir);
    mystrcat(workbuf, tgt_lib_hidden);
    rc = symlink(srcroot_lib, workbuf); // tgtroot_lib);
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(symlink_tgt);
            log_error("symlink failure for %s -> %s\n", srcroot_lib, tgtroot_lib);
            exit(EXIT_FAILURE);
        }
    }

    // STEP 2: mirror lib dir structure, adding BUILD.bazel files. no symlinks
    mirror_tree(srcroot_lib,
                /* tgtroot_lib, */
                false, "META", handle_lib_meta);

    mirror_tree(srcroot_bin,
                /* tgtroot_bin, */
                false, NULL, NULL);
}

void handle_lib_meta(char *rootdir,
                     char *pkgdir,
                     char *metafile)
{
    /* log_info("handle_lib_meta %s ; %s ; %s", rootdir, pkgdir, metafile); */

    char buf[PATH_MAX];
    buf[0] = '\0';
    mystrcat(buf, rootdir);
    mystrcat(buf, "/");
    mystrcat(buf, pkgdir);
    mystrcat(buf, "/");
    mystrcat(buf, metafile);

    /* mkdir_r(buf, "/"); */
    /* mystrcat(buf, "/BUILD.bazel"); */

    /* /\* log_debug("out buf: %s", buf); *\/ */
    /* FILE *f; */
    /* if ((f = fopen(buf, "w")) == NULL){ */
    /*     log_fatal("Error! opening file %s", buf); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    /* fprintf(f, "## test\n"); //  "src: %s/%s\n", pkgdir, metafile); */
    /* fclose(f); */

    errno = 0;
    struct obzl_meta_package *pkg = obzl_meta_parse_file(buf);
    if (pkg == NULL) {
        if (errno == -1)
            log_warn("Empty META file: %s", buf);
        else
            if (errno == -2)
                log_warn("META file contains only whitespace: %s", buf);
            else
                log_error("Error parsing %s", buf);
    } else {
        log_trace("PARSED %s", buf);

        emit_build_bazel(pkg, "opam", "lib");

        /* log_debug("%*sparsed name:      %s", indent, sp, obzl_meta_package_name(pkg)); */
        /* log_debug("%*sparsed directory: %s", indent, sp, obzl_meta_package_dir(pkg)); */
        /* log_debug("%*sparsed metafile:  %s", indent, sp, obzl_meta_package_src(pkg)); */
    }

}

int main(int argc, char *argv[]) // , char **envp)
{
    /* printf("argc: %d\n", argc); */

    /* for (char **env = envp; *env != 0; env++) { */
    /*     char *thisEnv = *env; */
    /*     printf("env: %s\n", thisEnv); */
    /* } */

    char *opam_switch;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        log_error("getcwd failure");
        exit(EXIT_FAILURE);
    }
    /* utarray_new(opam_packages, &ut_str_icd); */

#ifdef DEBUG
    char *opts = "b:o:s:v";     /* p: */
#else
    char *opts = "b:p:s:v";
#endif

    int opt;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case 'b':
            /* build_files */
            printf("option b: %s\n", optarg);
            printf("build file: %s\n", getenv(optarg));
            struct buildfile_s *the_buildfile = (struct buildfile_s *)calloc(sizeof (struct buildfile_s), 1);
            if (the_buildfile == NULL) {
                errnum = errno;
                fprintf(stderr, "main calloc failure for struct buildfile_s *\n");
                perror(getenv(optarg));
                exit(1);
            }
            the_buildfile->name = strndup(optarg, 512);
            the_buildfile->path = getenv(optarg);
            HASH_ADD_STR(buildfiles, name, the_buildfile);
            break;
        /* case 'p': */
        /*     printf("option p: %s\n", optarg); */
        /*     utarray_push_back(opam_packages, &optarg); */
        /*     break; */
        case 's':
            printf("option s: %s\n", optarg);
            opam_switch = strndup(optarg, PATH_MAX);
            break;
        case 'v':
            verbosity++;
            break;
#ifdef DEBUG
        case 'o':
            printf("option o: %s\n", optarg);
            if (optarg[0] == '/') {
                log_error("Absolute path not allowed for -o option.");
                exit(EXIT_FAILURE);
            }
            outdir[0] = '\0';
            mystrcat(outdir, cwd);
            mystrcat(outdir, "/");
            mystrcat(outdir, optarg);
            /* char *rp = realpath(optarg, outdir); */
            /* if (rp == NULL) { */
            /*     perror(optarg); */
            /*     log_fatal("realpath failed on %s", outdir); */
            /*     exit(EXIT_FAILURE); */
            /* } */
            break;
#endif
        default:
            ;
        }
    }
#ifdef DEBUG
    if (strlen(outdir) == 0) {
        mystrcat(outdir, cwd);
        mystrcat(outdir, "/tmp");
    }
#else
    mystrcat(outdir, "./");
#endif

    opam_config(opam_switch, outdir);

#ifdef DEBUG
    log_info("outdir: %s", outdir);
#endif
}

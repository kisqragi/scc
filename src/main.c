#include "scc.h"
#include <stdio.h>
#include <string.h>

static FILE *output_path;
static char *input_path;

static void usage(int status) {
    fprintf(stderr, "scc -o <path> <file>\n");
    exit(status);
}

static void write_file(char *path) {
    if (!path)
        error("error: missing filename after '-o'");

    if (*path == '-') {
        output_path = stdout;
    } else {
        output_path = fopen(path, "w");
        if (!output_path)
            error("cannot open %s: %s", path, strerror(errno));
    }
}

static void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp("--help", argv[i]))
            usage(0);

        if (!strcmp("-o", argv[i])) {
            if (!argv[++i])
                usage(1);
            if (!strcmp("-", argv[i]))
                output_path = stdout;
            else
                output_path = fopen(argv[i], "w");
            continue;
        }

        if (argv[i][0] == '-' && argv[i][1] != '\0')
            error("error: unrecognized command line option '%s'", argv[i]);

        input_path = argv[i];
        continue;
    }

    if (!input_path)
        error("no input files");
    if (!output_path)
        error("no output files");
}

int main(int argc, char **argv) {
    parse_args(argc, argv);

    Token *tok = tokenize_file(input_path);
    Obj *prog = parse(tok);
    codegen(prog, output_path);

    return 0;
}

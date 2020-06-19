#include <stdio.h>
#include <stdlib.h>

#include "code.h"
#include "disas.h"
#include "lex.h"
#include "parse.h"

static char *stringify_file(const char *path) {
    FILE *file = fopen(path, "rb");
    fseek(file, 0L, SEEK_END);
    size_t s = ftell(file);
    rewind(file);
    char *buffer = (char *) malloc(s + 1);
    size_t b = fread(buffer, sizeof(char), s, file);
    buffer[b] = '\0';
    return buffer;
}

int main(int argc, char **argv) {
    chunk_t c;
    c_init(&c, "main");
    if (argc == 2)
        y_compile(argv[1], &c);
    else
        y_compile(stringify_file(argv[2]), &c);
    d_code_chunk(&c);
    return 0;
}

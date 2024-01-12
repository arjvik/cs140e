#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int compile(int argc, char *argv[]) {
    assert(argc >= 2 && "Usage: ./cc <input_file> -o <output_file>");

    FILE *inputFile = fopen(argv[1], "r");
    FILE *outputFile = fopen("./temp-out.c", "w");
    assert(inputFile != NULL && "Failed to open input file");
    assert(outputFile != NULL && "Failed to create output file");
    
    // This possibly has line buffer overflow issues, but I'm too lazy to recall how to do it from CS111

    char line[1024];
    while (fgets(line, sizeof(line), inputFile)) {
        fputs(line, outputFile);
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

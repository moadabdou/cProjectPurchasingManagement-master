#include <stdlib.h>
#include <stdio.h>
#include "tools.h"
// Function to read file content
File_prop read_file(const char *filename,const char  *mode) {
    File_prop data_file;
    data_file.content = NULL;

    FILE *file = fopen(filename, mode);
    if (!file) {
        printf("Could not open file.\n");
        data_file.content =  NULL;
        return data_file;
    }
    fseek(file, 0, SEEK_END);
    data_file.length= ftell(file);
    rewind(file);
    data_file.content = (char *)malloc(data_file.length + 1);
    fread(data_file.content, sizeof(char), data_file.length, file);
    fclose(file);

    data_file.content[data_file.length] = '\0'; // Null-terminate the string
    return data_file;
}

// Function to write data to file
void write_file(const char *filename, const char *data , const char  *mode) {
    FILE *file = fopen(filename, mode);
    if (!file) {
        printf("Could not open file to write.\n");
        return;
    }
    fprintf(file, "%s", data);
    fclose(file);
}
#include "./html_render.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char *c_html_render(char *src, Props *props, int props_length) {
    FILE *file = fopen(src , "r+");
    
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    char *rendered_output;
    char line[2048], *pos_index, *type, error[64], change_state; 
    const char *TAG_CODE = "//c_code:";
    long r_length;

    // total output length
    fseek(file, 0, SEEK_END);
    r_length = ftell(file);
    rewind(file);

    
    for (int i = 0; i < props_length; i++) {   
        r_length += strlen(props[i].data)*props[i].count;
    }

    rendered_output = (char *)malloc(sizeof(char) * r_length);

    if (rendered_output == NULL) {
        perror("Failed to allocate memory for rendered_output");
        fclose(file); // Clean up before returning
        return NULL;
    }

    int offset = 0;
    while (!feof(file)) {
        if (fgets(line, sizeof(line), file) == NULL) {
            break; // Exit on EOF or read error
        }
        
        pos_index = strstr(line, TAG_CODE);
        if (pos_index == NULL) {
            strcpy(rendered_output + offset, line);
            offset += strlen(line);
        } else {
            type = pos_index + strlen(TAG_CODE);
            //printf("code c detected type : %s\n", type);
            change_state = 0;
            for (int i = 0; i < props_length; i++) {
                if (strncmp(props[i].tag, type, strlen(props[i].tag)) == 0) {
                    strcpy(rendered_output + offset, props[i].data);
                    offset += strlen(props[i].data);                 
                    rendered_output[offset++] = '\n'; // Append new line
                    change_state = 1;
                }
            }
            if (!change_state) {
                sprintf(error, "UNDEFINED TAG %s\n", type);
                strcpy(rendered_output + offset, error);
                offset += strlen(error);
            }
        }
    }
    
    if (fclose(file) != 0) {
        perror("Error closing file");
    }

    return rendered_output;
}
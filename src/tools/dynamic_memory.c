#include <stdlib.h>
#include<stdio.h>
#include <string.h>
void append_to_string(char **buffer, const char *new_string) {
    size_t old_len = *buffer ? strlen(*buffer) : 0;
    size_t new_len = old_len + strlen(new_string) + 1; // +1 for the null-terminator
    
    // Reallocate buffer with new length
    *buffer = realloc(*buffer, new_len);
    memset((char*)*buffer + old_len, 0, new_len - old_len); //set  new memory  to  zeros
    if (*buffer == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    strcat(*buffer, new_string);
}
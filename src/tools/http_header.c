#include <stdio.h>
#include<string.h>
#include "./tools.h"


//cookies 
Cookie get_cookie(char *buffer,const char *name) {
    Cookie cookie = {}; 
    char full_cookies[512],
         prefix[] = "Cookie: ",
         *cookies_start_position = strstr(buffer, prefix);

    // Check if 'Cookie: ' exists in the buffer
    if (cookies_start_position == NULL) {  
        
        printf("Cookie header not found\n");
        return cookie;  // Return early or handle this error case as needed
    }

    // Move pointer past "Cookie: "
    cookies_start_position += strlen(prefix);

    char name_end[strlen(name) + 2]; // For name and '='
    strcpy(name_end, name);
    strcat(name_end, "="); // "contains \0 "

    int cookies_length = 0;
    // Ensure not to read beyond buffer or cause segfault
    while (*(cookies_start_position + cookies_length) != '\r' && cookies_length < 512) {
        cookies_length++;
    }

    strncpy(full_cookies, cookies_start_position, cookies_length);
    full_cookies[cookies_length] = '\0'; // Null terminate the copied cookies string

    printf("\nCookies length: %d\n", cookies_length);

    // Tokenize the cookies
    char *single_cookie = strtok(full_cookies, "; ");
    
    // Loop to find the correct cookie
    while (single_cookie != NULL && strncmp(name_end, single_cookie, strlen(name_end)) != 0) {
        single_cookie = strtok(NULL, "; ");
    }

    // If the cookie was not found
    if (single_cookie == NULL) {
        printf("Cookie '%s' not found\n", name);
        return cookie;  // Handle this case
    }

    // Copy the name and value to the cookie struct
    strcpy(cookie.name, name);
    strcpy(cookie.value, single_cookie + strlen(name_end));

    return cookie;
}

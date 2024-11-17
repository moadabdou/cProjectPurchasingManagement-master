#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "pages.h"  
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "./../tools/html_render.h"
#include "../data_vars.h"

#define HTML_FILE "./layouts/index.html"

void index_html(SOCKET client_socket) {
    
    cJSON *shops = load_json_from_file(SHOPS_DATAFILE);
    if(shops == NULL){
        SEND_ERROR_500
        return;
    }

    char *shops_list =  NULL, 
        *shops_list_temp = "<option value=\"%d\">%s</option>"; 

    for(int i = 0 ; i < cJSON_GetArraySize(shops);  i++){
        cJSON* shop =  cJSON_GetArrayItem(shops, i);

        char shop_html[64];

        sprintf(shop_html, shops_list_temp, 
            cJSON_GetObjectItem(shop, "id")->valueint,
            cJSON_GetObjectItem(shop, "name")->valuestring
        );

        append_to_string(&shops_list, shop_html);

    }


    char  *rendered_content  =  c_html_render(HTML_FILE, (Props []){{
        shops_list == NULL ? "no shops" : shops_list,
        "shops_list",
        1
    }} , 1);

    if (rendered_content == NULL){
        SEND_ERROR_500;
        return;
    }

    char header[256*2];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(rendered_content));
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, rendered_content , strlen(rendered_content) , 0);
    
    free(rendered_content);
    if (shops_list != NULL){
        free(shops_list);
    }
}
#ifndef ADMIN_H
#define ADMIN_H

#define NOT_FOUND  "./layouts/404.html"
//ADMIN layouts
#define HTML_FILE_ADMIN "./layouts/admin/dashboard.html"
#define INSIGHTS_ADMIN "./layouts/admin/insights.html"
#define EMPLOYEERS_ADMIN "./layouts/admin/managers.html"
#define ABOUTS  "./layouts/admin/about.html"


void admin(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";  

    if (strlen(query) == 0 || strlen(query) == 1 || strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "0";
        int shop_id = -1;

        sscanf(query, "/insights/%d", &shop_id);
        side_content = get_insights(INSIGHTS_ADMIN, shop_id);
        if (side_content == NULL){
            SEND_ERROR_500;
            return;
        }

    }else if(strncmp(query, "/managers" , strlen("/managers"))== 0){
        active = "1"; 

        //getting  data  files 
        File_prop employeers = read_file(EMPLOYEES_DATAFILE ,"r");
        File_prop shops   = read_file(SHOPS_DATAFILE, "r");

        if(employeers.content == NULL || shops.content == NULL){
            printf("\n cant open one of the files [from dashoard/admin/managers]");
            SEND_ERROR_500;
            return;
        } 
        //parsing data  
        cJSON *employeers_json = cJSON_Parse(employeers.content);
        cJSON *shops_json = cJSON_Parse(shops.content);
        free(shops.content);
        free(employeers.content);

        if (!employeers_json || !cJSON_IsArray(employeers_json) || !shops_json || !cJSON_IsArray(shops_json)) {
            printf("Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }

        const char *employeer_template = 
                        "<div class=\"card mt-3\">"
                            "<div class=\"card-header\">"
                                " %s "
                            "</div>"
                            "<div class=\"card-body\">"
                                "<h4 class=\"card-title\">shop: #%s</h4>"
                                "<h5 class=\"card-title\">id: #%d</h5>"
                                "<p class=\"card-text\">email : %s</p>"
                                "<p class=\"card-text\">CIN : %s</p>"
                                "<p class=\"card-text\">phone number : %s</p>"
                                "<p class=\"card-text\">birth date : %s</p>"
                                "%s"
                            "</div>"
                        "</div>";
        char *employeers_list = NULL;
        char emp_tmp[512] = {0,};

        for (int i = 0;  i <  cJSON_GetArraySize(employeers_json); i++){ //managers are employeers  of  admin
            cJSON *employeer =  cJSON_GetArrayItem(employeers_json, i);
            if (cJSON_GetObjectItem(employeer ,"role")->valueint == 1){ 
                int shop_id = cJSON_GetObjectItem(employeer ,"shop_id")->valueint;
                char *shop_name = cJSON_GetObjectItem(
                        searchById(shops_json, shop_id), "name"
                    )->valuestring;
                sprintf(emp_tmp, employeer_template,
                    cJSON_GetObjectItem(employeer ,"name")->valuestring,
                    shop_name ,
                    cJSON_GetObjectItem(employeer ,"id")->valueint,
                    cJSON_GetObjectItem(employeer ,"email")->valuestring,
                    cJSON_GetObjectItem(employeer ,"CIN")->valuestring,
                    cJSON_GetObjectItem(employeer ,"phone_number")->valuestring,
                    cJSON_GetObjectItem(employeer ,"birthdate")->valuestring,
                    cJSON_GetObjectItem(employeer ,"state")->valueint ? 
                        "<a class=\"btn btn-warning\">Suspend</a>" : "<a class=\"btn btn-success\">Active</a>"
                );
                append_to_string(&employeers_list, emp_tmp);
            }
        }
        
        side_content = c_html_render(EMPLOYEERS_ADMIN, (Props []){{
            employeers_list,
            "managers_list",
            1
        }} , 1);

        if (employeers_list != NULL){
            free(employeers_list);
        }
    }else if(strncmp(query, "/about" , strlen("/about"))== 0){
        active = "2"; 
        int shop_id = 0;
        sscanf(query, "/about/%d", &shop_id);

        File_prop shops   = read_file(SHOPS_DATAFILE, "r");

        if(shops.content == NULL){
            printf("\n cant open one of the files :  %s ", SHOPS_DATAFILE);
            SEND_ERROR_500;
            return;
        } 
        //parsing data  
        cJSON *shops_json = cJSON_Parse(shops.content);
        free(shops.content);

        if (!shops_json || !cJSON_IsArray(shops_json)) {
            printf("[from admin/about] Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }

        cJSON *shop_data = searchById(shops_json, shop_id);

        side_content = c_html_render( ABOUTS , (Props[]){{
            cJSON_Print(shop_data),
            "shop_data",
            1
        }}, 1);

    }else{
        active = "a";
        side_content = c_html_render(NOT_FOUND, NULL , 0);
    }

    //render dashboard html 
    Props props[] ={
        {
            cJSON_GetObjectItem(user_data, "name")->valuestring,//to do :  check  if  the user name exist first
            "User_name",
            1
        }, 
        {   
            side_content ,
            "side_content",
            1
        },
        {
            active,
            "active_element",
            1
        }
    };

    int props_length =  sizeof(props)/sizeof(Props);

    char  *rendered_content  =  c_html_render(HTML_FILE_ADMIN, props , props_length);

    if (rendered_content == NULL){
        SEND_ERROR_500;
        return;
    }
    char header[256*2];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(rendered_content));
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, rendered_content , strlen(rendered_content), 0);
    
    //free(side_content);
    free(rendered_content);
}

#endif
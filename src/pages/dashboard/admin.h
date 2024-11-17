#ifndef ADMIN_H
#define ADMIN_H

#define NOT_FOUND  "./layouts/404.html"
//ADMIN layouts
#define SIDEBAR "./layouts/admin/sidebar.html"
#define INSIGHTS_ADMIN "./layouts/admin/insights.html"
#define EMPLOYEERS_ADMIN "./layouts/admin/managers.html"
#define ABOUTS  "./layouts/admin/about.html"
#define PRODUCT_PERFORMANCE "./layouts/admin/product_performance.html"

dashboardContent admin(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";  
    dashboardContent NULL_CONTENT = {NULL, NULL};
    if (strlen(query) == 0 || strlen(query) == 1 || strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "0";
        int shop_id = -1;

        sscanf(query, "/insights/%d", &shop_id);
        side_content = get_insights(INSIGHTS_ADMIN, shop_id, 1);
        if (side_content == NULL){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

    }else if(strncmp(query, "/managers" , strlen("/managers"))== 0){
        active = "1"; 

        //getting  data  files 
        //preparing product list  
        cJSON* employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
        cJSON* shops_json      = load_json_from_file(SHOPS_DATAFILE);
        if (employeers_json ==  NULL || shops_json == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
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
    }else if(strncmp(query, "/products" , strlen("/products")) == 0){
        active = "2";

        side_content = get_products_list();

        if (side_content == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

    }else if (strncmp(query, "/ProductPerformance" , strlen("/ProductPerformance"))== 0){
        active = "2";
        
        cJSON* products = load_json_from_file(PRODUCTS_DATAFILE); 

        if (products == NULL ){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

        int product_id = -1, 
            shop_id = -1;
        char date[32] = {0};

        sscanf(query, "/ProductPerformance/%d/%d?date=%s", &product_id, &shop_id, date);
        cJSON* product =  searchById(products, product_id);
        if (product == NULL){
            SEND_ERROR_404;
            return NULL_CONTENT;
        }
        
        char product_name_id[128];
        sprintf(product_name_id, "%s <span id =\"product_id\" style=\"color:#777;\">#%d</span>", cJSON_GetObjectItem(product, "name")->valuestring, product_id);
        

        char generale_sale_time[16] ;
        sprintf(generale_sale_time, "%d", get_sale_times(product_id, shop_id));

        cJSON *shops = load_json_from_file(SHOPS_DATAFILE);
        if(shops == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        char *sale_times_list = NULL;
        char *sale_times_template = 
            "<div class=\"col-4 mb-3\">\n"
            "    <div class=\"card bg-total-revenue\">\n"
            "        <div class=\"card-body\">\n"
            "            <h5 class=\"card-title\">\n"
            "                %s\n"
            "            </h5>\n"
            "            <p class=\"card-text\">\n"
            "                %d\n"
            "            </p>\n"
            "        </div>\n"
            "    </div>\n"
            "</div>";

        char *shops_list =  NULL, 
            *shops_list_temp = "<option value=\"%d\">%s</option>"; 

        for(int i = 0 ; i < cJSON_GetArraySize(shops);  i++){
            cJSON* shop =  cJSON_GetArrayItem(shops, i);

            char sale_times[512], shop_html[64];
            sprintf(sale_times, sale_times_template, 
                cJSON_GetObjectItem(shop, "name")->valuestring, 
                get_sale_times(product_id, 
                    cJSON_GetObjectItem(shop, "id")->valueint
                )
            );
            append_to_string(&sale_times_list, sale_times);

            sprintf(shop_html, shops_list_temp, 
                cJSON_GetObjectItem(shop, "id")->valueint,
                cJSON_GetObjectItem(shop, "name")->valuestring
            );

            append_to_string(&shops_list, shop_html);

        }

        //diagram
        if (date[0] == 0){
            struct tm  *now = get_current_date();
            sprintf(date, "%d-%d", 1900 + now->tm_year, now->tm_mon + 1);
        }

        char *shop_name = "all";
        if (shop_id != -1){
            cJSON* shop = searchById(shops, shop_id);
            if (shop == NULL){
                shop_name = "not found";
            }else {
                shop_name = cJSON_GetObjectItem(shop, "name")->valuestring;
            }
        }

        char *diagram = get_performance_diagram(date, shop_id, shop_name ,product_id);

        side_content =  c_html_render(PRODUCT_PERFORMANCE, (Props []){{
            product_name_id,
            "product_name_id",
            1
        },{
            generale_sale_time,
            "general",
            1
        },{
            sale_times_list, 
            "per_shop",
            1
        },{
            diagram, 
            "diagram",
            1
        },{
            shops_list,
            "shop_list",
            1
        }}, 5);

        if (sale_times_list != NULL){
            free(sale_times_list);
        }

        if (shops_list != NULL){
            free(shops_list);
        }

        if (diagram !=  NULL){
            free(diagram);
        }

    }else if(strncmp(query, "/about" , strlen("/about"))== 0){
        active = "3"; 
        int shop_id = 0;
        sscanf(query, "/about/%d", &shop_id);
 
        cJSON* shops_json = load_json_from_file(SHOPS_DATAFILE);
        if (shops_json ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
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

    return (dashboardContent){
        side_content : side_content,
        sidebar : c_html_render(SIDEBAR, (Props[]){{
            active,
            "active_element",
            1
        }}, 1)
    };
}

#undef SIDEBAR
#undef INSIGHTS_ADMIN 
#undef EMPLOYEERS_ADMIN
#undef PRODUCT_PERFORMANCE 
#undef ABOUTS  

#endif
#ifndef ADMIN_H
#define ADMIN_H

#define NOT_FOUND  "./layouts/404.html"
//ADMIN layouts
#define SIDEBAR "./layouts/admin/sidebar.html"
#define INSIGHTS_ADMIN "./layouts/admin/insights.html"
#define EMPLOYEERS_ADMIN "./layouts/admin/managers.html"
#define ABOUTS  "./layouts/admin/about.html"
#define PRODUCT_PERFORMANCE "./layouts/admin/product_performance.html"
#define  NEWSHOP  "./layouts/admin/newshop.html" 
#define  APPLY_MANAGER  "./layouts/admin/apply_manager.html" 
#define  APPS_MANAGER  "./layouts/admin/apps_manager.html" 

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

    }else if(strncmp(query, "/newshop" , strlen("/newshop"))== 0){
        active = "3";
        side_content =  c_html_render(NEWSHOP, NULL ,  0);

    }else if(strncmp(query, "/newmanager" , strlen("/newmanager"))== 0){
        active = "4";
        cJSON *shops = load_json_from_file(SHOPS_DATAFILE);

        if(shops == NULL){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

        char *html_list = NULL ; 

        const char *shop_template = 
                            "<div class=\"card mt-3\">"
                                "<div class=\"card-body\">"
                                    "<h4 class=\"card-title\">%s</h4>"
                                    "<a id = \"%s\" class=\"btn btn-success\" href=\"%s/%d\" >%s </a>"
                                "</div>"
                            "</div>";
        
        for (int i = 0;  i <  cJSON_GetArraySize(shops ) ;  i++){
            cJSON*  shop = cJSON_GetArrayItem(shops , i);
            int apply = cJSON_GetObjectItem(shop, "open_to_apply")->valueint;
            if (apply !=  0 ){
                int shop_app_id = cJSON_GetObjectItem(shop, "id")->valueint;
                char shop_html[1024];
                sprintf(shop_html, shop_template,
                    cJSON_GetObjectItem(shop, "name")->valuestring ,
                    apply == 1 ? "open_app" : "",
                    apply == 1 ?  "/api/shops/open" :  "/dashboard/show_apps",
                    shop_app_id, 
                    apply == 1 ?  "open" :  "see applications"
                );

                append_to_string(&html_list, shop_html);
            }
        }


        side_content =  c_html_render(APPLY_MANAGER , (Props[]){{
            html_list == NULL  ? "<center> it seems  like  all  shops already  have a manager </center>" : html_list, 
            "shops", 
            1
        }} ,  1);

        if (html_list != NULL ){
            free(html_list);
        }

    }else if(strncmp(query, "/show_apps" , strlen("/show_apps"))== 0){

        active = "4";

        int shop_id=0;
        sscanf(query, "/show_apps/%d",&shop_id);

        if(shop_id<=0){
            SEND_ERROR_404;
            return NULL_CONTENT;
        } 

        cJSON* apps_json =  load_json_from_file(APPS_MANAGER_DF);     
        cJSON* employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
        cJSON* shops_json      = load_json_from_file(SHOPS_DATAFILE);
        cJSON* sales           = load_json_from_file(SALES_DATAFILE);

        if (employeers_json ==  NULL || shops_json == NULL || apps_json == NULL || sales ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }


        cJSON *shop_apps = NULL;

        for (int  i  = 0 ;  i < cJSON_GetArraySize(apps_json) ; i++){
            cJSON *app = cJSON_GetArrayItem(apps_json, i);
            if (cJSON_GetObjectItem(app, "shop_id")->valueint == shop_id){

                shop_apps =  cJSON_GetObjectItem(app, "apps");
                break;
            }
        }

        if (shop_apps ==  NULL){
            SEND_ERROR_404;
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
                                "<p class=\"card-text\"><strong> total sales : %d </strong> </p>"
                                "<a class=\"btn btn-success\">approve</a>"
                            "</div>"
                        "</div>";

        char *employeers_list = NULL;
        char emp_tmp[512] = {0,};

        for (int i = 0;  i <  cJSON_GetArraySize(shop_apps); i++){ //managers are employeers  of  admin
            int  employeer_id =  cJSON_GetArrayItem(shop_apps, i)->valueint;

            cJSON *employeer =  searchById(employeers_json, employeer_id);

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
                    get_totale_sales(sales , employeer_id)
                );

            append_to_string(&employeers_list, emp_tmp);     
            
        }


        side_content =  c_html_render(APPS_MANAGER, (Props[]){{
            employeers_list == NULL ?  "no  applications yet" : employeers_list, 
            "apps_list", 
            1
        }} , 1);

        if (employeers_list != NULL){
            free(employeers_list);
        }

    }else if(strncmp(query, "/about" , strlen("/about"))== 0){
        active = "5"; 
        int shop_id = 0;
        sscanf(query, "/about/%d", &shop_id);
 
        cJSON* shops_json = load_json_from_file(SHOPS_DATAFILE);
        if (shops_json ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        char *shops_list =  NULL;
        char  *shops_list_temp = "<li><a class=\"dropdown-item\" href=\"/dashboard/about/%d \"> %s </a></li>"; 

        for(int i = 0 ; i < cJSON_GetArraySize(shops_json);  i++){
            cJSON* shop =  cJSON_GetArrayItem(shops_json, i);

            char shop_html[128];

            sprintf(shop_html, shops_list_temp, 
                cJSON_GetObjectItem(shop, "id")->valueint,
                cJSON_GetObjectItem(shop, "name")->valuestring
            );

            append_to_string(&shops_list, shop_html);

        }

        cJSON *shop_data = searchById(shops_json, shop_id);

        if (shop_data ==  NULL){
            SEND_ERROR_404;
            return NULL_CONTENT;
        }

        side_content = c_html_render( ABOUTS , (Props[]){{
            cJSON_Print(shop_data),
            "shop_data",
            1
        },{
            shops_list == NULL ? "no shops" : shops_list,
            "shops_list",
            1
        }}, 2);

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
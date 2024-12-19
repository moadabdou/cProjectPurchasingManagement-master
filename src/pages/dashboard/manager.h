#ifndef MANAGER_H
#define MANAGER_H

#define NOT_FOUND  "./layouts/404.html"
//manager layouts
#define SIDEBAR "./layouts/manager/sidebar.html"
#define SALES_LIST_MANAGER "./layouts/manager/sales_list.html"
#define INSIGHTS_MANAGER      "./layouts/manager/insights.html"
#define EMPLOYEERS_MANAGER  "./layouts/manager/employeers.html"
#define NEWPRODUCT          "./layouts/manager/newProduct.html"
#define PRODUCT_PERFORMANCE "./layouts/manager/product_performance.html"
#define ABOUT          "./layouts/manager/about.html"

dashboardContent manager(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";  
    //shop_id of the manager
    int shop_id_manager = cJSON_GetObjectItem(user_data, "shop_id")->valueint;
    dashboardContent NULL_CONTENT = {NULL, NULL};

    if (strlen(query) == 0 || strlen(query) == 1 || strncmp(query, "/sales" , strlen("/sales"))== 0 ){

        cJSON* employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
        if (employeers_json ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        //drop down menu of employeers     
        char *drop_down=NULL, 
            drop_element[512] = {0};

        for(int i = 0 ; i <  cJSON_GetArraySize(employeers_json) ; i++){
            cJSON* employeer = cJSON_GetArrayItem(employeers_json, i);
            if (cJSON_GetObjectItem(employeer, "role")->valueint == 0 &&
                shop_id_manager == cJSON_GetObjectItem(employeer, "shop_id")->valueint
                ){
                sprintf(drop_element,
                        "<li><a class=\"dropdown-item\" href=\"/dashboard/sales/%d\">%s <span style=\"opacity:0.5\">#%d</span></a></li>",
                        cJSON_GetObjectItem(employeer, "id")->valueint,
                        cJSON_GetObjectItem(employeer, "name")->valuestring,
                        cJSON_GetObjectItem(employeer, "id")->valueint);
                append_to_string(&drop_down, drop_element);
            }
        }


        //getting sales
        int employeer_id = 0;
        sscanf(query, "/sales/%d", &employeer_id);
        printf("trying to  get  sales of this user %d", employeer_id);

        char *sales_list = "<h3>choose an employeer to see his sales</h3>";
        short   is_sale_list_malloc = 0; 

        char employeers_name[64] = " "; 

        if (employeer_id >  0){
            sales_list = getSalesMadeBy(employeer_id);
            cJSON *emp = searchById(employeers_json, employeer_id);
            if (emp ==  NULL
                || cJSON_GetObjectItem(emp, "shop_id")->valueint != shop_id_manager){
                printf("\n>> cant  get  sales of this user");
                SEND_ERROR_404;
                return NULL_CONTENT;
            }

            char *name = cJSON_GetObjectItem(emp, "name")->valuestring;
            snprintf(employeers_name, sizeof(employeers_name), "<h3> theses are %s's sales :</h3>", name);
            is_sale_list_malloc = 1;
        }

        side_content = c_html_render(SALES_LIST_MANAGER, (Props []){{
            sales_list == NULL ? "<p style=\"text-align:center;\"> no  sales </p>":sales_list,
            "sales_list",
            1
        },{
            drop_down ? drop_down : " no employeers ",
            "employeers_drop_down",
            1
        },{
            employeers_name,
            "employeer_name",
            1
        }},3);

        if (is_sale_list_malloc && sales_list != NULL){
            free(sales_list);
        }

        if(drop_down == NULL){
            free(drop_down);
        }

        if (side_content == NULL){
            printf("\n>> cant  get  sales of this user");
            SEND_ERROR_500;
            return NULL_CONTENT;
        }



    }else if(strncmp(query, "/saleDetails" , strlen("/saleDetails"))== 0){ //sale details 
        side_content = getSale_detailes_query(query);
        if (side_content == NULL){
            printf("\n>> cant  get  sale  details");
            SEND_ERROR_500;
        }

    }else if(strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "1";
        side_content = get_insights(INSIGHTS_MANAGER, shop_id_manager, 0);
        if (side_content == NULL){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

    }else if(strncmp(query, "/employeers" , strlen("/employeers"))== 0){
        active = "2"; 
        //getting  data  files 
        cJSON* employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
        if (employeers_json ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        const char *employeer_template = 
                        "<div class=\"card mt-3\">"
                            "<div class=\"card-header\">"
                                " %s "
                            "</div>"
                            "<div class=\"card-body\">"
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

        for (int i = 0;  i <  cJSON_GetArraySize(employeers_json); i++){
            cJSON *employeer =  cJSON_GetArrayItem(employeers_json, i);
            if (cJSON_GetObjectItem(employeer ,"role")->valueint == 0 && 
                cJSON_GetObjectItem(employeer ,"shop_id")->valueint == shop_id_manager
            ){ 
                sprintf(emp_tmp, employeer_template,
                    cJSON_GetObjectItem(employeer ,"name")->valuestring,
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
        
        side_content = c_html_render(EMPLOYEERS_MANAGER, (Props []){{
            employeers_list ? employeers_list : "no  employeers" ,
            "employeers_list",
            1
        }} , 1);

        if (employeers_list != NULL){
            free(employeers_list);
        }
    }else if(strncmp(query, "/newProduct" , strlen("/newProduct"))== 0){
        active = "3";
        side_content = c_html_render( NEWPRODUCT , NULL, 0);
    }else if(strncmp(query, "/products" , strlen("/products")) == 0){
        active = "4";

        side_content = get_products_list();

        if (side_content == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

    }else if (strncmp(query, "/ProductPerformance" , strlen("/ProductPerformance"))== 0){

        active = "4";
                
        cJSON* products = load_json_from_file(PRODUCTS_DATAFILE); 

        if (products == NULL ){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

        int product_id = -1;
        char date[32] = {0};

        sscanf(query, "/ProductPerformance/%d?date=%s", &product_id, date);
        cJSON* product =  searchById(products, product_id);
        if (product == NULL){
            SEND_ERROR_404;
            return NULL_CONTENT;
        }
        
        char product_name_id[128];
        sprintf(product_name_id, "%s <span id =\"product_id\" style=\"color:#777;\">#%d</span>", cJSON_GetObjectItem(product, "name")->valuestring, product_id);
        

        char generale_sale_time[16] ;
        sprintf(generale_sale_time, "%d", get_sale_times(product_id, shop_id_manager));

        //diagram
        if (date[0] == 0){
            struct tm  *now = get_current_date();
            sprintf(date, "%d-%d", 1900 + now->tm_year, now->tm_mon + 1);
        }

        cJSON *shops = load_json_from_file(SHOPS_DATAFILE);
        if(shops == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        cJSON *shop = searchById(shops , shop_id_manager);


        char *diagram = get_performance_diagram(date, shop_id_manager, cJSON_GetObjectItem(shop, "name")->valuestring ,product_id);

        side_content =  c_html_render(PRODUCT_PERFORMANCE, (Props []){{
            product_name_id,
            "product_name_id",
            1
        },{
            generale_sale_time,
            "general",
            1
        },{
            diagram, 
            "diagram",
            1
        }}, 3);

        if (diagram !=  NULL){
            free(diagram);
        }


    }else if(strncmp(query, "/about" , strlen("/about")) == 0){
        active = "5";
        
        cJSON* shops_json = load_json_from_file(SHOPS_DATAFILE);
        if (shops_json ==  NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        cJSON *shop_data = searchById(shops_json, shop_id_manager);

        side_content = c_html_render( ABOUT , (Props[]){{
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
#undef SALES_LIST_MANAGER 
#undef INSIGHTS_MANAGER     
#undef EMPLOYEERS_MANAGER  
#undef NEWPRODUCT   
#undef PRODUCT_PERFORMANCE        
#undef ABOUT  
       
#endif
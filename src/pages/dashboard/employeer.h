#ifndef EMPLOYEER_H
#define EMPLOYEER_H

#define NOT_FOUND  "./layouts/404.html"

//employeer layouts
#define SIDEBAR "./layouts/employeer/sidebar.html"
#define INSIGHTS      "./layouts/employeer/insights.html"
#define NEWSALE      "./layouts/employeer/newsale.html"
#define SALES_LIST "./layouts/employeer/sales_list.html"
#define APPLY  "./layouts/employeer/apply.html"

dashboardContent employeer(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";
    int  shop_id =  cJSON_GetObjectItem(user_data, "shop_id")->valueint;
    dashboardContent NULL_CONTENT = {NULL, NULL};

    if (strlen(query) == 0 || strlen(query) == 1){ // `` or /
        char *sale_list= getSalesMadeBy(user_id);

        side_content =  c_html_render(SALES_LIST, (Props[]){{
            sale_list == NULL ?  (char *)" no sales " : sale_list,
            "sales_list",
            1
        }},1);

    }else if(strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "1";

        side_content = get_employeer_insights(user_id, INSIGHTS);
        if (side_content == NULL){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

    }else if(strncmp(query, "/newsale" , strlen("/newsale"))== 0){
        active = "2";

        //preparing product list  
        cJSON* shops_json = load_json_from_file(SHOPS_DATAFILE);
        cJSON *products_json = load_json_from_file(PRODUCTS_DATAFILE);
        if (shops_json ==  NULL || products_json == NULL){
            SEND_ERROR_500
            return NULL_CONTENT;
        }

        char *productname_list_html = NULL;

        cJSON* product = NULL;
        char template[128] = {'\0'};
        cJSON_ArrayForEach(product, products_json) {
            
            sprintf(template,"<option value=\"%d\">%s</option>" , 
                            cJSON_GetObjectItem(product, "id")->valueint,
                            cJSON_GetObjectItem(product, "name")->valuestring);
            append_to_string(&productname_list_html, template);

        }

        Props props[] = {{
                productname_list_html == NULL  ? (char *)" ": productname_list_html ,
                "available_products",
                1
            },{
                cJSON_Print(products_json),
                "products",
                1
            },{
                cJSON_Print(searchById(shops_json, shop_id)),
                "shop_data",
                1
            }};

        side_content = c_html_render(NEWSALE , props , 3);

        if (productname_list_html != NULL){
            free(productname_list_html);
        }


    }else if(strncmp(query, "/saleDetails" , strlen("/saleDetails"))== 0){ //sale details 
        active = "0";
        side_content = getSale_detailes_query(query);
        if (side_content == NULL){
            printf("\n>> cant  get  sale  details");
            SEND_ERROR_500;
            return NULL_CONTENT;
        }
    }else if(strncmp(query, "/apply" , strlen("/apply"))== 0){
        active = "3";
        cJSON *apps = load_json_from_file(APPS_MANAGER_DF);
        cJSON *shops = load_json_from_file(SHOPS_DATAFILE);
        if(apps == NULL|| shops == NULL){
            SEND_ERROR_500;
            return NULL_CONTENT;
        }

        char *html_list = NULL ; 

        const char *shop_template = 
                            "<div class=\"card mt-3\">"
                                "<div class=\"card-body\">"
                                    "<h4 class=\"card-title\">%s</h4>"
                                    "<a id = \"%s\" class=\"btn btn-success %s\" href=\"%s/%d\" >%s </a>"
                                "</div>"
                            "</div>";
        
        for (int i = 0;  i <  cJSON_GetArraySize(apps ) ;  i++){
            cJSON*  app = cJSON_GetArrayItem(apps , i);
            cJSON* shop = searchById(shops, cJSON_GetObjectItem(app,"shop_id")->valueint);
            cJSON* shop_apps = cJSON_GetObjectItem(app,"apps");
            char shop_html[1024];
            
            int applied =  0 ;  

            for (int i = 0 ;  i <  cJSON_GetArraySize(shop_apps) ; i++ ){
                int applier_id = cJSON_GetArrayItem(shop_apps, i)->valueint;
                if (applier_id == user_id){
                    applied =  1; 
                    break;
                }
            }

            if  (applied){
                sprintf(shop_html, shop_template,
                    cJSON_GetObjectItem(shop, "name")->valuestring ,
                    "",
                    "disabled",
                    "#",
                    0, 
                    "applied"
                );
            }else {
                sprintf(shop_html, shop_template,
                    cJSON_GetObjectItem(shop, "name")->valuestring ,
                    "apply_app" ,
                    "",
                    "/api/shops/apply",
                    cJSON_GetObjectItem(app,"shop_id")->valueint, 
                    "apply"
                );
            }
            append_to_string(&html_list, shop_html);
        }


        side_content = c_html_render(APPLY, (Props[]){{
            html_list == NULL  ? "<center> it seems  like there is no position open for now</center>" : html_list, 
            "shops", 
            1
        }} ,  1);

        if (html_list != NULL ){
            free(html_list);
        }

    }else{
        active = "a";
        side_content = c_html_render(NOT_FOUND, NULL , 0);
    }
    //end rendering sales
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
#undef INSIGHTS      
#undef NEWSALE     
#undef SALES_LIST 

#endif
#ifndef EMPLOYEER_H
#define EMPLOYEER_H

#define NOT_FOUND  "./layouts/404.html"

//employeer layouts
#define HTML_FILE_EMPLOYEER "./layouts/employeer/dashboard.html"
#define INSIGHTS      "./layouts/employeer/insights.html"
#define NEWSALE      "./layouts/employeer/newsale.html"
#define SALES_LIST "./layouts/employeer/sales_list.html"


void employeer(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";
    int  shop_id =  cJSON_GetObjectItem(user_data, "shop_id")->valueint;
    if (strlen(query) == 0 || strlen(query) == 1){ // `` or /
        char *sale_list= getSalesMadeBy(user_id);

        side_content =  c_html_render(SALES_LIST, (Props[]){{
            sale_list == NULL ?  (char *)" no sales " : sale_list,
            "sales_list",
            1
        }},1);

    }else if(strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "1";

        //getting  data  files 
        File_prop sales   = read_file(SALES_DATAFILE, "r");

        if(sales.content == NULL){
            printf("\n cant open one of the files :  %s ", SALES_DATAFILE);
            SEND_ERROR_500;
            return;
        } 
        //parsing data  
        cJSON *sales_json = cJSON_Parse(sales.content);
        free(sales.content);

        int sales_count = 0;
        float total_revenue = 0;
        int  week_sales_quantity[7] = {0};
        float week_sales[7] = {0};
        int year, month,day;//for date conversion

        for (int i =  cJSON_GetArraySize(sales_json)-1; i > -1 ; i--){
            cJSON* sale = cJSON_GetArrayItem(sales_json, i);
            if (cJSON_GetObjectItem(sale,"employee_id")->valueint == user_id){
                sales_count ++;
                total_revenue += cJSON_GetObjectItem(sale,"total_cost")->valuedouble;
                sscanf(cJSON_GetObjectItem(sale, "date")->valuestring, "%d-%d-%d", &year, &month,&day);
                int day_diff = days_between_dates(year, month, day);
                if (day_diff>-1 && day_diff < 7){
                    week_sales[6-day_diff] += cJSON_GetObjectItem(sale,"total_cost")->valuedouble; 
                    week_sales_quantity[6-day_diff] += 1;
                }
            }
        }

        //convert to  string
        time_t sale_time = time(NULL) - 7*60*60*24;

        char    sales_count_string[11] = {0},
                total_revenue_string[11] = {0}, 
                week_sales_string[80] = {0},// 10 digit number of sales for each day so : NNNNNNNNNN, = 11bytes * 7 = 77 + 1(NULL) = 78
                week_sales_quantity_string[50] = {},// 6 digit number of sales for each day so : NNNNNN, = 7bytes * 7 = 49 + 1(NULL) = 50
                week_days[100] = {0}; // 'yyyy-mm-dd', == 13 bytes * 7 = 81bytes  ; 
        for (int i =  0 ; i <  7 ; i++){
            sale_time += 60*60*24;
            struct tm *t = localtime(&sale_time);
            sprintf(week_sales_string + i*11, "%10.0f,",week_sales[i]);
            sprintf(week_sales_quantity_string + i*7, "%6d,",week_sales_quantity[i]);
            sprintf(week_days + i*13, "'%4d-%2d-%2d',", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
        }

        snprintf(sales_count_string, sizeof(sales_count_string), "%d", sales_count);
        snprintf(total_revenue_string, sizeof(total_revenue_string), "%.2f$", total_revenue);

        Props props[] = {{
            total_revenue_string,
            "total_revenue",
            1
        },{
            sales_count_string,
            "total_sales",
            1
        },{
            week_sales_string,
            "total_weeksales_revenue",
            1
        },{
            week_sales_quantity_string,
            "total_weeksales_quantity",
            1
        },{
            week_days,
            "days",
            2 //would be used  in two charts
        }};

        int props_length =  sizeof(props)/sizeof(Props);


        side_content = c_html_render(INSIGHTS, props , props_length);
    }else if(strncmp(query, "/newsale" , strlen("/newsale"))== 0){
        active = "2";

        //preparing product list 
        File_prop products   = read_file(PRODUCTS_DATAFILE ,"r"),
                  shops_data  = read_file(SHOPS_DATAFILE,"r");
        if(products.content == NULL|| shops_data.content == NULL){
                printf("\n cant open one of the files [from employeer/newsale]");
                SEND_ERROR_500;
                return;
            }  
        //parsing data  
        cJSON *products_json = cJSON_Parse(products.content);
        cJSON *shops_json = cJSON_Parse(shops_data.content);
        free(shops_data.content);
        free(products.content);

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
            return;
        }
    }else{
        active = "a";
        side_content = c_html_render(NOT_FOUND, NULL , 0);
    }
    //end rendering sales
    //render dashboard html 
    Props props[] ={
        {
            cJSON_GetObjectItem(user_data, "name")->valuestring,//to do :  check  if  the user name exist first
            "User_name",
            1
        }, 
        {   
            side_content,
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

    char  *rendered_content  =  c_html_render(HTML_FILE_EMPLOYEER, props , props_length);

    if (rendered_content == NULL){
        SEND_ERROR_500;
        return;
    }
    char header[256*2];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(rendered_content));

    send(client_socket, header, strlen(header), 0);
    send(client_socket, rendered_content , strlen(rendered_content), 0);

    free(side_content);
    free(rendered_content);
}


#endif
#ifndef DATA_HANDLERS_H
#define DATA_HANDLERS_H


//html temps
#define SALE_DETAILS "./layouts/general/sale_details.html"
#define PRODUCTS_LIST "./layouts/general/products_list.html"
#define DIAGRAM "./layouts/general/diagram.html"


char *getSale_detailes_query(char *query){
    //getting  sale id  from  the url 
    char *url_sale_id = query + strlen("/saleDetails/");
    char  *side_content;   
    printf("\n sale_id  %s", url_sale_id);
    
    char *endptr;  
    int sale_id =  strtol(url_sale_id , &endptr ,  10);

    if(*endptr == '\0'){

        //getting  data  files 
        cJSON *sales_json = load_json_from_file(SALES_DATAFILE);
        cJSON *sales_item_json = load_json_from_file(SALESITEMS_DATAFILE);
        cJSON *products_json = load_json_from_file(PRODUCTS_DATAFILE);
        if (sales_json ==  NULL || sales_item_json == NULL || products_json == NULL){
            return NULL;
        }

        //getting sale information
        cJSON *sale_data =  searchById(sales_json, sale_id);
        char str_cost[10];
        sprintf(str_cost, "%.2f", cJSON_GetObjectItem(sale_data, "total_cost")->valuedouble);
        char str_id[15]; 
        sprintf(str_id, "%d", cJSON_GetObjectItem(sale_data, "id")->valueint);

        //getting  products of this sale  
        char *products_list_html = NULL;

        cJSON *sale_products =  searchById_cutomized(sales_item_json, sale_id, "sale_id");
        if (sale_products != NULL){
            const char* product_item_html =   "<tr>\n"
                                        "    <td>%d</td>\n"
                                        "    <td>%s</td>\n"
                                        "    <td>%s</td>\n"
                                        "    <td>%.2lf</td>\n"
                                        "    <td>%d</td>\n"
                                        "    <td>$%.2lf</td>\n"
                                        "    <td>$%.2lf</td>\n"
                                        "</tr>\n";
            char html_template[512] = {'\0'};

            cJSON *products_list =  cJSON_GetObjectItem( sale_products , "products");
            
            cJSON* product = NULL;
            cJSON_ArrayForEach(product, products_list) {
                int product_id = cJSON_GetObjectItem(product, "product_id")->valueint;
                //getting the rest  of the data  
                cJSON* product_details = searchById(products_json, product_id);

                sprintf(html_template, product_item_html, 
                        product_id,
                        cJSON_GetObjectItem(product_details, "name")->valuestring,
                        cJSON_GetObjectItem(product_details, "category")->valuestring,
                        cJSON_GetObjectItem(product_details, "rating")->valuedouble,
                        cJSON_GetObjectItem(product, "quantity")->valueint,
                        cJSON_GetObjectItem(product_details, "price")->valuedouble,
                        cJSON_GetObjectItem(product_details, "price")->valuedouble * 
                        cJSON_GetObjectItem(product, "quantity")->valueint
                    );
                append_to_string(&products_list_html, html_template);

            }

        }
        //supposing that  the data  is clean and dont need to  be treated
        Props props[] = {{
            str_id,
            "sale_id",
            1
        },{
            cJSON_GetObjectItem(sale_data, "date")->valuestring,
            "sale_date",
            1 
        },{
            cJSON_GetObjectItem(sale_data, "notes")->valuestring,
            "sale_notes",
            1  
        },{
            str_cost,
            "total_cost",
            1 
        },{
            
            products_list_html == NULL ? 
            (char *)"no products" : products_list_html, 

            "products_list",
            1
        }};
        int props_length  =  sizeof(props)/sizeof(Props);
        side_content = c_html_render(SALE_DETAILS , props , props_length);

        if (products_list_html != NULL){
            free(products_list_html);
        }

    }else {
        side_content = c_html_render(SALE_DETAILS , NULL , 0);
    }

    return side_content;
}

char *getSalesMadeBy (int user_id){

    //getting  data  files 
    cJSON *sales_json = load_json_from_file(SALES_DATAFILE);
    if (sales_json ==  NULL){
        return NULL;
    }


    const char *sale_html_item =  "<div class=\"card mb-4\" >"
                            "  <div class=\"card-header\">"
                            "    sale id %d"
                            "  </div>"
                            "  <div class=\"card-body\">"
                            "    <p class=\"card-text\">%s</p>"
                            "    <p class=\"card-text\">totale cost : <strong>%.2lf$</strong></p>"
                            "    <footer class=\"blockquote-footer\">"
                            "      %s"
                            "    </footer>"
                            "    <a href=\"/dashboard/saleDetails/%d\" class=\"btn btn-primary\">more details</a>"
                            "    <button class=\"btn btn-warning removeSale\" data-saleid=\"%d\" >cancel</button>"
                            "  </div>"
                            "</div>";
    char  *sales_list = NULL;

    // Iterate over JSON array
    cJSON *item;


    for(int i= cJSON_GetArraySize(sales_json) - 1;  i > -1 ;  i--){
        item = cJSON_GetArrayItem(sales_json, i);
        cJSON *id = cJSON_GetObjectItem(item, "employee_id");
        if (cJSON_IsNumber(id) && id->valueint == user_id) {
            cJSON *sale_id = cJSON_GetObjectItem(item, "id"); 
            cJSON *date = cJSON_GetObjectItem(item, "date"); 
            cJSON *notes = cJSON_GetObjectItem(item, "notes"); 
            cJSON *total_cost = cJSON_GetObjectItem(item, "total_cost"); 
            if (cJSON_IsString(date) && cJSON_IsString(notes) && cJSON_IsNumber(total_cost)) {

                char html_template[1024] = {'\0'};
                
                sprintf(html_template, sale_html_item ,   sale_id->valueint,
                                                        notes->valuestring,
                                                        total_cost->valuedouble,
                                                        date->valuestring,
                                                        sale_id->valueint,
                                                        sale_id->valueint);

                append_to_string(&sales_list, html_template);

            }
        }
    }

    return sales_list;
}

char *get_insights(char *HTML_TEMPLATE, int shop_id, int is_admin){


    char *shops_list =  NULL;
    if (is_admin){
        char  *shops_list_temp = "<li><a class=\"dropdown-item\" href=\"/dashboard/insights/%d\"> %s </a></li>"; 
        cJSON *shops = load_json_from_file(SHOPS_DATAFILE);
        if(shops == NULL){
            return NULL;
        }

        for(int i = 0 ; i < cJSON_GetArraySize(shops);  i++){
            cJSON* shop =  cJSON_GetArrayItem(shops, i);

            char shop_html[128];

            sprintf(shop_html, shops_list_temp, 
                cJSON_GetObjectItem(shop, "id")->valueint,
                cJSON_GetObjectItem(shop, "name")->valuestring
            );

            append_to_string(&shops_list, shop_html);

        }

    }


    char *side_content;
    //getting  data  files 
    cJSON *sales_json = load_json_from_file(SALES_DATAFILE);
    cJSON *employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
    if (sales_json ==  NULL  || employeers_json == NULL){
        return NULL;
    }


    int sales_count = 0;
    float total_revenue = 0;
    int  week_sales_quantity[7] = {0};
    float week_sales[7] = {0};
    int year, month,day;//for date conversion

    for (int i =  cJSON_GetArraySize(sales_json)-1; i > -1 ; i--){
        cJSON* sale = cJSON_GetArrayItem(sales_json, i);
        if (
            cJSON_GetObjectItem(sale, "shop_id")->valueint ==  shop_id ||
            shop_id == -1 //-1 means get every thing
        ){
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


    //getting the contribution of each employeer 
    ElementCount occurenceCount[cJSON_GetArraySize(sales_json)];
    int Occurence_Size ;

    countOccurrencesById_accumulate_shop(sales_json , "employee_id","total_cost",occurenceCount, &Occurence_Size, shop_id);

    //convert to  string
    char *occurence_colors = (char*)malloc(Occurence_Size * 30 + 1); // 'rgb(244, 123, 100),' == 22bytes
    char *occurence_name_id = (char*)malloc(Occurence_Size * 64 + 1); // 'name#id' <= 50 bytes
    char *occurence_data = (char*)malloc(Occurence_Size * 7 + 1); // NNNNNN, <= 7 bytes
    char *occurence_revenue_data = (char*)malloc(Occurence_Size * 7 + 1);

    for(int i= 0; i < Occurence_Size ; i++){

        sprintf(occurence_data + i*7, "%6d,", occurenceCount[i].count);
        sprintf(occurence_revenue_data + i*7, "%6.0f,", occurenceCount[i].accumulate);
        sprintf(occurence_colors + i*30 ,"%30s", my_colors[i%30]); // to  loop  through colors
        cJSON *employeer = searchById(employeers_json, occurenceCount[i].id);
        printf(">> %s <<", cJSON_Print(employeer));
        sprintf(occurence_name_id + i*64 ,"'%52s#%8d',", cJSON_GetObjectItem(employeer, "name")->valuestring ,  occurenceCount[i].id);

    }

    printf("\n>>\n%s \n%s \n%s" , occurence_colors, occurence_name_id, occurence_data);

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
        shops_list == NULL ? "no shops" :  shops_list,
        "shops_list",
        1
    },{
        total_revenue_string,
        "total_revenue",
        1
    },{
        sales_count_string,
        "total_sales",
        1
    },{
        occurence_colors,
        "colors",
        2
    },{
        occurence_data,
        "contributions",
        1
    },{
        occurence_revenue_data,
        "cont_revenue",
        1
    },{
        occurence_name_id,
        "cont_labels",
        2
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

    side_content = c_html_render(HTML_TEMPLATE, props , props_length);

    free(occurence_colors);
    free(occurence_name_id);
    free(occurence_data);
    free(occurence_revenue_data);
    if (shops_list != NULL){
        free(shops_list);
    }
    return side_content;
}

int  get_totale_sales(cJSON *sales , int emp_id){
    int  count  = 0 ; 


    for (int i= 0; i <  cJSON_GetArraySize(sales) ;  i++){
        cJSON* sale = cJSON_GetArrayItem(sales, i);
        if (cJSON_GetObjectItem(sale, "employee_id")->valueint == emp_id){
            count ++;
        }
    }

    return count;

}

char *get_employeer_insights(int user_id, char* HTML_TEMPLATE){

    cJSON* sales_json = load_json_from_file(SALES_DATAFILE);
    if (sales_json ==  NULL){
        return NULL;
    }

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


    return c_html_render(HTML_TEMPLATE , props , props_length);
}

char *get_products_list(){

    cJSON *products =  load_json_from_file(PRODUCTS_DATAFILE);
    if (products ==  NULL){
        return NULL;
    }

    char *html_list = NULL ; 

    const char *product_template = 
                        "<div class=\"card mt-3\">"
                            "<div class=\"card-body\">"
                                "<h4 class=\"card-title\">%s</h4>"
                                "<p class=\"card-text\">category : %s</p>"
                                "<p class=\"card-text\">price : %.2f</p>"
                                "<p class=\"card-text\">description : %s</p>"
                                "<p class=\"card-text\">rating : %.1f</p>"
                                "<a class=\"btn btn-success\" href=\"/dashboard/ProductPerformance/%d\" >perfomance</a>"
                            "</div>"
                        "</div>";
    
    for (int i = 0;  i <  cJSON_GetArraySize(products) ;  i++){
        cJSON*  product = cJSON_GetArrayItem(products, i);
        char product_html[1024];
        sprintf(product_html, product_template,
            cJSON_GetObjectItem(product, "name")->valuestring ,
            cJSON_GetObjectItem(product, "category")->valuestring ,
            cJSON_GetObjectItem(product, "price")->valuedouble,
            cJSON_GetObjectItem(product, "description")->valuestring ,
            cJSON_GetObjectItem(product, "rating")->valuedouble,
            cJSON_GetObjectItem(product, "id")->valueint
        );

        append_to_string(&html_list, product_html);
    }

    char  *content = c_html_render(PRODUCTS_LIST, (Props[]){{
                        html_list == NULL ?  "no products" :  html_list, 
                        "products",
                        1
                    }}, 1);


    if (html_list != NULL ) free(html_list);

    return  content;
}

int get_sale_times(int product_id,int shop_id){

    int count = 0; 

    cJSON  *sales_items =  load_json_from_file(SALESITEMS_DATAFILE),
           *sales       =  load_json_from_file(SALES_DATAFILE);
    if (sales_items == NULL || sales == NULL){
        return count;
    }

    for  (int i= 0; i < cJSON_GetArraySize(sales_items) ;  i++){
        cJSON* items = cJSON_GetArrayItem(sales_items, i);
        int  sale_id = cJSON_GetObjectItem(items , "sale_id")->valueint;
        cJSON* sale  = searchById(sales, sale_id);

        if (sales != NULL 
            && (cJSON_GetObjectItem(sale,"shop_id")->valueint == shop_id || shop_id == -1)){
            
            cJSON* products_list  = cJSON_GetObjectItem(items, "products");
            for (int i = 0; i <  cJSON_GetArraySize(products_list) ;  i++){
                cJSON* product = cJSON_GetArrayItem(products_list, i);
                if  (cJSON_GetObjectItem(product, "product_id")->valueint == product_id){
                    count += cJSON_GetObjectItem(product, "quantity")->valueint;
                }
            }
        }
    }

    return count;
}


char *get_performance_diagram(char *date, int shop_id, char *shop_name , int product_id){
    int year, mon, mon_times[31] = {0};
    sscanf(date,"%d-%d", &year, &mon);

    cJSON  *sales_items =  load_json_from_file(SALESITEMS_DATAFILE),
           *sales       =  load_json_from_file(SALES_DATAFILE);
    if (sales_items == NULL || sales == NULL){
        return NULL;
    }

    for  (int i= 0; i < cJSON_GetArraySize(sales_items) ;  i++){
        cJSON* items = cJSON_GetArrayItem(sales_items, i);
        int  sale_id = cJSON_GetObjectItem(items , "sale_id")->valueint;
        cJSON* sale  = searchById(sales, sale_id);
        char *sale_date = cJSON_GetObjectItem(sale,"date")->valuestring;

        int sale_year, sale_mon, sale_day;
        sscanf(sale_date,"%d-%d-%d", &sale_year, &sale_mon, &sale_day);
        if (sales != NULL 
            && (cJSON_GetObjectItem(sale,"shop_id")->valueint == shop_id || shop_id == -1)
            && sale_year ==  year && sale_mon == mon){
            
            cJSON* products_list  = cJSON_GetObjectItem(items, "products");
            for (int i = 0; i <  cJSON_GetArraySize(products_list) ;  i++){
                cJSON* product = cJSON_GetArrayItem(products_list, i);
                if  (cJSON_GetObjectItem(product, "product_id")->valueint == product_id){
                    mon_times[sale_day-1] += cJSON_GetObjectItem(product, "quantity")->valueint;
                }
            }
        }
    }

    //convert  to srting 

    char data_string[256]; //6 * 31 :  186 [ NNNNN, * 31 ]
    for (int i =  0;  i < 31 ;  i++){
        sprintf(data_string + i * 6, "%5d,", mon_times[i]);
    }


    return c_html_render(DIAGRAM, (Props []){{
        data_string, 
        "data", 
        1
    },{
        date, 
        "date",
        1
    },{
        shop_name, 
        "shop_name",
        1
    }}, 3);
}


#endif
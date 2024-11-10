#ifndef MANAGER_H
#define MANAGER_H

#define NOT_FOUND  "./layouts/404.html"
//manager layouts
#define HTML_FILE_MANAGER "./layouts/manager/dashboard.html"
#define SALES_LIST_MANAGER "./layouts/manager/sales_list.html"
#define INSIGHTS_MANAGER      "./layouts/manager/insights.html"
#define EMPLOYEERS_MANAGER  "./layouts/manager/employeers.html"
#define NEWPRODUCT          "./layouts/manager/newProduct.html"
#define ABOUT          "./layouts/manager/about.html"

void manager(char *query, char *body, int user_id, SOCKET client_socket, cJSON *user_data){
    char *active = "0";
    char *side_content = " no  content ";  
    //shop_id of the manager
    int shop_id_manager = cJSON_GetObjectItem(user_data, "shop_id")->valueint;

    if (strlen(query) == 0 || strlen(query) == 1 || strncmp(query, "/sales" , strlen("/sales"))== 0 ){

        File_prop employeers = read_file(EMPLOYEES_DATAFILE ,"r");
        if(employeers.content == NULL){
            printf("\n cant open one of the files :  %s", EMPLOYEES_DATAFILE);
            SEND_ERROR_500;
            return ;
        }  
        //parsing data  
        cJSON *employeers_json = cJSON_Parse(employeers.content);
        free(employeers.content);
        if (employeers_json == NULL){
            printf("\n cant parse data one of the files :  %s", EMPLOYEES_DATAFILE);
            SEND_ERROR_500;
            return ;
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
                        "<li><a class=\"dropdown-item\" href=\"dashboard/sales/%d\">%s <span style=\"opacity:0.5\">#%d</span></a></li>",
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
            char *name = cJSON_GetObjectItem(emp, "name")->valuestring;
            snprintf(employeers_name, sizeof(employeers_name), "<h3> theses are %s's sales :</h3>", name);
            is_sale_list_malloc = 1;
        }

        side_content = c_html_render(SALES_LIST_MANAGER, (Props []){{
            sales_list == NULL ? "<p style=\"text-align:center;\"> no  sales </p>":sales_list,
            "sales_list",
            1
        },{
            drop_down,
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
            return;
        }



    }else if(strncmp(query, "/saleDetails" , strlen("/saleDetails"))== 0){ //sale details 
    
        side_content = getSale_detailes_query(query);
        if (side_content == NULL){
            printf("\n>> cant  get  sale  details");
            SEND_ERROR_500;
        }
    }else if(strncmp(query, "/insights" , strlen("/insights"))== 0){
        active = "1";

        //getting  data  files 
        File_prop sales   = read_file(SALES_DATAFILE, "r"),
                employeers = read_file(EMPLOYEES_DATAFILE ,"r");

        if(sales.content == NULL || employeers.content == NULL){
            printf("\n cant open one of the files :  %s %s ", SALES_DATAFILE, EMPLOYEES_DATAFILE);
            SEND_ERROR_500;
            return;
        } 
        //parsing data  
        cJSON *sales_json = cJSON_Parse(sales.content);
        cJSON *employeers_json = cJSON_Parse(employeers.content);
        free(employeers.content);
        free(sales.content);

        if (!sales_json || !cJSON_IsArray(sales_json) || !employeers_json || !cJSON_IsArray(employeers_json)) {
            printf("Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }

        int sales_count = 0;
        float total_revenue = 0;
        int  week_sales_quantity[7] = {0};
        float week_sales[7] = {0};
        int year, month,day;//for date conversion

        for (int i =  cJSON_GetArraySize(sales_json)-1; i > -1 ; i--){
            cJSON* sale = cJSON_GetArrayItem(sales_json, i);
            if (
                cJSON_GetObjectItem(sale, "shop_id")->valueint ==  shop_id_manager
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

        countOccurrencesById_accumulate_shop(sales_json , "employee_id","total_cost",occurenceCount, &Occurence_Size, shop_id_manager);

        //convert to  string
        char *occurence_colors = (char*)malloc(Occurence_Size * 30 + 1); // 'rgb(244, 123, 100),' == 22bytes
        char *occurence_name_id = (char*)malloc(Occurence_Size * 64 + 1); // 'name#id' <= 50 bytes
        char *occurence_data = (char*)malloc(Occurence_Size * 7 + 1); // NNNNNN, <= 7 bytes
        char *occurence_revenue_data = (char*)malloc(Occurence_Size * 7 + 1);

        for(int i= 0; i < Occurence_Size ; i++){

            sprintf(occurence_data + i*7, "%6d,", occurenceCount[i].count);
            sprintf(occurence_revenue_data + i*7, "%6.0f,", occurenceCount[i].accumulate);
            sprintf(occurence_colors + i*30 ,"%30s", my_colors[i]);
            cJSON *employeer = searchById(employeers_json, occurenceCount[i].id);
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

        side_content = c_html_render(INSIGHTS_MANAGER, props , props_length);

        free(occurence_colors);
        free(occurence_name_id);
        free(occurence_data);
        free(occurence_revenue_data);

    }else if(strncmp(query, "/employeers" , strlen("/employeers"))== 0){
        active = "2"; 
        //getting  data  files 
        File_prop employeers = read_file(EMPLOYEES_DATAFILE ,"r");

        if(employeers.content == NULL){
            printf("\n cant open one of the files :  %s ", EMPLOYEES_DATAFILE);
            SEND_ERROR_500;
            return;
        } 
        //parsing data  
        cJSON *employeers_json = cJSON_Parse(employeers.content);
        free(employeers.content);

        if (!employeers_json || !cJSON_IsArray(employeers_json)) {
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
            employeers_list,
            "employeers_list",
            1
        }} , 1);

        if (employeers_list != NULL){
            free(employeers_list);
        }
    }else if(strncmp(query, "/newProduct" , strlen("/newProduct"))== 0){
        active = "3";
        side_content = c_html_render( NEWPRODUCT , NULL, 0);
    }else if(strncmp(query, "/about" , strlen("/about")) == 0){
        active = "4";
        
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
            printf("[from manager/about] Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
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

    char  *rendered_content  =  c_html_render(HTML_FILE_MANAGER, props , props_length);

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
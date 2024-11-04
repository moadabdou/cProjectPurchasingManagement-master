#include "cJSON.h"




//http_header
//cookies
typedef struct Cookie{
    char name[32]; 
    char value[128];
}Cookie;
Cookie  get_cookie(char *buffer , const char *name);

//sessions
typedef struct  Sessions
{
    int  *sessions;
    int  max ;
}Sessions;

char open_session(Sessions SESSIONS,  int id);
void close_session(Sessions SESSIONS , int id );
char check_in_sessions(Sessions SESSIONS, int id);

//files.c
typedef struct File_prop{
    char *content ; 
    long long length ;
}File_prop;

File_prop read_file(const char *filename, const char *mode);
void write_file(const char *filename, const char *data, const char *mode);

//math_tools.c
int generate_id();

//indexed_array.c
void  update_data_in_indexed_array(cJSON *obj,cJSON *obj_array , char *identifier);
void  delete_data_in_indexed_array_id(int id ,cJSON *obj_array, char *identifier);
cJSON* searchById(cJSON* jsonArray, int targetId);
cJSON* searchById_cutomized(cJSON* jsonArray, int targetId, char *id_tag);

typedef struct {
    int id;
    float accumulate;
    int count;
} ElementCount;
void countOccurrencesById_accumulate(cJSON *array, char *id_tag, char *accumulate, ElementCount result[], int *resultSize);

//dynamic_memory
void append_to_string(char **buffer, const char *new_string);


//date_calculations  
int days_between_dates(int year, int month, int day);
struct tm* get_current_date();
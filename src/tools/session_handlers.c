#include "tools.h"


char open_session(Sessions SESSIONS,  int id){
    int i  =  0; 
    for(;  i <  SESSIONS.max ; i++){
        if (SESSIONS.sessions[i] == id){
            return -1;
        }else if (SESSIONS.sessions[i] == 0){
            SESSIONS.sessions[i] =  id ;
            return 1;
        }
    }
    
    return 0; //0 not open ,  1 open , -1 already exist
}

void close_session(Sessions SESSIONS , int id ){
    int i  =  0; 
    for(;  i <  SESSIONS.max ; i++){
        if (SESSIONS.sessions[i] == id ){
            SESSIONS.sessions[i] =  0 ;
            return;
        }
    }
}

char check_in_sessions(Sessions SESSIONS, int id){
    int i  =  0; 

    for(;  i <  SESSIONS.max ; i++){
        if (SESSIONS.sessions[i] ==  id ){
            return 1;
        }
    }
    
    return 0; //0 not exist ,  1 exist 
}

typedef struct Props{
    char  *data;
    char  *tag;
    int  count;
}Props;
char *c_html_render(char *src, Props *props, int props_length);
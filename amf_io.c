#include "amf_io.h"

//Implementation specifics functions
//Might need to be changed deppendin on where
//ASC mini Forth is ment to be embedded

//amf_input: this function returns a char entered by the input. The char should be echoed
//amf_output: print a char to the user
//amf_init_io: ran once when starting a shell, used to init what is needed by amf_input and amf_output
//amf_clean_io: clean memory used by amf_init_io

#include "stdio.h"
char amf_input(void){
    char ret = getchar();
    if(ret != -1){ //Check for unexpected char that could do bad things to  the rest of the parser
        return ret;
    }else{
        return ' ';
    }
}

void amf_output(char ch){
    putchar(ch);
}

void amf_init_io(void){}
void amf_clean_io(void){}

//Function that should not change depending on the implementation
#include "string.h"

//analogous to puts
void amf_print_string(const char* str){
    for(size_t i=0; i<strlen(str); i++){
        amf_output(str[i]);
    }
}

//print a number I will, in the future, make it lighter than using sprintf
void amf_print_num(amf_int_t num){
    char buf[100];
    sprintf(buf, "%" AMF_INT_PRINT " ", num);
    amf_print_string(buf);
}

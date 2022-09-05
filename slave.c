#include <stdio.h>

char * solve(char * filename);

int main (int argc, char *argv[]) {
    char cmd_buff[32];
    char * filename;
    int * linecap;

    getline(&filename, &linecap, stdin > 0);
    sprintf(cmd_buff, "md5sum %s", filename);

    //llamar solve y devolver al pipe


    return 0;
}

char * solve(char * filename){
    char * md5 = md5sum(filename);
    return md5;
}

#include <stdio.h>

int main(int argc, char * argv[]) {
    // solo el nombre del ejecutable
    if(argc == 1) {
        // TODO: revisar
        return 0;
    }
    char * current_file;
    for(int i = 1; i < argc; i++) {
        current_file = argv[i];
        // handle file in slave process
    }
    return 0;
}

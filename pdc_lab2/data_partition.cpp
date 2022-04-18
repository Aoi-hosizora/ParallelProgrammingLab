#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "predefined.h"

int main(int argc, char *agrv[]) {
    FILE *fp_in;
    if ((fp_in = fopen("./256M.txt", "rb")) == NULL) {
        printf("open file error \n");
        exit(1);
    }

    int i = 0;
    char outpaths[P][MAX_PATH_LEN] = {0};
    for (i = 0; i < P; i++) {
        char buf[MAX_PATH_LEN] = {0};
        sprintf(buf, "./256M_%d.txt", i + 1);
        strcpy(outpaths[i], buf);
    }

    FILE *fps[P] = {0};
    for (i = 0; i < P; i++) {
        fps[i] = fopen((const char *) (outpaths[i]), "wb");
    }

    while (!feof(fp_in)) {
        float buf[P] = {0};
        fread(buf, sizeof(float), P, fp_in);
        for (i = 0; i < P; i++) {
            fwrite(buf + i, sizeof(float), 1, fps[i]);
        }
    }

    fclose(fp_in);
    for (i = 0; i < P; i++) {
        fclose(fps[i]);
    }
    return 0;
}

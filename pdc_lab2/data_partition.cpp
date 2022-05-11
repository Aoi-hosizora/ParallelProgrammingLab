#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "predefined.h"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Usage: %s <Partition count>\n", argv[0]);
        exit(1);
    }
    int p = atoi(argv[1]);
    if (p == 0) {
        printf("Usage: %s <Partition count>\n", argv[0]);
        exit(1);
    }

    FILE *fp_in;
    if ((fp_in = fopen(FILENAME, "rb")) == NULL) {
        printf("open file error \n");
        exit(1);
    }

    int i = 0;
    char outpaths[p][MAX_PATH_LEN] = {0};
    for (i = 0; i < p; i++) {
        char buf[MAX_PATH_LEN] = {0};
        sprintf(buf, FILENAME_FMT, i + 1);
        strcpy(outpaths[i], buf);
    }

    FILE *fps[p] = {0};
    for (i = 0; i < p; i++) {
        fps[i] = fopen((const char *) (outpaths[i]), "wb");
    }

    while (!feof(fp_in)) {
        float buf[p] = {0};
        fread(buf, sizeof(float), p, fp_in);
        for (i = 0; i < p; i++) {
            fwrite(buf + i, sizeof(float), 1, fps[i]);
        }
    }

    fclose(fp_in);
    for (i = 0; i < p; i++) {
        fclose(fps[i]);
    }
    return 0;
}

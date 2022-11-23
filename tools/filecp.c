#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("usage: desfile offset srcfile.\n");
        return 0;
    }
    char* desFile = argv[1];
    int offset = atoi(argv[2]);
    char* srcFile = argv[3];
    FILE* df = fopen(desFile, "rb+");
    if (!df) {
        printf("open desfile error.\n");
        return 0;
    }
    FILE* sf = fopen(srcFile, "rb+");
    if (!sf) {
        printf("open srcfile error.\n");
        return 0;
    }
    fseek(sf, 0, SEEK_END);
    int len = ftell(sf);
    char* buf = malloc(len);
    if (buf == NULL) {
        printf("malloc error.\n");
        return 0;
    }
    fseek(sf, 0, SEEK_SET);
    fread(buf, 1, len, sf);

    fseek(df, offset, SEEK_SET);
    fwrite(buf, 1, len, df);
    printf("%s(%dbytes) -> %s(%doffset)\n", srcFile, len, desFile, offset);

    fclose(df);
    fclose(sf);
}


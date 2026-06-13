#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
int main(int argc, char *argv[])
{
    if (argc > 1)
    {

        FILE *f;
        f = fopen(argv[1], "r");
        if (!f)
        {
            if (_access(argv[1], 0) == 0)
                printf("文件无法打开\n");
            else
                printf("文件不存在\n");
            return 1;
        }
        int a = 0, b = 0, c = 0, ch, in = 0;
        while ((ch = fgetc(f)) != EOF)
        {
            a++;
            if (ch == '\n')
                b++;
            if (isspace(ch))
            {
                if (in)
                    c++;
            }
            else
                in = 1;
        }
        if (a > 0)
        {
            if (ch != '\n')
                b++;
            if (in)
                c++;
        }
        char s[256];
        snprintf(s, sizeof(s), "字符数: %d\n单词数: %d\n行数: %d\n", a, b, c);
        printf(s);
        FILE *f1 = fopen("result.txt", "w");
        if (!f1)
        {
            printf("写入result.txt失败\n");
            return 1;
        }
        fputs(s, f1);
        fclose(f1);
        fclose(f);
    }
}
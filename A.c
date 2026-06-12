#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
void pos(int x, int y,...)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hConsole, pos);
    va_list args;
    va_start(args, y);
    const char* fmt = va_arg(args, const char*);
    vprintf(fmt, args);
    va_end(args);
}
int main()
{
    struct A
    {
        char name[20];
        float score[3];
    } student[5];
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    for (int i = 0; i < 5; i++)
    {
        printf("请输入第%d个学生的信息：\n", i + 1);
        printf("请输入学生姓名：");
        scanf("%s", student[i].name);
        printf("请输入学生的三门成绩(用空格分隔)：");
        for (int j = 0; j < 3; j++)
        {
            scanf("%f", &student[i].score[j]);
        }
    }
    float sum[5];
    for(int i=0;i<5;i++){
        sum[i]=0;
        for(int j=0;j<3;j++){
            sum[i]+=student[i].score[j];
        }
    }
    int max=sum[0],index[5],in=0;
    for(int i=0;i<5;i++){
        index[i]=0;
    }
     system("cls");
    pos(0, 0,"学生姓名");
    pos(12, 0,"语文");
    pos(20, 0,"数学"); 
    pos(28, 0,"英语");
    pos(36, 0,"总成绩");
    pos(44, 0,"平均分");
    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            if(index[j]==0 && max<sum[j]){
                max=sum[j];
                in=j;
            }
        }
        index[in]=1;
       pos(0, 1+i,student[in].name);
       pos(12, 1+i,"%.2f",student[in].score[0]);
       pos(20, 1+i,"%.2f",student[in].score[1]);
       pos(28, 1+i,"%.2f",student[in].score[2]);
       pos(36, 1+i,"%.2f",sum[in]);
       pos(44, 1+i,"%.2f",sum[in]/3.0);

        max=0;
    }
    float maxf=0;
    char ss[][20]={"语文","数学","英语"};
    for(int i=0;i<3;i++){
        for(int j=0;j<5;j++){
                if(maxf<student[j].score[i]){
                    maxf=student[j].score[i];
                    in=j;
            }
        }
        printf("\n%s成绩最高的学生是：%s，成绩为：%.2f", ss[i], student[in].name, maxf);
         maxf=0;
    }
    return 0;
}
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <stdarg.h>
#include <stdlib.h>
HANDLE cin, stop, del, movl, movr, enter;
HANDLE _posX, _input;
char input[128] = "";
int posX = 0;

#define Trace(type)                                           \
    typedef struct trace_##type trace_##type;                 \
    struct trace_##type                                       \
    {                                                         \
        int n, m;                                             \
        type *a;                                              \
        void (*push)(trace_##type * q, type);                 \
        type (*pop)(trace_##type * q);                        \
        type (*fr)(trace_##type * q, int in);                 \
    };                                                        \
    void push_##type(trace_##type *q, type v)                 \
    {                                                         \
        if (q->n < q->m)                                      \
        {                                                     \
            q->a[q->n++] = v;                                 \
        }                                                     \
        else                                                  \
        {                                                     \
            type *p = realloc(q->a, 2 * q->m * sizeof(type)); \
            if (!p)                                           \
            {                                                 \
                exit(1);                                      \
            }                                                 \
            q->a = p;                                         \
            q->m *= 2;                                        \
            q->a[q->n++] = v;                                 \
        }                                                     \
    }                                                         \
    type pop_##type(trace_##type *q)                          \
    {                                                         \
        if (q->n > 0)                                         \
        {                                                     \
            return q->a[--q->n];                              \
        }                                                     \
    }                                                         \
    type fr_##type(trace_##type *q, int in)                   \
    {                                                         \
        return q->a[q->n - in - 1];                           \
    }                                                         \
    void NewTrace_##type(trace_##type *a)                     \
    {                                                         \
        a->a = malloc(20 * sizeof(type));                     \
        if (!a->a)                                            \
        {                                                     \
            exit(1);                                          \
        }                                                     \
        else                                                  \
        {                                                     \
            a->n = 0;                                         \
            a->m = 20;                                        \
            a->push = push_##type;                            \
            a->pop = pop_##type;                              \
            a->fr = fr_##type;                                \
        }                                                     \
    }

Trace(char)
    trace_char tc;
Trace(int)
    trace_int ti,
    eri, erii;
Trace(double)
    trace_double td;
void setPos(int x, int y)
{
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void printL(int x, int y, const char *s, ...)
{
    setPos(x, y);
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
}
void log(const char *s, ...)
{
    setPos(0, 4);
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
}
void err(int i, int state)
{
    eri.push(&eri, i);
    erii.push(&erii, state);
}
void errlog()
{
    static int old;
    char *s[] = {"右括号未匹配", "运算符前面缺少数字", "小数点前面必须有数字", "数字不合法", "小数点只能有一个", "前面缺少运算符", "连续运算符", "左括号未匹配"};
    for (int i = 0; i < old; i++)
    {
        printL(0, 5 + i, "                                                     ");
    }
    for (int i = 0; i < eri.n; i++)
    {
        printL(eri.a[i], 5 + i, s[erii.a[i]]);
    }
    old = eri.n;
}
int check()
{
    eri.n = 0;
    erii.n = 0;
    ti.n = 0;
    int state = 0, iu = 0, iiu = 0;
    for (int i = 0; input[i]; i++)
    {
        if (input[i] == ' ')
            continue;
        switch (state)
        {
        case 0:
            if (input[i] == '(')
            {
                ti.push(&ti, i);
            }
            else if (input[i] == ')')
            {

                if (ti.n <= 0)
                    err(i, 0);
                else
                    ti.pop(&ti);
            }
            else if (strchr("+-*/", input[i]))
            {
                if (iu == 0)
                    err(i, 1);
                else
                {
                    if (iiu == 0)
                        err(i, 6);
                    iiu = 1;
                }
            }
            else if (input[i] == '.')
            {
                err(i, 2);
                i++;
                while (input[i] && strchr("0123456789.", input[i]))
                    i++;
                i--;
                state = 1;
            }
            else
            {
                if (input[i] == '0')
                {
                    int in = i;
                    if (input[i + 1] && input[i + 1] == '.')
                    {
                        if (input[i + 2] && strchr("0123456789", input[i + 2]))
                        {
                            i += 2;
                            while (input[i] && strchr("0123456789", input[i]))
                                i++;
                            i--;
                        }
                        else
                        {
                            while (input[i] && strchr("0123456789.", input[i]))
                                i++;
                            i--;
                            err(in, 3);
                        }
                    }
                }
                else
                {
                    while (input[i] && strchr("0123456789", input[i]))
                        i++;
                    int t = 0;
                    while (input[i] && input[i] == '.')
                    {
                        i++;
                        if (t == 1)
                        {
                            err(i - 1, 4);
                            t = 2;
                        }
                        else if (t == 0)
                        {
                            t = 1;
                        }
                        while (input[i] && strchr("0123456789", input[i]))
                            i++;
                    }
                    i--;
                }
                state = 1;
            }
            break;
        case 1:
            if (input[i] == '(')
            {
                ti.push(&ti, i);
                err(i, 5);
            }
            else if (input[i] == ')')
            {
                if (ti.n <= 0)
                    err(i, 0);
                else
                    ti.pop(&ti);
            }
            else if (strchr("+-*/", input[i]))
            {
                iu = 1;
                state = 0;
            }
            else
            {
                err(i, 5);
                if (input[i] == '0')
                {
                    int in = i;
                    if (input[i + 1] && input[i + 1] == '.')
                    {
                        if (input[i + 2] && strchr("0123456789", input[i + 2]))
                        {
                            i += 2;
                            while (input[i] && strchr("0123456789", input[i]))
                                i++;
                            i--;
                        }
                        else
                        {
                            err(in, 4);
                        }
                    }
                }
                else
                {
                    while (input[i] && strchr("0123456789", input[i]))
                        i++;
                    int t = 0;
                    while (input[i++] == '.')
                    {
                        if (t == 1)
                        {
                            err(i - 1, 4);
                        }
                        else
                        {
                            t = 1;
                        }
                        while (input[i] && strchr("0123456789", input[i]))
                            i++;
                    }
                }
                state = 1;
            }
            break;
        }
    }
    for (int i = 0; i < ti.n; i++)
    {
        err(ti.a[i], 7);
    }
    return eri.n;
}
void F()
{
    char s[128];
    int n[64], nn = 0, dd = 0;
    double d[64];
    tc.n = 0;
    for (int i = 0; input[i]; i++)
    {
        if (input[i] == '(')
            tc.push(&tc, input[i]);
        else if (input[i] == ')')
        {
            while (tc.fr(&tc, 0) != '(')
            {
                n[nn++] = tc.pop(&tc);
                dd++;
            }
            tc.pop(&tc);
        }
        else if (strchr("*/", input[i]))
        {
            while (tc.n > 0 && strchr("*/", tc.fr(&tc, 0)))
            {
                n[nn++] = tc.pop(&tc);
                dd++;
            }
            tc.push(&tc, input[i]);
        }
        else if (strchr("+-", input[i]))
        {
            while (tc.n > 0 && tc.fr(&tc, 0) != '(')
            {
                n[nn++] = tc.pop(&tc);
                dd++;
            }
            tc.push(&tc, input[i]);
        }
        else
        {
            int in = 0;
            while (input[i] && strchr("0123456789.", input[i]))
                s[in++] = input[i++];
            i--;
            s[in] = 0;
            d[dd++] = atof(s);
            n[nn++] = 0;
        }
    }
    while (tc.n > 0)
    {
        n[nn++] = tc.pop(&tc);
        dd++;
    }
    // setPos(0, 3);
    // for (int i = 0; i < nn; i++)
    // {
    //     if (n[i] == 0)
    //         printf("%lf ", d[i]);
    //     else
    //         printf("%c ", n[i]);
    // }
    td.n = 0;
    int A=0;
    for (int i = 0; i < nn; i++)
    {
        if (n[i] == 0)
            td.push(&td, d[i]);
        else
        {
            double b = td.pop(&td), a = td.pop(&td);
            switch (n[i])
            {
            case '+':
                td.push(&td, a + b);
                break;
            case '-':
                td.push(&td, a - b);
                break;
            case '*':
                td.push(&td, a * b);
                break;
            case '/':
                if(b!=0)
                td.push(&td, a / b);
                else{
                    A=1;
                td.push(&td,0);
                }
                break;
            }
        }
    }
    if(A)
    printL(0, 2, "\033[31m=除零错误\033[0m");
    else
    printL(0, 2, "=%g", td.pop(&td));
}

unsigned int __stdcall draw(void *p)
{
    static int A=0;
    system("cls");
    printL(0, 0, "简易计算器(仅支持+-*/，回车输出结果，q退出)\n");
    printL(0, 1, "示例：9+(3-1)*3+10/2");
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, enter};
        DWORD wait_result = WaitForMultipleObjects(6, hEvents, FALSE, INFINITE);
        printL(0, 9, "%d", ti.n);
        if (wait_result == WAIT_OBJECT_0 + 1)
        {
            break;
        }
        else if (wait_result == WAIT_OBJECT_0 + 2)
        {
            WaitForSingleObject(_input, INFINITE);
            WaitForSingleObject(_posX, INFINITE);
            int l = strlen(input);
            for (int i = posX; i < l; i++)
            {
                printL(i, 1, "%c", input[i]);
            }
            printL(l, 1, " ");
            check();
            errlog();

            setPos(posX, 1);
            ReleaseMutex(_input);
            ReleaseMutex(_posX);
        }
        else if (wait_result == WAIT_OBJECT_0)
        {
            if(A==0)
            printL(0,1,"                     ");
            A=1;
            WaitForSingleObject(_input, INFINITE);
            WaitForSingleObject(_posX, INFINITE);
            int l = strlen(input);
            for (int i = posX - 1; i < l; i++)
            {
                printL(i, 1, "%c", input[i]);
            }
            check();
            errlog();
            setPos(posX, 1);
            ReleaseMutex(_input);
            ReleaseMutex(_posX);
        }
        else if (wait_result == WAIT_OBJECT_0 + 3)
        {
            WaitForSingleObject(_posX, INFINITE);
            setPos(posX, 1);
            ReleaseMutex(_posX);
        }
        else if (wait_result == WAIT_OBJECT_0 + 4)
        {
            WaitForSingleObject(_posX, INFINITE);
            setPos(posX, 1);
            ReleaseMutex(_posX);
        }
        else if (wait_result == WAIT_OBJECT_0 + 5)
        {
            if (check() == 0)
            {
                F();
                printL(0, 3, "           ");
            }
            else
                printL(0, 3, "\033[31m表达式有误\033[0m");
            printL(strlen(input), 1, "");
        }
    }
}
unsigned int __stdcall getcin(void *p)
{
    int c;
    while (1)
    {
        c = _getch();
        if (c == 'q'||c==27)
        {
            SetEvent(stop);
            break;
        }
        else if (c == 8)
        {
            WaitForSingleObject(_input, INFINITE);
            WaitForSingleObject(_posX, INFINITE);
            if (posX > 0)
            {
                int l = strlen(input);
                for (int i = posX - 1; i < l; i++)
                {
                    input[i] = input[i + 1];
                }
                input[l - 1] = '\0';
                posX--;
                SetEvent(del);
            }
            ReleaseMutex(_input);
            ReleaseMutex(_posX);
        }
        else if (c == 0 || c == 0xE0)
        {
            int c = _getch();
            if (c == 75)
            {
                WaitForSingleObject(_posX, INFINITE);
                if (posX > 0)
                    posX--;
                ReleaseMutex(_posX);
                SetEvent(movl);
            }
            else if (c == 77)
            {
                WaitForSingleObject(_posX, INFINITE);
                WaitForSingleObject(_input, INFINITE);
                if (posX < strlen(input))
                    posX++;
                ReleaseMutex(_input);
                ReleaseMutex(_posX);
                SetEvent(movr);
            }
            continue;
        }
        else if (c == 13) // 回车键
        {
            SetEvent(enter);
        }
        else if ('0' <= c && c <= '9' || c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '.')
        {
            WaitForSingleObject(_input, INFINITE);
            WaitForSingleObject(_posX, INFINITE);
            int l = strlen(input);
            if (l < sizeof(input) - 1)
            {
                for (int i = l; i >= posX; i--)
                {
                    input[i + 1] = input[i];
                }
                input[posX] = c;
                posX++;
                SetEvent(cin);
            }
            ReleaseMutex(_input);
            ReleaseMutex(_posX);
        }
    }
}
int main()
{
    NewTrace_char(&tc);
    NewTrace_int(&ti);
    NewTrace_int(&eri);
    NewTrace_int(&erii);
    NewTrace_double(&td);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    cin = CreateEvent(NULL, FALSE, FALSE, NULL);
    stop = CreateEvent(NULL, FALSE, FALSE, NULL);
    del = CreateEvent(NULL, FALSE, FALSE, NULL);
    movl = CreateEvent(NULL, FALSE, FALSE, NULL);
    movr = CreateEvent(NULL, FALSE, FALSE, NULL);
    enter = CreateEvent(NULL, FALSE, FALSE, NULL);
    _posX = CreateMutex(NULL, FALSE, NULL);
    _input = CreateMutex(NULL, FALSE, NULL);
    HANDLE hThreads[2];
    hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, draw, NULL, 0, NULL);
    hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, getcin, NULL, 0, NULL);

    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    printf("程序已退出。\n");
    for (int i = 0; i < 2; i++)
    {
        CloseHandle(hThreads[i]);
    }
    CloseHandle(cin);
    CloseHandle(stop);
    CloseHandle(del);
    CloseHandle(movl);
    CloseHandle(movr);
    CloseHandle(enter);
    CloseHandle(_posX);
    CloseHandle(_input);
    return 0;
}
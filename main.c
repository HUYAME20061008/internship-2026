#include "head.h"
#include "T1.c"
#include "T2.c"
#include "T3.c"
// 全局Windows内核同步对象
HANDLE cin, stop, del, movl, movr, movu, movd, enter, sw, sh, esc, back;
HANDLE ob; // 全局互斥锁，多线程访问界面/数据时加锁防止冲突
// ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼ ─ │

int W, H, X, Y; // X：分页按钮左右选中；Y：菜单/列表纵向选中索引
int state;
/**
 * @brief 单个联系人结构体
 * name：姓名 最大19字符+结束符
 * number：手机号码 最大19字符+结束符
 */
struct A a;
/**
 * @brief 联系人列表容器
 * N[200]：联系人数组，上限200条
 * n：当前有效联系人数量
 */

struct AB list, li; // list原始总数据；li查询临时筛选数据

/**
 * @brief 设置控制台光标坐标
 * @param x 横坐标
 * @param y 纵坐标
 */
void setPos(int x, int y)
{
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/**
 * @brief 显示/隐藏控制台光标
 * @param isHide 0显示 1隐藏
 */
void setHide(int isHide)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = isHide;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

/**
 * @brief 格式化打印函数（支持可变参数，定位输出）
 * @param x 坐标x
 * @param y 坐标y
 * @param s 格式化字符串 const防止字面量类型冲突
 */
void printL(int x, int y, const char *s, ...)
{
    setPos(x, y);
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
}

/**
 * @brief qsort排序回调函数，按姓名字典序比较
 */
int cmp(const void *p1, const void *p2)
{
    const struct A *a1 = (const struct A *)p1;
    const struct A *a2 = (const struct A *)p2;
    wchar_t w1[32] = {0};
    wchar_t w2[32] = {0};
    MultiByteToWideChar(CP_UTF8, 0, a1->name, -1, w1, 32);
    MultiByteToWideChar(CP_UTF8, 0, a2->name, -1, w2, 32);
    return wcscoll(w2, w1);
}

/**
 * @brief 计算UTF-8字符串视觉宽度（区分单字节/多字节汉字）
 * @param s 输入字符串
 * @return 屏幕占用字符宽度
 */
int lenStr(char *s)
{
    int n = 0;
    for (char *p = s; *p;)
    {
        if (((*p) & 0x80) == 0)
            p += 1, n += 1;
        else if (((*p) & 0xe0) == 0xc0)
            p += 2, n += 1;
        else if (((*p) & 0xf0) == 0xe0)
            p += 3, n += 2;
        else if (((*p) & 0xf8) == 0xf0)
            p += 4, n += 2;
        else
            p++;
    }
    return n;
}

/**
 * @brief 简易定位日志打印，同printL功能
 */
void flog(int x, int y, const char *s, ...)
{
    setPos(x, y);
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
}

/**
 * @brief 弹出红色提示弹窗
 * @param s 提示文字
 */
void wlog(const char *s)
{
    setHide(0);
    printf("\033[31m");
    printL(18 - lenStr(s) / 2 - 1, 5, "┌");
    for (int i = 0; i < lenStr(s) + 2; i++)
        printf("─");
    printf("┐");
    printL(17 - lenStr(s) / 2, 6, "│ %s │", s);
    printL(18 - lenStr(s) / 2 - 1, 7, "└");
    for (int i = 0; i < lenStr(s) + 2; i++)
        printf("─");
    printf("┘");
    printf("\033[0m");
    //  printL(20-lenStr(s)/2, 5, "───────────────────────\033[0m");
}

/**
 * @brief 添加联系人至总list数组
 * @param a 待添加联系人
 * @return 0成功 1已满
 */
int add(struct A a)
{
    if (list.n < 200)
    {
        list.N[list.n++] = a;
        return 0;
    }
    else
        return 1;
}

/**
 * @brief 添加联系人至临时查询li数组
 * @param a 待添加联系人
 * @return 0成功 1容量满
 */
int add1(struct A a)
{
    if (li.n < 200)
    {
        li.N[li.n++] = a;
        return 0;
    }
    else
        return 1;
}

/**
 * @brief 模糊查询过滤函数
 * @param s 检索关键词
 * 逻辑：匹配包含关键词的联系人，按关键词首次出现位置升序存入li临时列表
 */
void For(char *s)
{
    if (*s == 0)
    {
        li = list; // 空关键词返回全部数据
        return;
    }
    int B[200]; // 标记是否已加入结果
    for (int i = 0; i < 200; i++)
        B[i] = 0;
    li.n = 0;
    char *s1;
    while (1)
    {
        int min = 10000, l;
        // 找最靠前匹配位置
        for (int j = 0; j < list.n; j++)
            if (B[j] == 0)
            {
                s1 = strstr(list.N[j].name, s);
                if (s1 == 0)
                    continue;
                l = s1 - (list.N[j].name);
                if (min > l)
                    min = l;
            }
        if (min == 10000) // 无匹配结束
            break;
        // 同位置全部加入临时列表
        for (int j = 0; j < list.n; j++)
        {
            if (B[j] == 0 && (strstr(list.N[j].name, s) - (list.N[j].name) == min))
            {
                B[j] = 1;
                add1(list.N[j]);
            }
        }
    }
}

/**
 * @brief 查询界面渲染
 */
void V4()
{
    // li = list;
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldY, num, num1;
    oldY = 0;
    setHide(1);
    X = 1;
    Y = -1; // Y=-1代表顶部搜索框激活
go1:
    For(a.name); // 实时执行检索
    num1 = 1;
    num = li.n / 5 + (li.n % 5 ? 1 : 0);
go:
    printL(0, 1, "│           🔍 查询联系人              │");
    for (int i = 3; i < 14; i++)
        printL(0, i, "│                                      │");
    for (int i = 5; i < 12; i++)
        printL(14, i, "│");
    if (Y == -1)
        printL(2, 3, "\033[44m                                   ");
    else
        printL(2, 3, "                                   ");
    printL(5, 3, " 查询：%s", a.name);
    printf("\033[0m");
    printL(0, 4, "├─────────────┬────────────────────────┤");
    printL(3, 5, "姓名");
    printL(14, 5, "┬");
    printL(14, 5, "│  电话");
    printL(0, 6, "├─────────────┼────────────────────────┤");
    printL(0, 12, "├─────────────┴────────────────────────┤");
    printL(17, 13, "%d/%d", num1, num);
    printL(3, 13, "< 上一页");
    printL(28, 13, "下一页 >");
    if (Y == 5)
    {
        if (X == 0)
            printL(4, 13, "\033[44m 上一页 \033[0m");
        else
            printL(27, 13, "\033[44m 下一页 \033[0m");
    }
    // 渲染查询结果列表（每页5条）
    for (int i = num1 * 5 - 5, j = 0; i < li.n && j < 5; i++, j++)
    {
        if (i % 5 == Y)
            printL(2, 7 + j, "\033[44m            │                       ");
        printL(3, 7 + j, "%s", li.N[i].name);
        printL(17, 7 + j, "%s", li.N[i].number);
        if (i % 5 == Y)
            printf("\033[0m");
    }
    if (Y == -1)
    {
        setHide(1);
        setPos(12 + lenStr(a.name), 3);
    }
    else
        setHide(0);
    ReleaseMutex(ob);
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);
        if (Y == -1)
        {
            setHide(1);
        }
        else
            setHide(0);
        switch (wait)
        {
        case 9:
            printL(2, 3, "\033[44m                                   ");
        case 0:
            printL(5, 3, "\033[44m 查询：%s\033[0m", a.name);
            goto go1;
            break;

        case 3:
            if (Y == 5)
            {
                printL(4, 13, "\033[44m 上一页 \033[0m");
                printL(27, 13, " 下一页 ");
            }
            break;
        case 4:
            if (Y == 5)
            {
                printL(27, 13, "\033[44m 下一页 \033[0m");
                printL(4, 13, " 上一页 ");
            }
            break;
        case 5:
        case 6:
            if (Y == -1)
            {
                printL(2, 3, "\033[44m                                   ");
            }
            else
            {
                printL(2, 3, "                                   ");
            }
            printL(5, 3, " 查询：%s", a.name);
            printf("\033[0m");
            if (oldY != -1 && (wait != 5 || oldY != 5))
            {
                if (oldY != 5)
                {
                    printL(2, 7 + oldY, "            │                       ");
                    printL(3, 7 + oldY, "%s", li.N[oldY + num1 * 5 - 5].name);
                    printL(17, 7 + oldY, "%s", li.N[oldY + num1 * 5 - 5].number);
                }
            }
            else
            {
                printL(4, 13, " 上一页 ");
                printL(27, 13, " 下一页 ");
            }
            if (Y < 5 && (num1 != num || Y < li.n % 5 || li.n % 5 == 0) && Y != -1)
            {

                printL(2, 7 + Y, "\033[44m            │                       ");
                printL(3, 7 + Y, "%s", li.N[Y + num1 * 5 - 5].name);
                printL(17, 7 + Y, "%s", li.N[Y + num1 * 5 - 5].number);
                printf("\033[0m");
            }
            else if (Y == 5)
            {

                if (X == 0)
                    printL(4, 13, "\033[44m 上一页 \033[0m");
                else
                    printL(27, 13, "\033[44m 下一页 \033[0m");
            }
            if (Y == -1)
                setPos(12 + lenStr(a.name), 3);
            break;

        case 7:
            if (Y == 5)
            {
                if (X == 1)
                {
                    if (num1 < num)
                    {
                        num1++;
                        goto go;
                    }
                }
                else
                {
                    if (num1 > 1)
                    {
                        num1--;
                        goto go;
                    }
                }
            }
            break;
        case 8:
            return;
        }
        oldY = Y;
        ReleaseMutex(ob);
    }
}

/**
 * @brief 查询按键处理
 */
void O4()
{
    static int oldX, num, num1;
    oldX = 0;
    num1 = 1;
    num = li.n / 5 + (li.n % 5 ? 1 : 0);
    int c;
    while (1)
    {
        c = _getch();
        WaitForSingleObject(ob, INFINITE);
        For(a.name); // 输入实时刷新检索结果
        num1 = 1;
        num = li.n / 5 + (li.n % 5 ? 1 : 0);
        if (c == ESC)
        {
            SetEvent(esc);
            ReleaseMutex(ob);
            break;
        }
        else if (c == 0xE0)
        {
            int c = _getch();
            if (c == UP)
            {
                if (Y == 5)
                {
                    if (num1 == num && li.n % 5 != 0)
                        Y = li.n % 5 - 1;
                    else
                        Y = 4;
                }
                else
                {
                    if (Y > -1)
                        Y--;
                }
                SetEvent(movu);
            }
            else if (c == DOWN)
            {
                if (Y < 5 && (num1 != num || Y < li.n % 5 - 1 || li.n % 5 == 0))
                    Y++;
                else
                    Y = 5;
                SetEvent(movd);
            }
            else if (c == LEFT)
            {
                if (X == 1)
                    X = 0;
                SetEvent(movl);
            }
            else if (c == RIGHT)
            {
                if (X == 0)
                    X = 1;
                SetEvent(movr);
            }
        }
        else if (c == ENTER)
        {
            if (Y == 5)
            {
                if (X == 1)
                {
                    if (num1 < num)
                        num1++;
                }
                else
                {
                    if (num1 > 1)
                        num1--;
                }
            }
            SetEvent(enter);
        }
        else if (c == BACK) // 搜索框退格删除关键词
        {
            if (Y == -1)
            {
                int l = strlen(a.name);
                if (l > 0)
                {
                    while (l > 0 && (a.name[l - 1] & 0xc0) == 0x80)
                    {
                        l--;
                    }
                    a.name[l - 1] = 0;
                    SetEvent(back);
                }
            }
        }
        else
        {
            if (Y == -1) // 输入检索关键词
            {
                int l = strlen(a.name);
                if (l > 15)
                {
                    ReleaseMutex(ob);
                    continue;
                }
                if (((c) & 0x80) == 0)
                    a.name[l] = c, a.name[l + 1] = 0;
                else if (((c) & 0xe0) == 0xc0)
                {
                    a.name[l] = c;
                    a.name[l + 1] = _getch();
                    a.name[l + 2] = 0;
                }
                else if (((c) & 0xf0) == 0xe0)
                {
                    a.name[l] = c;
                    a.name[l + 1] = _getch();
                    a.name[l + 2] = _getch();
                    a.name[l + 3] = 0;
                }
                else if (((c) & 0xf8) == 0xf0)
                {
                    a.name[l] = c;
                    a.name[l + 1] = _getch();
                    a.name[l + 2] = _getch();
                    a.name[l + 3] = _getch();
                    a.name[l + 4] = 0;
                }
                else
                {
                    a.name[l] = c, a.name[l + 1] = 0;
                }
                SetEvent(cin);
            }
        }
        ReleaseMutex(ob);
    }
}

/**
 * @brief 显示全部联系人界面渲染
 */
void V5()
{
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldY, num, num1;
    oldY = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0);
    setHide(0);
    X = 1;
    Y = 0;
go:
    printL(0, 1, "│            📋 显示所有               │");
    for (int i = 3; i < 14; i++)
        printL(0, i, "│                                      │");
    for (int i = 5; i < 12; i++)
        printL(14, i, "│");
    printL(3, 3, "姓名");
    printL(14, 2, "┬");
    printL(14, 3, "│  电话");
    printL(0, 4, "├─────────────┼────────────────────────┤");
    printL(0, 12, "├─────────────┴────────────────────────┤");
    printL(17, 13, "%d/%d", num1, num);
    printL(3, 13, "< 上一页");
    printL(28, 13, "下一页 >");
    if (Y == 7)
    {
        if (X == 0)
            printL(4, 13, "\033[44m 上一页 \033[0m");
        else
            printL(27, 13, "\033[44m 下一页 \033[0m");
    }
    for (int i = num1 * 7 - 7, j = 0; i < list.n && j < 7; i++, j++)
    {
        if (i % 7 == Y)
            printL(2, 5 + j, "\033[44m            │                       ");
        printL(3, 5 + j, "%s", list.N[i].name);
        printL(17, 5 + j, "%s", list.N[i].number);
        if (i % 7 == Y)
            printf("\033[0m");
    }
    ReleaseMutex(ob);
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);

        switch (wait)
        {
        case 0:
        case 9:

        case 3:
            if (Y == 7)
            {
                printL(4, 13, "\033[44m 上一页 \033[0m");
                printL(27, 13, " 下一页 ");
            }
            break;
        case 4:
            if (Y == 7)
            {
                printL(27, 13, "\033[44m 下一页 \033[0m");
                printL(4, 13, " 上一页 ");
            }
            break;
        case 5:
        case 6:
            if (wait != 5 || oldY != 7)
            {
                if (oldY != 7)
                {
                    printL(2, 5 + oldY, "            │                       ");
                    printL(3, 5 + oldY, "%s", list.N[oldY + num1 * 7 - 7].name);
                    printL(17, 5 + oldY, "%s", list.N[oldY + num1 * 7 - 7].number);
                }
            }
            else
            {
                printL(4, 13, " 上一页 ");
                printL(27, 13, " 下一页 ");
            }
            if (Y < 7 && (num1 != num || Y < list.n % 7 || list.n % 7 == 0))
            {

                printL(2, 5 + Y, "\033[44m            │                       ");
                printL(3, 5 + Y, "%s", list.N[Y + num1 * 7 - 7].name);
                printL(17, 5 + Y, "%s", list.N[Y + num1 * 7 - 7].number);
                printf("\033[0m");
            }
            else if (Y == 7)
            {

                if (X == 0)
                    printL(4, 13, "\033[44m 上一页 \033[0m");
                else
                    printL(27, 13, "\033[44m 下一页 \033[0m");
            }
            break;

        case 7:
            if (Y == 7)
            {
                if (X == 1)
                {
                    if (num1 < num)
                    {
                        num1++;
                        goto go;
                    }
                }
                else
                {
                    if (num1 > 1)
                    {
                        num1--;
                        goto go;
                    }
                }
            }
            break;
        case 8:
            return;
        }
        oldY = Y;
        ReleaseMutex(ob);
    }
}

/**
 * @brief 显示全部按键处理
 */
void O5()
{
    static int oldX, num, num1;
    oldX = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0);
    int c;
    while (1)
    {
        c = _getch();
        WaitForSingleObject(ob, INFINITE);
        if (c == ESC)
        {
            SetEvent(esc);
            ReleaseMutex(ob);
            break;
        }
        else if (c == 0xE0)
        {
            int c = _getch();
            if (c == UP)
            {
                if (Y == 7)
                {
                    if (num1 == num && list.n % 7 != 0)
                        Y = list.n % 7 - 1;
                    else
                        Y = 6;
                }
                else
                {
                    if (Y > 0)
                        Y--;
                }
                SetEvent(movu);
            }
            else if (c == DOWN)
            {
                if (Y < 7 && (num1 != num || Y < list.n % 7 - 1 || list.n % 7 == 0))
                    Y++;
                else
                    Y = 7;
                SetEvent(movd);
            }
            else if (c == LEFT)
            {
                if (X == 1)
                    X = 0;
                SetEvent(movl);
            }
            else if (c == RIGHT)
            {
                if (X == 0)
                    X = 1;
                SetEvent(movr);
            }
        }
        else if (c == ENTER)
        {
            if (Y == 7)
            {
                if (X == 1)
                {
                    if (num1 < num)
                        num1++;
                }
                else
                {
                    if (num1 > 1)
                        num1--;
                }
            }
            SetEvent(enter);
        }
        else if (c == BACK)
        {
            if (Y == 0)
            {
                int l = strlen(a.name);
                if (l > 0)
                {
                    while (l > 0 && (a.name[l - 1] & 0xc0) == 0x80)
                    {
                        l--;
                    }
                    a.name[l - 1] = 0;
                    SetEvent(back);
                }
            }
            else if (Y == 1)
            {
                int l = strlen(a.number);
                if (l > 0)
                {
                    while (l > 0 && (a.number[l - 1] & 0xc0) == 0x80)
                    {
                        l--;
                    }
                    a.number[l - 1] = 0;
                    SetEvent(back);
                }
            }
        }
        else
        {
        }
        ReleaseMutex(ob);
    }
}

/**
 * @brief 排序界面渲染：程序进入自动qsort排序
 */
void V6()
{
    qsort(list.N, list.n, sizeof(struct A), cmp); // 全局数组姓名升序
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldY, num, num1;
    oldY = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0);
    setHide(0);
    X = 1;
    Y = 0;
go:
    printL(0, 1, "│           🔤 按姓名排序              │");
    for (int i = 3; i < 14; i++)
        printL(0, i, "│                                      │");
    for (int i = 5; i < 12; i++)
        printL(14, i, "│");
    printL(3, 3, "姓名");
    printL(14, 2, "┬");
    printL(14, 3, "│  电话");
    printL(0, 4, "├─────────────┼────────────────────────┤");
    printL(0, 12, "├─────────────┴────────────────────────┤");
    printL(17, 13, "%d/%d", num1, num);
    printL(3, 13, "< 上一页");
    printL(28, 13, "下一页 >");
    if (Y == 7)
    {
        if (X == 0)
            printL(4, 13, "\033[44m 上一页 \033[0m");
        else
            printL(27, 13, "\033[44m 下一页 \033[0m");
    }
    for (int i = num1 * 7 - 7, j = 0; i < list.n && j < 7; i++, j++)
    {
        if (i % 7 == Y)
            printL(2, 5 + j, "\033[44m            │                       ");
        printL(3, 5 + j, "%s", list.N[i].name);
        printL(17, 5 + j, "%s", list.N[i].number);
        if (i % 7 == Y)
            printf("\033[0m");
    }
    ReleaseMutex(ob);
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);

        switch (wait)
        {
        case 0:
        case 9:

        case 3:
            if (Y == 7)
            {
                printL(4, 13, "\033[44m 上一页 \033[0m");
                printL(27, 13, " 下一页 ");
            }
            break;
        case 4:
            if (Y == 7)
            {
                printL(27, 13, "\033[44m 下一页 \033[0m");
                printL(4, 13, " 上一页 ");
            }
            break;
        case 5:
        case 6:
            if (wait != 5 || oldY != 7)
            {
                if (oldY != 7)
                {
                    printL(2, 5 + oldY, "            │                       ");
                    printL(3, 5 + oldY, "%s", list.N[oldY + num1 * 7 - 7].name);
                    printL(17, 5 + oldY, "%s", list.N[oldY + num1 * 7 - 7].number);
                }
            }
            else
            {
                printL(4, 13, " 上一页 ");
                printL(27, 13, " 下一页 ");
            }
            if (Y < 7 && (num1 != num || Y < list.n % 7 || list.n % 7 == 0))
            {

                printL(2, 5 + Y, "\033[44m            │                       ");
                printL(3, 5 + Y, "%s", list.N[Y + num1 * 7 - 7].name);
                printL(17, 5 + Y, "%s", list.N[Y + num1 * 7 - 7].number);
                printf("\033[0m");
            }
            else if (Y == 7)
            {

                if (X == 0)
                    printL(4, 13, "\033[44m 上一页 \033[0m");
                else
                    printL(27, 13, "\033[44m 下一页 \033[0m");
            }
            break;

        case 7:
            if (Y == 7)
            {
                if (X == 1)
                {
                    if (num1 < num)
                    {
                        num1++;
                        goto go;
                    }
                }
                else
                {
                    if (num1 > 1)
                    {
                        num1--;
                        goto go;
                    }
                }
            }
            break;
        case 8:
            return;
        }
        oldY = Y;
        ReleaseMutex(ob);
    }
}

/**
 * @brief 排序界面按键处理
 */
void O6()
{
    static int oldX, num, num1;
    oldX = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0);
    int c;
    while (1)
    {
        c = _getch();
        WaitForSingleObject(ob, INFINITE);
        if (c == ESC)
        {
            SetEvent(esc);
            ReleaseMutex(ob);
            break;
        }
        else if (c == 0xE0)
        {
            int c = _getch();
            if (c == UP)
            {
                if (Y == 7)
                {
                    if (num1 == num && list.n % 7 != 0)
                        Y = list.n % 7 - 1;
                    else
                        Y = 6;
                }
                else
                {
                    if (Y > 0)
                        Y--;
                }
                SetEvent(movu);
            }
            else if (c == DOWN)
            {
                if (Y < 7 && (num1 != num || Y < list.n % 7 - 1 || list.n % 7 == 0))
                    Y++;
                else
                    Y = 7;
                SetEvent(movd);
            }
            else if (c == LEFT)
            {
                if (X == 1)
                    X = 0;
                SetEvent(movl);
            }
            else if (c == RIGHT)
            {
                if (X == 0)
                    X = 1;
                SetEvent(movr);
            }
        }
        else if (c == ENTER)
        {
            if (Y == 7)
            {
                if (X == 1)
                {
                    if (num1 < num)
                        num1++;
                }
                else
                {
                    if (num1 > 1)
                        num1--;
                }
            }
            SetEvent(enter);
        }
        else if (c == BACK)
        {
            if (Y == 0)
            {
                int l = strlen(a.name);
                if (l > 0)
                {
                    while (l > 0 && (a.name[l - 1] & 0xc0) == 0x80)
                    {
                        l--;
                    }
                    a.name[l - 1] = 0;
                    SetEvent(back);
                }
            }
            else if (Y == 1)
            {
                int l = strlen(a.number);
                if (l > 0)
                {
                    while (l > 0 && (a.number[l - 1] & 0xc0) == 0x80)
                    {
                        l--;
                    }
                    a.number[l - 1] = 0;
                    SetEvent(back);
                }
            }
        }
        else
        {
        }
        ReleaseMutex(ob);
    }
}

/**
 * @brief 持久化保存联系人至本地文件1123
 * 写入规则：首行为总记录数n，之后成对写入姓名、手机号，每行一条数据
 * 打开文件失败直接返回，不弹窗报错
 */
void save()
{
    FILE *F = fopen("1123", "w");
    if (!F)
        return;
    fprintf(F, "%d\n", list.n);
    for (int i = 0; i < list.n; i++)
    {
        fprintf(F, "%s\n%s\n", list.N[i].name, list.N[i].number);
    }
    fclose(F); // 关闭文件释放资源
}

/**
 * @brief 程序启动加载本地联系人文件1123
 * 文件不存在/损坏/数据越界直接置空列表；读取后截断每行换行符
 */
void load()
{
    FILE *f = fopen("1123", "r");
    if (!f)
        return; // 文件不存在，无历史数据
    int n;
    // 读取第一条：联系人总数
    if (fscanf(f, "%d\n", &n) != 1)
    {
        fclose(f);
        return;
    }
    // 数据合法性校验，防止数组越界
    if (n < 0 || n > 200)
    {
        fclose(f);
        return;
    }
    list.n = n;
    // 循环读取每条姓名、手机号
    for (int i = 0; i < n; i++)
    {
        if (!fgets(list.N[i].name, sizeof(list.N[i].name), f))
            break;
        if (!fgets(list.N[i].number, sizeof(list.N[i].number), f))
            break;
        // 去除fgets自带末尾换行符，避免打印多出空行
        list.N[i].name[strcspn(list.N[i].name, "\n")] = '\0';
        list.N[i].number[strcspn(list.N[i].number, "\n")] = '\0';
    }

    fclose(f);
}

/**
 * @brief 界面渲染线程函数
 * 功能：绘制系统主菜单、监听界面刷新事件、跳转各功能子界面
 * 线程类型：_beginthreadex 创建后台线程，持续刷新UI
 */
unsigned int __stdcall draw(void *p)
{
    static int oldY;                   // 缓存上一次纵向选中行，用于清除高亮
    WaitForSingleObject(ob, INFINITE); // 抢占互斥锁，独占控制台输出
    setHide(0);
    system("cls"); // 清屏刷新主界面
go:
    // 绘制主窗口外边框
    setHide(0);
    printL(0, 0, "┌──────────────────────────────────────┐");
    printL(0, 1, "│      📱 校园通讯录管理系统 v1.0      │");
    printL(0, 2, "├──────────────────────────────────────┤");
    for (int i = 3; i < 14; i++)
    {
        printL(0, i, "│                                      │");
    }
    printL(0, 14, "└──────────────────────────────────────┘");
    // 菜单文本数组，对应7项功能
    const char *items[] = {
        "➕ 添加联系人",
        "➖ 删除联系人",
        "✏️  修改联系人",
        "🔍 查询联系人",
        "📋 显示所有",
        "🔤 按姓名排序",
        "💾 保存并退出"};
    // 循环绘制菜单，当前选项蓝色高亮
    for (int i = 0; i < 7; i++)
    {
        if (i == Y)
        {
            printL(10, 5 + i, " \033[44m\033[37m> %s\033[0m", items[i]);
        }
        else
        {
            printL(10, 5 + i, "   %s", items[i]);
        }
    }
    // wlog("111");
    //  printL(0, 11, "↑↓ 选择 | Enter 执行 | ESC 退出");
    // printL(0, 12, "123456789-123456789-123456789-123456789-123456789-");
    ReleaseMutex(ob); // 释放锁，允许输入线程运行
    // 事件循环，等待界面刷新信号
    while (1)
    {
        void (*V[])() = {V1, V2, V3, V4, V5, V6}; // 子界面渲染函数数组
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);
        switch (wait)
        {
        case 1: // stop事件触发，程序退出线程
            ReleaseMutex(ob);
            return 0;
        case 5: // 上移事件，刷新菜单高亮
        case 6: // 下移事件，刷新菜单高亮
            printL(10, 5 + Y, " \033[44m\033[37m> %s\033[0m", items[Y]);
            printL(10, 5 + oldY, "   %s", items[oldY]);
            break;
        case 7: // 回车事件，进入对应功能界面
            V[Y]();
            Y = oldY;
            goto go; // 返回主菜单重绘
            break;
        }
        oldY = Y;
        ReleaseMutex(ob);
    }
}

/**
 * @brief 全局键盘输入监听线程
 * 实时捕获上下箭头、回车、ESC按键，切换菜单或进入功能
 */
unsigned int __stdcall getcin(void *p)
{
    int c;
    // 功能按键处理函数映射数组
    void (*O[])() = {O1, O2, O3, O4, O5, O6};
    while (1)
    {
        c = _getch(); // 无阻塞读取控制台按键
        WaitForSingleObject(ob, INFINITE);
        if (c == ESC) // ESC按下，触发全局退出
        {
            SetEvent(stop);
            ReleaseMutex(ob);
            break;
        }
        else if (c == 0xE0) // 扩展功能键前缀（方向键、Delete）
        {
            int c = _getch();
            if (c == UP)
            {
                if (Y == 0)
                    Y = 6;
                else
                    Y--;
                SetEvent(movu); // 发送上移界面事件
            }
            else if (c == DOWN)
            {
                if (Y == 6)
                    Y = 0;
                else
                    Y++;
                SetEvent(movd); // 发送下移界面事件
            }
        }
        else if (c == ENTER) // 回车键确认
        {
            if (Y != 6) // 前6项为功能，第6项是退出
            {
                SetEvent(enter);
                ReleaseMutex(ob);
                O[Y](); // 进入对应功能输入处理逻辑
            }
            else // 选中保存并退出
            {
                SetEvent(stop);
                ReleaseMutex(ob);
                break;
            }
        }
        ReleaseMutex(ob);
    }
}
unsigned int __stdcall LW(void *p)
{
    while (1)
    {
        HANDLE hEvents[] = {stop};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        if (wait == 0)
        {
            ReleaseMutex(ob);
            return 0;
        }
        WaitForSingleObject(ob, INFINITE);
        int w = 0, h = 0;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
        else
        {
            w = 80;
        }
        if (W != w || H != h)
        {
            if (W != w)
                SetEvent(sw);
            if (H != h)
                SetEvent(sh);
            W = w;
            H = h;
        }
        ReleaseMutex(ob);
        Sleep(20);
    }
}

/**
 * @brief 程序入口主函数
 * 执行流程：加载本地数据 → 创建同步内核对象 → 启动3条工作线程 → 等待线程结束 → 保存数据、释放资源
 */
int main()
{
    load(); // 程序启动先读取1123联系人文件
    setlocale(LC_ALL, "zh_CN.UTF-8");
    struct A a = {"12", "12"};
    // for (int i = 0; i < 15; i++)
    // {
    //     sprintf(a.name, "%d", 30 - i);
    //     add(a);
    // }
    // 设置控制台全局UTF8编码，解决中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 创建全部Windows事件同步对象
    cin = CreateEvent(NULL, FALSE, FALSE, NULL);
    stop = CreateEvent(NULL, TRUE, FALSE, NULL);
    del = CreateEvent(NULL, FALSE, FALSE, NULL);
    movl = CreateEvent(NULL, FALSE, FALSE, NULL);
    movr = CreateEvent(NULL, FALSE, FALSE, NULL);
    movu = CreateEvent(NULL, FALSE, FALSE, NULL);
    movd = CreateEvent(NULL, FALSE, FALSE, NULL);
    enter = CreateEvent(NULL, FALSE, FALSE, NULL);
    esc = CreateEvent(NULL, FALSE, FALSE, NULL);
    back = CreateEvent(NULL, FALSE, FALSE, NULL);
    sw = CreateEvent(NULL, FALSE, FALSE, NULL);
    sh = CreateEvent(NULL, FALSE, FALSE, NULL);
    ob = CreateMutex(NULL, FALSE, NULL); // 创建全局互斥锁
    // 创建三条后台线程：界面渲染、键盘输入、窗口监听
    HANDLE hThreads[3];
    hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, draw, NULL, 0, NULL);
    hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, getcin, NULL, 0, NULL);
    hThreads[2] = (HANDLE)_beginthreadex(NULL, 0, LW, NULL, 0, NULL);
    // 阻塞等待所有线程执行完毕
    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);
    save(); // 程序退出前自动保存所有联系人
    system("cls");
    printf("程序已退出。\n");
    // 关闭线程句柄释放资源
    for (int i = 0; i < 3; i++)
    {
        CloseHandle(hThreads[i]);
    }
    // 关闭所有事件、互斥内核对象，避免内存泄漏
    CloseHandle(cin);
    CloseHandle(stop);
    CloseHandle(del);
    CloseHandle(movl);
    CloseHandle(movr);
    CloseHandle(enter);
    CloseHandle(ob);
    return 0;
}
#include "head.h"

/**
 * @brief 修改联系人界面渲染
 */
void V3()
{
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldY, num, num1;
    oldY = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0);
    setHide(1);
    X = 0;
    Y = 0;
go:
    if (list.n == 0)
    {
        printL(0, 1, "│            ✏️  修改联系人             │");
        for (int i = 3; i < 14; i++)
            printL(0, i, "│                                      │");
        printL(4, 4, "暂无联系人");
        setHide(0);
        goto go1;
    }
    a = list.N[Y + num1 * 7 - 7]; // 加载选中联系人至临时缓存
    printL(0, 1, "│            ✏️  修改联系人             │");
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
    else
    {
        printL(2, 5 + Y, "\033[44m            │                       ");
        printL(3, 5 + Y, "%s", a.name);
        printL(17, 5 + Y, "%s", a.number);
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
    printf("\033[0m");
    if (Y != 7)
    {
        if (X == 0)
        {
            setPos(3 + lenStr(a.name), 5 + Y);
        }
        else
        {
            setPos(17 + lenStr(a.number), 5 + Y);
        }
    }
go1:
    ReleaseMutex(ob);
    while (1)
    {
    go2:;
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);
        if (list.n == 0)
        {
            for (int i = 3; i < 14; i++)
                printL(0, i, "│                                      │");
            if (wait == 8)
                return;
            printL(4, 4, "暂无联系人");
            ReleaseMutex(ob);
            goto go2;
        }
        // 界面刷新逻辑省略注释，结构同V2
        switch (wait)
        {
        case 9:
            printL(2, 5 + Y, "\033[44m            │                       ");
        case 0:
            printL(3, 5 + Y, "\033[44m%s\033[0m", a.name);
            printL(17, 5 + Y, "\033[44m%s\033[0m", a.number);
            if (X == 0)
            {

                setPos(3 + lenStr(a.name), 5 + Y);
            }
            else
            {

                setPos(17 + lenStr(a.number), 5 + Y);
            }
            break;

            break;
        case 2:
            break;
        case 3:
            if (Y == 7)
            {
                printL(4, 13, "\033[44m 上一页 \033[0m");
                printL(27, 13, " 下一页 ");
            }
            else
            {
                if (X == 0)
                {
                    printL(3, 5 + Y, "\033[44m%s\033[0m", a.name);
                    setPos(3 + lenStr(a.name), 5 + Y);
                }
                else
                {
                    printL(17, 5 + Y, "\033[44m%s\033[0m", a.number);
                    setPos(17 + lenStr(a.number), 5 + Y);
                }
            }
            break;
        case 4:
            if (Y == 7)
            {
                printL(27, 13, "\033[44m 下一页 \033[0m");
                printL(4, 13, " 上一页 ");
            }
            else
            {
                if (X == 0)
                {
                    printL(3, 5 + Y, "\033[44m%s\033[0m", a.name);
                    setPos(3 + lenStr(a.name), 5 + Y);
                }
                else
                {
                    printL(17, 5 + Y, "\033[44m%s\033[0m", a.number);
                    setPos(17 + lenStr(a.number), 5 + Y);
                }
            }
            break;
        case 5:
        case 6:
            if (Y == 7)
                setHide(0);
            else
                setHide(1);
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
                if (X == 0)
                {
                    setPos(3 + lenStr(a.name), 5 + Y);
                }
                else
                {
                    setPos(17 + lenStr(a.number), 5 + Y);
                }
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
 * @brief 修改联系人按键处理
 */
void O3()
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
        if (list.n == 0)
        {
            if (c == ESC) // 退出自动保存修改回原数组
            {
                if (Y != 7)
                    list.N[Y + num1 * 7 - 7] = a;
                SetEvent(esc);
                ReleaseMutex(ob);
                break;
            }
            ReleaseMutex(ob);
            continue;
        }
        if (c == ESC) // 退出自动保存修改回原数组
        {
            if (Y != 7)
                list.N[Y + num1 * 7 - 7] = a;
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
                    a = list.N[Y + num1 * 7 - 7];
                }
                else
                {

                    list.N[Y + num1 * 7 - 7] = a;
                    if (Y > 0)
                        Y--;
                    a = list.N[Y + num1 * 7 - 7];
                }
                SetEvent(movu);
            }
            else if (c == DOWN)
            {
                list.N[Y + num1 * 7 - 7] = a;
                if (Y < 7 && (num1 != num || Y < list.n % 7 - 1 || list.n % 7 == 0))
                {

                    Y++;
                    a = list.N[Y + num1 * 7 - 7];
                }
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
        else if (c == BACK) // 退格修改姓名/电话
        {
            if (Y != 7)
            {
                if (X == 0)
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
                else if (X == 1)
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
        }
        else // 输入字符修改内容
        {
            if (Y != 7)
            {
                if (X == 0)
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
                }
                else if (X == 1)
                {
                    int l = strlen(a.number);
                    if (l > 15)
                    {
                        ReleaseMutex(ob);
                        continue;
                    }
                    if (((c) & 0x80) == 0)
                        a.number[l] = c, a.number[l + 1] = 0;
                    else if (((c) & 0xe0) == 0xc0)
                    {
                        a.number[l] = c;
                        a.number[l + 1] = _getch();
                        a.number[l + 2] = 0;
                    }
                    else if (((c) & 0xf0) == 0xe0)
                    {
                        a.number[l] = c;
                        a.number[l + 1] = _getch();
                        a.number[l + 2] = _getch();
                        a.number[l + 3] = 0;
                    }
                    else if (((c) & 0xf8) == 0xf0)
                    {
                        a.number[l] = c;
                        a.number[l + 1] = _getch();
                        a.number[l + 2] = _getch();
                        a.number[l + 3] = _getch();
                        a.number[l + 4] = 0;
                    }
                    else
                    {
                        a.number[l] = c, a.number[l + 1] = 0;
                    }
                }
                SetEvent(cin);
            }
        }
        ReleaseMutex(ob);
    }
}

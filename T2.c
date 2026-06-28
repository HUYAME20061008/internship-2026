#include"head.h"

/**
 * @brief 删除联系人界面渲染
 */
void V2()
{
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldY, num, num1;
    oldY = 0;
    num1 = 1;
    num = list.n / 7 + (list.n % 7 ? 1 : 0); // 计算总页数
    setHide(0);
    X = 1;
    Y = 0;
go:
    printL(0, 1, "│           ➖ 删除联系人              │");
    for (int i = 3; i < 14; i++)
        printL(0, i, "│                                      │");
    for (int i = 5; i < 12; i++)
        printL(14, i, "│");
    printL(3, 3, "姓名");
    printL(14, 2, "┬");
    printL(14, 3, "│  电话");
    printL(0, 4, "├─────────────┼────────────────────────┤");
    printL(0, 12, "├─────────────┴────────────────────────┤");
    printL(17, 13, "%d/%d", num1, num); // 页码显示
    printL(3, 13, "< 上一页");
    printL(28, 13, "下一页 >");
    if (Y == 7)
    {
        if (X == 0)
            printL(4, 13, "\033[44m 上一页 \033[0m");
        else
            printL(27, 13, "\033[44m 下一页 \033[0m");
    }
    // 渲染当前页联系人列表
    for (int i = num1 * 7 - 7, j = 0; i < list.n && j < 7; i++, j++)
    {
        if (i % 7 == Y)
            printL(2, 5 + j, "\033[44m            │                       ");
        printL(3, 5 + j, "%s", list.N[i].name);
        printL(17, 5 + j, "%s", list.N[i].number);
        if (i % 7 == Y)
            printf("\033[0m");
    }

    // 每行右侧红色删除提示
    for (int i = 0; i < 7 && (num1 != num || i < list.n % 7 || list.n % 7 == 0); i++)
    {
        if (i % 7 == Y)
            printf("\033[44m");
        printL(34, 5 + i, "\033[31m删除\033[0m");
    }
    ReleaseMutex(ob);
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);
        // 界面刷新事件处理
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
                    printL(34, 5 + oldY, "\033[31m删除\033[0m");
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
                printL(34, 5 + Y, "\033[31m删除\033[0m");
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

        case 7: // 翻页刷新
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
            else
            {
                num = list.n / 7 + (list.n % 7 ? 1 : 0);
                if (num < num1)
                {
                    num1--;
                }
                goto go;
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
 * @brief 删除联系人按键处理
 */
void O2()
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
        else if (c == ENTER) // 回车删除当前选中条目
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
            else
            {
                if (list.n > 0)
                {
                    // 数组前移覆盖删除元素
                    list.n--;
                    for (int i = Y + num1 * 7 - 7; i < list.n; i++)
                        list.N[i] = list.N[i + 1];
                    num = list.n / 7 + (list.n % 7 ? 1 : 0);
                    if (num < num1)
                    {
                        Y = 6;
                        num1--;
                    }
                    else if (list.n % 7 == Y && num == num1)
                        Y--;
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

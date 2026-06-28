/**
 * @file T1.c
 * @brief 【添加联系人】功能模块代码
 * @author 黄耀盟
 * @date 2026-06-27
 * @version 1.0
 * @description
 * 1. V1()：添加联系人界面渲染函数，绘制输入框、确认/取消按钮、高亮选中；
 * 2. O1()：添加界面键盘输入处理线程函数；
 * 3. 支持UTF8中文姓名/手机号输入、退格删除、上下左右切换输入框；
 * 4. 回车确认新增联系人、ESC返回主菜单，多线程事件同步刷新界面；
 * @module 任务三校园通讯录-新增联系人子功能
 */

#include"head.h"
/**
 * @brief 【界面线程】添加联系人UI渲染逻辑
 */
void V1()
{
    a.name[0] = 0;
    a.number[0] = 0;
    static int oldX;
    oldX = 0;
    setHide(1);
    X = 0;
    // 绘制窗口标题与边框
    printL(0, 1, "│           ➕ 添加联系人              │");
    for (int i = 3; i < 14; i++)
        printL(0, i, "│                                      │");
    printL(13, 4, "\033[44m                    ");
    printL(5, 4, "\033[37m> 姓名: %-20s\033[0m", a.name);
    printL(5, 6, "  电话: %-20s\033", a.number);
    printL(0, 12, "├──────────────────────────────────────┤");
    printL(28, 13, "取消");
    printL(6, 13, "确认");
    setPos(13, 4);
    ReleaseMutex(ob); // 释放互斥锁，允许输入线程操作
                      // 循环等待按键事件
    while (1)
    {
        HANDLE hEvents[] = {cin, stop, del, movl, movr, movu, movd, enter, esc, back};
        DWORD wait = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
        WaitForSingleObject(ob, INFINITE);

        switch (wait)
        {
        case 0:
        case 9: // 输入/退格刷新输入框
            if (Y == 0)
            {
                printL(13, 4, "\033[44m                    ");
                printL(13, 4, "\033[44m\033[37m%-20s\033[0m", a.name);
                setPos(13 + lenStr(a.name), 4);
            }
            else if (Y == 1)
            {
                printL(13, 6, "\033[44m                    ");
                printL(13, 6, "\033[44m\033[37m%-20s\033[0m", a.number);
                setPos(13 + lenStr(a.number), 6);
            }
            break;
        case 3: // 左方向键，选中确认按钮
            if (Y == 2)
            {
                printL(5, 13, "\033[44m 确认 \033[0m");
                printL(27, 13, " 取消 ");
            }
            break;
        case 4: // 右方向键，选中取消按钮
            if (Y == 2)
            {
                printL(5, 13, " 确认 ");
                printL(27, 13, "\033[44m 取消 \033[0m");
            }
            break;
        case 5:
            if (Y == 0) // 上箭头切换输入框（姓名）
            {
                printL(5, 4, "\033[44m                            ");
                printL(5, 4, "\033[44m\033[37m> 姓名: %-20s\033[0m", a.name);
                printL(5, 6, "\033[0m                            ");
                printL(5, 6, "  电话: %-20s\033[0m", a.number);
                setPos(13 + lenStr(a.name), 4);
            }
            else if (Y == 1) // 切换电话输入框
            {
                setHide(1);
                printL(27, 13, " 取消 ");
                printL(5, 13, " 确认 ");
                printL(5, 6, "\033[44m                            ");
                printL(5, 6, "\033[44m\033[37m> 电话: %-20s\033[0m", a.number);
                setPos(13 + lenStr(a.number), 6);
            }
            break;
        case 6: // 下箭头切换输入框
            if (Y == 1)
            {
                printL(13, 4, "\033[0m                    ");
                printL(5, 4, "  姓名: %-20s", a.name);
                printL(5, 6, "\033[44m                            ");
                printL(5, 6, "\033[44m\033[37m> 电话: %-20s\033[0m", a.number);
                setPos(13 + lenStr(a.number), 6);
            }
            else if (Y == 2) // 切换底部按钮区
            {
                setHide(0);
                printL(5, 6, "\033[0m                            ");
                printL(5, 6, "  电话: %-20s", a.number);
                if (X == 0)
                {
                    printL(5, 13, "\033[44m 确认 \033[0m");
                }
                else if (X == 1)
                {
                    printL(27, 13, "\033[44m 取消 \033[0m");
                }
            }
            break;
        case 7: // Enter确认
            break;
        case 8: // ESC返回主菜单
            return;
        }
        oldX = X;
        ReleaseMutex(ob);
    }
}

/**
 * @brief 【输入线程】添加联系人按键处理逻辑
 */
void O1()
{
    int c;
    while (1)
    {
        c = _getch();
        WaitForSingleObject(ob, INFINITE);
        if (c == ESC) // ESC退出添加界面
        {
            SetEvent(esc);
            ReleaseMutex(ob);
            break;
        }
        else if (c == 0xE0) // 扩展功能键前缀
        {
            int c = _getch();
            if (c == UP)
            {
                if (Y > 0)
                    Y--;
                SetEvent(movu);
            }
            else if (c == DOWN)
            {
                if (Y < 3)
                    Y++;
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
        else if (c == ENTER) // 回车执行确认
        {
            if (Y == 2)
            {
                if (X == 0)
                {
                    add(a); // 确认新增联系人
                }
                SetEvent(esc);
                ReleaseMutex(ob);
                break;
            }
            SetEvent(enter);
        }
        else if (c == BACK) // 退格删除字符
        {
            if (Y == 0) // 姓名框退格
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
            else if (Y == 1) // 电话框退格
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
        else // 普通字符输入
        {
            if (Y == 0) // 输入姓名
            {
                int l = strlen(a.name);
                if (l > 15)
                {
                    ReleaseMutex(ob);
                    continue;
                }
                // 区分单字节/UTF8多字节字符读取
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
            else if (Y == 1) // 输入手机号
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
        ReleaseMutex(ob);
    }
}

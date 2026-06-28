/**
 * @file head.h
 * @brief 校园通讯录管理系统 全局头文件
 * @author 黄耀盟
 * @date 2026-06-27
 * @version 1.0
 * @description
 * 1. 定义全局结构体：单联系人A、联系人列表AB；
 * 2. 声明Windows多线程同步内核对象（互斥锁、事件）；
 * 3. 全局变量X/Y/W/H状态量、界面渲染/输入处理函数声明；
 * 4. 控制台工具函数、字符串处理、文件操作、排序函数声明；
 * 5. 方向键、ESC、回车、退格等按键枚举常量定义；
 * @note 依赖Windows.h多线程API，仅支持Windows系统编译运行；
 * 所有.c源文件必须引入该头文件才能正常编译。
 */

#ifndef HEAD_H
#define HEAD_H
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
// 全局Windows内核同步对象
extern HANDLE cin, stop, del, movl, movr, movu, movd, enter, sw, sh, esc, back;
extern HANDLE ob; // 全局互斥锁，多线程访问界面/数据时加锁防止冲突
// ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼ ─ │
// 方向键扩展按键码（_getch读取0xE0前缀后的值）
enum
{
    UP = 72,    // 上箭头
    DOWN = 80,  // 下箭头
    LEFT = 75,  // 左箭头
    RIGHT = 77, // 右箭头
    BACK = 8,   // 退格键
    ENTER = 13, // 回车键
    DEL = 0x53, // 删除键
    ESC = 27,   // ESC退出键
};
extern int W, H, X, Y; // X：分页按钮左右选中；Y：菜单/列表纵向选中索引
extern int state;
/**
 * @brief 单个联系人结构体
 * name：姓名 最大19字符+结束符
 * number：手机号码 最大19字符+结束符
 */

struct A
{
    char name[20], number[20];
} ;
extern struct A a;
/**
 * @brief 联系人列表容器
 * N[200]：联系人数组，上限200条
 * n：当前有效联系人数量
 */

struct AB
{
    struct A N[200];
    int n;
};
extern struct AB list, li; // list原始总数据；li查询临时筛选数据
/**
 * @brief 设置控制台光标坐标
 * @param x 横坐标
 * @param y 纵坐标
 */
void setPos(int x, int y);

/**
 * @brief 显示/隐藏控制台光标
 * @param isHide 0显示 1隐藏
 */
void setHide(int isHide);

/**
 * @brief 格式化打印函数（支持可变参数，定位输出）
 * @param x 坐标x
 * @param y 坐标y
 * @param s 格式化字符串 const防止字面量类型冲突
 */
void printL(int x, int y, const char *s, ...);

/**
 * @brief qsort排序回调函数，按姓名字典序比较
 */
int cmp(const void *p1, const void *p2);

/**
 * @brief 计算UTF-8字符串视觉宽度（区分单字节/多字节汉字）
 * @param s 输入字符串
 * @return 屏幕占用字符宽度
 */
int lenStr(char *s);

/**
 * @brief 简易定位日志打印，同printL功能
 */
void flog(int x, int y, const char *s, ...);

/**
 * @brief 弹出红色提示弹窗
 * @param s 提示文字
 */
void wlog(const char *s);

/**
 * @brief 添加联系人至总list数组
 * @param a 待添加联系人
 * @return 0成功 1已满
 */
int add(struct A a);

/**
 * @brief 添加联系人至临时查询li数组
 * @param a 待添加联系人
 * @return 0成功 1容量满
 */
int add1(struct A a);

/**
 * @brief 【界面线程】添加联系人UI渲染逻辑
 */
void V1();

/**
 * @brief 【输入线程】添加联系人按键处理逻辑
 */
void O1();

/**
 * @brief 删除联系人界面渲染
 */
void V2();
/**
 * @brief 删除联系人按键处理
 */
void O2();

/**
 * @brief 修改联系人界面渲染
 */
void V3();

/**
 * @brief 修改联系人按键处理
 */
void O3();
#endif
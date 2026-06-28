/**
 * @file contacts_manager.c
 * @brief 校园通讯录管理系统（控制台版）
 *
 * 本程序是一个基于 Windows 控制台的交互式通讯录管理工具，专为校园场景设计。
 * 采用多线程架构实现流畅的用户界面与响应式操作，支持中文输入与显示。
 *
 * @author [黄耀盟（2025102459），廖吉（2025102453）]
 * @date 2026年6月27日
 * @version 1.0
 *
 * @par 核心功能
 * - 增：添加新联系人（姓名、电话）
 * - 删：从列表中删除指定联系人
 * - 改：修改现有联系人的信息
 * - 查：支持子串模糊查询，并按匹配位置智能排序
 * - 显：分页浏览全部联系人
 * - 排：按姓名进行字典序排序
 * - 存：数据自动持久化至本地文件
 *
 * @par 技术特性
 * - 多线程架构：分离绘图、输入、窗口监听逻辑
 * - 线程同步：使用 Windows 事件(Event)与互斥锁(Mutex)确保数据安全
 * - 伪图形界面：通过精准光标定位与 ANSI 颜色码构建友好 UI
 * - 中文支持：全程 UTF-8 编码，正确处理中文姓名与提示
 * - 智能查询：`For()` 函数实现按关键词出现位置排序的模糊匹配
 * - 数据持久化：启动时加载 `1123` 文件，退出时自动保存
 *
 * @par 数据结构
 * - 单个联系人：`struct A { char name[20]; char number[20]; }`
 * - 联系人列表：`struct AB { struct A N[200]; int n; }` (最多200条)
 *
 * @note 编译环境要求：
 *       - Windows 操作系统（依赖 Windows.h API）
 *       - 支持 C99 标准的编译器（如 GCC MinGW-w64, MSVC）
 *       - 链接标准 C 库（无需额外链接库）
 *
 * @build 编译示例 (MinGW):
 *        gcc -o contacts.exe contacts_manager.c
 *
 * @license 本程序仅供学习与交流使用。
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
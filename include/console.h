//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#ifndef _CONSOLE_H__
#define _CONSOLE_H__

#include <string>
#include <iostream>
#include <memory.h>
#include <stdlib.h>
#include <pwd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>

//定义读取缓存区长度
#define BUFFER_SIZE 1024 

//控制台输出颜色宏定义
#define BOLD_ON "\e[1m"
#define BOLD_OFF "\e[0m"
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"

using namespace std;

//函数定义

void DisplayPrompt();

void InputCommand(char *input, const int len);
void InputCommand(char *input, const int len, ifstream& in);

#endif
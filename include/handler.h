//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#ifndef _HANDLER_H__
#define _HANDLER_H__

#include <csignal>

using namespace std;

//函数声明
void InitializeHandler();
void SIGCHLD_handler(int sig_no, siginfo_t* siginfo, void* arg);
void SIGTSTP_handler(int sig_no, siginfo_t* siginfo, void* arg);

#endif
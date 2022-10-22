//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#ifndef _COMMAND_H__
#define _COMMAND_H__

#include <ctime>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/limits.h>

using namespace std;

//定义单命令类
class Command
{
private:
    enum CommandType {BG, CD, CLR, DIR, ECHO, EXEC, EXIT, FG, HELP, JOBS, PWD, SET, TEST, TIME, UMASK, EXT} _type; //命令类型
    vector<string> _args; //参数集合
    enum {STDIN, FILEIN} _in_flag; //重定向标志
    enum {STDOUT, FILE_OVERRIDE, FILE_ADD, PIPE_OUT} _out_flag; //重定向标志
    string inPath; //输入重定向路径
    string outPath; //输出重定向路径
    string externName; //外部程序名
    bool isBackground; //是否是后台任务
    string execRes; //指令的输出缓存

    void KeywordAnalysis(vector<string>& split); //关键词分析
    void TypeAnalysis(vector<string>& split); //类型分析

    //内置指令
    void time_CMD(); 
    void set_CMD();
    void help_CMD();
    void exit_CMD();
    void pwd_CMD();
    void clr_CMD();
    void cd_CMD();
    void echo_CMD();

    //dir指令文件结构遍历的函数
    void fileTrav(string path);

    void dir_CMD();
    void umask_CMD();
    void exec_CMD();

    //用于不同类型的test指令的分函数
    bool fileTest(string& path, string& mode);
    bool numTest(string& lhs, string& rhs, string& mode);
    bool strTest(string& lhs, string& rhs, string& mode);

    void test_CMD();
    void jobs_CMD();
    void bg_CMD();
    void fg_CMD();
    void extern_CMD();
public:
    Command(vector<string>& split); //通过分割好的str向量生成command对象
    ~Command();
    void PipeOut(); //设置pipeout flag
    void Execute(); //执行命令，结果保存在execres中
    void RedirectInput(); //执行重定向输入
    void RedirectInput(string str); //管道的重定向输入
    void RedirectOutput();//执行重定向输出
};

#endif

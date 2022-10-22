//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#include "include/console.h"

using namespace std;

//显示提示符
void DisplayPrompt()
{
    char path[PATH_MAX]; //当前路径
    char hostname[HOST_NAME_MAX]; //当前主机名

    //获取基本用户信息
    getcwd(path,PATH_MAX);
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    string username = pwd->pw_name;
    gethostname(hostname, HOST_NAME_MAX);

    //命令行提示符输出（带有颜色）
    cout << LIGHT_RED << "[myshell]" 
    << GREEN << BOLD_ON << username << "@" << hostname << BOLD_OFF 
    << NONE << ":" << BLUE << BOLD_ON << path << BOLD_OFF << NONE << "$ ";
}

//从标准输入读取指令
void InputCommand(char *input, const int len)
{
    memset(input, 0, len);
    int i = 0;
    char ch;
    while ((ch = getchar()) != '\n') //指令用回车分割
    {
        input[i++] = ch;
    }

    if(input[0] == -1) //测试中出现fork子进程结束后的读取bug，原因不明，通过跳过第一个字符来修复
    {
        for(size_t i = 0; i < strlen(input); i++)
        {
            input[i] = input[i+1];
        }
    }
}

//从文件读取命令行
void InputCommand(char *input, const int len, ifstream& in)
{
    //通过getline反复读取每一行来实现，存在读取 空字符串 现象
    memset(input, 0, len);
    string str;
    getline(in, str);
    strcpy(input, str.c_str());
}
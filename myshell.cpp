//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#include "include/console.h"
#include "include/Command.h"
#include "include/parser.h"
#include "include/error.h"
#include "include/handler.h"
#include "include/joblist.h"

//全局变量--任务列表
joblist TaskList;

int main(int argc, char *argv[])
{
    //设置环境变量SHELL
    char temp[PATH_MAX];
    getcwd(temp, PATH_MAX);
    setenv("SHELL", (string(temp)+"/myshell").c_str(), 1);

    //初始化
    InitializeHandler();
    TaskList.Initialize();

    //文件流
    ifstream in;
    if(argc == 2)
    {
        in.open(argv[1], ios::in); //读入文件
        if (!in.is_open())
        {
            ERROR("No such file.");
            return 0;
        }
    }

    //程序循环
    while (true)
    {
        char cmd[BUFFER_SIZE]; //存储命令行
        if(argc == 1) //无参数,用户输入
        {
            DisplayPrompt(); //显示命令行提示符
            InputCommand(cmd, BUFFER_SIZE); //输入指令
        }
        else if(argc == 2)
        {
            //从文件中读入指令
            if(in.eof())
            {
                break;
            }
            InputCommand(cmd, BUFFER_SIZE, in); //文件输入指令 
        }
        else{
            //异常处理
            ERROR("Too many arguments.");
            break;
        }

        //命令行解析
        Parser(cmd);
    }

    //将任务列表进行共享内存释放和删除
    TaskList.Release();
    TaskList.Delete();
    
    return 0;
}
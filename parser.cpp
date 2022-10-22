//作业名：myshell模拟终端
//姓名：张嘉季 学号：3200105797

#include "include/parser.h"
#include "include/Command.h"
#include "include/joblist.h"
#include "include/error.h"

//将读取的cmd指令进行分割，结构放在res里
void Spliter(vector<string>& res, string& str)
{
    string temp = "";
    bool isPush = false;
    for (size_t i = 0; i < str.size(); i++)
    {
        if(str[i] != ' ' && str[i] != '\t') //遇到空格和制表符则跳过，其他直接读入
        {
            temp += str[i];
            isPush = false;
        }
        else {
            //防止重复读入或漏掉普通字符的检验
            if(!isPush)
            {
                res.push_back(temp);
                isPush = true;
            }
            temp = "";
        }
    }
    if (temp != "")
    {
        res.push_back(temp); //防止漏掉不是空格结尾的字符
    }
}

//解释器函数，解析执行命令
void Parser(const char* cmd)
{
    //除去非法读取的指令，直接返回
    if(cmd[0] <= 0)
    {
        return;
    }

    string line = cmd;
    vector<string> split;
    Spliter(split, line); //将指令分割为字符串向量
    
    auto pipeFlag = find(split.begin(), split.end(), "|"); //寻找管道符 |
    if(pipeFlag == split.end()) //无管道 单指令
    {
        //构造对象并执行命令
        Command command(split);
        command.RedirectInput();
        command.Execute();
        command.RedirectOutput();
    }
    else{
        //管道操作
        vector<string> lhs;
        vector<string> rhs;

        size_t index = pipeFlag - split.begin();
        for (size_t i = 0; i < split.size(); i++)
        {
            if(i < index)
            {
                lhs.push_back(split[i]);
            }
            else if(i > index)
            {
                rhs.push_back(split[i]);
            }
        }
        
        if(lhs.size() == 0 || rhs.size() == 0) //错误命令判断
        {
            ERROR("Syntax error.");
            return;
        }

        //首先正常生成两个单命令
        Command left_cmd(lhs);
        Command right_cmd(rhs);

        //创建管道
        int s;
        int pipefd[2];
        pipe(pipefd);

        pid_t pid = fork();

        if(pid == 0)
        {
            //子进程，关闭读端，只用写端
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO); //重定向标准输出到管道写端

            left_cmd.PipeOut();
            left_cmd.RedirectInput();
            left_cmd.Execute();
            left_cmd.RedirectOutput(); //重定向输出，pipeflag使得标准输出内容直接写进管道中

            close(pipefd[1]); //关闭管道，子进程退出
            exit(0);
        }
        else if(pid > 0)
        {
            //父进程
            waitpid(pid, &s, 0); //等待子进程结束
            close(pipefd[1]); //关闭管道写端
            char temp[PIPE_BUFFER];
            read(pipefd[0], temp, PIPE_BUFFER); //从管道中读取内容至缓存区

            right_cmd.RedirectInput(temp); //将缓存区内容重定向到参数表中
            right_cmd.Execute();
            right_cmd.RedirectOutput(); //正常执行并输出

            close(pipefd[0]); //关闭管道
        }
        else{
            ERROR("Failed to fork a process.");
        }
    }
}
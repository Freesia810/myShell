#ifndef _JOBLIST_H__
#define _JOBLIST_H__

#include <unistd.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <csignal>
#include <vector>

//定义最大任务数和任务名限制
#define MAX_JOB 64
#define MAX_JOB_NAME 32

using namespace std;

//定义joblist类
class joblist
{
private:
    //定义job结构体
    struct job
    {
        pid_t _pid; //任务的进程pid
        enum {RUNNING, HANG_UP, FINISHED} _state; //任务的状态：运行、挂起、完成
        bool _isBackground; //任务是否是后台进行
        char _name[MAX_JOB_NAME]; //任务名
    };
    int _shareMemoryID; //进程表的共享内存ID
    job* _listPt; //进程表数组的首地址
    size_t* _numPt; //进程表内的进程数量
public:
    joblist();
    void Initialize(); //进程表初始化
    int AddJob(pid_t pid, string name, bool isBackground); //添加进程
    void RmJob(pid_t pid); //删除进程
    void FinishJob(pid_t pid); //标记运行中的进程为完成
    void HangupJob(); //挂起运行中的进程
    void ContJob2Background(size_t ID); //bg指令，将挂起指令在后台运行
    void ContJob2Front(size_t ID); //fg指令，将后台指令在前台运行
    void PrintList(string& str); //打印当前的任务列表
    void Release(); //释放当前进程与共享内存的联系
    void Delete(); //删除共享内存空间
    ~joblist();
};




#endif

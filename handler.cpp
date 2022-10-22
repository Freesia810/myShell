#include "include/handler.h"
#include "include/joblist.h"

using namespace std;

extern joblist TaskList;

void InitializeHandler()
{
    //新建信号处理方式
    struct sigaction SIGCHD;
    SIGCHD.sa_flags = SA_SIGINFO; //通过采用siginfo使得捕获信号时也能获取发出信号的pid以便后续处理
    SIGCHD.sa_sigaction = SIGCHLD_handler;

    struct sigaction SIGSTP;
    SIGSTP.sa_flags = SA_SIGINFO;
    SIGSTP.sa_sigaction = SIGTSTP_handler;

    //创建自定义信号捕获处理的回调函数
    sigaction(SIGCHLD, &SIGCHD, nullptr);
    sigaction(SIGTSTP, &SIGSTP, nullptr);
}

//在挂起、结束、停止子进程时，子进程都会发出SIGCHLD信号，主进程捕获到这一信号在该函数进行处理
void SIGCHLD_handler(int sig_no, siginfo_t* siginfo, void* arg)
{
    TaskList.FinishJob(siginfo->si_pid);
}

//挂起子进程时，发出SIGTSTP信号，主进程进行处理
void SIGTSTP_handler(int sig_no, siginfo_t* siginfo, void* arg)
{
    printf("\n");
    TaskList.HangupJob();
}

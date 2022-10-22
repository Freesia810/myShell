#include "include/joblist.h"
#include "include/error.h"

using namespace std;

void joblist::Initialize()
{
    //创建共享内存
    _shareMemoryID = shmget(4396, sizeof(size_t) + sizeof(job) * MAX_JOB, 0666 | IPC_CREAT);
    if(_shareMemoryID == -1)
    {
        ERROR("Failed to create memory.");
        return;
    }
    //将当前进程连接到共享内存，返回地址
    void* locMemory = shmat(_shareMemoryID, 0, 0);
    if(locMemory == (void*)-1)
    {
        ERROR("Failed to locate memory.");
        return;
    }
    //将指针进行赋值给成员变量
    _numPt = (size_t*)locMemory;
    _listPt = (job*)((char*)locMemory + sizeof(size_t));
    memset(_listPt, 0, MAX_JOB);

    //将myshell作为第一个进程，添加到列表中
    *_numPt = 1;
    _listPt[0]._pid = getpid();
    strcpy(_listPt[0]._name, "myshell");
    _listPt[0]._isBackground = false;
    _listPt[0]._state = job::RUNNING;
}

joblist::joblist()
{
}

int joblist::AddJob(pid_t pid, string name, bool isBackground)
{
    if(*_numPt == MAX_JOB)
    {
        ERROR("Full jobs.");
        return -1;
    }

    //直接将列表向后写入一个job结构体
    _listPt[*_numPt]._pid = pid;
    strcpy(_listPt[*_numPt]._name, name.c_str());
    _listPt[*_numPt]._isBackground = isBackground;
    _listPt[*_numPt]._state = job::RUNNING;
    //计数器增加
    *_numPt = *_numPt + 1;

    //如果是后台任务，存在返回值，返回作业编号
    if(isBackground)
    {
        int res = 0;

        for (size_t i = 0; i < *_numPt; i++)
        {
            //跳过非后台任务
            if(_listPt[i]._isBackground)
            {
                res++;
            }
        }
        return res;
    }
    else{
        //前台任务返回-1
        return -1;
    }
}

//根据pid找到任务然后删除
void joblist::RmJob(pid_t pid)
{
    size_t flag;
    for(size_t i = 0; i < *_numPt; ++i)
    {
        if(_listPt[i]._pid == pid)
        {
            flag = i;
            break;
        }
    }

    if(flag < *_numPt)
    {
        //找到后将之后的任务都向前移动一个位置
        for (size_t i = flag; i < (*_numPt) - 1; i++)
        {
            _listPt[i] = _listPt[i+1];
        }
        *_numPt = *_numPt - 1;
    }
}

//根据指定pid将任务标记为完成
void joblist::FinishJob(pid_t pid)
{
    size_t flag;
    for(size_t i = 0; i < *_numPt; ++i)
    {
        if(_listPt[i]._pid == pid)
        {
            flag = i;
            break;
        }
    }
    
    if(flag < *_numPt)
    {
        //必须是正在进行的任务才能标记为完成
        if(_listPt[flag]._isBackground && _listPt[flag]._state == job::RUNNING)
        {
            _listPt[flag]._state = job::FINISHED;
        }
        else if (_listPt[flag]._isBackground && _listPt[flag]._state == job::HANG_UP)
        {
            //如果是后台挂起的任务，不做处理
        }       
        else{
            //前台完成任务，直接删除
            RmJob(pid);
        }
    }
}

void joblist::HangupJob()
{
    size_t flag;
    for(size_t i = *_numPt - 1; i > 0; i--)
    {
        if(!_listPt[i]._isBackground)
        {
            flag = i;
            break;
        }
    }
    //将最后的前台任务挂起，标记为后台任务
    if(flag > 0)
    {
        _listPt[flag]._state = job::HANG_UP;
        _listPt[flag]._isBackground = true;
    }
}

void joblist::ContJob2Background(size_t ID)
{
    size_t flag = 1;
    for (size_t i = 0; i < *_numPt; i++)
    {
        if(_listPt[i]._isBackground)
        {
            //如果是后台挂起任务且符合指定的pid
            if(_listPt[i]._state == job::HANG_UP && ID == flag)
            {
                //让该进程继续
                kill(_listPt[i]._pid, SIGCONT);

                //等待信号响应时间，删除会有bug
                sleep(1);
                //再将进程标记为运行
                _listPt[i]._state = job::RUNNING;
                break;
            }
            flag++;
        }
    }
    
}

void joblist::ContJob2Front(size_t ID)
{
    int s;
    size_t flag = 1;
    for (size_t i = 0; i < *_numPt; i++)
    {
        if(_listPt[i]._isBackground)
        {
            if(ID == flag)
            {
                //如果是后台挂起任务
                if(_listPt[i]._state == job::HANG_UP)
                {
                    //先让其继续
                    kill(_listPt[i]._pid, SIGCONT);
                    //等待信号传递
                    sleep(1);
                    //前台等待该进程结束
                    waitpid(_listPt[i]._pid, &s, 0);
                    _listPt[i]._state = job::RUNNING;
                    //标记该任务结束
                    FinishJob(_listPt[i]._pid);
                }
                else if(_listPt[i]._state == job::RUNNING)
                {
                    //如果是后台正在运行的任务，直接等待该进程结束
                    waitpid(_listPt[i]._pid, &s, 0);
                }
                //标记前台进程
                _listPt[i]._isBackground = false;
            }
            flag++;
        }
    }


}

//直接打印所有信息至str中
void joblist::PrintList(string& str)
{
    size_t backgroundNum = 0;
    vector<size_t> rmVec;
    for (size_t i = 0; i < *_numPt; i++)
    {
        if(_listPt[i]._isBackground)
        {
            str += "[" + to_string(backgroundNum + 1) + "] " + to_string(_listPt[i]._pid) + " ";
            switch (_listPt[i]._state)
            {
            case job::RUNNING:
                str += "RUNNING\t";
                break;
            case job::HANG_UP:
                str += "HANG_UP\t";
                break;
            case job::FINISHED:
                str += "FINISHED\t";
                //将完成的任务放入向量中等待删除
                rmVec.push_back(i);
                break;
            default:
                break;
            }
            str += _listPt[i]._name;
            str += "\n";
            backgroundNum++;
        }
    }
    
    //删除所有完成的任务
    for(auto i:rmVec)
    {
        RmJob(_listPt[i]._pid);
    }
}

void joblist::Release()
{
    //释放与共享内存的连接
    if(shmdt((void*)_numPt) == -1)
    {
        ERROR("Failed to disconnect memory");
    }  
}
void joblist::Delete()
{
    //删除共享内存
    if(shmctl(_shareMemoryID, IPC_RMID, nullptr) == -1)
    {
        ERROR("Failed to release memory");
    }
}

joblist::~joblist()
{
}

#include "include/Command.h"
#include "include/error.h"
#include "include/joblist.h"

extern joblist TaskList;

Command::Command(vector<string>& split)
{
    //构造函数，先提取关键词，再分析类型
    KeywordAnalysis(split);
    TypeAnalysis(split);
}

Command::~Command()
{
}

//关键词分析提取截断
void Command::KeywordAnalysis(vector<string>& split)
{
    if(split.back() == "&") //后台任务判断
    {
        this->isBackground = true;
        split.pop_back(); //把 & 删除
    }
    else{
        this->isBackground = false;
    }

    //默认重定向类型
    this->_in_flag = STDIN;
    this->_out_flag = STDOUT;

    //重定向分析
    auto inFlag = find(split.begin(), split.end(), "<"); //寻找输入标志
    if(inFlag != split.end())
    {
        this->_in_flag = this->FILEIN;
        auto next = split.erase(inFlag);
        this->inPath = *(next);
        split.erase(next);
    }
    auto outORFlag = find(split.begin(), split.end(), ">"); //寻找覆盖标志
    if (outORFlag != split.end())
    {
        this->_out_flag = this->FILE_OVERRIDE;
        auto next = split.erase(outORFlag);
        this->outPath = *(next);
        split.erase(next);
    }
    auto outADDFlag = find(split.begin(), split.end(), ">>"); //寻找追加标志
    if (outADDFlag != split.end())
    {
        this->_out_flag = this->FILE_ADD;
        auto next = split.erase(outADDFlag);
        this->outPath = *(next);
        split.erase(next);
    }
}

//命令类型分析
void Command::TypeAnalysis(vector<string>& split)
{
    string cmdName = split[0]; //获取命令名

    transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower); //支持大小写互换

    //根据字符串内容确定类型
    if(cmdName == "bg")
    {
        this->_type = BG;
    }
    else if (cmdName == "cd")
    {
        this->_type = CD;
    }
    else if (cmdName == "clr")
    {
        this->_type = CLR;
    }
    else if (cmdName == "dir")
    {
        this->_type = DIR;
    }
    else if (cmdName == "echo")
    {
        this->_type = ECHO;
    }
    else if (cmdName == "exec")
    {
        this->_type = EXEC;
    }
    else if (cmdName == "exit")
    {
        this->_type = EXIT;
    }
    else if (cmdName == "fg")
    {
        this->_type = FG;
    }
    else if (cmdName == "help")
    {
        this->_type = HELP;
    }
    else if (cmdName == "jobs")
    {
        this->_type = JOBS;
    }
    else if (cmdName == "pwd")
    {
        this->_type = PWD;
    }
    else if (cmdName == "set")
    {
        this->_type = SET;
    }
    else if (cmdName == "test")
    {
        this->_type = TEST;
    }
    else if (cmdName == "time")
    {
        this->_type = TIME;
    }
    else if (cmdName == "umask")
    {
        this->_type = UMASK;
    }
    else{
        //非默认命令识别为外部指令，并记录名字
        this->_type = EXT;
        this->externName = split[0];
    }

    //剩下的分割结果作为参数
    split.erase(split.begin());
    this->_args = split;
}

void Command::time_CMD() //获取系统时间
{
    time_t now;
	time(&now); //获取系统时间戳
	tm* t = localtime(&now); //获取当地时间

    char temp[30]; //格式化输出
    sprintf(temp, "%d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec); //将时间格式化输出
    execRes = temp;
}

void Command::set_CMD() //获取所有环境变量
{
    extern char **environ; //系统环境变量的字符串数组

    for(int i = 0; environ[i] != NULL; i++)
    {
        char temp[PATH_MAX];
        sprintf(temp, "%s\n",environ[i]); //将每一个环境变量写入
        execRes += temp;
    }
}

void Command::exit_CMD() //退出程序
{
    exit(0);
}

void Command::clr_CMD() //清屏指令
{
    printf("\033c"); //输出清屏的标识符
}

void Command::pwd_CMD() //显示当前目录
{
    char* temp = getenv("PWD"); //直接去获取当前的环境变量即可
    if(temp == nullptr)
    {
        ERROR("Failed to get PWD path.");
        return;
    }
    execRes = string(temp) + "\n";
}

void Command::cd_CMD() //切换工作目录
{
    if(_args.size()==1) //如果有正确的输入
    {
        if(_args[0] == "~") //主目录提示符
        {
            _args[0] = getenv("HOME"); //获取主目录绝对路径
        }
        if(!chdir(_args[0].c_str()))
        {
            setenv("PWD", _args[0].c_str(), 1); //改变pwd环境变量
        }
        else
        {
            ERROR("Failed to change the working directory.");
        }
    }
    else if (_args.size()==0)
    {
        //无参数，不用改变目录
    }
    else
    {
        ERROR("Too many arguments.");
    }
}

void Command::fileTrav(string path)
{
    if(path == "~") //主目录识别
    {
        path = getenv("HOME");
    }
    auto dir = opendir(path.c_str()); //打开目录
    execRes += path + ":" + "\n"; //输出路径信息
    if (dir == nullptr)
    {
        ERROR("Failed to open the directory.");
        return;
    }
    struct dirent* stdir;
    while (true)
    {
        //遍历目录内所有文件
        if ((stdir = readdir(dir)) == 0)
        {
            break;
        }
        if(string(stdir->d_name) != "." && string(stdir->d_name) != "..") //略去"." 和 ".."这两个文件
        {
            execRes += string(stdir->d_name) + "\n";
        }
    }
    closedir(dir); //关闭目录
}
void Command::dir_CMD() //列出目录下所有文件
{
    if(_args.size() == 0) //输出当前目录的结构
    {
        char temp[PATH_MAX];
	    getcwd(temp, PATH_MAX); //保存当前路径
        fileTrav(temp);
        return;
    }
    for (size_t i = 0; i < _args.size(); i++) //多参数分别输出
    {
        fileTrav(_args[i]);
    }
}

void Command::umask_CMD()
{
    if(_args.size() == 0) //如果无参数
    {
        //输出当前掩码
        mode_t curMask;
        curMask = umask(0); //先用umask的返回值获得当前掩码
        umask(curMask); //再将返回的掩码重新设置
        char temp[10];
        sprintf(temp, "%04d\n", curMask);
        execRes = temp;
    }
    else if(_args.size() == 1)
    {
        //对于输入掩码参数的合法性判断
        if(_args[0].size() <= 4)
        {
            for (size_t i = 0; i < _args[0].size(); i++)
            {
                if(_args[0][i] >= '8'|| _args[0][i] < '0') //必须是0-7的数字
                {
                    ERROR("Umask code should be octal.");
                    return;
                }
            }
            mode_t new_mask = atoi(_args[0].c_str()) % 1000;
            umask(new_mask); //设置新的掩码
        }
        else{
            ERROR("Umask code should be less than 5 digits.");
        }
    }
    else{
        ERROR("Too many arguments.");
    }
}

void Command::exec_CMD()
{
    if(_args.size() >= 1) //第一个参数是指令名 后面是参数
    {
        char** arguments = new char*[_args.size()]; //将参数进行复制
        for (size_t i = 0; i < _args.size(); i++)
        {
            strcpy(arguments[i], _args[i].c_str());
        }
        if(execvp(_args[0].c_str(), arguments) != 0) //调用execvp执行外部程序，结束后myshell退出
        {
            ERROR("Failed to execute the program.");
        }
        delete[] arguments;
    }
    else{
        ERROR("No arguments.");
    }
}

void Command::echo_CMD()
{
    for(auto arg:_args) //将所有参数进行输出，略去多空格和制表符
    {
        if(arg[0] == arg[arg.size()-1] && (arg[0] == '\"' || arg[0] == '\'')) //对于字符串，输出会将引号略去
        {
            arg = arg.substr(1, arg.size() - 2);
        }
        execRes += arg + " ";
    }
    if(_args.size() != 0)
    {
        execRes += "\n";
    }
}

bool Command::fileTest(string& path, string& mode)
{
    if(access(path.c_str(), F_OK) == 0) //文件存在
    {
        if(mode == "-e") //-e 文件是否存在
        {
            return true;
        }

        struct stat buffer; //获取文件统计信息
        if(stat(path.c_str(), &buffer) == 0)
        {
            if (mode == "-f") //-f 文件是否是普通文件
            {
                return S_ISREG(buffer.st_mode);
            }
            else if (mode == "-d") //-d 文件是否是目录
            {
                return S_ISDIR(buffer.st_mode);
            }
            else{
                ERROR("Wrong argument.");
                return false;
            }
        }
        else{
            ERROR("Failed to find the attribute.");
            return false;
        }
    }
    else{
        //非法参数报错
        if(mode != "-e" && mode != "-f" && mode != "-d")
        {
            ERROR("Wrong argument.");
        }
        return false;
    }
}
bool Command::numTest(string& lhs, string& rhs, string& mode) //检测整数的test
{
    for (size_t i = 0; i < lhs.size(); i++) //输入参数必须是整数
    {
        if(lhs[i] > '9' || lhs[i] < '0')
        {
            ERROR("Please input an integer.");
            return false;
        }
    }
    for (size_t i = 0; i < rhs.size(); i++)
    {
        if(rhs[i] > '9' || rhs[i] < '0')
        {
            ERROR("Please input an integer.");
            return false;
        }
    }

    if(mode == "-eq") //-eq 判断相等
    {
        return atoi(lhs.c_str()) == atoi(rhs.c_str());
    }
    else if (mode == "-ne") //-ne 判断不等
    {
        return atoi(lhs.c_str()) != atoi(rhs.c_str());
    }
    else if (mode == "-lt") //-lt 判断小于
    {
        return atoi(lhs.c_str()) < atoi(rhs.c_str());
    }
    else if (mode == "-le") //-le 判断小于等于
    {
        return atoi(lhs.c_str()) <= atoi(rhs.c_str());
    }
    else if (mode == "-gt") //-gt 判断大于
    {
        return atoi(lhs.c_str()) > atoi(rhs.c_str());
    }
    else if (mode == "-ge") //-ge 判断大于等于
    {
        return atoi(lhs.c_str()) >= atoi(rhs.c_str());
    }
    else{
        //非法参数报错
        ERROR("Wrong argument.");
        return false;
    }
}
bool Command::strTest(string& lhs, string& rhs, string& mode) //判断是否空中，单参数只用rhs1
{
    //将字符串引号略去
    if((lhs[0] == lhs[lhs.size()-1] && (lhs[0] == '\"' || lhs[0] == '\'')) && (rhs[0] == rhs[rhs.size()-1] && (rhs[0] == '\"' || rhs[0] == '\'')))
    {
        if(mode == "-z") //判断是否为空
        {
            return rhs == "";
        }
        else if (mode == "-n") //判断是否非空
        {
            return rhs != "";
        }
        else if (mode == "=") //判断是否相等
        {
            return lhs == rhs;
        }
        else if (mode == "==") //判断是否相等
        {
            return lhs == rhs;
        }
        else if (mode == "!=") //判断是否不等
        {
            return lhs != rhs;
        }
        else{
            ERROR("Wrong argument.");
            return false;
        }
    }
    else{
        ERROR("A string should be included with \' or \".");
        return false;
    }
}
void Command::test_CMD() //test指令
{
    bool res;
    //根据不同参数调用不同的test函数
    if(_args.size() == 2)
    {
        if(_args[0] == "-z" || _args[0] == "-n")
        {
            res = strTest(_args[1], _args[1], _args[0]);
        }
        else{
            res = fileTest(_args[1], _args[0]);
        }
    }
    else if(_args.size() == 3)
    {
        if(_args[1] == "=" || _args[1] == "==" || _args[1] == "!=")
        {
            res = strTest(_args[0], _args[2], _args[1]);
        }
        else{
            res = numTest(_args[0], _args[2], _args[1]);
        }
    }
    else{
        ERROR("Too many arguments.");
    }
    //结果输出
    execRes = (res == true?"True\n":"False\n");
}

void Command::bg_CMD()
{
    //直接调用joblist提供的接口，将挂起任务转至后台
    if(_args.size() == 1)
    {
        TaskList.ContJob2Background((size_t)atoi(_args[0].c_str()));
    }
    else{
        ERROR("Invalid arguments.");
    }
}
void Command::fg_CMD()
{
    //直接调用接口，将后台任务转至前台
    if(_args.size() == 1)
    {
        TaskList.ContJob2Front((size_t)atoi(_args[0].c_str()));
    }
    else{
        ERROR("Invalid arguments.");
    }
}
void Command::jobs_CMD()
{
    //打印任务表
    TaskList.PrintList(execRes);
}
void Command::help_CMD()
{
    //如果是标准输出，使用more指令过滤
    if(_out_flag == STDOUT)
    {
        pid_t pid = fork(); //创建子进程

        if(pid < 0)
        {
            ERROR("Failed to fork a process.");
        }
        else if(pid == 0)
        {
            //子进程运行
            string envPath = "PARENT=" + string(getenv("SHELL"));
            char* temp = (char*)malloc(sizeof(char) * PATH_MAX);
            strcpy(temp, envPath.c_str());
            putenv(temp); 
            free(temp); 

            //调用execvp
            char** arguments = new char*[1];
            string s = getenv("SHELL");
            s = s.substr(0, s.size() - 7);
            s = s + "README";
            strcpy(arguments[0], s.c_str());
            if(execvp("more", arguments) != 0) //调用more打开帮助文档
            {
                ERROR("Failed to execute the program.");
            }
            delete[] arguments;

            //正常退出子进程
            exit(0);
        }
        else{
            //这个子进程前台运行，阻塞父进程
            int s;
            waitpid(pid, &s, 0);
        }
    }
    else{
        //非标准输出直接将帮助文档内容存储在结果中
        ifstream fs("README");
        stringstream ss;
        ss << fs.rdbuf();
        execRes = ss.str();
    }
}

void Command::extern_CMD() //执行外部命令
{
    pid_t pid = fork(); //创建子进程

    if(pid < 0)
    {
        ERROR("Failed to fork a process.");
    }
    else if(pid == 0)
    {
        //子进程运行
        string envPath = "PARENT=" + string(getenv("SHELL")); //设置环境变量
        char* temp = (char*)malloc(sizeof(char) * PATH_MAX);
        strcpy(temp, envPath.c_str());
        putenv(temp); 
        free(temp); 

        //调用execvp
        char** arguments = new char*[_args.size() + 1];
        strcpy(arguments[0], this->externName.c_str()); //参数复制
        for (size_t i = 0; i < _args.size(); i++)
        {
            strcpy(arguments[i + 1], _args[i].c_str());
        }
        if(execvp(this->externName.c_str(), arguments) != 0) //执行外部程序
        {
            ERROR("Failed to execute the program.");
        }
        delete[] arguments;

        //正常退出子进程
        exit(0);
    }
    else{
        //父进程运行
        if(this->isBackground) //这个子进程需要在后台运行
        {
            //直接在后台运行 WNOHANG，并添加到任务列表中
            int s;
            int jobID = TaskList.AddJob(pid, externName, true);
            printf("[%d] %d\n",jobID, pid);
            waitpid(pid, &s, WNOHANG);
        }
        else{
            //这个子进程前台运行，阻塞父进程
            int s;
            TaskList.AddJob(pid, externName, false);
            waitpid(pid, &s, 0);
        }
    }
}


void Command::RedirectInput(string str)
{
    //适用于管道的重定向，将读取的参数push到参数表中
    _args.push_back(str);
}
void Command::RedirectInput()
{
    ifstream in;
    string arg;
    switch (_in_flag)
    {
    case FILEIN: //文件重定向，从文件中读取参数
        in.open(inPath, ios::in);
        in >> arg;
        _args.push_back(arg);
        break;
    case STDIN:
        break;
    default:
        break;
    }
}
void Command::RedirectOutput()
{
    ofstream os;
    switch (_out_flag)
    {
    case STDOUT: //标准重定向，直接输出
        cout << execRes;
        break;
    case FILE_OVERRIDE: //文件覆盖重定向
        os.open(outPath, ios::out);
        os << execRes;
        os.close();
        break;
    case FILE_ADD: //文件追加重定向
        os.open(outPath, ios::out|ios::app);
        os << execRes;
        os.close();
        break;
    case PIPE_OUT: //管道输出
        if(execRes[execRes.size()-1] == '\n') //管道输出去掉回车符
        {
            execRes = execRes.substr(0, execRes.size()-1);
        }
        cout << execRes;
        cout << '\0'; //因为输出到管道时默认没有结束符，需要手动加上结束符，否则会错误读取
        break;
    default:
        break;
    }
}

void Command::Execute()
{
    //外部程序单独执行
    if(_type == EXT)
    {
        extern_CMD();
        return;
    }

    //如果是内部后台程序，则在子进程内执行
    if(isBackground)
    {
        pid_t pid = fork();

        if(pid < 0)
        {
            ERROR("Failed to fork a process.");
        }
        else if(pid == 0)
        {   
            switch (_type)
            {
            case CD:
                cd_CMD();
                break;
            case DIR:
                dir_CMD();
                break;
            case ECHO:
                echo_CMD();
                break;
            case EXEC:
                exec_CMD();
                break;
            case JOBS:
                jobs_CMD();
                break;
            case PWD:
                pwd_CMD();
                break;
            case SET:
                set_CMD();
                break;
            case TEST:
                test_CMD();
                break;
            case TIME:
                time_CMD();
                break;
            case UMASK:
                umask_CMD();
                break;
            default:
                ERROR("Unsupported background command.");
                break;
            }
            RedirectOutput(); //子进程的重定向输出
            //正常退出子进程
            exit(0);
        }
        else{
            //父进程运行
            int s;
            int jobID = TaskList.AddJob(pid, "Command", true); //内部指令任务名均为Command
            printf("[%d] %d\n",jobID, pid);
            waitpid(pid, &s, WNOHANG); //后台进程不必等待
        }
    }
    else{
        //前台任务，直接进行
        switch (_type)
        {
        case BG:
            bg_CMD();
            break;
        case CD:
            cd_CMD();
            break;
        case CLR:
            clr_CMD();
            break;
        case DIR:
            dir_CMD();
            break;
        case ECHO:
            echo_CMD();
            break;
        case EXEC:
            exec_CMD();
            break;
        case EXIT:
            exit_CMD();
            break;
        case FG:
            fg_CMD();
            break;
        case HELP:
            help_CMD();
            break;
        case JOBS:
            jobs_CMD();
            break;
        case PWD:
            pwd_CMD();
            break;
        case SET:
            set_CMD();
            break;
        case TEST:
            test_CMD();
            break;
        case TIME:
            time_CMD();
            break;
        case UMASK:
            umask_CMD();
            break;
        default:
            break;
        }
    }
}

void Command::PipeOut()
{
    //用于调整重定向标志为pipe
    _out_flag = PIPE_OUT;
}

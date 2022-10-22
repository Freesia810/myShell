#ifndef _PARSER_H__
#define _PARSER_H__

#include <vector>
#include <string>

#define PIPE_BUFFER 4096 //定义管道缓存区大小

using namespace std;

//函数声明
void Spliter(vector<string>& res, string& str);//分割字符串
void Parser(const char* cmd);//解析指令

#endif

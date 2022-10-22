#ifndef _ERROR_H__
#define _ERROR_H__

#include "console.h"

//定义统一的错误输出格式，加红加粗
#define ERROR(err) printf(RED BOLD_ON "[myshell]ERROR: " RED BOLD_ON err BOLD_OFF "\n")

#endif

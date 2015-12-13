


#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdlib.h>
#include <vector>
#include <Windows.h>
#include <regex>
#include <sstream>


using namespace std;

unsigned int binary_to_uint(string &s, int &bits);		//二进制串转换为int型数值
bool trans(string &exp, string &postexp);				//将算术表达式exp转换成后缀表达式postexp
bool compvalue(string &postexp, double &value);			//计算后缀表达式postexp的值

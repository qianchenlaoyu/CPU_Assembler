// MIPS_asm.cpp : 定义控制台应用程序的入口点。
//

/* MIPS汇编器，用于转换汇编语言到机器语言，输出ROM初始化文件 */
/* 只作简单替换操作，支持宏定义，所有指令都从0地址连续排放 */
/* 支持部份指令，更多的指令可以对照添加，指令根据脚本文件生成 */
/* 支持标号，用于产生跳转地址 */






/* 汇编过程 */
/* 判断调用参数，如果为合理参数就进行指定文件汇编，没有要求手动输入参数，不合理参数输出提示 */
/* 根据同目录下的指令脚本文件生成指令表，找不到指令脚本文件输出错误 */
/* 初始化系统变量，包括PC指向0地址，建立符号链表，建立输出缓冲 */
/* 循环单行进行汇编，直到文件结束或遇到错误，其间的空行和注释将被忽略掉 */
/* 输出汇编结果 */

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdlib.h>
#include <vector>
#include <Windows.h>
#include <regex>

using namespace std;




/* 符号表 */
/* 符号表采用动态向量方式存储 */
/* 符号名最长不作限制 */
/* 符号数不作限制，受限于int型变量能表达的最大正整数值 */
typedef struct{
	string name;
	int value;
}symbol_tab_str;

struct{
	int number;
	vector<symbol_tab_str> symbol_vector;
}symbol_tab;



/* 中间结果 */
/* 动态向量方式存储 */
/* 内容包括二进制和符号 */
/* 使用结构体位域,直接合成32位宽的2进制数 */
typedef union{
	struct{
		volatile char32_t func : 6;
		volatile char32_t sa : 5;
		volatile char32_t rd : 5;
		volatile char32_t rt : 5;
		volatile char32_t rs : 5;
		volatile char32_t op : 6;
	};

	struct{
		volatile char32_t immediate : 16;
		volatile char32_t : 16;
	};

	struct{
		volatile char32_t target : 26;
		volatile char32_t : 6;
	};

	volatile char32_t bin;
}bin_str;

struct{
	int count;
	vector<bin_str> bin_data;
}middle_result;


/* 前向引有符号表 */
/* 记录未知的符号引用，在下一轮插补时参考 */
typedef struct{
	string name;
	enum class symbol_tyep{ IMMEDIATE, SKEWING, TARGET };
}unknown_symbol_str;

struct{
	int count;
	vector<unknown_symbol_str> unknown_symbol;
}unknown_symbol_tab;

/* 输出结果 */
/* 输出到源文件同目录下 */
/* 与源文件同名，后缀为.coe */
/* 格式为ROM初始化文件格式 */



bool instruction_compile(void)
{
	string instruction_path;
	ifstream instruction_file_stream;
	char buf[1000];

	GetCurrentDirectoryA(1000, buf);
	instruction_path = buf;
	cout << "当前目录：" << instruction_path << endl;
	instruction_path += "\\instruction.txt";

	cout << "指令脚本：" << instruction_path << endl;

	instruction_file_stream.open(instruction_path, fstream::in);

	if (instruction_file_stream)
		cout << "指令脚本文件打开成功" << endl;
	else
	{
		cout << "指令脚本文件打开失败" << endl;
		return false;
	}
		
	regex r("");
	string ins_str;

	while (instruction_file_stream.getline(buf, 200))
	{
		cout << buf << endl;
	}




	
	

	
	return true;
}

int main(int argc, char *argv[])
{
	string s1;
	string file_path;
	ifstream source_file_stream;

	cout << "当前目录：" << argv[0] << endl;

	if (argc == 1)
	{
		//没有传入文件路径，手动输入
		cout << "输入文件路径：" << endl;
		cin >> file_path;
	}
	else
	{
		file_path = argv[1];
	}

	//指出操作文件
	cout << "文件：" << file_path << endl;

	//判断文件路径合理性
	source_file_stream.open(file_path, ifstream::in);

	if (source_file_stream)
	{
		cout << "打开文件成功。" << endl;
	}
	else
	{
		cout << "没能成功打开文件。" << endl;
		system("PAUSE");
		return 0;
	}

	//根据指令定义脚本生成指令处理方案
	if (instruction_compile() == false)
	{
		cout << "指令脚本错误" << endl;
		system("PAUSE");
	}
	else
	{
		cout << "读取指令脚本成功" << endl;
	}

	system("PAUSE");
}
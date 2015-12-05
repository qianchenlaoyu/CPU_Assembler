// MIPS_asm.cpp : 定义控制台应用程序的入口点。
//

/* MIPS汇编器，用于转换汇编语言到机器语言，输出ROM初始化文件 */
/* 只作简单替换操作，支持宏定义，所有指令都从0地址连续排放 */
/* 支持部份指令，更多的指令可以对照添加，指令根据脚本文件生成 */
/* 支持标号，用于产生跳转地址 */






/* 汇编过程 */
/* 判断输入参数，如果为合理参数就进行指定文件汇编，没有要求手动输入参数，不合理参数输出提示 */
/* 根据同目录下的指令脚本文件生成指令表，找不到指令脚本文件输出错误 */
/* 初始化系统变量，包括PC指向0地址，建立符号链表，建立输出缓冲 */
/* 循环单行进行汇编，直到文件结束或遇到错误，其间的空行和注释将被忽略掉 */
/* 输出汇编结果 */

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;




/* 符号表 */
/* 符号表采用静态方式 */
/* 符号名最长39个字符 */
/* 符号数最多100个 */
struct{
	int number;
	struct{
		char name[40];
		unsigned int value;
	}tab[100];
}symbol_tab;



/* 中间结果 */
/* 静态方式存储 */
/* 内容包括二进制和符号 */
struct{
	int count;
	struct{

	};
}middle_result;


/* 汇编结果 */
/* 静态方式存储 */
/* 内容全为二进制数据 */


/* 输出结果 */
/* 输出到源文件同目录下 */
/* 与源文件同名，后缀为.coe */



int main(int argc, char *argv[])
{
	string s1;

	std::cin >> s1;

}
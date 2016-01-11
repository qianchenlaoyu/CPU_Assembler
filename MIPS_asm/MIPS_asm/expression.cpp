/** @file expression.cpp
  * @brief 实现表达式求值
  *
  *用户输入一个包含“+”、“-”、“*”、“/”、正整数和圆括号的合法算术表达式，计算该表达式的运算结果
  *
  *已实现\n
  *1、多字符运算符支持\n
  *3、位运算支持，“\<\<”、“\>\>”、“&”、“|”、“^”、“~”\n
  *6、十六进制支持，以“0x”开头表示十六进制数\n
  *\n
  *未实现\n
  *2、正负数的直接表示\n
  *4、支持取模运算，“%”\n
  *5、字符输入，以单引号包围的一个字符\n
  */

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
#include "expression.h"

using namespace std;


#define Maxop 13			///<运算符数量
#define MaxSize 50			///<运算符栈的最大长度



/** \brief 运算符优先级结构体*/
struct pri_str{
	string ch;
	int pri;
};


pri_str lpri[] = { { "=", 0 }, { "+", 3 }, { "-", 3 }, { "*", 5 }, { "/", 5 }, { "(", 1 }, { ")", 12 }, { "<<", 10 }, { ">>", 10 }, { "&", 10 }, { "|", 10 }, { "^", 10 }, { "~", 11 } };
pri_str rpri[] = { { "=", 0 }, { "+", 2 }, { "-", 2 }, { "*", 4 }, { "/", 4 }, { "(", 12 }, { ")", 1 }, { "<<", 6 }, { ">>", 6 }, { "&", 6 }, { "|", 6 }, { "^", 6 }, { "~", 11 } };




/*
将二进制串转化为int型
从头扫描，遇到‘#’重新开始
#000001 -> 1
00001 -> 1
*/
unsigned int binary_to_uint(string &s, int &bits)
{
	char32_t value = 0;
	int count = 0;

	if (s.begin() != s.end())
	{
		for (auto index = s.begin(); index != s.end(); index++)
		{

			value <<= 1;
			count++;

			if (*index == '1')
			{
				value += 1;
			}
			else if (*index == '#')
			{
				value = 0;
				count = 0;
			}

		}
	}

	bits = count;
	return value;
}


int leftpri(string &op)//求左运算符op的优先级
{
	int i;
	for (i = 0; i < Maxop; i++)
		if (lpri[i].ch == op)
			return lpri[i].pri;
	return (-1);
}


int rightpri(string &op)//求右运算符op的优先级
{
	int i;
	for (i = 0; i < Maxop; i++)
		if (rpri[i].ch == op)
			return rpri[i].pri;
	return (-1);
}



bool Inop(string &ch)//判断ch是否为运算符,返回逻辑值
{
	if (ch == "=" || ch == "+" || ch == "-" || ch == "*" || ch == "/"
		|| ch == "(" || ch == ")" || ch == "<<" || ch == ">>" || ch == "&"
		|| ch == "|" || ch == "^" || ch == "~")
		return true;
	else
		return false;
}




int Precede(string &op1, string &op2)//op1和o02运算符优先级的比较结果
{
	if (leftpri(op1) == rightpri(op2))
		return 0;
	else if (leftpri(op1) < rightpri(op2))
		return -1;
	else
		return 1;
}





bool trans(string &exp, string &postexp)//将算术表达式exp转换成后缀表达式postexp
{
	vector<string> stack;  //存放运算符
	string s1;
	char ch;

	stack.clear();
	stack.push_back("=");

	int index = 0;

	while (exp[index] != '\0')	//exp表达式未扫描完时循环
	{
		if (exp[index] == ' ')
		{
			//忽略掉空格
			index++;
		}
		else if (exp[index] == '0' && (exp[index + 1] == 'x' || exp[index + 1] == 'X'))
		{
			//十六进制数
			postexp += exp[index++];
			postexp += exp[index++];

			ch = exp[index];
			while ((ch >= '0' && ch <= '9') || ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
			{
				postexp += exp[index++];
				ch = exp[index];
			}
			postexp += '#';
		}
		else if (exp[index] >= '0'&& exp[index] <= '9')		//为数字字符的情况
		{
			while (exp[index] >= '0' && exp[index] <= '9')//判定为数字
			{
				postexp += exp[index++];
			}
			postexp += '#';				//用#标识一个数值串的结束
		}
		else
		{
			//提取出运算符
			s1.clear();
			s1 += exp[index];
			if (!Inop(s1))
			{
				s1 += exp[index + 1];
				if (!Inop(s1))
				{
					//未能获取运算符
					return false;
				}
			}


			switch (Precede(stack.back(), s1))//为运算符的情况
			{
			case -1://如果优先级高于前一个运算符，将运算符入栈
				stack.push_back(s1);
				index += s1.size();
				break;

			case 0://相等的运算符，消去运算符
				stack.pop_back();
				index += s1.size();
				break;

			case 1://如果优先级低于前一个运算符，将前一个运算符写入后缀表达式中
				postexp += stack.back();
				stack.pop_back();
				break;
			}
		}
	}


	while (!stack.empty())
	{
		if (stack.back() != "=")
			postexp += stack.back();
		stack.pop_back();
	}

	return true;
}




bool compvalue(string &postexp, double &value)//计算后缀表达式postexp的值
{
	vector<double> data;		//存储数值
	string op;					//运算符
	char ch;
	double d, a, b, c;
	int index = 0;

	data.clear();

	while (postexp[index] != '\0')		//postexp字符串未扫描完时循环
	{
		if (postexp[index] == '0' && (postexp[index + 1] == 'x' || postexp[index + 1] == 'X'))
		{
			index += 2;
			//读取十六进制数

			d = 0;

			ch = postexp[index];
			while ((ch >= '0' && ch <= '9') || ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
			{
				if (ch >= '0' && ch <= '9')			d = 16 * d + ch - '0';
				else if (ch >= 'a' && ch <= 'f')	d = 16 * d + ch - 'a' + 10;
				else if (ch >= 'A' && ch <= 'F')	d = 16 * d + ch - 'A' + 10;

				index++;
				ch = postexp[index];
			}

			if (ch == '#')
			{
				index++;
			}
			else
				return false;

			//将数字存入栈中
			data.push_back(d);
		}
		else if (postexp[index] >= '0'&& postexp[index] <= '9')
		{
			//读取十进制数
			d = 0;
			while (postexp[index] >= '0' && postexp[index] <= '9')//为数字字符循环
			{
				d = 10 * d + postexp[index] - '0';
				index++;
			}

			if (postexp[index] == '#')
			{
				index++;
			}
			else
			{
				//后缀表达式错误
				return false;
			}

			//将数字存入栈中
			data.push_back(d);
		}
		else
		{
			//读取运算符
			op.clear();
			op += postexp[index++];
			if (!Inop(op))
			{
				op += postexp[index++];
				if (!Inop(op))
				{
					//未能获取运算符
					return false;
				}
			}

			if (!data.empty())
			{
				a = data.back();	//第二操作数
				data.pop_back();
			}
			else
				return false;


			if (op == "~")
			{
				c = ~((unsigned int)a);
				data.push_back(c);
			}
			else
			{
				if (!data.empty())
					b = data.back();	//第一操作数
				else
					return false;

				if (op == "+")				data.back() = b + a;
				else if (op == "-")			data.back() = b - a;
				else if (op == "*")			data.back() = b * a;
				else if (op == "/")
				{
					if (a != 0)
					{
						data.back() = b / a;
					}
					else
					{
						//除零错误
						return false;
					}
				}
				else if (op == "<<")		data.back() = (unsigned int)b << (unsigned int)a;
				else if (op == ">>")		data.back() = (unsigned int)b >> (unsigned int)a;
				else if (op == "&")			data.back() = (unsigned int)b & (unsigned int)a;
				else if (op == "|")			data.back() = (unsigned int)b | (unsigned int)a;
				else if (op == "^")			data.back() = (unsigned int)b ^ (unsigned int)a;
				else
				{
					//非法运算符
					return false;
				}
			}
		}
	}

	if (data.size() == 1)
	{
		value = data.back();
		return true;
	}
	else
	{
		return false;
	}
}






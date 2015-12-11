

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



/*
�������ƴ�ת��Ϊint��
��ͷɨ�裬������#�����¿�ʼ
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









#define Maxop 13			//���������
#define MaxSize 50			//�����ջ����󳤶�



//�û�����һ��������+������-������*������/������������Բ���ŵĺϷ��������ʽ������ñ��ʽ��������


//1�����ַ������֧��
//3��λ����֧�֣���<<������>>������&������|������^������~��
//6��ʮ������֧�֣��ԡ�0x����ͷ��ʾʮ��������


//2����������ֱ�ӱ�ʾ
//4��֧��ȡģ���㣬��%��
//5���ַ����룬�Ե����Ű�Χ��һ���ַ�



//��������ȼ���ѯ��
struct pri_str{
	string ch;
	int pri;
};


pri_str lpri[] = { { "=", 0 }, { "+", 3 }, { "-", 3 }, { "*", 5 }, { "/", 5 }, { "(", 1 }, { ")", 12 }, { "<<", 10 }, { ">>", 10 }, { "&", 10 }, { "|", 10 }, { "^", 10 }, { "~", 11 } };
pri_str rpri[] = { { "=", 0 }, { "+", 2 }, { "-", 2 }, { "*", 4 }, { "/", 4 }, { "(", 12 }, { ")", 1 }, { "<<", 6 }, { ">>", 6 }, { "&", 6 }, { "|", 6 }, { "^", 6 }, { "~", 11 } };





int leftpri(string &op)//���������op�����ȼ�
{
	int i;
	for (i = 0; i < Maxop; i++)
		if (lpri[i].ch == op)
			return lpri[i].pri;
	return (-1);
}


int rightpri(string &op)//���������op�����ȼ�
{
	int i;
	for (i = 0; i < Maxop; i++)
		if (rpri[i].ch == op)
			return rpri[i].pri;
	return (-1);
}



bool Inop(string &ch)//�ж�ch�Ƿ�Ϊ�����,�����߼�ֵ
{
	if (ch == "=" || ch == "+" || ch == "-" || ch == "*" || ch == "/"
		|| ch == "(" || ch == ")" || ch == "<<" || ch == ">>" || ch == "&"
		|| ch == "|" || ch == "^" || ch == "~")
		return true;
	else
		return false;
}




int Precede(string &op1, string &op2)//op1��o02��������ȼ��ıȽϽ��
{
	if (leftpri(op1) == rightpri(op2))
		return 0;
	else if (leftpri(op1) < rightpri(op2))
		return -1;
	else
		return 1;
}





bool trans(string &exp, string &postexp)//���������ʽexpת���ɺ�׺���ʽpostexp
{
	vector<string> stack;  //��������
	string s1;
	char ch;

	stack.clear();
	stack.push_back("=");

	int index = 0;

	while (exp[index] != '\0')	//exp���ʽδɨ����ʱѭ��
	{
		if (exp[index] == ' ')
		{
			//���Ե��ո�
			index++;
		}
		else if (exp[index] == '0' && (exp[index + 1] == 'x' || exp[index + 1] == 'X'))
		{
			//ʮ��������
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
		else if (exp[index] >= '0'&& exp[index] <= '9')		//Ϊ�����ַ������
		{
			while (exp[index] >= '0' && exp[index] <= '9')//�ж�Ϊ����
			{
				postexp += exp[index++];
			}
			postexp += '#';				//��#��ʶһ����ֵ���Ľ���
		}
		else
		{
			//��ȡ�������
			s1.clear();
			s1 += exp[index];
			if (!Inop(s1))
			{
				s1 += exp[index + 1];
				if (!Inop(s1))
				{
					//δ�ܻ�ȡ�����
					return false;
				}
			}


			switch (Precede(stack.back(), s1))//Ϊ����������
			{
			case -1://������ȼ�����ǰһ������������������ջ
				stack.push_back(s1);
				index += s1.size();
				break;

			case 0://��ȵ����������ȥ�����
				stack.pop_back();
				index += s1.size();
				break;

			case 1://������ȼ�����ǰһ�����������ǰһ�������д���׺���ʽ��
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




bool compvalue(string &postexp, double &value)//�����׺���ʽpostexp��ֵ
{
	vector<double> data;		//�洢��ֵ
	string op;					//�����
	char ch;
	double d, a, b, c;
	int index = 0;

	data.clear();

	while (postexp[index] != '\0')		//postexp�ַ���δɨ����ʱѭ��
	{
		if (postexp[index] == '0' && (postexp[index + 1] == 'x' || postexp[index + 1] == 'X'))
		{
			index += 2;
			//��ȡʮ��������

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

			//�����ִ���ջ��
			data.push_back(d);
		}
		else if (postexp[index] >= '0'&& postexp[index] <= '9')
		{
			//��ȡʮ������
			d = 0;
			while (postexp[index] >= '0' && postexp[index] <= '9')//Ϊ�����ַ�ѭ��
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
				//��׺���ʽ����
				return false;
			}

			//�����ִ���ջ��
			data.push_back(d);
		}
		else
		{
			//��ȡ�����
			op.clear();
			op += postexp[index++];
			if (!Inop(op))
			{
				op += postexp[index++];
				if (!Inop(op))
				{
					//δ�ܻ�ȡ�����
					return false;
				}
			}

			if (!data.empty())
			{
				a = data.back();	//�ڶ�������
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
					b = data.back();	//��һ������
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
						//�������
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
					//�Ƿ������
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






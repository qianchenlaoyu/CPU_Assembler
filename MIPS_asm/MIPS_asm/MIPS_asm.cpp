// MIPS_asm.cpp : �������̨Ӧ�ó������ڵ㡣
//

/* MIPS�����������ת��������Ե��������ԣ����ROM��ʼ���ļ� */
/* ֻ�����滻������֧�ֺ궨�壬����ָ���0��ַ�����ŷ� */
/* ֧�ֲ���ָ������ָ����Զ�����ӣ�ָ����ݽű��ļ����� */
/* ֧�ֱ�ţ����ڲ�����ת��ַ */






/* ������ */
/* �жϵ��ò��������Ϊ��������ͽ���ָ���ļ���࣬û��Ҫ���ֶ������������������������ʾ */
/* ����ͬĿ¼�µ�ָ��ű��ļ�����ָ����Ҳ���ָ��ű��ļ�������� */
/* ��ʼ��ϵͳ����������PCָ��0��ַ������������������������� */
/* ѭ�����н��л�ֱ࣬���ļ������������������Ŀ��к�ע�ͽ������Ե� */
/* �������� */

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdlib.h>
#include <vector>

using namespace std;




/* ���ű� */
/* ���ű���ö�̬������ʽ�洢 */
/* ��������������� */
/* �������������ƣ�������int�ͱ����ܱ������������ֵ */
typedef struct{
	string name;
	int value;
}symbol_tab_str;

struct{
	int number;
	vector<symbol_tab_str> symbol_vector;
}symbol_tab;



/* �м��� */
/* ��̬������ʽ�洢 */
/* ���ݰ��������ƺͷ��� */
/* ʹ�ýṹ��λ��,ֱ�Ӻϳ�32λ���2������ */
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


/* ǰ�����з��ű� */
/* ��¼δ֪�ķ������ã�����һ�ֲ岹ʱ�ο� */
typedef struct{
	string name;
	enum class symbol_tyep{ IMMEDIATE, SKEWING, TARGET };
}unknown_symbol_str;

struct{
	int count;
	vector<unknown_symbol_str> unknown_symbol;
}unknown_symbol_tab;

/* ������ */
/* �����Դ�ļ�ͬĿ¼�� */
/* ��Դ�ļ�ͬ������׺Ϊ.coe */
/* ��ʽΪROM��ʼ���ļ���ʽ */


int main(int argc, char *argv[])
{
	string s1;
	string file_path;
	ifstream source_file_stream;

	cout << "��ǰĿ¼��" << argv[0] << endl;

	if (argc == 1)
	{
		//û�д����ļ�·�����ֶ�����
		cout << "�����ļ�·����" << endl;
		cin >> file_path;
	}
	else
	{
		file_path = argv[1];
	}

	//ָ�������ļ�
	cout << "�ļ���" << file_path << endl;

	//�ж��ļ�·��������
	source_file_stream.open(file_path, ifstream::in);

	if (source_file_stream)
	{
		cout << "���ļ��ɹ���" << endl;
	}
	else
	{
		cout << "û�ܳɹ����ļ���" << endl;
		system("PAUSE");
		return 0;
	}



	system("PAUSE");
}
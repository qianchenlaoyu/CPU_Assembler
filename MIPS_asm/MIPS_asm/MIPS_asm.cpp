// MIPS_asm.cpp : �������̨Ӧ�ó������ڵ㡣
//

/* MIPS�����������ת��������Ե��������ԣ����ROM��ʼ���ļ� */
/* ֻ�����滻������֧�ֺ궨�壬����ָ���0��ַ�����ŷ� */
/* ֧�ֲ���ָ������ָ����Զ�����ӣ�ָ����ݽű��ļ����� */
/* ֧�ֱ�ţ����ڲ�����ת��ַ */






/* ������ */
/* �ж�������������Ϊ��������ͽ���ָ���ļ���࣬û��Ҫ���ֶ������������������������ʾ */
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

using namespace std;




/* ���ű� */
/* ���ű���þ�̬��ʽ */
/* �������39���ַ� */
/* ���������100�� */
struct{
	int number;
	struct{
		char name[40];
		unsigned int value;
	}tab[100];
}symbol_tab;



/* �м��� */
/* ��̬��ʽ�洢 */
/* ���ݰ��������ƺͷ��� */
struct{
	int count;
	struct{

	};
}middle_result;


/* ����� */
/* ��̬��ʽ�洢 */
/* ����ȫΪ���������� */


/* ������ */
/* �����Դ�ļ�ͬĿ¼�� */
/* ��Դ�ļ�ͬ������׺Ϊ.coe */



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
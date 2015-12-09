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
#include <Windows.h>
#include <regex>

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


/*
	����ָ��ű��ļ�����ָ�
	ÿ��ָ����Դƥ���ʽ����������ʽ���
	ָ�������ɺ�������Ϣ������ָ�����
*/
typedef struct{
	regex r;
	string source_format;
	string output_format;
}INS_STR;

struct{
	int count;
	vector<INS_STR>  ins_tab;
}ins;

bool instruction_compile(void)
{
	string instruction_path;
	ifstream instruction_file_stream;
	char buf[1000];


	GetCurrentDirectoryA(1000, buf);
	instruction_path = buf;
	cout << "��ǰĿ¼��" << instruction_path << endl;
	instruction_path += "\\instruction.txt";

	cout << "ָ��ű���" << instruction_path << endl;

	instruction_file_stream.open(instruction_path);

	if (instruction_file_stream)
		cout << "ָ��ű��ļ��򿪳ɹ�" << endl;
	else
	{
		cout << "ָ��ű��ļ���ʧ��" << endl;
		return false;
	}
		


	while (instruction_file_stream.getline(buf, 300))
	{
		cout << buf << endl;
	}

	//instruction_file_stream.seekp(0);
	instruction_file_stream.close();
	instruction_file_stream.open(instruction_path);

	regex r;
	smatch result;
	string ins_str;
	string str_temp;

	INS_STR ins_temp;

	ins.count = 0;
	instruction_file_stream.seekg(0);


	while (instruction_file_stream.getline(buf, 300))
	{
		ins_str = "#\\w+\\{\\s*\\{\\.*([^\\}]*)\\};\\s*\\{\\.*([^\\}]*)\\}\\s*\\}";
		try{
			r = ins_str;
		}
		catch (regex_error e){
			cout << e.what() << "\ncode:" << e.code() << endl;
		}

		ins_str = buf;
		if (regex_match(ins_str, result, r))
		{
			cout << result.str(1) << endl;
			cout << result.str(2) << endl;

			ins_temp.source_format = result.str(1);
			ins_temp.output_format = result.str(2);
		
			
			try{
				r = "\\b([a-zA-Z]+)(\\s+([a-zA-Z]+)(?:,(\\w+))?(?:,(\\w+))?)";
			}
			catch (regex_error e){
				cout << e.what() << "\ncode:" << e.code() << endl;
			}

			ins_str = result.str(1);
			if (regex_match(ins_str, result, r))
			{
				str_temp = "\\b";
				str_temp += result[1].str();


				if (result[2].matched)
				{
					str_temp += "\\s+";
					str_temp += "\\w+";


					for (int i = 4; i < 6; i++)
					{
						if (result[i].matched)
						{
							str_temp += "\\s*,\\s*";
							str_temp += "\\w+";
						}
					}
				}

				try{
					ins_temp.r = str_temp;
				}
				catch (regex_error e){
					cout << e.what() << "\ncode:" << e.code() << endl;
				}
				
			}

			ins.ins_tab.push_back(ins_temp);
			ins.count++;
		}
	}


	
	return true;
}

int main(int argc, char *argv[])
{
	string s1;
	string file_path;
	ifstream source_file_stream;

	//cout << "��ǰĿ¼��" << argv[0] << endl;

	//if (argc == 1)
	//{
	//	//û�д����ļ�·�����ֶ�����
	//	cout << "�����ļ�·����" << endl;
	//	cin >> file_path;
	//}
	//else
	//{
	//	file_path = argv[1];
	//}

	////ָ�������ļ�
	//cout << "�ļ���" << file_path << endl;

	////�ж��ļ�·��������
	//source_file_stream.open(file_path, ifstream::in);

	//if (source_file_stream)
	//{
	//	cout << "���ļ��ɹ���" << endl;
	//}
	//else
	//{
	//	cout << "û�ܳɹ����ļ���" << endl;
	//	system("PAUSE");
	//	return 0;
	//}

	//����ָ���ű�����ָ�����
	if (instruction_compile() == false)
	{
		cout << "ָ��ű�����" << endl;
		system("PAUSE");
	}
	else
	{
		cout << "��ȡָ��ű��ɹ�" << endl;
	}

	system("PAUSE");
}
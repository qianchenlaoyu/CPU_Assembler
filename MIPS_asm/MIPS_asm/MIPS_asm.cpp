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
#include <sstream>

using namespace std;




#define MAX_ERROR_NUMBER	5




string work_path;
char buf[1000];

/* ���ű� */
/* ���ű���ö�̬������ʽ�洢 */
/* ��������������� */
/* �������������ƣ�������int�ͱ����ܱ������������ֵ */
struct symbol_str{
	string name;
	int value;
};

struct symbol_tab_str{
	int number;
	vector<symbol_str> symbol_vector;
}symbol_tab;



/* �м��� */
/* ��̬������ʽ�洢 */
/* ���ݰ��������ƺͷ��� */
/* ʹ�ýṹ��λ��,ֱ�Ӻϳ�32λ���2������ */
union bin_str{
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
};

struct middle_result_str{
	int count;
	vector<bin_str> bin_data;
}middle_result;


/* ǰ�����з��ű� */
/* ��¼δ֪�ķ������ã�����һ�ֲ岹ʱ�ο� */
struct unknown_symbol_str{
	string name;
	enum class symbol_tyep{ IMMEDIATE, SKEWING, TARGET };
};

struct unknown_symbol_tab_str{
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
struct INS_STR{
	regex r;
	string source_format;
	string output_format;
};

struct ins_str{
	int count;
	vector<INS_STR>  ins_tab;
}ins;

bool instruction_compile(void)
{
	string instruction_path;
	ifstream instruction_file_stream;

	instruction_path = work_path + "\\instruction.txt";

	cout << "ָ��ű���" << instruction_path << endl;

	instruction_file_stream.open(instruction_path);

	if (instruction_file_stream)
		cout << "ָ��ű��ļ��򿪳ɹ�" << endl;
	else
	{
		cout << "ָ��ű��ļ���ʧ��" << endl;
		return false;
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
				str_temp = "(^\\s*\\b";
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

				str_temp += "\\s*($|//))|(^\\s*($|//))";

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



/*
	ִ�л��
	�����߼�ֵ
	�����Ϣ������ȫ�ֱ�����
*/
struct error_information_str{
	int number;
	vector<string> error;
}error_information;


struct assembly_information_str{
	int size;

}assembly_information;

string source_file_path;

bool assembly_execute(void)
{
	int i;
	int line;
	string s1,s2;
	smatch result;
	ifstream source_file_stream;
	stringstream s_stream;
	bin_str bin_temp;

	auto ins_index = ins.ins_tab.begin();



	//ָ�������ļ�,���ж��ļ�·��������
	cout << "����ļ���" << source_file_path << endl;
	source_file_stream.open(source_file_path, ifstream::in);
	if (source_file_stream)
	{
		cout << "�򿪻���ļ��ɹ���" << endl;
	}
	else
	{
		cout << "�򿪻���ļ�ʧ�ܡ�" << endl;
		return false;
	}

	//��ʼ������
	assembly_information.size = 0;
	error_information.number = 0;
	line = 1;

	while (source_file_stream.getline(buf,500))
	{
		s1 = buf;
		cout << s1 << endl;

		for (ins_index = ins.ins_tab.begin(); ins_index != ins.ins_tab.end(); ins_index++)
		{
			if (regex_search(s1, result, ins_index->r))
			{
				if (result[1].matched)
				{
					//ƥ�䵽һ����䣬���д���
					
					//��֪���ã�ֱ�Ӳ���������
					bin_temp.bin = 2034;
					middle_result.bin_data.push_back(bin_temp);


					//δ֪���ã�������ǰ�����ã�����δ֪���ű�






					assembly_information.size++;
				}
				break;
			}
		}

		//������
		if (ins_index == ins.ins_tab.end())
		{
			s_stream.clear();
			s_stream << line;
			s_stream >> s2;

			s2 = "error: line " + s2 + "    " + s1;
			error_information.error.push_back(s2);
			error_information.number++;

			//�ۼƴ���ﵽ����ֹͣ���
			if (error_information.number == MAX_ERROR_NUMBER)
				return false;
		}
		
		line++;
	}

	//���еڶ���ɨ�裬�滻ǰ�����÷���
	for (auto i = unknown_symbol_tab.unknown_symbol.begin(); i != unknown_symbol_tab.unknown_symbol.end(); i++)
	{



	}


	//�õ���ȫ�Ķ���������������ļ�








	if (error_information.number != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}



int main(int argc, char *argv[])
{


	GetCurrentDirectoryA(1000, buf);
	work_path = buf;

	if (argc == 1)
	{
		////û�д����ļ�·�����ֶ�����
		//cout << "�����ļ�·����" << endl;
		//cin >> file_path;
		source_file_path = work_path + "\\ASM_Test.txt";
	}
	else
	{
		source_file_path = argv[1];
	}



	cout << "��ǰĿ¼��" << work_path << endl;

	//����ָ���ű�����ָ�����
	if (instruction_compile() == false)
	{
		cout << "ָ��ű�����" << endl;
		system("PAUSE");
		return 0;
	}
	else
	{
		cout << "��ȡָ��ű��ɹ���" << "��ɨ�赽" << ins.count << "��ָ��" << endl;
	}

	//�������
	if (assembly_execute())
	{
		cout << "������" << endl;
		cout << "size:" << assembly_information.size << endl;
	}
	else
	{
		cout << "\n\n���ʧ��" << endl;
		cout << "����" << error_information.number << endl;

		for (auto i = error_information.error.begin(); i != error_information.error.end(); i++)
		{
			cout << *i << endl;
		}
	}


	system("PAUSE");
}
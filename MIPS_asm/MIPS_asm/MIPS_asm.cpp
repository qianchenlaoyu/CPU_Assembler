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
#include "expression.h"

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

	struct{
		volatile char32_t instr_index : 26;
		volatile char32_t : 6;
	};

	struct{
		volatile char32_t offset : 16;
		volatile char32_t : 5;
		volatile char32_t base : 5;
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
enum class symbol_type{ RS, RD, RT, SA, IMMEDIATE, TARGET, OFFSET, INSTR_INDEX, BASE, HINT };

struct unknown_symbol_str{
	string name;
	symbol_type symbol_x;
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

vector<string> ins_error_information;

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

	regex r_instruction_format;
	regex r_source_input;
	smatch result;
	string ins_str;
	string str_temp;

	INS_STR ins_temp;

	ins.count = 0;
	instruction_file_stream.seekg(0);


	try{
		r_instruction_format = "^#\\w+\\{\\s*\\{([^\\}]*)\\};\\s*\\{([^\\}]*)\\}\\s*\\}$";
	}
	catch (regex_error e){
		cout << e.what() << "\ncode:" << e.code() << endl;
	}

	try{
		r_source_input = "^\\b([a-zA-Z]+)(?:\\s+(\\w+)(?:,(\\w+))?(?:,(\\w+))?)?$";
	}
	catch (regex_error e){
		cout << e.what() << "\ncode:" << e.code() << endl;
	}

	while (instruction_file_stream.getline(buf, 300))
	{
		//������ָ���ʽ������ƥ��
		ins_str = buf;
		if (regex_match(ins_str, result, r_instruction_format))
		{
			ins_temp.source_format = result.str(1);
			ins_temp.output_format = result.str(2);

			//��һ����Դ�����ʽ������ƥ��
			ins_str = result.str(1);
			if (regex_match(ins_str, result, r_source_input))
			{
				str_temp = "(?:^\\s+\\b";
				str_temp += "(" + result[1].str() + ")";	//��һ�飬ָ����

				if (result[2].matched)
				{
					str_temp += "(?:";						//���ִ������
					str_temp += "\\s+";
					str_temp += "(\\w+)";					//�ڶ���

					if (result[3].matched)
					{
						str_temp += "(?:\\s*,\\s*(\\w+))?";			//������
					}

					if (result[4].matched)
					{
						str_temp += "(?:\\s*,\\s*(\\w+))?";			//������
					}

					str_temp += ")?";
				}

				str_temp += "\\s*(?:$|//))|(?:^\\s*(?:$|//))";		//����ע�ͺͿ���

				try{
					ins_temp.r = str_temp;
				}
				catch (regex_error e){
					cout << e.what() << "\ncode:" << e.code() << endl;
				}

				ins.ins_tab.push_back(ins_temp);
				ins.count++;
			}
			else{
				//��¼δʶ��ָ��
				ins_error_information.push_back(buf);
			}
		}
		else{
			//��¼δʶ��ָ��
			ins_error_information.push_back(buf);
		}
	}

	return true;
}



/*
	������ļ�
*/
bool output_to_file(void)
{
	ofstream output_file_stream;
	string output_file_path;
	string str_temp;
	char32_t temp;
	int j;

	output_file_path = work_path + "\\output.txt";
	
	output_file_stream.open(output_file_path);

	if (output_file_stream)
	{


	}
	else
	{
		cout << "�����ļ�ʧ��" << endl;
		return false;
	}

	//����ļ�ͷ
	output_file_stream << "memory_initialization_radix=2;" << endl;
	output_file_stream << "memory_initialization_vector=" << endl;


	//ѭ�������������
	for (auto i = middle_result.bin_data.begin(); i != middle_result.bin_data.end(); )
	{
		temp = i->bin;
		str_temp = "";

		for (j = 0; j < 32; j++)
		{
			if (temp & 0x80000000)
			{
				str_temp += "1";
			}
			else
			{
				str_temp += "0";
			}

			temp <<= 1;

			if (j == 5)
				str_temp += "    ";
			if (j == 10)
				str_temp += "    ";
			if (j == 15)
				str_temp += "    ";
			if (j == 20)
				str_temp += "    ";
		}

		i++;
		if (i != middle_result.bin_data.end())
			output_file_stream << str_temp << "," << endl;
		else
			output_file_stream << str_temp << ";" << endl;
	}


	//�ر��ļ�
	output_file_stream.close();

	return true;
}




/*
	���ʽ��ֵ
	�����߼�ֵ
*/
bool evaluation(string &exp, char32_t &value)
{
	//���ȶԱ��ʽ�еķ��Ž��н�����
	//Ȼ���Գ������ʽ����ʽ���м���
	//���ܽ����ã�����ʧ��
	//���ʽ������󣬷���ʧ��


	string postexp;
	double value_temp;

	if (!trans(exp, postexp))
	{
		return false;
	}

	if (!compvalue(postexp, value_temp))
	{
		return false;
	}

	value = (char32_t)value_temp;
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

struct source_input_str{
	unsigned int op;
	unsigned int rs;
	unsigned int rd;
	unsigned int rt;
	unsigned int sa;
	unsigned int immediate;
	unsigned int target;
	unsigned int offset;
	unsigned int tab;
}source_input;

struct bin_output_str{
	unsigned int op;
	unsigned int rs;
	unsigned int rd;
	unsigned int rt;
	unsigned int sa;
	unsigned int immediate;
	unsigned int target;
	unsigned int offset;
	unsigned int instr_index;
	unsigned int base;
	unsigned int hint;
	unsigned int func;
	unsigned int tab;
}bin_output;





bool assembly_execute(void)
{
	int i,j;
	int line;
	string s1,s2,s3;
	string str_reg;
	smatch result;
	ifstream source_file_stream;
	stringstream s_stream;
	bin_str bin_temp;
	regex reg_format("^\\bR((?:[012]?\\d)|(?:3[01]))$",regex::icase);
	regex r_source("^\\b([a-zA-Z]+)(?:\\s+(\\w+)(?:,(\\w+))?(?:,(\\w+))?)?$");
	regex r_output("^(#[01]{6})\\s+((?:\\w+)|(?:#[01]{5}))(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+(#[01]{6}))?$");
	smatch result_reg;
	smatch result_source;
	smatch result_output;
	int bit_count;
	int bits;
	char32_t value_temp;

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


	//ȡһ���ı�
	while (source_file_stream.getline(buf,500))
	{
		s1 = buf;
		for (ins_index = ins.ins_tab.begin(); ins_index != ins.ins_tab.end(); ins_index++)
		{
			if (regex_search(s1, result, ins_index->r))
			{
				//ƥ�䵽һ����䣬���д���

				//�ֱ��Դ�����ʽ���ͻ��������ƥ�䣬��ȡ������ֵ
				//���ÿһ��λ���ֵ
				//��֪���ã�ֱ�Ӳ���������
				//δ֪���ã�������ǰ�����ã�����δ֪���ű�

				
				if (result[1].matched)
				{
					regex_match(ins_index->source_format, result_source, r_source);
					
					//�������ÿ�����Ž��з�����ת��Ϊ��ֵ
					for (i = 2; i < 5; i++)
					{
						if (result[i].matched)
						{
							//�ж��Ƿ�Ϊ�Ĵ���
							s2 = result[i].str();
							if (regex_match(s2, result_reg, reg_format))
							{
								s3 = result_reg[1].str();

								//�ǼĴ�������һ���жϺ�����
								s2 = result_source[i].str();
								if ((s2 == "RS") || (s2 == "RT") || (s2 == "RD"))
								{
									//����Ĵ������ʹ�ã�ת��Ϊ�Ĵ������
									s_stream.clear();
									s_stream << s3;
									s_stream >> j;

									if (s2 == "RS")
										bin_output.rs = j;
									else if (s2 == "RT")
										bin_output.rt = j;
									else if (s2 == "RD")
										bin_output.rd = j;

								}
								else
								{
									//��������
									s_stream.clear();
									s_stream << line;
									s_stream >> s2;

									s2 = "error: line " + s2 ;
									s2 += "    �÷�����   " + s1;
									error_information.error.push_back(s2);
									error_information.number++;

									//�ۼƴ���ﵽ����ֹͣ���
									if (error_information.number == MAX_ERROR_NUMBER)
										return false;
								}

							}
							else
							{
								//���ǼĴ���
								//����Ӧ�õõ�һ������ֵ
								//����޷������ֵ�����˴μٶ�Ϊ��ǰ������
								//�޷�����ı��ʽ��������δ֪���ñ���

								if (evaluation(s2, value_temp))
								{
									//��ֱ�ӽ���
									

									if (result_source.str(i) == "immediate")
									{
										bin_output.immediate = value_temp;
									}
									else if (result_source.str(i) == "SA")
									{

									}
									else if (result_source.str(i) == "offset")
									{

									}
									else if (result_source.str(i) == "instr_index")
									{

									}
									else if (result_source.str(i) == "base")
									{

									}
									else if (result_source.str(i) == "hint")
									{

									}


								}
								else
								{
									//���ܽ��㣬����ǰ������

								}

							}

						}
						else
						{
							//û�п��õ�ƥ����˳�ѭ��
							break;
						}
					}


					//���������ʽ����λ���
					regex_match(ins_index->output_format, result_output, r_output);
					bit_count = 0;
					bin_temp.bin = 0;

					for (i = 1; i < 7; i++)
					{
						if (result_output[i].matched)
						{
							s2 = result_output[i].str();
							//���ֿ�ֱ�����Ͳ���
							if (s2[0] == '#')
							{
								//ֱ�������������ƴ�ת��Ϊint�ͣ��������м���
								value_temp = binary_to_uint(result_output[i].str(), bits);
								bit_count += bits;

								bin_temp.bin |= value_temp << (32 -bit_count);
							}
							else
							{
								//�ж��Ƿ��ǼĴ�������
								if ((s2 == "RS") || (s2 == "RT") || (s2 == "RD"))
								{
									if (s2 == "RS"){
										bin_temp.rs = bin_output.rs;
									}
									else if (s2 == "RT"){
										bin_temp.rt = bin_output.rt;
									}
									else if (s2 == "RD"){
										bin_temp.rd = bin_output.rd;
									}
									
									bit_count += 5;
								}
								else
								{
									if (s2 == "immediate")
									{
										bin_temp.immediate = bin_output.immediate;
										bit_count += 16;
									}
									else if (s2 == "SA")
									{

									}
									else if (s2 == "offset")
									{

									}
									else if (s2 == "instr_index")
									{

									}
									else if (s2 == "base")
									{

									}
									else if (s2 == "hint")
									{

									}

								}
							}
						}
						else
						{
							//û�п��õ�ƥ����˳�ѭ��
							break;
						}
					}
					
					//������
					middle_result.bin_data.push_back(bin_temp);
					middle_result.count++;
					assembly_information.size++;
				}
				else			
				{
					//����
				}

				//���䵽һ����䣬����ɴ���������һѭ��
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
		
		//�кż���
		line++;
	}



	//���еڶ���ɨ�裬�滻ǰ�����÷���
	for (auto i = unknown_symbol_tab.unknown_symbol.begin(); i != unknown_symbol_tab.unknown_symbol.end(); i++)
	{



	}


	//�õ���ȫ�Ķ���������������ļ�
	if (output_to_file())
	{
		cout << "����ļ��ɹ�" << endl;
	}
	else
	{
		cout << "����ļ�ʧ��" << endl;
	}







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
		cout << "δʶ��ָ�" << endl;
		for (auto i = ins_error_information.begin(); i != ins_error_information.end(); i++)
		{
			cout << *i << endl;
		}


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
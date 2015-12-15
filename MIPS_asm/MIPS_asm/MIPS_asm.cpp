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
#include <iomanip>
#include <math.h>

#include "expression.h"

using namespace std;
//using namespace regex;



#define MAX_ERROR_NUMBER	5
#define MAX_OFFSET_BACK		-32768
#define MAX_OFFSET_FRONT	32767
#define MAX_TARGET_BACK		-33554432
#define MAX_TARGET_FRONT	33554431



string work_path;
char buf[1000];
int address_count;

enum class symbol_type{ RS, RD, RT, SA, IMMEDIATE, TARGET, OFFSET, X_OFFSET, INSTR_INDEX, BASE, HINT, DEFINE, LABEL, STATEMENT};

/* ���ű� */
/* ���ű���ö�̬������ʽ�洢 */
/* ��������������� */
/* �������������ƣ�������int�ͱ����ܱ������������ֵ */
struct symbol_str{
	string name;
	int value;
	string symbol_string;
	symbol_type symbol_x;
};

vector<symbol_str> symbol_tab;



/* ǰ�����з��ű� */
/* ��¼δ֪�ķ������ã�����һ�ֲ岹ʱ�ο� */

struct INS_STR;

struct unknown_symbol_str{
	string name;
	symbol_type symbol_x;
	vector<INS_STR>::iterator ins_index;
	int line;
	int position;
};

vector<unknown_symbol_str> unknown_symbol_tab;


/*
	����ű���ӷ���
	������ű�������Ӧ�ķ��ţ����ش���
*/
bool symbol_add(symbol_str &ss)
{
	for (auto i = symbol_tab.begin(); i != symbol_tab.end(); i++)
	{
		if (i->name == ss.name)
		{
			return false;
		}
	}

	symbol_tab.push_back(ss);

	return true;
}






/* �м��� */
/* ��̬������ʽ�洢 */
/* ���ݰ��������ƺͷ��� */
/* ʹ�ýṹ��λ��,ֱ�Ӻϳ�32λ���2������ */
struct middle_result_str{
	int line;
	int address;
	union{
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
		volatile char arr[4];
	}bin_str;
};

struct middle_compile_information_str{
	int line;
	string source_string;
	int address;
	int bin_mask;
};


vector<middle_result_str> middle_result;
vector<middle_compile_information_str> middle_compile_information;


regex r_null_line("^\\s*$");													//ƥ�����
regex r_comment("^(.*?)((\\s*//.*)|(\\s*))$");									//ƥ��ע��
regex r_define("^\\s*#define\\s+([a-zA-Z]\\w*)\\s+(.*?)$");						//ƥ�䶨��
regex r_label("^([a-zA-Z]\\w*):$");												//ƥ���ǩ
regex r_pseudoinstruction_ds("^\\s+DS\\s+([^,\t\n]+?)\\s*$");					//ƥ��αָ��
regex r_pseudoinstruction_db("^\\s+DB\\s+(.+?)\\s*$");
regex r_pseudoinstruction_dw("^\\s+DW\\s+(.+?)\\s*$");
regex r_scan_exp("(?:,?\\s*)([^,\\t\\n]+)\\s*");								//�����������ʽ

regex r_instruction_format("^#\\w+\\{\\s*\\{([^\\}]*)\\};\\s*\\{([^\\}]*)\\}\\s*\\}$");							//ƥ��ָ���
regex r_instruction_source("^\\b([a-zA-Z]+)(?:\\s+(\\w+)(?:,(\\w+))?(?:(?:(?:,)|(?:@))(\\w+))?)?$");			//ƥ���������
regex r_instruction_output("^(#[01]{6})\\s+((?:\\w+)|(?:#[01]{5}))(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+(#[01]{6}))?$");

regex r_reg_format("^\\bR((?:[012]?\\d)|(?:3[01]))$");							//ƥ��Ĵ���
regex r_scan_symbol("(?:(?:<<)|(?:>>)|(?:[-+*/\\(\\)&|~^]+))|(?:(?:0x[0-9a-fA-F]+)|(?:\\d+))|([a-zA-Z]\\w*)");		//ƥ����ʽ�еķ���


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

vector<INS_STR>  ins;
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


	smatch result;
	string ins_str;
	string str_temp;

	INS_STR ins_temp;

	instruction_file_stream.seekg(0);

	while (instruction_file_stream.getline(buf, 300))
	{
		//������ָ���ʽ������ƥ��
		str_temp = buf;

		//ȥ������ע��
		ins_str = regex_replace(str_temp, r_comment, "$1");

		if (regex_match(ins_str, result, r_null_line))
		{
			//����
		}
		else if (regex_match(ins_str, result, r_instruction_format))
		{
			ins_temp.source_format = result.str(1);
			ins_temp.output_format = result.str(2);

			//��һ����Դ�����ʽ������ƥ��
			ins_str = result.str(1);
			if (regex_match(ins_str, result, r_instruction_source))
			{
				str_temp = "(?:^\\s+\\b";
				str_temp += "(" + result[1].str() + ")";	//��һ�飬ָ����

				if (result[2].matched)
				{
					str_temp += "(?:";						//���ִ������
					str_temp += "\\s+";
					str_temp += "([^,]+?)";					//�ڶ���

					if (result[3].matched)
					{
						str_temp += "(?:\\s*,\\s*([^,]+?))?";			//������
					}

					if (result[4].matched)
					{
						str_temp += "(?:\\s*(?:(?:,)|(?:@))\\s*([^,]+?))?";			//������
					}

					str_temp += ")?";
				}

				str_temp += "$)";

				try{
					ins_temp.r = str_temp;
				}
				catch (regex_error e){
					cout << e.what() << "\ncode:" << e.code() << endl;
				}

				ins.push_back(ins_temp);
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



//��char32_tת��Ϊ�����Ʊ�ʽ���ַ���
void convert_char32_t_string(string &s, char32_t d32, int mode)
{
	int j;

	s.clear();

	for (j = 0; j < 32; j++)
	{
		if (d32 & 0x80000000)
		{
			s += "1";
		}
		else
		{
			s += "0";
		}

		d32 <<= 1;

		if (mode)
		{
			if (j == 5)
				s += "  ";
			if (j == 10)
				s += "  ";
			if (j == 15)
				s += "  ";
			if (j == 20)
				s += "  ";
		}

	}
}


/*
	������ļ�
	path ���·��
*/
bool output_bin_file(string path)
{
	ofstream output_file_stream;
	string str_temp;

	output_file_stream.open(path);

	if (!output_file_stream)
		return false;

	//����ļ�ͷ
	output_file_stream << "memory_initialization_radix=2;" << endl;
	output_file_stream << "memory_initialization_vector=" << endl;


	//ѭ�������������
	for (auto i = middle_result.begin(); i != middle_result.end(); )
	{
		convert_char32_t_string(str_temp, i->bin_str.bin, 0);

		i++;
		if (i != middle_result.end())
			output_file_stream << str_temp << "," << endl;
		else
			output_file_stream << str_temp << ";" << endl;
	}

	//�ر��ļ�
	output_file_stream.close();
	return true;
}


/*
	��������Ϣ	
*/
bool output_compile_information_file(string path)
{
	ofstream output_file_stream;
	string str_temp;
	ostringstream os_stream;

	output_file_stream.open(path);

	if (!output_file_stream)
		return false;

	auto middle_result_index = middle_result.begin();

	//ѭ�����
	for (auto i = middle_compile_information.begin(); i != middle_compile_information.end(); i++)
	{
		os_stream.str("");

		if (i->bin_mask)
		{
			convert_char32_t_string(str_temp,middle_result_index->bin_str.bin, 1);
			os_stream << setw(45) << setiosflags(ios_base::left) << str_temp << resetiosflags(ios_base::left);
			os_stream << setw(8) << hex << i->address;
			middle_result_index++;
		}
		else
		{
			str_temp.clear();
			os_stream << setw(45) << str_temp << setw(8) << str_temp;
		}

		output_file_stream << os_stream.str()
						<< setw(6) << i->line << setw(4) << "" << i->source_string << endl;
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

	string s1, s2, s3;
	string postexp;
	stringstream s_stream;
	smatch result;

	double value_temp;
	int replace_count;
	int scan_count;

	s2 = exp;
	do{
		scan_count = 0;
		replace_count = 0;
		s3 = s2;
		s2.clear();
		for (sregex_iterator it(s3.begin(), s3.end(), r_scan_symbol), end_it; it != end_it; it++)
		{
			if ((*it)[1].matched)
			{
				//ɨ�赽һ������
				scan_count++;
				s1 = it->str(1);

				//�ж��Ƿ�Ϊ�Ĵ���
				if (regex_match(s1, result, r_reg_format))
				{
					//�ǼĴ���,�ԼĴ����ź��滻
					s1 = result.str(1);
					replace_count++;
				}
				else
				{
					for (auto index = symbol_tab.begin(); index != symbol_tab.end(); index++)
					{
						if (s1 == index->name)
						{
							if (index->symbol_x == symbol_type::DEFINE)
							{
								//�궨�壬�ı��滻
								s1 = index->symbol_string;
								replace_count++;
								break;
							}
							else if (index->symbol_x == symbol_type::LABEL)
							{
								//��ǩ����ֵת�ı����滻
								s_stream.clear();
								s_stream << index->value;
								s_stream >> s1;
								replace_count++;
								break;
							}
						}
					}
				}

				s2 += s1;
			}
			else
			{
				s1 = it->str();
				s2 += s1;
			}
		}
	} while (replace_count);	//replace_count != 0,���ܻ���Ƕ�׵ķ��ţ���������ɨ����滻



	//��ʱӦ�����еķ��Ŷ����滻���ˣ���Ȼ���Ǵ����
	if (scan_count != 0)
	{
		//����δʶ��ķ��ţ����ش���
		return false;
	}

	if (!trans(s2, postexp))
	{
		return false;
	}

	if (!compvalue(postexp, value_temp))
	{
		return false;
	}

	//��ֵ��ɣ����ر��ʽ��ֵ
	value = (char32_t)value_temp;
	return true;
}





/*
	ִ�л��
	�����߼�ֵ
	�����Ϣ������ȫ�ֱ�����
*/
vector<string> error_information;
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


/*
	����һ�����
	position: -1��ʾ��β�����Ԫ�أ�>=0��ʾ���±�洢
	error_code���ڷ��ش������
	1���÷�����
	2���޷�����
	3��ƫ�������
	4��ָ��δ֧�ֻ�����
*/
bool one_statement(string &source_string,vector<INS_STR>::iterator &ins_index, int position, int &error_code)
{
	smatch result_reg;
	smatch result_source_string;
	smatch result_source_format;
	smatch result_output_format;	

	symbol_type symbol_x_temp;
	stringstream s_stream;
	char32_t value_temp;
	middle_result_str middle_result_temp;

	string s1, s2;
	int bit_count;
	int bits;
	int i;

	signed int offset;
	signed int target;

	regex_match(source_string, result_source_string,ins_index->r);
	regex_match(ins_index->source_format, result_source_format, r_instruction_source);

	//�������ÿ�����Ž��з�����ת��Ϊ��ֵ
	for (i = 2; i < 5; i++)
	{
		if (result_source_format[i].matched)
		{
			//��ȡ��ǰ��������
			s2 = result_source_format.str(i);
			if (s2 == "immediate")				symbol_x_temp = symbol_type::IMMEDIATE;
			else if (s2 == "SA")				symbol_x_temp = symbol_type::SA;
			else if (s2 == "offset")			symbol_x_temp = symbol_type::OFFSET;
			else if (s2 == "x_offset")			symbol_x_temp = symbol_type::X_OFFSET;
			else if (s2 == "target")			symbol_x_temp = symbol_type::TARGET;
			else if (s2 == "instr_index")		symbol_x_temp = symbol_type::INSTR_INDEX;
			else if (s2 == "base")				symbol_x_temp = symbol_type::BASE;
			else if (s2 == "hint")				symbol_x_temp = symbol_type::HINT;
			else if (s2 == "RS")				symbol_x_temp = symbol_type::RS;
			else if (s2 == "RT")				symbol_x_temp = symbol_type::RT;
			else if (s2 == "RD")				symbol_x_temp = symbol_type::RD;

			//�����ּĴ�������������
			//����Ӧ�õõ�һ������ֵ
			//����޷������ֵ��������ʧ��
			s1 = result_source_string[i].str();
			if (evaluation(s1, value_temp))
			{
				//��ֱ�ӽ���
				switch (symbol_x_temp)
				{
				case symbol_type::IMMEDIATE:		bin_output.immediate = value_temp;	break;
				case symbol_type::SA:				bin_output.sa = value_temp;			break;
				case symbol_type::OFFSET:
				{
					//ƫ���� = Ŀ���ַ - ��ǰPC
					offset = value_temp - position;
					offset /= 4;

					if (offset<MAX_OFFSET_BACK || offset>MAX_OFFSET_FRONT)
					{
						//�������3��ƫ�������
						error_code = 3;
						return false;
					}

					//ת��Ϊ��Ӧ�����16λ������
					bin_output.offset = 65536 + offset;
				}break;
				case symbol_type::X_OFFSET:
				{
					//����ڼĴ���ƫ��
					offset = value_temp;
					offset /= 4;

					if (offset<MAX_OFFSET_BACK || offset>MAX_OFFSET_FRONT)
					{
						//�������3��ƫ�������
						error_code = 3;
						return false;
					}

					//ת��Ϊ��Ӧ�����16λ������
					bin_output.offset = 65536 + offset;
				}break;
				case symbol_type::TARGET:
				{
					//26λƫ����,�ڵ�ǰָ�����256MB�ķ�Χ����ת
					//ƫ���� = Ŀ���ַ - ��ǰPC
					//���� = 26λģ + ƫ����
					target = value_temp - position;
					target /= 4;

					if (target<MAX_TARGET_BACK || target>MAX_TARGET_FRONT)
					{
						//�������3��ƫ�������
						error_code = 3;
						return false;
					}

					//ת��Ϊ��Ӧ�����26λ������
					bin_output.target = 67108864 + target;
				}break;
				case symbol_type::INSTR_INDEX:break;
				case symbol_type::BASE:				bin_output.base = value_temp;		break;
				case symbol_type::HINT:break;
				case symbol_type::RS:				bin_output.rs = value_temp;			break;
				case symbol_type::RT:				bin_output.rt = value_temp;			break;
				case symbol_type::RD:				bin_output.rd = value_temp;			break;
				}
			}
			else
			{
				//���ش�����룬2���޷�����
				error_code = 2;
				return false;
			}	
		}
		else
		{
			//û�п��õ�ƥ�������ѭ��
			break;
		}
	}

	//�������
	//���������ʽ����λ���
	regex_match(ins_index->output_format, result_output_format, r_instruction_output);

	bit_count = 0;
	middle_result_temp.bin_str.bin = 0;

	for (i = 1; i < 7; i++)
	{
		if (result_output_format[i].matched)
		{
			//��ȡ��ǰ��������
			s2 = result_output_format[i].str();
			if (s2 == "immediate")				symbol_x_temp = symbol_type::IMMEDIATE;
			else if (s2 == "SA")				symbol_x_temp = symbol_type::SA;
			else if (s2 == "offset")			symbol_x_temp = symbol_type::OFFSET;
			else if (s2 == "target")			symbol_x_temp = symbol_type::TARGET;
			else if (s2 == "instr_index")		symbol_x_temp = symbol_type::INSTR_INDEX;
			else if (s2 == "base")				symbol_x_temp = symbol_type::BASE;
			else if (s2 == "hint")				symbol_x_temp = symbol_type::HINT;
			else if (s2 == "RS")				symbol_x_temp = symbol_type::RS;
			else if (s2 == "RT")				symbol_x_temp = symbol_type::RT;
			else if (s2 == "RD")				symbol_x_temp = symbol_type::RD;


			//���ֿ�ֱ�����Ͳ���
			if (s2[0] == '#')
			{
				//ֱ�������������ƴ�ת��Ϊint�ͣ��������м���
				value_temp = binary_to_uint(result_output_format[i].str(), bits);
				bit_count += bits;

				middle_result_temp.bin_str.bin |= value_temp << (32 - bit_count);
			}
			else
			{
				switch (symbol_x_temp)
				{
				case symbol_type::RS:			middle_result_temp.bin_str.rs = bin_output.rs;					bit_count += 5;		break;
				case symbol_type::RT:			middle_result_temp.bin_str.rt = bin_output.rt;					bit_count += 5;		break;
				case symbol_type::RD:			middle_result_temp.bin_str.rd = bin_output.rd;					bit_count += 5;		break;
				case symbol_type::IMMEDIATE:	middle_result_temp.bin_str.immediate = bin_output.immediate;	bit_count += 16;	break;
				case symbol_type::SA:			middle_result_temp.bin_str.sa = bin_output.sa;					bit_count += 5;		break;
				case symbol_type::OFFSET:		middle_result_temp.bin_str.offset = bin_output.offset;			bit_count += 16;	break;
				case symbol_type::TARGET:break;
				case symbol_type::INSTR_INDEX:	middle_result_temp.bin_str.target = bin_output.target;			bit_count += 26;	break;
				case symbol_type::BASE:			middle_result_temp.bin_str.base = bin_output.base;				bit_count += 5;		break;
				case symbol_type::HINT:break;
				default:break;
				}
			}
		}
		else
		{
			//û�п��õ�ƥ�������ѭ��
			break;
		}
	}

	//�˲�����ƽ��
	if (bit_count != 32)
	{
		//�������4��ָ��δ֧�ֻ�����
		error_code = 4;
		return false;
	}

	//������
	middle_result[position/4] = middle_result_temp;

	return true;
}



//��Ӵ����¼
void add_error_information(int line, int error_code, string &s)
{
	stringstream s_stream;
	string s_temp;
	s_stream.clear();
	s_stream << line;
	s_stream >> s_temp;
	s_temp = "error: line " + s_temp + "     ";

	switch (error_code)
	{
	case 1:		s_temp += "�÷�����";				break;
	case 2:		s_temp += "�޷�����";				break;
	case 3:		s_temp += "ƫ������� ";			break;
	case 4:		s_temp += "ָ��δ֧�ֻ����� ";	break;
	case 5:		s_temp += "�������ظ�";				break;
	}

	s_temp += s;
	error_information.push_back(s_temp);
}



bool assembly_execute(void)
{
	int line;
	int error_code;
	int i;

	char32_t value_temp;

	string s1,s2;
	string source_one_line;
	smatch result;
	ifstream source_file_stream;
	stringstream s_stream;
	
	smatch result_null_line;
	smatch result_label;
	smatch result_define;

	unknown_symbol_str unknown_symbol_temp;
	symbol_str	symbol_temp;
	middle_compile_information_str middle_compile_information_temp;
	middle_result_str middle_result_temp;

	auto ins_index = ins.begin();


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
	line = 1;
	address_count = 0;

	//ȡһ���ı�
	while (source_file_stream.getline(buf,500))
	{
		s1 = buf;

		//��¼�����Ϣ
		middle_compile_information_temp.line = line;
		middle_compile_information_temp.source_string = s1;
		middle_compile_information_temp.bin_mask = 0;

		//ȥ������ע��
		source_one_line = regex_replace(s1, r_comment, "$1");

		if (regex_match(source_one_line, result_null_line, r_null_line))
		{
			//����
		}
		else if (regex_match(source_one_line, result_define, r_define))
		{
			//�ж��Ǻ궨��
			symbol_temp.name = result_define.str(1);
			symbol_temp.symbol_string = result_define.str(2);
			symbol_temp.symbol_x = symbol_type::DEFINE;

			if (!symbol_add(symbol_temp))
			{
				//�������ظ�
				s2 = "�������ظ���" + symbol_temp.name;
				add_error_information(line, 5, s2);
			}
		}
		else if (regex_match(source_one_line, result_label, r_label))
		{
			//�ж��Ǳ�ǩ
			symbol_temp.name = result_label.str(1);
			symbol_temp.symbol_x = symbol_type::LABEL;
			symbol_temp.value = address_count;

			if (!symbol_add(symbol_temp))
			{
				//�������ظ�
				s2 = "�������ظ���" + symbol_temp.name;
				add_error_information(line, 5, s2);
			}
		}
		else if (regex_match(source_one_line, result, r_pseudoinstruction_ds))
		{
			//����һƬ�洢����
			if (evaluation(result.str(1), value_temp))
			{
				middle_result_temp.bin_str.bin = 0;
				value_temp = (char32_t)ceil(value_temp/4.0);			//����ȡ�������ֶ���
				
				while (value_temp--)
				{
					middle_result.push_back(middle_result_temp);
					address_count += 4;									//��ַ����
				}
			}
			else
			{
				//�������
				add_error_information(line, 0, s1);
			}
		}
		else if (regex_match(source_one_line, result, r_pseudoinstruction_db))
		{
			//���ֽ�Ϊ��λ��ʼ���洢��
			s2 = result.str(1);
			i = 0;

			for (sregex_iterator it(s2.begin(), s2.end(), r_scan_exp), end_it; it != end_it; it++)
			{
				if (evaluation((*it).str(1), value_temp))
				{
					middle_result_temp.bin_str.arr[i++] = value_temp;
					if (i == 4)
					{
						middle_result.push_back(middle_result_temp);
						address_count += 4;
						i = 0;
					}
				}
				else
				{
					//����
					add_error_information(line, 0, s1);
					break;
				}
			}
			
			//����
			if (i != 0)
			{
				middle_result.push_back(middle_result_temp);
				address_count += 4;
			}
		}
		else if (regex_match(source_one_line, result, r_pseudoinstruction_dw))
		{
			//����Ϊ��λ��ʼ���洢��
			s2 = result.str(1);
			for (sregex_iterator it(s2.begin(), s2.end(), r_scan_exp), end_it; it != end_it; it++)
			{
				if (evaluation((*it).str(1), value_temp))
				{
					middle_result_temp.bin_str.bin = value_temp;
					middle_result.push_back(middle_result_temp);
					address_count += 4;
				}
				else
				{
					//����
					add_error_information(line, 0, s1);
					break;
				}
			}


		}
		else
		{
			//�ֱ��Դ�����ʽ���ͻ��������ƥ�䣬��ȡ������ֵ
			for (ins_index = ins.begin(); ins_index != ins.end(); ins_index++)
			{
				if (regex_match(source_one_line, result, ins_index->r))
				{
					//ƥ�䵽һ����䣬���д���

					//���һ��ָ��Ŀռ�
					middle_result.push_back(middle_result_temp);
	
					if (one_statement(source_one_line, ins_index, address_count, error_code))
					{
						//����ɹ�
					}
					else
					{
						//����ʧ�ܣ�����δ֪���ű�
						unknown_symbol_temp.name = source_one_line;
						unknown_symbol_temp.position = address_count;
						unknown_symbol_temp.symbol_x = symbol_type::STATEMENT;
						unknown_symbol_temp.ins_index = ins_index;
						unknown_symbol_temp.line = line;
						unknown_symbol_tab.push_back(unknown_symbol_temp);
					}

					middle_compile_information_temp.bin_mask = 1;
					middle_compile_information_temp.address = address_count;

					address_count += 4;			//��ַ����

					//���䵽һ����䣬����ɴ�������ѭ��
					break;
				}
			}

			//������
			if (ins_index == ins.end())
			{
				add_error_information(line, 0, s1);
			}
		}
	
		//�кż���
		line++;

		//�洢�����Ϣ
		middle_compile_information.push_back(middle_compile_information_temp);
	}

	//�ر��ļ�
	source_file_stream.close();

	//���еڶ���ɨ�裬�滻ǰ�����÷���
	for (auto i = unknown_symbol_tab.begin(); i != unknown_symbol_tab.end(); i++)
	{
		if (one_statement(i->name, i->ins_index, i->position, error_code))
		{
			//�ڶ��ν���ɹ�
		}
		else
		{
			//�ڶ������޷����㣬�������
			add_error_information(line, error_code, i->name);
		}
	}


	if (error_information.empty())
	{
		//�õ���ȫ�Ķ���������������ļ�
		//�õ���ȫ�Ķ���������������ļ�
		if (output_bin_file(work_path + "\\output.txt"))
		{
			cout << "����ļ��ɹ�" << endl;
		}
		else
		{
			cout << "����ļ�ʧ��" << endl;
			return false;
		}

		if (output_compile_information_file(work_path + "\\x_output.txt"))
		{
			cout << "���������Ϣ�ɹ�" << endl;
		}
		else
		{
			cout << "����ļ�ʧ��" << endl;
			return false;
		}
	}
	else
		return false;

	return true;
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
		cout << "��ȡָ��ű��ɹ���" << "��ɨ�赽" << ins.size() << "��ָ��" << endl;
		if (!ins_error_information.empty())
		{
			cout << "δʶ��ָ�" << endl;
			for (auto i = ins_error_information.begin(); i != ins_error_information.end(); i++)
			{
				cout << *i << endl;
			}
		}
	}

	//�������
	if (assembly_execute())
	{
		cout << "������" << endl;
		cout << "size:" << address_count << endl;
	}
	else
	{
		cout << "\n\n���ʧ��" << endl;
		cout << "����" << error_information.size() << endl;

		for (auto i = error_information.begin(); i != error_information.end(); i++)
		{
			cout << *i << endl;
		}
	}


	system("PAUSE");
}

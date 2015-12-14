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
#define MAX_OFFSET_BACK		-32768
#define MAX_OFFSET_FRONT	32767
#define MAX_TARGET_BACK		-33554432
#define MAX_TARGET_FRONT	33554431



string work_path;
char buf[1000];


enum class symbol_type{ RS, RD, RT, SA, IMMEDIATE, TARGET, OFFSET, INSTR_INDEX, BASE, HINT, DEFINE, LABEL, STATEMENT};

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


vector<bin_str> middle_result;




regex r_source("^\\b([a-zA-Z]+)(?:\\s+(\\w+)(?:,(\\w+))?(?:,(\\w+))?)?$");
regex reg_format("^\\bR((?:[012]?\\d)|(?:3[01]))$");
regex r_output("^(#[01]{6})\\s+((?:\\w+)|(?:#[01]{5}))(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+((?:\\w+)|(?:#[01]{5})))?(?:\\s+(#[01]{6}))?$");
regex r_comment("^(.*?)((\\s*//.*)|(\\s*))$");
regex r_define("^\\s*#define\\s+([a-zA-Z]\\w*)\\s+(.*?)$");
regex r_label("^([a-zA-Z]\\w*):$");

regex r_null_line("^\\s*$");

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

	regex r_instruction_format("^#\\w+\\{\\s*\\{([^\\}]*)\\};\\s*\\{([^\\}]*)\\}\\s*\\}$");
	regex r_source_input("^\\b([a-zA-Z]+)(?:\\s+(\\w+)(?:,(\\w+))?(?:,(\\w+))?)?$");
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
			if (regex_match(ins_str, result, r_source_input))
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
						str_temp += "(?:\\s*,\\s*([^,]+?))?";			//������
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
	for (auto i = middle_result.begin(); i != middle_result.end(); )
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
		if (i != middle_result.end())
			output_file_stream << str_temp << "," << endl;
		else
			output_file_stream << str_temp << ";" << endl;
	}


	//�ر��ļ�
	output_file_stream.close();

	return true;
}



regex r_scan_symbol("(?:(?:<<)|(?:>>)|(?:[-+*/\\(\\)&|~^]+))|(?:(?:0x[0-9a-fA-F]+)|(?:\\d+))|([a-zA-Z]\\w*)");



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
				scan_count++;
				s1 = it->str(1);

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
	bin_str bin_temp;

	string s1, s2;
	int bit_count;
	int bits;
	int i, j;

	int pc;
	signed int offset;
	signed int target;

	//���һ��ָ��Ŀռ�
	if (position == -1)
	{
		middle_result.push_back(bin_temp);
		position = middle_result.size() - 1;
	}
	
	regex_match(source_string, result_source_string,ins_index->r);
	regex_match(ins_index->source_format, result_source_format, r_source);

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
			else if (s2 == "target")			symbol_x_temp = symbol_type::TARGET;
			else if (s2 == "instr_index")		symbol_x_temp = symbol_type::INSTR_INDEX;
			else if (s2 == "base")				symbol_x_temp = symbol_type::BASE;
			else if (s2 == "hint")				symbol_x_temp = symbol_type::HINT;
			else if (s2 == "RS")				symbol_x_temp = symbol_type::RS;
			else if (s2 == "RT")				symbol_x_temp = symbol_type::RT;
			else if (s2 == "RD")				symbol_x_temp = symbol_type::RD;

			//�ж��Ƿ�Ϊ�Ĵ���
			s1 = result_source_string[i].str();
			if (regex_match(s1, result_reg, reg_format))
			{
				//�ǼĴ�������һ���жϺ�����
				if (symbol_x_temp == symbol_type::RS || symbol_x_temp == symbol_type::RT || symbol_x_temp == symbol_type::RD)
				{
					//����Ĵ������ʹ�ã�ת��Ϊ�Ĵ������
					s_stream.clear();
					s_stream << result_reg[1].str();;
					s_stream >> j;

					switch (symbol_x_temp)
					{
					case symbol_type::RS:	bin_output.rs = j;		break;
					case symbol_type::RT:	bin_output.rt = j;		break;
					case symbol_type::RD:	bin_output.rd = j;		break;
					default:break;
					}
				}
				else
				{
					//���ش�����룬1���÷�����
					error_code = 1;
					return false;
				}
			}
			else
			{
				//���ǼĴ���
				//����Ӧ�õõ�һ������ֵ
				//����޷������ֵ��������ʧ��

				if (evaluation(s1, value_temp))
				{
					//��ֱ�ӽ���
					switch (symbol_x_temp)
					{
					case symbol_type::IMMEDIATE:		bin_output.immediate = value_temp;	break;
					case symbol_type::SA:;				bin_output.sa = value_temp;			break;
					case symbol_type::OFFSET:;						
					{
						//ƫ���� = Ŀ���ַ - ��ǰPC
						pc = position * 4;
						offset = value_temp - pc;
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
						pc = position * 4;
						target = value_temp - pc;
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
					case symbol_type::INSTR_INDEX:
					case symbol_type::BASE:;
					case symbol_type::HINT:;
					}
				}
				else
				{
					//���ش�����룬2���޷�����
					error_code = 2;
					return false;
				}
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
	regex_match(ins_index->output_format, result_output_format, r_output);

	bit_count = 0;
	bin_temp.bin = 0;

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

				bin_temp.bin |= value_temp << (32 - bit_count);
			}
			else
			{
				switch (symbol_x_temp)
				{
				case symbol_type::RS:			bin_temp.rs = bin_output.rs;				bit_count += 5;		break;
				case symbol_type::RT:			bin_temp.rt = bin_output.rt;				bit_count += 5;		break;
				case symbol_type::RD:			bin_temp.rd = bin_output.rd;				bit_count += 5;		break;
				case symbol_type::IMMEDIATE:	bin_temp.immediate = bin_output.immediate;	bit_count += 16;	break;
				case symbol_type::SA:			bin_temp.sa = bin_output.sa;				bit_count += 5;		break;
				case symbol_type::OFFSET:		bin_temp.offset = bin_output.offset;		bit_count += 16;	break;
				case symbol_type::TARGET:		
				case symbol_type::INSTR_INDEX:	bin_temp.target = bin_output.target;		bit_count += 26;	break;
				case symbol_type::BASE:
				case symbol_type::HINT:
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
	middle_result[position] = bin_temp;

	return true;
}



bool assembly_execute(void)
{
	int line;
	int error_code;

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


	//ȡһ���ı�
	while (source_file_stream.getline(buf,500))
	{
		s1 = buf;

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
				error_information.push_back(s2);
			}
		}
		else if (regex_match(source_one_line, result_label, r_label))
		{
			//�ж��Ǳ�ǩ
			symbol_temp.name = result_label.str(1);
			symbol_temp.symbol_x = symbol_type::LABEL;
			symbol_temp.value = middle_result.size() * 4;

			if (!symbol_add(symbol_temp))
			{
				//�������ظ�
				s2 = "�������ظ���" + symbol_temp.name;
				error_information.push_back(s2);
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
					if (one_statement(source_one_line, ins_index, -1, error_code))
					{
						//����ɹ�
					}
					else
					{
						//����ʧ�ܣ�����δ֪���ű�
						unknown_symbol_temp.name = source_one_line;
						unknown_symbol_temp.position = middle_result.size() - 1;
						unknown_symbol_temp.symbol_x = symbol_type::STATEMENT;
						unknown_symbol_temp.ins_index = ins_index;
						unknown_symbol_temp.line = line;
						unknown_symbol_tab.push_back(unknown_symbol_temp);
					}

					//���䵽һ����䣬����ɴ�������ѭ��
					break;
				}
			}

			//������
			if (ins_index == ins.end())
			{
				s_stream.clear();
				s_stream << line;
				s_stream >> s2;

				s2 = "error: line " + s2 + "    " + s1;
				error_information.push_back(s2);
			}
		}
	
		//�кż���
		line++;
	}



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
			s_stream.clear();
			s_stream << i->line;
			s_stream >> s2;
			s2 = "error: line " + s2;

			switch (error_code)
			{
			case 1:		s2 += "�÷�����";				break;
			case 2:		s2 += "�޷�����";				break;
			case 3:		s2 += "ƫ������� ";			break;
			case 4:		s2 += "ָ��δ֧�ֻ����� ";	break;
			}

			s2 += i->name;
			error_information.push_back(s2);
		}
	}


	if (error_information.empty())
	{
		//�õ���ȫ�Ķ���������������ļ�
		if (output_to_file())
		{
			cout << "����ļ��ɹ�" << endl;
			return true;
		}
		else
		{
			cout << "����ļ�ʧ��" << endl;
			return false;
		}
	}
	else
		return false;
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
		cout << "size:" << middle_result.size() << endl;
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

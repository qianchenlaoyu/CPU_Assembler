// MIPS_asm.cpp : 定义控制台应用程序的入口点。
//

/* MIPS汇编器，用于转换汇编语言到机器语言，输出ROM初始化文件 */
/* 只作简单替换操作，支持宏定义，所有指令都从0地址连续排放 */
/* 支持部份指令，更多的指令可以对照添加，指令根据脚本文件生成 */
/* 支持标号，用于产生跳转地址 */






/* 汇编过程 */
/* 判断调用参数，如果为合理参数就进行指定文件汇编，没有要求手动输入参数，不合理参数输出提示 */
/* 根据同目录下的指令脚本文件生成指令表，找不到指令脚本文件输出错误 */
/* 初始化系统变量，包括PC指向0地址，建立符号链表，建立输出缓冲 */
/* 循环单行进行汇编，直到文件结束或遇到错误，其间的空行和注释将被忽略掉 */
/* 输出汇编结果 */

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

/* 符号表 */
/* 符号表采用动态向量方式存储 */
/* 符号名最长不作限制 */
/* 符号数不作限制，受限于int型变量能表达的最大正整数值 */
struct symbol_str{
	string name;
	int value;
};

struct symbol_tab_str{
	int number;
	vector<symbol_str> symbol_vector;
}symbol_tab;



/* 中间结果 */
/* 动态向量方式存储 */
/* 内容包括二进制和符号 */
/* 使用结构体位域,直接合成32位宽的2进制数 */
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


/* 前向引有符号表 */
/* 记录未知的符号引用，在下一轮插补时参考 */
enum class symbol_type{ RS, RD, RT, SA, IMMEDIATE, TARGET, OFFSET, INSTR_INDEX, BASE, HINT };

struct unknown_symbol_str{
	string name;
	symbol_type symbol_x;
};

struct unknown_symbol_tab_str{
	int count;
	vector<unknown_symbol_str> unknown_symbol;
}unknown_symbol_tab;

/* 输出结果 */
/* 输出到源文件同目录下 */
/* 与源文件同名，后缀为.coe */
/* 格式为ROM初始化文件格式 */


/*
	根据指令脚本文件编译指令集
	每条指令由源匹配格式和输出处理格式组成
	指令编译完成后所有信息保存在指令表中
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

	cout << "指令脚本：" << instruction_path << endl;

	instruction_file_stream.open(instruction_path);

	if (instruction_file_stream)
		cout << "指令脚本文件打开成功" << endl;
	else
	{
		cout << "指令脚本文件打开失败" << endl;
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
		//对整条指令格式串进行匹配
		ins_str = buf;
		if (regex_match(ins_str, result, r_instruction_format))
		{
			ins_temp.source_format = result.str(1);
			ins_temp.output_format = result.str(2);

			//进一步对源输入格式串进行匹配
			ins_str = result.str(1);
			if (regex_match(ins_str, result, r_source_input))
			{
				str_temp = "(?:^\\s+\\b";
				str_temp += "(" + result[1].str() + ")";	//第一组，指令码

				if (result[2].matched)
				{
					str_temp += "(?:";						//区分带参情况
					str_temp += "\\s+";
					str_temp += "(\\w+)";					//第二组

					if (result[3].matched)
					{
						str_temp += "(?:\\s*,\\s*(\\w+))?";			//第三组
					}

					if (result[4].matched)
					{
						str_temp += "(?:\\s*,\\s*(\\w+))?";			//第四组
					}

					str_temp += ")?";
				}

				str_temp += "\\s*(?:$|//))|(?:^\\s*(?:$|//))";		//处理注释和空行

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
				//记录未识别指令
				ins_error_information.push_back(buf);
			}
		}
		else{
			//记录未识别指令
			ins_error_information.push_back(buf);
		}
	}

	return true;
}



/*
	输出到文件
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
		cout << "创建文件失败" << endl;
		return false;
	}

	//输出文件头
	output_file_stream << "memory_initialization_radix=2;" << endl;
	output_file_stream << "memory_initialization_vector=" << endl;


	//循环输出二进制数
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


	//关闭文件
	output_file_stream.close();

	return true;
}




/*
	表达式求值
	返回逻辑值
*/
bool evaluation(string &exp, char32_t &value)
{
	//首先对表达式中的符号进行解引用
	//然后以常量表达式的形式进行计算
	//不能解引用，返回失败
	//表达式计算错误，返回失败


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
	执行汇编
	返回逻辑值
	汇编信息保存在全局变量中
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



	//指出操作文件,并判断文件路径合理性
	cout << "汇编文件：" << source_file_path << endl;
	source_file_stream.open(source_file_path, ifstream::in);
	if (source_file_stream)
	{
		cout << "打开汇编文件成功。" << endl;
	}
	else
	{
		cout << "打开汇编文件失败。" << endl;
		return false;
	}

	//初始化变量
	assembly_information.size = 0;
	error_information.number = 0;
	line = 1;


	//取一行文本
	while (source_file_stream.getline(buf,500))
	{
		s1 = buf;
		for (ins_index = ins.ins_tab.begin(); ins_index != ins.ins_tab.end(); ins_index++)
		{
			if (regex_search(s1, result, ins_index->r))
			{
				//匹配到一条语句，进行处理

				//分别对源输入格式串和汇编语句进行匹配，提取出输入值
				//求得每一个位域的值
				//已知引用，直接产生机器码
				//未知引用，可能是前向引用，记入未知符号表

				
				if (result[1].matched)
				{
					regex_match(ins_index->source_format, result_source, r_source);
					
					//对输入的每个符号进行分析，转化为数值
					for (i = 2; i < 5; i++)
					{
						if (result[i].matched)
						{
							//判断是否为寄存器
							s2 = result[i].str();
							if (regex_match(s2, result_reg, reg_format))
							{
								s3 = result_reg[1].str();

								//是寄存器，进一步判断合理性
								s2 = result_source[i].str();
								if ((s2 == "RS") || (s2 == "RT") || (s2 == "RD"))
								{
									//合理寄存器编号使用，转化为寄存器编号
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
									//产生错误
									s_stream.clear();
									s_stream << line;
									s_stream >> s2;

									s2 = "error: line " + s2 ;
									s2 += "    用法错误   " + s1;
									error_information.error.push_back(s2);
									error_information.number++;

									//累计错误达到上限停止汇编
									if (error_information.number == MAX_ERROR_NUMBER)
										return false;
								}

							}
							else
							{
								//不是寄存器
								//这里应该得到一个常量值
								//如果无法解算出值来，此次假定为是前向引用
								//无法解算的表达式将被存入未知引用表里

								if (evaluation(s2, value_temp))
								{
									//可直接解算
									

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
									//不能解算，记入前向引用

								}

							}

						}
						else
						{
							//没有可用的匹配项，退出循环
							break;
						}
					}


					//依据输出格式，按位填充
					regex_match(ins_index->output_format, result_output, r_output);
					bit_count = 0;
					bin_temp.bin = 0;

					for (i = 1; i < 7; i++)
					{
						if (result_output[i].matched)
						{
							s2 = result_output[i].str();
							//区分开直接数和参数
							if (s2[0] == '#')
							{
								//直接数，将二进制串转化为int型，并存入中间结果
								value_temp = binary_to_uint(result_output[i].str(), bits);
								bit_count += bits;

								bin_temp.bin |= value_temp << (32 -bit_count);
							}
							else
							{
								//判断是否是寄存器参数
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
							//没有可用的匹配项，退出循环
							break;
						}
					}
					
					//存入结果
					middle_result.bin_data.push_back(bin_temp);
					middle_result.count++;
					assembly_information.size++;
				}
				else			
				{
					//空行
				}

				//区配到一条语句，并完成处理，进入下一循环
				break;
			}
			

		}

		//错误处理
		if (ins_index == ins.ins_tab.end())
		{
			s_stream.clear();
			s_stream << line;
			s_stream >> s2;

			s2 = "error: line " + s2 + "    " + s1;
			error_information.error.push_back(s2);
			error_information.number++;

			//累计错误达到上限停止汇编
			if (error_information.number == MAX_ERROR_NUMBER)
				return false;
		}
		
		//行号计数
		line++;
	}



	//进行第二次扫描，替换前向引用符号
	for (auto i = unknown_symbol_tab.unknown_symbol.begin(); i != unknown_symbol_tab.unknown_symbol.end(); i++)
	{



	}


	//得到完全的二进制数，输出到文件
	if (output_to_file())
	{
		cout << "输出文件成功" << endl;
	}
	else
	{
		cout << "输出文件失败" << endl;
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
		////没有传入文件路径，手动输入
		//cout << "输入文件路径：" << endl;
		//cin >> file_path;
		source_file_path = work_path + "\\ASM_Test.txt";
	}
	else
	{
		source_file_path = argv[1];
	}



	cout << "当前目录：" << work_path << endl;

	//根据指令定义脚本生成指令处理方案
	if (instruction_compile() == false)
	{
		cout << "指令脚本错误" << endl;
		system("PAUSE");
		return 0;
	}
	else
	{
		cout << "读取指令脚本成功，" << "共扫描到" << ins.count << "条指令" << endl;
		cout << "未识别指令：" << endl;
		for (auto i = ins_error_information.begin(); i != ins_error_information.end(); i++)
		{
			cout << *i << endl;
		}


	}

	//启动汇编
	if (assembly_execute())
	{
		cout << "汇编完成" << endl;
		cout << "size:" << assembly_information.size << endl;
	}
	else
	{
		cout << "\n\n汇编失败" << endl;
		cout << "错误：" << error_information.number << endl;

		for (auto i = error_information.error.begin(); i != error_information.error.end(); i++)
		{
			cout << *i << endl;
		}
	}


	system("PAUSE");
}
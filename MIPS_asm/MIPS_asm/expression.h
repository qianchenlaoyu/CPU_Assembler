


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

unsigned int binary_to_uint(string &s, int &bits);		//�����ƴ�ת��Ϊint����ֵ
bool trans(string &exp, string &postexp);				//���������ʽexpת���ɺ�׺���ʽpostexp
bool compvalue(string &postexp, double &value);			//�����׺���ʽpostexp��ֵ

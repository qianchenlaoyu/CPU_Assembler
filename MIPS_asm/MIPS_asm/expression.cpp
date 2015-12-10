

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
	将二进制串转化为int型
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




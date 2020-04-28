#include "dispose.h"
#include "util/rpn.h"
#include "util/kmp.h"
#include <algorithm>
#include <stack>

std::string ToBit(uint64_t number, int bit)
{
	std::stack<char> buf;
	std::string result;
	while (number)
	{
		int temp = number % bit;
		buf.push(temp<10?('0'+temp):('A'+temp-10));
		number /= bit;
	}

	while (!buf.empty())
	{
		result.push_back(buf.top());
		buf.pop();
	}

	return result;
}

void RemoveExcessZero(std::string& str)
{
	bool flag = true;
	auto end = std::remove_if(str.rbegin(), str.rend(), [&flag](const char& ch) {
		if (flag)
		{
			if (ch == '0') return true;
			if (ch == '.')
			{
				flag = false;
				return true;
			}
			flag = false;
		}
		return false;

	}).base();
	str.erase(str.begin(), end);
}


bool Dispose(int32_t type, int64_t from_discuss, int64_t from_qq, std::string msg, std::string& result)
{
	static std::string cmd = "计算";

	size_t index = util_kmp::KMP_Find(msg.c_str(), cmd.c_str());

	if (index == util_kmp::npos) return false;

	int to_bit = 0;
	size_t index_end = util_kmp::KMP_Find(msg.c_str(), "->");
	if (index_end == util_kmp::npos)
	{
		index_end = msg.length();
	}
	else
	{
		to_bit = atoi(msg.c_str() + index_end + 2);
		if (to_bit < 2 || to_bit > 36)
		{
			result = "请输入正确的进制数，允许进制范围：[2,36]";
			return true;
		}
	}

	result = "0";
	try {
		double calc = CalculateExpr(msg.substr(index + cmd.length(), index_end - cmd.length()).c_str());
		if (calc != 0)
		{
			if (to_bit == 0 || to_bit == 10)
			{
				result = std::to_string(calc);
				RemoveExcessZero(result);
			}
			else
			{
				calc = abs(calc);
				if (calc > 1) result = ToBit(static_cast<uint64_t>(calc), to_bit);
			}
		}
	}
	catch (const char* error_msg)
	{
		result = error_msg;
	}
	
	return true;
}
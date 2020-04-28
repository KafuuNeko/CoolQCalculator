#include "rpn.h"
#include <stack>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <math.h>

/**
** 转换任意16位及其16位以下的进制为十进制数
** @param str 进制文本
** @param str_len 进制文本有效长度
** @param bit 进制（2-16）
*/
inline int64_t toDec(const char* str, size_t str_len, int bit)
{
	int64_t result = 0;
	for (size_t i = 0; i < str_len; ++i)
	{
		const char& ch = str[i];
		result *= bit;
		if (ch >= '0' && ch <= '9')
		{
			result += ch - '0';
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			result += ch - 'a' + 10;
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			result += ch - 'A' + 10;
		}
		else
		{
			return 0;
		}
	}
	return result;
}

/**
** 判断字符是否是和数字有关的字符
** @param ch 输入字符
** @return 有关返回true，否则返回false
*/
constexpr bool isNumberChar(const char& ch)
{
	return (ch >= '0' && ch <= '9') //是数字
		|| (ch >= 'a' && ch <= 'f') //是否是十六进制字符
		|| (ch >= 'A' && ch <= 'F') //是否是十六进制字符
		|| (ch == 'O' || ch == 'H') //是否是位标记符（标记符B已经在判断是否十六位的时候判断了）
		|| (ch == '.');			//是否是小数点
}

/**
** 将数字缓冲区的内容转换到double数据
** @param buf 缓冲区指针
** @param sz 缓冲区有效数据
*/
constexpr double toDouble(const char* buf, size_t sz)
{
	switch (buf[sz - 1])
	{
	case 'B': return toDec(buf, sz - 1, 2);

	case 'O': return toDec(buf, sz - 1, 8);

	case 'H': return toDec(buf, sz - 1, 16);

	default: return atof(buf);
	}
}



/**
** 辅助函数，用于取两个栈顶元素
** @return 若stack里的元素不足两个，则返回false表示获取失败*/
template <typename T>
inline bool RpnTop2(std::stack<T>& rpn, T& e, T& s)
{
	if (rpn.size() < 2)
		return false;
	e = rpn.top();
	rpn.pop();
	s = rpn.top();
	rpn.pop();
	return true;
}

/**
** 计算逆波兰表达式
** @param rpn_exp 逆波兰表达式串
** @param exp_len 逆波兰表达式串的长度
** @return 返回最终计算结果 */
double CalculateRpn(const char* rpn_exp, size_t exp_len)
{
	constexpr size_t N_BUF_MAXSIZE = 64;

	std::stack<double> rpn;

	char number_buf[N_BUF_MAXSIZE];
	size_t buf_pos = 0;

	for (size_t p = 0; p < exp_len; ++p)
	{
		if (rpn_exp[p] == ' ')
		{
			//遇到空格，且数字缓冲区内有数据则数字一读取完成，转换后直接加入
			if (buf_pos != 0)
			{
				rpn.push(toDouble(number_buf, buf_pos));
				buf_pos = 0;
			}
		}
		else if (isNumberChar(rpn_exp[p]))
		{
			//判断如果加入到缓冲区是否超出范围
			if (buf_pos + 1 >= N_BUF_MAXSIZE) throw "Digital buffer overruns";
			number_buf[buf_pos++] = rpn_exp[p];
			number_buf[buf_pos] = '\0';
		}
		else
		{
			//e是stack第一次弹出的值，s是stack第二次弹出的值
			double s, e;
			switch (rpn_exp[p])
			{
			case '+':
				if (!RpnTop2(rpn, e, s)) throw "Expression error";
				rpn.push(s + e);
				break;

			case '-':
				if (!RpnTop2(rpn, e, s)) throw "Expression error";
				rpn.push(s - e);
				break;

			case '*':
				if (!RpnTop2(rpn, e, s)) throw "Expression error";
				rpn.push(s * e);
				break;

			case '/':
				if (!RpnTop2(rpn, e, s) || e == 0) throw "Divisor cannot be 0";
				rpn.push(s / e);
				break;

			case '%':
				if (!RpnTop2(rpn, e, s) || e == 0) throw "Divisor cannot be 0";
				rpn.push(static_cast<int64_t>(s) % static_cast<int64_t>(e));
				break;

			case '^':
				if (!RpnTop2(rpn, e, s)) throw "Expression error";
				rpn.push(pow(s, e));
				break;
			}
		}
	}

	//如果计算完毕后stack剩余的成员数不为1则失败
	if (rpn.size() != 1) throw "Expression error";

	return rpn.top();
}

/**
** 将一个数学公式构造为逆波兰表达式
** @param _math_exp 表达式串
** @param _exp_len 表达式串的长度
** @return 返回逆波兰表达式 */
std::string MakeRpn(const char* math_exp, size_t exp_len)
{
	bool first = true;

	std::stack<char> rpn_op;
	std::string result;

	for (size_t p = 0; p < exp_len; ++p)
	{
		if (math_exp[p] == ' ')
			continue;

		//开头第一个有效符号
		if (first)
		{
			//如果开头第一个有效符号是+或者-，则在开头补一个0
			if (math_exp[p] == '-' || math_exp[p] == '+')
			{
				result.push_back('0');
				result.push_back(' ');
			}

			first = false;
		}

		//用于标识是否追加过数字，若追加过数字，则需要在完成后在末尾插入空格
		bool flag = false;

		while (isNumberChar(math_exp[p]) || math_exp[p] == ' ')
		{
			if (math_exp[p] != ' ')
			{
				result.push_back(math_exp[p]);
				flag = true;
			}

			if (++p >= exp_len)
				break;
		}

		if (flag)			result.push_back(' ');
		if (p >= exp_len)	break;

		//临时变量，用于记录顶栈数据
		char top_ch;

		if (math_exp[p] == ')')
		{
			//如果遇到右括号，则不断弹出stack里的字符到result里,直到遇到左括号或全部弹出
			while (!rpn_op.empty())
			{
				top_ch = rpn_op.top();
				rpn_op.pop();
				//遇到左括号，退出循环
				if (top_ch == '(') break;
				//将弹出的内容输出到结果
				result.push_back(top_ch);
				result.push_back(' ');
			}
		}
		else if (math_exp[p] == '+' || math_exp[p] == '-')
		{
			//遇到加减法运算符，判断栈是否为空，若为空则直接加入到栈
			if (rpn_op.empty())
			{
				//Stack为空，直接push
				rpn_op.push(math_exp[p]);
			}
			else
			{
				/*
				 * 栈不为空，则不断将栈里的内容出栈，由于加减运算符的优先级是除去左括号以外最小的，
				 * 所以此次循环直到遇到左括号或全部弹出才推出，随后将此运算符入栈
				*/
				while (!rpn_op.empty())
				{
					top_ch = rpn_op.top();
					//遇到左括号，循环结束
					if (top_ch == '(') break;

					result.push_back(top_ch);
					result.push_back(' ');
					rpn_op.pop();
				}

				rpn_op.push(math_exp[p]);
			}
		}
		else if (math_exp[p] == '(')
		{
			//右括号，无条件直接加入
			rpn_op.push(math_exp[p]);
		}
		else if (math_exp[p] == '*' || math_exp[p] == '/' || math_exp[p] == '%' || math_exp[p] == '^')
		{
			while (!rpn_op.empty())
			{
				top_ch = rpn_op.top();
				//除非找到优先级比即将加入的操作符小的，否则全部出栈
				if (top_ch == '(' || top_ch == '+' || top_ch == '-')
					break;

				result.push_back(top_ch);
				result.push_back(' ');
				rpn_op.pop();
			}

			rpn_op.push(math_exp[p]);
		}
	}

	//处理完表达式字符串后，如果栈内还有残留数据，那么依次出栈，加入到结果
	while (!rpn_op.empty())
	{
		result.push_back(rpn_op.top());
		result.push_back(' ');
		rpn_op.pop();
	}

	return result;
}

double CalculateExpr(const char* expr)
{
	try {
		std::string rpn_exp = MakeRpn(expr, strlen(expr));
		return CalculateRpn(rpn_exp.c_str(), rpn_exp.length());
	}
	catch (const char* error_msg)
	{
		throw error_msg;
	}

}

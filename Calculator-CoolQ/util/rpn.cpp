#include "rpn.h"
#include <stack>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <math.h>

static const char* kExpressionError = "ExpressionError";
static const char* DivisorCannotZero = "DivisorCannotZero";
/**
** 转换任意16位及其16位以下的进制为十进制数
** @param str_beg 起始迭代器
** @param str_end 结束迭代器
** @param bit 进制（2-16）
*/
template <class _iter>
static int64_t toDec(_iter str_beg, _iter str_end, int bit)
{
	int64_t result = 0;
	for (; str_beg != str_end; ++str_beg)
	{
		const char& ch = *str_beg;
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
inline bool isNumberChar(const char& ch)
{
	return (ch >= '0' && ch <= '9')	   //是数字
		|| (ch >= 'a' && ch <= 'f') //是否是十六进制字符
		|| (ch >= 'A' && ch <= 'F') //是否是十六进制字符
		|| (ch == 'O' || ch == 'H') //是否是位标记符（标记符B已经在判断是否十六位的时候判断了）
		|| (ch == '.');			   //是否是小数点
}

/**
** 将数字缓冲区的内容转换到double数据
** @param buf 缓冲区指针
** @param sz 缓冲区有效数据
*/
inline double toDouble(const std::string& buf)
{
	size_t sz = buf.length();
	switch (buf[sz - 1])
	{
	case 'B':
		return toDec(buf.cbegin(), buf.cend() - 1, 2);

	case 'O':
		return toDec(buf.cbegin(), buf.cend() - 1, 8);

	case 'H':
		return toDec(buf.cbegin(), buf.cend() - 1, 16);

	default:
		return atof(buf.c_str());
	}
}

/**
** 辅助函数，用于取两个栈顶元素
** @return 若stack里的元素不足两个，则返回false表示获取失败*/
template <typename T>
inline bool RpnTop2(std::stack<T>& rpn, T& e, T& s)
{
	if (rpn.size() < 2)
	{
		return false;
	}

	e = rpn.top();
	rpn.pop();
	s = rpn.top();
	rpn.pop();

	return true;
}

/**
** 判断字符是否为数学操作符
** @param ch 要判断的字符
** @return 若为数学操作符则返回true，否则返回false
*/
inline bool isMathNotation(const char& ch)
{
	return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^';
}

/**
** 获取指定操作符优先级
** @param ch 数学操作符字符
** @return 优先级
*/
constexpr int getMathNotationPriority(const char& ch)
{
	if (ch == '(')
		return 0;
	if (ch == '+' || ch == '-')
		return 1;
	if (ch == '*' || ch == '/' || ch == '%')
		return 2;
	if (ch == '^')
		return 3;
	return -1;
}

/**
** 计算逆波兰表达式
** @param rpn_exp 逆波兰表达式串
** @param exp_len 逆波兰表达式串的长度
** @return 返回最终计算结果 */
double CalculateRpn(const std::string& rpn_exp)
{
	std::stack<double> rpn;
	std::string number_buf;

	for (auto iter = rpn_exp.begin(); iter != rpn_exp.end(); ++iter)
	{
		if (*iter == ' ')
		{
			//遇到空格，且数字缓冲区内有数据则转换转换数字缓冲区内的数据成Double类型，并入栈
			if (!number_buf.empty())
			{
				rpn.push(toDouble(number_buf));
				number_buf.clear();
			}
		}
		else if (isNumberChar(*iter))
		{
			//加入到数字缓冲区
			number_buf.push_back(*iter);
		}
		else
		{
			//e是stack第一次弹出的值，s是stack第二次弹出的值
			double s, e;
			switch (*iter)
			{
			case '+':
				if (!RpnTop2(rpn, e, s))
					throw kExpressionError;
				rpn.push(s + e);
				break;

			case '-':
				if (!RpnTop2(rpn, e, s))
					throw kExpressionError;
				rpn.push(s - e);
				break;

			case '*':
				if (!RpnTop2(rpn, e, s))
					throw kExpressionError;
				rpn.push(s * e);
				break;

			case '/':
				if (!RpnTop2(rpn, e, s) || e == 0)
					throw DivisorCannotZero;
				rpn.push(s / e);
				break;

			case '%':
				if (!RpnTop2(rpn, e, s) || e == 0)
					throw DivisorCannotZero;
				rpn.push(static_cast<int64_t>(s) % static_cast<int64_t>(e));
				break;

			case '^':
				if (!RpnTop2(rpn, e, s))
					throw kExpressionError;
				rpn.push(pow(s, e));
				break;
			}
		}
	}

	//如果计算完毕后stack剩余的成员数不为1则失败
	if (rpn.size() != 1)
		throw kExpressionError;

	return rpn.top();
}

/**
** 辅助函数 处理新字符
** 在将数学表达式构造为逆波兰表达式时处理新数学操作符时调用
** @param result 结果
** @param notation 运算符Stack
** @param new_ch 新字符
*/
static void MakeRpnDisposeNewChar(std::string& result, std::stack<char>& notation, const char& new_ch)
{
	int priority = getMathNotationPriority(new_ch);

	while (!notation.empty())
	{
		//特殊情况，^操作符运算顺序是从右到左
		if (new_ch == '^' && new_ch == notation.top()) break;
		//如果当前顶栈运算符优先级低于新运算符优先级，则结束循环
		if (getMathNotationPriority(notation.top()) < priority) break;

		//顶栈运算符优先级大于新运算符优先级，根据规则，优先级高于等于的的全部出栈
		result.push_back(notation.top());
		result.push_back(' ');
		notation.pop();
	}

	notation.push(new_ch);
}

/**
** 将一个数学表达式构造为逆波兰表达式
** @param _math_exp 表达式串
** @param _exp_len 表达式串的长度
** @return 返回逆波兰表达式 */
std::string MakeRpn(const std::string& math_exp)
{
	bool first = true;

	std::stack<char> notation;
	std::string result;

	auto iter_end = math_exp.end();
	for (auto iter = math_exp.begin(); iter != iter_end; ++iter)
	{
		if (*iter == ' ')
			continue;

		//开头第一个有效符号
		if (first)
		{
			//如果开头第一个有效符号是+或者-，则在开头补一个0
			if (*iter == '-' || *iter == '+')
			{
				result.push_back('0');
				result.push_back(' ');
			}

			first = false;
		}

		//用于标识是否追加过数字，若追加过数字，则需要在完成后在末尾插入空格
		bool flag = false;

		while (isNumberChar(*iter) || *iter == ' ')
		{
			if (*iter != ' ')
			{
				result.push_back(*iter);
				flag = true;
			}

			if (++iter == iter_end)
				break;
		}

		if (flag) //如果追加过数字，则在结果尾部加入空格
			result.push_back(' ');

		if (iter == iter_end)
			break;

		if (*iter == ')')
		{
			//如果遇到右括号，则不断弹出数学操作符栈中符号，直到遇到左括号或全部弹出
			while (!notation.empty())
			{
				char top_ch = notation.top();
				notation.pop();
				//遇到左括号，退出循环
				if (top_ch == '(')
					break;
				//将弹出的内容输出到结果
				result.push_back(top_ch);
				result.push_back(' ');
			}
		}
		else if (*iter == '(')
		{
			//右括号，无条件直接加入
			notation.push(*iter);
		}
		else if (isMathNotation(*iter))
		{
			MakeRpnDisposeNewChar(result, notation, *iter);
		}
	}

	//处理完表达式字符串后，如果栈内还有残留数据，那么依次出栈，加入到结果
	while (!notation.empty())
	{
		result.push_back(notation.top());
		result.push_back(' ');
		notation.pop();
	}

	return result;
}

double CalculateExpr(const std::string& expr)
{
	try
	{
		return CalculateRpn(MakeRpn(expr));
	}
	catch (const char* error_msg)
	{
		throw error_msg;
	}
}

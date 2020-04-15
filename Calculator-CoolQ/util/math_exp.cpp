#include "math_exp.h"
#include <stack>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <math.h>

/*
** 辅助函数，用于取两个栈顶元素
** @return 若stack里的元素不足两个，则返回false表示获取失败*/
template<typename T>
inline bool rpn_top2(std::stack<T>& rpn, T& e, T& s)
{
	if(rpn.size() < 2) return false;
	e = rpn.top();
	rpn.pop();
	s = rpn.top();
	rpn.pop();
	return true;
}

/*
** 计算逆波兰表达式
** @param _rpn_exp 逆波兰表达式串
** @param _exp_len 逆波兰表达式串的长度
** @return 返回最终计算结果 */
double calculate_rpn(const char* _rpn_exp, size_t _exp_len)
{
	constexpr size_t N_BUF_MAXSIZE = 32;
	
	std::stack<double> rpn;
	
	char number_buf[N_BUF_MAXSIZE];
	size_t buf_pos = 0;
	
	for(size_t p = 0; p < _exp_len; ++p)
	{
		if(_rpn_exp[p] == ' ')
		{
			//遇到空格，且数字缓冲区内有数据则数字一读取完成，转换后直接加入
			if(buf_pos != 0)
			{
				rpn.push(atof(number_buf));
				buf_pos = 0;
			}
			
		}
		else if(_rpn_exp[p] == '.' || (_rpn_exp[p] >= '0' && _rpn_exp[p] <= '9'))
		{
			//判断如果加入到缓冲区是否超出范围
			if(buf_pos+1 >= N_BUF_MAXSIZE) return 0;
			number_buf[buf_pos++] = _rpn_exp[p];
			number_buf[buf_pos] = '\0';
		}
		else
		{
			//e是stack第一次弹出的值，s是stack第二次弹出的值
			double s, e;
			switch(_rpn_exp[p])
			{
				case '+':
					if(!rpn_top2(rpn, e, s)) return 0;
					rpn.push(s + e);
					break;
				
				case '-':
					if(!rpn_top2(rpn, e, s)) return 0;
					rpn.push(s - e);
					break;
					
				case '*':
					if(!rpn_top2(rpn, e, s)) return 0;
					rpn.push(s * e);
					break;
					
				case '/':
					if(!rpn_top2(rpn, e, s) || e == 0) return 0;
					rpn.push(s / e);
					break;
					
				case '%':
					if(!rpn_top2(rpn, e, s) || e == 0) return 0;
					rpn.push(static_cast<int64_t>(s) % static_cast<int64_t>(e));
					break;
					
				case '^':
					if(!rpn_top2(rpn, e, s)) return 0;
					rpn.push(pow(s, e));
					break;
			}
		}
	}
	
	//如果计算完毕后stack剩余的成员数不为1则失败
	if(rpn.size() != 1) return 0;
	
	return rpn.top();
}

/*
** 将一个数学公式构造为逆波兰表达式
** @param _math_exp 表达式串
** @param _exp_len 表达式串的长度
** @return 返回逆波兰表达式 */
std::string make_rpn(const char* _math_exp, size_t _exp_len)
{
	bool first = true;
	
	std::stack<char> rpn_op;
	std::string result;
	
	for(size_t p = 0; p < _exp_len; ++p)
	{
		if(_math_exp[p] == ' ') continue;
		
		//开头第一个有效符号
		if(first)
		{
			//如果开头第一个有效符号是+或者-，则在开头补一个0 
			if(_math_exp[p] == '-' || _math_exp[p] == '+')
			{
				result.append(1, '0');
				result.append(1, ' ');
			}
			
			first = false;
		}
		
		//用于标识是否追加过数字，若追加过数字，则需要在完成后在末尾插入空格
		bool flag = false;
		while((_math_exp[p] >= '0' && _math_exp[p] <= '9') || _math_exp[p] == '.' || _math_exp[p] == ' ' )
		{
			if(_math_exp[p] != ' ')
			{
				result.append(1, _math_exp[p]);
				flag = true;
			}
			
			if(++p >= _exp_len) break;
		}
		
		if(flag) result.append(1, ' ');
		if(p >= _exp_len) break;
		
		//临时变量，用于记录顶栈数据
		char top_ch;
		
		if(_math_exp[p] == ')')
		{
			while(!rpn_op.empty())
			{
				top_ch = rpn_op.top();
				rpn_op.pop();
				if(top_ch == '(') break;
				result.append(1, top_ch);
				result.append(1, ' ');
			}
		}
		else if(_math_exp[p] == '+' || _math_exp[p] == '-')
		{
			//Stack为空，直接push 
			if(rpn_op.empty())
			{
				rpn_op.push(_math_exp[p]);
			}
			else
			{
				while(!rpn_op.empty())
				{
					top_ch = rpn_op.top();
					//除非找到优先级比加减法小的，否则全部出栈
					//左括号无优先级
					if(top_ch == '(') break;
					
					result.append(1, top_ch);
					result.append(1, ' ');
					rpn_op.pop();
					
				}
				
				rpn_op.push(_math_exp[p]);
			}
		}
		else if(_math_exp[p] == '(')
		{
			//右括号，无条件直接加入
			rpn_op.push(_math_exp[p]);
		}
		else if(_math_exp[p] == '*' || _math_exp[p] == '/' || _math_exp[p] == '%' || _math_exp[p] == '^')
		{
			while(!rpn_op.empty())
			{
				top_ch = rpn_op.top();
				//除非找到优先级比即将加入的操作符小的，否则全部出栈
				if(top_ch == '(' || top_ch == '+' || top_ch == '-') break;
				
				result.append(1, top_ch);
				result.append(1, ' ');
				rpn_op.pop();
			}
				
			rpn_op.push(_math_exp[p]);
		}
		
	}
	
	while(!rpn_op.empty())
	{
		result.append(1, rpn_op.top());
		result.append(1, ' ');
		rpn_op.pop();
	}
	
	return result;
}

double calculate_expr(const char* _expr)
{
	std::string rpn_exp = make_rpn(_expr, strlen(_expr));
	return calculate_rpn(rpn_exp.c_str(), rpn_exp.length());
}


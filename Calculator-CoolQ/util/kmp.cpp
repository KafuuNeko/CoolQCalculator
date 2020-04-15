#include "kmp.h"

#include <string.h>

size_t* KMP_MakeNext(const char* substr, size_t len, bool optimize)
{
	size_t* next = new size_t[len];
	next[0] = util_kmp::npos;

	size_t j = 0;
	size_t k = next[j];

	while (j < len - 1)
	{

		if (k == util_kmp::npos || substr[j] == substr[k])
		{
			++j;

			if (k == util_kmp::npos)
			{
				k = 0;
			}
			else
			{
				++k;
			}

			if (optimize && substr[j] == substr[k])
			{
				next[j] = next[k];
			}
			else
			{
				next[j] = k;
			}

		}
		else
		{
			k = next[k];
		}
	}

	return next;
}

size_t util_kmp::KMP_Find(const char* findstr, const char* substr)
{
	size_t find_len = strlen(findstr), subs_len = strlen(substr);

	if (subs_len == 0 || find_len < subs_len) return util_kmp::npos;

	size_t* next = KMP_MakeNext(substr, subs_len, false);

	size_t j = 0, k = 0;
	while (j < find_len && (k == npos || k < subs_len))
	{

		if (k == npos || findstr[j] == substr[k])
		{
			//指示后移，或匹配成功 
			++j;

			if (k == npos)
			{
				k = 0;
			}
			else
			{
				++k;
			}

		}
		else
		{
			k = next[k];
		}

	}

	delete[] next;

	if (k == subs_len)
	{
		return j - k;
	}
	else
	{
		return npos;
	}

}

size_t util_kmp::KMP_Find_Count(const char* findstr, const char* substr)
{
	size_t find_len = strlen(findstr), subs_len = strlen(substr);

	if (subs_len == 0 || find_len < subs_len) return 0;

	size_t* next = KMP_MakeNext(substr, subs_len, false);

	size_t count = 0;

	size_t j = 0, k = 0;
	while (j < find_len)
	{

		if (k == npos || findstr[j] == substr[k])
		{
			//指示后移，或匹配成功 
			++j;

			if (k == npos)
			{
				k = 0;
			}
			else
			{
				++k;
			}

			if (k == subs_len)
			{
				//子串匹配成功，计数+1，并回溯到匹配成功的位置，继续匹配 
				++count;
				k = next[k - 1];
				--j;
			}
		}
		else
		{
			k = next[k];
		}

	}

	delete[] next;

	return count;
}



#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <iostream>

using namespace std;

class CharVector
{
public:
	CharVector(): base_ptr(nullptr), len(0), cap(0)
	{
		reserve(1024);
	}
	CharVector(size_t n) : base_ptr(nullptr), len(0), cap(0)
	{
		reserve(n);
	}
	~CharVector()
	{
		if (base_ptr)
			free(base_ptr);
	}

	inline char * data() { return base_ptr; }
	inline size_t size() { return len;  }
	inline size_t capacity() { return cap;  }
	bool push_back(char ch) 
	{
		if (len < cap)
		{
			base_ptr[len] = ch;
			len++;
			return true;
		}
		else
		{
			size_t sz = cap;
			if (cap > 10 * 1024 * 1024)  // 10MB
			{
				sz = cap + 1024 * 1024;
			}
			else
			{
				sz = cap * 2;
			}
			bool ret = reserve(sz);
			if (ret == false)
				return false;


			base_ptr[len] = ch;
			len++;
			return true;
		}
	}

	inline void clear() { len = 0; }

	bool append(char * buf, size_t n)
	{
		bool ret = reserve(size() + n);
		if (ret == false)
			return false;
		memcpy(base_ptr + len, buf, n);
		len += n;

		return true;
	}

	bool reserve(size_t n) {

		size_t sz = 64;
		if (sz < cap)
			sz = cap;
		while (sz < n)
		{
			if (sz > 10 * 1024 * 1024)  // 10MB
			{
				sz = sz + 1024 * 1024;
			}
			else
			{
				sz = sz * 2;
			}
		}
		bool ret = allocMem(sz);

		//cout <<" reserve : "<< size() << "," << capacity() << endl;

		return ret;
	}

	inline bool resize(size_t n)
	{
		if (n > cap)
			if (reserve(n) == false)
				return false;
		len = n;
		return true;
	}

	bool copyTo(CharVector & other)
	{
		bool ret = other.reserve(this->size());
		if (ret == false)
			return false;
		memcpy(other.data(), this->data(), this->size());
		other.resize(this->size());
		return true;
	}

private:
	inline bool allocMem(size_t n)
	{
		if (base_ptr == nullptr)
		{
			base_ptr = (char *)malloc(n);
			if (base_ptr == nullptr)
				return false;

			cap = n;
			return true;
		}

		if (n > cap)
		{
			void *new_ptr = realloc(base_ptr, n);
			if (new_ptr == nullptr)
			{
				new_ptr = malloc(n);
				if (new_ptr == nullptr)
					return false;

				memcpy(new_ptr, base_ptr, len);
				cap = n;
				free(base_ptr);
				base_ptr = (char *)new_ptr;
				return true;
			}
			else
			{
				cap = n;
				if (base_ptr != new_ptr) // 
				{
					//must not call free(base_ptr), because realloc has done!!!
					base_ptr = (char *)new_ptr;
				}
				return true;
			}	
		}

		// n < cap, don't do anything
		return true;

	}
private:
	char * base_ptr;
	size_t len;
	size_t cap;

};


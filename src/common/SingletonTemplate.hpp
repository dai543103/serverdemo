#ifndef __SINGLETON_TEMPLATE_HPP__
#define __SINGLETON_TEMPLATE_HPP__
#include <stdio.h>
#include <stdlib.h>


template <class TYPE>
class CSingleton
{
public:
	static TYPE* Instance(void)
	{
		if(m_pSingleton == nullptr)
		{
			m_pSingleton = new CSingleton;
		}
		return &m_pSingleton->m_stInstance;
	}
protected:
	CSingleton() {}
protected:
	TYPE m_stInstance;
	static CSingleton<TYPE>* m_pSingleton;
};

template <class TYPE>
CSingleton<TYPE>* CSingleton<TYPE>::m_pSingleton = nullptr;

#endif
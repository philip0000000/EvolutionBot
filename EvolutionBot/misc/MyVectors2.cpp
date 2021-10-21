#include "config.h"
#ifdef HAVE_MY_VECTORS


#ifndef MY_VECTORS2_CPP
#define MY_VECTORS2_CPP


#include "MyVectors2.h"
#include <assert.h>
#include <new>


template <typename T>
MyVector<T>::MyVector() :
	m_pData(nullptr),
	m_nSize(0),
	m_nMaxSize(0)
{
}
template <typename T>
MyVector<T>::~MyVector()
{
	if (m_pData != nullptr)
		delete[] m_pData;
}

template <typename T>
T& MyVector<T>::operator[](int nIndex) const
{
	assert(m_nSize > 0 && nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}
template <typename T>
T& MyVector<T>::operator[](int nIndex)
{
	assert(m_nSize > 0 && nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

template <typename T>
void MyVector<T>::resize(const int& Newsize)
{
	if (Newsize >= m_nMaxSize) //reallocate array if size to small
	{
		int newSize = Newsize * 2;
		T* newArr = new T[newSize];

		if (m_pData != nullptr)
			memcpy(newArr, m_pData, m_nSize * sizeof(T));

		m_nMaxSize = newSize;
		if (m_pData != nullptr)
			delete[] m_pData;
		m_pData = newArr;
	}

	m_nSize = Newsize;
}
template <typename T>
int MyVector<T>::size() const
{
	return m_nSize;
}


#endif /* MY_VECTORS2_CPP */


#endif /* HAVE_MY_VECTORS */

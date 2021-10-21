/*#include "config.h"
#ifdef HAVE_MY_VECTORS


#include "VectorMLPBox.h"
#include <new>
#include <assert.h>
#include "MLPBrain2.h"


VectorMLPBox::VectorMLPBox() :
	m_pData(nullptr),
	m_nSize(0),
	m_nMaxSize(0)
{
}
VectorMLPBox::~VectorMLPBox()
{
	if (m_pData != nullptr)
		delete[] m_pData;
}

MLPBox& VectorMLPBox::operator[](int nIndex) const
{
	assert(m_nSize > 0 && nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}
MLPBox& VectorMLPBox::operator[](int nIndex)
{
	assert(m_nSize > 0 && nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

void VectorMLPBox::push_back(MLPBox& Val)
{
	if (m_nSize >= m_nMaxSize) //reallocate array if size to small
	{
		int newSize = (m_nMaxSize * 2) + 1;
		MLPBox* newArr = new MLPBox[newSize];

		if (m_pData != nullptr)
		{
			memcpy(newArr, m_pData, m_nMaxSize * sizeof(MLPBox));
			fprintf(stdout, "hello from here!!!123\n");
		}
			

		m_nMaxSize = newSize;
		if (m_pData != nullptr)
			delete[] m_pData;
		m_pData = newArr;
	}

	m_pData[m_nSize] = Val;
	++m_nSize;
}
int VectorMLPBox::size() const
{
	return m_nSize;
}


#endif /* HAVE_MY_VECTORS */

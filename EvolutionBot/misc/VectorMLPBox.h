/*#include "config.h"
#ifdef HAVE_MY_VECTORS


#ifndef VECTOR_MLP_BOX_H
#define VECTOR_MLP_BOX_H


//TODO:Move over VectorMLPBox to "MyVectors.h", later!


class MLPBox;


class VectorMLPBox
{
public:
	VectorMLPBox();
	~VectorMLPBox();

	MLPBox& operator[](int nIndex) const;
	MLPBox& operator[](int nIndex);

	void push_back(MLPBox& Val);	// Add a bot to the vector
	int size() const;

private:
	MLPBox*  m_pData;			// the actual array of data
	int      m_nSize;	// number of elements that are being used
	int      m_nMaxSize;	// max allocated
};


#endif /* VECTOR_MLP_BOX_H /


#endif /* HAVE_MY_VECTORS */

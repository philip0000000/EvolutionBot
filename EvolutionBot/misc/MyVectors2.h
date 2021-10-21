#include "config.h"
#ifdef HAVE_MY_VECTORS


#ifndef MY_VECTORS2_H
#define MY_VECTORS2_H



//For: vector<float> vector<int> vector<std::string>
template <typename T>
class MyVector
{
public:
	MyVector();
	~MyVector();

	T& operator[](int nIndex) const;
	T& operator[](int nIndex);

	void resize(const int& Newsize);
	int size() const;

private:
	T *m_pData;
	int m_nSize;
	int m_nMaxSize;
};


#include "MyVectors2.cpp"


#endif /* MY_VECTORS2_H */


#endif /* HAVE_MY_VECTORS */

#include "config.h"
#ifdef HAVE_MY_VECTORS


#ifndef MY_VECTORS_H
#define MY_VECTORS_H


#include "Agent2.h"
#include "MyEnum.hpp"


class VectorAgent
{
public:
	VectorAgent();
	~VectorAgent();

	Agent& operator[](int nIndex) const;
	Agent& operator[](int nIndex);

	void AddAgent(int nID, FeedingStrategi type);	// Add a bot to the vector
	void ReproduceAgent(Agent& a);
	int NrOfBots() const;
	int TotalSizeOfAllocatedSpace() const;

private:
	Agent*   m_pData;			// the actual array of data
	int      m_nNrOfBotsAlive;	// number of elements that are being used
	int      m_nTotalNrOfBots;	// max allocated
};


#endif /* MY_VECTORS_H */


#endif /* HAVE_MY_VECTORS */

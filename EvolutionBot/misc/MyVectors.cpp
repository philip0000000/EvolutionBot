#include "config.h"
#ifdef HAVE_MY_VECTORS


#include "MyVectors.h"
#include "Agent.h"
#include <new>
#include "MyEnum.hpp"


// VectorAgent
VectorAgent::VectorAgent() :
	m_pData(nullptr),
	m_nNrOfBotsAlive(0),
	m_nTotalNrOfBots(0)
{
}
VectorAgent::~VectorAgent()
{
	if (m_pData != nullptr)
		delete[] m_pData;
}

Agent& VectorAgent::operator[](int nIndex) const
{
	assert(m_nTotalNrOfBots > 0 && nIndex >= 0 && nIndex < m_nTotalNrOfBots);
	return m_pData[nIndex];
}
Agent& VectorAgent::operator[](int nIndex)
{
	assert(m_nTotalNrOfBots > 0 && nIndex >= 0 && nIndex < m_nTotalNrOfBots);
	return m_pData[nIndex];
}

void VectorAgent::AddAgent(int nID, FeedingStrategi type)
{
	int nIndex;

	// Find/make space for added bot.
	if (m_nNrOfBotsAlive >= m_nTotalNrOfBots)
	{
		// Reallocate array if size to small.
		int newSize = (m_nTotalNrOfBots * 2) + 1;
		Agent* newArr = new Agent[newSize];

		if (m_pData != nullptr)
			memcpy(newArr, m_pData, m_nTotalNrOfBots * sizeof(Agent));

		m_nTotalNrOfBots = newSize;
		if (m_pData != nullptr)
			delete[] m_pData;
		m_pData = newArr;

		nIndex = m_nNrOfBotsAlive;
	}
	else
	{
		// Find a dead bot that can be recycled.
		for (int m = 0; m < m_nTotalNrOfBots; m++)
			if (m_pData[m].getStatusAgentAlive() == false)
			{
				nIndex = m;
				break;
			}
	}

	m_pData[nIndex].RandomiseAgent(nID, type);

	++m_nNrOfBotsAlive;
}
void VectorAgent::ReproduceAgent(Agent& a)
{
	if (m_nTotalNrOfBots >= m_nNrOfBotsAlive) //reallocate array if size to small
	{
		int newSize = (m_nTotalNrOfBots * 2) + 1;
		Agent* newArr = new Agent[newSize];

		if (m_pData != nullptr)
			memcpy(newArr, m_pData, m_nTotalNrOfBots * sizeof(Agent));

		m_nTotalNrOfBots = newSize;
		if (m_pData != nullptr)
			delete[] m_pData;
		m_pData = newArr;
	}

	m_pData[m_nNrOfBotsAlive] = a;
	++m_nNrOfBotsAlive;
}
int VectorAgent::NrOfBots() const
{
	return m_nNrOfBotsAlive;
}
int VectorAgent::TotalSizeOfAllocatedSpace() const
{
	return m_nTotalNrOfBots;
}


#endif /* HAVE_MY_VECTORS */

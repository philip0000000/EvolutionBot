#include <cstdlib>
#include <ctime>
#include "seed.h"


ProgramSeed::ProgramSeed()
{
	m_nSeed = time(0);
	srand(m_nSeed);
}

void ProgramSeed::setSeed(int newSeed)
{
	m_nSeed = newSeed;
	srand(newSeed);
}
int  ProgramSeed::getSeed()
{
	return m_nSeed;
}

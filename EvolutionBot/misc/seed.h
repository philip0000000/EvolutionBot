#ifndef SEED_H
#define SEED_H


class ProgramSeed
{
public:
	ProgramSeed();
	
	void setSeed(int newSeed);
	int  getSeed();

private:
	int m_nSeed;
};


#endif /* SEED_H */

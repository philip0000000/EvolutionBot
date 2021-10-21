/*#ifndef READWRITE_H
#define READWRITE_H


#include "Agent.h"
#include "World.h"
#include "GLView.h"
#include <stdio.h>
#include "settings.h"


//#include <Windows.h>


class ReadWrite
{
public:
	ReadWrite();

	void loadSettings(const char * filename); //load text settings file

	void saveAgent(Agent * agent, const char * filename); //save a single agent
	void loadAgent(const char * filename);

	void saveWorld(World * world, float xpos, float ypos, const char * filename); //save world to text
	void loadWorld(World * world, float &xtranslate, float &ytranslate, const char * filename); //load world from text
	
	const char * ourfile;
	const char * lastfile;
};


#endif /* READWRITE_H */

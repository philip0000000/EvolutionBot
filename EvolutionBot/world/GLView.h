#ifndef GLVIEW_H
#define GLVIEW_H


#include "config.h"
#include "World.h"
#include "glui.h"
#include "cli.h"
#include "seed.h"


class GLView;


extern GLView* GLVIEW;


void gl_renderScene();
void gl_handleIdle();
void gl_changeSize(int w, int h);

void gl_processNormalKeys(unsigned char key, int x, int y);
void gl_processSpecialKeys(int key, int x, int y);
void gl_processReleasedKeys(unsigned char key, int x, int y);
void gl_processMouse(int button, int state, int x, int y);
void gl_processMouseActiveMotion(int x, int y);
void gl_processMousePassiveMotion(int x, int y);


void gl_menu(int key);


class GLView : public View
{
public:
	GLView(World* w);
	virtual ~GLView();
	
	virtual void drawCell(int x, int y, float quantity);
	virtual void drawAgent(const Agent &a);
	virtual void drawData();
	
	void setWorld(World* w);
	
	//GLUT functions
	void renderScene();
	void handleIdle();
	void changeSize(int w, int h);
	void processNormalKeys(unsigned char key, int x, int y);
	void processSpecialKeys(int key, int x, int y);
	void processReleasedKeys(unsigned char key, int x, int y);
	void processMouse(int button, int state, int x, int y);
	void processMouseActiveMotion(int x, int y);
	void processMousePassiveMotion(int x, int y);
	
	void menu(int key);  // Handle messages from GUI interface from GLUI and most keys.
	void menuS(int key); // Handle arrow keys.
	
	void glCreateMenu();
	int m_id; // main context menu // the window that appears when left clicking on the window that display the simulation.
	int win1; // The window that display the simulation.
	void gluiCreateMenu();
	
private:
    World *world;
	int live_worldclosed; //live variable support via glui
	int live_paused; //are we paused?
	int live_fastmode; //are we drawing?
	int live_skipdraw; //are we skipping some frames?
	int live_agentsvis; //are we drawing agents? If so, what's the scheme? 0= agents hidden, 1= normal, 2= stomach, 3= temp discomfort, 4= sound
	int live_layersvis; //what cell layer is currently active? 0= off, 1= plant food, 2= meat, 3= hazards, 4= fruit, 5= land
	int live_following;
#ifdef HAVE_DEBUG
	int live_debug; //are we debugging?
#endif HAVE_DEBUG
	bool debug;
	GLUI * Menu;

    char buf[100];
    char buf2[10];
    int modcounter; //tick counter
    int lastUpdate;
    int frames;
    
    float scalemult; // How far or close the camera is in the sumulating window.
    float xtranslate, ytranslate;
    int downb[5]; //value is 5 and not 4 to prevent bufferoverflow.
    int mousex, mousey;

	ProgramSeed seed;
	CommandLineInterface cli;
};


#endif /* GLVIEW_H */

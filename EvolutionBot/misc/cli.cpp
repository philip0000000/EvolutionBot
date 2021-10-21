#include "cli.h"
#include "settings.h"
#include <GL/freeglut.h>


void drawHUDString(float x, float y, const char* string)
{
	const char *c;
	glColor3f(0.255f, 0.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, conf::WWIDTH, conf::WHEIGHT, 0.0, -1.0, 10.0);
	glScalef(0.25f, -0.25f, 0.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	for (c = string; *c != '\0'; c++)
	{
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
	}
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}
void drawHUDRectangle(float Point1X, float Point1Y, float Point2X, float Point2Y, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(red, green, blue, alpha);
	glRectf(Point1X, Point1Y, Point2X, Point2Y);

	glDisable(GL_BLEND);
	glPopMatrix();

	/*glPushMatrix();		// Print command line interface grey rectangle.
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.72f, 0.72f, 0.72f, 0.85f);
	glBegin(GL_QUADS);
	glVertex2f(-0.8f, -0.9f);
	glVertex2f(-0.8f, -0.8f);
	glVertex2f(0.8f, -0.8f);
	glVertex2f(0.8f, -0.9f);
	glEnd();
	glDisable(GL_BLEND);
	glPopMatrix();*/
}


CommandLineInterface::CommandLineInterface() :
	m_bCLISelected(false),
	m_fXPosisionInput(300.0f),
	m_fYPosisionInput(-2610.0f),
	m_nMaxInputSize(20)
{
}

void CommandLineInterface::setCLISelected(bool bSelected)
{
	m_bCLISelected = bSelected;
}
bool CommandLineInterface::getCLISelected()
{
	return m_bCLISelected;
}

const std::string CommandLineInterface::getInputLine()
{
	return InputLine;
}

void CommandLineInterface::AddCLITextToBuffer(const char *s)
{
	std::vector<std::string>::iterator it;		// Add new string.
	it = Buffer.begin();
	Buffer.insert(it, s);
	if (Buffer.size() > 12) Buffer.pop_back();	// Delete last string. Also determines max CLI lines.
}
void CommandLineInterface::AddCLIInput(char cInput)
{
	if (InputLine.size() < m_nMaxInputSize)
	{
		InputLine.push_back(cInput);
	}
}
void CommandLineInterface::SubCLIInput()
{
	if (InputLine.size()) InputLine.pop_back();
}

int CommandLineInterface::ProcessInput()
{
	AddCLITextToBuffer(InputLine.c_str());
	
	if (InputLine.size() == 0)
	{
		return 1;
	}
	else if (InputLine == "commands")
	{
		return 2;
	}
	else if (InputLine == "seed") // return current seed.
	{
		return 3;
	}
	else if (InputLine.size() == 15 && InputLine.compare(0, 5, "seed ") == 0) // set new seed value and restart map.
	{
		return 4;
	}
	
	return 0;
}
void CommandLineInterface::NewInputLine()
{
	InputLine.clear();
}

void CommandLineInterface::PrintCLI()
{
	// Print input cli, a grey somewhat transparent rectangle.
	drawHUDRectangle(-0.9f, -0.75f, 0.3f, -0.9f, 0.72f, 0.72f, 0.72f, 0.85f);
	// Print cli history, a grey more transparent rectangle then the input cli rectangle.
	drawHUDRectangle(-0.9f, 0.95f, 0.3f, -0.75f, 0.72f, 0.72f, 0.72f, 0.98f);

	// Print command line interface text.
	drawHUDString(m_fXPosisionInput, m_fYPosisionInput, InputLine.c_str());
	std::vector<std::string>::iterator it;
	it = Buffer.begin();
	for (float y = m_fYPosisionInput + 200.0f; it < Buffer.end(); ++it, y += 200.0f)
	{
		if (it->size()) drawHUDString(m_fXPosisionInput, y, it->c_str());
	}
}

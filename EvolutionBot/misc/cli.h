#ifndef CLI_H
#define CLI_H


#include <vector>
#include <string>


class CommandLineInterface
{
public:
	CommandLineInterface();
	
	void setCLISelected(bool bSelected);
	bool getCLISelected();
	
	const std::string getInputLine();
	
	void AddCLITextToBuffer(const char *s);
	void AddCLIInput(char c);
	void SubCLIInput();
	int ProcessInput();
	void NewInputLine();
	
	void PrintCLI();
	
private:
	float m_fXPosisionInput;
	float m_fYPosisionInput;
	const int m_nMaxInputSize;

	std::vector<std::string> Buffer;
	std::string InputLine;
	
	bool m_bCLISelected;
};


#endif /* CLI_H */

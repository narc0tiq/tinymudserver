#include <stdexcept>

using namespace std;

#include "command.h"
#include "globals.h"

bool AnyCanExecute(tPlayer* p)
{
	return true;
}

tCommand::tCommand(string cmdName, tPermHandler ph, tHandler eh): CanExecute(ph), Execute(eh)
{
	map<std::string, string, ciLess>::const_iterator iter;

	iter = messagemap.find(cmdName + "-shorthelp");

	if(iter == messagemap.end())
		this->shorthelp = "No clue what this does.";
	else
		this->shorthelp = iter->second;

	iter = messagemap.find(cmdName + "-longhelp");

	if(iter == messagemap.end())
		this->longhelp = "No idea, sorry.\n";
	else
		this->longhelp = iter->second;
}

tCommand::tCommand(string cmdName, tHandler eh): CanExecute(AnyCanExecute), Execute(eh)
{
	map<std::string, string, ciLess>::const_iterator iter;

	iter = messagemap.find(cmdName + "-shorthelp");

	if(iter == messagemap.end())
		this->shorthelp = "No clue what this does.";
	else
		this->shorthelp = iter->second;

	iter = messagemap.find(cmdName + "-longhelp");

	if(iter == messagemap.end())
		this->longhelp = "No idea, sorry.\n";
	else
		this->longhelp = iter->second;
}

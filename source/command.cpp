#include <stdexcept>

using namespace std;

#include "command.h"
#include "globals.h"

bool AnyCanExecute(tPlayer* p)
{
	return true;
}

bool TpCanExecute(tPlayer* p)
{
	if(!p->HaveFlag("can_tp") and !p->HaveFlag("admin"))
		return false;

	return true;
}

bool AdminCanExecute(tPlayer* p)
{
	if(!p->HaveFlag("admin"))
		return false;

	return true;
}

bool GaglessCanExecute(tPlayer* p)
{
	if(p->HaveFlag("gagged"))
		return false;

	return true;
}


tCommand::tCommand(string cmdName, tPermHandler ph, tHandler eh): CanExecute(ph), Action(eh)
{
	this->GetHelp(cmdName);
}

tCommand::tCommand(string cmdName, tHandler eh): CanExecute(AnyCanExecute), Action(eh)
{
	this->GetHelp(cmdName);
}

void tCommand::GetHelp(string cmdName)
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

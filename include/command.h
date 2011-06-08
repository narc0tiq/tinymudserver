#ifndef TINYMUDSERVER_COMMAND_H
#define TINYMUDSERVER_COMMAND_H

#include <map>

#include "player.h"

typedef bool(*tPermHandler)(tPlayer * p);

class tCommand
{
	public:
		string shorthelp;
		string longhelp;

		const tPermHandler CanExecute;
		const tHandler Execute;

		tCommand(string sh, string lh, tPermHandler ph, tHandler eh):
			shorthelp(sh), longhelp(lh), CanExecute(ph), Execute(eh) {}
		tCommand(string cmdName, tPermHandler ph, tHandler eh);
		tCommand(string cmdName, tHandler eh);
};

typedef std::map <string, tCommand*> tCommandMap;
typedef tCommandMap::const_iterator tCommandMapIterator;

bool AnyCanExecute(tPlayer* p);

#endif

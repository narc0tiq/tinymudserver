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

		tPermHandler CanExecute;
		tHandler Execute;
};

typedef std::map <string, tCommand*> tCommandMap;
typedef tCommandMap::const_iterator tCommandMapIterator;

bool AnyCanExecute(tPlayer* p);

#endif

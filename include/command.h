#ifndef TINYMUDSERVER_COMMAND_H
#define TINYMUDSERVER_COMMAND_H

#include <map>
#include <stdexcept>

#include "player.h"

typedef bool(*tPermHandler)(tPlayer * p);

class tCommand
{
	private:
		void GetHelp(string cmdName);

	public:
		string shorthelp;
		string longhelp;

		const tPermHandler CanExecute;
		const tHandler Action;

		tCommand(string sh, string lh, tPermHandler ph, tHandler eh):
			shorthelp(sh), longhelp(lh), CanExecute(ph), Action(eh) {}
		tCommand(string cmdName, tPermHandler ph, tHandler eh);
		tCommand(string cmdName, tHandler eh);

		void Execute(tPlayer* p, istream& sArgs)
		{
			if(!this->CanExecute(p))
				throw runtime_error("You cannot do that.");

			this->Action(p, sArgs);
		}
};

typedef std::map <string, tCommand*> tCommandMap;
typedef tCommandMap::const_iterator tCommandMapIterator;

bool AnyCanExecute(tPlayer* p);
bool TpCanExecute(tPlayer* p);
bool AdminCanExecute(tPlayer* p);
bool GaglessCanExecute(tPlayer* p);

#endif

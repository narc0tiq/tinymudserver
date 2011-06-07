/*

 tinymudserver - an example MUD server

 Author:	Nick Gammon
					http://www.gammon.com.au/

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.

*/

// standard library includes ...

#include <stdexcept>
#include <iostream>
#include <stdlib.h>

using namespace std;

#include "utils.h"
#include "constants.h"
#include "strings.h"
#include "player.h"
#include "room.h"
#include "globals.h"

void NoMore(tPlayer * p, istream & sArgs)
{
	string sLine;
	getline(sArgs, sLine);
	if(!sLine.empty())
		throw runtime_error("Unexpected input: " + sLine);
} // end of NoMore

// helper function for say, tell, chat, etc.
string GetMessage(istream & sArgs, const string & noMessageError)
{
	string message;
	sArgs >> ws;	// skip leading spaces
	getline(sArgs, message); // get rest of line
	if(message.empty()) // better have something
		throw runtime_error(noMessageError);
	return message;
} // end of GetMessage

// helper function for get a flag
string GetFlag(istream & sArgs, const string & noFlagError)
{
	string flag;
	sArgs >> ws >> flag;
	if(flag.empty())
		throw runtime_error(noFlagError);
	if(flag.find_first_not_of(valid_player_name) != string::npos)
		throw runtime_error("Flag name not valid.");
	return flag;
} // end of GetFlag

void PlayerToRoom(tPlayer * p,			 // which player
	const int & vnum, // which room
	const string & sPlayerMessage,	// what to tell the player
	const string & sOthersDepartMessage,	// tell people in original room
	const string & sOthersArrriveMessage) // tell people in new room
{
	tRoom * r = FindRoom(vnum); // find the destination room(throws exception if not there)

	if(r == NULL)
		throw runtime_error("Can't move player: you said /WHAT/ room?");

	SendToAll(sOthersDepartMessage, p, p->room);	// tell others where s/he went
	p->room = vnum;	// move to new room
	*p << sPlayerMessage; // tell player
	p->DoCommand("look");	 // look around new room
	SendToAll(sOthersArrriveMessage, p, p->room);	// tell others ws/he has arrived
} // end of PlayerToRoom

void DoDirection(tPlayer * p, const string & sArgs)
{
	// get current room(throws exception if not there)
	tRoom * r = FindRoom(p->room);

	// find the exit
 tExitMap::const_iterator exititer = r->exits.find(sArgs);

	if(exititer == r->exits.end())
		throw runtime_error("You cannot go that way.");

	// move player
	PlayerToRoom(p, exititer->second,
								"You go " + sArgs + "\n",
								p->playername + " goes " + sArgs + "\n",
								p->playername + " enters.\n");

} // end of DoDirection

/* quit */

void DoQuit(tPlayer * p, istream & sArgs)
{
	NoMore(p, sArgs);	// check no more input

	/* if s/he finished connecting, tell others s/he has left */

	if(p->connstate == ePlaying)
	{
		*p << "See you next time!\n";
		cout << "Player " << p->playername << " has left the game.\n";
		SendToAll("Player " + p->playername + " has left the game.\n", p);
	} /* end of properly connected */

	p->ClosePlayer();
} // end of DoQuit

/* look */

void DoLook(tPlayer * p, istream & sArgs)
{
	// TODO: add: look(thing)
	NoMore(p, sArgs); // check no more input

	// find our current room, throws exception if not there
	tRoom * r = FindRoom(p->room);

	*p << r->roomname << " (" << p->room << ")\n";
	*p << r->description;

	// show available exits
	if(!r->exits.empty())
	{
		*p << "Exits: ";
		for(tExitMap::const_iterator exititer = r->exits.begin();
				 exititer != r->exits.end(); ++exititer)
			*p << exititer->first << " ";
		*p << "\n";
	}

	/* list other players in the same room */

	int iOthers = 0;
	for(tPlayerListIterator listiter = playerlist.begin();
			listiter != playerlist.end();
			listiter++)
	{
		tPlayer *otherp = *listiter;
		if(otherp != p && /* we don't see ourselves */
				otherp->IsPlaying() &&
				otherp->room == p->room)	// need to be in same room
		{
			if(iOthers++ == 0)
				*p << "You also see ";
			else
				*p << ", ";
			*p << otherp->playername;
		}
	}	 /* end of looping through all players */

	/* If we listed anyone, finish up the line with a period, newline */
	if(iOthers)
		*p << ".\n";
} // end of DoLook

/* say <something> */

void DoSay(tPlayer * p, istream & sArgs)
{
	if(p->HaveFlag("gagged") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	string what = GetMessage(sArgs, "Say what?");	// what
	*p << "You say, \"" << what << "\"\n";	// confirm
	SendToAll(p->playername + " says, \"" + what + "\"\n",
						p, p->room);	// say it
} // end of DoSay

/* tell <someone> <something> */
void DoTell(tPlayer * p, istream & sArgs)
{
	if(p->HaveFlag("gagged") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	tPlayer * ptarget = p->GetPlayer(sArgs, "Tell whom?", true);	// who
	string what = GetMessage(sArgs, "Tell " + p->playername + " what?");	// what
	*p << "You tell " << ptarget->playername << ", \"" << what << "\"\n";		 // confirm
	*ptarget << p->playername << " tells you, \"" << what << "\"\n";		// tell them
} // end of DoTell

void DoSave	(tPlayer * p, istream & sArgs)
{
	p->Save();
	*p << "Saved.\n";
}

void DoChat(tPlayer * p, istream & sArgs)
{
	if(p->HaveFlag("gagged") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	string what = GetMessage(sArgs, "Chat what?");	// what
	SendToAll(p->playername + " chats, \"" + what + "\"\n");	// chat it
}

void DoEmote(tPlayer * p, istream & sArgs)
{
	string what = GetMessage(sArgs, "Emote what?");	// what
	SendToAll(p->playername + " " + what + "\n", 0, p->room);	// emote it
}

void DoWho(tPlayer * p, istream & sArgs)
{
	NoMore(p, sArgs);	// check no more input
	*p << "Connected players ...\n";

	int count = 0;
	for(tPlayerListIterator iter = playerlist.begin();
			 iter != playerlist.end();
			 ++iter)
	{
		tPlayer * pTarget = *iter;		// the player
		if(pTarget->IsPlaying())
		{
			*p << "	" << pTarget->playername <<
						" in room " << pTarget->room << "\n";
			++count;
		} // end of if playing
	} // end of doing each player

	*p << count << " player(s)\n";
} // end of DoWho

void DoSetFlag(tPlayer * p, istream & sArgs)
{
	if(!p->HaveFlag("can_setflag") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	tPlayer * ptarget = p->GetPlayer(sArgs, "Usage: setflag <who> <flag>");	// who
	string flag = GetFlag(sArgs, "Set which flag?"); // what
	NoMore(p, sArgs);	// check no more input
	if(ptarget->flags.find(flag) != ptarget->flags.end())		// check not set
		throw runtime_error("Flag already set.");

	ptarget->flags.insert(flag);	 // set it
	*p << "You set the flag '" << flag << "' for " << ptarget->playername << "\n";	// confirm

} // end of DoSetFlag

void DoClearFlag(tPlayer * p, istream & sArgs)
{
	if(!p->HaveFlag("can_setflag") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	tPlayer * ptarget = p->GetPlayer(sArgs, "Usage: clearflag <who> <flag>");	// who
	string flag = GetFlag(sArgs, "Clear which flag?"); // what
	NoMore(p, sArgs);	// check no more input
	if(ptarget->flags.find(flag) == ptarget->flags.end())		// check set
		throw runtime_error("Flag not set.");

	ptarget->flags.erase(flag);		// clear it
	*p << "You clear the flag '" << flag << "' for " << ptarget->playername << "\n";	// confirm

} // end of DoClearFlag

void DoShutdown(tPlayer * p, istream & sArgs)
{
	NoMore(p, sArgs); // check no more input

	if(!p->HaveFlag("can_shutdown") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	SendToAll(p->playername + " shuts down the game\n");
	bStopNow = true;
} // end of DoShutdown

void DoHelp(tPlayer * p, istream & sArgs)
{
	NoMore(p, sArgs);	// check no more input
	*p << messagemap ["help"];
} // end of DoHelp

void DoTeleport(tPlayer * p, istream & sArgs)
{
	if(!p->HaveFlag("can_transfer") and !p->HaveFlag("admin"))
		throw runtime_error("You are not permitted to do that.");

	tPlayer* pTarget;
	string name;
	int room;

	sArgs >> name;

	if(ciStringEqual(name, "me") || ciStringEqual(name, "self"))
		pTarget = p;
	else
		pTarget = FindPlayer(name);

	if( pTarget == NULL )
	{
		room = atoi( name.c_str() );
		pTarget = p;
	}
	else
		sArgs >> room;

	if((sArgs.fail()) || (room == 0))
		throw runtime_error("Usage: tp [who] <where>");

	NoMore(p, sArgs); // check no more input

	string playerMsg = "You teleport to another room!\n";
	if(pTarget != p)
	{
		*p << "You teleport " << pTarget->playername << " to another room!\n";
		playerMsg = "You get teleported to another room!\n";
	}

	PlayerToRoom(pTarget, room, playerMsg,
		"You blink, and " + pTarget->playername + " has vanished!\n",
		pTarget->playername + " suddenly pops up in the middle of the room!\n");
}

void DoInfo(tPlayer * p, istream & sArgs)
{
	tPlayer * ptarget = p->GetPlayer (sArgs, "Usage: info <who>");  // who
	NoMore(p, sArgs);  // check no more input
	*p << "Flags: for " << ptarget->playername << " : ";
	copy(ptarget->flags.begin(), ptarget->flags.end(), player_output_iterator<string>(*p, " "));
	*p << "\n";
} // end of DoShowFlags
/* process commands when player is connected */

void ProcessCommand(tPlayer * p, istream & sArgs)
{
	string command;
	sArgs >> command >> ws;	 // get command, eat whitespace after it

	// first see if they have entered a movement command(eg. n, s, e, w)
	set<string>::const_iterator direction_iter = directionset.find(command);
	if(direction_iter != directionset.end())
		DoDirection(p, command);
	else
	{
		// otherwise, look up command in commands map
		map<string, tHandler>::const_iterator command_iter = commandmap.find(command);
		if(command_iter == commandmap.end())
			throw runtime_error("Huh?");

		command_iter->second(p, sArgs); // execute command(eg. DoLook)
	}
} /* end of ProcessCommand */


void LoadCommands()
{
	commandmap["look"]      = DoLook;       // look around
	commandmap["l"]         = DoLook;       // synonymm for look
	commandmap["quit"]      = DoQuit;       // bye bye
	commandmap["say"]       = DoSay;        // say something
	commandmap["\""]        = DoSay;        // synonym for say
	commandmap["tell"]      = DoTell;       // tell someone
	commandmap["shut"]      = DoShutdown;   // shut MUD down
	commandmap["help"]      = DoHelp;       // show help message
	commandmap["setflag"]   = DoSetFlag;    // set a player's flag
	commandmap["clearflag"] = DoClearFlag;  // clear a player's flag
	commandmap["save"]      = DoSave;       // save a player
	commandmap["chat"]      = DoChat;       // chat
	commandmap["emote"]     = DoEmote;      // emote
	commandmap["/me"]       = DoEmote;      // synonym for emote
	commandmap["who"]       = DoWho;        // who is on?
	commandmap["info"]      = DoInfo;       // Show information on the player.
	commandmap["tp"]        = DoTeleport;
} // end of LoadCommands

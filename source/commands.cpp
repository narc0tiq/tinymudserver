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
#include <iomanip>

using namespace std;

#include "utils.h"
#include "constants.h"
#include "strings.h"
#include "player.h"
#include "room.h"
#include "globals.h"
#include "command.h"

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
	sArgs >> ws; // skip leading spaces
	getline(sArgs, message); // get rest of line
	if(message.empty()) // better have something
		throw runtime_error(noMessageError);
	return message;
} // end of GetMessage

// fetch a word out of a stream, with configurable error messages
string GetWord(istream& sArgs, const string& noWordError, const string& badWordError)
{
	string theword;
	sArgs >> ws >> theword;
	if(theword.empty())
		throw runtime_error(noWordError);
	if(theword.find_first_not_of(valid_player_name) != string::npos)
		throw runtime_error(badWordError);
	return theword;
}

void PlayerToRoom(tPlayer * p, // which player
	const int & vnum, // which room
	const string & sPlayerMessage, // what to tell the player
	const string & sOthersDepartMessage, // tell people in original room
	const string & sOthersArrriveMessage) // tell people in new room
{
	tRoom* r = FindRoom(vnum); // find the destination room(throws exception if not there)

	if(r == NULL)
		throw runtime_error("Can't move player: you said /WHAT/ room?");

	SendToAll(sOthersDepartMessage, p, p->room); // tell others where s/he went
	p->room = vnum; // move to new room
	*p << sPlayerMessage; // tell player
	p->DoCommand("/look"); // look around new room
	SendToAll(sOthersArrriveMessage, p, p->room); // tell others ws/he has arrived
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
	string what = GetMessage(sArgs, "Say what?");	// what
	*p << "You say, \"" << what << "\"\n";	// confirm
	SendToAll(p->playername + " says, \"" + what + "\"\n",
		p, p->room); // say it
} // end of DoSay

/* tell <someone> <something> */
void DoTell(tPlayer * p, istream & sArgs)
{
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

void DoShout(tPlayer * p, istream & sArgs)
{
	string what = GetMessage(sArgs, "Shout what?");
	SendToAll(p->playername + " shouts, \"" + what + "\"\n");
}

void DoEmote(tPlayer * p, istream & sArgs)
{
	string what = GetMessage(sArgs, "Emote what?");
	SendToAll(p->playername + " " + what + "\n", 0, p->room);
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

void DoFlag(tPlayer * p, istream & sArgs)
{
	string action = GetWord(sArgs, "Usage: /flag <set|clear> <flag> [who]", "Eh?");
	bool set = false;
	if(ciStringEqual(action, "set"))
		set = true;
	else if(!ciStringEqual(action, "clear"))
		throw runtime_error("Well, do you want to [yellow]set[/color] or [yellow]clear[/color] a flag?");

	string flag = GetWord(sArgs, "Which flag?", "Not a valid flag.");

	tPlayer* pTarget;
	string targetname;
	sArgs >> ws >> targetname;

	if(targetname.empty())
		pTarget = p;
	else
		pTarget = p->GetPlayer(targetname, "You should never see this", false);

	NoMore(p, sArgs);

	if(set)
	{
		if(pTarget->flags.find(flag) != pTarget->flags.end())
			throw runtime_error("Flag already set.");

		pTarget->flags.insert(flag);
		*p << "You set the flag '" << flag << "' for " << pTarget->playername << "\n";
		*pTarget << "You now have the flag '" << flag << "'\n";
	}
	else
	{
		if(pTarget->flags.find(flag) == pTarget->flags.end())
			throw runtime_error("Flag already cleared.");

		pTarget->flags.erase(flag);
		*p << "You clear the flag '" << flag << "' for " << pTarget->playername << "\n";
		*pTarget << "You no longer have the flag '" << flag << "'\n";
	}
}

void DoRoomAdd(tPlayer* p, istream& sArgs)
{
	int vnum;
	sArgs >> vnum;

	if((sArgs.fail()) || (vnum == 0))
		throw runtime_error("[bold]WHAT[/color] room do you want to add?");

	if(roommap[vnum] != NULL)
		throw runtime_error("That room already exists!");

	tRoom* newroom = new tRoom();
	roommap[vnum] = newroom;
	newroom->SaveRoom(vnum);

	*p << "Room " << vnum << " has been created!\n";
}

void DoRoomDelete(tPlayer* p, istream& sArgs)
{
	int vnum;
	sArgs >> vnum;

	if((sArgs.fail()) || (vnum == 0))
		throw runtime_error("[bold]WHAT[/color] room do you want to delete?");

	if(roommap[vnum] == NULL)
		throw runtime_error("That room doesn't exist!");

	if(!RoomIsEmpty(vnum))
		throw runtime_error("You can't delete a room with PEOPLE in it!");

	tRoom* theroom = roommap[vnum];
	roommap.erase(vnum);
	delete(theroom);

	DeleteRoom(vnum);

	*p << "Room " << vnum << " has been deleted!\n";
}

void DoRoomRename(tPlayer* p, istream& sArgs)
{
	int vnum = p->room;

	if(roommap[vnum] == NULL)
		throw runtime_error("You can't rename a room that doesn't exist! What are you doing here, anyway?");

	string newname = GetMessage(sArgs, "What do you want to rename the room to?");

	roommap[vnum]->roomname = newname;
	roommap[vnum]->SaveRoom(vnum);

	*p << "Room " << vnum << " is now named '" << newname << "'!\n";
}

void DoRoomDescribe(tPlayer* p, istream& sArgs)
{
	int vnum = p->room;

	if(roommap[vnum] == NULL)
		throw runtime_error("You can't describe a room that doesn't exist! What are you doing here, anyway?");

	string firstword;
	sArgs >> firstword >> ws;

	if(sArgs.fail()) // just /room desc by itself
	{
		string desc = roommap[vnum]->description;

		desc = FindAndReplace(desc, "]", "\\]");

		*p << "Current description: \n" << desc << "\n";
	}
	else if(ciStringEqual(firstword, "+")) // /room desc + add a line
	{
		string newline = GetMessage(sArgs, "You need to specify a new line of text, if you're going to do that!");

		roommap[vnum]->description += newline + "\n";
		*p << "Description updated!\n";
		p->DoCommand("/room desc");

		roommap[vnum]->SaveRoom(vnum);
	}
	else if(ciStringEqual(firstword, "-")) // /room desc - delete text or clear description
	{
		string newline;
		getline(sArgs, newline);

		if(newline.empty()) // Want to clear the whole damn thing?
			roommap[vnum]->description = "";
		else
		{
			string desc = roommap[vnum]->description;

			desc = FindAndReplace(desc, newline, "");
			desc = FindAndReplace(desc, "  ", " ");
			desc = FindAndReplace(desc, "\n\n", "\n");

			roommap[vnum]->description = desc;
		}

		*p << "Description updated!\n";
		p->DoCommand("/room desc");

		roommap[vnum]->SaveRoom(vnum);
	}
	else // not a +, not a -, but definitely words -- user wants to replace description.
	{
		string newline;
		getline(sArgs, newline);
		newline = firstword + " " + newline;

		roommap[vnum]->description = newline;

		*p << "Description updated!\n";
		p->DoCommand("/room desc");

		roommap[vnum]->SaveRoom(vnum);
	}
}

void DoRoomExit(tPlayer* p, istream& sArgs)
{
	string direction = GetWord(sArgs, "Usage: /room exit direction [target]", "That's not a direction!");

	int target = 0;
	sArgs >> ws >> target;

	tRoom* room = roommap[p->room];

	if(room == NULL)
		throw runtime_error("You're not in a room! How did you get here?");

	if(sArgs.fail() || (target == 0)) // deleting exit
	{
		tExitMapIterator iter = room->exits.find(direction);

		if(iter == room->exits.end())
			throw runtime_error("You can't delete an exit that doesn't exist!");

		room->exits.erase(direction);
		room->SaveRoom(p->room);

		*p << "There is no longer an exit from this room in the '" << direction << "' direction.\n";
	}
	else
	{
		LoadRoom(target);

		if(roommap[target] == NULL)
			throw runtime_error("No, you can't make an exit to a room that doesn't exist! Bad " + p->playername + "!");

		room->exits[direction] = target;

		room->SaveRoom(p->room);
	}
}

void DoRoom(tPlayer* p, istream& sArgs)
{
	string action = GetWord(sArgs, "You forgot the verb. Do /help /room if you need it.", "Eh?");

	if(ciStringEqual(action, "add"))
		return DoRoomAdd(p, sArgs);
	else if(ciStringEqual(action, "delete"))
		return DoRoomDelete(p, sArgs);
	else if(ciStringEqual(action, "name"))
		return DoRoomRename(p, sArgs);
	else if(ciStringEqual(action, "desc"))
		return DoRoomDescribe(p, sArgs);
	else if(ciStringEqual(action, "exit"))
		return DoRoomExit(p, sArgs);

	throw runtime_error("Sorry, I have no idea what to do for '/room " + action + "'");
}

void DoShutdown(tPlayer * p, istream & sArgs)
{
	NoMore(p, sArgs); // check no more input

	SendToAll(p->playername + " shuts down the game\n");
	bStopNow = true;
} // end of DoShutdown

void DoHelp(tPlayer * p, istream & sArgs)
{
	string cmd;

	sArgs >> cmd;

	NoMore(p, sArgs); // check no more input

	if(cmd.empty()) // just /help by itself, no argument
	{
		for(tCommandMapIterator iter = commandmap.begin(); iter != commandmap.end(); ++iter)
		{
			if(!iter->second->CanExecute(p))
				continue;

			*p << setiosflags(ios::left) << setw(15) << iter->first << iter->second->shorthelp << "\n";
		}
	}
	else
	{
		tCommandMapIterator iter = commandmap.find(cmd);

		if(iter == commandmap.end())
			throw runtime_error("No such command: " + cmd);

		*p << iter->second->longhelp << "\n";
	}
} // end of DoHelp

void DoTeleport(tPlayer * p, istream & sArgs)
{
	tPlayer* pTarget;
	string name;
	int room = 0;

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

void DoNothing(tPlayer * p, istream & sArgs)
{
	return; // Do nothing, like the title says.
}

void ProcessCommand(tPlayer * p, istream& sArgs)
{
	string command;
	streampos origin = sArgs.tellg();
	sArgs >> command >> ws; // get command, eat whitespace after it

	// First, is it a command?
	tCommandMapIterator command_iter = commandmap.find(command);
	if(command_iter != commandmap.end())
		return command_iter->second->Execute(p, sArgs);

	// If not, is it a direction in the current room?
	tRoom* r = FindRoom(p->room);
	tExitMapIterator iter = r->exits.find(command);

	if(iter != r->exits.end())
		return DoDirection(p, command);

	// If not, then it's a /say
	sArgs.clear(); // Clear EOF bit
	sArgs.seekg(origin);
	commandmap["/say"]->Execute(p, sArgs);

} /* end of ProcessCommand */


void LoadCommands()
{
	commandmap["/look"]      = new tCommand("/look",                         DoLook);
	commandmap["/l"]         = commandmap["/look"];
	commandmap["/help"]      = new tCommand("/help",                         DoHelp);
	commandmap["/save"]      = new tCommand("/save",                         DoSave);
	commandmap["/quit"]      = new tCommand("/quit",                         DoQuit);
	commandmap["/me"]        = new tCommand("/me",                           DoEmote);
	commandmap["/who"]       = new tCommand("/who",                          DoWho);
	commandmap["/tp"]        = new tCommand("/tp",        TpCanExecute,      DoTeleport);
	commandmap["/flag"]      = new tCommand("/flag",      AdminCanExecute,   DoFlag);
	commandmap["/shutdown"]  = new tCommand("/shutdown",  AdminCanExecute,   DoShutdown);
	commandmap["/info"]      = new tCommand("/info",      AdminCanExecute,   DoInfo);
	commandmap["/say"]       = new tCommand("/say",       GaglessCanExecute, DoSay);
	commandmap["\""]         = commandmap["/say"];
	commandmap["/tell"]      = new tCommand("/tell",      GaglessCanExecute, DoTell);
	commandmap["/whisper"]   = commandmap["/tell"];
	commandmap["/msg"]       = commandmap["/tell"];
	commandmap["/m"]         = commandmap["/tell"];
	commandmap["/shout"]     = new tCommand("/shout",     GaglessCanExecute, DoShout);
	commandmap["/room"]      = new tCommand("/room",      AdminCanExecute,   DoRoom);
} // end of LoadCommands

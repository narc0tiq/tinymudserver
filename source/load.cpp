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

#include <limits>
#include <fstream>
#include <iostream>

#include <tinyxml.h>

using namespace std;

#include "utils.h"
#include "globals.h"

void LoadCommands(); // in commands.cpp
void LoadStates(); // in states.cpp

// load things from the control file(directions, prohibited names, blocked addresses)
void LoadControlFile()
{
	// load control file
	ifstream fControl(CONTROL_FILE, ios::in);
	if(!fControl)
	{
		cerr << "Could not open control file: " << CONTROL_FILE << endl;
		return;
		}

	LoadSet(fControl, directionset); // possible directions, eg. n, s, e, w
	LoadSet(fControl, badnameset);	 // bad names for new players, eg. new, quit, look, admin
	LoadSet(fControl, blockedIP);		// blocked IP addresses
} // end of LoadControlFile

// load messages stored on messages file
void LoadMessages()
{
	TiXmlElement* xmleMsgs = 0;
	TiXmlNode* node = 0;
	TiXmlDocument doc( MESSAGES_FILE );
	bool loadOkay = doc.LoadFile();
	
	if( loadOkay )
	{
		TiXmlHandle docHandle( &doc );
		xmleMsgs = docHandle.FirstChild("messages").Element();
		if( xmleMsgs != NULL )
		{
			 for( node = xmleMsgs->FirstChild( "message" );
			      node;
			      node = node->NextSibling( "message" ) )
			{
				
				string sMessageCode, sMessageText;
				
				sMessageCode = node->FirstChild( "name" )->ToElement()->GetText();
				
				sMessageText = node->FirstChild( "text" )->ToElement()->GetText();
				
				messagemap[ tolower(sMessageCode) ] = sMessageText;
			}
		}
	}
	else
	{
		cerr << "Could not open messages file: " << MESSAGES_FILE << endl;
		return;
	}
} // end of LoadMessages

// load rooms and exits
void LoadRooms()
{
	// load rooms file
	ifstream fRooms(ROOMS_FILE, ios::in);
	if(!fRooms)
	{
		cerr << "Could not open rooms file: " << ROOMS_FILE << endl;
		return;
	}

	while(!(fRooms.eof()))
	{
		int vnum;
		fRooms >> vnum;
		fRooms.ignore(numeric_limits<int>::max(), '\n'); // skip rest of this line
		string description;
		getline(fRooms, description);

		// give up if no vnum or description
		if(vnum == 0 || description.empty())
			break;

		// get exits
		string sLine;
		getline(fRooms, sLine);

		// don't have duplicate rooms
		if(roommap [vnum] != 0)
		{
			cerr << "Room " << vnum << " appears more than once in room file" << endl;
			continue;
		}

		tRoom * room = new tRoom(FindAndReplace(description, "%r", "\n") + "\n");
		roommap [vnum] = room;

		// read exits from line(format is: <dir> <vnum> ...	eg. n 1234 s 5678)
		istringstream is(sLine);
		while(is.good())
		{
			string dir;
			int dir_vnum;

			is >> dir;	// direction, eg. n
			is >> dir_vnum >> ws; // vnum, eg. 1234

			if(is.fail())
			{
				cerr << "Bad vnum for exit " << dir << " for room " << vnum << endl;
				continue;
				}

			// direction must be valid(eg. n, s, e, w) or it won't be recognised
			set<string>::const_iterator direction_iter = directionset.find(dir);
			if(direction_iter == directionset.end())
			{
				cerr << "Direction " << dir << " for room " << vnum
						 << " not in list of directions in control file" << endl;
				continue;
				}

			// stop if nonsense
			if(dir.empty() || dir_vnum == 0)
				break;

			room->exits [dir] = dir_vnum;	 // add exit

			} // end of getting each direction
		} // end of read loop

} // end of LoadRooms

void LoadScreens()
{
	string sLine;
	ifstream fScreen(SCREEN_FILE, ios::in);
	
	if(!fScreen)
	{
		cerr << "Could not open rooms file: " << SCREEN_FILE << endl;
		return;
	}
	
	while(!(fScreen.eof()))
	{
		getline(fScreen, sLine);
		screenLayout.append( sLine );
	}
}

// build up our commands map and connection states
void LoadThings()
{

	LoadCommands();
	LoadStates();

	// load files
	LoadControlFile();
	LoadMessages();
	LoadRooms();
	LoadScreens();

} // end of LoadThings

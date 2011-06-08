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

	LoadSet(fControl, directionset);	// possible directions, eg. n, s, e, w
	LoadSet(fControl, badnameset);		// bad names for new players, eg. new, quit, look, admin
	LoadSet(fControl, blockedIP);			// blocked IP addresses
	LoadSet( fControl, posResp );			// Load positive responses
	LoadSet( fControl, negResp );			// Load negative responses
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

				sMessageCode = node->ToElement()->Attribute( "name" );
				sMessageText = node->ToElement()->GetText();

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
	LoadMessages();

	LoadCommands();
	LoadStates();

	// load files
	LoadControlFile();
	LoadScreens();

} // end of LoadThings

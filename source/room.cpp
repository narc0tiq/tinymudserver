/*

 tinymudserver - an example MUD server

 Author:    Nick Gammon
            http://www.gammon.com.au/

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.

*/

// standard library includes ...

#include <stdexcept>

using namespace std;

#include "utils.h"
#include "room.h"
#include "globals.h"

#include "tinyxml.h"

tRoom* FindRoom(const int& vnum)
{
	tRoomMapIterator roomiter = roommap.find (vnum);

	if (roomiter == roommap.end ())
		throw runtime_error (MAKE_STRING("Room number " << vnum << " does not exist."));

	return roomiter->second;
}

tRoom* LoadRoom(const int& vnum)
{
	string roomfile = MAKE_STRING( ROOMS_DIR << vnum << ROOMS_EXT );
	TiXmlDocument doc( roomfile );
	bool loadOkay = doc.LoadFile();

	if( !loadOkay )
	{
		cerr << "Could not open room file \"" << roomfile << "\": " << doc.ErrorDesc() << endl;
		return NULL;
	}

	TiXmlHandle docHandle( &doc );
	TiXmlElement* xmleRoom = docHandle.FirstChild( "room" ).Element();
	if( xmleRoom == NULL )
	{
		cerr << "Invalid room \"" << roomfile << "\": first element not <room>!" << endl;
		return NULL;
	}

	string roomname = xmleRoom->Attribute( "name" );
	string roomdesc = xmleRoom->FirstChild( "description" )->ToElement()->GetText();

	tRoom* newroom = new tRoom(roomname, roomdesc + "\n");

	TiXmlElement* xmleExits = xmleRoom->FirstChild( "exits" )->ToElement();
	if( xmleExits == NULL )
	{
		cerr << "Room file \"" << roomfile << "\" is invalid: No exits! " << endl;
		return NULL;
	}

	for( TiXmlNode* node = xmleExits->FirstChild( "exit" );
		 node;
		 node = node->NextSibling( "exit" ) )
	{
		string direction = node->ToElement()->Attribute( "direction" );
		int target = atoi( node->ToElement()->Attribute( "target" ) );

		if(direction.empty() || target == 0)
		{
			cerr << "Malformed exit in room: " << roomfile << endl;
			break;
		}

		newroom->exits[direction] = target;
	}

	return newroom;
}

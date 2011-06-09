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
#include "npc.h"

#include "tinyxml.h"

void AddNPC( tRoom* room, string NPC, int number )
{
	tNPC *newNPC;

	newNPC = FindNPC( NPC );
}

tRoom* LoadRoom(const int& vnum)
{
	if( roommap[vnum] != NULL)
		return roommap[vnum];

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

	TiXmlElement* xmleExits = xmleRoom->FirstChild( "exits" )->ToElement();
	if( xmleExits == NULL )
	{
		cerr << "Room file \"" << roomfile << "\" is invalid: No exits! " << endl;
		return NULL;
	}

	tRoom* newroom = new tRoom(roomname, roomdesc);

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

//	TiXmlElement* xmleNPCs = xmleRoom->FirstChild( "npcs" )->ToElement();
	TiXmlElement* xmleNPCs = docHandle.FirstChild( "room" ).FirstChild( "npcs" ).Element();
	if( xmleNPCs != NULL )
	{
		printf( "FFS...." );
		// We have some NPC's in this room, so load them up.
		for( TiXmlNode* node = xmleNPCs->FirstChild( "npc" );
			 node;
			 node = node->NextSibling( "npc" ) )
		{
			string NPCname = node->ToElement()->Attribute( "name" );
			int number = atoi( node->ToElement()->Attribute( "number" ) );

			AddNPC( newroom, NPCname, number );
		}
	}

	roommap[vnum] = newroom;

	return newroom;
}

tRoom* FindRoom(const int& vnum)
{
	tRoomMapIterator roomiter = roommap.find( vnum );

	if (roomiter == roommap.end())
	{
		tRoom* room = LoadRoom( vnum );

		if( room == NULL )
			throw runtime_error( MAKE_STRING( "Room number " << vnum << " does not exist." ));

		return room;
	}

	return roomiter->second;
}

void DeleteRoom(const int& vnum)
{
	string roomfile = MAKE_STRING( ROOMS_DIR << vnum << ROOMS_EXT );

	unlink(roomfile.c_str());
}

void tRoom::SaveRoom(const int& vnum)
{
	string roomfile = MAKE_STRING(ROOMS_DIR << vnum << ROOMS_EXT);
	TiXmlDocument doc(roomfile);

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	TiXmlElement* elRoom = new TiXmlElement("room");
	elRoom->SetAttribute("name", this->roomname);
	doc.LinkEndChild(elRoom);

	TiXmlElement* elDesc = new TiXmlElement("description");
	elRoom->LinkEndChild(elDesc);

	TiXmlText* txtDesc = new TiXmlText(this->description);
	txtDesc->SetCDATA(true);
	elDesc->LinkEndChild(txtDesc);

	TiXmlElement* elExits = new TiXmlElement("exits");
	elRoom->LinkEndChild(elExits);

	for(tExitMapIterator iter = this->exits.begin(); iter != this->exits.end(); ++iter)
	{
		TiXmlElement* el = new TiXmlElement("exit");
		el->SetAttribute("direction", iter->first);
		el->SetAttribute("target", iter->second);

		elExits->LinkEndChild(el);
	}

	if(!doc.SaveFile())
		throw runtime_error(MAKE_STRING("Trouble saving the room: " << doc.ErrorDesc()));
}

bool RoomIsEmpty(const int& vnum)
{
	for(tPlayerListIterator iter = playerlist.begin(); iter != playerlist.end(); ++iter)
	{
		if(((tPlayer*)(*iter))->room == vnum)
			return false;
	}

	return true;
}

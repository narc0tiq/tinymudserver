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

#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iterator>
#include <errno.h>

#include "tinyxml.h"

using namespace std;

#include "utils.h"
#include "constants.h"
#include "strings.h"
#include "player.h"
#include "room.h"
#include "globals.h"

/* find a player by name */

tPlayer * FindPlayer(const string & name)
{
	tPlayerListIterator i =
		find_if(playerlist.begin(), playerlist.end(), findPlayerName(name));

	if(i == playerlist.end())
		return NULL;
	else
		return *i;

} /* end of FindPlayer */

// member functions to find another playing, including myself
tPlayer * tPlayer::GetPlayer(istream & args, const string & noNameMessage, const bool & notme)
{
	string name;
	args >> name;
	return this->GetPlayer(name, noNameMessage, notme);
}

tPlayer * tPlayer::GetPlayer(string& name, const string& noNameMessage, const bool& notme)
{
	if(name.empty())
		throw runtime_error(noNameMessage);

	tPlayer * p = this;

	if(!ciStringEqual(name, "me") && !ciStringEqual(name, "self"))
		p = FindPlayer(name);

	if(p == NULL)
		throw runtime_error(MAKE_STRING("Player " << tocapitals(name) << " is not connected."));

	if(notme && p == this)
		throw runtime_error("You cannot do that to yourself.");

	return p;
}

string tPlayer::getPlayerNameCentered( unsigned int size )
{
	string retVal;

	retVal = playername;

	if( playername.length() < size )
	{
		retVal.insert( 0, ( size - playername.length() ) / 2, ' ' );
		while( retVal.length() < size )
			retVal.append( " " );
	}
	else
	{
		retVal.erase( size, retVal.length() - size );
	}

	return retVal;
}

void tPlayer::ProcessException()
{
	/* signals can cause exceptions, don't get too excited. :) */
	cerr << "Exception on socket " << s << endl;
} /* end of tPlayer::ProcessException */

void tPlayer::Load()
{
	TiXmlDocument  xmlDoc( (PLAYER_DIR + playername + PLAYER_EXT).c_str() );
	TiXmlElement  *xmlePlr;
	TiXmlNode     *node = 0;
	bool loadOkay = xmlDoc.LoadFile();

	if( loadOkay )
	{
		TiXmlHandle xmlDocHandle( &xmlDoc );

		xmlePlr = xmlDocHandle.FirstChild( "player" ).Element();
		if( xmlePlr != NULL )
		{
			password = xmlePlr->Attribute( "password" );
			room = atoi( xmlePlr->Attribute( "room" ) );
			for( node = xmlePlr->FirstChild( "flags" )->FirstChild( "flag" );
				 node;
				 node = node->NextSibling( "flag" ) )
			{
				flags.insert( node->ToElement()->GetText() );
			}

			node = xmlePlr->FirstChild( "stats" );

			if( node )
			{
				maxhp = atoi( node->FirstChild( "maxHP" )->FirstChild()->Value() );
				curhp = atoi( node->FirstChild( "curHP" )->FirstChild()->Value() );
				baseskl = atoi( node->FirstChild( "baseSkill" )->FirstChild()->Value() );
			}
			else
				throw runtime_error( "Player data corrupt, please contact an admin for assistance" );
		}
	}
	else
		throw runtime_error( "That player does not exist, type 'new' to create a new one." );

} /* end of tPlayer::Load */

void tPlayer::Save()
{
	TiXmlDocument xmlDoc( (PLAYER_DIR + playername + PLAYER_EXT).c_str() );
	TiXmlHandle 	xmlDocHandle( &xmlDoc );
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
	TiXmlText		 	xmlNodeText( "" );
	TiXmlElement 	root( "player" );
	TiXmlElement	flags_( "flags" );
	TiXmlElement	flag( "" );
	TiXmlElement	stats( "stats" );
	TiXmlElement  stat( "" );
	char stmp[1024];

	std::set<string, ciLess>::iterator flagIter;
	string tmp;

	root.SetAttribute( "password", password );
	root.SetAttribute( "room", room );

	flagIter = flags.begin();
	for(; flagIter != flags.end(); ++flagIter )
	{
		flag.Clear();
		flag.SetValue( "flag" );
		tmp = *flagIter;
		xmlNodeText.SetValue( tmp.c_str() );
		flag.InsertEndChild( xmlNodeText );
		flags_.InsertEndChild( flag );
	}
	root.InsertEndChild( flags_ );

	stat.Clear();
	stat.SetValue( "maxHP" );
	sprintf( stmp, "%d", maxhp );
	xmlNodeText.SetValue( stmp );
	stat.InsertEndChild( xmlNodeText );
	stats.InsertEndChild( stat );

	stat.Clear();
	stat.SetValue( "curHP" );
	sprintf( stmp, "%d", curhp );
	xmlNodeText.SetValue( stmp );
	stat.InsertEndChild( xmlNodeText );
	stats.InsertEndChild( stat );

	stat.Clear();
	stat.SetValue( "baseSkill" );
	sprintf( stmp, "%d", baseskl );
	xmlNodeText.SetValue( stmp );
	stat.InsertEndChild( xmlNodeText );
	stats.InsertEndChild( stat );

	root.InsertEndChild( stats );
	xmlDoc.LinkEndChild( decl );
	xmlDoc.InsertEndChild( root );
	xmlDoc.SaveFile();
} /* end of tPlayer::Save */


void tPlayer::DoCommand(const string & command)
{
	istringstream is(command);
	ProcessCommand(this, is);
} /* end of tPlayer::Load */

// is flag set?
bool tPlayer::HaveFlag(const string & name)
{
	return flags.find(name) != flags.end();
} // end of NeedFlag

// flag must be set
void tPlayer::NeedFlag(const string & name)
{
	if(!HaveFlag(name))
		throw runtime_error("You are not permitted to do that.");
} // end of NeedFlag

// flag must not be set
void tPlayer::NeedNoFlag(const string & name)
{
	if(HaveFlag(name))
		throw runtime_error("You are not permitted to do that.");
} // end of NeedNoFlag

/* Here when there is outstanding data to be read for this player */

void tPlayer::ProcessRead()
{

	if(closing)
		return;	 // once closed, don't handle any pending input

	// I make it static to save allocating a buffer each time.
	// Hopefully this function won't be called recursively.
	static vector<char> buf(1000);	// reserve 1000 bytes for reading into

	int nRead = read(s, &buf [0], buf.size());

	if(nRead == -1)
	{
		if(errno != EWOULDBLOCK)
			perror("read from player");
		return;
	}

	if(nRead <= 0)
	{
		close(s);
		cerr << "Connection " << s << " closed" << endl;
		s = NO_SOCKET;
		DoCommand("/quit");	// tell others the s/he has left
		return;
	}

	inbuf += string(&buf [0], nRead);		/* add to input buffer */

	/* try to extract lines from the input buffer */
	for( ; ; )
	{
		string::size_type i = inbuf.find('\n');
		if(i == string::npos)
			break;	/* no more at present */

		string sLine = inbuf.substr(0, i);	/* extract first line */
		inbuf = inbuf.substr(i + 1, string::npos); /* get rest of string */

		ProcessPlayerInput(this, Trim(sLine));	/* now, do something with it */
	}
} /* end of tPlayer::ProcessRead */

/* Here when we can send stuff to the player. We are allowing for large
 volumes of output that might not be sent all at once, so whatever cannot
 go this time gets put into the list of outstanding strings for this player. */

void tPlayer::ProcessWrite()
{
//	*p << "[CLS]" << screenLayout;

	if( connstate == ePlaying )
		outbuf.append( screenLayout );

	outbuf = TextFormatting( outbuf, this );
	/* we will loop attempting to write all in buffer, until write blocks */
	while(s != NO_SOCKET && !outbuf.empty())
	{

		// send a maximum of 512 at a time
		int iLength = min<int>(outbuf.size(), 512);

		// send to player
		int nWrite = write(s, outbuf.c_str(), iLength );

		// check for bad write
		if(nWrite < 0)
		{
			if(errno != EWOULDBLOCK )
				perror("send to player");	/* some other error? */
			return;
		}

		// remove what we successfully sent from the buffer
		outbuf.erase(0, nWrite);

		// if partial write, exit
		if(nWrite < iLength)
			 break;

	} /* end of having write loop */
}	 /* end of tPlayer::ProcessWrite	*/

// functor for sending messages to all players
struct sendToPlayer
{
	const string message;
	const tPlayer * except;
	const int room;

	// ctor
	sendToPlayer(const string & m, const tPlayer * e = NULL, const int r = 0)
			: message(m), except(e), room(r) {}
	// send to this player
	void operator()(tPlayer * p)
	{
		if(p->IsPlaying() && p != except &&(room == 0 || p->room == room))
			*p << message;
	} // end of operator()
};	// end of sendToPlayer

// send message to all connected players
// possibly excepting one(eg. the player who said something)
// possibly only in one room(eg. for saying in a room)
void SendToAll(const string & message, const tPlayer * ExceptThis, const int InRoom)
{
	for_each(playerlist.begin(), playerlist.end(), sendToPlayer("\n" + message, ExceptThis, InRoom));
} /* end of SendToAll */

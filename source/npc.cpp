#include <stdexcept>

using namespace std;

#include "utils.h"
#include "npc.h"
#include "globals.h"

#include "tinyxml.h"

tNPC* NewNPC(const string name)
{
	tNPC* newnpc = new tNPC;
	
	
	
	return newnpc;
}

tNPC* LoadNPC(const string name)
{
	if( npcmmap[ name ] != NULL )
		return NewNPC( name );
	
	string npcfile = MAKE_STRING( NPCS_DIR << name << NPCS_EXT );
	TiXmlDocument doc( npcfile );
	bool loadOkay = doc.LoadFile();

	if( !loadOkay )
	{
		cerr << "Could not open npc file \"" << npcfile << "\": " << doc.ErrorDesc() << endl;
		return NULL;
	}
	
	return NULL;
}

tNPC* FindNPC(const string name)
{
	return NULL;
}
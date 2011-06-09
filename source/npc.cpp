#include <stdexcept>

using namespace std;

#include "utils.h"
#include "npc.h"
#include "globals.h"

#include "tinyxml.h"

tNPC* NewNPC(const string name)
{
	tNPC* newnpc = NULL;
	tNPCMaster* npcMaster;
	
	npcMaster = FindNPC( name );
	
	if( npcMaster )
	{
		newnpc = new tNPC;
		
		newnpc->master = npcMaster;
		newnpc->curhp = npcMaster->maxhp;
	}
	
	return newnpc;
}

tNPCMaster* LoadNPC(const string name)
{
	if( npcmmap[ name ] != NULL )
		return npcmmap[ name ];
	
	string npcfile = MAKE_STRING( NPCS_DIR << name << NPCS_EXT );
	TiXmlDocument doc( npcfile );
	bool loadOkay = doc.LoadFile();

	if( !loadOkay )
	{
		cerr << "Could not open npc file \"" << npcfile << "\": " << doc.ErrorDesc() << endl;
		return NULL;
	}
	
	TiXmlHandle docHandle( &doc );
	TiXmlElement* xmleNPC = docHandle.FirstChild( "npc" ).Element();
	if( xmleNPC == NULL )
	{
		cerr << "Invalid npc \"" << npcfile << "\": first element not <npc>!" << endl;
		return NULL;
	}

	string npcname = xmleNPC->Attribute( "name" );
	string npcdesc = xmleNPC->FirstChild( "description" )->ToElement()->GetText();
	
	TiXmlElement* xmleStats = xmleNPC->FirstChild( "stats" )->ToElement();
	if( xmleStats == NULL )
	{
		cerr << "NPC file \"" << npcfile << "\" is invalid: No stats! " << endl;
		return NULL;
	}
	
	tNPCMaster* newNPC = new tNPCMaster( npcname, npcdesc );
	
	short maxhp = atoi( xmleNPC->FirstChild( "maxHP" )->FirstChild()->Value() );
	short baseskl = atoi( xmleNPC->FirstChild( "baseSkill" )->FirstChild()->Value() );
	
	newNPC->maxhp = maxhp;
	newNPC->weaponSkill = baseskl;
	
	return newNPC;
}

tNPCMaster* FindNPC(const string name)
{
	return NULL;
}
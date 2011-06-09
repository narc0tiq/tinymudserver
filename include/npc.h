#ifndef TINYMUDSERVER_NPC_H
#define TINYMUDSERVER_NPC_H

//EzTarget|TODO : make this work better

#include <map>

// an NPC Master (the list of default NPC information)
class tNPCMaster
{
public:
	string npcname; // NPC name
	string description; // Some information from the MUDopedia :)
	short maxhp; // NPC's initial HP when spawned
	short weaponSkill; // Weapon skill of the NPC;
//	tWeapon weapon; // The weapon information that this NPC is using.

	tNPCMaster(const string& s) : description(s) {};
	tNPCMaster(const string& name, const string& desc): npcname(name), description(desc) {};

}; // end of class tNPC

class tNPC
{
public:
	short curhp;
	tNPCMaster *master;
};

// we will use a map of NPC's
typedef std::map <string, tNPCMaster*> tNPCMMap;
typedef tNPCMMap::const_iterator tNPCMMapIterator;

tNPC* NewNPC(const string name);
tNPCMaster* FindNPC(const string name);
//tNPC* LoadNPC(const int& vnum);

#endif // TINYMUDSERVER_ROOM_H

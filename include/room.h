#ifndef TINYMUDSERVER_ROOM_H
#define TINYMUDSERVER_ROOM_H

#include <map>

// map of exits for rooms
typedef std::map<string, int> tExitMap;
typedef std::map<int, int> tNPCSet;
typedef tExitMap::const_iterator tExitMapIterator;

// a room(vnum of room is in the room map)
class tRoom
{
	public:

	string roomname;    // room name
	string description; // what it looks like
	tExitMap exits;     // map of exits
	tNPCSet npcs;       // map of the NPC's in this room.

	tRoom(): roomname("Empty Room"), description("") {};
	tRoom(const string& s): roomname("Empty Room"), description(s) {};
	tRoom(const string& name, const string& desc): roomname(name), description(desc) {};

	void SaveRoom(const int& vnum);

}; // end of class tRoom

// we will use a map of rooms
typedef std::map <int, tRoom*> tRoomMap;
typedef tRoomMap::const_iterator tRoomMapIterator;

tRoom* FindRoom(const int& vnum);
//tRoom* LoadRoom(const int& vnum);

void DeleteRoom(const int& vnum);

bool RoomIsEmpty(const int& vnum);

#endif // TINYMUDSERVER_ROOM_H

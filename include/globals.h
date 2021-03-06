#include <set>
#include <string>
#include <map>

#include "player.h" // for player list
#include "room.h" // for rooms and exits
#include "command.h" // command lists
#include "log.h"

// bad player names
extern std::set<std::string, ciLess> badnameset;
// blocked IP addresses
extern std::set<std::string> blockedIP;
// list of all connected players
extern tPlayerList playerlist;
// map of all rooms
extern tRoomMap roommap;
// map of known commands(eg. look, quit, north etc.)
extern tCommandMap commandmap;
// map of things to do for various connection states
extern std::map<tConnectionStates, tHandler> statemap;
// messages
extern std::map<std::string, string, ciLess> messagemap;
// directions
extern std::set<std::string, ciLess> directionset;
// positive responses
extern std::set<string, ciLess> posResp;
// negative responses
extern std::set<string, ciLess> negResp;

// global variables
extern bool	 bStopNow;			// when set, the MUD shuts down
extern time_t tLastMessage;			// time we last sent a periodic message
extern int		iControl;	// socket for accepting new connections

extern string screenLayout;

string TextFormatting( string sMessageText, class tPlayer *p );

int dieRoll( int sides );

//Loging functions
extern TextLog SystemLog;
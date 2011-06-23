#include "log.h"

using namespace std;

std::string TimeStamp()
{
	char str[9];
	time_t a = time(0);
	struct tm* b = gmtime( &a );
	strftime( str, 9, "%H:%M:%S", b );
	return str;
}

std::string DateStamp()
{
	char str[11];
	time_t a = time(0);
	struct tm* b = gmtime( &a );
	strftime( str, 11, "%Y.%m.%d", b );
	return str;
}

template<class decorator>
Logger<decorator>::Logger( const std::string& p_filename, const std::string& p_logtitle, bool p_timestamp, bool p_datestamp )
{
	fstream filetester( p_filename.c_str(), std::ios::in );
	if( filetester.is_open() )
	{
		filetester.close();
		m_logfile.open( p_filename.c_str(), std::ios::out | std::ios::app );
	}
	else
	{
		m_logfile.open( p_filename.c_str(), std::ios::out );
		m_logfile << decorator::FileHeader( p_logtitle );
	}
	m_timestamp = true;
	m_datestamp = true;
	m_logfile << decorator::SessionOpen();
	Log( "Session opened." );
	m_timestamp = p_timestamp;
	m_datestamp = p_datestamp;
}

template< class decorator >
Logger< decorator >::~Logger()
{
	m_timestamp = true;
	m_datestamp = true;
	Log( "Session closed." );
	m_logfile << decorator::SessionClose();
}

template< class decorator >
void Logger< decorator >::Log( const std::string& p_entry )
{
	std::string message;
	if( m_datestamp )
	{
		message += "[" + DateStamp() + "] ";
	}
	if( m_timestamp )
	{
		message += "[" + TimeStamp() + "] ";
	}
	message += p_entry;
	m_logfile << decorator::Decorate( message );
}

TextLog SystemLog( "syslog.log", "System Log" );
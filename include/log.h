#ifndef TINYMUDSERVER_LOG_H
#define TINYMUDSERVER_LOG_H

#include <sys/time.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class TextDecorator
{
public:
	static std::string FileHeader( const std::string& p_title )
	{
		return "==================================================\n" +
	         p_title + "\n" +
					 "==================================================\n\n";
	}
	static std::string SessionOpen()
	{
		return "\n";
	}
	static std::string SessionClose()
	{
		return "\n";
	}
	static std::string Decorate( const std::string& p_string )
	{
		return p_string + "\n";
	}
};

template<class decorator>
class Logger
{
public:
	Logger( const std::string& p_filename, const std::string& p_logtitle, bool p_timestamp = false, bool p_datestamp = false );
	~Logger();
	void Log( const std::string& p_entry );
protected:
	fstream m_logfile;
	bool m_timestamp;
	bool m_datestamp;
};

typedef Logger<TextDecorator> TextLog;

#endif
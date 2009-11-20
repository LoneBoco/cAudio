#ifndef CLOGGER_H_INCLUDED
#define CLOGGER_H_INCLUDED

#include "../include/ILogger.h"
#include "../Headers/cMutex.h"
#include <map>
#include <stdarg.h>

namespace cAudio
{
	class cLogger : public ILogger
    {
    public:
		cLogger();
		virtual ~cLogger() { }

		virtual void logCritical( const char* sender, const char *msg, ... );
		virtual void logError( const char* sender, const char *msg, ... );
		virtual void logWarning( const char* sender, const char *msg, ... );
		virtual void logInfo( const char* sender, const char *msg, ... );
		virtual void logDebug( const char* sender, const char *msg, ... );

		virtual const LogLevel& getLogLevel() const { return MinLogLevel; }
		virtual void setLogLevel( const LogLevel& logLevel );

		//! Register Log Receiver
		//! Note: Any class registered will become owned by the internal thread.
		//! If threading is enabled, you MUST make the receiver threadsafe if you plan to access it in your application while it is registered
		virtual bool registerLogReceiver(ILogReceiver* receiver, std::string name);
		//!Unregister a Log Receiver
		//!Will NOT delete any user added receiver, you must do that yourself
		virtual void unRegisterLogReceiver(std::string name);
		//!Returns whether an log receiver is currently registered
		virtual bool isLogReceiverRegistered(std::string name);
		//!Returns a registered log receiver
		virtual ILogReceiver* getLogReceiver(std::string name);

	protected:
		void broadcastMessage( LogLevel level, const char* sender, const char* msg, va_list args );

		cAudioMutex Mutex;
		unsigned long StartTime;
		char TempTextBuf[2048];
		LogLevel MinLogLevel;
		std::map<std::string, ILogReceiver*> Receivers;
	private:
    };
};
#endif //! CLOGGER_H_INCLUDED

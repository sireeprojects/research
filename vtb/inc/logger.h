#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <thread>
#include <sstream>
#include <atomic>

namespace vtb {

enum class LogLevel {
   DEFAULT = 0,
   FULL    = 1
};

class Logger {
public:
   static Logger& getInstance();
   
   void init(const std::string& filename, LogLevel level);
   LogLevel getLevel() const;
   void log(LogLevel msg_level, const std::string& message);
   
   ~Logger();

private:
   Logger();
   Logger(const Logger&) = delete;
   Logger& operator=(const Logger&) = delete;
   
   void flushLoop();
   
   LogLevel m_level;
   std::ofstream m_file;
   std::stringstream m_buffer;
   std::mutex m_mutex;
   std::thread m_flush_thread;
   std::atomic<bool> m_running;
   std::atomic<bool> m_initialized;
};

}

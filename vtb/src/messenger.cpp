#include "messenger.h"

namespace vtb {

   LogStream::LogStream(LogLevel level, const std::string& prefix) 
       : m_level(level), m_prefix(prefix) {}
   
   LogStream::~LogStream() {
      // Only log if the global level allows it
      if (m_level <= Logger::getInstance().getLevel()) {
         Logger::getInstance().log(m_level, m_prefix + m_oss.str());
      }
   }
   
   LogStream info() {
      return LogStream(LogLevel::DEFAULT, "[INFO] ");
   }
   
   LogStream error() {
      return LogStream(LogLevel::DEFAULT, "[ERROR] ");
   }
   
   LogStream details() {
      return LogStream(LogLevel::FULL, "[DETAILS] ");
   }
}

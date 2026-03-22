#include "logger.h"
#include <iostream>
#include <chrono>

namespace vtb {

Logger& Logger::getInstance() {
   static Logger instance;
   return instance;
}

Logger::Logger() 
   : m_level(LogLevel::DEFAULT), m_running(false), m_initialized(false) {}

void Logger::init(const std::string& filename, LogLevel level) {
   if (m_initialized.exchange(true)) return;
   
   m_level = level;
   if (!filename.empty()) {
       m_file.open(filename, std::ios::out | std::ios::app);
   }
   
   m_running = true;
   m_flush_thread = std::thread(&Logger::flushLoop, this);
}

LogLevel Logger::getLevel() const {
   return m_level;
}

void Logger::log(LogLevel msg_level, const std::string& message) {
   if (msg_level <= m_level) {
       std::lock_guard<std::mutex> lock(m_mutex);
       m_buffer << message << "\n";
   }
}

void Logger::flushLoop() {
   while (m_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      
      std::string to_write;
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         to_write = m_buffer.str();
         m_buffer.str("");
         m_buffer.clear();
      }
      
      if (!to_write.empty()) {
         std::cout << to_write << std::flush;
         if (m_file.is_open()) {
            m_file << to_write << std::flush;
         }
      }
   }
   
   // Final drain
   std::string final_content;
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      final_content = m_buffer.str();
   }
   if (!final_content.empty()) {
      std::cout << final_content << std::flush;
      if (m_file.is_open()) {
         m_file << final_content << std::flush;
      }
   }
}

Logger::~Logger() {
   m_running = false;
   if (m_flush_thread.joinable()) {
      m_flush_thread.join();
   }
}

}

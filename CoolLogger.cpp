#include "LoggerModule.hpp"
#include <vector>

int main()
{
  // Initialize the logger
  LoggerModule::init();

  // Log messages with different log levels
  LOG("This is an info message");
  ERROR_LOG("This is an error message");
  CRITICAL_LOG("This is a critical message");
  WARN_LOG("This is a warning message");
  DEBUG_LOG("This is a debug message");

  std::vector<unsigned char> data = {0x48, 0x65, 0x6C, 0x6C};
  LOG("This is a message with a hexdump: {}", data);
  
  return 0;
}

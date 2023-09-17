// server_status.hpp
#ifndef SERVER_STATUS_HPP
#define SERVER_STATUS_HPP
#include <cstdint>

enum class Status : uint16_t
{
    SUCCESS_SAVE = 210,
    SUCCESS_FILE_LIST = 211,
    ERROR_FILE_NOT_FOUND = 1001,
    ERROR_NO_FILES = 1002, // Error status when client has no files on server
    FAILURE = 1003,        // general error status
    PROCESSING = 1         // default status, still processing request - for inner use
};

// Op codes
static const uint8_t OP_SAVE_FILE = 100;
static const uint8_t OP_RESTORE_FILE = 200;
static const uint8_t OP_DELETE_FILE = 201;
static const uint8_t OP_GET_FILE_LIST = 202;


#endif // SERVER_STATUS_HPP
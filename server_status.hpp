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
#pragma once

#include "server_status.hpp"

#include <string>
#include <vector>
#include <algorithm>

// the handler class functions are called from the session class
// according to the op code
// the handlers functions return a vector of chars that contains the response
class FileHandler
{
private:
    std::string folder_path_; // the name of the folder that contains the files
    std::vector<std::string> file_list_;
    std::string file_contents_;

public:
    FileHandler(const std::string &folder_path);

    std::vector<char> get_file_list(uint32_t user_id);

    std::vector<char> save_file(uint32_t user_id,
                                const std::string &filename,
                                const std::string &content);

    std::vector<char> restore_file(uint32_t user_id,
                            const std::string &filename);

    std::vector<char> delete_file(uint32_t user_id,
                            const std::string &filename);
                            
private:
    // this private function packs the response according to the protocol.
    // it is called from the handler functions above,
    // it uses the Status and the class var member to pack the reponse
    // response returns to the session classfrom the handler functions
    std::vector<char> pack_response(Status status, uint8_t op_,
        const std::string &filename);
};

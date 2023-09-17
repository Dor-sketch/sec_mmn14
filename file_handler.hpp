#pragma once

#include "server_status.hpp"

#include <string>
#include <vector>
#include <algorithm>

class FileHandler
{
private:
    std::string folder_path_;
    std::vector<std::string> file_list_;
    std::string filename_;
    std::string file_contents_;
    uint32_t user_id_;
    Status status_;
    uint8_t op_;
    uint32_t file_size_;

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
    std::vector<char> pack_response(Status status, uint8_t op_);
};

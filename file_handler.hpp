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

public:
    FileHandler(const std::string &folder_path);
    Status save_file(const std::string &filename, const std::string &content);
    std::string restore_file(const std::string &filename);
    Status delete_file(const std::string &filename);
    std::vector<std::string> get_file_list();

private:
    void parse_payload();
    void pack_response(Status status, std::vector<char> &responseBuffer);
};

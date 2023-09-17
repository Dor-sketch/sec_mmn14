#include "file_handler.hpp"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/sort_subrange.hpp>
#include <dirent.h> // For opendir, readdir, etc.
#include <algorithm> // For std::sort
#include <string>
#include <vector>

FileHandler::FileHandler(const std::string &folder_path) 
: folder_path_(folder_path),
  file_list_{} // Initialize file_list_ with size 0
{
    //check if the folder "backupsvr" exists and if not, create it
    if (!boost::filesystem::exists(folder_path_))
    {
        boost::filesystem::create_directory(folder_path_);
    }
}

std::vector<char> FileHandler::save_file(uint32_t user_id, const std::string &filename, const std::string &content)
{
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;
    std::string file_name_str = filename;

    std::cout << file_name_str << std::endl;
    std::string full_path = dir_path + "/" + file_name_str;

    std::cout << "Saving file to: " << full_path << '\n';
    
    // Check and create directory if it doesn't exist
    if (!boost::filesystem::exists(dir_path))
    {
        if (!boost::filesystem::create_directories(dir_path))
        {
            std::cerr << "Failed to create directory: " << dir_path << '\n';
            return pack_response(Status::FAILURE, OP_SAVE_FILE);
        }
    }

    std::ofstream ofs(full_path, std::ios::binary); // Consider if you really need binary mode here

    if (!ofs)
    {
        std::cerr << "Failed to open file: " << full_path << '\n';
        return pack_response(Status::FAILURE, OP_SAVE_FILE);
    }

    ofs << content; // use stream insertion for strings, more idiomatic

    if (!ofs.good())
    {
        ofs.close();
        std::cerr << "Failed to write to file: " << full_path << '\n';
        return pack_response(Status::FAILURE,OP_SAVE_FILE);
    }

    ofs.close();
    return pack_response(Status::SUCCESS_SAVE, OP_SAVE_FILE);
}










std::vector<char> FileHandler::restore_file(const std::string &filename)
{
    std::string full_path = folder_path_ + "/" + filename;
    std::ifstream ifs(full_path, std::ios::binary);

    if (!ifs)
    {
        // Error opening file for reading
        return pack_response(Status::FAILURE,OP_RESTORE_FILE); // or throw an exception or return an error status
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    file_contents_ = content;

    return pack_response(Status::SUCCESS_SAVE, OP_RESTORE_FILE);
}







std::vector<char> FileHandler::delete_file(const std::string &filename)
{
    std::string full_path = folder_path_ + "/" + filename;

    if (std::remove(full_path.c_str()) != 0)
    {
        pack_response(Status::FAILURE, OP_DELETE_FILE);
    }
    // no success status is defined in the protocol, so we use SUCCESS_SAVE
    pack_response(Status::SUCCESS_SAVE, OP_DELETE_FILE); 
}










std::vector<char> FileHandler::get_file_list(uint32_t user_id)
{
    printf("inside get_file_list\n\n\n");
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;

    std::vector<std::string> file_list;
    if (auto dir = opendir(dir_path.c_str()))
    {
        while (auto f = readdir(dir))
        {
            if (!f->d_name || f->d_name[0] == '.')
            {
                continue; // Skip everything that starts with a dot
            }
            file_list.push_back(std::string(f->d_name));
        }
        closedir(dir);
    }
    std::sort(file_list.begin(), file_list.end());

    file_list_ = file_list;

    return pack_response (Status::SUCCESS_FILE_LIST, OP_GET_FILE_LIST);
}

std::vector<char> FileHandler::pack_response(Status status, uint8_t op_)
{
    std::vector<char> responseBuffer;
    std::cout << "inside pack_response" << std::endl;
    uint8_t version = 1;
    uint16_t status_code = static_cast<uint16_t>(status);
    uint16_t name_len = 0;
    uint32_t payload_size = 0;
    std::string payload;

    switch (op_)
    {
    case OP_SAVE_FILE:
        if (status == Status::SUCCESS_SAVE)
        {
            // Set response fields
            name_len = filename_.length();
            payload_size = file_size_;

            // Pack response
            payload.append(reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
            payload.append(file_contents_);
        }
        break;

    case OP_RESTORE_FILE:
        if (status == Status::SUCCESS_SAVE)
        {
            // Set response fields
            name_len = filename_.length();
            payload_size = file_size_;

            // Pack response
            payload.append(reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
            payload.append(file_contents_);
        }
        break;

    case OP_DELETE_FILE:
        if (status == Status::SUCCESS_FILE_LIST)
        {
            // Set response fields
            // payload = algorithm::join(file_list_, "\n");
            payload_size = payload.length();

            // Pack response
            payload.insert(0, reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
        }
        break;

    case OP_GET_FILE_LIST:
        //... Handle get file list response logic
        break;

    default:
        payload = "Error: Invalid operation.";
        payload_size = payload.size();
        break;
    }
    // Packing the header. (This is just a simple example, adapt as needed.)
    responseBuffer.push_back(version);
    responseBuffer.push_back(static_cast<char>(status_code));
    responseBuffer.push_back(name_len & 0xFF);            // lower byte
    responseBuffer.push_back((name_len >> 8) & 0xFF);     // upper byte
    responseBuffer.push_back(payload_size & 0xFF);        // lower byte
    responseBuffer.push_back((payload_size >> 8) & 0xFF); // upper byte
    // ... pack other header fields as requi
    // Packing the payload
    responseBuffer.insert(responseBuffer.end(), payload.begin(), payload.end());
    std::cout << "responseBuffer: " << responseBuffer.data() << std::endl;
    for (const auto &byte : responseBuffer)
    {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;

    return responseBuffer;

}


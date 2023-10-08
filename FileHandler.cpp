// Description: This file contains the implementation of the FileHandler class.
//              it is used to handle the file operations and pack the response

#include "FileHandler.hpp"
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

std::vector<char> FileHandler::save_file(uint32_t user_id,
    const std::string &filename, const std::string &content)
{
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;
    std::string file_name_str = filename;
    std::string full_path = dir_path + "/" + file_name_str;

    // Check and create directory if it doesn't exist
    if (!boost::filesystem::exists(dir_path))
    {
        if (!boost::filesystem::create_directories(dir_path))
        {
            std::cerr << "Failed to create directory: " << dir_path << '\n';
            return pack_response(Status::FAILURE, OP_SAVE_FILE, file_name_str);
        }
    }

    std::ofstream ofs(full_path, std::ios::binary); 

    if (!ofs)
    {
        std::cerr << "Failed to open file: " << full_path << '\n';
        return pack_response(Status::FAILURE, OP_SAVE_FILE, file_name_str);
    }

    ofs << content; // use stream insertion for strings, more idiomatic

    if (!ofs.good())
    {
        ofs.close();
        std::cerr << "Failed to write to file: " << full_path << '\n';
        return pack_response(Status::FAILURE, OP_SAVE_FILE, file_name_str);
    }

    ofs.close();
    return pack_response(Status::SUCCESS_SAVE, OP_SAVE_FILE, file_name_str);
}


std::vector<char> FileHandler::restore_file(uint32_t user_id, const std::string &filename)
{
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;
    std::string file_name_str = filename;
    std::string full_path = dir_path + "/" + file_name_str;
    std::ifstream ifs(full_path, std::ios::binary);

    if (!ifs)
    {
        // Error opening file for reading
        return pack_response(Status::ERROR_FILE_NOT_FOUND,
                             OP_RESTORE_FILE, file_name_str); 
                             // or throw an exception or return an error status
    }

    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    file_contents_ = content;
    return pack_response(Status::SUCCESS_RESTORE, OP_RESTORE_FILE, file_name_str);
}


std::vector<char> FileHandler::delete_file(uint32_t user_id, const std::string &filename)
{
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;
    std::string file_name_str = filename;
    std::string full_path = dir_path + "/" + file_name_str;

    if (remove(full_path.c_str()) != 0)
    {
        std::cerr << "Error deleting file: " << full_path << '\n';
        return pack_response(Status::FAILURE, OP_DELETE_FILE, file_name_str);
    }
    else
    {
        // std::cout << "File deleted successfully" << '\n';
    }

    // no success status is defined in the protocol, so we use SUCCESS_DELETE
    return pack_response(Status::SUCCESS_DELETE, OP_DELETE_FILE, file_name_str);
}


std::vector<char> FileHandler::get_file_list(uint32_t user_id)
{
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
    std::string filename;
    if (file_list_.empty())
    {
        return pack_response(Status::ERROR_NO_FILES,
            OP_GET_FILE_LIST, filename);
    }
    else
    {
        filename = file_list_.front();
    }

    return pack_response(Status::SUCCESS_FILE_LIST,
        OP_GET_FILE_LIST, filename);
}


std::vector<char> FileHandler::pack_response(Status status, uint8_t op_,
    const std::string &filename)
{
    std::vector<char> responseBuffer;
    // Set response fields
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
            name_len = filename.length();
            // no payload for save file
            payload_size = 0;
        }
        break;

    case OP_RESTORE_FILE:
        // no specal staus for restore file
        if (status == Status::SUCCESS_RESTORE)
        {
            // Set response fields
            name_len = filename.length();
            payload_size = file_contents_.length();

            // Pack response
            payload.append(reinterpret_cast<const char *>(&payload_size),
                sizeof(payload_size));
            payload.append(file_contents_);
        }
        break;

    case OP_DELETE_FILE:
        if (status == Status::SUCCESS_DELETE)
        {
            name_len = 0;
            payload_size = 0;
        }
        break;

    case OP_GET_FILE_LIST:
        if (status == Status::ERROR_NO_FILES)
        {
            // Set response fields
            name_len = 0; // filename_ is empty
            payload_size = 0;
            status_code = static_cast<uint16_t>(Status::ERROR_NO_FILES);
        }
        else if (status == Status::SUCCESS_FILE_LIST)
        {
            // Compute the total length of the payload
            payload_size = 0;
            for (const auto &file : file_list_)
            {
                payload_size += file.size() + 1; // +1 for the newline character
            }
            // Append each file name and newline to the payload
            for (const auto &file : file_list_)
            {
                payload.append(file);
                payload.append("\n");
            }
        }
        else
        {
            payload = "Error: Invalid operation.";
            payload_size = payload.size();
            break;
        }
    }

    // Packing the response header
    responseBuffer.push_back(version); // byte 0

    // byte 1-2 status code in little-endian format
    responseBuffer.push_back(status_code & 0xFF);        // lower byte of status code
    responseBuffer.push_back((status_code >> 8) & 0xFF); // upper byte of status code
    responseBuffer.push_back(name_len & 0xFF);   // lower byte of name length
    responseBuffer.push_back((name_len >> 8) & 0xFF);   // upper byte of name length
    responseBuffer.insert(responseBuffer.end(), filename.data(),
                          filename.data() + name_len); // name (variable length)

    // Packing the payload size
    responseBuffer.push_back(payload_size & 0xFF);        // byte 0
    responseBuffer.push_back((payload_size >> 8) & 0xFF); // byte 1
    responseBuffer.push_back((payload_size >> 16) & 0xFF); // byte 2
    responseBuffer.push_back((payload_size >> 24) & 0xFF); // byte 3
    
    // Packing the payload data
    responseBuffer.insert(responseBuffer.end(), payload.begin(), payload.end());

    return responseBuffer;
}

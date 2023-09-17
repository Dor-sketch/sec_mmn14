#include "file_handler.hpp"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/sort_subrange.hpp>
#include <dirent.h> // For opendir, readdir, etc.
#include <algorithm> // For std::sort
#include <string>

FileHandler::FileHandler(const std::string &folder_path) 
: folder_path_(folder_path),
  file_list_{} // Initialize file_list_ with size 0
{
    //check if the folder "backupsvr" exists and if not, create it
    if (!boost::filesystem::exists(folder_path_))
    {
        boost::filesystem::create_directory(folder_path_);
    }
    file_list_ = get_file_list();
}

Status FileHandler::save_file(uint32_t user_id, const std::string &filename, const std::string &content)
{
    std::string user_id_str = std::to_string(user_id);
    std::string dir_path = folder_path_ + "/" + user_id_str;
    std::string full_path = dir_path + "/" + filename;

    // Check and create directory if it doesn't exist
    if (!boost::filesystem::exists(dir_path))
    {
        if (!boost::filesystem::create_directories(dir_path))
        {
            std::cerr << "Failed to create directory: " << dir_path << '\n';
            return Status::FAILURE;
        }
    }

    std::ofstream ofs(full_path, std::ios::binary); // Consider if you really need binary mode here

    if (!ofs)
    {
        std::cerr << "Failed to open file: " << full_path << '\n';
        return Status::FAILURE;
    }

    ofs << content; // use stream insertion for strings, more idiomatic

    if (!ofs.good())
    {
        ofs.close();
        std::cerr << "Failed to write to file: " << full_path << '\n';
        return Status::FAILURE;
    }

    ofs.close();
    return Status::SUCCESS_SAVE;
}

std::string FileHandler::restore_file(const std::string &filename)
{
    std::string full_path = folder_path_ + "/" + filename;
    std::ifstream ifs(full_path, std::ios::binary);

    if (!ifs)
    {
        // Error opening file for reading
        return ""; // or throw an exception or return an error status
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    return content;
}




Status FileHandler::delete_file(const std::string &filename)
{
    std::string full_path = folder_path_ + "/" + filename;

    if (std::remove(full_path.c_str()) != 0)
    {
        return Status::FAILURE;
    }
    // no success status is defined in the protocol, so we use SUCCESS_SAVE
    return Status::SUCCESS_SAVE; 
}





std::vector<std::string> FileHandler::get_file_list()
{
    std::vector<std::string> file_list;
    if (auto dir = opendir(folder_path_.c_str()))
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
    
    return file_list;
}

void FileHandler::parse_payload()
{
    // ... Parse payload logic ...
}

void FileHandler::pack_response(Status status, std::vector<char> &responseBuffer)
{
    // ... Pack response logic ...
}

// ... other method implementations ...



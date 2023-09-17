#include "file_handler.hpp"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/sort_subrange.hpp>
#include <dirent.h> // For opendir, readdir, etc.

namespace fs = boost::filesystem;

FileHandler::FileHandler(const std::string &folder_path) 
: folder_path_(folder_path),
  file_list_{} // Initialize file_list_ with size 0
{
    //check if the folder "backupsvr" exists and if not, create it
    if (!fs::exists(folder_path_))
    {
        fs::create_directory(folder_path_);
    }
    file_list_ = get_file_list();
}


Status FileHandler::save_file(const std::string &filename, const std::string &content)
{
    std::string full_path = folder_path_ + "/" + filename;
    std::ofstream ofs(full_path, std::ios::binary);

    if (!ofs)
    {
        // Error opening file for writing
        return Status::FAILURE;
    }

    ofs.write(content.c_str(), content.size());

    if (!ofs.good())
    {
        // Error during writing
        return Status::FAILURE;
    }

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



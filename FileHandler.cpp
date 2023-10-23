#include "FileHandler.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "LoggerModule.hpp"

FileHandler::FileHandler(const std::string &folder_path) : folder_path_(folder_path)
{
  if (!boost::filesystem::exists(folder_path_))
  {
    boost::filesystem::create_directory(folder_path_);
  }
}


std::vector<char> FileHandler::save_file(uint32_t user_id, const std::string &filename, const std::string &content)
{
  DEBUG_LOG("Starting to save file: {}", filename);

  if (content.empty())
  {
    WARN_LOG("Provided content for file {} is empty", filename);
    return pack_response(Status::FAILURE, OP_SAVE_FILE, filename);
  }

  std::string dir_path = constructPath(user_id);
  createDirectoryIfNotExists(dir_path);

  std::string full_path = dir_path + "/" + filename;
  std::ofstream ofs(full_path, std::ios::binary);

  if (!ofs)
  {
    ERROR_LOG("Failed to open file {} for writing", full_path);
    return pack_response(Status::FAILURE, OP_SAVE_FILE, filename);
  }

  ofs << content;
  ofs.flush(); // Ensure the content is written to the file

  if (ofs.fail())
  {
    ERROR_LOG("Failed to write to file {}", full_path);
    return pack_response(Status::FAILURE, OP_SAVE_FILE, filename);
  }

  DEBUG_LOG("Successfully saved file {}", filename);
  return pack_response(Status::SUCCESS_SAVE, OP_SAVE_FILE, filename);
}

#include <iostream>     // for std::cerr
#include <system_error> // for std::error_code

std::vector<char> FileHandler::restore_file(uint32_t user_id, const std::string &filename)
{
  std::string full_path = constructPath(user_id) + "/" + filename;

  // Debugging output
  DEBUG_LOG("Trying to restore file at path: {}", full_path);

  if (!boost::filesystem::exists(full_path))
  {
    WARN_LOG("File \"{}\" does not exist. Was it deleted recenetly?", full_path);
    return pack_response(Status::ERROR_FILE_NOT_FOUND, OP_RESTORE_FILE, filename);
  }

  std::ifstream ifs(full_path, std::ios::binary);
  if (!ifs.is_open())
  {
    ERROR_LOG("Failed to open file for reading: {}", full_path);
    return pack_response(Status::ERROR_FILE_NOT_FOUND, OP_RESTORE_FILE, filename);
  }

  try
  {
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    file_contents_ = content;
    DEBUG_LOG("Successfully read content of size: {} bytes", file_contents_.size());
  }
  catch (const std::exception &e)
  {
    ERROR_LOG("Exception caught while reading file: {}", e.what());
    return pack_response(Status::FAILURE, OP_RESTORE_FILE, filename);
  }

  return pack_response(Status::SUCCESS_RESTORE, OP_RESTORE_FILE, filename);
}

std::vector<char> FileHandler::delete_file(uint32_t user_id, const std::string &filename)
{
  std::string full_path = constructPath(user_id) + "/" + filename;

  if (remove(full_path.c_str()) != 0)
  {
    return pack_response(Status::FAILURE, OP_DELETE_FILE, filename);
  }

  return pack_response(Status::SUCCESS_DELETE, OP_DELETE_FILE, filename);
}

std::vector<char> FileHandler::get_file_list(uint32_t user_id)
{
  std::string dir_path = constructPath(user_id);
  auto file_list = getFilesInDirectory(dir_path);

  if (file_list.empty())
  {
    return pack_response(Status::ERROR_NO_FILES, OP_GET_FILE_LIST, "");
  }

  file_list_ = file_list;
  return pack_response(Status::SUCCESS_FILE_LIST, OP_GET_FILE_LIST, file_list_.front());
}

std::vector<char> FileHandler::pack_response(Status status, uint8_t op_,
                                             const std::string &filename)
{
  std::vector<char> responseBuffer;
  uint8_t version = 1;
  uint16_t status_code = static_cast<uint16_t>(status);
  uint16_t name_len = static_cast<uint16_t>(filename.length());
  uint32_t payload_size = 0;
  std::string payload;

  auto pushToBuffer = [&responseBuffer](auto val)
  {
    for (size_t i = 0; i < sizeof(val); ++i)
    {
      responseBuffer.push_back(val & 0xFF);
      val >>= 8;
    }
  };

  switch (op_)
  {
  case OP_SAVE_FILE:
    if (status != Status::SUCCESS_SAVE)
      name_len = 0;
    break;

  case OP_RESTORE_FILE:
    if (status == Status::SUCCESS_RESTORE)
    {
      payload_size = file_contents_.length();
      payload = file_contents_;
    }
    else
    {
      name_len = 0;
    }
    break;

  case OP_DELETE_FILE:
    if (status != Status::SUCCESS_DELETE)
      name_len = 0;
    break;

  case OP_GET_FILE_LIST:
    if (status == Status::SUCCESS_FILE_LIST)
    {
      for (const auto &file : file_list_)
      {
        payload_size += file.size() + 1;
        payload.append(file).append("\n");
      }
    }
    else if (status != Status::ERROR_NO_FILES)
    {
      payload = "Error: Invalid operation.";
      payload_size = payload.size();
    }
    break;
  }

  responseBuffer.push_back(version);
  pushToBuffer(status_code);
  pushToBuffer(name_len);
  responseBuffer.insert(responseBuffer.end(), filename.data(), filename.data() + name_len);
  pushToBuffer(payload_size);
  responseBuffer.insert(responseBuffer.end(), payload.begin(), payload.end());

  DEBUG_LOG("Packed response of size {} bytes for operation {} with status code {}",
                responseBuffer.size(), op_, status_code);

  return responseBuffer;
}

std::string FileHandler::constructPath(uint32_t user_id) const
{
  return folder_path_ + "/" + std::to_string(user_id);
}

void FileHandler::createDirectoryIfNotExists(const std::string &dir_path) const
{
  if (!boost::filesystem::exists(dir_path))
  {
    boost::filesystem::create_directories(dir_path);
  }
}

std::vector<std::string> FileHandler::getFilesInDirectory(const std::string &dir_path) const
{
  std::vector<std::string> file_list;

  if (auto dir = opendir(dir_path.c_str()))
  {
    while (auto f = readdir(dir))
    {
      if (!f->d_name || f->d_name[0] == '.')
      {
        continue;
      }
      file_list.push_back(std::string(f->d_name));
    }
    closedir(dir);
  }
  std::sort(file_list.begin(), file_list.end());

  return file_list;
}
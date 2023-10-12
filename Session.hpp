#pragma once

#include "FileHandler.hpp"
#include "Message.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <string>

class Session : public std::enable_shared_from_this<Session> {
private:
  boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket_;
  FileHandler file_handler_;
  std::vector<char> requestBuffer_;
  Message message_;
  char header_buffer_[8];

  // A function to parse header fields from client requests
  void readHeader();

  // used by all except OP_GET_FILE_LIST
  void readFilename();

  // used only for the OP_SAVE_FILE
  void readFileSize();

  // called by do_read_fileSize
  void readPayload();

  void send_response(const std::string &response);

  // called by send_response
  void graceful_close();

public:
  Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket,
          const std::string &folder_path);

  void start();
};

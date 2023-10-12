#pragma once

#include "Status.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <string>
#include <vector>

class Message : public std::enable_shared_from_this<Message> {
public:
  static constexpr uint8_t HEADER_LENGTH = 8; // Without filename.
  static constexpr uint8_t FILE_SIZE_BUFFER_LENGTH = 4;

private:
  char headerBuffer_[HEADER_LENGTH];
  char fileSizeBuffer_[FILE_SIZE_BUFFER_LENGTH];
  uint8_t version_;
  uint8_t operationCode_;
  uint16_t nameLength_;
  uint32_t userId_;
  uint32_t fileSize_;
  std::string fileContents_;
  std::vector<char> buffer_; // To store large messages and interact with ASIO.
  std::string filename_;

public:
  Message();

  // Getters.
  uint8_t getOperationCode() const;
  uint16_t getNameLength() const;
  uint32_t getFileSize() const;
  uint32_t getUserId() const;
  const std::string &getFileContent() const;
  std::string getHeaderBuffer() const;
  char *getFileSizeBuffer();
  const std::string &getFilename() const;
  std::vector<char> &getBuffer();
  char *getFileSizeBufferDirect();

  // Setters.
  void setFileContent();
  void setFilename();
  void setHeaderBuffer(const char *buffer);
  void setFileSize();

  // Parses the fixed-size header after the request header is read. Determines
  // the control flow based on the operation code.
  bool parseFixedHeader();
};

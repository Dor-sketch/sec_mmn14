#include "Message.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

constexpr uint8_t Message::FILE_SIZE_BUFFER_LENGTH;

Message::Message()
    : headerBuffer_{}, fileSizeBuffer_{0}, version_{0}, operationCode_{0},
      nameLength_{0}, userId_{0}, fileSize_{0},
      fileContents_{""}, buffer_{}, filename_{""} {}

bool Message::parseFixedHeader() {
  userId_ = *reinterpret_cast<uint32_t *>(&headerBuffer_[0]);
  version_ = headerBuffer_[4];
  operationCode_ = headerBuffer_[5];
  nameLength_ = *reinterpret_cast<uint16_t *>(&headerBuffer_[6]);

  if (operationCode_ != OP_SAVE_FILE && operationCode_ != OP_RESTORE_FILE &&
      operationCode_ != OP_DELETE_FILE && operationCode_ != OP_GET_FILE_LIST) {
    std::cerr << "Error: Invalid OP code received." << std::endl;
    throw std::runtime_error("Invalid OP code received.");
  }

  return true;
}

uint8_t Message::getOperationCode() const { return operationCode_; }

uint16_t Message::getNameLength() const { return nameLength_; }

uint32_t Message::getFileSize() const { return fileSize_; }

uint32_t Message::getUserId() const { return userId_; }

const std::string &Message::getFileContent() const { return fileContents_; }

std::string Message::getHeaderBuffer() const {
  return std::string(headerBuffer_, HEADER_LENGTH);
}

char *Message::getFileSizeBuffer() { return fileSizeBuffer_; }

const std::string &Message::getFilename() const { return filename_; }

std::vector<char> &Message::getBuffer() { return buffer_; }

char *Message::getFileSizeBufferDirect() { return fileSizeBuffer_; }

void Message::setFileContent() {
  if (operationCode_ != OP_SAVE_FILE) {
    std::cerr << "Error: invalid op for file content" << std::endl;
    return;
  } else if (buffer_.empty()) {
    std::cerr << "Error: Cannot set filename. Buffer is empty." << std::endl;
  }

  std::copy(buffer_.begin() + nameLength_,
            buffer_.begin() + nameLength_ + fileSize_,
            std::back_inserter(fileContents_));
}

void Message::setHeaderBuffer(const char *buffer) {
  std::copy(buffer, buffer + HEADER_LENGTH, headerBuffer_);
  headerBuffer_[HEADER_LENGTH] = {0}; // avoid buffer overflow
}

void Message::setFilename() {
  if (operationCode_ == OP_GET_FILE_LIST) {
    std::cerr << "Error: Cannot set filename for OP_GET_FILE_LIST" << std::endl;
    return;
  } else if (buffer_.empty()) {
    std::cerr << "Error: Cannot set filename. Buffer is empty." << std::endl;
  }

  filename_ = std::string(buffer_.data(), nameLength_);
}

void Message::setFileSize() {
  fileSize_ = (*reinterpret_cast<uint32_t *>(&fileSizeBuffer_[0]));
}
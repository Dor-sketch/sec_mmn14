// // Inside the Session class
// void start()
// {
//     auto self(shared_from_this());
//     boost::asio::async_read(socket_, boost::asio::buffer(data_, 8), [this, self](boost::system::error_code ec, std::size_t length)
//                             {
//         if (!ec) {
//             // Extract fields
//             uint32_t userId = *reinterpret_cast<uint32_t*>(data_);       // Assuming little endian
//             uint8_t version = data_[4];
//             uint8_t op = data_[5];
//             uint16_t name_len = *reinterpret_cast<uint16_t*>(data_ + 6); // Assuming little endian
//             // TODO: Continue reading the filename, then payload size and payload
//         } });
// }

// private:
// tcp::socket socket_;
// char data_[4096]; // Adjust buffer size as needed

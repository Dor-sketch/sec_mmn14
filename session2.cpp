// class Session : public std::enable_shared_from_this<Session>
// {
// private:
//     enum class ReadState
//     {
//         ReadHeader,
//         ReadFilename,
//         ReadFileSize,
//         ReadPayload,
//         Completed
//     };

//     ReadState currentState = ReadState::ReadHeader; // Start with the header state
//     char header_buffer_[Message::HEADER_LENGTH];

//     void handle_read(boost::system::error_code ec, std::size_t length)
//     {
//         if (ec)
//         {
//             std::cerr << "Error reading from socket: " << ec.message() << std::endl;
//             return; // Stop further reading due to error
//         }

//         // Check the current state to determine what was just read and what to do next
//         switch (currentState)
//         {
//         case ReadState::ReadHeader:
//             message_.set_header_buffer(header_buffer_);
//             if (message_.parse_fixed_header())
//             {
//                 // Decide next steps based on the header's operation code
//                 if (message_.get_op_code() != Message::OP_GET_FILE_LIST)
//                 {
//                     currentState = ReadState::ReadFilename;
//                     do_read_filename();
//                 }
//                 else if (message_.get_op_code() == Message::OP_SAVE_FILE)
//                 {
//                     currentState = ReadState::ReadFileSize;
//                     do_read_fileSize();
//                 }
//                 else
//                 {
//                     handle_request();
//                     currentState = ReadState::Completed;
//                 }
//             }
//             else
//             {
//                 std::cerr << "Error parsing header" << std::endl;
//             }
//             break;

//         case ReadState::ReadFilename:
//             // Process filename
//             message_.set_filename();
//             currentState = ReadState::ReadFileSize;
//             do_read_fileSize();
//             break;

//         case ReadState::ReadFileSize:
//             // Process file size
//             message_.set_file_size();
//             currentState = ReadState::ReadPayload;
//             do_read_payload();
//             break;

//         case ReadState::ReadPayload:
//             // Process payload
//             message_.set_file_content();
//             currentState = ReadState::Completed;
//             handle_request();
//             break;

//         case ReadState::Completed:
//             // The reading is complete, you can process the data or reset for a new message
//             break;
//         }
//     }

// public:
//     void start()
//     {
//         // Starts the reading process with the header
//         do_read_header();
//     }

//     void do_read_header()
//     {
//         auto self(shared_from_this());
//         boost::asio::async_read(socket_,
//                                 boost::asio::buffer(header_buffer_, Message::HEADER_LENGTH),
//                                 [this, self](boost::system::error_code ec, std::size_t length)
//                                 {
//                                     handle_read(ec, length);
//                                 });
//     }

//     // ... Other methods ...
// };
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>

using boost::asio::ip::tcp;

const int max_length = 1024;

void clear(char message[], int length)
{
    for (int i = 0; i < length; i++)
        message[i] = '\0';
}



class Session : public std::enable_shared_from_this<Session> {
public:
    // server folder path added to constructor
    Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket, std::string folder_path)
        : socket_(std::move(socket)), folder_path_(std::move(folder_path)) {}

    void start() {
        do_read_header();
        if (get_status() != Status::FAILURE) {
            handle_request();
        }
        pack_response();
    }

private:
    boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket_;
    enum
    {
        header_length = 8
    }; // without filename
    char header_[header_length];

    // Op codes
    static const uint8_t OP_SAVE_FILE = 100;
    static const uint8_t OP_RESTORE_FILE = 200;
    static const uint8_t OP_DELETE_FILE = 201;
    static const uint8_t OP_GET_FILE_LIST = 202;

    // Header fields
    uint32_t user_id_; // 4 bytes
    uint8_t version_;
    uint8_t op_;           // operation code
    uint16_t name_len_;    // length of filename
    std::string filename_; // filename size is variable

    // Payload fields
    uint32_t file_size_;        // unit32_t is 4 bytes
    std::string file_contents_; // file contents size is variable

    // extra fields
    std::string backup_dir_;
    std::string folder_path_;
    std::vector<std::string> file_list_;


    enum class Status : uint16_t
    {
        SUCCESS_SAVE = 210,
        SUCCESS_FILE_LIST = 211,
        ERROR_FILE_NOT_FOUND = 1001,
        ERROR_NO_FILES = 1002, // Error status when client has no files on server
        FAILURE = 1003,         // general error status
        PROCESSING = 1 // default status, still processing request - for inner use
    };

    Status status_ = Status::PROCESSING; // default status is failure

    Status get_status()
    {
        return status_;
    }

    void do_read_header() 
    {
        auto self(shared_from_this());
        boost::asio::async_read( //reading header from client
            socket_,
            boost::asio::buffer(header_, header_length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec && parse_header()) { // if no error and header is  
                    std::cout << "header processed" << std::endl;
                    return 1;
                } else {
                    std::cerr << "Error reading header: " << ec.message() << std::endl;
                    status_ = Status::FAILURE;
                }
            }
        ); // end of async_read
    }

    void send_response(Status status)
    {
        uint16_t status_code = static_cast<uint16_t>(status);
        uint16_t name_len = filename_.length();
        uint32_t payload_size = 0;

        std::vector<boost::asio::const_buffer> response_buffers;
        response_buffers.push_back(boost::asio::buffer(&version_, sizeof(version_)));
        response_buffers.push_back(boost::asio::buffer(&status_code, sizeof(status_code)));
        response_buffers.push_back(boost::asio::buffer(&name_len, sizeof(name_len)));
        response_buffers.push_back(boost::asio::buffer(filename_.c_str(), name_len));

        // process payload only if op was save or get file list and status is success
        if (status == Status::SUCCESS_SAVE || status == Status::SUCCESS_FILE_LIST)
        {
            payload_size = file_contents_.length();
            response_buffers.push_back(boost::asio::buffer(&payload_size, sizeof(payload_size)));
            response_buffers.push_back(boost::asio::buffer(file_contents_.c_str(), payload_size));
        }

        auto self(shared_from_this());
        boost::asio::async_write(socket_, response_buffers,
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (ec)
                                     {
                                         std::cerr << "Error sending response: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void do_read_payload()
    {
        enum { payload_length = 4 }; // without file contents
        char payload_[payload_length];

        auto self(shared_from_this());
        boost::asio::async_read(socket_,
        boost::asio::buffer(payload_, payload_length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    handle_request();
                }
                else
                {
                    std::cerr << "Error reading payload: " << ec.message() << std::endl;
                }
            }
        );
    }

    bool parse_header()
    {
        // Extract fields from header
        user_id_ = *reinterpret_cast<uint32_t *>(header_); // Assuming little endian
        version_ = header_[4];
        op_ = header_[5];
        name_len_ = *reinterpret_cast<uint16_t *>(header_ + 6); // little endian

        // Copy filename from header and add null terminator
        filename_.assign(header_ + header_length - name_len_, name_len_);
        filename_.push_back('\0');

        // Set backup directory
        backup_dir_ = folder_path_ + "/" + std::to_string(user_id_);

        return true;
    }

    bool parse_payload()
    {
        enum{ payload_length = 4 }; // without file contents
        char payload_[payload_length];
        // Extract fields from payload
        file_size_ = *reinterpret_cast<uint32_t *>(payload_); // Assuming little endian

        // Copy file contents from payload
        file_contents_.assign(payload_ + payload_length - file_size_, file_size_);

        return true;
    }



    void pack_response()
    {
        uint8_t version = 1;
        uint16_t status = static_cast<uint16_t>(status_);
        uint16_t name_len = 0;
        uint32_t payload_size = 0;
        std::string payload;

        switch (op_)
        {
        case OP_SAVE_FILE:
            if (status_ == Status::SUCCESS_SAVE)
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
            if (status_ == Status::SUCCESS_SAVE)
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
            if (status_ == Status::SUCCESS_SAVE)
            {
                // Set response fields
            }
            break;

        case OP_GET_FILE_LIST:
            if (status_ == Status::SUCCESS_FILE_LIST)
            {
                // Set response fields
                payload = boost::algorithm::join(file_list_, "\n");
                payload_size = payload.length();

                // Pack response
                payload.insert(0, reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
            }
            break;

        default: // Invalid op code or general failure
            // Set response fields
            break;
        }

        // Pack header
        std::vector<boost::asio::const_buffer> header_buffers;
        header_buffers.push_back(boost::asio::buffer(&version, sizeof(version)));
        header_buffers.push_back(boost::asio::buffer(&status, sizeof(status)));
        header_buffers.push_back(boost::asio::buffer(&name_len, sizeof(name_len)));

        // Pack payload
        std::vector<boost::asio::const_buffer> payload_buffers;
        if (payload_size > 0)
        {
            payload_buffers.push_back(boost::asio::buffer(payload.c_str(), payload_size));
        }

        // Combine header and payload
        std::vector<boost::asio::const_buffer> response_buffers;
        response_buffers.insert(response_buffers.end(), header_buffers.begin(), header_buffers.end());
        response_buffers.insert(response_buffers.end(), payload_buffers.begin(), payload_buffers.end());

        // Send response to client
        boost::asio::async_write(socket_, response_buffers,
                                 [this](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (ec)
                                     {
                                         std::cerr << "Error sending response: " << ec.message() << std::endl;
                                     }
                                 });
    }
    


    void handle_request()
    {
        int status = 0;

        // Handle request based on op code
        switch (op_)
        {
        case OP_SAVE_FILE:
            save_file();
        case OP_RESTORE_FILE:
            restore_file();
            break;
        case OP_DELETE_FILE:
            delete_file();
            break;
        case OP_GET_FILE_LIST:
            get_file_list();
        default:
            std::cerr << "Error: invalid op code: " << static_cast<int>(op_) << std::endl;
            status_ = Status::FAILURE;
        }
    }

    void save_file()
    {
        
        parse_payload();
        // Create backup directory if it doesn't exist
        if (!boost::filesystem::exists(backup_dir_))
        {
            boost::filesystem::create_directory(backup_dir_);
        }

        // Save file to backup directory
        std::ofstream file(backup_dir_ + "/" + filename_, std::ios::binary);
        file.write(file_contents_.c_str(), file_size_);
        file.close();
    }

    void restore_file()
    {
        // TODO: Implement restoring a file
    }

    void delete_file()
    {
        // TODO: Implement deleting a file
    }



    void get_file_list()
    {
        // use private member variable backup_dir_ to get file list
        if (!boost::filesystem::exists(backup_dir_))
        {
            std::cerr << "Error: user id does not exist" << std::endl;
            return;
        }

        // Generate file list on the private member variable file_list_
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(backup_dir_); itr != end_itr; ++itr)
        {
            if (boost::filesystem::is_regular_file(itr->path()))
            {
                file_list_.push_back(itr->path().filename().string());
            }
        }

        // Convert file list to string
        std::string file_list_str = boost::algorithm::join(file_list_, "\n");

        // // Send file list to client
        // std::ostringstream response_stream;
        // response_stream << "File list for user " << user_id_ << ":\n"
        //                 << file_list_str << "\n";
        // std::string response = response_stream.str();
        // boost::asio::async_write(socket_, boost::asio::buffer(response.c_str(), response.length()),
        //                          [this](boost::system::error_code ec, std::size_t /*length*/)
        //                          {
        //                              if (ec)
        //                              {
        //                                  std::cerr << "Error sending response: " << ec.message() << std::endl;
        //                              }
        //                          });
    }

};

class Server
{
public:
    Server(boost::asio::io_context &io_context, unsigned short port, const std::string &folder_path)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          folder_path_(folder_path) // Store the folder path
    {
        // Create backup directory if it doesn't exist
        boost::filesystem::path backup_dir(folder_path); // Use the provided folder_path
        if (!boost::filesystem::exists(backup_dir))
        {
            boost::filesystem::create_directory(backup_dir);
        }

        accept(); // Start accepting clients
    }

private:
    void accept()
    {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket)
                               {
                                   if (!ec)
                                   {
                                       std::make_shared<Session>(std::move(socket), folder_path_)->start(); // Pass the folder_path_ to Session
                                   }
                                   accept(); // Continue accepting
                               });
    }

    tcp::acceptor acceptor_;
    std::string folder_path_; // Added member variable for folder path
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        Server server(io_context, std::atoi(argv[1]), "backupssvr"); // Pass the folder path "backupssvr" to Server

        io_context.run(); // Run the I/O context to start processing network events
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
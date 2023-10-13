# Backup Server 💾

## Description 📝

This program is designed to perform various file operations such as saving, restoring, and deleting files on a server. It is implemented using C++ and Python and utilizes the Boost.Asio library for asynchronous I/O operations. The program features dynamic parsing utilities to support very large files.

## Server Details 🖥️

### Files 📂

- `Message.hpp` and `Message.cpp`: Define the `Message` class, representing messages sent between client and server.
- `Session.hpp` and `Session.cpp`: Handle a single client connection.
- `Server.hpp` and `Server.cpp`: Create a TCP server and listen for connections.
- `FileHandler.hpp` and `FileHandler.cpp`: Handle file operations like save, restore, and delete.
- `main.cpp`: Contains the main function that starts the server.
- `Makefile`: Build instructions for the program.
- `.gitignore`: Specifies which files and directories should be ignored by Git.

### Usage 🛠️

1. **Compilation**: Compile the server program using the `Makefile`:

   ```bash
   make
   ```

   This will generate an executable named `server_app`.

2. **Configuration**: Provide a `server.info` file with the server IP address and port:

   ```bash
   <IP address>:<port number>
   ```

   For example:

   ```bash
   127.0.0.1:8080
   ```

3. **Execution**: Run the server using:

   ```bash
   ./server_app
   ```

## Client Details 📱

### Client Files 📂

- `client.py`: A Python client to test the server's functionality. The client has been refactored for improved object-oriented design, streamlined flow of operations, and enhanced error handling.

### Client Usage 🛠️

1. **Preparation**: Ensure both `server.info` and `backup.info` are in the same directory as the server and client. `server.info` contains the server IP and port, while `backup.info` lists filenames for the client to process.

2. **Running the Client**: Test the server's functionality with:

   ```bash
   python3 tester.py
   ```

### Notable Updates 🌟

- The client transitioned from procedural design to object-oriented programming for improved maintainability.
- Enhanced clarity with the segregation of the send_and_receive method into specific sub-methods.
- Included dynamic handling of server responses and error validations.

### Troubleshooting 🔧

- Ensure the server is running before executing the client.
- Files in `backup.info` should be in the client script's directory.
- `server.info` should have valid IP and port data.

## License 📜

This project is licensed under the MIT License.

## Acknowledgements 🙏

This project was completed as part of the Open University of Israel course Defensive System-Programming (20937), taken in 2023c. Earned 100 points.

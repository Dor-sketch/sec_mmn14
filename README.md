![back-up_server_cover](https://github.com/Dor-sketch/sec_mmn14/assets/138825033/61e66d6d-08f4-4dac-943e-3e2c5415e1da)


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

1. **Dependencies**: Before building and running this project, make sure you have the following dependencies installed on your system:

   - **C++ Compiler**: You'll need a C++ compiler. This project was developed using `g++`, but other C++ compilers should work as well.
   - **Boost Libraries**: This project uses the Boost C++ Libraries for networking and filesystem operations. You can download and install Boost from [the official website](https://www.boost.org/).
   - **FMT Library (Optional)**: If you need the FMT library, you can download it from [the FMT GitHub repository](https://github.com/fmtlib/fmt) or use your package manager if available.

2. **Compilation**: Compile the server program using the `Makefile`:

   ```bash
   make
   ```

   This will generate an executable named `server_app`.

3. **Configuration**: Provide a `server.info` file with the server IP address and port:

   ```bash
   <IP address>:<port number>
   ```

   For example:

   ```bash
   127.0.0.1:8080
   ```

4. **Execution**: Run the server using:

   ```bash
   ./server_app
   ```

## Client Details 📱

### Client Files 📂

- `client.py`: A Python client to test the server's functionality. The client has been refactored for improved object-oriented design, streamlined flow of operations, and enhanced error handling.

### Client Usage 🛠️

1. **Packages Preparation**: Before running this script, ensure you have the following package installed:

   - Python `termcolor` package (version 1.1.0)

   You can install the required package using `pip`:

   ```bash
   pip install termcolor==1.1.0
   ```

2. **Files Preparation**: Ensure both `server.info` and `backup.info` are in the same directory as the server and client. `server.info` contains the server IP and port, while `backup.info` lists filenames for the client to process.

3. **Running the Client**: Test the server's functionality with:

   ```bash
   python3 tester.py
   ```

## Cool Logger 🌟

The project includes a "Cool Logger" module for efficient and colorful logging. Here are the key features of the logger:

- **Log Location Information:** Each log entry includes the file and line number where the log message was generated. This provides valuable context for debugging.

    ```bash
    [CoolLogger.cpp:10] [info] This is an info message
    [CoolLogger.cpp:11] [error] This is an error message
    [CoolLogger.cpp:12] [critical] This is a critical message
    [CoolLogger.cpp:13] [warning] This is a warning message
    [CoolLogger.cpp:14] [debug] This is a debug message
    ```

- **Hexadecimal Dump:** The logger supports hexadecimal dumps for binary data, making it easier to inspect binary content in log messages.

    ```cpp
    LOG("This is a message with a hexdump:", data);
    ```

    Output:

    ```bash
    [CoolLogger.cpp:17] [info] This is a message with a hexdump: 48 65 6c 6c
    ```

- **Colored Log Levels:** Log levels (e.g., info, error, critical, warn, debug) are displayed in color for easy identification.

    Example: [INFO], [ERROR], [CRITICAL]

- **Dynamic Log Level Detection:** The logger automatically detects the log level based on the log method used, simplifying log message creation.

### Logger Usage 🛠️

Integrating the Cool Logger into your code is a breeze:

1. Include the `LoggerModule.hpp` header.
2. Initialize the logger using `LoggerModule::init()`.
3. Employ the provided macros like `LOG`, `ERROR_LOG`, `CRITICAL_LOG`, `WARN_LOG`, and `DEBUG_LOG`, and employ `{}` for string formatting.

The Cool Logger streamlines the process, handling log level detection, file and line number tracking, and even hexadecimal dumps. Logging is now effortless and stylish!

For a complete example, refer to `CoolLogger.cpp`:

```cpp
int main()
{
  // Initialize the logger
  LoggerModule::init();

  // Log messages with different log levels
  LOG("This is an info message");
  ERROR_LOG("This is an error message");
  CRITICAL_LOG("This is a critical message");
  WARN_LOG("This is a warning message");
  DEBUG_LOG("This is a debug message");

  // Log a message with a hexadecimal dump
  std::vector<unsigned char> data = {0x48, 0x65, 0x6C, 0x6C};
  LOG("This is a message with a hexdump:", data);

  return 0;
}
```

---

### Adjusting the Log Level 🛠️

The logger supports modulating the log level based on the compilation mode. This is done by defining a specific macro in the `Makefile`. To change the log level, simply compile the program in the desired mode:

- **Debug Mode**: When the program is compiled in debug mode, the logger will display all log levels.

    ```bash
    make debug
    ./logger_app
    ```
    Output:
    ```bash
    [CoolLogger.cpp:10] [info] This is an info message
    [CoolLogger.cpp:11] [error] This is an error message
    [CoolLogger.cpp:12] [critical] This is a critical message
    ```

- **Release Mode**: When the program is compiled in release mode, the logger will only display error, critical, and warning messages.

    ```bash
    make
    ./logger_app
    ```
    Output:
    ```bash
    [CoolLogger.cpp:11] [error] This is an error message
    [CoolLogger.cpp:12] [critical] This is a critical message
    [CoolLogger.cpp:13] [warning] This is a warning message
    ```

With these features, the "Cool Logger" module simplifies logging and debugging while adding a touch of style.

---

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

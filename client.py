import socket
import struct
import random
import os
from termcolor import colored
import logging


class RequestsColoredFormatter(logging.Formatter):
    COLORS = {
        "DEBUG": ("blue", []),
        "INFO": ("green", []),
        "WARNING": ("magenta", ["bold"]),
        "ERROR": ("red", ["bold"]),
        "CRITICAL": ("red", []),
    }

    def format(self, record):
        log_message = super().format(record)
        color, attrs = self.COLORS.get(record.levelname, ("white", []))
        return colored(log_message, color, attrs=attrs)


logger = logging.getLogger("logs")  # Changed to "logs" as per your request

# Set the logging level for logger
logger.setLevel(logging.DEBUG)

# Check if the logger object already has handlers attached
if not logger.handlers:
    # Set up the console handler with the custom formatter for logger
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    formatter = RequestsColoredFormatter(
        "[logs] %(asctime)s [%(levelname)s] %(message)s"
    )
    console_handler.setFormatter(formatter)

    # Ensure that logger doesn't propagate messages to root logger
    logger.propagate = False

    # Add the console handler to the logger object
    logger.addHandler(console_handler)


# Constants for operations and status codes
VERSION = 1
OP_SAVE = 100
OP_RESTORE = 200
OP_DELETE = 201
OP_LIST = 202

STATUS_FILE_RESTORED = 210
STATUS_LIST_OK = 211
STATUS_FILE_SAVED = 212
STATUS_FILE_DELETED = 213
STATUS_FILE_NOT_FOUND = 1001
STATUS_NO_FILES_FOUND = 1002
FAILURE = 1003


class Client:
    def __init__(self, user_id, server_address, server_port):
        self.id = user_id
        self.server_address = server_address
        self.server_port = server_port
        self.file_data = {}
        self.filenames = []
        self.version = VERSION
        self.load_files()

    def load_files(self):
        try:
            with open("backup.info", "r") as f:
                potential_files = f.read().strip().split("\n")
                for filename in potential_files:
                    if not os.path.isfile(filename):
                        logger.warning(f"File {filename} not found")
                    else:
                        with open(filename, "rb") as f:
                            self.file_data[filename] = f.read()
                            self.filenames.append(filename)
        except FileNotFoundError:
            logger.critical("backup.info file not found")
            return
        for filename, content in self.file_data.items():
            if len(content) == 0:
                logger.warning(f"File {filename} appears to be empty.")
            else:
                logger.debug(f"Loaded {filename} of size: {len(content)} bytes")


    def create_message(self, op, file_index):
        if file_index >= len(self.filenames):
            logger.error(f"File index {file_index} out of range!")
            return None

        filename = self.filenames[file_index].encode()
        file_contents = self.file_data[self.filenames[file_index]]

        FIXED_FORMAT = "<I B B H"
        message = struct.pack(FIXED_FORMAT, self.id, self.version, op, len(filename))
        message += filename
        message += struct.pack("<I", len(file_contents)) + file_contents
        logger.debug(f"Creating message for {filename}. File content size: {len(file_contents)} bytes.")
        return message

    def format_bytes(self, byte_data):
        """Format bytes to display them in groups of 16 bytes per row."""
        return '\n'.join([' '.join(['{:02x}'.format(b) for b in byte_data[i:i+8]]) for i in range(0, len(byte_data), 8)])


    def send_and_receive(self, message):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            try:
                # Set a timeout for socket operations, e.g., 10 seconds
                sock.settimeout(10)

                sock.connect((self.server_address, self.server_port))
                logger.debug(f"Connected to {self.server_address}:{self.server_port}")

                # Log the message details here
                logger.debug(f"Sending message of size {len(message)} bytes to the server.")

                # Using the format_bytes function
                formatted_message = self.format_bytes(message[:256])  # Displaying only the first 256 bytes as an example
                logger.debug(f"Message content (first 256 bytes):\n{formatted_message}") 

                sock.sendall(message)
                logger.debug(f"Message sent to server.")

                # Log before trying to receive a response
                logger.debug("Waiting for server response...")
                
                # Receive and check the response
                response = self.receive_response(sock)
                if response is None:
                    logger.error("No valid response received from the server.")
                    return

                version, status, filename, payload = response
                logger.debug("Received server response.")

            except socket.timeout as e:
                logger.error(f"Socket operation timed out: {e}")
                return

            except socket.error as e:
                logger.error(f"Socket error: {e}")
                return

            except Exception as e:
                logger.critical(f"Unexpected error: {e}")
                return

            # Handle the server response based on the status
            self.handle_server_response(status, filename, payload)

    def receive_response(self, sock):
        fixed_response_format = "<BHH"
        response_header = self.recvall(sock, struct.calcsize(fixed_response_format))
        version, status, name_len = struct.unpack(
            fixed_response_format, response_header
        )
        logger.debug(f"Receiving response header of size {struct.calcsize(fixed_response_format)} bytes.")
        filename = None
        payload = None
        if name_len:
            try:
                filename = self.recvall(sock, name_len).decode()
            except UnicodeDecodeError:
                logger.warning("Received filename is not valid UTF-8.")
                filename = "<INVALID_FILENAME>"
        if status in [STATUS_LIST_OK, STATUS_FILE_RESTORED]:
            raw_file_size_bytes = self.recvall(sock, 4)
            file_size = struct.unpack("<I", raw_file_size_bytes)[0]
            payload = self.recvall(sock, file_size)
        elif status not in [STATUS_NO_FILES_FOUND, FAILURE, STATUS_FILE_NOT_FOUND, STATUS_FILE_DELETED]:
            logger.warning(f"Unhandled status code received: {status}")
        return version, status, filename, payload


    def recvall(self, sock, count):
        buf = b""
        while count:
            newbuf = sock.recv(count)
            if not newbuf:
                raise ConnectionError(
                    "Socket connection was closed by the remote server."
                )
            buf += newbuf
            count -= len(newbuf)
        return buf

    def handle_server_response(self, status, filename, payload):
        # Handle server response messages using logger instead of print
        if status == STATUS_FILE_RESTORED:
            with open(f"temp.{filename.split('.')[1]}", "wb") as f:
                f.write(payload)
            logger.info(
                f"File {filename} was restored successfully and saved to temp.{filename.split('.')[1]}"
            )
        elif status == STATUS_FILE_NOT_FOUND:
            logger.error("File not found")
        elif status == STATUS_LIST_OK:
            logger.info(f"Client files list: {payload.decode()}")
        elif status == STATUS_NO_FILES_FOUND:
            logger.warning("No files found for this client")
        elif status == STATUS_FILE_SAVED:
            logger.info(f"File {filename} was saved successfully")
        elif status == STATUS_FILE_DELETED:
            logger.info("File deleted")
        elif status == FAILURE:
            logger.error("Server error")
        else:
            logger.warning(f"Unknown status code: {status}")

    # Decorator for logger
    def log_operation(op_name):
        def decorator(func):
            def wrapper(self, *args, **kwargs):
                filename = None
                if args:
                    try:
                        filename = self.filenames[args[0]]
                    except IndexError:
                        pass
                logger.info(
                    f"Sending message to {op_name} the file: {filename if filename else 'N/A'}"
                )
                return func(self, *args, **kwargs)

            return wrapper

        return decorator

    @log_operation("get list")
    def test_get_list(self, _unused=None):
        if self.filenames:
            message_get_list = self.create_message(OP_LIST, 0)
            self.send_and_receive(message_get_list)
        else:
            logger.error("No filenames found!")

    @log_operation("save")
    def test_save(self, file_index):
        self.validate_file_index(file_index)
        message = self.create_message(OP_SAVE, file_index)
        self.send_and_receive(message)

    @log_operation("restore")
    def test_restore(self, file_index):
        self.validate_file_index(file_index)
        message = self.create_message(OP_RESTORE, file_index)
        self.send_and_receive(message)

    @log_operation("delete")
    def test_delete(self, file_index):
        self.validate_file_index(file_index)
        message = self.create_message(OP_DELETE, file_index)
        self.send_and_receive(message)

    def validate_file_index(self, file_index):
        if not (0 <= file_index < len(self.filenames)):
            raise ValueError(f"Invalid file index {file_index}")


def main():
    logger.debug("1. Generating random ID")
    random_id = random.getrandbits(32)
    logger.debug(f"Random ID: {random_id}")

    logger.debug("2. Reading server configuration from server.info")
    with open("server.info", "r") as f:
        server_address, server_port = f.readline().strip().split(":")
    server_port = int(server_port)

    tester = Client(random_id, server_address, server_port)

    # Define the operations and the file indices for each operation
    operations = [
        (tester.test_get_list, 0),
        (tester.test_save, 0),  # Save first file
        (
            tester.test_save,
            1,
        ),  # Save second file (course_book.pdf, if it's the second one)
        (tester.test_get_list, 0),
        (tester.test_restore, 0),
        (tester.test_delete, 0),
        (tester.test_restore, 0),
    ]

    for idx, (operation, file_index) in enumerate(operations, start=4):
        logger.debug(f"{idx}. {'='*10}")
        operation(file_index)

    logger.debug("11. Quitting the client")


if __name__ == "__main__":
    main()

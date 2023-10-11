import socket
import struct
import random
import os

# Constants for operations and status codes
VERSION = 1
OP_SAVE = 100
OP_RESTORE = 200
OP_DELETE = 201
OP_LIST = 202

STATUS_FILE_RESTORED = 210
STATUS_LIST_OK = 211
STATUS_FILE_SAVED = 212 
STATUS_FILE_DELETED = 213 # same as above
STATUS_FILE_NOT_FOUND = 1001
STATUS_NO_FILES_FOUND = 1002
FAILURE = 1003 

class Tester:
    def __init__(self, user_id, server_address, server_port):
        self.id = user_id
        self.server_address = server_address
        self.server_port = server_port
        self.file_data = {}  # Dictionary to store filename-content pairs
        self.filenames = []  # List to store filenames
        self.version = VERSION
        self.load_files()

    def load_files(self):
        try:
            with open('backup.info', 'r') as f:
                self.filenames = f.read().strip().split('\n')
        except FileNotFoundError:
            print("backup.info file not found")
            return

        for filename in self.filenames:
            if not os.path.isfile(filename):
                print(f"File {filename} not found")
            else:
                with open(filename, 'rb') as f:
                    self.file_data[filename] = f.read()

    def create_message(self, op, file_index):
        filename = self.filenames[file_index].encode()
        file_contents = self.file_data[self.filenames[file_index]]

        FIXED_FORMAT = '<I B B H'  # user_id, version, op, name_len
        message = struct.pack(FIXED_FORMAT, self.id, self.version, op, len(filename))
        message += filename
        message += struct.pack('<I', len(file_contents)) + file_contents

        return message

    def send_and_receive(self, message):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            try:
                sock.connect((self.server_address, self.server_port))
                sock.sendall(message)
                version, status, filename, payload = self.receive_response(sock)
            except Exception as e:
                print(f"Unexpected error: {e}")
                return

            # Handle the server response based on the status
            self.handle_server_response(status, filename, payload)

    def receive_response(self, sock):
        fixed_response_format = '<BHH'
        response_header = self.recvall(sock, struct.calcsize(fixed_response_format))
        version, status, name_len = struct.unpack(fixed_response_format, response_header)

        if status in [STATUS_NO_FILES_FOUND, FAILURE, STATUS_FILE_NOT_FOUND, STATUS_FILE_DELETED]:
            return version, status, None, None

        filename = self.recvall(sock, name_len).decode()

        if status in [STATUS_LIST_OK, STATUS_FILE_RESTORED]:
            raw_file_size_bytes = self.recvall(sock, 4)
            file_size = struct.unpack('<I', raw_file_size_bytes)[0]
            payload = self.recvall(sock, file_size)
            return version, status, filename, payload

        return version, status, filename, None

    def recvall(self, sock, count):
        buf = b''
        while count:
            newbuf = sock.recv(count)
            if not newbuf:
                raise ConnectionError("Socket connection was closed by the remote server.")
            buf += newbuf
            count -= len(newbuf)
        return buf

    def handle_server_response(self, status, filename, payload):
        if status == STATUS_FILE_RESTORED:
            filename_extension = filename.split('.')[1]
            new_filename = f'temp.{filename_extension}'
            with open(new_filename, 'wb') as f:
                f.write(payload)
            print(f"File {filename} was restored successfully")
            print(f"File contents saved to {new_filename}")
        elif status == STATUS_FILE_NOT_FOUND:
            print("File not found")
        elif status == STATUS_LIST_OK:
            print("Client files list:")
            print(payload.decode())
        elif status == STATUS_NO_FILES_FOUND:
            print("No files found for this client")
        elif status == STATUS_FILE_SAVED:
            print(f"File {filename} was saved successfully")
        elif status == STATUS_FILE_DELETED:
            print("File deleted")
        elif status == FAILURE:
            print("Server error")
        else:
            print("Unknown status code:", status)

    # Decorator for logging
    def log_operation(op_name):
        def decorator(func):
            def wrapper(self, *args, **kwargs):
                filename = None
                if args:
                    try:
                        filename = self.filenames[args[0]]
                    except IndexError:
                        pass
                print(f"Sending message to {op_name} the file: {filename if filename else 'N/A'}")
                return func(self, *args, **kwargs)
            return wrapper
        return decorator

    @log_operation("get list")
    def test_get_list(self, _unused=None):
        if self.filenames:
            message_get_list = self.create_message(OP_LIST, 0)
            self.send_and_receive(message_get_list)
        else:
            print("No filenames found!")

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
    print("1. Generating random ID")
    random_id = random.getrandbits(32)
    print(f"Random ID: {random_id}")

    print("2. Reading server configuration from server.info")
    with open('server.info', 'r') as f:
        server_address, server_port = f.readline().strip().split(':')
    server_port = int(server_port)

    tester = Tester(random_id, server_address, server_port)

    # Define the operations and the file indices for each operation
    operations = [
        (tester.test_get_list, 0),
        (tester.test_save, 0),  # Save first file
        (tester.test_save, 1),  # Save second file (course_book.pdf, if it's the second one)
        (tester.test_get_list, 0),
        (tester.test_restore, 0),
        (tester.test_delete, 0),
        (tester.test_restore, 0)
    ]

    for idx, (operation, file_index) in enumerate(operations, start=4):
        print(f"{idx}. {'='*10}")
        operation(file_index)

    print("11. Quitting the client")


if __name__ == '__main__':
    main()


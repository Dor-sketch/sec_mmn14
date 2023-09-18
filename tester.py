import socket
import struct
import random
import configparser



VERSION = 1

"""
enum class Status : uint16_t
{
    SUCCESS_SAVE = 210,
    SUCCESS_FILE_LIST = 211,
    ERROR_FILE_NOT_FOUND = 1001,
    ERROR_NO_FILES = 1002, // Error status when client has no files on server
    FAILURE = 1003,        // general error status
    PROCESSING = 1         // default status, still processing request - for inner use
};

// Op codes
static const uint8_t OP_SAVE_FILE = 100;
static const uint8_t OP_RESTORE_FILE = 200;
static const uint8_t OP_DELETE_FILE = 201;
static const uint8_t OP_GET_FILE_LIST = 202;
"""



def create_message(user_id, version, op, filename, file_contents):
    FIXED_FORMAT = '<I B B H' 
    NAME_FORMAT = '{}s'.format(len(filename))
    PAYLOAD_FORMAT = '{}s'.format(len(file_contents))

    message = struct.pack(FIXED_FORMAT, user_id, version, op, len(filename))
    message += struct.pack(NAME_FORMAT, filename)
    message += struct.pack('<I', len(file_contents)) + struct.pack(PAYLOAD_FORMAT, file_contents)

    print("packed message:", message.hex())
    return message


def receive_response(sock):
    # Receive the response header
    fixed_response_format = 'BBH'
    response_header = recvall(sock, struct.calcsize(fixed_response_format))
    version, status, name_len = struct.unpack(fixed_response_format, response_header)

    # Receive and unpack the filename
    filename = recvall(sock, name_len)

    # Receive and unpack the payload size
    file_size = struct.unpack('<I', recvall(sock, 4))[0]

    # Receive and unpack the payload
    payload = recvall(sock, file_size)

    # Return the response
    return version, status, filename.decode(), payload.decode()


def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf:
            raise ConnectionError("Socket connection was closed by the remote server.")
        buf += newbuf
        count -= len(newbuf)
    return buf

def receive_response(sock):
    # Receive the response header
    fixed_response_format = 'BBH'
    response_header = recvall(sock, struct.calcsize(fixed_response_format))
    version, status, name_len = struct.unpack(fixed_response_format, response_header)

    # Receive and unpack the filename
    filename = recvall(sock, name_len)

    # Receive and unpack the payload size
    file_size = struct.unpack('<I', recvall(sock, 4))[0]

    # Receive and unpack the payload
    payload = recvall(sock, file_size)

    # Return the response
    return version, status, filename.decode(), payload.decode()


def main():
    # create 4 bytes random id
    random_id =  random.getrandbits(32)
    print("Random ID:", random_id)

    # Read the server configuration from the server.info file
    with open('server.info', 'r') as f:
        line = f.readline().strip()
        server_address, server_port = line.split(':')

    # Convert the server port to an integer
    server_port = int(server_port)

    # Print the server address and port
    print('Server address:', server_address)
    print('Server port:', server_port)

    with open('backup.info', 'r') as f:
        filenames = f.read().split('\n')

    print("filenames:", filenames)

    with open(filenames[0], 'rb') as f:
        file_contents = f.read()
    print("file contents:", file_contents)

    filename = filenames[0].encode()

    # Create the message
    message = create_message(random_id, VERSION, 100, filename, file_contents)
    print("Sending message:", message.hex())

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((server_address, server_port))
        try:
            # Send the message to the server
            sock.sendall(message)

            # Receive the response
            version, status, filename, payload = receive_response(sock)

            # Display the response
            print('Version:', version)
            print('Status:', status)
            print('Filename:', filename.decode())
            print('Payload:', payload.decode())

        except ConnectionResetError:
            print("Connection reset by peer")


if __name__ == '__main__':
    main()
    
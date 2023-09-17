import socket
import struct

SERVER_ADDRESS = 'localhost'
SERVER_PORT = 1234
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

# Define the test data
user_id = 1233
version = 1
op = 202
filename = b'ligall.txt'
file_contents = b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'

FIXED_FORMAT = '<I B B H' 
NAME_FORMAT = '{}s'.format(len(filename))
PAYLOAD_FORMAT = '{}s'.format(len(file_contents))

message = struct.pack(FIXED_FORMAT, user_id, version, op, len(filename))
message += struct.pack(NAME_FORMAT, filename)
message += struct.pack('<I', len(file_contents)) + struct.pack(PAYLOAD_FORMAT, file_contents)

print("Sending message:", message.hex())

def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf:
            raise ConnectionError("Socket connection was closed by the remote server.")
        buf += newbuf
        count -= len(newbuf)
    return buf

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((SERVER_ADDRESS, SERVER_PORT))
    try:
        # Send the message to the server
        sock.sendall(message)
    
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
    
        # Display the response
        print('Version:', version)
        print('Status:', status)
        print('Filename:', filename.decode())
        print('Payload:', payload.decode())
    
    except ConnectionResetError:
        print("Connection reset by peer")

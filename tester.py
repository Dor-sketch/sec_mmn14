import socket
import struct

SERVER_ADDRESS = 'localhost'  # Replace with the IP address or hostname of the server
SERVER_PORT = 1234  # Replace with the port number of the server



# Define the test data
user_id = 1233
version = 1
op = 100
filename = b'dor.txt'
file_contents = b'acx'

# Compute lengths
name_len = len(filename)
payload_size = len(file_contents)
print("playload size: ", payload_size)
print("\n\n\n filename: ", filename)

# Create format strings
FIXED_FORMAT = '<I B B H' # < for little endian, I for unsigned int, B for unsigned char, H for unsigned short
NAME_FORMAT = '{}s'.format(name_len)
PAYLOAD_FORMAT = '{}s'.format(payload_size)

print(user_id, version, op, name_len)
# Pack data
fixed_data = struct.pack(FIXED_FORMAT, user_id, version, op, name_len)
name_data = struct.pack(NAME_FORMAT, filename)
payload_data = struct.pack(PAYLOAD_FORMAT, file_contents)

# Concatenate
message = fixed_data + name_data + struct.pack('<I', payload_size) + payload_data

print("message: ", message)
print(message.hex())
def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf:  # Connection closed
            raise ConnectionError("Socket connection was closed by the remote server.")
        buf += newbuf
        count -= len(newbuf)
    return buf

# Connect to the server and send the message
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((SERVER_ADDRESS, SERVER_PORT))
    sock.sendall(message)

    # Receive the response header
    fixed_response_format = 'BBH'  # B for version, B for status, H for name_len
    fixed_size = struct.calcsize(fixed_response_format)
    response_header = sock.recv(fixed_size)
    version, status, name_len = struct.unpack(fixed_response_format, response_header)

    # Receive and unpack the filename using the obtained name_len
    filename_format = '{}s'.format(name_len)
    try:
        filename = struct.unpack(filename_format, recvall(sock, name_len))[0]
    except ConnectionResetError:
        print("Connection was reset by the server.")    # Receive and unpack the payload size
    size_format = 'I'
    size = struct.unpack(size_format, sock.recv(4))[0]

    # Receive the payload using the obtained size
    payload = recvall(sock, size)

# Print the response
print('Version:', version)
print('Status:', status)
print('Filename:', filename.decode())
print('Payload:', payload.decode())
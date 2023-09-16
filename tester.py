import socket
import struct

# Define the server address and port
SERVER_ADDRESS = 'localhost'
SERVER_PORT = 12345

# Define the message format
MESSAGE_FORMAT = '<IBBH{}sI{}s'.format('s' * 256, 's' * 1024)

# Define the test data
user_id = 1234
version = 1
op = 1
filename = 'test.txt'
file_contents = 'Hello, world!'

# Pack the message
name_len = len(filename)
payload_size = len(file_contents)
message = struct.pack(MESSAGE_FORMAT, user_id, version, op, name_len, filename.encode(), payload_size, file_contents.encode())

# Connect to the server and send the message
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((SERVER_ADDRESS, SERVER_PORT))
    sock.sendall(message)

    # Receive the response
    response = sock.recv(1024)

# Unpack the response
version, status, name_len, filename, payload = struct.unpack(MESSAGE_FORMAT[:9] + '{}s'.format(name_len) + '{}s'.format(payload_size), response)

# Print the response
print('Version:', version)
print('Status:', status)
print('Filename:', filename.decode())
print('Payload:', payload.decode())
# Description: This file contains a client that will test the server's functionality.
#              It will send a series of messages to the server and print the responses.


import socket
import struct
import random
import configparser

VERSION = 1
OP_SAVE = 100
OP_RESTORE = 200
OP_DELETE = 201
OP_LIST = 202
STATUS_FILE_RESTORED = 210
STATUS_FILE_NOT_FOUND = 1001
STATUS_LIST_OK = 211
STATUS_NO_FILES_FOUND = 1002
STATUS_FILE_SAVED = 212 # the spec doesn't say anything about this status code, but I think it should be there



def create_message(user_id, version, op, filename, file_contents):
    FIXED_FORMAT = '<I B B H' 
    NAME_FORMAT = '{}s'.format(len(filename))
    PAYLOAD_FORMAT = '{}s'.format(len(file_contents))

    message = struct.pack(FIXED_FORMAT, user_id, version, op, len(filename))
    message += struct.pack(NAME_FORMAT, filename)
    message += struct.pack('<I', len(file_contents)) \
        + struct.pack(PAYLOAD_FORMAT, file_contents)

    print("packed message:", message.hex())
    return message


def receive_response(sock):
    # Receive the response header
    fixed_response_format = 'BBH'
    response_header = recvall(sock, struct.calcsize(fixed_response_format))
    version, status, name_len = \
        struct.unpack(fixed_response_format, response_header)

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
    fixed_response_format = 'BBH'
    response_header = recvall(sock, struct.calcsize(fixed_response_format))
    version, status, name_len = struct.unpack(fixed_response_format, response_header)
    filename = recvall(sock, name_len)
    file_size = struct.unpack('<I', recvall(sock, 4))[0]
    payload = recvall(sock, file_size)

    return version, status, filename.decode(), payload.decode()

def send_nd_recieve(message):
    print("Sending message:", message.hex())
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((server_address, server_port))
        
        try:
            # Send the message to the server
            sock.sendall(message)
        
            # Receive the response 
            version, status, filename1, payload = receive_response(sock)
            print('Version:', version)
            print('Status:', status)
            print('Filename:', filename1.decode())
            print('Payload:', payload.decode())

        except ConnectionResetError:
            print("Connection reset by peer")
    print()

    if (status == STATUS_FILE_RESTORED):
        with open(temp, 'wb') as f:
            f.write(payload)

    elif (status == STATUS_FILE_NOT_FOUND):
        print("File not found")

    elif (status == STATUS_LIST_OK):
        print("Client files list:")
        print(payload.decode())

    elif (status == STATUS_NO_FILES_FOUND):
        print("No files found for this client")

    elif (status == STATUS_FILE_SAVED):
        print("File {} was saved".format(filename1.decode()))

    else:
        print("Unknown status code:", status)
                

           
def main():
    # ===========================================================
    # (1) create 4 bytes random id ==============================
    random_id =  random.getrandbits(32)
    print("Random ID:", random_id)

    # ===========================================================
    # (2) Read the server configuration from the server.info file
    with open('server.info', 'r') as f:
        line = f.readline().strip()
        server_address, server_port = line.split(':')

    server_port = int(server_port)

    print('Server address:', server_address)
    print('Server port:', server_port)

    # ===========================================================
    # (3) Read the file names from backup.info 
    with open('backup.info', 'r') as f:
        filenames = f.read().split('\n')
    print("filenames:", filenames)

    with open(filenames[0], 'rb') as f:
        file_contents1 = f.read()

    with open(filenames[1], 'rb') as f:
        file_contents2 = f.read()

    filename1 = filenames[0].encode()
    filename2 = filenames[1].encode()

    # ===========================================================
    # (4) msg server to get file list
    #     (should respond with STATUS_NO_FILES_FOUND)
    message_get_list = create_message(random_id, VERSION, OP_LIST,\
        filename1, file_contents1)
    send_nd_recieve(message_get_list)

    # ===========================================================
    # (5) msg server to save the first file from backup.info
    #     (no status code for this one according to the spec)
    #     (should respond with STATUS_FILE_SAVED)
    message = create_message(random_id, VERSION, OP_SAVE,\
        filename1, file_contents)
    send_nd_recieve(message)

    # ===========================================================
    # (6) msg server to save the second file from backup.info
    #   (should respond with STATUS_FILE_SAVED)
    message = create_message(random_id, VERSION, OP_SAVE,\
        filename2, file_contents2)
    send_nd_recieve(message)

    # ===========================================================
    # (7) msg server to get file list again
    #     (should respond STATUS_LIST_OK and list of files)
    send_nd_recieve(message_get_list)

    # ===========================================================
    # (8) msg srver to restore the first file from backup.info
    #     (should respond with STATUS_FILE_RESTORED)
    #     (should create a temp file with the restored file contents)
    message = create_message(random_id, VERSION, OP_RESTORE,\
        filename1, file_contents)

    # ===========================================================
    # (9) msg server to delete the first file from backup.info
    #     (should respond with STATUS_FILE_DELETED)
    message = create_message(random_id, VERSION, OP_DELETE,\
        filename1, file_contents)

    # ===========================================================
    # (10) msg server to restore the first file from backup.info
    #      (should fail)
    message = create_message(random_id, VERSION, OP_RESTORE,\
        filename1, file_contents)

    # ===========================================================
    # (11) quitting the client







if __name__ == '__main__':
    main()
    
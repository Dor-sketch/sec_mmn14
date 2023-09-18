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
FAILURE = 1003 
STATUS_FILE_SAVED = 212 # the spec doesn't say anything about this status code, but I think it should be there
STATUS_FILE_DELETED = 213 # same as above



def create_message(user_id, version, op, filename, file_contents=b''):
    FIXED_FORMAT = '<I B B H' 
    NAME_FORMAT = '{}s'.format(len(filename))
    PAYLOAD_FORMAT = '{}s'.format(len(file_contents))

    message = struct.pack(FIXED_FORMAT, user_id, version, op, len(filename))
    message += struct.pack(NAME_FORMAT, filename)
    message += struct.pack('<I', len(file_contents)) \
        + struct.pack(PAYLOAD_FORMAT, file_contents)

    # print("packed message:", message.hex())
    return message




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
    fixed_response_format = '<BHH'
    response_header = recvall(sock, struct.calcsize(fixed_response_format))
    
    print("response_header in bytes:", bytes(response_header).hex())

    version, status, name_len = struct.unpack(fixed_response_format, response_header)
    print("version:", version)
    print("status:", status)
    print("name_len:", name_len)

    # no more data to receive if the status is STATUS_NO_FILES_FOUND
    if (status == STATUS_FILE_NOT_FOUND or status == STATUS_NO_FILES_FOUND or status == FAILURE):
        return version, status, None, None

    # get the filename
    filename = recvall(sock, name_len)
    

    # get the payload if the status is STATUS_LIST_OK or STATUS_FILE_RESTORED
    if status in [STATUS_LIST_OK, STATUS_FILE_RESTORED]:
        raw_file_size_bytes = recvall(sock, 4)        
        # print(raw_file_size_bytes.hex())
        file_size = struct.unpack('<I', raw_file_size_bytes)[0]

        raw_payload = recvall(sock, file_size)
        # print(raw_payload.hex())
        return version, status, filename, raw_payload

    return version, status, filename, None














def send_nd_recieve(message, server_address, server_port):
    # print("Sending message:", message.hex())
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((server_address, server_port))
        
        try:
            # Send the message to the server
            sock.sendall(message)
        
            # Receive the response 
            version, status, filename1, payload = receive_response(sock)
            print('Version:', version)
            print('Status:', status)
            print('Filename:', filename1)
            # print('Payload:', payload)

        except ConnectionResetError:
            print("Connection reset by peer")
    print()

    if (status == STATUS_FILE_RESTORED):
        filename_extension = filename1.decode().split('.')[1]
        filename = 'temp.{}'.format(filename_extension)
        with open(filename, 'wb') as f:
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
    print("=========================================================")
    print("1. Generating random ID")
    random_id =  random.getrandbits(32)
    print("Random ID:", random_id)

    # ===========================================================
    # (2) Read the server configuration from the server.info file
    print("=========================================================")
    print("2. Reading server configuration from server.info")
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
    print("=========================================================")
    print("4. Sending message to get file list")
    message_get_list = create_message(random_id, VERSION, OP_LIST,\
        filename1, file_contents1)
    send_nd_recieve(message_get_list, server_address, server_port)

    # # ===========================================================
    # # (5) msg server to save the first file from backup.info
    # #     (no status code for this one according to the spec)
    # #     (should respond with STATUS_FILE_SAVED)
    print("=========================================================")
    print("5. Sending message to save the first file")
    message = create_message(random_id, VERSION, OP_SAVE,\
        filename1, file_contents1)
    send_nd_recieve(message, server_address, server_port)

    # ===========================================================
    # (6) msg server to save the second file from backup.info
    #   (should respond with STATUS_FILE_SAVED)
    print("=========================================================")
    print("6. Sending message to save the second file")
    message = create_message(random_id, VERSION, OP_SAVE,\
        filename2, file_contents2)
    send_nd_recieve(message, server_address, server_port)

    # ===========================================================
    # (7) msg server to get file list again
    #     (should respond STATUS_LIST_OK and list of files)
    print("=========================================================")
    print("7. Sending message to get file list")
    send_nd_recieve(message_get_list, server_address, server_port)

    # ===========================================================
    # (8) msg srver to restore the first file from backup.info
    #     (should respond with STATUS_FILE_RESTORED)
    #     (should create a temp file with the restored file contents)
    print("=========================================================")
    print("8. Sending message to restore the first file")
    message = create_message(random_id, VERSION, OP_RESTORE,\
        filename1)
    send_nd_recieve(message, server_address, server_port)

    # ===========================================================
    # (9) msg server to delete the first file from backup.info
    #     (should respond with STATUS_FILE_DELETED)
    print("9. =========================================================")
    print("Sending message to delete the first file")
    message = create_message(random_id, VERSION, OP_DELETE,\
        filename1)
    send_nd_recieve(message, server_address, server_port)

    # ===========================================================
    # (10) msg server to restore the first file from backup.info
    #      (should fail)
    print("10. =========================================================")
    print("Sending message to restore the first file")
    message = create_message(random_id, VERSION, OP_RESTORE,\
        filename1)
    send_nd_recieve(message, server_address, server_port)

    # ===========================================================
    # (11) quitting the client
    print("11. =========================================================")
    print("Quitting the client")







if __name__ == '__main__':
    main()
    
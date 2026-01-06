import socket
import threading
import sys
import os

def receive_messages(sock):
    while True:
        try:
            data, address = sock.recvfrom(1024)
            if data.decode().startswith("transfer"):
                receive_file(sock, data.decode().split()[1])
            else:
                print(f"Message received from {address}: {data.decode()}")
        except Exception as e:
            print(f"Error receiving message: {e}")
            break

def send_messages(sock, server_address):
    while True:
        try:
            message = input("Enter message (or 'file <filename>' to send a file): ")
            if message.lower() == "quit":
                print("Closing connection.")
                sock.sendto(message.encode(), server_address)
                break
            elif message.lower().startswith("file"):
                send_file(sock, server_address, message.split()[1])
            else:
                sock.sendto(message.encode(), server_address)
        except Exception as e:
            print(f"Error sending message: {e}")
            break

def send_file(sock, server_address, filename):
    try:
        with open(filename, "rb") as file:
            filename_bytes = filename.encode()
            sock.sendto(b"transfer " + filename_bytes, server_address)
            data = file.read(1024)
            while data:
                sock.sendto(data, server_address)
                data = file.read(1024)
        sock.sendto(b"done", server_address)
        print(f"File '{filename}' sent successfully.")
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    except Exception as e:
        print(f"Error sending file: {e}")

def receive_file(sock, filename):
    try:
        with open(filename, "wb") as file:
            while True:
                data, address = sock.recvfrom(1024)
                if data == b"done":
                    break
                file.write(data)
        print(f"File '{filename}' received successfully.")
    except Exception as e:
        print(f"Error receiving file: {e}")

def main():
    host = "localhost"

    # Get port number from command-line argument or use default port 12345
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    else:
        port = 12345

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Bind the socket to the host and port
    sock.bind((host, port))

    # Prompt user for server address
    server_host = input("Enter server host address: ")
    server_port = int(input("Enter server port number: "))
    server_address = (server_host, server_port)

    # Start thread to receive messages
    receive_thread = threading.Thread(target=receive_messages, args=(sock,))
    receive_thread.start()

    # Start sending messages
    send_messages(sock, server_address)

    # Wait for receive thread to complete
    receive_thread.join()

    # Close the socket
    sock.close()

if __name__ == "__main__":
    main()

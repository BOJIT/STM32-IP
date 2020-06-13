import socket
import time

UDP_IP = "192.168.1.90"
UDP_PORT = 1883
MESSAGE = "My offspring wanted scuba gear for his birthday. Thats all he wanted. I am not letting him swim off by himself to be taken for a baby seal by a great white and I will be fucked if I am going in there with him to be taken for an old skinny seal by a great white. When I explained to him that scuba gear is only for the sea and he, being such a small human, would be taken for a baby seal by a great white, he stated that he would see them coming because of the mask and added 'speargun' and 'knife' to his birthday list."

print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
print("message:", MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

for i in range(0,10):
    sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
    time.sleep(0.02)

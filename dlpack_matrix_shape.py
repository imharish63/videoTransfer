
import numpy as np
import socket
import time
from time import perf_counter
import concurrent.futures
from imutils.video import FPS


WIDTH = 128
HEIGHT = 128
MODULUS = 256
BATCH_SIZE = 16
UDP_IP = "192.168.1.10"
# UDP_IP = "172.16.7.254" 
FRAMES = 2
UDP_PORT = 5001
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# A_orig = np.arange(256).reshape((16,16))
# print(A_orig)

# A_packed_notranspose = A_orig.reshape(4,4,4,4)
# print(A_packed_notranspose)

# A_packed = A_packed_notranspose.transpose((0,2,1,3))

# print(A_packed)


print("[info] sampling frames from webacm ...")
print(" Height and width = %d X %d " %(HEIGHT,WIDTH) )

fps = FPS().start()

seq_num = 0


markerBit = 0
packetNumFrame = 0
k = 0

data = (np.remainder(np.arange(WIDTH*HEIGHT),MODULUS)).reshape((WIDTH,HEIGHT))
image = data.astype(np.uint8)
before_transpose = image.reshape(WIDTH//BATCH_SIZE, BATCH_SIZE, HEIGHT//BATCH_SIZE , BATCH_SIZE)
strides_of_array_bf_transpose = before_transpose.strides
image_packed = image.reshape(WIDTH//BATCH_SIZE, BATCH_SIZE, HEIGHT//BATCH_SIZE , BATCH_SIZE).transpose((0,2,1,3))
print(image_packed)
strides_of_array = image_packed.strides
image_flattened = image_packed.flatten()
print(image_flattened)

while k < FRAMES:
    # image = (k % 256)*image
    for i in range ((len(image_flattened)//1400)+1):
        
        if (i <(len(image_flattened)//1400)):
            markerBit = 0
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , image_flattened[1400*i:(1400+1400*i)]), axis=None)
            buf = bytes(buf_header)
        
        elif (i == (len(image_flattened)//1400)):
            markerBit = 1
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , image_flattened[1400*i:len(image_flattened)]), axis=None)
            buf = bytes(buf_header)
            packetNumFrame = 0
        else:
            print("program looking out of bounds for image data ")
         
      #print(buf)
        sock.sendto(buf_header, (UDP_IP, UDP_PORT))
        seq_num += 1
        packetNumFrame += 1
    k += 1
    packetNumFrame = 0
    time.sleep(12/1000)
fps.stop()
print ("[INFO] elasped time : {:.2f}".format(fps.elapsed()))
print ("[INFO] FPS : {:.2f}".format(FRAMES/fps.elapsed()))

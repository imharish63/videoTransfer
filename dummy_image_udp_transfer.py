#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time
import numpy as np
from time import perf_counter
import concurrent.futures
from imutils.video import FPS

UDP_IP = "192.168.1.10"
FRAMES = 5
WIDTH = 256 
HEIGHT = 256
UDP_PORT = 5001
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


        
        
print("[info] sampling frames from webacm ...")
print(" Height and width = %d X %d " %(HEIGHT,WIDTH) )


fps = FPS().start()
# start_time = perf_counter()
seq_num = 0


markerBit = 0
packetNumFrame = 0
k = 0

imag = np.array(np.ones(WIDTH*HEIGHT, dtype=np.uint8))
while k < FRAMES:
    image = (k % 256)*imag
    for i in range (int(len(image)/1400)+1):
        
        if (i <int(len(image)/1400)):
            markerBit = 0
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , image[1400*i:(1400+1400*i)]), axis=None)
            buf = bytes(buf_header)
        
        elif (i == int(len(image)/1400)):
            markerBit = 1
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , image[1400*i:len(image)]), axis=None)
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

        
        





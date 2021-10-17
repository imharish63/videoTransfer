#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Oct 14 11:34:02 2021

@author: harish
"""
import cv2
import socket
#import sys
import numpy as np
# Open the device at the ID 0
# Use the camera ID based on
# /dev/videoID needed
cap = cv2.VideoCapture(1)
UDP_IP = "192.168.0.1"
UDP_PORT = 5001

#Check if camera was opened correctly
if not (cap.isOpened()):
    print("Could not open video device")


#Set the resolution
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 800)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 608)
seq_num = 0 # packet number

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
# Capture frame-by-frame
while(True):
    markerBit = 0
    packetNumFrame = 0 
    ret, frame = cap.read() #
     
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray_byte = np.array(gray.flatten(order='C').tolist(),dtype=np.uint8)
    
    for i in range (int(len(gray_byte)/1400)+1):
        # all packets expect the last packet
        if (i <int(len(gray_byte)/1400)):
            markerBit = 0 # first byte after first 4 bytes of sequence num
            packetNumFrame = (packetNumFrame & 0x0000ffff) # 7 and 8th btye for packet num of frame and 6th byte is reserved  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff)], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray_byte[1400*i:(1400+1400*i)]), axis=None) # merge header and 1400 bytes of data
            buf = bytes(buf_header) # convert to byteoder to transmit it through UDP
            
        elif (i == int(len(gray_byte)/1400)):
            markerBit = 1 # last packet of frame set to 1 is markerbit
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff)], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray_byte[1400*i:len(gray_byte)]), axis=None)
            buf = bytes(buf_header)
            packetNumFrame = 0
        else:
            print("program looking out of bounds for image data ")
        #print(buf)
        sock.sendto(buf_header, (UDP_IP, UDP_PORT))
        seq_num += 1
        packetNumFrame += 1
        
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
            #Waits for a user input to quit the application
               
    
# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()

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
seq_num = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
# Capture frame-by-frame
while(True):
    markerBit = 0
    packetNumFrame = 0 
    ret, frame = cap.read()
    #print(frame);
    #byte_frame = frame.tobytes()
    #print(byte_frame)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray_byte = np.array(gray.flatten(order='C').tolist(),dtype=np.uint8)
    #gray_byte = gray.tobytes()
    #gray_byte = gray.flatten(order='C').hex()
    print(gray_byte)
    print(len(gray_byte))
    for i in range (int(len(gray_byte)/1400)+1):
        
        if (i <int(len(gray_byte)/1400)):
            markerBit = 0
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff)], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray_byte[1400*i:(1400+1400*i)]), axis=None)
            buf = bytes(buf_header)
            
        elif (i == int(len(gray_byte)/1400)):
            markerBit = 1
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff)], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray_byte[1400*i:len(gray_byte)]), axis=None)
            buf = bytes(buf_header)
            packetNumFrame = 0
        else:
            print("program looking out of bounds for image data ")
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break   
        #print(buf)
        sock.sendto(buf_header, (UDP_IP, UDP_PORT))
        seq_num += 1
        packetNumFrame += 1
        
    
    
        # Display the resulting frame
        
    #cv2.imshow("preview",frame)
    #cv2.imshow('Gray image', gray)
    cv2.imwrite("outputImageGray.jpg", gray)   
    cv2.imwrite("outputImage.jpg", frame)
    #im = cv2.imread('outputImage.jpg')
    #im_resize = cv2.resize(im, (320, 240))
    
    #is_success, im_buf_arr = cv2.imencode(".jpg", im_resize)
    #byte_im = im_buf_arr.tobytes()
    
    #print(im_buf_arr,np.shape(im_buf_arr))
    
    #print(byte_im,sys.getsizeof(byte_im))
    
    # or using BytesIO
    # io_buf = io.BytesIO(im_buf_arr)
    # byte_im = io_buf.getvalue()
        #Waits for a user input to quit the application
    
    
# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()

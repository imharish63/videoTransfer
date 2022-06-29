#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Oct 14 11:34:02 2021

@author: harish
"""
import cv2
import socket
#import sys
import argparse
import time
import concurrent.futures
import numpy as np
from imutils.video import FPS
import torch
# from PIL import image
import torchvision.transforms as transforms
from torch.utils.dlpack import to_dlpack
# Open the device at the ID 0
# Use the camera ID based on
# /dev/videoID needed
cap = cv2.VideoCapture(0)
UDP_IP = "192.168.1.10"
UDP_PORT = 5001
WIDTH = 320
HEIGHT = 240

#Check if camera was opened correctly
if not (cap.isOpened()):
    print("Could not open video device")

ap = argparse.ArgumentParser()
ap.add_argument("-n", "--num-frames", type=int , default=15,
            help= "# of frames to loop over for FPS test")
ap.add_argument("-d", "--display", type=int , default=-1, 
            help = "Whether or not frames should be displayed")
args = vars(ap.parse_args())

print("[info] sampling frmaes from webacm ...")

fps = FPS().start()
#Set the resolution
cap.set(cv2.CAP_PROP_FRAME_WIDTH, WIDTH)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, HEIGHT)
seq_num = 0
count = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
# Capture frame-by-frame
while fps._numFrames < args["num_frames"]:
    
    markerBit = 0
    packetNumFrame = 0 
    ret, frame = cap.read()
    #print(frame);
    #byte_frame = frame.tobytes()
    #print(byte_frame)
    transform = transforms.Compose([transforms.ToTensor()])
    frame_tensor = transform(frame)
    dlp = to_dlpack(frame_tensor)
    dlp.get_shape()
    print (dlp)
    # dlp_byte = np.array(dlp.flatten(order='C').tolist(),dtype=np.uint8)
    gray1 = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    #dlp1 = to_dlpack(gray1)
    num_rows, num_cols = gray1.shape
    gray=np.array(np.reshape(gray1, (num_rows * num_cols) , order='F'),dtype=np.uint8)
    count +=1
    #gray_byte = gray.tobytes()
    #gray_byte = gray.flatten(order='C').hex()
    #print(gray)
    #print(count)
    
    for i in range (int(len(gray)/1400)+1):
        
        if (i <int(len(gray)/1400)):
            markerBit = 0
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray[1400*i:(1400+1400*i)]), axis=None)
            buf = bytes(buf_header)
        
        elif (i == int(len(gray)/1400)):
            markerBit = 1
            packetNumFrame = (packetNumFrame & 0x0000ffff)  
            buf = np.array([(seq_num & 0xff000000) >> 24 ,(seq_num & 0x00ff0000) >>16 ,(seq_num & 0x0000ff00) >>8 , (seq_num & 0x000000ff) ,markerBit ,(packetNumFrame & 0x00ff0000) >> 16,(packetNumFrame & 0x0000ff00)>>8 ,(packetNumFrame & 0x000000ff),(int(WIDTH/8) & 0x000000ff ),(int(HEIGHT/8) & 0x000000ff )], dtype=np.uint8)
            buf_header = np.concatenate((buf , gray[1400*i:len(gray)]), axis=None)
            buf = bytes(buf_header)
            packetNumFrame = 0
        else:
            print("program looking out of bounds for image data ")
         
     #print(buf)
        sock.sendto(buf_header, (UDP_IP, UDP_PORT))
        seq_num += 1
        packetNumFrame += 1
     
    fps.update()
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
    # time.sleep(1000/1000)

fps.stop()
print ("[INFO] elasped time : {:.2f}".format(fps.elapsed()))
print ("[INFO] FPS : {:.2f}".format(fps.fps()))

       
    
    
# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()

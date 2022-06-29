#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Nov 25 14:53:27 2021

@author: harish
"""

# import the necessary packages 
from __future__ import print_function
from imutils.video import WebcamVideoStream
from imutils.video import FPS
import argparse
import imutils
import cv2
import time

ap = argparse.ArgumentParser()
ap.add_argument("-n", "--num-frames", type=int , default=100,
                help= "# of frames to loop over for FPS test")
ap.add_argument("-d", "--display", type=int , default=-1, 
                help = "Whether or not frames should be displayed")
args = vars(ap.parse_args())

print("[INFO] sampling frames from webcam ...")
stream = cv2.VideoCapture(0)
fps = FPS().start()

while fps._numFrames < args["num_frames"]:
    (grabbed , frame) = stream.read()
    frame = imutils.resize(frame , width = 400)
    
    
    if args["display"] > 0:
        cv2.imshow("Frame", frame)
        key = cv2.waitKey(1) & 0xFF
        
    fps.update()
fps.stop()
print ("[INFO] elasped time : {:.2f}".format(fps.elapsed()))
print ("[INFO] FPS : {:.2f}".format(fps.fps()))


stream.release()
cv2.destroyAllWindows()

print("[info] sampling Threaded frames from webacm ...")
vs = WebcamVideoStream(src=0).start()
fps = FPS().start()

while fps._numFrames < args["num_frames"]:
    frame = vs.read()
    frame = imutils.resize(frame , width = 400)
    
    
    if args["display"] > 0:
        cv2.imshow("Frame", frame)
        key = cv2.waitKey(1) & 0xFF
        
    fps.update()
    time.sleep(6/1000)
fps.stop()
print ("[INFO] elasped time : {:.2f}".format(fps.elapsed()))
print ("[INFO] FPS : {:.2f}".format(fps.fps()))

stream.release()
cv2.destroyAllWindows()
    
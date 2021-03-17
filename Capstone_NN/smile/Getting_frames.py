import cv2
import math

videoFile = "/home/sepehr/python_projects/Ratous/Ratousv1/ratous-models/test/smile/data/positive/1.mp4"
imagesFolder = "/home/sepehr/python_projects/Ratous/Ratousv1/ratous-models/test/smile/data/positive"
cap = cv2.VideoCapture(videoFile)
frameRate = cap.get(5) #frame rate
while(cap.isOpened()):
    frameId = cap.get(1) #current frame number
    ret, frame = cap.read()
    if (ret != True):
        break
    if (frameId % math.floor(frameRate) == 0):
        filename = imagesFolder + "/image_" +  str(int(frameId)) + ".jpg"
        cv2.imwrite(filename, frame)
cap.release()
print("Done!")
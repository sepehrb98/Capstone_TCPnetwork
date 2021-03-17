import cv2 as cv
import dlib
import numpy as np
import os
import sys
from tensorflow.keras.models import load_model
from pathlib import Path
base= os.path.dirname(os.path.realpath(__file__))+"/"



def smile_detector(inputImage):
  try:
    ###Initialize Model
    network= load_model(base+r"smile_detection_CNN_v100.h5")
    classes=["Normal","Smile"]
    detector=dlib.get_frontal_face_detector()
    image = cv.imread(inputImage, -1)
    smiles=0
    normal=0
    rotation = 0
    correct_rotation = 0
    face_detected=0
    
    frame_gs_process=cv.cvtColor(image,cv.COLOR_BGR2GRAY)
    frame_gs_process=cv.cvtColor(frame_gs_process,cv.COLOR_GRAY2BGR)
    ###Decrease Resolution
    width = int(frame_gs_process.shape[1])
    height = int(frame_gs_process.shape[0])

    if( (width >1000)or(height>1000)):
        scale_percent = 30
    elif ((width >720)or(height>720)):
       scale_percent = 80
    else:
      scale_percent = 100
      width = int(frame_gs_process.shape[1] * scale_percent / 100)
      height = int(frame_gs_process.shape[0] * scale_percent / 100)
      dim = (width, height)
      frame_gs_process = cv.resize(frame_gs_process, dim, interpolation = cv.INTER_AREA)
      frame_bgr_process = cv.resize(image, dim, interpolation = cv.INTER_AREA)
    ###Face & Smile Detection
    rects=detector(frame_gs_process,1)
    if rects == 0:
      return({'retCode': 3, 'message': "AI alogrithm: No face detected.\n"})

    for item in rects:
      face_detected+=1
      (x1,y1)=(item.left(),item.top())
      (x2,y2)=(item.right(),item.bottom())
      roi=frame_gs_process[y1:y2,x1:x2]
      if(roi.shape[0]>64) and (roi.shape[1]>64):
        roi=cv.resize(roi,(64,64))
        roi=roi/255
        roi=np.expand_dims(roi,axis=0)
        probs=network.predict(roi)[0]
        predicted_label=np.argmax(probs,axis=0)
        if(classes[predicted_label]=="Smile"):
          smiles+=1
        else:
          normal+=1
    
    ###When the video is Valid finish the process
    ###Final outputs
    cv.destroyAllWindows()
    if (smiles - normal>0):
      return({'retCode': 0, 'message': "AI algorithm: Smile detected.\n"})
    
    else:
      return({'retCode': 2, 'message': "AI algorithm: No smile detected.\n"})

  
  except Exception as e:
    print(e)
    return {'retCode' : 3, 'message' : 'AI algorithm: Unknown error. Please try again.\n'}

a = smile_detector("/home/sepehr/python_projects/Ratous/Ratousv1/ratous-models/test/smile/data/positive/image_0.jpg")
print(a['message'])
sys.exit(a['retCode'])

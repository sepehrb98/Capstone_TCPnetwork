import cv2 as cv
import dlib
import numpy as np
import os
import sys
from tensorflow.keras.models import load_model
from pathlib import Path
base= os.path.dirname(os.path.realpath(__file__))+"/"



def smile_detector(inputvideo):
  try:
    ###Initialize Model
    network= load_model(base+r"smile_detection_CNN_v100.h5")
    classes=["Normal","Smile"]
    detector=dlib.get_frontal_face_detector()
    cap=cv.VideoCapture(inputvideo)
    smiles=0
    normal=0
    rotation = 0
    correct_rotation = 0
    face_detected=0
    frame_num=0
    saveframes=[]
    ###Process Video
    while True:
    ###Get frame
      ret,frame_org=cap.read()
      if frame_org is None:
        break
      frame_num+=1
    ###Convert to Grayscale
      frame_gs_process=cv.cvtColor(frame_org,cv.COLOR_BGR2GRAY)
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
      frame_bgr_process = cv.resize(frame_org, dim, interpolation = cv.INTER_AREA)
    ###Rotation if needed
      if (face_detected == 0)  and (frame_num > 1) and (rotation==0):
        frame_org = cv.rotate(frame_org, cv.ROTATE_90_COUNTERCLOCKWISE)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_90_COUNTERCLOCKWISE)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_90_COUNTERCLOCKWISE)
        rotation=1
      elif (face_detected == 0)  and (frame_num > 1) and (rotation==1):
        frame_org = cv.rotate(frame_org, cv.ROTATE_180)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_180)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_180)
        rotation=2
      elif (face_detected == 0)  and (frame_num > 1) and (rotation==2):
        frame_org = cv.rotate(frame_org, cv.ROTATE_90_CLOCKWISE)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_90_CLOCKWISE)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_90_CLOCKWISE)
        rotation=3
      elif (face_detected == 0)  and (frame_num > 1) and (rotation==3):
        rotation=4
      elif (frame_num > 2) and (correct_rotation==1) :
        frame_org = cv.rotate(frame_org, cv.ROTATE_90_COUNTERCLOCKWISE)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_90_COUNTERCLOCKWISE)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_90_COUNTERCLOCKWISE)
      elif (frame_num > 2) and (correct_rotation==2) :
        frame_org = cv.rotate(frame_org, cv.ROTATE_180)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_180)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_180)
      elif (frame_num > 2) and (correct_rotation==3) :
        frame_org = cv.rotate(frame_org, cv.ROTATE_90_CLOCKWISE)
        frame_gs_process = cv.rotate(frame_gs_process, cv.ROTATE_90_CLOCKWISE)
        frame_bgr_process = cv.rotate(frame_bgr_process, cv.ROTATE_90_CLOCKWISE)
    ###Face & Smile Detection
      rects=detector(frame_gs_process,1)
      for item in rects:
        saveframes.append(frame_org)
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
    ###Show resluts on video(This part isn't necessary if you dont want it comment it.)
        '''cv.rectangle(frame_bgr_process,(x1,y1),(x2,y2),(200,122,233),3)
        text="{}: {}".format(classes[predicted_label],probs[predicted_label])
        cv.putText(frame_bgr_process,text,(x1-10,y1-10),cv.FONT_HERSHEY_SIMPLEX,.5,(200,122,233),2)
      cv.imshow("Smile Detection",frame_bgr_process)
      if cv.waitKey(1) == ord('q'):
        break'''
    ###Correct Rotation
      if face_detected > 0 :
        correct_rotation= rotation
    ###When the video is Valid finish the process
      if (smiles>10) and (face_detected>=3) :
        break
    ###Final outputs
    cv.destroyAllWindows()
    if (smiles>10):
  ###Frame Saving for face matching
      if(face_detected<3):
        return({'retCode': 3, 'message': "AI alogrithm: No face detected.\n"})
      '''
      Path(outputframes).mkdir(parents=True, exist_ok=True)
      for smileframe in range(3):
        file_name = "smile_frame" + str(smileframe)
        filename = os.path.join(outputframes, file_name + ".jpg")
        cv.imwrite(filename,saveframes[smileframe])
      '''
      return({'retCode': 0, 'message': "AI algorithm: Smile detected.\n"})

    elif frame_num == 0:
      return({'retCode': 3, 'message': "AI algorithm: Video file not found.\n"})

    elif face_detected == 0:
      return({'retCode': 1, 'message': "AI alogrithm: No face detected.\n"})

    else:
      return({'retCode': 2, 'message': "AI algorithm: No smile detected.\n"})

  
  except Exception as e:
    return {'retCode' : 3, 'message' : 'AI algorithm: Unknown error. Please try again.\n'}

a = smile_detector(sys.argv[1])
print(a['message'])
sys.exit(a['retCode'])

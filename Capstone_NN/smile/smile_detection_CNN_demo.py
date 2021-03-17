import glob
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import models
from tensorflow.keras import layers
from tensorflow.keras.utils import to_categorical

#Initialize Dataset
data=[]
labels=[]
for item in glob.glob(r"C:\Users\surfqace\Desktop\ratous-gesture-recognition\SmileDetection\SmileDataset\*\*"):
    img=cv.imread(item)
    r_img=cv.resize(img,(32,32))
    data.append(r_img)
    label=item.split("\\")[-2]
    labels.append(label)


data= np.array(data)/255
le= LabelEncoder()
labels= le.fit_transform(labels)
labels= to_categorical(labels,2)


X_train,X_test,y_train,y_test= train_test_split(data,labels,test_size=0.3)
classTotals= labels.sum(axis=0)
classW= classTotals.max()/classTotals
classWeight={0:classW[0],1:classW[1]}

#Initialize CNN
CNN= models.Sequential([
    tf.keras.layers.Conv2D(32,(3,3),activation='relu',input_shape=(32,32,3),padding="same"),
    tf.keras.layers.BatchNormalization(),

    tf.keras.layers.Conv2D(32,(3,3),activation='relu',padding="same"),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.MaxPool2D((2,2)),

    tf.keras.layers.Conv2D(64,(3,3),activation='relu'),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.MaxPool2D((2,2)),
    
    tf.keras.layers.Flatten(),

    tf.keras.layers.Dense(120,activation='relu'),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dense(2,activation='softmax')
])

#Train CNN
CNN.compile(optimizer="sgd",loss="binary_crossentropy",metrics=["accuracy"])
Result= CNN.fit(X_train,y_train,epochs=100,batch_size=32,validation_data=(X_test,y_test),class_weight=classWeight)

#Show Train Results
plt.style.use("ggplot")
plt.plot(np.arange(100),Result.history["accuracy"],label="acc_train")
plt.plot(np.arange(100),Result.history["val_accuracy"],label="acc_test")
plt.plot(np.arange(100),Result.history["loss"],label="loss_train")
plt.plot(np.arange(100),Result.history["val_loss"],label="loss_test")
plt.title("Smile Detection")
plt.xlabel("epochs")
plt.ylabel("accuracy")
plt.legend()
plt.show()

#Save Trained CNN for use in detector
CNN.save("SmileDetection_CNN.h5")
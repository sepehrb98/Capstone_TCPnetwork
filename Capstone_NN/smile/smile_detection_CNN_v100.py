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
from tensorflow.keras.preprocessing.image import  ImageDataGenerator
from tensorflow.keras.utils import to_categorical
from tensorflow.keras.optimizers import Adam

#Initialize Dataset
aug= ImageDataGenerator(rotation_range=10,width_shift_range=0.1,height_shift_range=0.1,shear_range=0.1,zoom_range=0.2,horizontal_flip=True,fill_mode="nearest")
data=[]
labels=[]
for item in glob.glob(r" "):
    img=cv.imread(item)
    r_img=cv.resize(img,(64,64))
    data.append(r_img)
    label=item.split("\\")[-2]
    labels.append(label)


data= np.array(data)/255
le= LabelEncoder()
labels= le.fit_transform(labels)
labels= to_categorical(labels,2)


X_train,X_test,y_train,y_test= train_test_split(data,labels,test_size=0.4)
classTotals= labels.sum(axis=0)
classW= classTotals.max()/classTotals
classWeight={0:classW[0],1:classW[1]}

#Initialize CNN
CNN= models.Sequential([
    tf.keras.layers.Conv2D(64,(3,3),activation='relu',input_shape=(64,64,3),padding="same"),
    tf.keras.layers.BatchNormalization(),

    tf.keras.layers.Conv2D(64,(3,3),activation='relu',padding="same"),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.MaxPool2D((4,4)),

    tf.keras.layers.Conv2D(128,(3,3),activation='relu'),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.MaxPool2D((4,4)),
    
    tf.keras.layers.Flatten(),

    tf.keras.layers.Dense(240,activation='relu'),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dense(2,activation='softmax')
])

opt=Adam(lr=0.001,decay=0.001/200)

#Train CNN
CNN.compile(optimizer=opt,loss="binary_crossentropy",metrics=["accuracy"])
Result= CNN.fit_generator(aug.flow(X_train,y_train,batch_size=64),epochs=200,validation_data=(X_test,y_test),steps_per_epoch=len(X_train)//64,class_weight=classWeight)

#Show Train Results
plt.style.use("ggplot")
plt.plot(np.arange(200),Result.history["accuracy"],label="acc_train")
plt.plot(np.arange(200),Result.history["val_accuracy"],label="acc_test")
plt.plot(np.arange(200),Result.history["loss"],label="loss_train")
plt.plot(np.arange(200),Result.history["val_loss"],label="loss_test")
plt.title("Smile Detection CNN V1.0.1")
plt.xlabel("epochs")
plt.ylabel("accuracy")
plt.legend()
plt.show()

#Save Trained CNN for use in detector
CNN.save("smile_detection_CNN_V101.h5")

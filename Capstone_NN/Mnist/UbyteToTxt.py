import gzip
import matplotlib.pyplot as plt
f = gzip.open('/home/sepehr/Downloads/t10k-images-idx3-ubyte.gz','rb')
l= gzip.open('/home/sepehr/SeFar/Capstone project/t10k-labels-idx1-ubyte.gz','rb')

image_size = 28
num_images = 6

import numpy as np
f.read(16)
l.read(8)

image = np.zeros([29,28])
for k in range(10):
    image[0, :] = ord(l.read(1))
    for i in range(28):
        for j in range(28):
            image[i+1, j] = ord(f.read(1))
    print(image.shape)
    
    np.savetxt("img_%i.txt" % k, image, fmt='%d')


import cv2
import numpy as np
import matplotlib.pyplot as plt

filename = "sample_image.png"
y_range=[50,200]
u_range =[0,120]
v_range=[0,120]
image = cv2.imread(filename)
# print(image.shape) #[width, height]
image = cv2.resize(image, (int(image.shape[1]/ 4), int(image.shape[0]/4)))
# print(image.shape)

YUV = cv2.cvtColor(image, cv2.COLOR_BGR2YUV)
Filtered = np.zeros([YUV.shape[0], YUV.shape[1]])
for y in range(YUV.shape[0]):
    for x in range(YUV.shape[1]):
        if YUV[y,x,0] >= y_range[0] and YUV[y,x,0] <= y_range[1] and YUV[y,x,1]>=u_range[0] and YUV[y,x,1]<=u_range[1] and YUV[y,x,2]>=v_range[0] and YUV[y,x,2]>=v_range[1]:
            Filtered[y,x] = 1

plt.figure();
RGB = cv2.cvtColor(image, cv2.COLOR_BGR2RGB);
plt.imshow(RGB);
plt.title('Original image');

plt.figure()
plt.imshow(Filtered);
plt.title('Filtered image')

plt.show()
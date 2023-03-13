import cv2
from sklearn.cluster import KMeans,DBSCAN
import numpy as np
import matplotlib.pyplot as plt
np.set_printoptions(threshold=np.inf)
filename = "sample_image.png"

def RGB_2_YUV(R,G,B):
    Y = (12062107 *B + 886* (70100* G + 35707 *R))/105807109
    U = (4 * (3385827488 + 13225875 * B - 8762500 * G - 4463375 * R))/105807109
    V = -(4 *(-3385827488 + 2150875 *B + 11075000* G - 13225875* R))/105807109
    return Y,U,V
def YUV_2_RGB(Y,U,V):
    R = Y + 1.402 * (V - 128)
    G = Y - 0.34414 * (U - 128) - 0.71414 * (V - 128)
    B = Y + 1.772 * (U - 128)
    return R,G,B

#for green we need very low U and V
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


Movement =np.zeros([YUV.shape[0], YUV.shape[1]])
Max = 0.25
# Filtered=Filtered.transpose()

for i in range(0,len(Filtered)):
    percent = sum(Filtered[i])/len(Filtered[i])
    if percent<Max:
        Movement[i] = np.zeros(YUV.shape[1])
    else:
        Movement[i] = Filtered[i]

# Filtered = Filtered.transpose()
plt.figure()
RGB = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
plt.imshow(YUV)
plt.title('Original image')

plt.figure()
plt.imshow(Filtered)
plt.title('Filtered image')

cluster = KMeans(n_clusters=3)
X = YUV.reshape(-1,1)

kmeans = cluster.fit(X)
lab = kmeans.labels_
img_clusters = 128/(lab.max() -lab.min()) * lab
img_clusters = img_clusters.reshape(YUV.shape)
centers = cluster.cluster_centers_
print(centers)

plt.figure()
plt.imshow(img_clusters)
plt.title('K_Means cluster image')



plt.show()
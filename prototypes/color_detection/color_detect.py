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
def YUV_2_RGB(coord):
    Y = coord[0]
    U = coord[1]
    V = coord[2]
    R = Y + 1.402 * (V - 128)
    G = Y - 0.34414 * (U - 128) - 0.71414 * (V - 128)
    B = Y + 1.772 * (U - 128)
    out = [int(R),int(G),int(B)]
    return out

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
plt.imshow(RGB)
plt.title('Original image')

plt.figure()
plt.imshow(Filtered)
plt.title('Filtered image')


## Clustering
cluster = KMeans(n_clusters=6)
# cluster_db_scan = DBSCAN(eps=10,min_samples=100)
X=[]

for x in YUV:
    for y in x:
        X.append(y)
X = np.array(X)

kmeans = cluster.fit(X)
img_clusters = kmeans.labels_
centers = cluster.cluster_centers_
# img_clusters = 1/(lab.max() -lab.min()) * lab
# print(img_clusters)
# print(len(img_clusters),len(X))
# exit()
# img_clusters = img_clusters.reshape(YUV.shape)
for i in range(0,len(img_clusters)):
    X[i] = centers[img_clusters[i]]
X = X.reshape(YUV.shape)


## Colour change (not needed on drone)
X_colour = []

for x in X:
    for y in x:
        X_colour.append(YUV_2_RGB(y))
X_colour=np.array(X_colour)
X_colour = X_colour.reshape(X.shape)

for i in centers:
    print("cluster ", i, "RGB: ", YUV_2_RGB(i))

plt.figure()
plt.imshow(X_colour)
plt.title('K_Means cluster image')



plt.show()
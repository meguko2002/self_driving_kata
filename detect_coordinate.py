import cv2
import matplotlib.pyplot as plt

image = cv2.imread('./input/4777.png', 0)
plt.imshow(image, cmap = 'gray')
plt.show()
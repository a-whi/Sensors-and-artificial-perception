# Created by Alex Whitfield

import cv2
import numpy as np

# Set image dimensions
image_width = 2000
image_height = 2000

# Create a blank image
image = np.ones((image_height, image_width, 3), dtype=np.uint8) * 255

rows = 6
cols = 3
angles = range(0,90,5) # Creates and array from 0 to 90 with increments of 5
angle_index = 0 

# Function to draw the ellipses
def generate_ellipse(image, row, col, angle):
    x_center = (col + 0.5) * (image_width // cols)  # Calculate x-coordinate of the center
    y_center = (row + 0.5) * (image_height // rows)  # Calculate y-coordinate of the center
    size = (50, 125)  # Size of the ellipse
    cv2.ellipse(image, (int(x_center), int(y_center)), size, angle, 0, 360, (0, 0, 0), -1)  # Draw filled rotated ellipse


# Generate and draw ellipse
for i in range(rows):
    for j in range(cols):
        generate_ellipse(image, i, j, angles[angle_index])
        angle_index += 1

# Display the image
cv2.imshow("Shapes", image)
cv2.waitKey(0)
cv2.destroyAllWindows()

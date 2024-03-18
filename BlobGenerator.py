import cv2
import numpy as np
import random

# Set image dimensions
image_width = 800
image_height = 800

# Create a blank image
image = np.ones((image_height, image_width, 3), dtype=np.uint8) * 255

rows = 3
cols = 3
angles = [0, 0, 0, 5, 10, 15, 20, 30, 45]
angle_index = 0 

# Function to generate a random rectangle
def generate_rectangle(image, row, col, angle):
    x1 = col * (image_width // 3) + 50  # Calculate x-coordinate for top-left corner
    y1 = row * (image_height // 3) + 50  # Calculate y-coordinate for top-left corner
    x2 = x1 + 50  # Calculate x-coordinate for bottom-right corner
    y2 = y1 + 150  # Calculate y-coordinate for bottom-right corner
    color = (random.randint(0, 5), random.randint(0, 5), random.randint(0, 5))  # Random color
    center = ((x1 + x2) // 2, (y1 + y2) // 2)  # Center of the rectangle
    size = (50, 125)  # Size of the rectangle
    print('CUNT ', angle)
    cv2.ellipse(image, center, size, angle, 0, 360, color, -1)  # Draw filled rotated rectangle

# Generate and draw rectangles
for i in range(rows):
    for j in range(cols):
        print('PING: ', angle_index)
        generate_rectangle(image, i, j, angles[angle_index])
        angle_index += 1

# Display the image
cv2.imshow("Shapes", image)
cv2.waitKey(0)
cv2.destroyAllWindows()

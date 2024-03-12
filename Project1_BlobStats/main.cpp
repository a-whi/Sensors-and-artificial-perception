#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

/*
  g++ -o opencv_cam opencv_cam.cpp -std=c++17^
    -I "C:\msys64\mingw64\include\opencv4"^
    -L "C:\msys64\mingw64\bin"^
    -lopencv_core-409 -lopencv_highgui-409 -lopencv_imgcodecs-409^
    -lopencv_imgproc-409 -lopencv_videoio-409
*/
/*
Created by: Alexander Whitfield
Last updated: 06/03/24

*/

int main() {

    int totalPixelValue = 0;
    int pixelCounter = 0;

    cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    if (!cap.isOpened()) {
        std::cerr << "Error opening the camera!" << std::endl;
        return -1;
    }

    cv::Mat origFrame; // Original frame
    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame
    
    cap >> origFrame; // Do i need cap

    if (origFrame.empty()) {
        std::cerr << "No frame captured?" << std::endl;
        return -1;
    }

    // Loop through the image to make every pixel grey
    // It also takes the average of the grey scale pixels to work out the threshold
    for (int j = 0; j < origFrame.rows; j++){
        for (int i = 0; i < origFrame.cols; i++){
            // This gives RGB value of pixel
            cv::Vec3b pixel = origFrame.at<cv::Vec3b>(j, i);

            // Convert pixel to greyscale
            int greyValue = 0.2126 * pixel[2] + 0.7152 * pixel[1] + 0.0722 * pixel[0];

            // Set the grayscale value for the current pixel in the new image
            greyFrame.at<uchar>(j, i) = greyValue;

            totalPixelValue+= greyValue;
            pixelCounter++; 
        }
    }

    int threshold = std::round(totalPixelValue/pixelCounter); // Binary threshold value

    // Converting the greyscale image to black & white (binary)
    for (int j = 0; j < greyFrame.rows; j++){
        for (int i = 0; i < greyFrame.cols; i++){

            // Get the grayscale value for the current pixel
            uchar greyValue = greyFrame.at<uchar>(j, i);

            // Set the grayscale value for the current pixel in the new image
            if (greyValue < threshold){
                binaryFrame.at<uchar>(j, i) = 0;
            } else {
                binaryFrame.at<uchar>(j, i) = 255;
            }
        }
    }

    // Finding the number of components, size of objects and centroid
    cv::Mat labels, stats, centroid;
    int numLabels = cv::connectedComponentsWithStats(binaryFrame, labels, stats, centroid);

    // Get the centroid of the components
    for(int i = 0; i < numLabels; i++){
        double* centroid_ptr = centroid.ptr<double>(i);
        double centroid_x = centroid_ptr[0];
        double centroid_y = centroid_ptr[1];
        std::cout << "Pixel centroid is: " << centroid_x << ", " << centroid_y<< "\n";
    }

    // Drawing on the original image
    // Length of cross arms
    int crossArmLength = 20;

    // Calculate the endpoints of each arm
    cv::Point pt1_45(centroid_x + crossArmLength * cos(45 * CV_PI / 180), centroid_y + crossArmLength * sin(45 * CV_PI / 180));
    cv::Point pt2_45(centroid_x - crossArmLength * cos(45 * CV_PI / 180), centroid_y - crossArmLength * sin(45 * CV_PI / 180));

    cv::Point pt1_135(centroid_x + crossArmLength * cos(135 * CV_PI / 180), centroid_y + crossArmLength * sin(135 * CV_PI / 180));
    cv::Point pt2_135(centroid_x - crossArmLength * cos(135 * CV_PI / 180), centroid_y - crossArmLength * sin(135 * CV_PI / 180));

    // Draw the lines for each arm in red
    cv::line(origFrame, pt1_45, pt2_45, cv::Scalar(0, 0, 255), 2);
    cv::line(origFrame, pt1_135, pt2_135, cv::Scalar(0, 0, 255), 2);















    // Old code
    cv::imshow("Frame", origFrame);
/*
    // Prints RGB in terminal
    cv::Vec3b pixel = origFrame.at<cv::Vec3b>(j, i);
    std::cout << "Pixel values at (" << i << ", " << j << "): ";
    std::cout << "B: " << static_cast<int>(pixel[0]) << " ";
    std::cout << "G: " << static_cast<int>(pixel[1]) << " ";
    std::cout << "R: " << static_cast<int>(pixel[2]) << std::endl;
    cv::waitKey(0);
*/

    // Closes all windows
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
} // main()
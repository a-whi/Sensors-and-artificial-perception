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
//
    // cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    // if (!cap.isOpened()) {
    //     std::cerr << "Error opening the camera!" << std::endl;
    //     return -1;
    // }
//
    cv::Mat origFrame = cv::imread("/Users/alex/Downloads/photo1.jpg");

    if (origFrame.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::imshow("origFrame", origFrame);

    //cv::Mat origFrame; // Original frame
    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame

//    
    //cap >> origFrame; // Do i need cap

    // if (origFrame.empty()) {
    //     std::cerr << "No frame captured?" << std::endl;
    //     return -1;
    // }

    // cv::imshow("Original", origFrame);
//

    // Convert image to grey scale
    cv::cvtColor(origFrame, greyFrame, cv::COLOR_BGR2GRAY);

    // Converting grey scale to binary
    cv::threshold(greyFrame, binaryFrame, 0, 255, cv::THRESH_OTSU);
//Delete
    cv::imshow("Grey", greyFrame);
    cv::imshow("Binary", binaryFrame);
//
    // Finding the number of components, size of objects and centroid
    cv::Mat labels, stats, centroid;
    int numLabels = cv::connectedComponentsWithStats(binaryFrame, labels, stats, centroid, 4);
    std::cout << "hello" << std::endl;
    // Get the centroid of the components
    for(int i = 1; i < numLabels; i++){
        // double* centroid_ptr = centroid.ptr<double>(i);
        // double centroid_x = centroid_ptr[0];
        // double centroid_y = centroid_ptr[1];
        double centroid_x = centroid.at<double>(i, 0);
        double centroid_y = centroid.at<double>(i, 1);
        // Print out the center coordinates
        std::cout << "Centroid " << i << " is: " << centroid_x << ", " << centroid_y<< " ";

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
    }
    // Display the original frame with crosses
    cv::imshow("Image with Crosses", origFrame);

// //// For axis part
    // cv::Moments m = moments(binaryFrame, true);
    // cv::Point centroid(m.m10 / m.m00, m.m01 / m.m00);
//         int lineLength = 50;
//         cv::line(origFrame, centroid_x, centroid_x + lineLength, cv::Scalar(0, 255, 0), 2);
//         cv::line(origFrame, centroid_x, centroid_x - lineLength, cv::Scalar(0, 255, 0), 2);


    // Closes all windows
    // Wait for a key press indefinitely
    cv::waitKey(0);
 //   cap.release();
    cv::destroyAllWindows();
    
    return 0;
}

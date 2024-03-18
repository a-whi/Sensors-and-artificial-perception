#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
/*
Created by: Alexander Whitfield
Last updated: 06/03/24

*/
int momentsCal(cv::Mat binaryFrame){
    cv::Moments m = moments(binaryFrame, true);
    int m00 = 0, m11 = 0, m10 = 0, m01 = 0, m20 = 0, m02 = 0;
    for(int i = 0; i < binaryFrame.rows; i++){
        for (int j = 0; j < binaryFrame.cols; j++){
            if (binaryFrame.at<uchar>(i,j) != 0){
                m00 += 1;
                m11 += i * j;
                m10 += i; 
                m01 += j;
                m20 += i * i;
                m02 += j * j;
            }
        }
    }
    // Print out the moments
    std::cout << "m00: " << m00 << " m10: " << m10 << " m01: " << m01 << " m20: " << m20 << " m02: " << m02 << " m11: " << m11 << " \n";

    // Find orientation of the object using arc tan
    double numerator = 2 * ((m00 * m11) - (m10 * m01));
    double denominator = ((m00 * m20) - (m10 * m10)) - ((m00 * m02) - (m01 * m01));

    std::cout << "num: " << numerator << " denominator: " << denominator << "\n";

    //double orientation = 0.5 * atan(numerator/denominator);

    double orientation;
    if (numerator > 0 && denominator > 0) { // +
        orientation = 0.5 * (((180/CV_PI) * (atan2(numerator,denominator))));
        //print in q1
        std :: cout << "Q1" << std::endl;

    } else if (numerator > 0 && denominator < 0) { // + & -
        orientation = 0.5 * (((180/CV_PI) * (atan2(numerator,denominator)))) + 180;
        //print in q2
        std :: cout << "Q2" << std::endl;

    } else if (numerator < 0 && denominator < 0) { //  -
        orientation = 0.5 * (((180/CV_PI) * (atan2(numerator, denominator)))) - 90;
        //print in q3
        std :: cout << "Q3" << std::endl;

    } else { // numerator < 0 && denominator > 0 - & +
        orientation = 0.5 * (((180/CV_PI) * ( atan2(numerator,denominator)))) + 90;
        //print in q4
        std :: cout << "Q4" << std::endl;
    }

    // double orientation = 0.5 * atan2(numerator, denominator);
    return orientation;
}


int main() {

    // cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    // if (!cap.isOpened()) {
    //     std::cerr << "Error opening the camera!" << std::endl;
    //     return -1;
    // }

    cv::Mat origFrame = cv::imread("/Users/vinaypanicker/Desktop/fish.png");

    if (origFrame.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::imshow("origFrame", origFrame);

   //cv::Mat origFrame; // Original frame
    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame

   
    // cap >> origFrame; // Do i need cap
    // if (origFrame.empty()) {
    //     std::cerr << "No frame captured?" << std::endl;
    //     return -1;
    // }

    // cv::imshow("Original", origFrame);


    // Convert image to grey scale
    cv::cvtColor(origFrame, greyFrame, cv::COLOR_BGR2GRAY);

    // Converting grey scale to binary
    cv::threshold(greyFrame, binaryFrame, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cv::bitwise_not(binaryFrame, binaryFrame);

    cv::imshow("biFrame", binaryFrame);

    // Finding the number of components, size of objects and centroid
    cv::Mat labels, stats, centroid;
    int numLabels = cv::connectedComponentsWithStats(binaryFrame, labels, stats, centroid, 8);

    // std::vector<std::vector<cv::Point>> contours;
    // cv::findContours(binaryFrame, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    
    // Get the centroid of the components
    for(int i = 1; i < numLabels; i++){
        // double* centroid_ptr = centroid.ptr<double>(i);
        // double centroid_x = centroid_ptr[0];
        // double centroid_y = centroid_ptr[1];
        double centroid_x = centroid.at<double>(i, 0);
        double centroid_y = centroid.at<double>(i, 1);
        // Print out the center coordinates
        std::cout << "Centroid " << i << " is: " << centroid_x << ", " << centroid_y<< " \n";


        // Extract the binary image for the current component
        cv::Mat componentBinary = (labels == i);

        double orientation =  momentsCal(componentBinary);
        // Print out the orientation
        std::cout << "Orientation " << i << " is: " << orientation << " \n";

        // Draw the line of the orientation
        int lineLength = 100;
        cv::line(origFrame, cv::Point(centroid_x, centroid_y), cv::Point(centroid_x + lineLength * cos(orientation*CV_PI / 180), centroid_y + lineLength * sin(orientation*CV_PI / 180)), cv::Scalar(0, 255, 0), 4);
        cv::line(origFrame, cv::Point(centroid_x, centroid_y), cv::Point(centroid_x - lineLength * cos(orientation*CV_PI / 180), centroid_y - lineLength * sin(orientation*CV_PI / 180)), cv::Scalar(0, 255, 0), 4);

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

    // Closes all windows
    // Wait for a key press indefinitely
    cv::waitKey(0);
 //   cap.release();
    cv::destroyAllWindows();
    
    return 0;
}

/*
old moments stuff
        // Extract the binary image for the current component
        cv::Mat componentBinary = (labels == i);

        // Calculate moments for the current component
        // cv::Moments m = moments(componentBinary, true);
        //cv::Moments m = cv::moments(contours[i]);

        // double m11 = m.m10*m.m01;
        // double m20 = m.m10*m.m10;
        // double m02 = m.m01*m.m01;

        // i = m10, j = m01
        // m11 = m10*m01
        // m20 = m10*m10
        // m02 = m01*m01

        // Print out the moments
        std::cout << "m00: " << m.m00 << " m10: " << m.m10 << " m01: " << m.m01 << " m20: " << m20 << " m02: " << m02 << " m11: " << m11 << " \n";

        // Find orientation of the object using arc tan
        double numerator = 2 * ((m.m00 * m11) - (m.m10 * m.m01));
        double denominator = ((m.m00 * m20) - (m.m10 * m.m10)) - ((m.m00 * m02) - (m.m01 * m.m01));

        std::cout << "num: " << numerator << " denominator: " << denominator << "\n";

        //double orientation = 0.5 * atan(numerator/denominator);

        double orientation;
        if (numerator > 0 && denominator > 0) { // +
            orientation = 0.5 * (((180/CV_PI) * (atan(numerator / denominator))));
        } else if (numerator > 0 && denominator < 0) { // + & -
            orientation = 0.5 * (((180/CV_PI) * (atan(numerator / denominator))) - 270);
        } else if (numerator < 0 && denominator < 0) { //  -
            orientation = 0.5 * (((180/CV_PI) * (atan(numerator / denominator))) - 180);
        } else { // numerator < 0 && denominator > 0 - & +
            orientation = 0.5 * (((180/CV_PI) * ( atan(numerator / denominator))) - 90);
        }
        
        // Print out the orientation
        std::cout << "Orientation " << i << " is: " << orientation << " \n";*/

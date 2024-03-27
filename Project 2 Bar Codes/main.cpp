/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 1: Calculating Blob Statistics   
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <tuple>

int main() {

    cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    if (!cap.isOpened()) {
        std::cerr << "Error opening the camera!" << std::endl;
        return -1;
    } 

    cv::Mat origFrame;  
    cv::Mat origFrame2;
    //cap >> origFrame;

    // Variables for the box around the barcode
    vector<Point> corners;
    vector<string> decode_info;
    vector<string> decode_type;

    // Loop to keep camera on till button is pressed and image is captured
    while (true) {
        cap >> origFrame; // Capture frame from camera

        // Check if the frame is empty
        if (origFrame.empty()) {
            std::cerr << "No frame captured?" << std::endl;
            break;
        }

        cv::imshow("Camera", origFrame);

        // Wait for keypress (100 ms delay)
        int key = cv::waitKey(100);

        // Check if the user pressed any key to capture the photo
        if (key != -1) {
            // Display the captured photo
            cv::imshow("Captured Photo", origFrame);
            break; // Exit the loop after capturing the photo
        }
        // Check if the user pressed the ESC key to exit
        else if (key == 27) {
            break; // Exit the loop if ESC is pressed
        }
    }

    cap >> origFrame2;

    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame

    // Converting grey scale to binary
    cv::threshold(greyFrame, binaryFrame, 100, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);



    cv::barcode::BarcodeDetector bardet;
    cv::Mat img = cv::imread("barcode_image.jpg");
    
    // Detect and decode barcodes
    bool success;
    std::string decoded_info;
    int decoded_type;
    std::tie(success, decoded_info, decoded_type) = bardet.detect(img);











    // Display the original frame
    cv::imshow("Image with Crosses", origFrame2);

    // Closes all windows
    // Wait for a key press indefinitely
    cv::waitKey(0);
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}

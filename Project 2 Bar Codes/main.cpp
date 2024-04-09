/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 2: Decoding an EAN-13 Barcode 
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <tuple>

int main() {

    
    return 0;
}



// This function crops the captured frame so its just the barcode
cv::Mat crop_rect(cv::RotatedRect rect, std::vector<cv::Point2f> box, cv::Mat img) {
    float W = rect.size.width;
    float H = rect.size.height;

    std::vector<float> Xs;
    std::vector<float> Ys;

    // Thix takes the coordinates of the corners of the box and stores it in Xs and Ys
    for (auto point : box) {
        Xs.push_back(point.x);
        Ys.push_back(point.y);
    }


    float x1 = *std::min_element(Xs.begin(), Xs.end());
    float x2 = *std::max_element(Xs.begin(), Xs.end());
    float y1 = *std::min_element(Ys.begin(), Ys.end());
    float y2 = *std::max_element(Ys.begin(), Ys.end());


    // Finds the center point of the box
    cv::Point2f center = cv::Point2f((x1 + x2) / 2, (y1 + y2) / 2);
    cv::Size2f size = cv::Size2f(x2 - x1, y2 - y1);
    // Crops the barcode given the box size
    cv::Mat cropped;
    cv::getRectSubPix(img, size, center, cropped);

    // Rotates the image so the barcode is in correct orientation
    float angle = rect.angle;
    if (angle != 90) { // need rotation
        if (angle > 45) {
            angle = 0 - (90 - angle);
        }
        cv::Mat M = cv::getRotationMatrix2D(cv::Point2f(size.width / 2, size.height / 2), angle, 1.0);
        cv::warpAffine(cropped, cropped, M, size);

        // Determine the width and height of the final rotated region   
        float croppedW = (H > W) ? H : W; // If H is greater than W, use H, otherwise use W
        float croppedH = (H < W) ? H : W;

        //Final rotated region from the rotated cropped image
        cv::getRectSubPix(cropped, cv::Size2f(croppedW, croppedH), cv::Point2f(size.width / 2, size.height / 2), cropped);
        return cropped;
    }
    return cropped;
}

/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 2: Decoding an EAN-13 Barcode 
*/

#include <opencv2/opencv.hpp>
#include <iostream>
//////////////////////////////////////////////////////

int getAT_Value(double value){
    if (value < 2.5/7){
        return 2;
    }elif (value < 3.5/7){
        return 3;
    }elif (value < 4.5/7){
        return 4;
    }else{
        return 5;
    }
}





void convert_patterns_to_length(std::vector<int>& pattern) {
    for (int i = 0; i <= pattern.size(); i++){
        patterns[i] = patterns[i].size();
    }
}

std::tuple<std::string, bool> decode_line(const cv::Mat& line) {
    std::vector<Code> bars = read_bars(line);

    auto[leftGuard, leftBarcode, centerGaurd, rightBarcode, rightGuard] = classify_bars(bars);

    convert_patterns_to_length(leftBarcode);
    convert_patterns_to_length(rightBarcode);
    auto[left_codes] = read_patterns(leftBarcode, true);
    auto[right_codes] = read_patterns(rightBarcode, false);

    // Get EAN-13 from left and right codes
    std::string ean13 = get_ean13(left_codes, right_codes);

    bool is_valid = verify(ean13);

    return std::make_tuple(ean13, is_valid);
}

std::tuple<std::string, bool, cv::Mat> decode(const cv::Mat& img) {
    cv::Mat grey;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::Mat thresh;

    cv::threshold(greyFrame, binaryFrame, 200, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    cv::bitwise_not(thresh, thresh);

    std::string ean13;
    bool is_valid = false;

    for (int i = img.rows - 1; i >= 0; --i) {
        std::tie(ean13, is_valid, std::ignore) = decode_line(thresh.row(i));
        if (is_valid) {
        break;
        }
    }
    return std::make_tuple(ean13, is_valid, thresh);
}


//////////////////////////////////////////////////

cv::Mat crop_rect(cv::RotatedRect rect, cv::Point2f box[], cv::Mat img) {
    float W = rect.size.width;
    float H = rect.size.height;
    std::vector<float> Xs, Ys;
    for (int i = 0; i < 4; ++i) {
        Xs.push_back(box[i].x);
        Ys.push_back(box[i].y);
    }
    float x1 = *std::min_element(Xs.begin(), Xs.end());
    float x2 = *std::max_element(Xs.begin(), Xs.end());
    float y1 = *std::min_element(Ys.begin(), Ys.end());
    float y2 = *std::max_element(Ys.begin(), Ys.end());

    // Center of rectangle in source image
    cv::Point2f center = cv::Point2f((x1 + x2) / 2, (y1 + y2) / 2);
    // Size of the upright rectangle bounding the rotated rectangle
    cv::Size2f size = cv::Size2f(x2 - x1, y2 - y1);
    // Cropped upright rectangle
    cv::Mat cropped;
    cv::getRectSubPix(img, size, center, cropped);

    float angle = rect.angle;
    if (angle != 90) { // need rotation
        if (angle > 45)
            angle = 0 - (90 - angle);
        // Rotation matrix
        cv::Mat M = cv::getRotationMatrix2D(cv::Point2f(size.width / 2, size.height / 2), angle, 1.0);
        cv::warpAffine(cropped, cropped, M, size);

        float croppedW = H > W ? H : W;
        float croppedH = H < W ? H : W;
        // Final cropped & rotated rectangle
        cv::getRectSubPix(cropped, cv::Size2f(croppedW, croppedH), cv::Point2f(size.width / 2, size.height / 2), cropped);
    }
    return cropped;
}

std::map<std::string, cv::Mat> detect_barcode(int camera_index = 0) {
    // Initialize video capture device
    cv::VideoCapture cap(camera_index);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera" << std::endl;
        return {};
    }

    bool film = true;

    while (film) {
        // Capture frame-by-frame
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cout << "Failed to grab frame" << std::endl;
            break;
        }

        // Resize the frame
        cv::resize(frame, frame, cv::Size(), 0.7, 0.7, cv::INTER_CUBIC);

        // Convert frame to grayscale
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Calculate x & y gradient
        cv::Mat gradX, gradY;
        cv::Sobel(gray, gradX, CV_32F, 1, 0, -1);
        cv::Sobel(gray, gradY, CV_32F, 0, 1, -1);

        // Subtract the y-gradient from the x-gradient
        cv::Mat gradient;
        cv::subtract(gradX, gradY, gradient);
        cv::convertScaleAbs(gradient, gradient);

        // Blur the image
        cv::Mat blurred;
        cv::blur(gradient, blurred, cv::Size(3, 3));

        // Threshold the image
        cv::Mat thresh;
        cv::threshold(blurred, thresh, 225, 255, cv::THRESH_BINARY);

        // Construct a closing kernel and apply it to the thresholded image
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 7));
        cv::Mat closed;
        cv::morphologyEx(thresh, closed, cv::MORPH_CLOSE, kernel);

        // Perform a series of erosions and dilations
        cv::erode(closed, closed, cv::Mat(), cv::Point(-1, -1), 4);
        cv::dilate(closed, closed, cv::Mat(), cv::Point(-1, -1), 4);

        // Find the contours in the thresholded image, then sort the contours
        // by their area, keeping only the largest one
        std::vector<std::vector<cv::Point>> cnts;
        cv::findContours(closed.clone(), cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if (!cnts.empty()) {
            std::vector<cv::Point> c = *std::max_element(cnts.begin(), cnts.end(),
                [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                    return cv::contourArea(a) < cv::contourArea(b);
                });

            // Compute the rotated bounding box of the largest contour
            cv::RotatedRect rect = cv::minAreaRect(c);
            cv::Point2f box[4];
            rect.points(box);
            cv::Mat cropped = crop_rect(rect, box, frame);

            // Draw a bounding box around the detected barcode
            cv::drawContours(frame, std::vector<std::vector<cv::Point>>({std::vector<cv::Point>(box, box + 4)}), -1, cv::Scalar(0, 255, 0), 3);

            // Save the cropped region as an image
            cv::imwrite("cropped.jpg", cropped);

            // Plot the centroid of the barcode
            cv::Moments M = cv::moments(c, true);
            cv::Point centroid(M.m10 / M.m00, M.m01 / M.m00);
            cv::circle(frame, centroid, 5, cv::Scalar(0, 0, 255), -1);
        }

        // Display the resulting frame
        cv::imshow("Barcode Detection", frame);

        // Break the loop when 'q' is pressed
        if (cv::waitKey(1) == 'q')
            break;
    }

    // Release the video capture device and close all windows
    cap.release();
    cv::destroyAllWindows();
    return {};
}

int main() {
    detect_barcode();
    return 0;
}

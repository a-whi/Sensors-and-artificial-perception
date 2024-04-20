/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 2: Decoding an EAN-13 Barcode 
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

// Function Definitions
std::vector<int> getSegmentLengths(const std::vector<int>& arr) {
    /**
     * Calculates the lengths of the black and white segments for entire barcode.
     *
     * @param arr Horizontal Line across the barcode's pixel values as 0 or 1.
     * @return Vector of black and white segment lengths.
     */
    
    std::vector<int> segmentLengths;
    int count = 1; 

    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] == arr[i - 1]) {
            ++count;
        } else {
            segmentLengths.push_back(count);
            count = 1; 
        }
    }
    segmentLengths.erase(segmentLengths.begin());

    return segmentLengths;
}

int calcAt(double ti_t) {
     /**
     * Calculates the At value based on the given ti/t ratio value.
     *
     * @param ti_t Ratio value based on ti length divided by total length
     * @return Integer At value 
     */
    if (1.5/7 <= ti_t && ti_t <= 2.5/7) {
        return 2;
    } else if (2.5/7 <= ti_t && ti_t <= 3.5/7) {
        return 3;
    } else if (3.5/7 <= ti_t && ti_t <= 4.5/7) {
        return 4;
    } else if (4.5/7 <= ti_t && ti_t <= 5.5/7) {
        return 5;
    } else {
        return 1;
    }
}

std::tuple<std::string, std::string> getDecodedValue(const int& t1, const int& t2, const int& t4) {
    /**
     * Calculates parity and digit value based on the given ti values. Based on lecture slides.
     *
     * At Values found based on segment ratios
     * @param t1
     * @param t2
     * @param t4 
     * 
     * @return Parity and Digit based on decoding table. Slide 37
     */
    static const std::map<std::tuple<int, int, int>, std::tuple<std::string, std::string>> valueMap = {
        // Prioritize checks with t4
        {{3, 3, 2}, {"E", "2"}},
        {{3, 3, 3}, {"E", "8"}},
        {{3, 4, 2}, {"O", "1"}},
        {{3, 4, 1}, {"O", "7"}},
        {{4, 3, 2}, {"O", "2"}},
        {{4, 3, 1}, {"O", "8"}},
        {{4, 4, 1}, {"E", "1"}},
        {{4, 4, 2}, {"E", "7"}},
        // Other checks
        {{2, 2, 0}, {"E", "6"}},
        {{2, 3, 0}, {"O", "0"}},
        {{2, 4, 0}, {"E", "4"}},
        {{2, 5, 0}, {"O", "3"}},
        {{3, 2, 0}, {"O", "9"}},
        {{3, 5, 0}, {"E", "5"}},
        {{4, 2, 0}, {"E", "9"}},
        {{4, 5, 0}, {"O", "5"}},
        {{5, 2, 0}, {"O", "6"}},
        {{5, 3, 0}, {"E", "0"}},
        {{5, 4, 0}, {"O", "4"}},
        {{5, 5, 0}, {"E", "3"}},
    };

    auto it = valueMap.find({t1, t2, t4});
    if (it != valueMap.end()) {
        return it->second;
    }

    auto it_t4_zero = valueMap.find({t1, t2, 0});
    if (it_t4_zero != valueMap.end()) {
        return it_t4_zero->second;
    }

    return {"", ""};
}

void processSegmentLengths(const std::vector<int>& data_lengths, std::string& currentParity, std::string& currentDigit) {
    /**
     * Processes the segment lengths to get the parity and digit values. Uses above Helper Functions
     *
     * @param data_lengths Vector of segment lengths
     * @param currentParity Current parity value
     * @param currentDigit Current digit value
     */
    for (size_t i = 0; i < data_lengths.size(); ++i) {
        if (i % 4 == 3) {
            double t = data_lengths[i] + data_lengths[i-1] + data_lengths[i-2] + data_lengths[i-3]; // Total length
            double t1_t = (data_lengths[i] + data_lengths[i-1]) / t;
            double t2_t = (data_lengths[i-1] + data_lengths[i-2]) / t;
            double t4_t = (data_lengths[i-3]) / t;

            int at1 = calcAt(t1_t);
            int at2 = calcAt(t2_t);
            int at4 = calcAt(t4_t);

            // Debugging
            // std::cout << "at1: " << at1 << " at2: " << at2 << " at4: " << at4 << std::endl;

            std::string parity, digit;
            std::tie(parity, digit) = getDecodedValue(at1, at2, at4);

            currentParity += parity;
            currentDigit += digit;
        }
    }
}

std:: string getBarcode(std::string parity_l, std::string parity_r, std::string digit_l, std::string digit_r){
    /**
     * Merges the left and right parity and digit values to get the final barcode.
     *
     * @param parity_l Left parity value
     * @param parity_r Right parity value
     * @param digit_l Left digit value
     * @param digit_r Right digit value
     * @return Final barcode
     */
    if (parity_l == "EEEEEE"){
        std::reverse(parity_r.begin(), parity_r.end());
        std::reverse(digit_r.begin(), digit_r.end());
        std::reverse(digit_l.begin(), digit_l.end());

        std::swap(parity_l, parity_r);
        std::swap(digit_l, digit_r);

        for (char& c : parity_l) {
            c = (c == 'O') ? 'E' : 'O';
        }
    }  

    std::unordered_map<std::string, std::string> parityMap = {
        {"OOOOOO", "0"},
        {"OOEOEE", "1"},
        {"OOEEOE", "2"},
        {"OOEEEO", "3"},
        {"OEOOEE", "4"},
        {"OEEOOE", "5"},
        {"OEEEOO", "6"},
        {"OEOEOE", "7"},
        {"OEOEEO", "8"},
        {"OEEOEO", "9"}
    };

    std::string barcode= parityMap[parity_l] + digit_l + digit_r;

    return barcode;
}

int getChecksum(const std::string& barcode){
    /**
     * Takes the barcode calculates and verifies the check sum.
     * 
     * @param barcode Barcode to calculate checksum for
     * @return barcode if it is correct 
     * @return "" if the barcode is incorrect
    */

    int odd = 0;
    int even = 0;

    //Convert barcode string to an array of integers
    std::vector<int> barcode_int;
    for (char c : barcode){
        barcode_int.push_back(c - '0');
    }

    for (int i = 0; i<=11; i++){
        if(i%2 != 0){
            odd += barcode_int[i]*3;
        }
        else{
            even += barcode_int[i];
        }
    }

    int sum = (odd + even)%10;

    if (sum != 0){
        // If sum is not 0, subtract it from 10
        sum = 10 - sum;
    }

    // Print out the correct check sum only if it is correct 
    if (sum == barcode_int[12]){
        std::cout << "Correct Check Sum: " << sum << std::endl;
    }

    // Used for SECTION 2 DATA
    // std::cout << "Barcode:" << barcode << std::endl;
    
    return (sum == barcode_int[12]) ? 1 : 0;
}


std::string getDecodedBarcode(cv::Mat frame) {
    /**
     * Decodes the barcode from the given frame.
     *
     * @param frame Image Frame of the barcode captured from the camera and cropped.
     * @return Decoded barcode if valid
     * @return "" if the barcode is invalid
    */

    cv::Mat gray;
    cv::Mat thresh;

    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, thresh, 200, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

    // Invert the image and get the middle row of the image to pass to getSegmentLengths
    thresh = ~thresh;
    cv::Mat line = thresh.row(static_cast<int>(frame.rows / 2));

    // Get the segment lengths takes the middle row of the image
    std::vector<int> segmentLengths = getSegmentLengths(line);

    std::vector<int> l_data_lengths(segmentLengths.begin() + 3, segmentLengths.begin() + 27);
    std::vector<int> r_data_lengths(segmentLengths.begin() + 32, segmentLengths.begin() + 56);

    // Process left and right data lengths
    std::string currentParity_l = "";
    std::string currentDigit_l = "";
    processSegmentLengths(l_data_lengths, currentParity_l, currentDigit_l);

    std::string currentParity_r = "";
    std::string currentDigit_r = "";
    processSegmentLengths(r_data_lengths, currentParity_r, currentDigit_r);
    
    // Merge left and right data lengths based on parity
    std::string barcode = getBarcode(currentParity_l, currentParity_r, currentDigit_l, currentDigit_r);
    int correct = getChecksum(barcode);
    
    if (correct == 1){
        // Only return the barcode if the check sum is correct
        return barcode;
    }

    // Return empty string if the check sum is incorrect
    return "";
}


cv::Mat getRotateCrop(cv::RotatedRect rect, cv::Point2f box[], cv::Mat frame) {
    /**
     * Rotates and crops the image based on the bounding rectangle.
     *
     * @param rect Rotated rectangle of the barcode
     * @param box Bounding box of the barcode
     * @param frame Image Frame of the barcode captured from the camera
     * @return Cropped image of the barcode
     */

    // Extract coordinates
    std::vector<float> Xs, Ys;
    for (int i = 0; i < 4; ++i) {
        Xs.push_back(box[i].x);
        Ys.push_back(box[i].y);
    }
    float x1 = *std::min_element(Xs.begin(), Xs.end()), x2 = *std::max_element(Xs.begin(), Xs.end());
    float y1 = *std::min_element(Ys.begin(), Ys.end()), y2 = *std::max_element(Ys.begin(), Ys.end());

    // Center & size of bounding rectangle
    cv::Point2f center = cv::Point2f((x1 + x2) / 2, (y1 + y2) / 2);
    cv::Size2f size = cv::Size2f(x2 - x1, y2 - y1);

    // Crop the image based on the bounding rectangle passed
    cv::Mat cropped;
    cv::getRectSubPix(frame, size, center, cropped);

    // Rotate 
    float angle = rect.angle;
    if (angle != 90) {
        angle = angle > 45 ? 0 - (90 - angle) : angle;
        cv::Mat M = cv::getRotationMatrix2D(cv::Point2f(size.width / 2, size.height / 2), angle, 1.0);
        cv::warpAffine(cropped, cropped, M, size);
        float W = std::max(rect.size.height, rect.size.width);
        float H = std::min(rect.size.height, rect.size.width);
        cv::getRectSubPix(cropped, cv::Size2f(W, H), cv::Point2f(size.width / 2, size.height / 2), cropped);
    }
    return cropped;
}

std::map<std::string, cv::Mat> detectBarcode(int camera_index = 1) {
    /**
     * Detects the barcode from the camera feed, using helper functions.
     *
     * @param camera_index Index of the camera device
     * @return {}
     */

    // Initialize video capture device
    cv::VideoCapture cap(camera_index);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera" << std::endl;
        return {};
    }

    bool film = true;
    cv::Mat Displayframe;

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
            cv::Mat cropped = getRotateCrop(rect, box, frame);

            // Draw a bounding box around the detected barcode
            cv::drawContours(frame, std::vector<std::vector<cv::Point>>({std::vector<cv::Point>(box, box + 4)}), -1, cv::Scalar(0, 255, 0), 3);
        
            // Break the loop when 'q' is pressed
            if (cv::waitKey(1) == 'q'){
                std::string result = getDecodedBarcode(cropped);
                if (result != "") {

                    // Print out the barcode (check sum gets printed in getChecksum function)
                    std::cout << "Barcode: " << result << std::endl;

                    // Print out current time and date used for SECTION 2 DATA
                    time_t now = time(0);
                    char* dt = ctime(&now);
                    std::cout << "Date and Time: " << dt << std::endl;
                    
                    // Save the cropped region as an image used for SECTION 2 DATA
                    cv::imwrite("cropped.jpg", cropped);
                    break;
                }
                // If the barcode is not detected print out an error message
                std::cout << "Barcode incorrectly detected" << std::endl;

                // Save the cropped region as an image used for SECTION 2 DATA
                cv::imwrite("cropped.jpg", cropped);
                break;
            }

            // Plot the centroid of the barcode
            cv::Moments M = cv::moments(c, true);
            cv::Point centroid(M.m10 / M.m00, M.m01 / M.m00);
            cv::circle(frame, centroid, 5, cv::Scalar(0, 0, 255), -1);
        }

        // Display the resulting frame
        cv::imshow("Barcode Detection", frame);

    }

    // Release the video capture device and close all windows
    cap.release();
    cv::destroyAllWindows();
    return {};
}


int main() {
    /*
    Main function to run the barcode detection.
    */
    detectBarcode();
    return 0;
}

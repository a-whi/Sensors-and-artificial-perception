/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 2: Decoding an EAN-13 Barcode 
*/

#include <opencv2/opencv.hpp>
#include <iostream>

#include <opencv2/opencv.hpp>
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
     * Calculates parity and digit value based on the given ti values.
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
            double t = data_lengths[i] + data_lengths[i-1] + data_lengths[i-2] + data_lengths[i-3];
            double t1_t = (data_lengths[i] + data_lengths[i-1]) / t;
            double t2_t = (data_lengths[i-1] + data_lengths[i-2]) / t;
            double t4_t = (data_lengths[i-3]) / t;

            int at1 = calcAt(t1_t);
            int at2 = calcAt(t2_t);
            int at4 = calcAt(t4_t);

            //std::cout << "at1: " << at1 << " at2: " << at2 << " at4: " << at4 << std::endl;

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

    //std::cout << "PARTIY_L: " << parity_l << std::endl;

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

int verify(const std::string& barcode){

    int odd = 0;
    int even = 0;

    //convert barcode to an array of integers
    std::vector<int> barcode_int;
    for (char c : barcode){
        barcode_int.push_back(c - '0');
    }


    for (int i = 0; i<=11; i++){
        //std::cout << "Barcode[" << i << "]: " << barcode_int[i] << std::endl;
        if(i%2 != 0){
            odd += barcode_int[i]*3;
        }
        else{
            even += barcode_int[i];
        }
    }
    int sum = (odd + even)%10;
    sum = 10 - sum;

    // std::cout << "Barcode[12]: " << barcode_int[12] << std::endl;
    // std::cout << "Sum: " << sum << std::endl;

    return (sum == barcode_int[12]) ? 1 : 0;
}


std::string decode(cv::Mat img) {
    cv::Mat gray;
    cv::Mat thresh;

    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, thresh, 200, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

    thresh = ~thresh;
    cv::Mat line = thresh.row(static_cast<int>(img.rows / 2));
    line.setTo(1, line == 255);

    std::vector<int> segmentLengths = getSegmentLengths(line);

    // Split segmentLengths vector
    std::vector<int> l_guard_lengths(segmentLengths.begin(), segmentLengths.begin() + 3);
    std::vector<int> l_data_lengths(segmentLengths.begin() + 3, segmentLengths.begin() + 27);
    std::vector<int> m_guard_lengths(segmentLengths.begin() + 27, segmentLengths.begin() + 32);
    std::vector<int> r_data_lengths(segmentLengths.begin() + 32, segmentLengths.begin() + 56);
    std::vector<int> r_guard_lengths(segmentLengths.begin() + 56, segmentLengths.begin() + 59);

    // Process left and right data lengths
    std::string currentParity_l = "";
    std::string currentDigit_l = "";
    processSegmentLengths(l_data_lengths, currentParity_l, currentDigit_l);

    std::string currentParity_r = "";
    std::string currentDigit_r = "";
    processSegmentLengths(r_data_lengths, currentParity_r, currentDigit_r);
    
    // Merge left and right data lengths based on parity
    std::string barcode = getBarcode(currentParity_l, currentParity_r, currentDigit_l, currentDigit_r);
    int correct = verify(barcode);

    // std::cout << "Barcode: " << barcode << std::endl;
    // std::cout << "Is Valid? " << correct << std::endl;
    if (correct == 1){
        return barcode;
    }
    return "";
}


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

std::map<std::string, cv::Mat> detect_barcode(int camera_index = 1) {
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
            cv::Mat cropped = crop_rect(rect, box, frame);


            // Draw a bounding box around the detected barcode
            cv::drawContours(frame, std::vector<std::vector<cv::Point>>({std::vector<cv::Point>(box, box + 4)}), -1, cv::Scalar(0, 255, 0), 3);
//
//not needed
//             // Save the cropped region as an imag
//             
            if (cv::waitKey(1) == 'q'){
                std::string result = decode(cropped);
                if (result != "") {
                    //cv::putText(frame, "Barcode: " + result, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                    std::cout << "Barcode: " << result << std::endl;
                    break;
                }
                cv::imwrite("cropped.jpg", cropped);
                break;
            }
// //
            // Plot the centroid of the barcode
            cv::Moments M = cv::moments(c, true);
            cv::Point centroid(M.m10 / M.m00, M.m01 / M.m00);
            cv::circle(frame, centroid, 5, cv::Scalar(0, 0, 255), -1);
        }

        // Display the resulting frame
        cv::imshow("Barcode Detection", frame);

        // Break the loop when 'q' is pressed
        
     
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

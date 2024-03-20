/*
Created by: Alex Whitfield and Vinay Panicker
TRC 3500 Project 1: Calculating Blob Statistics   
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <tuple>


// Function used for grouping the blobs by letter
char getGroupLetter(int groupNumber) {
    return static_cast<char>('A' + groupNumber);
}

int main() {

    // Camera Capture
    cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    if (!cap.isOpened()) {
        std::cerr << "Error opening the camera!" << std::endl;
        return -1;
    }

//Delete    //cv::Mat origFrame = cv::imread("/Users/alex/Downloads/nice.png");
    cv::Mat origFrame;

    if (origFrame.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame
    cv::Mat hsvFrame; // Creating a HSV frame

    // Apply Gaussian blur to the original image
    cv::GaussianBlur(origFrame, origFrame, cv::Size(101, 101),0);
    cv::cvtColor(origFrame, greyFrame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(origFrame, hsvFrame, cv::COLOR_BGR2HSV);

    // Converting grey scale to binary
    cv::threshold(greyFrame, binaryFrame, 100, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(29, 29));
    cv::morphologyEx(binaryFrame, binaryFrame, cv::MORPH_OPEN, kernel);

    // Finding the number of components, size of objects and centroid
    // This is uesd later for drawing boxes around the blobs and grouping them
    cv::Mat labels, stats, centroid;
    int numLabels = cv::connectedComponentsWithStats(binaryFrame, labels, stats, centroid, 8);
    
    // creat empty array to store component data
    std::vector<std::tuple<int, double, int>> componentData;

    // Define an array to store x and y components for labeling groups
    std::vector<std::pair<int, int>> xyComponents;
    
    // Loop to get centroids, axis of minimum moment of inertia and draw on the original image
    for(int i = 1; i < numLabels; i++){

        // Extract the binary image for the current component
        cv::Mat componentBinary = (labels == i);
        cv::Moments m = moments(componentBinary, true);

        // Get centroid via moments
        double centroid_xm = m.m10/m.m00;
        double centroid_ym = m.m01/m.m00;

        // Calculate axis of minimum moment of inertia
        double orientation = 0.5 * atan2(2 * m.mu11, m.mu20-m.mu02);

        // Print out the center coordinates
        std::cout << "Centroid_M " << i << " is: " << centroid_xm << ", " << centroid_ym<< " \n";
        // Print out the orientation
        std::cout << "Orientation " << i << " is: " << orientation * 180/CV_PI << " \n";

        // Draw red crosses at blob centroid
        int crossArmLength = 20;
        cv::Point pt1_45(centroid_xm + crossArmLength * cos(45 * CV_PI / 180), centroid_ym + crossArmLength * sin(45 * CV_PI / 180));
        cv::Point pt2_45(centroid_xm - crossArmLength * cos(45 * CV_PI / 180), centroid_ym - crossArmLength * sin(45 * CV_PI / 180));

        cv::Point pt1_135(centroid_xm + crossArmLength * cos(135 * CV_PI / 180), centroid_ym + crossArmLength * sin(135 * CV_PI / 180));
        cv::Point pt2_135(centroid_xm - crossArmLength * cos(135 * CV_PI / 180), centroid_ym - crossArmLength * sin(135 * CV_PI / 180));

        cv::line(origFrame, pt1_45, pt2_45, cv::Scalar(0, 0, 255), 2);
        cv::line(origFrame, pt1_135, pt2_135, cv::Scalar(0, 0, 255), 2);

        // Draw the axis line in green
        int lineLength = 100;
        cv::line(origFrame, cv::Point(centroid_xm, centroid_ym), cv::Point(centroid_xm + lineLength * cos(orientation), centroid_ym + lineLength * sin(orientation)), cv::Scalar(0, 255, 0), 4);
        cv::line(origFrame, cv::Point(centroid_xm, centroid_ym), cv::Point(centroid_xm - lineLength * cos(orientation), centroid_ym - lineLength * sin(orientation)), cv::Scalar(0, 255, 0), 4);

        // Get x & y values to draw boxes around the blobs
        int x = stats.at<int>(i, cv::CC_STAT_LEFT);
        int y = stats.at<int>(i, cv::CC_STAT_TOP);
        int width = stats.at<int>(i, cv::CC_STAT_WIDTH);
        int height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        int area = stats.at<int>(i, cv::CC_STAT_AREA);

        // Draw the bounding box
        cv::rectangle(origFrame, cv::Point(x, y), cv::Point(x + width, y + height), cv::Scalar(255, 0, 0), 2);

        // Add x and y components to the array to be used later when labeling by group
        xyComponents.push_back(std::make_pair(x, y));

        // Create tuple to store component data
        std::tuple<int, double, int> comp;

        // Creates a ROI for the HSV frame
        cv::Mat roi = hsvFrame(cv::Rect(x, y, width, height));
        // Calculate the mean value of each channel (Hue, Saturation, Value) in the ROI
        cv::Scalar avgHsv = cv::mean(roi);
        
        // Calculate average hsv value for each component by adding adding each value and dividing by 3
        double avg = (avgHsv[0] + avgHsv[1] + avgHsv[2]) / 3;

        // Store data in tuple to be used for grouping
        comp = std::make_tuple(i, avg, area);
        componentData.push_back(comp);

        // Print newline
        std::cout << "\n";
    }

    // Sort the components by average HSV value
    std::sort(componentData.begin(), componentData.end(), [](const std::tuple<int, double, int>& a, const std::tuple<int, double, int>& b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    // Group components with similar average HSV values +/- 1.5
    std::vector<std::vector<std::tuple<int, double, int>>> groupedComponents;
    std::vector<std::tuple<int, double, int>> currentGroup;
    currentGroup.push_back(componentData[0]);

    for (int i = 1; i < componentData.size(); i++) {
        if (std::abs(std::get<1>(componentData[i]) - std::get<1>(currentGroup[0])) < 1.5) {
            currentGroup.push_back(componentData[i]);
        } else {
            groupedComponents.push_back(currentGroup);
            currentGroup.clear();
            currentGroup.push_back(componentData[i]);
        }
    }

    groupedComponents.push_back(currentGroup);

    // Print out the grouped components and also add the group letter to the boxes on the original frame
    for (int i = 0; i < groupedComponents.size(); i++) {
        std::cout << "Group " << getGroupLetter(i) << " has the following components: ";
        for (auto const& comp : groupedComponents[i]) {
            int index = std::get<0>(comp) - 1; // Adjust index to start from 0
            cv::putText(origFrame, std::string(1, std::toupper(getGroupLetter(i))), cv::Point(xyComponents[index].first, xyComponents[index].second), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
            std::cout << std::get<0>(comp) << " ";
        }
        std::cout << "\n";
    }

    // Display the original frame
    cv::imshow("Image with Crosses", origFrame);

    // Closes all windows
    // Wait for a key press indefinitely
    cv::waitKey(0);
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}

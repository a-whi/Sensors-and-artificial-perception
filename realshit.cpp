#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <tuple>


int main() {

    // cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    // if (!cap.isOpened()) {
    //     std::cerr << "Error opening the camera!" << std::endl;
    //     return -1;
    // }

    cv::Mat origFrame = cv::imread("/Users/vinaypanicker/Desktop/c++/blob_stats/Photos/DEMO_components_02.png");

    if (origFrame.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::imshow("origFrame", origFrame);

    cv::Mat greyFrame; // Creating a greyscale frame
    cv::Mat binaryFrame; // Creating a black & white frame
    cv::Mat hsvFrame; // Creating a HSV frame


    // Apply Gaussian blur to the original image
    cv::GaussianBlur(origFrame, origFrame, cv::Size(101, 101),0);
    cv::cvtColor(origFrame, greyFrame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(origFrame, hsvFrame, cv::COLOR_BGR2HSV);


    cv::imshow("bijjFrame", greyFrame);
    cv::imshow("hsvFrame", hsvFrame);

    // Converting grey scale to binary
    cv::threshold(greyFrame, binaryFrame, 100, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    cv::imshow("biFrame", binaryFrame);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(29, 29));
   
    // cv::morphologyEx(binaryFrame, binaryFrame, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(binaryFrame, binaryFrame, cv::MORPH_OPEN, kernel);

    cv::imshow("biFrame2", binaryFrame);


    // Finding the number of components, size of objects and centroid
    cv::Mat labels, stats, centroid;
    int numLabels = cv::connectedComponentsWithStats(binaryFrame, labels, stats, centroid, 8);
    
    // creat empty array to store component data
    std::vector<std::tuple<int, double, int>> componentData;
    
    // Get the centroid of the components
    for(int i = 1; i < numLabels; i++){

        double centroid_x = centroid.at<double>(i, 0);
        double centroid_y = centroid.at<double>(i, 1);

        std::cout << "Centroid " << i << " is: " << centroid_x << ", " << centroid_y<< " \n";

        // Extract the binary image for the current component
        cv::Mat componentBinary = (labels == i);
        cv::Moments m = moments(componentBinary, true);

        //get centroid via moments
        double centroid_xm = m.m10/m.m00;
        double centroid_ym = m.m01/m.m00;

        // Print out the center coordinates
        std::cout << "Centroid_M " << i << " is: " << centroid_xm << ", " << centroid_ym<< " \n";

        double orientation = 0.5 * atan2(2 * m.mu11, m.mu20-m.mu02);

        // Draw the line of the orientation
        int lineLength = 100;
        cv::line(origFrame, cv::Point(centroid_x, centroid_y), cv::Point(centroid_x + lineLength * cos(orientation), centroid_y + lineLength * sin(orientation)), cv::Scalar(0, 255, 0), 4);
        cv::line(origFrame, cv::Point(centroid_x, centroid_y), cv::Point(centroid_x - lineLength * cos(orientation), centroid_y - lineLength * sin(orientation)), cv::Scalar(0, 255, 0), 4);

        // Print out the orientation
        std::cout << "Orientation " << i << " is: " << orientation * 180/CV_PI << " \n";

        // Drawing on the original image
        int crossArmLength = 20;

        // Calculate the endpoints of each arm
        cv::Point pt1_45(centroid_x + crossArmLength * cos(45 * CV_PI / 180), centroid_y + crossArmLength * sin(45 * CV_PI / 180));
        cv::Point pt2_45(centroid_x - crossArmLength * cos(45 * CV_PI / 180), centroid_y - crossArmLength * sin(45 * CV_PI / 180));

        cv::Point pt1_135(centroid_x + crossArmLength * cos(135 * CV_PI / 180), centroid_y + crossArmLength * sin(135 * CV_PI / 180));
        cv::Point pt2_135(centroid_x - crossArmLength * cos(135 * CV_PI / 180), centroid_y - crossArmLength * sin(135 * CV_PI / 180));

        // Draw the lines for each arm in red
        cv::line(origFrame, pt1_45, pt2_45, cv::Scalar(0, 0, 255), 2);
        cv::line(origFrame, pt1_135, pt2_135, cv::Scalar(0, 0, 255), 2);

        // Get x,y
        int x = stats.at<int>(i, cv::CC_STAT_LEFT);
        int y = stats.at<int>(i, cv::CC_STAT_TOP);
        int width = stats.at<int>(i, cv::CC_STAT_WIDTH);
        int height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        int area = stats.at<int>(i, cv::CC_STAT_AREA);

        // Print out the stats

        // Draw the bounding box + label box with index
        cv::rectangle(origFrame, cv::Point(x, y), cv::Point(x + width, y + height), cv::Scalar(255, 0, 0), 2);
        cv::putText(origFrame, std::to_string(i), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        
        // Group objects by average hsv value in box and components area

        // Create tuple to store component data
        std::tuple<int, double, int> comp;

        cv::Mat roi = hsvFrame(cv::Rect(x, y, width, height));
        cv::Scalar avgHsv = cv::mean(roi);
        std::cout << "Average HSV for component " << i << " is: " << avgHsv << " \n";
        std::cout << "Area for component " << i << " is: " << area << " \n";
        
        // calculate average hsv value for each component by adding adding each value and dividing by 3
        double avg = (avgHsv[0] + avgHsv[1] + avgHsv[2]) / 3;

        std::cout << "Average HSV-m for component " << i << " is: " << avg << " \n";

        comp = std::make_tuple(i, avg, area);
        componentData.push_back(comp);



        // print newline


        std::cout << "\n";



    }

    // Sort the components by average hsv value
    std::sort(componentData.begin(), componentData.end(), [](const std::tuple<int, double, int>& a, const std::tuple<int, double, int>& b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    // Print out the sorted components
    for (auto const& comp : componentData) {
        std::cout << "Component " << std::get<0>(comp) << " has average HSV value: " << std::get<1>(comp) << " and area: " << std::get<2>(comp) << " \n";
    }

    // group components with similar average hsv values +/- 1.5
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

    // Print out the grouped components
    for (int i = 0; i < groupedComponents.size(); i++) {
        std::cout << "Group " << i << " has the following components: ";
        for (auto const& comp : groupedComponents[i]) {
            std::cout << std::get<0>(comp) << " ";
        }
        std::cout << "\n";
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

#include <opencv2/opencv.hpp>
#include <iostream>

/* Ahmet 13-Feb-2024

Sample program for capturing and displaying a frame, and printing the pixel
values at a given position.   
To compile, copy the following lines into a file, say "compile.bat", and
save it in the directory where this file is stored. Then, run it in a Windows terminal.

  g++ -o opencv_cam opencv_cam.cpp -std=c++17^
    -I "C:\msys64\mingw64\include\opencv4"^
    -L "C:\msys64\mingw64\bin"^
    -lopencv_core-409 -lopencv_highgui-409 -lopencv_imgcodecs-409^
    -lopencv_imgproc-409 -lopencv_videoio-409
*/

int main() {
    cv::VideoCapture cap(0); // On my laptop "0" is the built-in camera. 
    if (!cap.isOpened()) {
        std::cerr << "Error opening the camera!" << std::endl;
        return -1;
    }

    int x = 100; int y = 200;  // I am interested in this pixel 

    cv::Mat frame;
    
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "No frame captured?" << std::endl;
        return -1;
    }

    cv::imshow("Frame", frame);

    cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);
    std::cout << "Pixel values at (" << x << ", " << y << "): ";
    std::cout << "B: " << static_cast<int>(pixel[0]) << " ";
    std::cout << "G: " << static_cast<int>(pixel[1]) << " ";
    std::cout << "R: " << static_cast<int>(pixel[2]) << std::endl;
    cv::waitKey(0);
    
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
} // main()
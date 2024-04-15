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

            std::cout << "at1: " << at1 << " at2: " << at2 << " at4: " << at4 << std::endl;

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

    std::cout << "PARTIY_L: " << parity_l << std::endl;

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

    // print barcode_int array
    for (int i = 0; i<barcode_int.size(); i++){
        std::cout << barcode_int[i] << " ";
    }

    for (int i = 0; i<=11; i++){
        std::cout << "Barcode[" << i << "]: " << barcode_int[i] << std::endl;
        if(i%2 != 0){
            odd += barcode_int[i]*3;
        }
        else{
            even += barcode_int[i];
        }
    }
    int sum = (odd + even)%10;
    sum = 10 - sum;

    std::cout << "Barcode[12]: " << barcode_int[12] << std::endl;
    std::cout << "Sum: " << sum << std::endl;

    return (sum == barcode_int[12]) ? 1 : 0;
}


int main() {
    cv::Mat img = cv::imread("/Users/vinaypanicker/Desktop/c++/barcodes/EAN13_Reader/scanned.jpg");
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
    std::cout << "Barcode: " << barcode << std::endl;
    std::cout << "Is Valid? " << correct << std::endl;

    return 0;
}



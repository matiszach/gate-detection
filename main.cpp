#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <array>
#include <iostream>

struct Color {
    u_int8_t r, g, b, a;

    Color(const cv::Vec4b& color)
     : r(color[0]), g(color[1]), b(color[2]), a(color[3]) {}

    Color(const cv::Vec3b& color)
     : r(color[2]), g(color[1]), b(color[0]), a(255) {}
    
    Color(const u_int8_t& r, const u_int8_t& g, const u_int8_t& b, const u_int8_t& a)
     : r(r), g(g), b(b), a(a) {}
    
    Color()
     : r(0), g(0), b(0), a(0) {}
};

#define WIDTH 1280
#define HEIGHT 540

template <typename T>
using BitMap = std::array<std::array<T, WIDTH>, HEIGHT>;

using Dist = double;

Dist dist(const Color& c1, const Color& c2) {
    return sqrt((Dist)(c1.r - c2.r) * (c1.r - c2.r) + 
                (Dist)(c1.g - c2.g) * (c1.g - c2.g) + 
                (Dist)(c1.b - c2.b) * (c1.b - c2.b));
}

void detectGate(BitMap<Color> bitmap) {

    // 1. Detection of all pixels of similiar color.

    BitMap<bool> result = {};

    const Dist max_diff = 25.0;
    const Color color(97, 76, 121, 0);

    for (size_t i = 0; i < HEIGHT; ++i) {
        for (size_t j = 0; j < WIDTH; ++j) {
            if (dist(bitmap[i][j], color) <= max_diff) {
                result[i][j] = 1;
            }
        }
    }

    // 1a. Pixel clustering.

    const Dist extra = 2.0;

    for (size_t i = 1; i < HEIGHT - 1; ++i) {
        for (size_t j = 1; j < WIDTH - 1; ++j) {
            if (result[i][j]) continue;
            int cnt = 0;
            for (size_t posx = i - 1; posx <= i + 1; ++posx) {
                for (size_t posy = j - 1; posy <= j + 1; ++posy) {
                    cnt += result[posx][posy];
                }
            }
            if (dist(bitmap[i][j], color) <= max_diff - 10.0 + cnt * extra) {
                result[i][j] = 1;
            }
        }
    }

    // Convertion to image.

    cv::Mat image = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
    image.setTo(cv::Scalar(255, 255, 255));

    for (size_t i = 0; i < HEIGHT; ++i) {
        for (size_t j = 0; j < WIDTH; ++j) {
            if (result[i][j]) {
                image.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
            }
        }
    }

    if (cv::imwrite("../output/test.png", image)) {
        std::cout << "Image saved successfully!" << std::endl;
    } else {
        std::cerr << "Failed to save the image." << std::endl;
    }
}

bool convertImage(const std::string& imagePath, BitMap<Color>& bitmap) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

    if (image.empty()) return false;

    for (size_t i = 0; i < HEIGHT; ++i) {
        for (size_t j = 0; j < WIDTH; ++j) {
            bitmap[i][j] = Color(image.at<cv::Vec3b>(i, j));
        }
    }

    return true;
}

int main() {
    BitMap<Color> test{};
    const std::string imagePath = "../input/img10.png";

    convertImage(imagePath, test);

    detectGate(test);

    // Hello world!
}
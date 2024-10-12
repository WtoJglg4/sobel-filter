#include <opencv2/opencv.hpp>
#include <iostream>
#include <pthread.h>
#include <cmath>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace cv;

// Horizontal kernel - detects quick brightness changes in the horizontal direction.
const int kernelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

// Vertical kernel - detects quick brightness changes in the vertical direction.
const int kernelY[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
};

// Source image and result.
// Mat - OpenCV class to storing data.
Mat image;
Mat result;

// TreadData uses for passing parameters to threads.
struct ThreadData {
    int startRow;
    int endRow;
};

// applySobelFilter applies Sobel filter to a part of source image.
void* applySobelFilter(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int gx, gy;

    for (int i = data->startRow; i < data->endRow; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            gx = gy = 0;
            // Apply Sobel kernels, with boundary checks
            for (int k = -1; k <= 1; ++k) {
                for (int l = -1; l <= 1; ++l) {
                    int row = i + k;
                    int col = j + l;
                    // Check boundaries
                    if (row >= 0 && row < image.rows && col >= 0 && col < image.cols) {
                        int pixel = image.at<uchar>(row, col);
                        gx += pixel * kernelX[k + 1][l + 1];
                        gy += pixel * kernelY[k + 1][l + 1];
                    }
                }
            }
            int magnitude = sqrt(gx * gx + gy * gy);
            result.at<uchar>(i, j) = saturate_cast<uchar>(magnitude);
        }
    }
    return nullptr;
}

int main() {
    // Loading a grayscale image
    // This is important because the Sobel filter is applied to a single-channel image.
    string imagePath = "goslov.jpg";
    image = imread(imagePath, IMREAD_GRAYSCALE);

    if (image.empty()) {
        cerr << "Ошибка загрузки изображения!" << endl;
        return -1;
    }

    // Creating result matrix
    result = Mat::zeros(image.rows, image.cols, image.type());

    // Testing threads number
    vector<int> threadCounts = {1, 2, 4, 8, 16, 32};

    for (int threadCount : threadCounts) {
        // Start of time measurement
        auto start = high_resolution_clock::now();

        pthread_t threads[threadCount];
        ThreadData threadData[threadCount];

        int rowsPerThread = image.rows / threadCount;

        // Treads creating
        for (int i = 0; i < threadCount; ++i) {
            threadData[i].startRow = i * rowsPerThread;
            threadData[i].endRow = (i == threadCount - 1) ? image.rows : (i + 1) * rowsPerThread;
            pthread_create(&threads[i], nullptr, applySobelFilter, &threadData[i]);
        }

        // Waiting all threads will be done
        for (int i = 0; i < threadCount; ++i) {
            pthread_join(threads[i], nullptr);
        }

        // End of time measurement
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();

        cout << "Threads: " << threadCount << ", Time: " << duration << " ms" << endl;
    }

    // Store the result
    imwrite("sobel_output.jpg", result);

    return 0;
}

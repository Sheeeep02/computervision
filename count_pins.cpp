#include <opencv2/opencv.hpp>
#include <opencv2/geometry.hpp>   // OpenCV 5 里 contourArea / moments 移到了这里
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace cv;
using namespace std;

int main()
{
#ifdef _WIN32
    // 源码里的中文是 UTF-8，而 Windows 控制台默认是 GBK(936)，不设置就会乱码
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 1. 读入灰度图（回形针计数不需要颜色信息）
    Mat gray = imread("C:\\Users\\gongc\\Desktop\\pins.jpg", IMREAD_GRAYSCALE);
    if (gray.empty())
    {
        cout << "图片读取失败，请检查路径是否正确" << endl;
        return -1;
    }

    // 2. 反相二值化：回形针是深色、桌面是浅色
    //    用 OTSU 自动找最佳阈值，比手工指定 100 更稳
    Mat binary;
    threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    // 3. 形态学闭运算：把铁丝上可能出现的细小断裂连上，保证一个回形针是一个整体
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel, Point(-1, -1), 2);

    // 4. 只取外轮廓：一个回形针 = 一个连通体 = 一条外轮廓
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 5. 面积过滤掉噪点、桌面阴影等小连通块
    Mat result;
    cvtColor(gray, result, COLOR_GRAY2BGR);
    int rows = gray.rows, cols = gray.cols;
    int count = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area < 800) continue;          // 面积阈值：太小的是噪声

        // 排除贴着图像边缘的轮廓：照片黑边框、桌面边缘等
        Rect box = boundingRect(contours[i]);
        bool touchBorder = box.x <= 2 || box.y <= 2 ||
                           box.x + box.width >= cols - 2 ||
                           box.y + box.height >= rows - 2;
        if (touchBorder) continue;

        count++;
        drawContours(result, contours, (int)i, Scalar(0, 0, 255), 2);

        // 用重心位置标注序号
        Moments m = moments(contours[i]);
        Point center(cvRound(m.m10 / m.m00), cvRound(m.m01 / m.m00));
        putText(result, to_string(count), center, FONT_HERSHEY_SIMPLEX,
                0.9, Scalar(255, 0, 0), 2);
    }

    cout << "回形针数量: " << count << endl;

    imwrite("C:\\Users\\gongc\\Desktop\\pins_result.jpg", result);
    imshow("result", result);
    waitKey(0);
    return 0;
}

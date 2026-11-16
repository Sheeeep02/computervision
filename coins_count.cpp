#include <opencv2/opencv.hpp>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace cv;
using namespace std;

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // 防止控制台中文乱码
#endif

    //  1. 读图 
    string path = "C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\coins.jpg";
    Mat gray = imread(path, IMREAD_GRAYSCALE);
    if (gray.empty())
    {
        cout << "图片读取失败，请检查路径: " << path << endl;
        return -1;
    }

    //  2. 阈值分割 
    // 硬币是亮的、桌面是暗的，取"亮于 100"的像素（实测 90~130 都能稳定分出 10 枚）
    Mat binary;
    threshold(gray, binary, 100, 255, THRESH_BINARY);

    //  3. 连通域统计 
    // 每个连通域返回：外接框 left/top/width/height、像素面积 area，以及质心坐标 centroids
    Mat labels, stats, centroids;
    int n = connectedComponentsWithStats(binary, labels, stats, centroids, 8, CV_32S);
    cout << "连通域总数(含背景): " << n << endl;

    //  4. 逐个判断是不是硬币 
    Mat result;
    cvtColor(gray, result, COLOR_GRAY2BGR);
    int count = 0;

    for (int i = 1; i < n; i++)   // 0 号是背景，从 1 开始
    {
        int area = stats.at<int>(i, CC_STAT_AREA);
        int x = stats.at<int>(i, CC_STAT_LEFT);
        int y = stats.at<int>(i, CC_STAT_TOP);
        int w = stats.at<int>(i, CC_STAT_WIDTH);
        int h = stats.at<int>(i, CC_STAT_HEIGHT);

        // 条件1：面积——太小是噪点，太大是整片背景
        if (area < 2000 || area > 30000) continue;

        // 条件2：外接框长宽比接近 1——硬币接近圆形，外接框就是正方形
        double ratio = (double)w / h;
        if (ratio < 1) ratio = 1.0 / ratio;
        if (ratio > 1.4) continue;

        count++;

        // 画外接框 + 质心 + 编号
        rectangle(result, Rect(x, y, w, h), Scalar(0, 0, 255), 2);
        Point c(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1)));
        circle(result, c, 3, Scalar(0, 255, 0), -1);
        putText(result, to_string(count), c + Point(-10, 8),
                FONT_HERSHEY_SIMPLEX, 0.9, Scalar(255, 0, 0), 2);

        printf("第 %d 枚: 质心(%d,%d)  外接框 %dx%d  面积 %d  长宽比 %.2f\n",
               count, c.x, c.y, w, h, area, ratio);
    }

    cout << "硬币数量: " << count << endl;

    //  5. 输出 
    imwrite("C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\coins_result.jpg", result);
    imwrite("C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\coins_binary.jpg", binary);

    imshow("coins", result);
    imshow("binary", binary);
    waitKey(0);
    return 0;
}

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

    //   1. 读图  
    string path = "C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\wheel.jpg";
    Mat gray = imread(path, IMREAD_GRAYSCALE);
    if (gray.empty())
    {
        cout << "图片读取失败，请检查路径: " << path << endl;
        return -1;
    }

    //   2. 阈值分割  
    // 孔是暗的、轮毂是亮的，所以要取"暗于 130"的像素，用反相二值化 THRESH_BINARY_INV
    Mat binary;
    threshold(gray, binary, 130, 255, THRESH_BINARY_INV);

    //   3. 连通域统计  
    Mat labels, stats, centroids;
    int n = connectedComponentsWithStats(binary, labels, stats, centroids, 8, CV_32S);
    cout << "连通域总数(含背景): " << n << endl;

    //   4. 逐个判断是不是孔  
    Mat result;
    cvtColor(gray, result, COLOR_GRAY2BGR);
    int count = 0;

    for (int i = 1; i < n; i++)
    {
        int area = stats.at<int>(i, CC_STAT_AREA);
        int x = stats.at<int>(i, CC_STAT_LEFT);
        int y = stats.at<int>(i, CC_STAT_TOP);
        int w = stats.at<int>(i, CC_STAT_WIDTH);
        int h = stats.at<int>(i, CC_STAT_HEIGHT);

        // 条件1：面积——太小是表面麻点，太大是中心大盘或左侧大凹槽
        if (area < 1000 || area > 15000) continue;

        // 条件2：外接框长宽比接近 1——孔是圆的，外接框就是正方形
        double ratio = (double)w / h;
        if (ratio < 1) ratio = 1.0 / ratio;
        if (ratio > 1.4) continue;

        count++;

        rectangle(result, Rect(x, y, w, h), Scalar(0, 0, 255), 2);
        Point c(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1)));
        circle(result, c, 3, Scalar(0, 255, 0), -1);
        putText(result, to_string(count), c + Point(-10, 8),
                FONT_HERSHEY_SIMPLEX, 0.9, Scalar(255, 0, 0), 2);

        printf("第 %d 个孔: 质心(%d,%d)  外接框 %dx%d  面积 %d  长宽比 %.2f\n",
               count, c.x, c.y, w, h, area, ratio);
    }

    cout << "轮毂孔数量: " << count << endl;

    //   5. 输出  
    imwrite("C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\wheel_result.jpg", result);
    imwrite("C:\\Users\\gongc\\WorkBuddy\\2026-10-29-11-20-23\\opencv-demo\\pic\\wheel_binary.jpg", binary);

    imshow("wheel holes", result);
    imshow("binary", binary);
    waitKey(0);
    return 0;
}

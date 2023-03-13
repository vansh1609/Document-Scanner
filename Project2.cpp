#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>
using namespace std;
using namespace cv;

//PROJECT 2- DOCUMENT SCANNER//

Mat preprocessing(Mat img)
{
	Mat imgGray,imgBlur,imgCanny,imgDil;
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	return imgDil;
}
vector<Point> getContours(Mat imgDil, Mat img)
{
	vector<vector<Point>> contours;
	vector<Point> biggest;
	vector<Vec4i> heirarchy;
	findContours(imgDil, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundingrect(contours.size());
	//drawContours(img, contours, -1, Scalar(255, 0, 255), 5);
	int maxarea = 0;
	for (int i = 0;i < contours.size();i++)
	{

		int objcorner;
		string type;
		int area = contourArea(contours[i]);
		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			if (area > maxarea && conPoly[i].size()==4)
			{
				drawContours(img, conPoly, i, Scalar(255, 0, 255), 2);
				biggest = { conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3] };
				maxarea = area;
			}
			
			//drawContours(img, conPoly, i, Scalar(255, 0, 255), 2);
			//rectangle(img, boundingrect[i].tl(), boundingrect[i].br(), Scalar(0, 255, 0), 2);

		}
	}
	return biggest;
}
void drawPoints(vector<Point> points, Scalar color, Mat img)
{
	for (int i = 0;i < points.size();i++)
	{
		circle(img, points[i], 5, color, FILLED);
		putText(img, to_string(i), points[i], FONT_HERSHEY_PLAIN, 2, color, 2);
	}
}
vector<Point> reorder(vector<Point>& points)
{
	vector<Point> newpoint;
	vector<int> sum, diff;
	for (int i = 0;i < points.size();i++)
	{
		sum.push_back(points[i].x + points[i].y);
		diff.push_back(points[i].x - points[i].y);
	}
	newpoint.push_back(points[min_element(sum.begin(), sum.end()) - sum.begin()]);//0
	newpoint.push_back(points[max_element(diff.begin(), diff.end()) - diff.begin()]);//1
	newpoint.push_back(points[min_element(diff.begin(), diff.end()) - diff.begin()]);//2
	newpoint.push_back(points[max_element(sum.begin(), sum.end()) - sum.begin()]);//3

	return newpoint;
}
Mat getWarp(Mat img,vector<Point> &points,float w,float h)
{
	Mat imgWarp;
	Point2f src[4] = {points[0],points[1],points[2],points[3]};
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };
	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));
	return imgWarp;
}
int main()
{
	string path = "Resources/paper.png";
	Mat img = imread(path);
	vector<Point> initial,docPoints;
	float w = 420, h = 596;
	resize(img, img, Size(), 0.5,0.5);
	Mat imgThres=preprocessing(img);
	initial = getContours(imgThres, img);
	//drawPoints(initial, Scalar(0, 0, 255), img);
	docPoints = reorder(initial);
	drawPoints(docPoints, Scalar(0, 255, 0), img);
	Mat imgWarp = getWarp(img, docPoints, w, h);
	Rect roi(5, 5, w - (2 * 5), h - (2 * 5));
	Mat imgCrop = imgWarp(roi);
	imshow("image",img);
	imshow("image2", imgThres);
	imshow("image3", imgWarp);
	imshow("image4", imgCrop);
	waitKey(0);
	return 0;
}
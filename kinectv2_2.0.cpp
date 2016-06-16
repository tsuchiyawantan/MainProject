#include <iostream>
#include <sstream>
#include <time.h>
#include "stdafx.h"
#include "KinectControl.h"
#include "Depth.h"
#include "Dot.h"
#include "NeonDesign.h"
#include "Log.h"
#include "CatmullSpline.h"
#include "ExecuteSpaceFiltering.h"
#define HUE 60
#define SPACESIZE 10
#define SCALESIZE 1
#define FILTERSIZE 81

string hstate[] = { "unknown", "nottracked", "Open", "Closed", "Lasso" };
string hconf[] = { "low", "high" };

string str(pair<int, int> p) {
	stringstream ss;
	ss << hstate[p.first] << ":" << hconf[p.second];
	return ss.str();
}

string str(pair<int, int> left, pair<int, int>right) {
	stringstream ss;
	ss << str(left) << " " << str(right);
	return ss.str();
}
void doCatmull(cv::Mat &srcImg, vector<vector<pair<int, int>>> &approximationLine){
	cv::Mat resultImg = cv::Mat(srcImg.rows, srcImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	CatmullSpline catmull;
	for (int i = 0; i < approximationLine.size(); i++){
		catmull.drawLine(resultImg, approximationLine[i], HUE);
	}
	//SpaceFiltering
	//catmull.exeGaussian(approximationLine, resultImg, FILTERSIZE);
	//Opencv Gaussian
	cv::GaussianBlur(resultImg, resultImg, cv::Size(19, 15), 0, 0);
	catmull.drawInline(resultImg, HUE);
	cv::imshow("Catmull Spline", resultImg);
}
void doDot(cv::Mat &srcImg, Log log){
	Dot dot;
	clock_t start = clock();
	dot.setWhiteDots(srcImg);
	clock_t end = clock();
	log.Write("��setWhiteDots: " + to_string((double)(end - start) / CLOCKS_PER_SEC));

	dot.findStart(srcImg);
	dot.makeLine(srcImg);
	dot.makeSpace(SPACESIZE);
	dot.scalable(SCALESIZE);
	doCatmull(srcImg, dot.approximationLine);
}

void main() {
	try {
		KinectControl kinect;
		Depth depth;
		Log log;
		log.Initialize("logPOINTER.txt");
		while (1) {
			clock_t start = clock();
			depth.setBodyDepth();
			clock_t end = clock();
			cv::imshow("body index depth", depth.bodyDepthImage);
			depth.setNormalizeDepth(depth.bodyDepthImage);
			cv::imshow("normalize depth image", depth.normalizeDepthImage);
			depth.setContour(depth.normalizeDepthImage);
			cv::imshow("contour image", depth.contourImage);
			doDot(depth.contourImage, log);
			cv::imshow("complete image", depth.contourImage);
			auto key = cv::waitKey(20);
			if (key == 'q') break;
		}
	}
	catch (exception& ex) {
		cout << ex.what() << endl;
		string s;
		cin >> s;
	}
}

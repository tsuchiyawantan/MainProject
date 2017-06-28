#pragma once
#include <opencv2/opencv.hpp>

#include <iostream>
#include <sstream>
#include <Windows.h>
#include <time.h>
#include "NeonDesign.h"
#include "Node.h"

using namespace std;

class CatmullSpline{
private:
	const double T_SIZE = 0.05;
public:
	vector<vector<pair<int, int>>> catmullLine;
	cv::Mat resultImg;

	CatmullSpline(){}
	~CatmullSpline(){}

	void init(){
		catmullLine.clear();
	}
	double catmullRom(double p0, double p1, double p2, double p3, double t){
		double t2 = t*t;
		double t3 = t2*t;
		return 0.5*((2 * p1) + (p2 - p0)*t + (2 * p0 - 5 * p1 + 4 * p2 - p3)*t2 + (-p0 + 3 * p1 - 3 * p2 + p3)*t3);
	}
	double catmullRomFirstLast(double p1, double p2, double t){
		double t2 = t*t;
		double t3 = t2*t;
		return 0.5*((2 * p1) + (p2 - p1)*t + (2 * p1 - 5 * p1 + 4 * p2 - p2)*t2 + (-p1 + 3 * p1 - 3 * p2 + p2)*t3);
	}
	void adjust(vector<pair<int, int>> &yx){
		int j = yx.size() + (4 - yx.size() % 4);
		while (yx.size() < j){
			yx.push_back(make_pair(yx.back().first, yx.back().second));
		}
	}

	boolean check8(cv::Mat& srcImg, int y, int x) {
		int n[8][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } };
		int count = 0;
		for (int i = 0; i < 8; i++) {
			int dy = y + n[i][0];
			int dx = x + n[i][1];
			if (dy < 0 || dy >= srcImg.rows || dx < 0 || dx >= srcImg.cols) continue;
			if (srcImg.at<cv::Vec3b>(dy, dx)[0] != 255 &&
				srcImg.at<cv::Vec3b>(dy, dx)[1] != 255 &&
				srcImg.at<cv::Vec3b>(dy, dx)[2] != 255) return true;
		}
		return false;
	}

	void drawInline(cv::Mat &srcImg, vector<vector<Node *>> &node_array, int hue){
		NeonDesign design;
		int limit = 0;
		int b = 0, g = 0, r = 0;
		for (int i = 0; i < node_array.size(); i++){
			if (node_array[i].size() == 0) continue;
			double size = 1;
			int k = 0;
			for (int j = 0; j < node_array[i].size() - 1; j++){
				Node *first_node = node_array[i].at(j);
				Node *next_node = node_array[i].at(j + 1);
				if (j == 0 || j == node_array[i].size() - 2){ //始点or終点
					limit = k + (1.0 / T_SIZE);
					design.rgb(hue, 100, 255, b, g, r);
					while (k < limit - 1){
						int first_y = catmullLine[i].at(k).first;
						int first_x = catmullLine[i].at(k).second;
						int next_y = catmullLine[i].at(k + 1).first;
						int next_x = catmullLine[i].at(k + 1).second;
						cv::line(srcImg, cv::Point(first_x, first_y), cv::Point(next_x, next_y), cv::Scalar(b, g, r), size, 4);
						k++;
					}
				}
				else{
					if (first_node->isAngularNode() && next_node->isAngularNode()){
						size = 2.0;
						design.rgb(hue, 100, 255, b, g, r);
					}
					else if (!first_node->isAngularNode() && !next_node->isAngularNode()){
						design.rgb(hue, 100, 255, b, g, r);
						size = 0.5;
					}
					else {
						design.rgb(hue, 255 - 100, 255, b, g, r);
						size = 1.0;
					}
					limit = k + (1.0 / T_SIZE);
					while (k < limit - 1){
						int first_y = catmullLine[i].at(k).first;
						int first_x = catmullLine[i].at(k).second;
						int next_y = catmullLine[i].at(k + 1).first;
						int next_x = catmullLine[i].at(k + 1).second;
						//if (size!=0.5)
						cv::line(srcImg, cv::Point(first_x, first_y), cv::Point(next_x, next_y), cv::Scalar(b, g, r), size, 4);
						k++;
					}
				}
			}
		}
	}

	void doGaussian(vector<pair<int, int>> &catCtr, cv::Mat &srcImg){
		int mid = catCtr.size() / 2;
		int filtersize = mid;
		int y = catCtr.at(mid).first;
		int x = catCtr.at(mid).second;
		int x1 = x - filtersize;
		int y1 = y - filtersize;
		int y2 = filtersize * 2;
		int x2 = filtersize * 2;

		fixSize(y1, x1, srcImg);
		fixSize2(y1, x1, y2, x2, srcImg);

		cv::Mat regionOfImage(srcImg, cv::Rect(x1, y1, x2, y2));
		cv::GaussianBlur(regionOfImage, regionOfImage, cv::Size(11, 11), 0, 0);
	}

	void fixSize(int &y, int &x, cv::Mat &srcImg){
		if (x < 0) { x = 0; }
		if (y < 0) { y = 0; }
		if (x >= srcImg.cols){ x = srcImg.cols - 1; }
		if (y >= srcImg.rows){ y = srcImg.rows - 1; }
	}
	void fixSize2(int y1, int x1, int &y_sub, int &x_sub, cv::Mat &srcImg){
		int y2 = y1 + y_sub;
		int x2 = x1 + x_sub;

		if (y2 >= srcImg.rows) {
			y2 = srcImg.rows - 1;
			y_sub = y2 - y1;
		}
		if (x2 >= srcImg.cols) {
			x2 = srcImg.cols - 1;
			x_sub = x2 - x1;
		}
	}

	void drawLine(cv::Mat &resultImg, vector<vector<Node *>> node_array, int hue){
		NeonDesign design;
		int b = 0, g = 0, r = 0;
		double size = 5;
		vector<pair<int, int>> ctr;
		cv::Point first;
		cv::Point second;
		cv::Point third;
		cv::Point forth;
		design.rgb(hue, 255, 255 - 100, b, g, r);

		for (int i = 0; i < node_array.size(); i++){
			ctr.clear();
			for (int j = 0; j < node_array[i].size(); j++){
				Node *node = node_array[i].at(j);
				int y = (*node).getNodeY();
				int x = (*node).getNodeX();
				if (j >= node_array[i].size() || j + 1 >= node_array[i].size() || j + 2 >= node_array[i].size() || j + 3 >= node_array[i].size()) break;
				if (j == 0){ //始点
					design.rgb(hue, 255, 255 - 120, b, g, r);
					Node *first_node = node_array[i].at(0);
					Node *second_node = node_array[i].at(1);
					first.y = (*first_node).getNodeY();
					first.x = (*first_node).getNodeX();
					second.y = (*second_node).getNodeY();
					second.x = (*second_node).getNodeX();
					for (double t = 0; t <= 1.0; t += T_SIZE){
						y = catmullRomFirstLast(first.y, second.y, t);
						x = catmullRomFirstLast(first.x, second.x, t);
						ctr.push_back(make_pair(y, x));
						circle(resultImg, cv::Point(x, y), size, cv::Scalar(b, g, r), -1, 8);
					}
				}
				Node *first_node = node_array[i].at(j);
				Node *second_node = node_array[i].at(j + 1);
				Node *third_node = node_array[i].at(j + 2);
				Node *forth_node = node_array[i].at(j + 3);
				first.y = (*first_node).getNodeY();
				first.x = (*first_node).getNodeX();
				second.y = (*second_node).getNodeY();
				second.x = (*second_node).getNodeX();
				third.y = (*third_node).getNodeY();
				third.x = (*third_node).getNodeX();
				forth.y = (*forth_node).getNodeY();
				forth.x = (*forth_node).getNodeX();

				if (third_node->isAngularNode() && forth_node->isAngularNode()){
					size = 6;
					design.rgb(hue, 255, 255-70, b, g, r);

				}
				else if (!third_node->isAngularNode() && !forth_node->isAngularNode()){
					size = 5.5;
					design.rgb(hue, 255, 255 - 150, b, g, r);

				}
				else {
					size = 5;
					design.rgb(hue, 255, 255 - 120, b, g, r);
				}
				for (double t = 0; t <= 1.0; t += T_SIZE){
					y = catmullRom(first.y, second.y, third.y, forth.y, t);
					x = catmullRom(first.x, second.x, third.x, forth.x, t);
					ctr.push_back(make_pair(y, x));
					circle(resultImg, cv::Point(x, y), size, cv::Scalar(b, g, r), -1, 8);
				}
				if (j == node_array[i].size() - 4){ //（終点-4）番目
					size = 5.5;
					Node *third_node = node_array[i].at(node_array[i].size() - 2);
					Node *forth_node = node_array[i].at(node_array[i].size() - 1);
					third.y = (*third_node).getNodeY();
					third.x = (*third_node).getNodeX();
					forth.y = (*forth_node).getNodeY();
					forth.x = (*forth_node).getNodeX();
					for (double t = 0; t <= 1.0; t += T_SIZE){
						y = catmullRomFirstLast(third.y, forth.y, t);
						x = catmullRomFirstLast(third.x, forth.x, t);
						ctr.push_back(make_pair(y, x));
						circle(resultImg, cv::Point(x, y), 5, cv::Scalar(b, g, r), -1, 8);
					}
					break;
				}
			}
			catmullLine.push_back(ctr);
		}
	}
};
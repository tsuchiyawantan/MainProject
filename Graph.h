#pragma once
#include <opencv2/opencv.hpp>
#include <time.h>
#include "Node.h"
#include "Edge.h"
#include "Voronoi.h"

using namespace std;

class Graph{
private:
public:
	Graph(){}
	~Graph(){}

	void toGraph(cv::Mat& src_img, vector<vector<cv::Point>> divcon, vector<vector<Node *>> &node_array){
		//ノードの用意
		for (int i = 0; i < divcon.size(); i++){
			vector<Node *> node_array_child;
			cv::Point node;

			//エッジがない場合＝ノードが右隣にいない
			if (divcon[i].size() == 1) {
				node = divcon[i].at(0);
				node_array_child.push_back(new Node(node, 0));
				continue;
			}

			//エッジを一個持ってる状態
			for (int j = 0; j < divcon[i].size(); j++){
				node = divcon[i].at(j);
				node_array_child.push_back(new Node(node, 1));
			}
			//終点はノードが右隣にいない
			node = divcon[i].at(divcon[i].size() - 1);
			node_array_child.push_back(new Node(node, 0));

			//ノードの連結操作
			Node *this_node;
			Node *prev_node;
			Node *next_node;

			for (int l = 0; l < node_array_child.size(); l++){
				if (l == 0){ //始点
					this_node = node_array_child.at(l);
					next_node = node_array_child.at(l + 1);
					(*this_node).addEdgeNode2(next_node, 0);
				}
				else if (l == node_array_child.size() - 1){ //終点
					this_node = node_array_child.at(l);
					prev_node = node_array_child.at(l - 1);
					int edgearray_num = (*prev_node).hasEdge(this_node);
					if (edgearray_num >= 0){
						Edge *edge = (*prev_node).getEdge(edgearray_num);
						(*this_node).addEdge(edge);
					}
				}
				else {
					this_node = node_array_child.at(l);
					prev_node = node_array_child.at(l - 1);
					next_node = node_array_child.at(l + 1);
					(*this_node).addEdgeNode2(next_node, 0);
					int edgearray_num = (*prev_node).hasEdge(this_node);
					if (edgearray_num >= 0){
						Edge *edge = (*prev_node).getEdge(edgearray_num);
						(*this_node).addEdge(edge);
					}
				}
			}
			node_array.push_back(node_array_child);
		}
	}

	//8近傍にforwardがあればtrue
	boolean dotExist(cv::Mat& src_img, cv::Point mid, cv::Point forward){
		int n[8][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } };
		int count = 0;
		int y = mid.y;
		int x = mid.x;
		//中点がforwardの場合
		if (y == forward.y && x == forward.x) return true;
		for (int i = 0; i < 8; i++) {
			int dy = y + n[i][0];
			int dx = x + n[i][1];
			if (dy < 0 || dy >= src_img.rows || dx < 0 || dx >= src_img.cols) continue;
			if (dy == forward.y && dx == forward.x) return true;
		}
		return false;
	}

	//点列の角であろう点だけをset
	void setCorner(cv::Mat& src_img, vector<vector<Node *>> &node_array, vector<vector<Node *>> &newnode_array){
		vector<Node *> node_array_child;
		cv::Point start;
		cv::Point goal;
		cv::Point mid;
		cv::Point forward;
		Node *start_node;
		Node *goal_node;
		Node *forward_node;
		int di;
		int j = 0;

		for (int i = 0; i < node_array.size(); i++){
			node_array_child.clear();
			di = 1;
			//2個先の点と直線を引く
			//直線の中点の8近傍に1個先の点がいなければ、1個先の点は角の可能性あり
			for (j = 0; j < node_array[i].size() - 2; j++){
				start_node = node_array[i].at(j);
				goal_node = node_array[i].at(j + 2);
				forward_node = node_array[i].at(j + 1);
				start.y = (*start_node).getNodeY();
				start.x = (*start_node).getNodeX();
				goal.y = (*goal_node).getNodeY();
				goal.x = (*goal_node).getNodeX();
				forward.y = (*forward_node).getNodeY();
				forward.x = (*forward_node).getNodeX();
				mid.y = (start.y + goal.y) / 2;
				mid.x = (start.x + goal.x) / 2;

				if (!dotExist(src_img, mid, forward)){
					di = 1;
				}
				//角じゃない点が3回続いたら、1点間引く
				//詰まった線ではなく、シュッとした線になる
				if (di % 3 == 0){
					di = 1;
					node_array_child.pop_back();
				}
				node_array_child.push_back(start_node);
				di++;
			}
			while (j < node_array[i].size()){
				node_array_child.push_back(node_array[i].at(j));
				j++;
			}
			newnode_array.push_back(node_array_child);
		}
	}

	void deformeNode(cv::Mat src_img, vector<vector<Node *>> &node_array, vector<vector<Node *>> &former_node){
		if (former_node.size() == 0){
			for (int i = 0; i < node_array.size(); i++){
				for (int j = 0; j < node_array[i].size(); j++){
					Node *node = node_array[i].at(j);
					(*node).circleNode((*node).getNodeX(), (*node).getNodeY());
				}
			}
		}
		else {
			for (int i = 0; i < node_array.size(); i++){
				for (int j = 0; j < node_array[i].size(); j++){
					Voronoi vor;
					vor.mkVoronoiDelaunay(src_img, former_node, node_array[i]);
				//	vor.deforme(src_img, node_array[i], vor.subdiv);

				}
			}		
		}
	}
};
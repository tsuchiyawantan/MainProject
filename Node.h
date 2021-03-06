#pragma once
#include <opencv2/opencv.hpp>
#include <time.h>
#include "Edge.h"

using namespace std;

class Node{
private:
	cv::Point node;
	//0番目は右隣のエッジ、1番目は左隣へのエッジ
	vector<Edge *> edge_array;
	bool has_right_edge = true;
	bool angular = false;
public:

	Node(cv::Point mynode){
	//	srand((unsigned int)time(NULL));
		node = mynode;
		edge_array.push_back(new Edge(this));
	}
	Node(){}
	~Node(){}


	//コピーコンストラクタ
	Node(const Node& copy_node){
		//edgearrayにedgeを生成して、edgeをつっこむ
		//eddgeのコピーコンストラクタが必要
		copy_node.getMyNode(node);
		if (copy_node.hasRightEdge()) {
			edge_array.push_back(new Edge(this));
		}
	}

	//代入演算子
	Node& operator=(const Node& copy_node){
		if (this == &copy_node) return *this;   //自己代入
		if (edge_array.size()){
			for (int i = 0; i < edge_array.size(); i++){
				delete edge_array.at(i);
			}
			edge_array.clear();
			edge_array.shrink_to_fit();
		}
		copy_node.getMyNode(node);
		if (copy_node.hasRightEdge()) {
			Edge *o_edge = new Edge(copy_node);
			edge_array.push_back(o_edge);
		}
		return *this;
	}

	//i番目のエッジのnode2を埋める
	void addEdgeNode2(Node *node2, int i){
		(*edge_array.at(i)).setNode2(node2);
	}

	//新たなエッジを追加
	void addEdge(Edge *edge){
		edge_array.push_back(edge);
	}

	//新たなエッジを追加
	void setEdge(Edge *edge, int num){
		edge_array.at(num) = edge;
	}

	Edge *getEdge(int n) const{
		return edge_array.at(n);
	}

	int getNodeY(){
		return node.y;
	}

	int getNodeX(){
		return node.x;
	}

	void setNodeY(int mynode){
		node.y = mynode;
	}
	
	void setNodeX(int mynode){
		node.x = mynode;
	}

	void setNode(cv::Point mynodes){
		setNodeY(mynodes.y);
		setNodeX(mynodes.x);
	}

	void getMyNode(cv::Point &mynode) const{
		mynode.y = node.y;
		mynode.x = node.x;
	}
	
	int getEdgeNum() const{
		int num = 0;
		for (int i = 0; i < edge_array.size(); i++){
			if (edge_array.at(i)->getNode2() != NULL) num++;
		}
		return edge_array.size();
	}

	int hasEdge(Node *node2){
		Edge *edge;
		for (int i = 0; i < edge_array.size(); i++){
			edge = edge_array.at(i);
			if ((*edge).getNode2() == node2) return i;
		}
		return -1;
	}

	int getRandom(int min, int max)
	{
		return min + (int)(rand()*(max - min + 1.0) / (1.0 + RAND_MAX));
	}

	void circleNode(int x, int y, int R=3){
		node.x = x + getRandom(-R, R);
		node.y = y + getRandom(-R, R);
		if (node.x < 0 || node.y < 0) circleNode(x, y, R);
	}

	bool hasRightEdge() const{
		return has_right_edge;
	}

	void setRightEdge(bool flag){
		has_right_edge = flag;
	}
	
	void setAngularNode(bool flag = true){
		angular = flag;
	}

	bool isAngularNode(){
		return angular;
	}

};

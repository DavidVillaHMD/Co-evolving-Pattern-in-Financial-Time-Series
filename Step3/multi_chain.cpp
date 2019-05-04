#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include<vector>
#define MIN_PATTERN_LEN 10 // when generating a tree, if a node's ts_num < MIN_PATTERN_LEN, then we can prune this node
#pragma warning(disable:4996)
using namespace std;

typedef vector<string> string_list;
typedef vector<string_list> string_list_list;
struct node {
	vector<int> relate_index;
	string_list timestamps;
	vector<node*> child;
	int level;
};
void print_string_list(string_list whatever) {// this function is designed for debugging
	for (int i = 0; i < whatever.size(); i++)
		cout << whatever[i] << "                         " << i << endl;
}
void print_node_to_txt(node whatever) {
	ofstream file("my_result.txt", ios::app);
	for (int i = 0;i < whatever.relate_index.size(); i++)
		file << whatever.relate_index[i] << " ";
	file << endl;
	for (int i = 0; i < whatever.timestamps.size(); i++)
		file << whatever.timestamps[i] << "                         " << i << endl;
	file.close();
}
string_list read_stock_list(string namelist) { // this function can also be used to collect timestamps from txt file.
	string_list result;
	const char* nl = namelist.data();
	ifstream file(nl, ios::in);
	string temp;
	while (getline(file, temp)) {
		stringstream ss;
		ss<<temp;
		string ff;
		ss>>ff;
		result.push_back(ff);
	}
	file.close();
	//print_string_list(result);
	return result;
}
string_list_list get_all_timestamps(string_list namelist) {
	string_list_list result;
	for (int i = 0; i < namelist.size(); i++)
		result.push_back(read_stock_list("CODE" + namelist[i] + ".txt"));
	return result;
}
node* initial(string_list namelist, string_list_list all_data) {
	node* root = new node;
	root->level = 0;
	int length = namelist.size();
	for (int i = 0; i < length; i++) {
		root->child.push_back(new node);
		(root->child)[i]->level = 1;
		(root->child)[i]->relate_index.push_back(i);
		(root->child)[i]->timestamps.assign(all_data[i].begin(), all_data[i].end());
		//cout << i << endl;
		//print_string_list((root->child)[i]->timestamps);
	}
	return root;
}
int datecmp(string a, string b) {
	const char* aa = a.data();
	const char* bb = b.data();
	return strcmp(aa, bb);
}
int comparing(string_list old, string_list now, int old_pt, int now_pt, string_list* result) {
	if (old_pt >= old.size() || now_pt >= now.size())
		return 463;
	int flag = datecmp(old[old_pt], now[now_pt]);
	if (flag == 0) {
		result->push_back(old[old_pt]);
	}
	return flag;
}
string_list compare(string_list old, string_list now) {
	string_list result;
	string_list* result_ptr = &result;
	int opt = 0;
	int npt = 0;
	//int iterflag = 0;
	while (1) {
		int temp = comparing(old, now, opt, npt, result_ptr);
		if (temp == 463)
			break;
		if (temp == 0) {
			opt++;
			npt++;
		}
		else if (temp > 0) {
			npt++;
		}
		else opt++;
	}
	//print_string_list(result);
	return result;
}
void generate_tree(node* root, string_list_list name_of_stocks) {
	int max_index_now = root->relate_index.back();
	if (name_of_stocks.size() == max_index_now)
		return;
	if (root->level > 7){
		print_string_list(root->timestamps);
		if(root->relate_index.size()>=7)
			print_node_to_txt(*root);
		return;
	}
	for (int i = max_index_now + 1; i < name_of_stocks.size(); i++) {
		string_list temp;
		cout << "Now we are dealing " << i << ", level " << root->level << endl;
		for (int i = 0; i < root->relate_index.size(); i++)
			cout << root->relate_index[i] << " ";
		cout << endl;
		temp = compare(root->timestamps, name_of_stocks[i]);
		if (temp.size() < 28)//////////////////////////////////////////////////////////original is 7
			continue;
		else {
			//print_string_list(temp);
			root->child.push_back(new node);
			node* next_node = root->child.back();
			next_node->level = root->level + 1;
			next_node->timestamps.assign(temp.begin(), temp.end());
			next_node->relate_index.assign(root->relate_index.begin(), root->relate_index.end());
			next_node->relate_index.push_back(i);
			generate_tree(next_node, name_of_stocks);
		}
	}
}
int main() {
	string_list stock_namelist = read_stock_list("gagaga.txt");
	print_string_list(stock_namelist);
	string_list_list raw_data = get_all_timestamps(stock_namelist);
	node* root_tree = initial(stock_namelist, raw_data);
	generate_tree(root_tree->child[0], raw_data);
	system("pause");
}

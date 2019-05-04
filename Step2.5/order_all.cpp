#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#define MIN_PATTERN_LEN 10 // when generating a tree, if a node's ts_num < MIN_PATTERN_LEN, then we can prune this node
#pragma warning(disable:4996)
#define PATTERN_SIZE 8
using namespace std;
enum mode { NEG = 0, POS = 1 };//NEGATIVE&POSITIVE
typedef vector<string> string_list;
typedef vector<string_list> string_list_list;
struct node {
	vector<int> relate_index;
	string_list timestamps;
	vector<node*> child;
	int level;
	int mode_num; // a number which is decimal after transforming from mode_array as a binary number
};
node* last_printed_node = NULL;
string_list stock_namelist;
string_list read_stock_list(string namelist);
void print_string_list(string_list whatever) {// this function is designed for debugging
	for (int i = 0; i < whatever.size(); i++)
		cout << whatever[i] << "                         " << i << endl;
}
string_list read_stock_list(string namelist) { // this function can also be used to collect timestamps from txt file.
	string_list result;
	const char* nl = namelist.data();
	ifstream file(nl, ios::in);
	string temp;
	while (getline(file, temp)) {
		stringstream ss;
		ss << temp;
		string ff;
		ss >> ff;
		result.push_back(ff);
	}
	file.close();
	//print_string_list(result);
	return result;
}
string_list_list get_all_timestamps(string_list namelist, mode M) {
	string_list_list result;
	if (M == NEG) {
		for (int i = 0; i < namelist.size(); i++)
			result.push_back(read_stock_list("./input0/DATE" + namelist[i] + ".txt"));
	}
	else {
		for (int i = 0; i < namelist.size(); i++)
			result.push_back(read_stock_list("./input1/DATE" + namelist[i] + ".txt"));
	}
	return result;
}
int datecmp(string a, string b) {
	const char* aa = a.data();
	const char* bb = b.data();
	return strcmp(aa, bb);
}
int comparing(string_list old, string_list now, int old_pt, int now_pt, string_list* result) {
	if (old_pt >= old.size() || now_pt >= now.size())
		return 463;
	//cout << old[old_pt] << "------------------------" << now[now_pt] << endl;
	int flag = datecmp(old[old_pt], now[now_pt]);
	if (flag == 0) {
		//cout << old[old_pt]  << "????" << endl;
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
void print_vector_to_txt(vector<string> whatever) {
	ofstream file("sector_new.txt", ios::app);
	for (int i = 0; i < whatever.size(); i++)
		file << whatever[i] << endl;
	file.close();
}
int get_degree_of_a_stock(int index, string_list_list sll0, string_list_list sll1) {
	int result = 0;
	string_list temp0 = sll0[index];
	string_list temp1 = sll1[index];
	cout << result << endl;
	for (int i = 0; i < sll0.size(); i++) {
		result += (compare(temp0, sll0[i]).size());
		result += (compare(temp1, sll0[i]).size());
		result += (compare(temp0, sll1[i]).size());
		result += (compare(temp1, sll1[i]).size());
		cout << result << endl;
	}
	return result - temp0.size() - temp1.size();
}
void main(void) {
	string_list stock_namelist = read_stock_list("sector.txt");
	//print_string_list(stock_namelist);
	string_list_list raw_data0 = get_all_timestamps(stock_namelist, NEG);
	string_list_list raw_data1 = get_all_timestamps(stock_namelist, POS);
	vector<int> degree_list;
	for (int i = 0; i < raw_data0.size(); i++) {
		//cout << get_degree_of_a_stock(i, raw_data0, raw_data1) << endl;
		cout << "Now dealing " << i << "/" << raw_data0.size() << endl;
		degree_list.push_back(get_degree_of_a_stock(i, raw_data0, raw_data1));
	}
	for (int i = 0; i < degree_list.size(); i++)
		cout << degree_list[i] << endl;

	for (int i = 0; i < degree_list.size(); i++) {
		for (int k = 0; k < degree_list.size() - i - 1; k++) {
			if (degree_list[k] < degree_list[k + 1]) {
				int temp_int = degree_list[k];
				string temp_string = stock_namelist[k];
				degree_list[k] = degree_list[k + 1];
				stock_namelist[k] = stock_namelist[k + 1];
				degree_list[k + 1] = temp_int;
				stock_namelist[k + 1] = temp_string;
			}
		}
	}
	for (int i = 0; i < degree_list.size(); i++)
		cout << degree_list[i] << endl;
	system("pause");
	print_vector_to_txt(stock_namelist);
}

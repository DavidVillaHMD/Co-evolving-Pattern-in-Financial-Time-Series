#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#define MAX_TIME 3650
#pragma warning(disable:4996)
using namespace std;
// 这个代码暂时还不会自己创建目录。。
////////////////////////extern variable///////////////////////////////////////////////////////////////////////////////////////
int total_evoving_timestamps = 0;
int total_timestamps = 0;
int pos_or_neg;   // 0 denotes we are mining negative evolving timestamps, 1 denotes mining positive.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct harr_wavelet {
	float average;
	int first;
	int last;
	float difference;
	int level;
};
struct segment {
	float changerate;
	int first;
	int last;
};
struct segment_array {
	segment* start;
	int length;
};
struct data_point {
	float slope;
	float weight;
	int first;
	int last;
	float average;
};
struct two {
	int index;
	float data;
};
struct tuple_array {
	two* start;
	int length;
};
struct data_point_array {
	data_point* start;
	int length;
};
class float_array { // because i dont know how to use vector now...
public:
	float* start;
	int length;
};
class int_array { // because i dont know how to use vector now...
public:
	int* start;
	int length;
};

class data_of_day {
public:
	char date[14];
	//float high;
	//float low;
	//float open;
	//float close;
	//float volume;
	float adj_close;
};
float max_2(float a, float b) {
	return (a >= b) ? a : b;
}
float min_2(float a, float b) {
	return (a <= b) ? a : b;
}
float max_float(float a, float b, float c) {
	if (a >= b && a >= c)
		return a;
	else if (b >= c)
		return b;
	else return c;
}
bool is_in(float a, float b, float c) {
	return (c >= a && c <= b) ? true : false;
}
harr_wavelet* preprocess(float* start, int length) {
	harr_wavelet* result = new harr_wavelet[length];
	for (int i = 0; i < length; i++) {
		harr_wavelet temp;
		temp.average = *(start + i);
		temp.first = i;
		temp.last = i;
		temp.difference = 0;
		temp.level = 0;
		*(result + i) = temp;
	}
	return result;
}
harr_wavelet fuse_2(harr_wavelet a, harr_wavelet b) {
	harr_wavelet result;
	result.average = 0.5*(a.average + b.average);
	result.first = a.first;
	result.last = b.last;
	result.difference = a.average - result.average;
	result.level = a.level + 1;
	return result;
}
harr_wavelet fuse_3(harr_wavelet a, harr_wavelet b, harr_wavelet c) {
	harr_wavelet result;
	result.average = (a.average + b.average + c.average) / 3;
	result.first = a.first;
	result.last = c.last;
	result.difference = max_float(a.average - result.average, b.average - result.average, c.average - result.average);
	result.level = a.level + 1;
	return result;
}
int* processing(harr_wavelet* preprocessed, int length, int max_level, float min_change_coefficient) {
	// in the paper, here is min_level, but we changed it(only order) 
	// ordinaryly we should judge if length is bigger than 1, but its unecessary because i am pretty sure each dataset's length
	// is at least 30.
	//for (int i = 0; i < length; i++)
		//cout << preprocessed[i].first << "  " << preprocessed[i].last << "  " << preprocessed[i].average << "  " << preprocessed[i].difference << "  " << preprocessed[i].level << endl;
	//system("pause");
	int now = length;
	harr_wavelet* temp = new harr_wavelet[2 * length];
	for (int i = 0; i < length; i++)
		*(temp + i) = *(preprocessed + i);
	harr_wavelet* write = temp + length;
	while (now > 1) {
		int new_now = now / 2;
		if (now % 2 == 1) {
			for (int i = 0; i < new_now - 1; i++)
				*(write + i) = fuse_2(*(write - now + 2 * i), *(write - now + 2 * i + 1));
			*(write + new_now - 1) = fuse_3(*(write - 3), *(write - 2), *(write - 1));
			write += new_now;
			now = new_now;
		}
		else {
			for (int i = 0; i < new_now; i++)
				*(write + i) = fuse_2(*(write - now + 2 * i), *(write - now + 2 * i + 1));
			write += new_now;
			now = new_now;
		}
	}
	///*
	cout << "Testing MODE." << endl;
	write--;
	while (write > temp) {
		cout << write->first << "  " << write->last << "  " << write->difference << "  " << write->average << "  " << write->level << endl;
		write--;
	}
	system("pause");
	//*/
	int* result = new int[length];
	for (int i = 0; i < length; i++)
		*(result + i) = 0;
	write--;
	for (int k = 0; write > temp + length - 1; write--, k++) {
		if (pos_or_neg == 0) {
			if ((write->level <= max_level) && (write->difference >= min_change_coefficient)) {
				for (int i = write->first; i <= write->last; i++)
					result[i] ++;
			}
		}
		else {
			if ((write->level <= max_level) && (write->difference <= min_change_coefficient*(-1))) {
				for (int i = write->first; i <= write->last; i++)
					result[i] ++;
			}
		}
	}
	delete[] temp;
	return result;
}

tuple_array get_evolving_intervals(float* start, int length) {
	harr_wavelet* preprocessed_data = preprocess(start, length);
	harr_wavelet* preprocessed = preprocessed_data;
	//for (int i = 0; i < length; i++)
		//cout << preprocessed[i].first << "  " << preprocessed[i].last << "  " << preprocessed[i].average << "  " << preprocessed[i].difference << "  " << preprocessed[i].level << endl;
	int* flag_array = processing(preprocessed_data, length, 4, 0.05);///5 denotes at most 32 continuing timestamps calculated once////
	int count = 0;//////////////////////改成4吧，求你了或者不重复搞？？？？？？？？？？？？？？
	for (int i = 0; i < length; i++)
		if (flag_array[i] > 0)
			count++;
	two* data_and_index_array = new two[count + 1];
	two* write = data_and_index_array;
	for (int i = 0; i < length; i++)
		if (flag_array[i] > 0) {////???????????????????????????????????????????????????????????????????????????/////
			two temp;
			temp.index = i;
			temp.data = preprocessed_data[i].average;
			*write = temp;
			write++;
		}
	tuple_array evolving_intervals;
	evolving_intervals.start = data_and_index_array;
	evolving_intervals.length = count;
	//cout << count << '/' << length << endl;
	total_evoving_timestamps += count;
	total_timestamps += length;
	delete[] preprocessed_data;
	delete[] flag_array;
	//for (int i = 0; i < evolving_intervals.length; i++)
		//cout << evolving_intervals.start[i].index << ' ' << evolving_intervals.start[i].data << endl;
	return evolving_intervals;
}
float_array read_data(string filename) {
	float_array result;
	result.start = new float[MAX_TIME];
	result.length = 0;
	ifstream input_data;
	input_data.open(filename, ios::in);
	string temp;
	int first_flag = 1;
	float first = 1;
	while (getline(input_data, temp)) {
		stringstream ss;
		ss << temp;
		float data;
		ss >> data;
		if (first_flag == 1) {
			first = data;
			first_flag = 0;
		}
		result.start[result.length] = data / first; // we record a rate comparing to the first element instead of its real value.
		result.length++;
		//cout << data / first << endl;
	}
	return result;
}
int_array read_data_for_int(string filename) {
	int_array result;
	result.start = new int[MAX_TIME];
	result.length = 0;
	ifstream input_data;
	input_data.open(filename, ios::in);
	string temp;
	while (getline(input_data, temp)) {
		stringstream ss;
		ss << temp;
		int data;
		ss >> data;
		result.start[result.length] = data; 
		result.length++;
		//cout << data / first << endl;
	}
	return result;
}

segment_array before_segment(tuple_array evolving_intervals) {
	segment* temp = new segment[evolving_intervals.length - 1];
	//cout << temp.changerate << "   " << temp.first << "   " << temp.last << endl;
	two* temp_two = evolving_intervals.start;
	int count = 0;
	for (int i = 0; i < evolving_intervals.length - 1; i++) {
		if (temp_two[i + 1].index - temp_two[i].index == 1) {
			count++;
			temp[i].changerate = (evolving_intervals.start[i + 1].data) - (evolving_intervals.start[i].data);
			temp[i].first = temp_two[i].index;
			temp[i].last = temp_two[i].index + 1;
		}
		else {
			temp[i].changerate = 100.0;
		}
	}
	segment* wtf = new segment[count];
	segment_array result;
	result.length = count;
	result.start = wtf;
	int needle = 0;
	for (int i = 0; i < evolving_intervals.length - 1; i++) {
		if (temp[i].changerate != 100.0) {
			result.start[needle] = temp[i];
			needle++;
		}
	}
	segment* ttemp = result.start;
	delete[] temp;
	return result;
}
int segsegseg(segment_array seg_data, int start, int end, segment** write_add, float threshold) {
	if (start > end)
		return 0;
	segment* ttemp = seg_data.start;
	int now = start;
	int flag = 1;
	float upbond = seg_data.start[now].changerate + threshold;
	float lowbond = seg_data.start[now].changerate - threshold;
	segment temp;
	temp.changerate = seg_data.start[now].changerate;
	temp.first = start;
	temp.last = now;
	while (flag == 1) {
		if (start == end) {
			*(*write_add) = temp;
			return 0;//0 denotes end
		}
		if ((is_in(lowbond, upbond, seg_data.start[now + 1].changerate) && seg_data.start[now + 1].first == seg_data.start[now].last)) {
			temp.last++;
			upbond = min_2(upbond, seg_data.start[now + 1].changerate + threshold);
			lowbond = max_2(lowbond, seg_data.start[now + 1].changerate - threshold);
			temp.changerate = (lowbond + upbond) / 2;
			now++;
		}
		else flag = 0;
	}
	//cout << temp.first << "  " << temp.last << "  " << temp.changerate << endl;
	*(*write_add) = temp;
	(*write_add)++;
	if (temp.last + 1 > end)
		return 0;
	return temp.last + 1;
}
segment_array segmentation(segment_array preprocessed_seg, float threshold) {
	//for (int i = 0; i < preprocessed_seg.length; i++)
		//cout << i << "  " << preprocessed_seg.start[i].first << " " << preprocessed_seg.start[i].last << "  " << preprocessed_seg.start[i].changerate << endl;
	segment* tempo = new segment[preprocessed_seg.length];
	segment* tempo_bro = tempo;
	segment** tttemp = &tempo_bro;
	//segment* write = tempo;
	int count = 0;
	int iter_flag = 0;
	do {
		iter_flag = segsegseg(preprocessed_seg, iter_flag, preprocessed_seg.length - 1, tttemp, threshold);
		count++;
	} while (iter_flag != 0);
	segment_array result;
	result.length = count;
	result.start = new segment[count];
	for (int i = 0; i < count; i++) {
		result.start[i].first = preprocessed_seg.start[tempo[i].first].first;
		result.start[i].last = preprocessed_seg.start[tempo[i].last].last;
		//if (result.start[i].last < 0 && result.start[i].first < 0)
		//cout << result.start[i].first << "  " << result.start[i].last << endl;
	}
	delete[] tempo;
	return result;
}

data_point_array segment_to_slope(segment_array sega, float_array data) {
	segment* temp = sega.start;
	data_point_array result;
	result.length = sega.length;
	result.start = new data_point[sega.length];
	data_point* tempp = result.start;
	for (int i = 0; i < sega.length; i++) {
		result.start[i].slope = (data.start[temp[i].last] - data.start[temp[i].first]) / (temp[i].last - temp[i].first);
		result.start[i].first = sega.start[i].first;
		result.start[i].last = sega.start[i].last;
		result.start[i].weight = result.start[i].last - result.start[i].first;
		//result.start[i].average = 
	}
	return result;
}
float bandwidth(data_point_array slope_array) {
	float sum = 0;
	float total = 0;
	float mean;
	for (int i = 0; i < slope_array.length; i++) {
		sum += slope_array.start[i].slope*slope_array.start[i].weight;
		total += slope_array.start[i].weight;
	}
	mean = sum / total;
	float sum_of_square = 0;
	for (int i = 0; i < slope_array.length; i++)
		sum_of_square += (slope_array.start[i].slope - mean)*(slope_array.start[i].slope - mean)*slope_array.start[i].weight;

	float standard_deviation = sqrt(sum_of_square / total);
	float result = 1.06*standard_deviation*pow(slope_array.length, -0.2);
	//https://en.wikipedia.org/wiki/Kernel_density_estimation
	cout << result << endl;
	return result;
}
float ski(data_point_array skier, float start_point, float radius) {
	float upbond = start_point + radius;
	float lowbond = start_point - radius;
	float sum = 0;
	float total = 0;
	for (int i = 0; i < skier.length; i++) {
		if (skier.start[i].slope >= lowbond && skier.start[i].slope <= upbond) {
			sum += skier.start[i].slope * skier.start[i].weight;
			total += skier.start[i].weight;
		}
	}
	float now = sum / total;
	if (now == start_point)
		return now;
	else
		return ski(skier, now, radius);
}
void calculate_average(data_point_array* sloper) {
	int len = sloper->length;
	float radius = bandwidth(*sloper) / 2;
	for (int k = 0; k < len; k++) {
		float key = ski(*sloper, sloper->start[k].slope, radius);
		for (int r = 0; r < len; r++)
			if (sloper->start[r].slope >= key - radius && sloper->start[r].slope <= key + radius)
				sloper->start[r].average = key;
	}
	data_point* tempp = sloper->start;
	//for(int i =0;i<len;i++)
		//cout << tempp[i].slope << "   " << tempp[i].first << "   " << tempp[i].last << "   " << tempp[i].weight << "   " << tempp[i].average << endl;
}
void output_session(string name, data_point_array finished, int minsupp) {
	ofstream file(name, ios::app);/////////////////////////////////////////////////alternate
	//for (int i = 0; i < finished.length; i++)
		//cout << finished.start[i].first << "  " << finished.start[i].last << "  " << finished.start[i].average << endl;
	float* temp = new float[finished.length];
	int len_of_result = 0;
	int now = 0;
	for (int i = 0; i < finished.length; i++) {
		int flag = 1;
		for (int k = 0; k <= now - 1; k++)
			if (finished.start[i].average == temp[k])
				flag = 0;
		if (flag == 0)
			continue;
		else {
			temp[now] = finished.start[i].average;
			now++;
		}
	}
	//for (int i = 0; i < now; i++)
		//cout << i << "  " << temp[i] << endl;
	for (int i = 0; i < now; i++) {
		int* gonnabe = new int[2 * finished.length];
		int write = 0;
		for (int k = 0; k < finished.length; k++) {
			if (abs(finished.start[k].average) > 10)
				continue;
			if (finished.start[k].average == temp[i]) {
				gonnabe[write] = finished.start[k].first;
				gonnabe[write + 1] = finished.start[k].last;
				write += 2;
			}
		}
		//cout << write <<" "<< temp[i] << endl;
		if (write / 2 >= minsupp) {
			for (int i = 0; i < write; i += 2) {
				for (int k = gonnabe[i]; k <= gonnabe[i + 1]; k++) {//////////////////was gonnabe[i + 1] - 1 here
					file << k << endl;
					len_of_result++;
				}
			}
		}
	}
	cout << "This file's result has" << len_of_result << "timestamps." << endl;
	delete[] temp;
}
void output_before_segmentation(tuple_array answer, string name) {
	if (answer.length == 0)
		return;
	int ending = name.find('_');
	string real_name = name.substr(0, ending);
	//ofstream file("./output/DATE"+real_name+".txt", ios::app);
	ofstream file0("./output0/DATE" + real_name + ".txt", ios::app);
	ofstream file1("./output1/DATE" + real_name + ".txt", ios::app);
	ifstream input_data;
	input_data.open("./input/" + name+"_DATE.txt", ios::in);
	string temp;
	int count = 0;
	vector<string> date_string;
	int date_string_is_null = 1;
	while (getline(input_data, temp)) {
		stringstream ss;
		ss << temp;
		string data;
		ss >> data;
		date_string.push_back(data);
		date_string_is_null = 0;
	}
	if (date_string_is_null == 1)
		return;
	if (pos_or_neg == 1) {
		for (int i = 0; i < answer.length; i++) {
			file1 << date_string[answer.start[i].index] << endl;
		}
	}
	else {
		for (int i = 0; i < answer.length; i++) {
			file0 << date_string[answer.start[i].index] << endl;
		}
	}
}
void main_process(string name, float max_error = 0.18, int min_supp_of_intervals = 65) {
	float_array data_array = read_data("./input/"+name + ".txt");
	tuple_array answer = get_evolving_intervals(data_array.start, data_array.length);
	output_before_segmentation(answer, name);
	/*            THIS SECTION IS THE OLD VERSION CONTAINS SEGMENTATION
	segment_array seg_array = before_segment(answer);
	segment_array seg_array1 = segmentation(seg_array, max_error);
	data_point_array slope_array = segment_to_slope(seg_array1, data_array);
	calculate_average(&slope_array);
	output_session("S" + name + ".txt", slope_array, min_supp_of_intervals);
	*/
}
int main(void) {
	pos_or_neg = 1;
	/*
	ifstream file("gagaga.txt", ios::in);
	string temp;

	while (getline(file, temp)) {
		cout << temp << endl;
		//main_process(temp);
		main_process("000002_1");
		break;
	}
	file.close();
	*/
	while (pos_or_neg >= 0) {
		int test_mode = 1; // 0 denotes not test mode
		if (test_mode == 0) {
			int_array num_array = read_data_for_int("./input/estate_num.txt");
			int num = 0;
			ifstream file("./input/sector.txt", ios::in);
			string temp;
			while (getline(file, temp)) {
				cout << temp << endl;
				for (int i = 1; i <= num_array.start[num]; i++) {
					string m = temp + "_";
					m += to_string(i);
					main_process(m);
				}
				cout << total_evoving_timestamps << "/" << total_timestamps << endl;
				total_evoving_timestamps = 0;
				total_timestamps = 0;
				num++;
			}
			file.close();
		}
		else {
			for (int i = 17; i <= 17; i++) {
				string m = "nlmy_";
				m += to_string(i);
				main_process(m);
			}
		}
		pos_or_neg--;
	}
	system("pause");
	return 0;
}

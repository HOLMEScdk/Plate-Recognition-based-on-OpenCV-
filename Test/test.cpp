#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include<map>
#include <stdio.h>
using namespace cv;
using namespace std;
#define DEFINE_THUMB_IMGW 32//ָ�ƿ�/��
#define DEFINE_THUMB_IMGH 32
#define DEFINE_INT_SIZE (DEFINE_THUMB_IMGW * DEFINE_THUMB_IMGH / 32)
std::map<std::string, int[DEFINE_INT_SIZE]> map_data;
std::map<std::string, int[DEFINE_INT_SIZE]>::iterator map_data_iter;
char ANSWER[36] = { 0 };
int ii = 0;
int hammingDistance(int x, int y) {
	int count = 0;
	for (int i = 0; i < 32; ++i) {//�ж���ͬλ��
		if ((x & 0x1) != (y & 0x1))
			count++;
		x = x >> 1;
		y = y >> 1;
	}
	return count;
}
bool get_imgthumb(int* _inout_imgthumb, int& _out_filesize, char* _in_filename, int x) {
	if (_inout_imgthumb == 0 || _in_filename == 0)
		return false;
	_out_filesize = 0;
	memset(_inout_imgthumb, 0, DEFINE_THUMB_IMGW * DEFINE_THUMB_IMGH / 8);
	//////////////////////////////////////////////////////////////////////////
	IplImage* image = cvLoadImage(_in_filename, 0);
	if (x == 1) {
		cvAdaptiveThreshold(image, image, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 59);
		//cvThreshold(image, image, 105, 255, CV_THRESH_BINARY_INV);
		//cvSaveImage("data\\model36.jpg", image);
	}
	if (!image)
		return false;
	//cout << "1" << endl;

	IplImage* image_scale = cvCreateImage(CvSize(DEFINE_THUMB_IMGW, DEFINE_THUMB_IMGH), IPL_DEPTH_8U, 1);
	cvZero(image_scale);
	_out_filesize = image->width * image->height;
	cvResize(image, image_scale, 1);
	unsigned char* p_gray = (unsigned char*)image_scale->imageData;
	//���������������ƽ��ֵ
	unsigned int i_sum = 0;
	for (int i = 0; i < DEFINE_THUMB_IMGW * DEFINE_THUMB_IMGH; ++i) {
		i_sum += p_gray[i];
	}
	i_sum = i_sum / (DEFINE_THUMB_IMGW * DEFINE_THUMB_IMGH);
	//�Ƚ����صĻҶ�
	//��ÿ�����صĻҶȣ���ƽ��ֵ���бȽϡ����ڻ����ƽ��ֵ����Ϊ1��С��ƽ��ֵ����Ϊ0��
	//char c_hash[16];//�����ϣֵ
	for (int i = 0; i < DEFINE_THUMB_IMGW * DEFINE_THUMB_IMGH; ++i)
	{
		int o = i / 32;
		int i0 = i % 32;
		if (p_gray[i] >= i_sum) {
			_inout_imgthumb[o] = _inout_imgthumb[o] | (1 << i0);
		}
	}
	// 
	// 	//////////////////////////////////////////////////////////////////////////
	cvReleaseImage(&image);
	cvReleaseImage(&image_scale);
	return true;
}
void init_char() {
	for (int i = 0; i <= 36; i++) {
		if (i <= 9) {
			ANSWER[i] = i + '0';
		}
		else if (i>9 && i <= 35) {
			ANSWER[i] = (i - 10) + 'A';
		}
		else {
			ANSWER[i] = '-1';
		}
	}
}
void init_template() {//template initial
	char filename[100];
	int i_filesize = 0;
	for (int i = 0; i <= 36; i++) {
		sprintf_s(filename, "data\\model%d.jpg", i);
		int i_imgthumb[DEFINE_INT_SIZE];
		get_imgthumb(i_imgthumb, i_filesize, filename, 0);
		for (int p = 0; p < DEFINE_INT_SIZE; ++p) {
			stringstream ss;
			string temp;
			ss << "model" << i << ".jpg";
			ss >> temp;
			map_data[temp][p] = i_imgthumb[p];
		}
	}
}
void detect_char(IplImage *image, int j) {
	int i = 0;
	int i_imgthumb_this[DEFINE_INT_SIZE];
	int i_filesize = 0;
	cvThreshold(image, image, 105, 255, CV_THRESH_BINARY_INV);
	//cvShowImage("Look", image);
	//Testing pictures
	char filename1[100];
	sprintf_s(filename1, "submit\\split_%d_%d.jpg", ii, j);
	int height = image->height;
	int width = image->width;
	//cout << height << " " << width << endl;
	if (width < 20 && j <= 1)return;
	get_imgthumb(i_imgthumb_this, i_filesize, filename1, 1);
	map<std::string, int> map_value;
	map<string, int>::iterator iter;
	for (map_data_iter = map_data.begin(); map_data_iter != map_data.end(); ++map_data_iter) {
		int count = 0;
		int count_invert = 0;
		for (int i = 0; i < DEFINE_INT_SIZE; ++i) {
			int v = map_data_iter->second[i];
			int dv = i_imgthumb_this[i];
			int dv_invert = ~dv;
			count += hammingDistance(dv, v);
			count_invert += hammingDistance(dv_invert, v);
		}
		map_value[map_data_iter->first] = min(count, count_invert);
	}
	//Mininum
	int value_min = 0;
	int number = 0, index = 0, flag = 1;
	string filename_min = "";
	for (iter = map_value.begin(); iter != map_value.end(); ++iter) {
		//cout << iter->first << " " << iter->second << endl;
		if ((flag&&value_min == 0) || value_min > iter->second) {
			number = 0;//���Լ�Ϊ5��ʱ�� ��⵽R
			value_min = iter->second;
			filename_min = iter->first;
			flag = 0;//��һ�ν���
			for (int i = 0; i < filename_min.size(); i++) {
				if (filename_min[i] <= '9'&&filename_min[i] >= '0') {
					number = number * 10 + filename_min[i] - '0';
				}
			}
		}
	}
	//cout << "ModelPicture: " << filename_min << endl;
	if (number != 36) {
		//cout << "This world is " << ANSWER[number] << endl;
		cout << ANSWER[number];
	}
	else {
		cout << "��";
	}
}
vector<Mat> verticalProjectionMat(Mat srcImg)//��ֱͶӰ
{
	Mat binImg;
	//int from = srcImg.cols-429;
	//int end = srcImg.rows-100;
	cvtColor(srcImg, binImg, CV_BayerRG2GRAY);//��ת�Ҷ�ͼ ��Ҫ��ԭͼ������н���
											  //imshow("gray", binImg);
	blur(binImg, binImg, Size(3, 3));
	//imshow("blur", binImg);
	//threshold(binImg, binImg, 100, 255, THRESH_BINARY_INV);//�õ���ֵͼ �ؼ��ڶ�ֵͼ�Ĵ�������173�ı��0��Ϊ��reverse������
	adaptiveThreshold(binImg, binImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 119, -25);
	//imshow("thre", binImg);
	//imwrite("submit\\thre.jpg", binImg);
	int perPixelValue;//ÿ�����ص�ֵ
	int width = srcImg.cols;
	int height = srcImg.rows;
	int* projectValArry = new int[width];//�������ڴ���ÿ�а�ɫ���ظ���������
	memset(projectValArry, 0, width * 4);//��ʼ������
	int sum = 0;
	for (int col = 0; col < width; col++)//��
	{
		projectValArry[col] = 0;
		for (int row = 0; row < height; row++)//��
		{
			perPixelValue = binImg.at<uchar>(row, col);
			//cout << perPixelValue << endl;
			if (perPixelValue == 0)//����ǰ׵׺��� ������ ����ʾ���ֵ���һ��ֱ��ͼ�ķֲ�
			{
				projectValArry[col]++;

			}
		}
		sum += projectValArry[col];
	}
	sum /= width;
	//for (int i = 0; i <width; i++) {
	//	cout << projectValArry[i] << endl;
	//}
	//cout << "Average:" << sum << endl;
	Mat verticalProjectionMat(height, width, CV_8UC1);//��ֱͶӰ�Ļ���
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			perPixelValue = 255;  //��������Ϊ��ɫ
			verticalProjectionMat.at<uchar>(i, j) = perPixelValue;
		}
	}
	for (int i = 0; i < width; i++)//��ֱͶӰֱ��ͼ
	{
		for (int j = 0; j < projectValArry[i]; j++)
		{
			perPixelValue = 0;  //ֱ��ͼ����Ϊ��ɫ  
			verticalProjectionMat.at<uchar>(height - 1 - j, i) = perPixelValue;
		}
	}
	//imshow("��ֱͶӰ", verticalProjectionMat);
	//imwrite("ͶӰ.jpg", verticalProjectionMat);
	//cvWaitKey(0);
	vector<Mat> roiList;//���ڴ���ָ������ÿ���ַ�
	int startIndex = 0;//��¼�����ַ���������
	int endIndex = 0;//��¼����հ����������
	bool inBlock = false;//�Ƿ���������ַ�����
	int cnt = 0;
	for (int i = 0; i < srcImg.cols; i++) {
		if (projectValArry[i] == 0)cnt++;
	}
	//cout << "CNT:" << cnt << endl;
	//block ����ǲ�������֮�� projetҲ��������� �������Ը�������indexȷ�������С
	int firs_two = 0, sum2 = 0;
	cnt = 0;
	for (int i = 0; i < srcImg.cols; i++)//cols=width
	{
		if (!inBlock && projectValArry[i] != 0 && projectValArry[i] > sum)//�����ַ���
		{
			inBlock = true;
			startIndex = i;
		}
		else if (inBlock&&projectValArry[i] < sum*0.5)//����հ���
		{
			endIndex = i;
			inBlock = false;
			firs_two++;
			//if (firs_two > 2) {
			//cout << "Width:" << endIndex + 1 - startIndex << endl;
			sum2 += endIndex + 1 - startIndex;
			cnt++;
		}
	}
	sum2 /= cnt;
	//cout << "Limit_SUM2:" << sum2 << endl;
	for (int i = 0; i < srcImg.cols; i++)//cols=width
	{
		if (!inBlock && projectValArry[i] != 0 && projectValArry[i]>sum)//�����ַ���
		{
			inBlock = true;
			startIndex = i;
		}
		else if (inBlock&&projectValArry[i]<sum*0.3)//����հ���
		{
			endIndex = i;
			inBlock = false;
			firs_two++;
			//if (firs_two > 2) {
			//cout << "Width:" << endIndex + 1 - startIndex << endl;
			if (endIndex + 1 - startIndex >= sum2 - 5 || endIndex + 1 - startIndex >= 10) {
				Mat roiImg = srcImg(Range(0, srcImg.rows), Range(startIndex, endIndex + 1));
				roiList.push_back(roiImg);
			}
		}
	}
	delete[] projectValArry;
	return roiList;
}
int main(int argc, char* argv[]) {
	init_char();
	init_template();
	//�Ƚ��д�ֱͶӰ���ٽ���ˮƽͶӰ�ָ��ַ���
	for (ii = 2; ii <= 121; ii++) {
		char srcname[100] = { 0 };
		sprintf_s(srcname, "SPLIT_PLATE\\plate_%d.jpg", ii);
		Mat srcImg = imread(srcname, 0);//����ԭͼ��
		if (srcImg.empty()) {
			//cout << "JUMP\n";
			continue;
		}
		char szName[30] = { 0 };
		vector<Mat> b = verticalProjectionMat(srcImg);//�Ƚ��д�ֱͶӰ	
													  //��ֱͶӰ�Ժ�Ľ��
													  //cout << b.size() << endl;
		cout << ii << ":";
		for (int j = 0; j < b.size(); j++) {
			sprintf(szName, "submit\\split_%d_%d.jpg", ii, j);
			/*if(i==121)
			imshow(szName, b[j]);*/
			IplImage img = IplImage(b[j]);
			cvSaveImage(szName, &img);//������
			detect_char(&img, j);
		}
		cout << endl;

	}
	cout << "DONE\n";
	while (true) {
		if (waitKey(30) == 27)break;
	}
	return 0;
}

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include<string>

#include "config.h"
#include "Calibrate.h"
#include "PairAlign.h"

float config_num[50];

void ArgvConfig()
{
	Get_ConfigNum();
	board_size.width = (int)(config_num[0]); // 6;
	board_size.height = (int)(config_num[1]); // 9;
	square_size.width = config_num[2]; // 15;
	square_size.height = config_num[3]; // 15;
	Registration_flag = (int)(config_num[4]);
	Camera_ID == (int)(config_num[5]); // 1;
	KSearchnum = (int)(config_num[6]); // 
	MaxCorrespondenceDistance = config_num[7]; //��Ӧ��֮��������루0.1��, ����׼�����У����Դ��ڸ���ֵ�ĵ�
	LeafSize = config_num[8]; 
	TransformationEpsilon = config_num[9];//����������
	downsample_flag = (bool)(config_num[10]);
	Cali_Pic_Num = (int)(config_num[11]);
	GetRough_T_flag = (int)(config_num[12]);
	for (int i = 0; i < 12; i++)
	{
		GetR360Rough_T(i/4, i%4) = config_num[i+13];
	}
}

void Get_ConfigNum()
{
	std::string a[50];              //���� string ���ͣ���100�е��ı�����Ҫ������ 

	int i = 0;
	std::ifstream infile;

	//try
	//{
	infile.open("../config/config.txt", std::ios::in);
	//}
	

	while (!infile.eof())            // ��δ���ļ�����һֱѭ�� 
	{
		getline(infile, a[i], '\n');//��ȡһ�У��Ի��з����������� a[] ��
		i++;                    //��һ��
	}
	for (int ii = 0; ii<i; ii++)        // ��ʾ��ȡ��txt���� 
	{
		std::cout << a[ii] << std::endl;
		std::string::size_type position = a[ii].find(":");
		a[ii]=a[ii].substr(position+1);
		config_num[ii] = std::atof(a[ii].c_str());
	}
	infile.close();
}
// �������̨Ӧ�ó������ڵ㡣
//�ص���Microsoft���ŵ�����
//
// Created by �쿭 on 17/4/4.
//
#include "stdafx.h"
#include "Calibrate.h"
#include "PairAlign.h"
#include "config.h"

#include <pcl/visualization/cloud_viewer.h>  
#include <iostream>  
#include <pcl/io/io.h>  
#include <pcl/io/pcd_io.h>  

#include <cv.h>
#include <highgui.h>
#include<stdio.h>
#include <string.h>
int _tmain(int argc, char** argv)
{
	unsigned short int total_cloud = 0;
	cout << "Please Input Number of cloud files (Max: 18):";//��ʾ�û�����
	cin >> total_cloud;//����Num_Of_Ply
	while (true){
		if (total_cloud <= 0 && total_cloud >= 19)
		{
			std::cout << "Number of clouds ranges from 1 to 18." << endl;
			cout << "Please Input Number of cloud files (Max: 18):";//��ʾ�û�����
			cin >> total_cloud;//����Num_Of_Ply
		}
		else{
			switch (total_cloud)
			{
			case 18:{
				char *str18 = "18_point_cloud.ply";
				argv[18] = str18;
			}
			case 17:{
				char *str17 = "17_point_cloud.ply";
				argv[17] = str17;
			}
			case 16:{
				char *str16 = "16_point_cloud.ply";
				argv[16] = str16;
			}
			case 15:{
				char *str15 = "15_point_cloud.ply";
				argv[15] = str15;
			}
			case 14:{
				char *str14 = "14_point_cloud.ply";
				argv[14] = str14;
			}
			case 13:{
				char *str13 = "13_point_cloud.ply";
				argv[13] = str13;
			}
			case 12:{
				char *str12 = "12_point_cloud.ply";
				argv[12] = str12;
			}
			case 11:{
				char *str11 = "11_point_cloud.ply";
				argv[11] = str11;
			}
			case 10:{
				char *str10 = "10_point_cloud.ply";
				argv[10] = str10;
			}

			case 9:{
				char *str9 = "9_point_cloud.ply";
				argv[9] = str9;
			}
			case 8:{
				char *str8 = "8_point_cloud.ply";
				argv[8] = str8;
			}
			case 7:{
				char *str7 = "7_point_cloud.ply";
				argv[7] = str7;
			}
			case 6:{
				char *str6 = "6_point_cloud.ply";
				argv[6] = str6;
			}
			case 5:{
				char *str5 = "5_point_cloud.ply";
				argv[5] = str5;
			}
			case 4:{
				char *str4 = "4_point_cloud.ply";
				argv[4] = str4;
			}
			case 3:{
				char *str3 = "3_point_cloud.ply";
				argv[3] = str3;
			}
			case 2:{
				char *str2 = "2_point_cloud.ply";
				argv[2] = str2;
			}
			case 1:{
				char *str1 = "1_point_cloud.ply";
				argv[1] = str1;
			}
			default: {
				std::cout << "[Log: Status]: .ply Defined Successfully." << endl;
				break;
			}
			}
			break;
		}
	}
	
	
	//std::cout << "[Log:Debug]: .ply Defined Successfully." << endl;
	//std::cout << "[Log:Debug]: Lost File : ./config/config.txt " << endl;

	ArgvConfig();					// ../config/config.txt �ļ�ȱʧ

	std::cout << "[Log: Status]: .ply Configurd Successfully." << endl;
	argc = total_cloud+1;

	//��ȡ����
	//std::cout << "[Log:Debug]: Start Reading Model Datas" << endl;
	std::vector<PCD, Eigen::aligned_allocator<PCD> > data; //ģ��
	std::cout << "[Log: Status]: Model Datas Read Successfully." << endl;

	//std::cout << "[Log:Debug]: Start Reading PCD Files" << endl;
	loadData(argc, argv, data); //��ȡpcd�ļ����ݣ����������	
	std::cout << "[Log: Status]: PCD Files Read Successfully." << endl;

	//std::cout << "[Log:Debug]: CameraParam Configuring." << endl;
	inputCameraParam(intrinsic_matrix, distortion_coeffs, Cam_extrinsic_matrix, Pro_extrinsic_matrix);//���ļ��ж�ȡ�������
	std::cout << "[Log: Status]: CameraParam Configured Successfully." << endl;

	if (GetRough_T_flag == 1)
	{
		//std::cout << "[Log:Debug]: Programe Enter : if (GetRough_T_flag == 1)" << endl;
		//std::cout << "[Log:Debug]: Start Calculating Mat" << endl;
		find_rotation_mat();//���ÿ���궨ͼ�����α任�������ȫ�ֱ���T_mat_4x4��
		std::cout << "[Log: Status]: Homogeneous transformation Mat Calculated Successfully." << endl;
	}
	std::cout << "[Log: Status]: Start Stitching!" << endl;
	AccurateRegistration2(data);//��ϸƴ��

	return 0;
}


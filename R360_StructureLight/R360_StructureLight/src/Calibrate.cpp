
#include "stdafx.h"
#include "Calibrate.h"
#include <highgui.h> 
#include "cv.h"
#include <iostream> 
#include "opencv2/opencv.hpp"

#include <Windows.h>
#include <stdlib.h>
using namespace std;

CvSize board_size = cvSize(7, 10);   //�궨��ǵ���
CvSize2D32f square_size = cvSize2D32f(10, 10);                //���񳤿�

int Camera_ID = 1;
int Cali_Pic_Num;

CvMat * intrinsic_matrix = cvCreateMat(3, 3, CV_64FC1);                //�ڲ�������
CvMat * distortion_coeffs = cvCreateMat(5, 1, CV_64FC1);        //����ϵ��
CvMat * Cam_extrinsic_matrix = cvCreateMat(4, 4, CV_32FC1);    //�������ϵ��ͶӰ��
CvMat * Pro_extrinsic_matrix = cvCreateMat(4, 4, CV_32FC1);    //��������ϵ��ͶӰ��

vector<CvMat> T_mat_4x4;					//��ת����

void inputCameraParam(CvMat * intrinsic_matrix1, CvMat * distortion_coeffs1, CvMat * Cam_extrinsic_matrix1, CvMat * Pro_extrinsic_matrix1)
{
	CvMat * rotation_vec = cvCreateMat(3, 1, CV_32FC1);                //��ת����
	CvMat * translation_vec = cvCreateMat(3, 1, CV_32FC1);        //ƽ�ƾ���
	CvMat *temp = cvCreateMat(2, 3, CV_64FC1);        

	//��ȡ�������
	CvFileStorage *fs;
	fs = cvOpenFileStorage("..//calibrationXML//camera.xml", 0, CV_STORAGE_READ);
	if (fs)
	{
		*intrinsic_matrix1 = *cvCloneMat((CvMat *)cvReadByName(fs, NULL, "intrinsic"));
		*distortion_coeffs1 = *cvCloneMat((CvMat *)cvReadByName(fs, NULL, "distortion"));//����������������������ͷ�
		*temp = *cvCloneMat((CvMat *)cvReadByName(fs, NULL, "extrinsic"));
		cvReleaseFileStorage(&fs);
	}
	else
	{
		cout << "Error: can not find the intrinsics!" << endl;
	}

	for (int i = 0; i < 3; i++)
	{
		CV_MAT_ELEM(*rotation_vec, float, i, 0) = CV_MAT_ELEM(*temp, double, 0, i);
		CV_MAT_ELEM(*translation_vec, float, i, 0) = CV_MAT_ELEM(*temp, double, 1, i);
	}

	caculate_Tmat(rotation_vec, translation_vec, Cam_extrinsic_matrix1);

	//��ȡͶӰ�ǲ���
	CvFileStorage *fs1;
	fs1 = cvOpenFileStorage("..//calibrationXML//projector.xml", 0, CV_STORAGE_READ);
	if (fs1)
	{
		*temp = *cvCloneMat((CvMat *)cvReadByName(fs1, NULL, "extrinsic"));
		cvReleaseFileStorage(&fs1);
	}
	else
	{
		cout << "Error: can not find the intrinsics!" << endl;
	}

	for (int i = 0; i < 3; i++)
	{
		CV_MAT_ELEM(*rotation_vec, float, i, 0) = CV_MAT_ELEM(*temp, double, 0, i);
		CV_MAT_ELEM(*translation_vec, float, i, 0) = CV_MAT_ELEM(*temp, double, 1, i);
	}
	caculate_Tmat(rotation_vec, translation_vec, Pro_extrinsic_matrix1);

	cv::Mat a;
	a = intrinsic_matrix1;
	std::cout << "intrinsic:"<<endl;
	std::cout << a << endl;

	a = distortion_coeffs1;
	std::cout << "distortion:" << endl;
	std::cout << a << endl;

	a = Cam_extrinsic_matrix;
	std::cout << "Cam_extrinsic:" << endl;
	std::cout << a << endl;

	a = Pro_extrinsic_matrix;
	std::cout << "Pro_extrinsic_matrix:" << endl;
	std::cout << a << endl;

	cvReleaseMat(&rotation_vec);
	cvReleaseMat(&translation_vec);
	cvReleaseMat(&temp); 
}

void caculate_Tmat(CvMat * r_vec, CvMat * t_vec, CvMat * T_mat)
{
	CvMat * r_mat = cvCreateMat(3, 3, CV_32FC1);
	cvRodrigues2(r_vec, r_mat);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			CV_MAT_ELEM(*T_mat, float, i, j) = CV_MAT_ELEM(*r_mat, float, i, j);
			CV_MAT_ELEM(*T_mat, float, 3, j) = 0;
		}
		CV_MAT_ELEM(*T_mat, float, i, 3) = CV_MAT_ELEM(*t_vec, float, i, 0);
	}
	CV_MAT_ELEM(*T_mat, float, 3, 3) = 1;

	cvReleaseMat(&r_mat);
}

void rotate_R360Plant(unsigned int n)
{
	HANDLE hcom;
	hcom = CreateFile(_T("COM5"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hcom == INVALID_HANDLE_VALUE)
	{
		std::cout<<"Connection failed."<<endl;
	}
	SetupComm(hcom, 1024, 1024);
	DCB dcb;
	GetCommState(hcom, &dcb);
	dcb.BaudRate = 4800;
	dcb.ByteSize = 8;
	dcb.Parity = 0;
	dcb.StopBits = 1;
	SetCommState(hcom, &dcb);

	char data[1];
	data[0] = (char)n;
//	data[1] = 'a';

	DWORD dwWrittenLen = 0;
	if (!WriteFile(hcom, data, 1, &dwWrittenLen, NULL))
	{
		std::cout << "Failed to send "<<endl;
	}

	_sleep(1000);

	char str[2];
	char aa, bb;
	DWORD wCount;//��ȡ���ֽ���		 
	BOOL bReadStat;
	bReadStat = ReadFile(hcom, str, 1, &wCount, NULL);
	if (!bReadStat)
	{
		std::cout << "Read failed "<<endl;
	}
	aa = str[0];
	//	bb=str[1];
	std::cout << "receive: "<<aa;
	//	dlp::CmdLine::Print("����:",bb);

	CloseHandle(hcom);  //�ر�ͨѶ�˿�

}



int find_rotation_mat()
{
	char  cali_flag;
	char t[10];
	cout << "��Calibration��: 1    ��stitching��: 2 " << endl;
	cin >> cali_flag;

	cout << "Enter how many rotations per round:" << endl;
	cin >> t;


	int number_image = 1;
	char *str1;
	str1 = ".jpg";
	char filename[20] = "";

	float square_length = square_size.width;                //���񳤶�
	float square_height = square_size.height;                //����߶�
	int board_width = board_size.width;   //ÿ�нǵ���
	int board_height = board_size.height;  //ÿ�нǵ���
	int total_per_image = board_width*board_height;  //ÿ��ͼƬ�ǵ�����

	if (cali_flag == '1')
	{
		CvCapture* capture;
		capture = cvCreateCameraCapture(Camera_ID);

		if (capture == 0)
		{
			printf("Unable to capture camera device!\n\n");
			return 0;
		}
		else
		{
			printf("Capture camera device successfully! \n\n");
		}

		IplImage* frame = NULL;

		cvNamedWindow("Camera frame capture window", 1);

		printf("Press ��C�� to capture the current frame and save it as the calibration picture ... \n Press ��Q�� to exit the process of capturing frames ...\n\n");


		while (true)
		{
			frame = cvQueryFrame(capture);
			if (!frame)
				break;
			cvShowImage("Camera frame capture window", frame);

			if (cvWaitKey(10) == 'c')
			{
				sprintf_s(filename, "..\\imgs\\%d.jpg", number_image);
				cvSaveImage(filename, frame);
				cout << "Successfully get the current frame with the file name " << filename << " Saveing...\n\n";
				printf("Press ��C�� to capture the current frame and save it as the calibration picture ... \n Press ��Q�� to exit the process of capturing frames ...\n\n");
				number_image++;
				rotate_R360Plant(atoi(t));
			}
			else if (cvWaitKey(10) == 'q')
			{
				printf("The process of capturing image frames is complete ...\n\n");
				cout << "A total of " << --number_image << " frames were successfully captured!\n\n";
				break;
			}
		}

		cvDestroyWindow("Camera frame capture window");
		//	cvReleaseImage(&frame);
		cvReleaseCapture(&capture);
	}
	else if (cali_flag == '2')
	{
		number_image = Cali_Pic_Num;
	}

	IplImage * show;
	cvNamedWindow("RePlay", 1);
	std::cout << "RePlay Windows Create Successful!" << endl;

	int number_image_copy = number_image;  //����ͼ��֡��

	int count;  //�洢ÿ֡ͼ����ʵ��ʶ��Ľǵ���
	int found;        //ʶ��궨��ǵ�ı�־λ
	int step;        //�洢������step=successes*total_per_image;
	int successes = 0;        //�洢�ɹ��ҵ��궨�������нǵ��ͼ��֡��
	int a = 1;        //��ʱ��������ʾ�ڲ�����a֡ͼ��


	CvPoint2D32f * image_points_buf = new CvPoint2D32f[total_per_image];   //�洢�ǵ�ͼ�����������
	CvMat * image_points = cvCreateMat(number_image*total_per_image, 2, CV_32FC1);        //�洢�ǵ��ͼ������ľ���                
	CvMat * object_points = cvCreateMat(number_image*total_per_image, 3, CV_32FC1);        //�洢�ǵ����ά����ľ���
	CvMat * point_counts = cvCreateMat(number_image, 1, CV_32SC1);                //�洢ÿ֡ͼ���ʶ��Ľǵ���

	while (a <= number_image_copy)
	{
		sprintf_s(filename, "..\\imgs\\%d.jpg", a);
	
		show = cvLoadImage(filename, -1);

		found = cvFindChessboardCorners(show, board_size, image_points_buf, &count,
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
		if (found == 0)
		{
			cout << "The" << a << "frame of the picture cannot find all the corners of the checkerboard\n\n";
			cvNamedWindow("RePlay", 1); 
			cvShowImage("RePlay", show);
			cvWaitKey(0);

		}
		else
		{
			cout << "The" << a << "frame image successfully obtained " << count << " vertexs.\n";

			cvNamedWindow("RePlay", 1);

			IplImage * gray_image = cvCreateImage(cvGetSize(show), 8, 1); 
			cvCvtColor(show, gray_image, CV_BGR2GRAY);
			cout << "The process of obtaining the grayscale image of the source image is complete...\n";
			cvFindCornerSubPix(gray_image, image_points_buf, count, cvSize(11, 11), cvSize(-1, -1),
				cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			cout << "The process of subpixelization of grayscale image is complete...\n";
			cvDrawChessboardCorners(show, board_size, image_points_buf, count, found);
			cout << "The process of drawing vertexs on the source image is complete...\n\n";
			cvShowImage("RePlay", show);

			cvWaitKey(0);
		}

		if (total_per_image == count)//�ҵ������нǵ�
		{
			step = successes*total_per_image;
			for (int i = step, j = 0; j < total_per_image; ++i, ++j)
			{
				CV_MAT_ELEM(*image_points, float, i, 0) = image_points_buf[j].x;
				CV_MAT_ELEM(*image_points, float, i, 1) = image_points_buf[j].y;
				CV_MAT_ELEM(*object_points, float, i, 0) = (float)((j / board_width)*square_length);//��
				CV_MAT_ELEM(*object_points, float, i, 1) = (float)((j%board_width)*square_height);//��
				CV_MAT_ELEM(*object_points, float, i, 2) = 0.0f;
			}
			CV_MAT_ELEM(*point_counts, int, successes, 0) = total_per_image;
			successes++;
		}
		a++;
	}

	std::cout << "[DebugLog]: run to line Suspected" << endl;
	CvSize Pic_size = cvGetSize(show);
	std::cout << "[DebugLog]: God , this line is not a bug!" << endl;
	cvReleaseImage(&show);
	cvDestroyWindow("RePlay");


	cout << "*********************************************\n";
	cout << number_image << "In the frame picture, the successfully calibrated picture is " << successes << " frame...\n";
	cout << number_image << "In the frame picture, the successfully calibrated picture is " << number_image - successes << " frame...\n\n";
	cout << "*********************************************\n\n";

	CvMat * object_points2 = cvCreateMat(total_per_image, 3, CV_32FC1);
	CvMat * image_points2 = cvCreateMat(total_per_image, 2, CV_32FC1);
	CvMat * point_counts2 = cvCreateMat(successes, 1, CV_32SC1);

	CvMat * rotation_vec = cvCreateMat(3, 1, CV_32FC1);                //��ת����
	CvMat * translation_vec = cvCreateMat(3, 1, CV_32FC1);        //ƽ�ƾ���
	CvMat * T_Mat = cvCreateMat(4, 4, CV_32FC1);        //ƽ�ƾ���

	for (int j = 0; j < successes; ++j)
	{
		for (int i = 0; i < total_per_image; ++i)
		{
			CV_MAT_ELEM(*image_points2, float, i, 0) = CV_MAT_ELEM(*image_points, float, i + j*total_per_image, 0);
			CV_MAT_ELEM(*image_points2, float, i, 1) = CV_MAT_ELEM(*image_points, float, i + j*total_per_image, 1);
			CV_MAT_ELEM(*object_points2, float, i, 0) = CV_MAT_ELEM(*object_points, float, i + j*total_per_image, 0);
			CV_MAT_ELEM(*object_points2, float, i, 1) = CV_MAT_ELEM(*object_points, float, i + j*total_per_image, 1);
			CV_MAT_ELEM(*object_points2, float, i, 2) = CV_MAT_ELEM(*object_points, float, i + j*total_per_image, 2);
		}
		CV_MAT_ELEM(*point_counts2, int, j, 0) = CV_MAT_ELEM(*point_counts, int, j, 0);

		cvFindExtrinsicCameraParams2(object_points2, image_points2, intrinsic_matrix, distortion_coeffs, rotation_vec, translation_vec);
		caculate_Tmat(rotation_vec, translation_vec, T_Mat);//������α任
		T_mat_4x4.push_back(*cvCloneMat(T_Mat));
	}

	cvReleaseMat(&object_points);
	cvReleaseMat(&image_points);
	cvReleaseMat(&point_counts);

	cvReleaseMat(&object_points2);
	cvReleaseMat(&image_points2);
	cvReleaseMat(&point_counts2);

	cvReleaseMat(&rotation_vec);
	cvReleaseMat(&translation_vec);
	cvReleaseMat(&T_Mat);
}

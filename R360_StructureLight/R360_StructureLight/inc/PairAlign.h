//
// Created by �쿭 on 17/4/4.
//

//#pragma once

#ifndef PAIR_ALIGN_ZK_H
#define PAIR_ALIGN_ZK_H

#include <string>

#include <boost/make_shared.hpp> //����ָ��
//��/����
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>
//pcd�ļ�����/���
#include <pcl/io/pcd_io.h>
//�˲�
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/filter.h>
//����
#include <pcl/features/normal_3d.h>
//��׼
#include <pcl/registration/icp.h> //ICP����
#include <pcl/registration/icp_nl.h>
#include <pcl/registration/transforms.h>
//���ӻ�
#include <pcl/visualization/pcl_visualizer.h>

#include "cv.h"

//�����ռ�
using pcl::visualization::PointCloudColorHandlerGenericField;
using pcl::visualization::PointCloudColorHandlerCustom;

//�������͵ı���
typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;

//����ṹ�壬���ڴ������
struct PCD
{
	PointCloud::Ptr cloud; //����ָ��
	std::string f_name; //�ļ���
	//���캯��
	PCD() : cloud(new PointCloud) {}; //��ʼ��
};

extern int total_clude;
extern int Registration_flag;//0:ת̨ 1��icp 2��ת̨+icp
extern int KSearchnum;
extern float MaxCorrespondenceDistance; //��Ӧ��֮��������루0.1��, ����׼�����У����Դ��ڸ���ֵ�ĵ�
extern float LeafSize;
extern float TransformationEpsilon ;//����������
extern bool downsample_flag;
extern int GetRough_T_flag;
extern Eigen::Matrix4f GetR360Rough_T;

void showCloudsLeft(const PointCloud::Ptr cloud_target, const PointCloud::Ptr cloud_source);

void showCloudsRight(const PointCloudWithNormals::Ptr cloud_target, const PointCloudWithNormals::Ptr cloud_source);

void loadData(int argc, char **argv, std::vector<PCD, Eigen::aligned_allocator<PCD> > &models);

void pairAlign(const PointCloud::Ptr cloud_src, const PointCloud::Ptr cloud_tgt, PointCloud::Ptr output, Eigen::Matrix4f &final_transform, bool downsample = false);

void CvMatToMatrix4fzk(Eigen::Matrix4f *pcl_T, CvMat *cv_T);

void roughTranslation(PointCloud::Ptr cloud, Eigen::Matrix4f &T, int n );

void AccurateRegistration(std::vector<PCD, Eigen::aligned_allocator<PCD> > &data_temp);

void AccurateRegistration2(std::vector<PCD, Eigen::aligned_allocator<PCD> > &data_temp);

#endif
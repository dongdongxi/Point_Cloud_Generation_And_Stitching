//
// Created by �쿭 on 17/4/16.
//http://www.cnblogs.com/li-yao7758258/p/6497446.html

#include "stdafx.h"
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>  //kd-tree����������ඨ���ͷ�ļ�
#include <pcl/surface/mls.h>        //��С���˷�ƽ�������ඨ��ͷ�ļ�

void  mls_Filte()//��С���˷��˲�
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>());
	pcl::io::loadPCDFile("9.pcd", *cloud);

	// ���� KD-Tree
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);

	// Output has the PointNormal type in order to store the normals calculated by MLS
	pcl::PointCloud<pcl::PointNormal> mls_points;

	// ������С����ʵ�ֵĶ���mls
	pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal> mls;

	mls.setComputeNormals(true);  //��������С���˼�������Ҫ���з��߹���

	// Set parameters
	mls.setInputCloud(cloud);
	mls.setPolynomialFit(true);//�Ƿ��ö���ʽ��ϱƽ�����
	mls.setSearchMethod(tree);
	mls.setSearchRadius(0.03);//���ö���ʽ���k�ٽ������뾶

	// Reconstruct
	mls.process(mls_points);//������

	// Save output
	pcl::io::savePCDFile("bun0-mls.pcd", mls_points);
}


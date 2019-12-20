//http://blog.csdn.net/xuezhisdc/article/details/51030943

#include "stdafx.h"
#include "PairAlign.h"
#include "Calibrate.h"

#include <boost/make_shared.hpp> //����ָ��
//��/����
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>
//pcd�ļ�����/���
#include <pcl/io/pcd_io.h>
//ply�ļ�����/���
#include <pcl/io/ply_io.h>  
#include <pcl/io/ply/ply.h>
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


//ȫ�ֱ���
//���ӻ�����
pcl::visualization::PCLVisualizer *p;
//�������������������ӻ����ڷֳ�����������
int vp_1, vp_2;

int total_clude;//�ܵ�����
int Registration_flag=2;//0:ת̨ 1��icp 2��ת̨+icp
int KSearchnum = 30;
float MaxCorrespondenceDistance = 0.1; //��Ӧ��֮��������루0.1��, ����׼�����У����Դ��ڸ���ֵ�ĵ�
float LeafSize = 0.05;
float TransformationEpsilon = 1e-6;//����������
bool downsample_flag = true;
int GetRough_T_flag = 1;//1��ʾ���±궨��ã�0��ʾ�ѱ���ֱ�Ӷ�ȡ
Eigen::Matrix4f GetR360Rough_T = Eigen::Matrix4f::Identity();


// �����µĵ��﷽ʽ< x, y, z, curvature > ����+����
class MyPointRepresentation : public pcl::PointRepresentation <PointNormalT> //�̳й�ϵ
{
	using pcl::PointRepresentation<PointNormalT>::nr_dimensions_;
public:
	MyPointRepresentation()
	{
		//ָ��ά��
		nr_dimensions_ = 4;
	}

	//���غ���copyToFloatArray���Զ����Լ�����������
	virtual void copyToFloatArray(const PointNormalT &p, float * out) const
	{
		//< x, y, z, curvature > ����xyz������
		out[0] = p.x;
		out[1] = p.y;
		out[2] = p.z;
		out[3] = p.curvature;
	}
};

void CvMatToMatrix4fzk(Eigen::Matrix4f *pcl_T, CvMat *cv_T)
{
	for (int i = 0; i < 4;i++)
	{
		for (int j = 0; j < 4; j++)
		{
			(*pcl_T)(i, j) = CV_MAT_ELEM(*cv_T, float, i, j);
		}
	}
}

//�ڴ��ڵ����������򵥵���ʾԴ���ƺ�Ŀ�����
void showCloudsLeft(const PointCloud::Ptr cloud_target, const PointCloud::Ptr cloud_source)
{
	p->removePointCloud("vp1_target"); //���ݸ�����ID������Ļ��ȥ��һ�����ơ�������ID
	p->removePointCloud("vp1_source"); //
	PointCloudColorHandlerCustom<PointT> tgt_h(cloud_target, 0, 255, 0); //Ŀ�������ɫ
	PointCloudColorHandlerCustom<PointT> src_h(cloud_source, 255, 0, 0); //Դ���ƺ�ɫ
	p->addPointCloud(cloud_target, tgt_h, "vp1_target", vp_1); //���ص���
	p->addPointCloud(cloud_source, src_h, "vp1_source", vp_1);
	PCL_INFO("Press Q To Begin The Registration.\n"); //������������ʾ��ʾ��Ϣ
	p->spin();
}


//�ڴ��ڵ����������򵥵���ʾԴ���ƺ�Ŀ�����
void showCloudsRight(const PointCloudWithNormals::Ptr cloud_target, const PointCloudWithNormals::Ptr cloud_source)
{
	p->removePointCloud("source"); //���ݸ�����ID������Ļ��ȥ��һ�����ơ�������ID
	p->removePointCloud("target");
	PointCloudColorHandlerGenericField<PointNormalT> tgt_color_handler(cloud_target, "curvature"); //Ŀ����Ʋ�ɫ���
	if (!tgt_color_handler.isCapable())
		PCL_WARN("Cannot Create Curvature Color Handler!");
	PointCloudColorHandlerGenericField<PointNormalT> src_color_handler(cloud_source, "curvature"); //Դ���Ʋ�ɫ���
	if (!src_color_handler.isCapable())
		PCL_WARN("Cannot Create Curvature Color Handler!");
	p->addPointCloud(cloud_target, tgt_color_handler, "target", vp_2); //���ص���
	p->addPointCloud(cloud_source, src_color_handler, "source", vp_2);
	p->spinOnce();
}


// ��ȡһϵ�е�PCD�ļ���ϣ����׼�ĵ����ļ���
// ����argc ����������������main()��
// ����argv �������б�����main()��
// ����models �������ݼ��Ľ������
void loadData(int argc, char **argv, std::vector<PCD, Eigen::aligned_allocator<PCD> > &models)
{

	SetCurrentDirectory("..\\input\\");  ////����·������������

	std::string extension(".ply"); //��������ʼ��string���ͱ���extension����ʾ�ļ���׺��
	// ͨ�������ļ�������ȡpcd�ļ�
	for (int i = 1; i < argc; i++) //�������е��ļ������Թ���������
	{
		std::string fname = std::string(argv[i]);
		if (fname.size() <= extension.size()) //�ļ����ĳ����Ƿ����Ҫ��
			continue;

		std::transform(fname.begin(), fname.end(), fname.begin(), (int(*)(int))tolower); //��ĳ����(Сд��ĸ��)Ӧ����ָ����Χ��ÿ��Ԫ��
		//����ļ��Ƿ���xxx�ļ�
		if (fname.compare(fname.size() - extension.size(), extension.size(), extension) == 0)
		{
			// ��ȡ���ƣ������浽models
			PCD m;
			m.f_name = argv[i];
			if (extension == ".ply")
			{
				pcl::PLYReader reader;
				if (reader.read(argv[i], *m.cloud) < 0)
					std::cout << "��ʧ��" << endl;
			}
			else if (extension == ".pcd")
			{
				pcl::io::loadPCDFile(argv[i], *m.cloud); //��ȡ��������
			}
			//ȥ�������е�NaN�㣨xyz����NaN��
			std::vector<int> indices; //����ȥ���ĵ������
			pcl::removeNaNFromPointCloud(*m.cloud, *m.cloud, indices); //ȥ�������е�NaN��

			models.push_back(m);
		}
	}
	//����û�����
	if (models.empty())
	{
		PCL_ERROR("Syntax is: %s <source.pcd> <target.pcd> [*]", argv[0]); //�﷨
		PCL_ERROR("[*] - multiple files can be added. The registration results of (i, i+1) will be registered against (i+2), etc"); //����ʹ�ö���ļ�
	}
	PCL_INFO("Loaded %d datasets.\n", (int)models.size()); //��ʾ��ȡ�˶��ٸ������ļ�

	SetCurrentDirectory("..\\R360_StructureLight\\");  //����·������������
}

void roughTranslation(PointCloud::Ptr cloud, Eigen::Matrix4f &T,int n=1)//���ԵĽ�һƬ���Ʊ任����һ�����Ƶ�λ�ã�n���������任����
{
	Eigen::Matrix4f T_temp=Eigen::Matrix4f::Identity();
	PointCloud::Ptr temp(new PointCloud); //������ʱ����ָ��
	*temp = *cloud;
	for (int i = 0; i < n; i++)
	{
		T_temp = T_temp*T;//һ��ת�˼��ξͳ˼���
	}
	pcl::transformPointCloud(*temp, *cloud, T_temp);
}

//�򵥵���׼һ�Ե������ݣ������ؽ��
//����cloud_src  Դ����
//����cloud_tgt  Ŀ�����
//����output     �������
//����final_transform �ɶԱ任���� a pair of 
//����downsample �Ƿ��²���
void pairAlign(const PointCloud::Ptr cloud_src, const PointCloud::Ptr cloud_tgt, PointCloud::Ptr output, Eigen::Matrix4f &final_transform, bool downsample)//��cloud_tgt�ƶ���cloud_src
{
	//
	//Ϊ��һ���Ժ��ٶȣ��²���
	// \note enable this for large datasets
	PointCloud::Ptr src(new PointCloud); //��������ָ��
	PointCloud::Ptr tgt(new PointCloud);
	pcl::VoxelGrid<PointT> grid; //VoxelGrid ��һ�������ĵ��ƣ��ۼ���һ���ֲ���3D������,���²������˲���������
	if (downsample) //�²���
	{
		grid.setLeafSize(LeafSize, LeafSize, LeafSize); //������Ԫ�����Ҷ�Ӵ�С
		//�²��� Դ����
		grid.setInputCloud(cloud_src); //�����������
		grid.filter(*src); //�²������˲������洢��src��
		//�²��� Ŀ�����
		grid.setInputCloud(cloud_tgt);
		grid.filter(*tgt);
	}
	else //���²���
	{
	//	PCL_INFO("���²���/n");
		src = cloud_src; //ֱ�Ӹ���
		tgt = cloud_tgt;
	}

	//��������ķ�����������
	PCL_INFO("��������ķ�����������/n");
	PointCloudWithNormals::Ptr points_with_normals_src(new PointCloudWithNormals); //����Դ����ָ�루ע�������Ͱ�������ͷ�������
	PointCloudWithNormals::Ptr points_with_normals_tgt(new PointCloudWithNormals); //����Ŀ�����ָ�루ע�������Ͱ�������ͷ�������
	pcl::NormalEstimation<PointT, PointNormalT> norm_est; //�ö������ڼ��㷨����
	//PCL_INFO("bug here/n");
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>()); //����kd�������ڼ��㷨��������������
	//PCL_INFO("bug not here/n");

	norm_est.setSearchMethod(tree); //������������
	norm_est.setKSearch(KSearchnum); //��������ڵ�����

	norm_est.setInputCloud(src); //����������
	norm_est.compute(*points_with_normals_src); //���㷨���������洢��points_with_normals_src
	pcl::copyPointCloud(*src, *points_with_normals_src); //���Ƶ��ƣ����꣩��points_with_normals_src����������ͷ�������

	norm_est.setInputCloud(tgt); //��3�м���Ŀ����Ƶķ�������ͬ��
	norm_est.compute(*points_with_normals_tgt);//��仰�е�����
	pcl::copyPointCloud(*tgt, *points_with_normals_tgt);
	//����һ�� �Զ�����﷽ʽ�� ʵ��
	MyPointRepresentation point_representation;
	//��Ȩ����ά�ȣ��Ժ�����xyz����ƽ��
	float alpha[4] = { 1.0, 1.0, 1.0, 1.0 };
	point_representation.setRescaleValues(alpha); //��������ֵ����������ʱʹ�ã�

	//����������ICP���� �����ò���
	pcl::IterativeClosestPointNonLinear<PointNormalT, PointNormalT> reg; //����������ICP����ICP���壬ʹ��Levenberg-Marquardt���Ż���
	reg.setTransformationEpsilon(TransformationEpsilon); //���������������������Ż���
	//***** ע�⣺�����Լ����ݿ�Ĵ�С���ڸò���
	reg.setMaxCorrespondenceDistance(MaxCorrespondenceDistance);  //���ö�Ӧ��֮��������루0.1��,����׼�����У����Դ��ڸ���ֵ�ĵ�
	reg.setPointRepresentation(boost::make_shared<const MyPointRepresentation>(point_representation)); //���õ���
	//����Դ���ƺ�Ŀ�����
	//reg.setInputSource (points_with_normals_src); //�汾�����ϣ�ʹ����������
	reg.setInputCloud(points_with_normals_src); //����������ƣ����任�ĵ��ƣ�
	reg.setInputTarget(points_with_normals_tgt); //����Ŀ�����
	reg.setMaximumIterations(2); //�����ڲ��Ż��ĵ�������

	// Run the same optimization in a loop and visualize the results
	Eigen::Matrix4f Ti = Eigen::Matrix4f::Identity(), prev, targetToSource;
	PointCloudWithNormals::Ptr reg_result = points_with_normals_src; //���ڴ洢���������+��������

	for (int i = 0; i < 30; ++i) //����
	{
		PCL_INFO("Iteration Nr. %d.\n", i); //��������ʾ�����Ĵ���
		//������ƣ����ڿ��ӻ�
		points_with_normals_src = reg_result; //
		//����
		//reg.setInputSource (points_with_normals_src);
		reg.setInputCloud(points_with_normals_src); //��������������ƣ����任�ĵ��ƣ�����Ϊ������һ�ε������Ѿ������任��
		reg.align(*reg_result); //���루��׼����������

		Ti = reg.getFinalTransformation() * Ti; //�ۻ���ÿ�ε����ģ��任����
		//�����α任���ϴα任��������ֵС��ͨ����С���Ķ�Ӧ�����ķ�������һ��ϸ��
		if (fabs((reg.getLastIncrementalTransformation() - prev).sum()) < reg.getTransformationEpsilon())
			reg.setMaxCorrespondenceDistance(reg.getMaxCorrespondenceDistance() - 0.001); //��С��Ӧ��֮��������루�������ù���
		prev = reg.getLastIncrementalTransformation(); //��һ�α任�����
		//��ʾ��ǰ��׼״̬���ڴ��ڵ����������򵥵���ʾԴ���ƺ�Ŀ�����
		showCloudsRight(points_with_normals_tgt, points_with_normals_src);
	}

	targetToSource = Ti.inverse(); //�����Ŀ����Ƶ�Դ���Ƶı任����
	pcl::transformPointCloud(*cloud_tgt, *output, targetToSource); //��Ŀ����� �任�ص� Դ����֡����һ���Ǿ���

	p->removePointCloud("source"); //���ݸ�����ID������Ļ��ȥ��һ�����ơ�������ID
	p->removePointCloud("target");
	PointCloudColorHandlerCustom<PointT> cloud_tgt_h(output, 0, 255, 0); //���õ�����ʾ��ɫ����ͬ
	PointCloudColorHandlerCustom<PointT> cloud_src_h(cloud_src, 255, 0, 0);
	p->addPointCloud(output, cloud_tgt_h, "target", vp_2); //��ӵ������ݣ���ͬ
	p->addPointCloud(cloud_src, cloud_src_h, "source", vp_2);

	PCL_INFO("Press Q To Continue The Registration.\n");
	p->spin();

	p->removePointCloud("source");
	p->removePointCloud("target");

	//add the source to the transformed target
	*output += *cloud_src; // ƴ�ӵ���ͼ���ĵ㣩������Ŀ���������Ƶĵ�����

	final_transform = targetToSource; //���յı任��Ŀ����Ƶ�Դ���Ƶı任����
}

void AccurateRegistration(std::vector<PCD, Eigen::aligned_allocator<PCD> > &data_temp)//�Ȱ����е����ƶ���1��Ȼ������1Ϊ����ƴ��������
{
	//����һ�� PCLVisualizer ����
	p = new pcl::visualization::PCLVisualizer("Pairwise Incremental Registration example"); //p��ȫ�ֱ���
	p->createViewPort(0.0, 0, 0.5, 1.0, vp_1); //����������
	p->createViewPort(0.5, 0, 1.0, 1.0, vp_2); //����������

	//��������ָ��ͱ任����
	PointCloud::Ptr result(new PointCloud), source(new PointCloud), target; //����3������ָ�룬�ֱ����ڽ����Դ���ƺ�Ŀ�����
	//ȫ�ֱ任���󣬵�λ���󣬳ɶԱ任
	//���ű��ʽ���ȴ���һ����λ����Ȼ�󽫳ɶԱ任 ���� ȫ�ֱ任
	Eigen::Matrix4f pairTransform;//GlobalTransform = Eigen::Matrix4f::Identity(), 
	Eigen::Matrix4f T1 = Eigen::Matrix4f::Identity(), T2 = Eigen::Matrix4f::Identity(), R360Plant_Transform = Eigen::Matrix4f::Zero(), T_Pro2Cam = Eigen::Matrix4f::Identity(), T_Pro2chessboard = Eigen::Matrix4f::Identity(), T_Cam2chessboard = Eigen::Matrix4f::Identity(), T_Rz180 = Eigen::Matrix4f::Identity();
	
	if (GetRough_T_flag == 1)
	{
		CvMatToMatrix4fzk(&T_Pro2chessboard, Cam_extrinsic_matrix);
		CvMatToMatrix4fzk(&T_Cam2chessboard, Pro_extrinsic_matrix);
		T_Pro2Cam = T_Pro2chessboard*(T_Cam2chessboard.inverse());
//		T_Pro2Cam = T_Pro2chessboard;
		T_Rz180(0, 0) = -1;
		T_Rz180(1, 1) = -1;

		//����任����p1=T1p0,p2=T2p0,p1=T1*inverserT2p2   (p0��p1,p2�Ƕ�Ӧ�㣬���Ե�p1��p2�غϵı任��������),������2�ƶ���1,T1,T2��ָ�ӵ�������ϵ������TI���룬��ͶӰ������ϵ�����ų������������ϵ�Ŀ��ܣ���Z��ת180��õ����任����������ϵ
		for (int i = 0; i < (T_mat_4x4.size() - 1); i++)//���������궨�õ��ľ������Ȼ����ƽ����Ҳ��ĳ�������ƽ����α任�ɣ�
		{
			CvMatToMatrix4fzk(&T1, &(T_mat_4x4[i]));
			CvMatToMatrix4fzk(&T2, &(T_mat_4x4[i + 1]));

//			R360Plant_Transform += ((T_Rz180*T1)) * ((T_Rz180*T2).inverse());//�˴��������������ϵ������ͶӰ������ϵ��((T_Rz180*T_Pro2Cam*T1)) * ((T_Rz180*T_Pro2Cam*T2).inverse())
			R360Plant_Transform += ((T_Rz180*T_Pro2Cam*T1)) * (((T_Rz180*T_Pro2Cam*T2)).inverse());//��1ת��2�˴���������ǻ���ͶӰ������ϵ
			std::cout << T1 << endl;
			std::cout << T2 << endl;
			std::cout << "T1*(T2.inverse()):" << endl;
			std::cout << T1*(T2.inverse()) << endl;
			std::cout << ((T_Rz180*T_Pro2Cam*T1)) * (((T_Rz180*T_Pro2Cam*T2)).inverse()) << endl;
			std::cout << R360Plant_Transform << endl;
		}
		R360Plant_Transform = R360Plant_Transform / (T_mat_4x4.size() - 1);
		std::cout << R360Plant_Transform << endl;//������ĶԲ���
	}
	else if (GetRough_T_flag == 0)
	{
		//CvMat *temp1 = cvCreateMat(4, 4, CV_64FC1);
		//CvMat *temp2 = cvCreateMat(4, 4, CV_32FC1);

		//CvFileStorage *fs1;
		//fs1 = cvOpenFileStorage(".//result//Rough_T.xml", 0, CV_STORAGE_READ);
		//if (fs1)
		//{
		//	*temp1 = *cvCloneMat((CvMat *)cvReadByName(fs1, NULL, "Rough_T"));
		//	cvReleaseFileStorage(&fs1);
		//}
		//else
		//{
		//	cout << "Error: can not find the Rough_T!!!!!" << endl;
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	for (int j = 0; j < 4; j++)
		//	{
		//		CV_MAT_ELEM(*temp2, float, i, j) = CV_MAT_ELEM(*temp1, double, i, j);
		//	}
		//}
		//CvMatToMatrix4fzk(&R360Plant_Transform, temp2);
		R360Plant_Transform = GetR360Rough_T;
		std::cout << R360Plant_Transform << endl;
	}

	if ((Registration_flag == 0) || (Registration_flag == 2))
	{
		//��ƴ�ӣ�ת̨��α任��
		for (size_t i = 1; i < data_temp.size(); ++i)
		{
			roughTranslation(data_temp[i].cloud, R360Plant_Transform, i);//�����е����ƶ���1�ŵ��Ƶ�λ��
		}
	}
	//��ƴ�ӣ��������еĵ����ļ�
//	PointCloud::Ptr temp(new PointCloud); //������ʱ����ָ��
	*result = *(data_temp[0].cloud);
	for (size_t i = 1; i < data_temp.size(); ++i)//������׼��1+2����2�Ƶ�1λ��
	{
		*source = *result; //Դ����
		target = data_temp[i].cloud; //Ŀ�����
		showCloudsLeft(source, target); //�����������򵥵���ʾԴ���ƺ�Ŀ�����		

		//��ʾ������׼�ĵ����ļ����͸��Եĵ���
		PCL_INFO("Aligning %s (%d points) with %s (%d points).\n", data_temp[i - 1].f_name.c_str(), source->points.size(), data_temp[i].f_name.c_str(), target->points.size());

		//********************************************************
		if ((Registration_flag == 1) || (Registration_flag == 2))
		{
			//��׼2�����ƣ��������������
			pairAlign(target, source, result, pairTransform, downsample_flag);//temp���ǽ�sourceƴtarget�ںϲ��ĵ���
		}
		else if (Registration_flag == 0)
		{
			*result = *source + *target;
		}
		//********************************************************

		p->removePointCloud("source"); //���ݸ�����ID������Ļ��ȥ��һ�����ơ�������ID
		p->removePointCloud("target");
		PointCloudColorHandlerCustom<PointT> cloud_tgt_a(result, 0, 255, 0); //���õ�����ʾ��ɫ����ͬ
		//		PointCloudColorHandlerCustom<PointT> cloud_src_h(cloud_src, 255, 0, 0);
		p->addPointCloud(result, cloud_tgt_a, "target", vp_2); //��ӵ������ݣ���ͬ
		//		p->addPointCloud(cloud_src, cloud_src_h, "source", vp_2);

		PCL_INFO("Processing...\n");
		p->spinOnce();
		//		Sleep(5000);
		//p->removePointCloud("source");
		//p->removePointCloud("target");
	}
	char s[30];
	std::cout << "Please Enter Output File Name��" << endl;
	std:cin >> s;
	std::stringstream output_filename; //�������������ļ���
	output_filename << ".//result//" << s << ".ply";
	pcl::PLYWriter writer;
	writer.write(output_filename.str(), *result);
//	pcl::io::savePCDFile(ss.str(), *result); //����ɶԵ���׼���
}

void AccurateRegistration2(std::vector<PCD, Eigen::aligned_allocator<PCD> > &data_temp)//��1�ƶ���2��icpƴ�ӣ��ڽ�����ƶ���3��3icp
{
	//����һ�� PCLVisualizer ����
	p = new pcl::visualization::PCLVisualizer("Pairwise Incremental Registration example"); //p��ȫ�ֱ���
	p->createViewPort(0.0, 0, 0.5, 1.0, vp_1); //����������
	p->createViewPort(0.5, 0, 1.0, 1.0, vp_2); //����������

	//��������ָ��ͱ任����
	PointCloud::Ptr result(new PointCloud), source(new PointCloud), target; //����3������ָ�룬�ֱ����ڽ����Դ���ƺ�Ŀ�����
	//ȫ�ֱ任���󣬵�λ���󣬳ɶԱ任
	//���ű��ʽ���ȴ���һ����λ����Ȼ�󽫳ɶԱ任 ���� ȫ�ֱ任
	Eigen::Matrix4f pairTransform;//GlobalTransform = Eigen::Matrix4f::Identity(), 
	Eigen::Matrix4f T1 = Eigen::Matrix4f::Identity(), T2 = Eigen::Matrix4f::Identity(), R360Plant_Transform = Eigen::Matrix4f::Zero(), T_Pro2Cam = Eigen::Matrix4f::Identity(), T_Pro2chessboard = Eigen::Matrix4f::Identity(), T_Cam2chessboard = Eigen::Matrix4f::Identity(), T_Rz180 = Eigen::Matrix4f::Identity();

	if (GetRough_T_flag == 1)
	{
		CvMatToMatrix4fzk(&T_Pro2chessboard, Cam_extrinsic_matrix);
		CvMatToMatrix4fzk(&T_Cam2chessboard, Pro_extrinsic_matrix);
		T_Pro2Cam = T_Pro2chessboard*(T_Cam2chessboard.inverse());
		T_Rz180(0, 0) = -1;
		T_Rz180(1, 1) = -1;

		//����任����p1=T1p0,p2=T2p0,p1=T1*inverserT2p2   (p0��p1,p2�Ƕ�Ӧ�㣬���Ե�p1��p2�غϵı任��������),������2�ƶ���1,T1,T2��ָ�ӵ�������ϵ������TI���룬��ͶӰ������ϵ�����ų������������ϵ�Ŀ��ܣ���Z��ת180��õ����任����������ϵ
		for (int i = 0; i < (T_mat_4x4.size() - 1); i++)//���������궨�õ��ľ������Ȼ����ƽ����Ҳ��ĳ�������ƽ����α任�ɣ�
		{
			CvMatToMatrix4fzk(&T1, &(T_mat_4x4[i]));
			CvMatToMatrix4fzk(&T2, &(T_mat_4x4[i + 1]));

			R360Plant_Transform += ((T_Rz180*T2)) * ((T_Rz180*T1).inverse());
			std::cout << T1 << endl;
			std::cout << T2 << endl;
			std::cout << ((T_Rz180*T2)) * ((T_Rz180*T1).inverse()) << endl;//�˴��������������ϵ
			std::cout << R360Plant_Transform << endl;
		}
		R360Plant_Transform = R360Plant_Transform / (T_mat_4x4.size() - 1);
		std::cout << R360Plant_Transform << endl;//������ĶԲ���
	}
	else if (GetRough_T_flag == 0)
	{
		R360Plant_Transform = GetR360Rough_T.inverse();
		std::cout << GetR360Rough_T << endl;
		std::cout << R360Plant_Transform << endl;
	}

	//��ƴ�ӣ��������еĵ����ļ�
	//	PointCloud::Ptr temp(new PointCloud); //������ʱ����ָ��
	*result = *(data_temp[0].cloud);
	for (size_t i = 1; i < data_temp.size(); ++i)//������׼��1+2����2�Ƶ�1λ��
	{
		*source = *result; //Դ����
		target = data_temp[i].cloud; //Ŀ�����

		if ((Registration_flag == 0) || (Registration_flag == 2))
		{
			roughTranslation(source, R360Plant_Transform, 1);//�����������ƶ�����һ���Ƶ�λ��	
		}
		showCloudsLeft(source, target); //�����������򵥵���ʾԴ���ƺ�Ŀ�����		

		//��ʾ������׼�ĵ����ļ����͸��Եĵ���
		PCL_INFO("Aligning %s (%d points) with %s (%d points).\n", data_temp[i - 1].f_name.c_str(), source->points.size(), data_temp[i].f_name.c_str(), target->points.size());

		//********************************************************

		if ((Registration_flag == 1) || (Registration_flag == 2))
		{
			//��׼2�����ƣ��������������

			pairAlign(target, source, result, pairTransform, downsample_flag);//temp���ǽ�srcƴ��target�ϲ��ĵ���

		}
		else if (Registration_flag == 0)
		{
			*result = *source + *target;
		}
		//********************************************************

		p->removePointCloud("source"); //���ݸ�����ID������Ļ��ȥ��һ�����ơ�������ID
		p->removePointCloud("target");
		PointCloudColorHandlerCustom<PointT> cloud_tgt_a(result, 0, 255, 0); //���õ�����ʾ��ɫ����ͬ
		//		PointCloudColorHandlerCustom<PointT> cloud_src_h(cloud_src, 255, 0, 0);
		p->addPointCloud(result, cloud_tgt_a, "target", vp_2); //��ӵ������ݣ���ͬ
		//		p->addPointCloud(cloud_src, cloud_src_h, "source", vp_2);

		PCL_INFO("Processing...\n");
		p->spinOnce();
		//		Sleep(5000);
		//p->removePointCloud("source");
		//p->removePointCloud("target");
	}
	char s[30];
	std::cout << "Please Enter Output File Name��" << endl;
    std:cin >> s;
	std::stringstream output_filename; //�������������ļ���
	output_filename << "..//output//" << s << ".ply";
	pcl::PLYWriter writer;
	writer.write(output_filename.str(), *result);
	//	pcl::io::savePCDFile(ss.str(), *result); //����ɶԵ���׼���
}



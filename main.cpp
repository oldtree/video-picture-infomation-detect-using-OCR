#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <mysql.h>
#include <boost/lexical_cast.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/core/wimage.hpp>
#include <math.h>
#include <wchar.h>
#include <list>
#include <time.h>
#include <ctime>
#include <strstream>
#include <omp.h>
using namespace std;
using namespace cv;
//////////////////////////////////////////////////////////////////////////
void  readfile(int target);
bool  DBconnect();
bool  DBdisconnect();
bool  DBop(string body,string head,int order);
void  DBinsertResult(string body,string timepoint);
void  getRect(Mat image);
int   process(VideoCapture& capture);
void  target_Pro(string temp,int target,string path);
void  show_result(int target,VideoCapture &capture);
void  read_target_info(int target);
void  setUser(string newuser);
void  setPassword(string newpassword);
void  setDatabase(string newdatabase);
void  setPort(unsigned newPort);
void  setTable(string newtable);
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
int   width =0;
int   heigh =0;
Rect  TOP; //顶部的字符方块分布矩形
Rect  DOWN;//底部的字符方块分布矩形
Rect  TOP_temp;//顶部的字符方块分布矩形临时存储，用以接下来的比较
Rect  DOWN_temp;//底部的字符方块分布矩形临时存储，用以接下来的比较
int   top=100; //初始值，用以调节顶部矩形的高
int   down=30; //初始值，用以调节底部矩形的高
int   temp_position1=0;
int   temp_position2=0;
///////////////////////////////////////////////////////
Mat   target1;//目标1的存储矩阵
Mat   target2;//目标2的存储矩阵
Mat   target1_temp;//目标1的临时存储矩阵  ，用与以后进行比较
Mat   target2_temp;//目标2的临时存储矩阵	，用与以后进行比较
time_t begintime;
time_t endtime;
struct tm * timeinfo;
//////////////////////////////////////////////////////////////////////////
string   user =     "root";
string   password = "123456";
string   host =     "localhost";
string   database=  "data";
string   table=     "info";
string   resulttable=" result";
unsigned int  port = 3306;
MYSQL        mycon;
MYSQL_RES  * resualt;
MYSQL_ROW    sql_row;
MYSQL_FIELD *field;
bool   db =  false;//数据库是否连接

int fps_num = 0;//记录视频读取的帧数
string videoname;
//////////////////////////////////////////////////////////////////////////
struct TargetNode
{
	bool mark;
	int fps;
	string info;
	int time_point;
	string path;
};
//////////////////////////////////////////////////////////////////////////

string target1_node_temp;
bool target1_mark = false;
string target2_node_temp;
bool target2_mark = false;
list<TargetNode> target1_list;
int target1_num = 1;
list<TargetNode> target2_list;
int target2_num = 1;
//////////////////////////////////////////////////////////////////////////
void target_Pro(string temp,int target,string path)
{	
	if (target==1)
	{
		TargetNode newnode;
		newnode.fps=fps_num;
		newnode.info.assign(temp.c_str());
		newnode.time_point=fps_num/24;
		newnode.path.assign(path.c_str());
		target1_list.push_back(newnode);
	}
	if (target==2)
	{
		TargetNode newnode;
		newnode.fps=fps_num;
		newnode.info.assign(temp.c_str());
		newnode.time_point=fps_num/24;
		newnode.path.assign(path.c_str());
		target2_list.push_back(newnode);
	}
	return ;
};
void show_result(int target,VideoCapture &capture)
{
	list<TargetNode>::iterator target_iterator;
	int count=0;
	if (target==1)
	{
		Mat target1_collection;
		Mat target1_show;
		for (target_iterator=target1_list.begin();target_iterator!=target1_list.end();target_iterator++)
		{	
			if (target_iterator->mark)
			{  
				for(;count<=target_iterator->fps;count++);
				{
					capture>>target1_show;
				}
				imshow(target_iterator->path.append("all"),target1_show);
				target1_collection=imread(target_iterator->path);
				imshow(target_iterator->path,target1_collection);
				waitKey(3000);
			}   
		}
	}																							 
	if (target==2)
	{
		Mat target2_collection;
		Mat target2_show;
		for (target_iterator=target2_list.begin();target_iterator!=target2_list.end();target_iterator++)
		{
			if (target_iterator->mark)
			{  
				for(;count<=target_iterator->fps;count++);
				{
					capture>>target2_show;
				}
				target2_collection=imread(target_iterator->path);
				imshow(target_iterator->path,target2_collection);
				imshow(target_iterator->path.append("all"),target2_show);

				waitKey(3000);
			} 
		}
	}
	return ;
};
void read_target_info(int target)
{
	list<TargetNode>::iterator target_iterator;
	if (target==1)
	{
		Mat target1_collection;
		string str1_exe;
		for (target_iterator=target1_list.begin();target_iterator!=target1_list.end();target_iterator++)
		{
			str1_exe.append("tesseract.exe ").append(target_iterator->path).append(" target1  -l chi_sim");
			system(str1_exe.c_str());
			readfile(1);
			target_iterator->mark=target1_mark;
			target1_mark=false;
			target_iterator->info.clear();
			target_iterator->info.assign(target1_node_temp.c_str());
			target1_node_temp.clear();
			str1_exe.clear();
		}
	}
	if (target==2)
	{
		Mat target2_collection;
		string str2_exe;
		for (target_iterator=target2_list.begin();target_iterator!=target2_list.end();target_iterator++)
		{
			str2_exe.append("tesseract.exe ").append(target_iterator->path).append(" target2  -l chi_sim");
			system(str2_exe.c_str());
			readfile(2);
			target_iterator->mark=target2_mark;
			target2_mark=false;
			target_iterator->info.clear();
			target_iterator->info.assign(target2_node_temp.c_str());
			target2_node_temp.clear();
			str2_exe.clear();
		}
	}
	return ;
};
void setVideoname(string newvideoname)
	{
		videoname=newvideoname;
	};
void setUser(string newuser)
{
	user=newuser;
	return ;
};
void setPassword(string newpassword)
{
	password=newpassword;
	return ;
};
void setDatabase(string newdatabase)
{
	database=newdatabase;
};
void setPort(unsigned newPort)
{
	port=newPort;
};
void setTable(string newtable)
{
	table=newtable;
	return ;
};
void setResultTable(string newresulttable)
	{
		 resulttable=newresulttable;
	};
bool DBconnect()
{
	if (db)
	{
		return true;
	}
	mysql_init(&mycon);
	if (mysql_real_connect(&mycon,host.c_str(),user.c_str(),password.c_str(),database.c_str(),port,NULL,0))
	{
		mysql_query(&mycon,"set names GBK");
		db = true;
		return true;
	}
	else
	{
		db = false;
		return false;
	}
};
bool  DBdisconnect()
{
	if (!db)
	{
		return false;
	}
	try
	{
		mysql_close(&mycon);
		return true;
	}
	catch (Exception* e)
	{
		mysql_close(&mycon);
	}
	return false;

};
bool DBop(string body,string head,int order,int target)
{
	if (!db)
	{
		return false;
	}
	int hcount=0,mcount=0,scount=0;
	strstream sbuf;
	string s1,s2,s3;
	if (order==1)
	{
		mysql_query(&mycon,"SET NAMES GBK");
		string sql="insert into " ;
		sql.append(table);
		sql.append(" (head, body ) values ( '");
		sql.append(head);
		sql.append("','");
		sql.append(body);
		sql.append(" ')");
		mysql_query(&mycon,sql.c_str());
		return true;
	}
	if (order==2)
	{
		char column[32][32];
		string sql="select * from " ;
		sql.append(table);
		sql.append(" where head = '");
		sql.append(head);
		sql.append("'");
		mysql_query(&mycon,sql.c_str());
		resualt=mysql_store_result(&mycon);
		if (resualt)
		{
			int i,j=0;
			if ((unsigned long)mysql_num_rows(resualt)==0)
			{
				return false;
			}
			j=mysql_num_fields(resualt);
			while (sql_row=mysql_fetch_row(resualt))
			{	
				for (i=2;i<j;i++)
				{
					string temp(sql_row[i]);
					
					if (temp==body.substr(0,temp.length()))
					{
						if (target==1)
						{
							target1_mark=true;
							target1_node_temp.assign(temp.c_str());
							hcount=(fps_num/24)/3600;
							sbuf << hcount;
							sbuf >> s1;
							sbuf.clear();
							mcount=((fps_num/24)-hcount*3600)/60;
							sbuf << mcount;
							sbuf >> s2;
							sbuf.clear();
							scount=((fps_num/24)-hcount*3600-mcount*60);
							sbuf << scount;
							sbuf >> s3;
							sbuf.clear();
							s1.append(":").append(s2).append(":").append(s3);
							DBinsertResult(body,s1);
							cout<<s1<<endl;
							s1.clear();s2.clear();s3.clear();
						}
						if (target==2)
						{
							target2_mark=true;
							target2_node_temp.assign(temp.c_str());
							hcount=(fps_num/24)/3600;
							sbuf << hcount;
							sbuf >> s1;
							sbuf.clear();
							mcount=((fps_num/24)-hcount*3600)/60;
							sbuf << mcount;
							sbuf >> s2;
							sbuf.clear();
							scount=((fps_num/24)-hcount*3600-mcount*60);
							sbuf << scount;
							sbuf >> s3;
							sbuf.clear();
							s1.append(":").append(s2).append(":").append(s3);
							DBinsertResult(body,s1);
							cout<<s1<<endl;
							s1.clear();s2.clear();s3.clear();
						}
					}
				}
			}
		}
		else
		{
			return false;
		}
		if (resualt!=NULL)
		{
			mysql_free_result(resualt);
		}
		return true;
	}
	
};
void DBinsertResult(string body,string timepoint)
	{
		mysql_query(&mycon,"SET NAMES GBK");
		string sql="insert into  " ;
		sql.append(resulttable);
		sql.append("(video, info,timepoint ) values ( '");
		sql.append(videoname);
		sql.append("','");
		sql.append(body);
		sql.append("','");
		sql.append(timepoint);
		sql.append(" ')");
		mysql_query(&mycon,sql.c_str());
		cout<<body<<"  "<<timepoint<<endl;
		return ;
	} ;
void getRect(Mat image)
{
	Mat src;
	Mat dst, cdst;
	cvtColor(image,src,CV_BGR2GRAY);
	if (width!=image.cols||heigh!=image.rows)
	{ 
		width=image.cols;
		heigh=image.rows;
	}
	Canny(src, dst,10,100,3,false);
	cvtColor(dst, cdst, CV_GRAY2BGR);
	vector<Vec4i> lines;
	HoughLinesP(dst, lines, 1, CV_PI/180, 60,30,20 );
	for( size_t i = 0; i < lines.size(); i++ )
	{
		Vec4i l = lines[i];
		if (l[1]>l[3]+4||l[1]<l[3]-4||l[2]-l[0]<(src.cols*0.75))
		{	
			continue;
		}
		//////////////////////////////////////////////////////////////////////////
		temp_position1=(l[1]+l[3])/2;
		if ((temp_position1<top)&&(temp_position1<src.rows/2))
		{
			top=temp_position1;
		}
		if ((temp_position1>down)&&(temp_position1>src.rows/2))
		{
			down=temp_position1;
		}	
	}
	if ((src.rows-down)<(src.rows-down+top)/2)
	{
		down= src.rows-top;
	}
	if (top<(src.rows-down+top)/2)
	{
		top= src.rows-down;
	}
	TOP.x=0;TOP.y=0;TOP.width=src.cols;TOP.height=top;
	DOWN.x=0;DOWN.y=down;DOWN.width=src.cols;DOWN.height=src.rows-down;
	cv::rectangle(cdst,TOP,Scalar(0,255,0),3,8);
	cv::rectangle(cdst,DOWN,Scalar(255,0,0),3,8);
	imshow("src", src);
	imshow("detected lines", cdst);
	return ;
};
int process(VideoCapture& capture)
{	
	bool over1=false,over2=false,over3=false;
	time ( &begintime );
#pragma omp parallel for
	for (int op=1;op<=3;op++)
	{
		if (op==1)
		{
			char pathname1[40];
			char pathname2[40];
			Mat frame;
			Mat target1_dst;
			Mat target2_dst;
			bool readpic=true;
			fps_num=0;
			while (readpic) {
				capture>>frame;
				fps_num++;
				if (frame.empty())
				{
					over1=true;
					if (over1==true&&over2==true&&over3==true)
					{ 
						Sleep(2000);
						readpic=false;
					}
					else
					{
						Sleep(2000);
						continue;
					}
				}
				if ((fps_num%20)!=0)
				{
					continue;
				}
				getRect(frame);

				target1=frame(TOP);
				sprintf(pathname1, "pic1/100%d.jpg", target1_num);
				target1_num++;
				cv::imwrite(pathname1,target1);
				if ((cv::mean(target1).operator[](0)>5))
				{
					target1.release();
					if ((TOP.width!=TOP_temp.width||TOP.height!=TOP_temp.height))
					{
						target_Pro("#",1,pathname1);
						TOP_temp.x=TOP.x;
						TOP_temp.y=TOP.y;
						TOP_temp.width=TOP.width;
						TOP_temp.height=TOP.height;
					}
					else
					{
						target1=imread(pathname1,0);
						cv::subtract(target1,target1_temp,target1_dst);
						threshold(target1_dst,target1_dst,50,200,CV_THRESH_TOZERO);
						Canny(target1_dst,target1_dst,50,100);
						if ((cv::mean(target1_dst).operator[](0)>5.0))
						{   	    
							target_Pro("#",1,pathname1);
						}
					}
				}
				target1_temp=imread(pathname1,0);
				target2=frame(DOWN);
				sprintf(pathname2, "pic2/200%d.jpg", target2_num);
				target2_num++;
				cv::imwrite(pathname2,target2);	
				if((cv::mean(target2).operator[](0)>5))
				{
					target2.release();
					if (DOWN.width!=DOWN_temp.width||DOWN.height!=DOWN_temp.height)
					{
						target_Pro("#",2,pathname2);
						//////////////////////////////////////////////////////////////////////////
						DOWN_temp.x=DOWN.x;
						DOWN_temp.y=DOWN.y;
						DOWN_temp.width=DOWN.width;
						DOWN_temp.height=DOWN.height;
					}
					else
					{
						target2=imread(pathname2,0);
						cv::subtract(target2,target2_temp,target2_dst);
						threshold(target2_dst,target2_dst,20,200,CV_THRESH_TOZERO);
						Canny(target2_dst,target2_dst,30,100);
						if ((cv::mean(target2_dst).operator[](0)>5.0))
						{ 
							target_Pro("#",2,pathname2);
						}
					}
				}
				target2_temp=imread(pathname2,0);
				waitKey(1);
			}
			target1_dst.release();
			target2_dst.release();
		}
		if (op==2)
		{
			Sleep(5000);
			cout<<"2 start"<<endl;
			read_target_info(1);
			over2=true;
			cout<<"other not done yet  thread    2---->  "<<over2<<endl;
		}
		if (op==3)
		{
			Sleep(5000);
			cout<<"3 start"<<endl;
			read_target_info(2);
			over3=true;
			cout<<"other not done yet  thread    3---->  "<<over3<<endl;
		}
	}
	time ( &endtime );
	endtime=endtime-begintime;
	timeinfo = localtime ( &endtime );
	cout<<"用时："<<timeinfo->tm_hour<<" : "<<timeinfo->tm_min<<" : "<<timeinfo->tm_sec<<endl;
	target1.release();
	target1_temp.release();
	target2.release();
	target2_temp.release();
	return 0;
};
void readfile(int target)
{   
	string   body;
	string   head;
	wchar_t  buf[2];
	FILE   * file;
	wstring  wstr;										

	if (target==1)
	{
		file  =  _wfopen(L"target1.txt ",L"rt+,ccs=UTF-8"); 
	}
	else
	{
		file  =  _wfopen(L"target2.txt ",L"rt+,ccs=UTF-8");
	} 
	locale loc(""); 
	while(!feof(file)) 
	{ 
		fgetws(buf,2,file);
		if (buf[0]==10)									  
		{
			break;
		}
		wstr.append(buf);
	}
	fclose(file);
	//////////////////////////////////////////////////////////////////////////
	string curlocal=setlocale(LC_ALL,NULL);
	setlocale(LC_ALL,"");
	//////////////////////////////////////////////////////////////////////////
	const wchar_t *source;
	size_t temp_size = 0;
	size_t head_size=0;
	char * temp = NULL;
	char * head_temp=NULL;
	for (int i=0;i<wstr.length();i++)
	{
		source = &wstr.c_str()[i];
		temp_size = 2 * (wstr.size()-i)+ 1;
		head_size=2+1;
		temp = new char[temp_size];
		head_temp=new char[head_size];
		memset(temp,0,temp_size);
		memset(head_temp,0,2+1);
		wcstombs(temp,source,temp_size);
		wcstombs(head_temp,source,2+1);
		body = temp;
		head = head_temp;
		{
			DBop(body,head,2,target);
		}
	}							  	
	//////////////////////////////////////////////////////////////////////////
	setlocale(LC_ALL,curlocal.c_str());
	delete [] temp;
	delete [] head_temp;
};


int main()
{
	if (DBconnect())
	{

		cout<<"-------DB connect sucssesed!--------"<<endl;
		videoname="诸葛亮经典励志演讲视频.flv";
		VideoCapture cap("C:/Users/ubuntu/Desktop/奥巴马开学演讲.flv");
		if (!cap.isOpened())
		{
			cap.open(-1);
			cout<<"视频打开失败，请检查！"<<endl;
		}
		process(cap);
		cap.release();
		cout<<"检查完毕"<<endl;
		VideoCapture cap2("C:/Users/ubuntu/Desktop/奥巴马开学演讲.flv");
		show_result(2,cap2);
		if (DBdisconnect())
		{
			cout<<"-------DB closed sucssesed!--------";
			system("pause");
			return 1;
		}else
		{
			cout<<"---------task is done-----------"<<endl;
			cout<<"-------but DB have some problem!--------"<<endl;
			system("pause");
			return 0;
		}
	}
	else
	{
		system("pause");
		return 0;
	}
}

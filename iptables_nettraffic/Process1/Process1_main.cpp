/************************************************************
*Copyright (C), 2017-2027,lcb0281at163.com lcb0281atgmail.com
*FileName: Process1_main.cpp
*Date:     2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Description:����wsd_connect�ӿڽ�IP��ַ���ݸ�ͳ�ƽ���
*Others: 
***********************************************************/
#include<stdio.h>       
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<termios.h>
#include<sys/types.h>   
#include<sys/stat.h>    
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<signal.h>

#include "IPAddrInterFace.h"


#define MAXLINE 256
#define PORT    6666
int fd;
/*
linux ctrl + C ����� SIGINT�ź�
���յ�SIGINT �źŽ���ú���
*/
void stop(int signo)
{
    printf("client stop\n");
	close(fd);  
	_exit(0);
}
 
/*�ͻ��˴�������*/
void client_process(void)
{
	char readbuff[MAXLINE];
	char writebuff[MAXLINE];
	char * write = "I am client";
	int num = 0;
	
	while(1)
	{
		num = recv(fd,readbuff,MAXLINE,0);/*���շ���˵����ݣ�recv���������û�����ݻ�����*/
		if(num > 0)
		{
			printf("client read data : %s \n",readbuff);
			send(fd, write, strlen(write)+1, 0); /*���յ����ݺ��������˷���һ���ַ���*/
		}
		else if(num == 0)/*recv����ֵΪ0 ��ʱ���ʾ������Ѿ��Ͽ�������*/
		{
			stop(1); /*ִ���˳�����*/
		}
	}
}
 
int main(int argc, char** argv)
{
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int ret;
 
	fd = socket(AF_INET,SOCK_STREAM,0);/*������ʽ�׽���*/
	if(fd < 0)
	{
		printf("clinet socket err \n");
	}
 
	/*���÷���˵�ַ*/
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET; 				/*AF_INET��ʾ IPv4 Intern Э��*/
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*INADDR_ANY ���Լ�������IP */
	server_addr.sin_port = htons(PORT);				/*���ö˿�*/
 
	inet_pton(AF_INET,"192.168.20.221",&server_addr.sin_addr);/*���û�������ַ������͵�IP��ַתΪ����*/
	//connect(fd,(struct sockaddr*)&server_addr,sizeof(server_addr));/*���ӷ�����*/
	wsd_connect(fd,(struct sockaddr*)&server_addr,sizeof(server_addr));/*���ӷ�����*/
	signal(SIGINT,stop);	/*ע��SIGINT�ź�*/
	client_process();		/*���봦������*/
	
	close(fd);/*�ر��ļ�*/
	return 0;
}
 
 
 




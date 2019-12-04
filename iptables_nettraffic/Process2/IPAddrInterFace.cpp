/************************************************************
*Copyright (C), 2017-2027,lcb0281at163.com lcb0281atgmail.com
*FileName: IPAddrInterFace.cpp
*Date:     2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Description:
*History:
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>


#include "IPAddrInterFace.h"

/************************************************* 
Function:    IPAddWriteToPipe  
Description: ��IP��ַд��ܵ��С�
Input: u32IPAddr
output:none
Return:0 �ɹ�����0��ʧ��
Others:ע�������ܵ��Ķ�д����
	1.�Է�����ֻд��ʽ��ʱ���ڶ������ͬʱд�����ݣ�ע��д���ԭ���ԡ�
	2.�Է�����ֻд��ʽ��ʱ�����û��һ�������ڶ��ܵ����򿪻�ʧ�ܡ�
	3.�Է�����ֻд��ʽ��ʱ��������ж��ܵ����̹رգ�д���̻��յ�SIGPIPE�ź�
	   ���д���̲���SIGPIPE�źŽ��д����ᵼ��д�����˳���
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
static int IPAddWriteToPipe(unsigned int u32IPAddr)
{
	int  l_s32Ret = 0;
    int  l_s32PipeFd = -1;
	int  l_s32SendCount = 3;
    char l_arrs8Buffer[32] = {0};
	IP_ADDR_NUM_S l_stIpAddr = {0};

	/**�жϹܵ��Ƿ����**/
    if(access(PIPE_NAME, F_OK) == -1)
    {
        printf ("Create the fifo pipe.\n");
        l_s32Ret = mkfifo(PIPE_NAME, 0777);
 
        if(l_s32Ret != 0)
        {
            fprintf(stderr, "Could not create fifo %s\n", PIPE_NAME);
           	return l_s32Ret;
        }
    }

	/**�Է������ķ�ʽȥ�򿪹ܵ�**/
    l_s32PipeFd = open(PIPE_NAME, O_NONBLOCK|O_WRONLY);
    if(l_s32PipeFd !=-1)
    {
    	l_stIpAddr.u8StartFlag  = IP_START_FLAG;
		l_stIpAddr.u8EndFlag    = IP_END_FLAG;
		l_stIpAddr.u32IPAddrNum = u32IPAddr;
		memcpy(l_arrs8Buffer,&l_stIpAddr,sizeof(IP_ADDR_NUM_S));
		
		l_s32Ret = write(l_s32PipeFd, l_arrs8Buffer, sizeof(IP_ADDR_NUM_S));
		if(l_s32Ret == -1)
		{
			while((l_s32SendCount--)>0)
			{
				sleep(1);
				if(-1 != write(l_s32PipeFd, l_arrs8Buffer, sizeof(IP_ADDR_NUM_S)))
				{
					l_s32Ret = 0;
					break;
				}else
				{
					l_s32Ret = -1;
				}
			}			
		}	
    }
	else
	{
		printf("open pipe errror !\n");
		l_s32Ret  = -1;
	}
	
	close(l_s32PipeFd);
	
	return l_s32Ret;
}


/************************************************* 
Function:    lcb_connect  
Description: ���·�װconnet��������connect������Ӧ��
			 ��ȫһ��
Input: connect ϵͳ�����ķ���ֵ
Return:
Others: ����������У���IP��ַ��ʮ������ֵд�뵽�ܵ���
	ͨ��wsd_GetIpAddr�ӿڻ�ȡIPֵ����ʵ��ȥ��ϼ����̼�
	ͨ��
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int wsd_connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
	int l_s32Ret = 0;
	unsigned int l_u32IpAddr = 0; 
	struct sockaddr_in *l_stAddrIn = NULL;
		
	l_s32Ret = connect(sockfd,addr,addrlen);

	l_stAddrIn  = (struct sockaddr_in *)addr;
	l_u32IpAddr = l_stAddrIn->sin_addr.s_addr;

	IPAddWriteToPipe(l_u32IpAddr);

	return l_s32Ret;
}



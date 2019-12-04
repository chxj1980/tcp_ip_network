/************************************************************
*Copyright (C), 2017-2027,lcb0281at163.com lcb0281atgmail.com
*FileName: NetTrafficStati.cpp
*Date:     2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Description:��������ͳ��
*Others:
*History:
***********************************************************/
#include <string.h>
#include <stdio.h>       
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>

#include "IPAddrInterFace.h"
#include "NetTrafficStati.h"

#define IPTABLE_RES_FILE 	"/tmp/iptable_res"
//#define READ_IPTABLE		"iptables -n -v -L -t filter -x >/tmp/iptable_res"
#define READ_IPTABLE		"iptables -n -v -L -t filter -x >/var/iptable_res"

/******************************************************** 
Function:	 NetTrafficStati
Description: ���캯��
Input:	none
OutPut: none
Return: none
Others: 
Author: Caibiao Lee
Date:	2019-06-21
*********************************************************/
NetTrafficStati::NetTrafficStati()
{
	m_pstIpTable = &m_stIpTable;
	m_s32PipeFileID = -1;
	
	IpTable_Init(m_pstIpTable);
	memset(&m_stFlowCount,0,sizeof(m_stFlowCount));
}

/******************************************************** 
Function:	 ~NetTrafficStati
Description: ��������
Input:	none
OutPut: none
Return: none
Others: 
Author: Caibiao Lee
Date:	2019-06-21
*********************************************************/
NetTrafficStati::~NetTrafficStati()
{

}

/******************************************************** 
Function:	 NTS_Instance
Description: ��ȡ��̬����
Input:	none
OutPut: none
Return: none
Others: 
Author: Caibiao Lee
Date:	2019-06-21
*********************************************************/
NetTrafficStati *NetTrafficStati::NTS_Instance()
{
    static NetTrafficStati * pClsInstance = NULL;
    if (pClsInstance == NULL)
    {
        pClsInstance = new NetTrafficStati();
    }
    return pClsInstance;

}

/************************************************* 
Function:    IpTable_Init  
Description: ��ʼ��IP�б�
Input:   stIpTable   
Return: stIpTable
Others: 
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::IpTable_Init(IP_TABLE_S *stIpTable)
{
	memset((char*)stIpTable,0,sizeof(IP_TABLE_S));	
	stIpTable->u8InitFlag = 1;

	return 0;
}

/************************************************* 
Function:    IpTable_Check  
Description: ���IpNumIP�Ƿ��Ѿ�������IP�б���
Input:   IpNum ��Ҫ����IPֵ   
Return:  0 IP �б���û�и�IP����0���Ѿ����ڸ�IP
Others: 
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::IpTable_Check(IP_TABLE_S *stIpTable,unsigned int IpNum)
{
	if(NULL==stIpTable)
	{
		printf("Error:input stIpTable is NULL \n");
		return -1;
	}
	
	for(int i=0;i<stIpTable->u8UsedIpCount;i++)
	{
		/**�Ѿ����ڸ�IP**/
		if(IpNum==stIpTable->u32IPAddrNum[i])
		{
			return -1;
		}
	}
	
	return 0;
}

/************************************************* 
Function:    IpTable_Add  
Description: ����һ��IP��IP�б���
Input: stIpTable IP �б�, IpNum IP ��
Return:
Others: 0 ����ɹ�����0������ʧ��
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::IpTable_Add(IP_TABLE_S *stIpTable,unsigned int IpNum)
{
	if(NULL==stIpTable)
	{
		printf("Error:input stIpTable is NULL \n");
		return -1;
	}
	/**�ж�IP���Ƿ��ʼ��**/
	if(1!=stIpTable->u8InitFlag)
	{
		IpTable_Init(stIpTable);
	}
	
	/**���IpNum �Ƿ��Ѿ�������IP����**/
	if(0==IpTable_Check(stIpTable,IpNum))
	{	
		/**���IpNum ��IP����**/
		if(stIpTable->u8UsedIpCount < MAX_IP_COUNT)
		{
			stIpTable->u32IPAddrNum[stIpTable->u8UsedIpCount] = IpNum;
			stIpTable->u8UsedIpCount++;	
			printf("add IP %u OK \n",IpNum);
			return 0;
		}else
		{
			IpTable_Init(stIpTable);
			stIpTable->u32IPAddrNum[stIpTable->u8UsedIpCount] = IpNum;
			stIpTable->u8UsedIpCount++;	
			printf("add IP %u OK \n",IpNum);
			return 0;
		}
	}else
	{
		printf("ip %d is already set \n",IpNum);
		return -2;
	}
	
	return -3;

}

/************************************************* 
Function:    iptables_rulesAdd  
Description: ����IP��ӵ�iptables�Ĺ�����
Input: stIpTable IP �б�, IpNum IP ��
Return:
Others: 0 ����ɹ�����0������ʧ��
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::iptables_rulesAdd(struct sockaddr_in *addr)
{
	int  l_s32Ret = 0;
	char l_arrs8IPdotdec[20] = {0}; // ��ŵ��ʮ����IP��ַ
	char l_IptablesCmd[64] = {0};

	if(AF_INET!=addr->sin_family)//IPv6
	{
		return -1;
	}

	if(0==IpTable_Add(m_pstIpTable,addr->sin_addr.s_addr))
	{

		if(NULL!=inet_ntop(AF_INET,&addr->sin_addr,l_arrs8IPdotdec,16));
		{
			/**set input rule**/
			snprintf(l_IptablesCmd,sizeof(l_IptablesCmd),"iptables -I INPUT -s %s ",l_arrs8IPdotdec);
			printf("cmd : %s \n",l_IptablesCmd);
			system(l_IptablesCmd);
			
			/**set output rule**/
			memset(l_IptablesCmd,0,sizeof(l_IptablesCmd));
			snprintf(l_IptablesCmd,sizeof(l_IptablesCmd),"iptables -I OUTPUT -d %s ",l_arrs8IPdotdec);
			printf("cmd : %s \n",l_IptablesCmd);
			system(l_IptablesCmd);

			printf("add IP: %s is OK\n",l_arrs8IPdotdec);
			l_s32Ret = 0;
		}		
	}else
	{
		if(NULL!=inet_ntop(AF_INET,&addr->sin_addr,l_arrs8IPdotdec,16))
		{
			printf("IP: %s is in the Iptable \n",l_arrs8IPdotdec);
		}
		
		l_s32Ret = -1;
	}
	
	return l_s32Ret;

}


/************************************************* 
Function:    NTS_GetNetWorkCardFlow  
Description: ��ȡ��������
Input: u64Count
Output:u64Count
Return: 0 �ɹ�����0��ʧ��
Others: 
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::NTS_GetNetWorkCardFlow(unsigned long long *u64Count)
{
	int l_s32Ret = 0;
	FILE *l_pFd = NULL;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	unsigned long long l_u64ReadTx = 0;
	unsigned long long l_u64ReadRx = 0;

	l_pFd=fopen(NETWORK_CARD_TX,"r");

	if(NULL!=l_pFd)
	{
		if((read = getline(&line, &len, l_pFd)) != -1)
		{
			sscanf(line,"%lld ",&l_u64ReadTx);
			printf("read Tx %lld Byte \n",l_u64ReadTx);
		}

		if(NULL!=l_pFd)
		{
			fclose(l_pFd);
			l_pFd = NULL;
		}
	    
	    if (NULL!=line)
		{
			free(line);
			line = NULL;
		}
	}else
	{
		printf("open %s error ! \n",NETWORK_CARD_TX);
		l_s32Ret = -1;
	}

	l_pFd=fopen(NETWORK_CARD_RX,"r");
	if(NULL!=l_pFd)
	{
		if((read = getline(&line, &len, l_pFd)) != -1)
		{
			sscanf(line,"%lld ",&l_u64ReadRx);
			printf("read Rx %lld Byte \n",l_u64ReadRx);
		}

		if(NULL!=l_pFd)
		{
			fclose(l_pFd);
			l_pFd = NULL;
		}
	    
	    if (NULL!=line)
		{
			free(line);
			line = NULL;
		}
	}else
	{
		printf("open %s error ! \n",NETWORK_CARD_TX);
		l_s32Ret = -2;
	}

	*u64Count = l_u64ReadTx + l_u64ReadRx;

	return l_s32Ret;

}

/************************************************* 
Function:    NTS_ReadIptableIOByte  
Description: ��ȡIPTableͳ�Ƶ�������
Input: u64Count
Output:none
Return:0 �ɹ�����0��ʧ��
Others:
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::NTS_ReadIptableIOByte(unsigned long long *u64Count)
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	char str1[16] = {0};
	char str2[16] = {0};
	char str3[16] = {0};
	char str4[16] = {0};
	char str5[16] = {0};
	char str6[16] = {0};
	unsigned long long num1 = 0;
	unsigned long long num2 = 0;
	unsigned long long l_u64DataByte = 0;
	ssize_t read;
	
	unsigned long long  l_u64ToTalInputByte = 0;
	unsigned long long  l_u64ToTalOutputByte = 0;
	unsigned long long  l_u64ToTalNetByte = 0;
	
	char l_s8Status = -1;
	
	int i = 0;
	//printf("cmd = %s \n",READ_IPTABLE);
	system(READ_IPTABLE);
	
    fp = fopen(IPTABLE_RES_FILE, "r");
    if (NULL==fp)
	{
		printf("open file: %s error \n",IPTABLE_RES_FILE);
		return -1;
	}
       
    while ((read = getline(&line, &len, fp)) != -1) 
	{
		//sscanf(line,"%s %s %s %s %lld %s %lld %s ",str1,str2,str3,str4,&num1,str5,&num2,str6);
		//l_u64DataByte += num2;
		if(NULL!=strstr(line,"Chain INPUT"))
		{
			l_s8Status = 1;
			sscanf(line,"%s %s %s %s %lld %s %lld %s ",str1,str2,str3,str4,&num1,str5,&num2,str6);
			l_u64ToTalInputByte = num2;
			if((read = getline(&line, &len, fp)) != -1)
			{
				continue;
			}
		}
		
		if(NULL!=strstr(line,"Chain FORWARD"))
		{
			l_s8Status = 0;
			if((read = getline(&line, &len, fp)) != -1)
			{
				continue;
			}
		}
		
		if(NULL!=strstr(line,"Chain OUTPUT"))
		{
			l_s8Status = 2;
			sscanf(line,"%s %s %s %s %lld %s %lld %s ",str1,str2,str3,str4,&num1,str5,&num2,str6);
			l_u64ToTalOutputByte = num2;
			if((read = getline(&line, &len, fp)) != -1)
			{
				continue;
			}
		}
		
		if(l_s8Status>0)
		{
			if(NULL!=strstr(line,"all"))
			{
				//printf("%s ",line);
				sscanf(line,"%lld %lld %s %s %s %s %s %s ",&num1,&num2,str1,str2,str3,str4,str5,str6);
				//printf("num1 =%lld num2 = %lld \n",num1,num2);
				l_u64ToTalNetByte += num2;
			}
		}
    }
	
	if(NULL!=fp)
	{
		fclose(fp);
		fp = NULL;
	}
    
    if (NULL!=line)
	{
		free(line);
		line = NULL;
	}
	
	//printf("l_u64ToTalInputByte  = %lld \n",l_u64ToTalInputByte);
	//printf("l_u64ToTalOutputByte = %lld \n",l_u64ToTalOutputByte);
	//printf("l_u64ToTalNetByte    = %lld \n",l_u64ToTalNetByte);

	*u64Count = l_u64ToTalNetByte;
	
	return 0;
}

/************************************************* 
Function:    NTS_GetIpFromPipe  
Description: �ӹܵ��л�ȡIP��Ϣ���ж��Ƿ����IP�б���
	��������ڣ�����ӽ�IP�б������Iptable����
Input: none
Output:none
Return:0 �ɹ�����0��ʧ��
Others: ע������Ĺܵ����ڸý����У��Է������ķ�ʽ��
	�����Ժ��ܹرղ���������رգ�д�ܵ���򲻿�
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::NTS_GetIpFromPiPe(void)
{
	int  l_s32Ret = 0;
    int  l_s32PipeFd = -1;
	int  l_s32DataLen = -1;
	int  l_s32ReadPos = -1;
	int  l_s32SendCount = 3;
    char l_arrs8Buffer[128] = {0};
	IP_ADDR_NUM_S l_stIpAddr = {0};
	struct sockaddr_in stIpAdr = {0};
	struct sockaddr_in *stpIpAdr = &stIpAdr;

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

	if(-1 ==m_s32PipeFileID)
	{
		l_s32PipeFd = open(PIPE_NAME, O_NONBLOCK|O_RDONLY);
		m_s32PipeFileID = l_s32PipeFd;
	}
	/**�Է������ķ�ʽȥ�򿪹ܵ�**/
    
    if(m_s32PipeFileID !=-1)
    {
    	l_s32DataLen = 0;
		l_s32ReadPos = 0;
		l_s32DataLen = read(m_s32PipeFileID, l_arrs8Buffer, sizeof(l_arrs8Buffer));
		while(l_s32DataLen > 0)
		{
			if(l_s32DataLen>=sizeof(IP_ADDR_NUM_S))
			{
				memcpy(&l_stIpAddr,&l_arrs8Buffer[l_s32ReadPos],sizeof(IP_ADDR_NUM_S));
				l_s32ReadPos += sizeof(IP_ADDR_NUM_S);
				l_s32DataLen -= sizeof(IP_ADDR_NUM_S);

				if((IP_START_FLAG==l_stIpAddr.u8StartFlag)&&(IP_END_FLAG==l_stIpAddr.u8EndFlag))
				{
					stIpAdr.sin_family = AF_INET;  //���õ�ַ����
					//stIpAdr.sin_port = htons(800);  //���ö˿�
					stIpAdr.sin_addr.s_addr = l_stIpAddr.u32IPAddrNum;  //���õ�ַ
					//printf("IP ddr NUM = %u \n",l_stIpAddr.u32IPAddrNum);
					iptables_rulesAdd(stpIpAdr);
					l_s32Ret = 0;
				}
			}
		}
    }
	else
	{
		printf("open pipe errror !\n");
		l_s32Ret  = -1;
	}

	/**���ر�**/
	//close(m_s32PipeFileID);

	return 0;
}

/************************************************* 
Function:    NTS_GetIpFromPipe  
Description: �ӹܵ��л�ȡIP��Ϣ���ж��Ƿ����IP�б���
	��������ڣ�����ӽ�IP�б������Iptable����
Input: none
Return:
Others: �ú�����Ҫ��ҵ��������ڵ��ã�����1S����һ��
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::NTS_AddIpToIPTable(void)
{
	NTS_GetIpFromPiPe();	
}


/************************************************* 
Function:    NTS_UpdateFlowData  
Description: ����ͳ�Ƹ���
Input: none
Return:
Others: �ú�����Ҫ��ҵ��������ڵ��ã�����1S����һ��
Author: Caibiao Lee
Date:   2019.06.21
*************************************************/
int NetTrafficStati::NTS_UpdateFlowData(void)
{
	int l_s32Ret = 0;
	unsigned long long l_u64NetWorkCardFlow  = 0;
	unsigned long long *l_pu64NetWorkCardFlow = &l_u64NetWorkCardFlow;

	unsigned long long l_u64IptableIOByte  = 0;
	unsigned long long *l_pu64IptableIOByte = &l_u64IptableIOByte;

	if(0==NTS_GetNetWorkCardFlow(l_pu64NetWorkCardFlow))
	{
		printf("l_pu64NetWorkCardFlow = %lld \n",l_u64NetWorkCardFlow);
		m_stFlowCount.u64NetWorkCount = l_u64NetWorkCardFlow;
	}

	if(0==NTS_ReadIptableIOByte(l_pu64IptableIOByte))
	{
		printf("l_pu64IptableIOByte = %lld \n",l_u64IptableIOByte);
		m_stFlowCount.u64IptableCount= l_u64IptableIOByte;
	}
	
}



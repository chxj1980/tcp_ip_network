/************************************************************
*Copyright (C), 2017-2027,lcb0281at163.com lcb0281atgmail.com
*FileName: NetTrafficStati.h
*Date:     2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Description:��������ͳ��
*Others:
*History:
***********************************************************/
#ifndef __NET_TRAFFIC_STATI_H_
#define __NET_TRAFFIC_STATI_H_

/**��·�������շ���ȡ��ַ**/
#define NETWORK_CARD_TX "/sys/class/net/ens33/statistics/tx_bytes"   
#define NETWORK_CARD_RX "/sys/class/net/ens33/statistics/rx_bytes"  

/**�������ݱ����ַ**/
#define NET_FLOW_REC_PATH "/mnt/NetTrafficStatistics.rec" 

/**���ο������֧�ּ�¼IP����**/
#define MAX_IP_COUNT 			64  


/**��������Iptable��IP�б�**/
typedef struct
{
	unsigned int  u32IPAddrNum[MAX_IP_COUNT];
	unsigned char u8UsedIpCount;
	unsigned char u8InitFlag;
}IP_TABLE_S;

/**��������**/
typedef struct 
{
	char s8Date[3];
	unsigned long long u64NetWorkCount; /**��������**/			
	unsigned long long u64IptableCount; /**�������**/				
}NetFlowCount_S;


class NetTrafficStati
{
public:
	NetTrafficStati();
	~NetTrafficStati();
	static NetTrafficStati *NTS_Instance();
private :
	int IpTable_Init(IP_TABLE_S *stIpTable);
	int IpTable_Check(IP_TABLE_S *stIpTable,unsigned int IpNum);
	int IpTable_Add(IP_TABLE_S *stIpTable,unsigned int IpNum);
	int iptables_rulesAdd(struct sockaddr_in *addr);

	int NTS_GetNetWorkCardFlow(unsigned long long *u64Count);
	int NTS_ReadIptableIOByte(unsigned long long *u64Count);

	int NTS_GetIpFromPiPe(void);
	
public:		
	int NTS_AddIpToIPTable(void);
	int NTS_UpdateFlowData(void);

private :
	int m_s32PipeFileID;
	IP_TABLE_S  m_stIpTable;
	IP_TABLE_S* m_pstIpTable;
	NetFlowCount_S m_stFlowCount;

};

#endif




#include "stdafx.h"
#include "pub.h"
#include <tchar.h>
#include <string.h>
//#include <windows.h>
#include "datasafe.h"
#include "db_table.h"
#include   <fstream> 
#include <time.h>
//#include <iostream>
//#include <atltime.h>
using namespace std;

STRUCT_SYSCONFIG sysconfig={0};

char *titleMsg[]={
"audit.dbtype=",\
"audit.ip=",\
"audit.user=",\
"audit.pwd=",\
"audit.dbname=",\
"audit.port=",\
"osc.dbtype=",\
"osc.ip=",\
"osc.user=",\
"osc.pwd=",\
"osc.dbname=",\
"osc.port=",\
"node.code=",\
"node.level=",\
"node.monitorPort=",\
"node.videoPath=",\
"node.auditPath=",\
"node.maxOutRows=",\
"node.maxThreads=",\
"node.maxCommitRows=",\
"node.warningTimes_1=",\
"node.warningTimes_2=",\
"node.warningTimes_3=",\
"node.proxyIp=",\
"node.proxyConnectPort=",\
NULL
};

unsigned int configFieldsLen[]={sizeof(sysconfig.auditConn.dbType),sizeof(sysconfig.auditConn.dbHost),sizeof(sysconfig.auditConn.dbUser),\
sizeof(sysconfig.auditConn.dbPwd),sizeof(sysconfig.auditConn.dbName),sizeof(sysconfig.auditConn.dbPort),\
sizeof(sysconfig.oscConn.dbType),sizeof(sysconfig.oscConn.dbHost),sizeof(sysconfig.oscConn.dbUser),\
sizeof(sysconfig.oscConn.dbPwd),sizeof(sysconfig.oscConn.dbName),sizeof(sysconfig.oscConn.dbPort),\
sizeof(sysconfig.code),sizeof(sysconfig.level),sizeof(sysconfig.monitorPort),\
sizeof(sysconfig.videoPath),sizeof(sysconfig.auditPath),sizeof(sysconfig.maxOutRows),\
sizeof(sysconfig.maxThreads),sizeof(sysconfig.maxCommitRows),sizeof(sysconfig.warningTimes_1),\
sizeof(sysconfig.warningTimes_2),sizeof(sysconfig.warningTimes_3),\
sizeof(sysconfig.proxyIp),sizeof(sysconfig.proxyConnectPort)};


unsigned __int64 GetNTime( void ) 
{
	LARGE_INTEGER nFrequency;
	LARGE_INTEGER nStartCounter;
	if(::QueryPerformanceFrequency(&nFrequency))
	{
		::QueryPerformanceCounter(&nStartCounter);
	}
	__int64 ret=0;
	//ret=nStartCounter.LowPart;
	//srand((int)time(0));//��ֹ�߲������������QuadPartֵ��ͬ
	ret = nStartCounter.QuadPart+rand()%10000;
	return ret;
}

void get32Id(char *id)
{
	char buff[1024];
	char tmpId[33];
	int len=0;

	memset(tmpId,0,sizeof(tmpId));
//	sprintf(tmpId,"%d",GetTickCount());
	sprintf(tmpId,"%I64d",GetNTime());

	memset(buff,0,sizeof(buff));
	DataSafe.encrypt(tmpId,strlen(tmpId),buff);
	len=strlen(buff);
	if(len<32)
	{
		len=32;
	}
	strcpy(id,&buff[len-32]); 
}

int WharToMByte(wchar_t * wideChar,char* narrowChar) 
{
	int nLength = WideCharToMultiByte(CP_ACP,NULL,wideChar,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(CP_ACP,NULL,wideChar,-1,narrowChar,nLength,NULL,NULL);
	return 1 ;
}

BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return FALSE;
	}
	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);  
	return TRUE;
}

typedef struct _IPHeader        // 20�ֽڵ�IPͷ   
{  
    UCHAR     iphVerLen;      // �汾�ź�ͷ���ȣ���ռ4λ��   
    UCHAR     ipTOS;          // ��������    
    USHORT    ipLength;       // ����ܳ��ȣ�������IP���ĳ���   
    USHORT    ipID;           // �����ʶ��Ωһ��ʶ���͵�ÿһ�����ݱ�   
    USHORT    ipFlags;        // ��־   
    UCHAR     ipTTL;          // ����ʱ�䣬����TTL   
    UCHAR     ipProtocol;     // Э�飬������TCP��UDP��ICMP��   
    USHORT    ipChecksum;     // У���   
    ULONG     ipSource;       // ԴIP��ַ   
    ULONG     ipDestination;  // Ŀ��IP��ַ   
} IPHeader, *PIPHeader;   

typedef struct icmp_hdr
{
    unsigned char   icmp_type;		// ��Ϣ����
    unsigned char   icmp_code;		// ����
    unsigned short  icmp_checksum;	// У���

	// �����ǻ���ͷ
    unsigned short  icmp_id;		// ����Ωһ��ʶ�������ID�ţ�ͨ������Ϊ����ID
    unsigned short  icmp_sequence;	// ���к�
    unsigned long   icmp_timestamp;     // ʱ���
} ICMP_HDR, *PICMP_HDR;

BOOL SetTimeout(SOCKET s, int nTime, BOOL bRecv)
{
	int ret = ::setsockopt(s, SOL_SOCKET,bRecv ? SO_RCVTIMEO : SO_SNDTIMEO, (char*)&nTime, sizeof(nTime));
	return ret != SOCKET_ERROR;
}
USHORT checksum(USHORT* buff, int size)
{
	unsigned long cksum = 0;
	while(size>1)
	{
		cksum += *buff++;
		size -= sizeof(USHORT);
	}
	// ������
	if(size)
	{
		cksum += *(UCHAR*)buff;
	}
	// ��32λ��chsum��16λ�͵�16λ��ӣ�Ȼ��ȡ��
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);			
	return (USHORT)(~cksum);
}

int getLocalAddrByPing(char *szDestIp,char *localAddr)
{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		SOCKET sRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);  

		// ���ý��ճ�ʱ  
		SetTimeout(sRaw, 3000, TRUE);  
		struct in_addr sin_addr;  

		// ����Ŀ�ĵ�ַ  
		SOCKADDR_IN dest;  
		dest.sin_family = AF_INET;  
		dest.sin_port = htons(0);  
		dest.sin_addr.S_un.S_addr = inet_addr(szDestIp);  

		// ����ICMP���  
		char buff[sizeof(ICMP_HDR) + 32];  
		ICMP_HDR* pIcmp = (ICMP_HDR*)buff;  

		// ��дICMP������ݣ�����һ��ICMP����  
		pIcmp->icmp_type = 8;      
		pIcmp->icmp_code = 0;  
		pIcmp->icmp_id = (USHORT)GetCurrentProcessId();  
		pIcmp->icmp_checksum = 0;  
		pIcmp->icmp_sequence = 0;  

		// ������ݲ��֣�����Ϊ����  
		memset(&buff[sizeof(ICMP_HDR)], 'E', 32);  

		// ��ʼ���ͺͽ���ICMP���  
		USHORT  nSeq = 0;  
		char recvBuf[1024];  
		SOCKADDR_IN from;  
		int nLen = sizeof(from);  
//		while(TRUE)  
		{  
//			static int nCount = 0;  
			int nRet;  

			// ping����  
//			if(nCount++ == 1000)  
//				break;  

			pIcmp->icmp_checksum = 0;  
			pIcmp->icmp_timestamp = GetTickCount();  
			pIcmp->icmp_sequence = nSeq++;  
			pIcmp->icmp_checksum = checksum((USHORT*)buff, sizeof(ICMP_HDR) + 32);  
			nRet = sendto(sRaw, buff, sizeof(ICMP_HDR) + 32, 0, (SOCKADDR *)&dest, sizeof(dest));  
			if(nRet == SOCKET_ERROR)  
			{  
				printf(" sendto() failed: %d \n", ::WSAGetLastError());  
				closesocket(sRaw);
				return -1;  
			}
			nRet = recvfrom(sRaw, recvBuf, 1024, 0, (sockaddr*)&from, &nLen);
			if(nRet == SOCKET_ERROR)
			{  
				if(WSAGetLastError() == WSAETIMEDOUT)  
				{  
					printf(" timed out\n");  
//					continue;  
				}
				printf("recvfrom() failed: %d\n", WSAGetLastError());  
				closesocket(sRaw);
				return -1;
			}  

			// ���濪ʼ�������յ���ICMP���
/*
			int nTick = ::GetTickCount();  
			if(nRet < sizeof(IPHeader) + sizeof(ICMP_HDR))  
			{  
				printf(" Too few bytes from %s \n", inet_ntoa(from.sin_addr));  
			}  
*/			
			if(localAddr!=NULL)
			{
				PIPHeader pHeader=(PIPHeader)recvBuf;
				
				sin_addr.S_un.S_addr=pHeader->ipSource;//ipDestination;
				
				sprintf(localAddr,"%d.%d.%d.%d",
						   sin_addr.S_un.S_un_b.s_b1,
						   sin_addr.S_un.S_un_b.s_b2,
						   sin_addr.S_un.S_un_b.s_b3,
						   sin_addr.S_un.S_un_b.s_b4);
			}

/*
			// ���յ��������а���IPͷ��IPͷ��СΪ20���ֽڣ����Լ�20�õ�ICMPͷ  
			// (ICMP_HDR*)(recvBuf + sizeof(IPHeader));  
			ICMP_HDR* pRecvIcmp = (ICMP_HDR*)(recvBuf + 20);   
			if(pRecvIcmp->icmp_type != 0)    // ����  
			{  
				printf(" nonecho type %d recvd \n", pRecvIcmp->icmp_type);  
				return -1;  
			}

			if(pRecvIcmp->icmp_id != GetCurrentProcessId())  
			{  
				printf(" someone else's packet! \n");  
				return -1;  
			}  

			printf("�� %s ���� %d �ֽ�:", inet_ntoa(from.sin_addr),nRet);  
			printf(" ���ݰ����к� = %d. \t", pRecvIcmp->icmp_sequence);  
			printf(" ��ʱ��С: %d ms", nTick - pRecvIcmp->icmp_timestamp);  
			printf(" \n");  
*/
			// ÿһ�뷢��һ�ξ�����  
//			Sleep(1000);
		}

		closesocket(sRaw);
		WSACleanup();
		return 0;
}

void getRemoteLogicDiskInfo(char *drivers)
{
	int DType;
	int si = 0;
	char DStr[1024];
	int DSLength = GetLogicalDriveStrings(0,NULL);//ͨ���ú�����ȡ�����������ַ�����Ϣ����
	memset(DStr,0,sizeof(DStr));
	GetLogicalDriveStrings(DSLength,(LPTSTR)DStr);
	for(int i=0,j=0;i<DSLength/4;++i)
	{
		DType = GetDriveType((LPTSTR)DStr+i*4);//ͨ���ú�����ȡ���̵�����
		if(DType == DRIVE_REMOTE)
		{
			drivers[j++]=DStr[si];
		}
		si+=8;

		if(j>99)
		{
			break;
		}
	}
}

//extern "C" _declspec(dllexport)

int getLogicDiskInfo()
{
	int DType;
	int si = 0;
	BOOL result;
	unsigned _int64 i64FreeBytesToCaller;
	unsigned _int64 i64TotalBytes;
	unsigned _int64 i64FreeBytes;
	float totalSize;//�ܿռ�
	float usableSize;//���ÿռ�
	char DStr[1024];
	int DSLength = GetLogicalDriveStrings(0,NULL);//ͨ���ú�����ȡ�����������ַ�����Ϣ����
//	cout<<"����Ϊ��"<<DSLength<<endl;
//	ofstream in;//���ļ�д����
//	in.open("D:\\disk\\info.txt",ios::trunc);//ios::trunc��ʾ�ڴ��ļ�ǰ���ļ���գ�������д����������ļ��������򴴽�
//	char* DStr = (char *)malloc(DSLength*sizeof(char)); //new char[DSLength];
	memset(DStr,0,sizeof(DStr));
	GetLogicalDriveStrings(DSLength,(LPTSTR)DStr);
	for(int i=0;i<DSLength/4;++i)
	{
		char dir[3] = {DStr[si],':','\\'};
		//cout<<"��������Ϊ��"<<dir[0]<<dir[1]<<dir[2]<<endl;
		char str[3] = {0,0,0};
		str[0] = dir[0];
		str[1] = dir[1];
		string dirName = str;
		DType = GetDriveType((LPTSTR)DStr+i*4);//ͨ���ú�����ȡ���̵�����
		string driverType;
		if(DType == DRIVE_FIXED)
		{
			driverType = "���ش���";
			//cout<<driverType<<endl;
		}
		else if(DType == DRIVE_CDROM)
		{
			driverType = "����";
			//cout<<driverType<<endl;
		}
		else if(DType == DRIVE_REMOVABLE)
		{
			driverType = "���ƶ�����";
			//cout<<driverType<<endl;
		}
		else if(DType == DRIVE_REMOTE)
		{
			driverType = "�������";
			//cout<<driverType<<endl;
		}
		else if(DType == DRIVE_RAMDISK)
		{
			driverType = "����RAM����";
			//cout<<driverType<<endl;
		}
		else if(DType == DRIVE_UNKNOWN)
		{
			driverType = "δ֪�豸";
			//cout<<driverType<<endl;
		}
		
		if(dirName.compare("C:")==0)//������ΪC��ʱ
		{
			result = GetDiskFreeSpaceEx(_T("C:"),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);//��ȡ���̵Ŀռ�״̬
			if(result)
			{
				totalSize = (float)i64TotalBytes/1024/1024/1024;
				usableSize = (float)i64FreeBytesToCaller/1024/1024/1024;
				//cout<<"�ܿռ�Ϊ��"<<totalSize<<"GB"<<endl;
				//cout<<"���ÿռ�Ϊ��"<<usableSize<<"GB"<<endl;
				//cout<<"=============================================================="<<endl;
			}
			else
			{
				//cout<<"δ��⵽�����豸"<<endl;
			}
		}
		else if(dirName.compare("D:")==0)//������ΪD��ʱ
		{
			result = GetDiskFreeSpaceEx(_T("D:"),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);//��ȡ���̵Ŀռ�״̬
			if(result)
			{
				totalSize = (float)i64TotalBytes/1024/1024/1024;
				usableSize = (float)i64FreeBytesToCaller/1024/1024/1024;
				//cout<<"�ܿռ�Ϊ��"<<totalSize<<"GB"<<endl;
				//cout<<"���ÿռ�Ϊ��"<<usableSize<<"GB"<<endl;
				//cout<<"=============================================================="<<endl;
			}
			else
			{
				//cout<<"δ��⵽�����豸"<<endl;
			}
		}
		else if(dirName.compare("E:")==0)//������ΪE��ʱ
		{
			result = GetDiskFreeSpaceEx(_T("E:"),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);//��ȡ���̵Ŀռ�״̬
			if(result)
			{
				totalSize = (float)i64TotalBytes/1024/1024/1024;
				usableSize = (float)i64FreeBytesToCaller/1024/1024/1024;
				//cout<<"�ܿռ�Ϊ��"<<totalSize<<"GB"<<endl;
				//cout<<"���ÿռ�Ϊ��"<<usableSize<<"GB"<<endl;
				//cout<<"=============================================================="<<endl;
			}
			else
			{
				//cout<<"δ��⵽�����豸"<<endl;
			}
		}
		else if(dirName.compare("F:")==0)//������ΪF��ʱ
		{
			result = GetDiskFreeSpaceEx(_T("F:"),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);//��ȡ���̵Ŀռ�״̬
			if(result)
			{
				totalSize = (float)i64TotalBytes/1024/1024/1024;
				usableSize = (float)i64FreeBytesToCaller/1024/1024/1024;
				//cout<<"�ܿռ�Ϊ��"<<totalSize<<"GB"<<endl;
				//cout<<"���ÿռ�Ϊ��"<<usableSize<<"GB"<<endl;
				//cout<<"=============================================================="<<endl;
			}
			else
			{
				//cout<<"δ��⵽�����豸"<<endl;
			}
		}
		else if(dirName.compare("G:")==0)//������ΪE��ʱ
		{
			result = GetDiskFreeSpaceEx(_T("G:"),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);//��ȡ���̵Ŀռ�״̬
			if(result)
			{
				totalSize = (float)i64TotalBytes/1024/1024/1024;
				usableSize = (float)i64FreeBytesToCaller/1024/1024/1024;
				//cout<<"�ܿռ�Ϊ��"<<totalSize<<"GB"<<endl;
				//cout<<"���ÿռ�Ϊ��"<<usableSize<<"GB"<<endl;
				//cout<<"=============================================================="<<endl;
			}
			else
			{
				//cout<<"δ��⵽�����豸"<<endl;
			}
		}
		si+=8;
	}
	
	if(NULL != DType)
	{
		return DType;
	}
	return -1;
}



//wchar->byte(ansi)
DWORD WharToMByteANSI(wchar_t *wideChar,char* narrowChar)
{
	int nLength = WideCharToMultiByte(CP_ACP,NULL,wideChar,-1,NULL,0,NULL,NULL);
	return WideCharToMultiByte(CP_ACP,NULL,wideChar,-1,narrowChar,nLength,NULL,NULL);
}
//wchar->byte(utf8)
DWORD WharToMByteUTF8(wchar_t *wideChar,char* narrowChar)
{
	int nLength = WideCharToMultiByte(CP_UTF8,NULL,wideChar,-1,NULL,0,NULL,NULL);
	return WideCharToMultiByte(CP_UTF8,NULL,wideChar,-1,narrowChar,nLength,NULL,NULL);
}
//byte(ansi)->wchar
DWORD MByteANSIToWChar(char *lpcszStr, wchar_t *lpwszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return 0;
	}
	return MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
}
//byte(utf8)->wchar
DWORD MByteUTF8ToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return 0;
	}
	return MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, lpwszStr, dwMinSize);
}

//byte(ansi)->wchar->byte(utf8)
DWORD MByteANSIToMByteUTF8(char *lpcszStr, int len, char* utf8Char)
{
//	wchar_t lpwszStr[2048]={0};
    DWORD ret=0;
  	DWORD dwMinSize;
	wchar_t *lpwszStr=new wchar_t[2*len+1];
    memset(lpwszStr,0,sizeof(wchar_t)*(2*len+1));
    dwMinSize=MByteANSIToWChar(lpcszStr,lpwszStr,(sizeof(wchar_t)*(2*len+1))/sizeof(lpwszStr[0]));
    if(dwMinSize<(2*len+1))
	{
         ret=WharToMByteUTF8(lpwszStr,utf8Char);   //WChar to UTF8
	}
    delete lpwszStr;
	return ret;
}
//byte(utf8)->wchar->byte(ansi)
DWORD MByteUTF8ToMByteANSI(char* utf8Char,int len,char *lpcszStr)
{
//	wchar_t lpwszStr[2048]={0};
    DWORD ret=0;
    DWORD dwMinSize;
	wchar_t *lpwszStr=new wchar_t[2*len+1];
    memset(lpwszStr,0,sizeof(wchar_t)*(2*len+1));
    dwMinSize=MByteUTF8ToWChar(utf8Char,lpwszStr,(sizeof(wchar_t)*(2*len+1))/sizeof(lpwszStr[0]));
    if(dwMinSize<2*len+1)
	{
         ret= WharToMByteANSI(lpwszStr,lpcszStr);   //WChar to UTF8
	}
    delete lpwszStr;
	return ret;
}

void loadCfgFile(char *cfgFile)
{
	//������ִ��loadCfgFile
	 if (strlen(sysconfig.auditConn.dbType) > 0)
	 {
		 return;
	 }
     char buf[1024];
	 char preSufix[50];
	 char preSufixBuf[2048];
	 memset(&sysconfig,0,sizeof(STRUCT_SYSCONFIG));
	 memset(preSufix,0,sizeof(preSufix));

	 ifstream infile(cfgFile);
     if(!infile) return;

     infile.seekg(0);
     while(!infile.eof()){
        memset(buf,0,sizeof(buf));
        infile.getline(buf,sizeof(buf)-1);        
		char *op=strstr(buf,"[audit]");
		if(op!=NULL)
		{
			strcpy(preSufix,"audit");
			continue;
		}
		op=strstr(buf,"[osc]");
		if(op!=NULL)
		{
			strcpy(preSufix,"osc");
			continue;
		}
		op=strstr(buf,"[node]");
		if(op!=NULL)
		{
			strcpy(preSufix,"node");
			continue;
		}

		int i=0;
		int offset=0;
		char *offsetOp=sysconfig.auditConn.dbType;
		memset(preSufixBuf,0,sizeof(preSufixBuf));
		sprintf(preSufixBuf,"%s.%s",preSufix,buf);
		while(titleMsg[i]!=NULL)
		{
			op=strstr(preSufixBuf,titleMsg[i]);
			if(op!=NULL)
			{
				op += strlen(titleMsg[i]);
				int len=strlen(op);
				if(len>=configFieldsLen[i])
				{
					len=configFieldsLen[i]-1;
				}
				memcpy((char *)(offsetOp+offset),op,len);            
				break;
			}
			offset += configFieldsLen[i];
			i++;
		}
     }
     infile.close();
}


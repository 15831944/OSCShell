// OSCShellApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "../OSCShell/OSCShell.h"
#include "screen2swf.h"
#include <atltime.h>
#include <TlHelp32.h>
#include "sessionrdp.h"
//#include <iostream>
//#include <fstream>
//using namespace std;
//#include <Winuser.h>
#include "db_table.h"
#include "sessionMgr.h"
#include "resource.h"
#define  WARNTIME  300
#define  WARNTIME1 180
#define  WARNTIME2 60
#define  ID_SWITCH_CONSOLEMODE 0xE00F 
#define  OSCAUDITWINDOW "OscAuditServerApp"

void GetSysInfo();
//���ֽ��������ӽ���֮�󣬳������˳�
void getChildProcessID(IN DWORD parentProcessID,OUT DWORD * childProcessIDs,IN int MAXIDS,OUT int& childsCount );
typedef struct
{
    char title[10];
	char msg[1024];
	UINT type;
}STRUCT_MSG;

static HHOOK  g_hook = NULL;
static HWND OscHandle=NULL;
static NOTICE_STRUCT noticeMsg;

static HANDLE hProcess=NULL;
static HANDLE appThreadHandle=NULL;
static HANDLE sessionThreadHandle=NULL;

static int isExitSession=0;

char msg1[]					= "��Դ����ʱ������";
char msg1_english[]			= "resource access time limit";
char msg2[]					= "�������ʱ������";
char msg2_english[]			= "apply access time limit";
char msg3[]					= "����ʱ������";
char msg3_english[]			= "policy time limit";
char msg4[]					= "��������ʱ������";
char msg4_english[]			= "apply password time limit";
char msg5[]					= "�˺��ѱ��������롢ʱ�䵽��";
char msg5_english[]			= "account has been password apply��the end of time";
char msg6[]					= "Զ�̷��ʽ���%dСʱ�������";
char msg6_english[]			= "remote access at the end of %d hours";
char msg7[]					= "Զ�̷��ʽ���%d���Ӻ������";
char msg7_english[]			= "remote access at the end of %d minute";
char msg8[]					= "Զ�̷��ʽ���%d���Ӻ������";
char msg8_english[]			= "remote access at the end of %d second";
char msg9[]					= "������������ϵ����Ա��";
char msg9_english[]			= "if you have any questions please contact administrator��";
char msg10[]				= "Զ�̷��ʽ��ڰ�Сʱ�������";
char msg10_english[]		= "remote access at the end of halfhours";
char msg11[]				= "�뱸�ݺ�����";
char msg11_english[]		= "please backup your data";
char msg12[]				= "�뱸�ݺ����ݻ�����ʱ����";
char msg12_english[]		= "please backup your data or apply for delay ";
char msg13[]				= "�˺������ѱ����롢���ʽ���ǰ�����ʱ����Ϊ%ld���ӣ�";
char msg13_english[]		= "account has been password apply��visit before the end of the maximum delay for %ld minutes";

void CheckTimePolicy();

DWORD static WINAPI MsgShowThread(LPVOID lparam)
{
	STRUCT_MSG *msgHandle=(STRUCT_MSG *)lparam;
	HWND hWnd = GetForegroundWindow();
	MessageBox(hWnd,msgHandle->msg,msgHandle->title,msgHandle->type); 
	return 0;
}
DWORD static WINAPI MsgSFTPShowThread(LPVOID lparam)
{
	HWND hWnd = GetForegroundWindow();
	MessageBox(hWnd,"SFTP������,�����ĵȴ�...","��ʾ",MB_ICONWARNING|MB_RETRYCANCEL|MB_TOPMOST); 
	return 0;
}
void static MsgSFTPShow()
{
    DWORD  dwThreadId;
	CreateThread(NULL,NULL,MsgSFTPShowThread,NULL,0,&dwThreadId);
}
void static MsgShow(char *msg,char *title,UINT type)
{
	STRUCT_MSG msgHandle;
	memset(&msgHandle,0,sizeof(STRUCT_MSG));
	strcpy(msgHandle.msg,msg);
	strcpy(msgHandle.title,title);
	msgHandle.type=type;
    DWORD  dwThreadId;
	CreateThread(NULL,NULL,MsgShowThread,&msgHandle,0,&dwThreadId);
}

void ShowWarning(char *msg)
{
   MsgShow(msg,"warning",MB_ICONWARNING|MB_OK|MB_TOPMOST);
}

DWORD WINAPI AnswerThread(LPVOID lparam)
{
	HANDLE handle=(HANDLE)lparam;
	int nhandles=1;
	DWORD  dw=WaitForSingleObject(handle,INFINITE);
	WriteLog("[AnswerThread]Ӧ�ùر�!");
	if(paraSession.isLoop() && isExitSession==0)
	{
		if(sessionThreadHandle!=NULL)
		{
			CloseHandle(sessionThreadHandle);
			sessionThreadHandle=NULL;
		}
		//WriteLog("[AnswerThread]ע���Ự!");
		//paraSession.ExitOscShell_Session(1);
		//�߳���ִ�У��޷���֤���߳��˳�֮ǰִ�����
		paraSession.ExitOscShell(1);
	}
	WriteLog("[AnswerThread]�ȴ�ע���Ự!");
	return  0;
}
struct CHILDPROCESS
{
	HANDLE childHandles[16];//�ӽ���ID
	int childsCount;//�ӽ�����
};
int processIdToHandle(DWORD* processId,HANDLE* handles,int IdCount)
{
	int num = 0;
	if (processId == NULL || handles == NULL)
	{
		return 0;
	}
	for (int i=0;i<IdCount;i++)
	{
		handles[i] = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId[i]);
		if (handles[i] == NULL)
		{
			char errMsg[128];
			memset(errMsg,0,sizeof(errMsg));
			sprintf(errMsg,"OpenProcess[%d]failed!(%s)",processId[i],GetLastError());
			WriteLog(errMsg);
		}
		else
		{
			num++;
		}
	}
	return num;
}
//��������ӽ����˳�
DWORD WINAPI AnswerThreadEx(LPVOID lparam)
{
    CHILDPROCESS *childs = (CHILDPROCESS*)lparam;
	DWORD dw = WaitForMultipleObjects(childs->childsCount,childs->childHandles,TRUE,INFINITE);
	for (int i=0;i<childs->childsCount;i++)
	{
		CloseHandle(childs->childHandles[i]);
	}
	//DWORD  dw=WaitForSingleObject(handle,INFINITE);
	WriteLog("[AnswerThread]Ӧ�ùر�!");
	if(paraSession.isLoop() && isExitSession==0)
	{

		if(sessionThreadHandle!=NULL)
		{
			CloseHandle(sessionThreadHandle);
			sessionThreadHandle=NULL;
		}
		WriteLog("[AnswerThread]ע���Ự!");
		paraSession.ExitOscShell_Session(1);
	}
	WriteLog("[AnswerThread]�ȴ�ע���Ự!");
	return  0;
}
void RestartService()
{
	ShellExecute(NULL, "open", "cmd.exe", "/c net stop NFS & net start NFS", "", SW_HIDE);
	return;
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		WriteLog("[WM_DESTROY]");
		PostQuitMessage(0);
		break;
	case WM_ENDSESSION:
	case WM_QUERYENDSESSION:
		{
			WriteLog("[WM_ENDSESSION��WM_QUERYENDSESSION]�Ự��ǿ�ƽ�����");
			//MessageBox(NULL,"�Ự��ǿ�ƽ���","����",MB_ICONWARNING|MB_OK|MB_TOPMOST);
			//if(hProcess != NULL)
			//	TerminateProcess(hProcess,0);
			paraSession.ExitOscShell(0);

			// �ȴ�����ʱ���ִ���˳�������ʹ¼���߳��г����ʱ�������˳�������ᵼ��¼���ļ��޷���
			Sleep(4000);
			PostQuitMessage(0);
			WriteLog("[WM_ENDSESSION��WM_QUERYENDSESSION]ϵͳǿ���˳���");
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

DWORD static WINAPI ShowDisableWindowThread(LPVOID lparam)
{
	//////////////////////////////////////////////////[]
	WriteLog("[main]�������ش��ڷ�ֹǿ�ƽ������̵���¼����Ʋ�����==========================");
	HINSTANCE hInstance;  
	hInstance=GetModuleHandle(NULL);  
	WNDCLASS Draw;  
	Draw.cbClsExtra = 0;  
	Draw.cbWndExtra = 0;  
	Draw.hCursor = LoadCursor(hInstance, IDC_ARROW);;  
	Draw.hIcon = LoadIcon(hInstance, IDI_APPLICATION);;  
	Draw.lpszMenuName = NULL;  
	Draw.style = CS_HREDRAW | CS_VREDRAW;  
	Draw.hbrBackground = (HBRUSH)COLOR_WINDOW;  
	Draw.lpfnWndProc = WindowProc;  
	Draw.lpszClassName = _T("DDraw");  
	Draw.hInstance = hInstance;  
	RegisterClass(&Draw);  
	HWND hwnd = CreateWindow(    
		_T("DDraw"),           //����ע���������Ҫ��ȫһ��    
		_T("oscshell"),  //���ڱ�������    
		WS_OVERLAPPEDWINDOW, //���������ʽ    
		38,             //��������ڸ�����X����    
		20,             //��������ڸ�����Y����    
		480,                //���ڵĿ��    
		250,                //���ڵĸ߶�    
		NULL,               //û�и����ڣ�ΪNULL    
		NULL,               //û�в˵���ΪNULL    
		hInstance,          //��ǰӦ�ó����ʵ�����    
		NULL);              //û�и������ݣ�ΪNULL    
	ShowWindow(hwnd, SW_HIDE);      
	UpdateWindow(hwnd); 

	MSG msg;    
	while(GetMessage(&msg, NULL, 0, 0))    
	{    
		TranslateMessage(&msg);    
		DispatchMessage(&msg);    
	}  

	return 0;
}
void static ShowDisableWindow()
{
	DWORD  dwThreadId;
	CreateThread(NULL,NULL,ShowDisableWindowThread,NULL,0,&dwThreadId);
}
HANDLE singleInstanceSem = NULL;
void singleSemporeWatcher()
{
	WriteLog("��ʼ���RDP�Ự��");
	WaitForSingleObject(singleInstanceSem,INFINITE);
	WriteLog("[singleSemporeWatcher]����RDP�Ự�������˳���ǰ�Ự��");
	if(singleInstanceSem != NULL)
		CloseHandle(singleInstanceSem);
	paraSession.ExitOscShell_Session(0);
}
int main(int argc, char* argv[])
{
	int retval=0;	
	char szParam[1024] = {0};
	RestartService();
	memset(&noticeMsg,0,sizeof(NOTICE_STRUCT));
	noticeMsg.sid=getCurrSessionID();
	if(argc < 2){
		MessageBox(NULL,"OSCShellApp�������� < 1","����",MB_ICONWARNING|MB_OK|MB_TOPMOST);
		return paraSession.exitSession();

	}else if(argc > 2){
		strcpy(szParam,argv[1]);
		strcat(szParam,argv[2]);
	}else{
		strcpy(szParam,argv[1]);
	}

	char szAccessId[50] = {0};
	char szClientIP[20]={0};
	char szTmpFlag[20]={0};
	char warningMsg[512]={0};

    int  diskFlag=0;
    int  wShowWindow = SW_MAXIMIZE;

	//��������@�ָ� ����ID@�˻�ID@��������@OSCƽ̨���˺�ID@�ͻ���IP@���˻�ID(����Զ�̵�¼�� su �Ǳ�Ҫ����)
	int count = 0;
	char* pszParam = strtok(szParam,"@");
	while(pszParam)
	{
		switch(count)
		{
		case 0:
			strcpy(szAccessId,pszParam);
			break;
		case 1:
			strcpy(szClientIP,pszParam);
			break;
		case 2:
			memset(szTmpFlag,0,sizeof(szTmpFlag));
			strcpy(szTmpFlag,pszParam);
			if(strlen(szTmpFlag)>0){
			   diskFlag=atoi(szTmpFlag);
			}
			break;
		case 3:
			memset(szTmpFlag,0,sizeof(szTmpFlag));
			strcpy(szTmpFlag,pszParam);
			if(strlen(szTmpFlag)>0){
			   if(atoi(szTmpFlag)==0)
			   {
                  wShowWindow=SW_NORMAL;  
			   }
			}
			break;
		}
		pszParam = strtok(NULL, "@");
		count++;
	}
	//��ʼ����־
	paraSession.initSession(szAccessId);
	char osclogPrefix[MAX_PATH]={0};
	sprintf(osclogPrefix,"%s-%s",szAccessId,szClientIP);
	paraSession.openLogs(osclogPrefix);
	char sessionIds[64]={0};
	sprintf(sessionIds,"[main]sessionId:[%d]",noticeMsg.sid);
	WriteLog(sessionIds);
	WriteLog(szAccessId);
	//���̻�����
	//HANDLE singleInstanceMutex = NULL;
	////singleInstanceMutex  = CreateMutexA(NULL,FALSE,"oscShellAppSingleMutex");
	//singleInstanceMutex  = CreateMutexA(NULL,TRUE,"oscShellAppSingleMutex");
	//if (GetLastError() == ERROR_ALREADY_EXISTS)
	//{
	//	CloseHandle(singleInstanceMutex);
	//	singleInstanceMutex = NULL;
	//	char *msg="��һ���û��Ѿ����ӵ���Զ�̼�������볢���ٴ����ӻ���ϵ����Ա��";
	//	WriteLog(msg);
	//	MessageBox(NULL,msg,"Զ����������",MB_OK|MB_HELP);
	//	return paraSession.exitSession();
	//	//return paraSession.ExitOscShell_Session(0);
	//}
	singleInstanceSem = CreateSemaphore(NULL,0,1,"oscShellAppSingleMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		char *msg="��һ���û��Ѿ����ӵ���Զ�̼�������볢���ٴ����ӻ���ϵ����Ա��";
		//MessageBox(NULL,msg,"Զ����������",MB_OK);
		ReleaseSemaphore(singleInstanceSem,1,NULL);
		CloseHandle(singleInstanceSem);
		singleInstanceSem = NULL;
		WriteLog(msg);
		//return paraSession.exitSession();
	}
	if (singleInstanceSem == NULL)
	{
		char errMsg[32]={0};
		sprintf(errMsg,"CreateSemaphore(%d)",GetLastError());
		WriteLog(errMsg);
	}
	if (count < 2)
	{
		MessageBox(NULL,"param count < 2","warning",MB_ICONWARNING|MB_OK|MB_TOPMOST);
		return paraSession.exitSession();
	}  
	int newtype = 0;
	DWORD processId = 0;
	//if(strstr(szClientIP,"172.28.112.144") != NULL || strstr(szClientIP,"172.28.112.135") != NULL)
	//MsgSFTPShow();
	WriteLog("[main]��ʼ��������Ӧ��==========================");
	//дӦ������������־
	FILE *file = NULL;
	char prePath[MAX_PATH] = {0};
	char logName[MAX_PATH] = {0};
	if (strlen(prePath) <= 0)
	{
		memset(prePath,0,sizeof(prePath));
		::GetModuleFileName(NULL, prePath, _MAX_PATH);
		if(strlen(prePath) <= 0)
			return 0;
		char *slash = strrchr(prePath,static_cast<int>('\\'));
		if(slash != NULL)
			*slash = 0;
	}
	//��ȡ��ǰʱ��
	time_t now_sec = time(NULL);
	char timestr[24];
	strftime(timestr, _countof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now_sec));
	
	//2016-01-28 ���� �ж�logĿ¼�Ƿ���ڣ��粻���ڣ��½��ļ���
	char baseDirectory[MAX_PATH] = {0};
	sprintf(baseDirectory, "%s\\log", prePath);
	if (!PathFileExists(baseDirectory))
	{
		CreateDirectory(baseDirectory, NULL);
	}
	//oscShellAppLogin�ļ���������������Ϣ
	tm *myTime = localtime(&now_sec);
	int year = myTime->tm_year + 1900;
	int month = myTime->tm_mon + 1;
	sprintf(logName,"%s\\log\\oscShellAppLogin%4d%02d.log",prePath, year, month);
	//2016-01-28***************************************************
	file = fopen(logName, "a+");
	fprintf(file, "%s:OSCShellApp �����в���[%s] Ӧ����������!\n",timestr, argv[1]);
	fclose(file);
	int haschild = 0;//
	int accessType = 0;
	hProcess = OSCShellEx(szAccessId,szClientIP,diskFlag,wShowWindow,paraSession.szloginfile,paraSession.szAvifile,&newtype,&processId, &accessType, &haschild);
	if(!hProcess)
	{
		MessageBox(NULL,"application start error,session exit!","error",MB_ICONWARNING|MB_OK|MB_TOPMOST);
		return paraSession.ExitOscShell_Session(1);		
	}
	//////////////////////////////////////////////////[]
	 ShowDisableWindow();
	//////////////////////////////////////////////////
	DWORD  dwThreadId;
	WriteLog("[main]���RDPsession����==========================");
	CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)singleSemporeWatcher,NULL,0,&dwThreadId);// wait calling process to end
	WriteLog("[main]������Ӧ��״̬==========================");
	//�û��Լ���д��½�û���������ʱ��windows���򣬼���Ƿ����ӽ���
	//����Ӧ��linux���ʡ����ݿ���ʣ�����Ҫ����ӽ���
	if (haschild != 0)
	{
		DWORD childsID[15]={0};
		int childsNum = 0;
		getChildProcessID(processId,childsID,15,childsNum);
		DWORD  dwThreadId;
		if (childsNum == 0)
		{
			appThreadHandle=CreateThread(NULL,NULL,AnswerThread,hProcess,0,&dwThreadId);// wait calling process to end
		}
		else
		{
			static CHILDPROCESS childs={0};
			childs.childHandles[0] = hProcess;
			childs.childsCount = processIdToHandle(childsID,childs.childHandles+1,childsNum);
			appThreadHandle=CreateThread(NULL,NULL,AnswerThreadEx,&childs,0,&dwThreadId);// wait calling processes to end
		}
	}
	else
	{
		dwThreadId;
		appThreadHandle=CreateThread(NULL,NULL,AnswerThread,hProcess,0,&dwThreadId);// wait calling process to end
	}
	paraSession.processId = processId;

	WriteLog("[main]����¼��===================================");
	paraSession.startRecord();

	WriteLog("[main]������Ƶ���===============================");
	retval = ShellRemoteControlServer();

    if(retval>0)
	{
	    memset(warningMsg,0,sizeof(warningMsg));
		HWND hWnd = GetForegroundWindow();
		switch(retval)
		{
			case 1:
				strcpy(warningMsg,"Database or table error��");
				break;
			case 2:
				strcpy(warningMsg,"Monitor service start failed��");
				break;
			default:
				strcpy(warningMsg,"Data conversion treatment failed��");
				break;
	   }
       WriteLog("init error,session exit!");
	   MessageBox(hWnd,warningMsg,"error",MB_ICONERROR|MB_OK|MB_TOPMOST);
	   return paraSession.ExitOscShell_Session(0);
	}
	WriteLog("[main]ѭ�����ʱ����Կ���========================");
	CheckTimePolicy();
	//if (singleInstanceMutex != NULL)
	//{
	//	CloseHandle(singleInstanceMutex);
	//	singleInstanceMutex = NULL;
	//}
	if(singleInstanceSem != NULL)
	CloseHandle(singleInstanceSem);
	WriteLog("[main]OSCShell ��������!==========================");
	////дӦ�������ǳ���־
	//��ȡ��ǰʱ��
	memset(timestr, 0x0, 24);
	now_sec = time(NULL);
	strftime(timestr, _countof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now_sec));
	file = fopen(logName, "a+");
	fprintf(file, "%s:OSCShellApp �����в���[%s] Ӧ����������!\n",timestr, argv[1]);

	fclose(file);
  	return paraSession.ExitOscShell_Session(0);	
}


//ѭ�����ʱ����Կ���
void CheckTimePolicy()
{
	int count = 0;
	char warningMsg[512]={0};
	int retval;
	//����Ӧ���ػ����� ���rdp������״̬ ����ǶϿ�״̬ ֹͣ¼�� �˳�OSCSheelApp
	int showmsg = 0;
	int checkFlag=0;
	int remindFlag = 0;
	//��ȡ�����ļ��о���ʱ��
	char szPath[512] = {0};
	GetModuleFileName(NULL,szPath,MAX_PATH);
	char *szSlash = strrchr(szPath, static_cast<int>('\\'));
	if (szSlash)
	{
		*++szSlash = '\0';
	}
	strcat(szSlash, "cfg.ini");
	int warningTimes_1 = 0;
	int warningTimes_2 = 0;
	int warningTimes_3 = 0;
	oscLoadCfgFile(szPath, &warningTimes_1, &warningTimes_2, &warningTimes_3);	
	memset(szPath,0,sizeof(szPath));

	while (paraSession.isLoop())
	{
		Sleep(1000);
		if(checkFlag==0)
		{
		   count++;
		}

		//2016-01-21 �޸� ��ÿ2sˢ��һ��onLineTimeʱ��ĳ�ÿ10sˢ��һ��
		if(count>0 && count%10==0 && paraSession.isLoop())
		{
			DoLoginLog(2);  //����
		}
		//2016-01-21 **************************************************

		//����Ӽ��һ�� ʱ����Կ���
		if(count >= 30) 
		{
			checkFlag=1;
			count=0;
		}

		if(checkFlag==1) 
		{

			checkFlag = 0;
			int state = 0;
			int warningFlag = 0;

			retval=0;
			retval=GetVistitimeEx(&state,&remindFlag);
			if(state == 0)
			{
				WriteLog("access time zero, session exit!");
				if(hProcess!=NULL)
				{
					WriteLog("TerminateProcess process") ;
					TerminateProcess(hProcess,0);
					hProcess=NULL;
				}				
				break;
			}	
			memset(warningMsg,0,sizeof(warningMsg));
			if(showmsg == 0 && state>0)
			{
				switch(retval)
				{
					case -9:
						if(GetLocalLang() == 0){
							sprintf(warningMsg,"�˺������ѱ����롢���ʽ���ǰ�����ʱ����Ϊ%ld���ӣ�\n�뱸�ݺ����ݣ�������������ϵ����Ա��",state);
						}else{
							sprintf(warningMsg,"account has been password apply,visit before the end of the maximum delay for %ld minutes\nplease backup your data,if you have any questions please contact administrator.",state);
						}
						break;
					case -16:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"���˺ű������˺����롢���ڰ�Сʱ���˳���\n�뱸�ݺ����ݣ�������������ϵ����Ա��");
						}else{
							strcpy(warningMsg,"account has been password apply,remote access at the end of halfhours!\nplease backup your data,if you have any questions please contact administrator.");
						}
						break;
				}
			}
			if(state>0)
			{
							
				switch(retval)
				{
					case 1:
					case 9:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"��Դ����ʱ������");
						}else{
							strcpy(warningMsg,"resource access time limit");
						}
						break;
					case 2:
					case 3:
					case 10:
					case 11:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�������ʱ������");
						}else{
							strcpy(warningMsg,"apply access time limit");
						}
						break;
					case 4:
					case 12:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"����ʱ������");
						}else{
							strcpy(warningMsg,"policy time limit");
						}
						break;
					case 8:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"��������ʱ������");
						}else{
							strcpy(warningMsg,"apply password time limit");
						}
						break;
					case 16:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�˺��ѱ��������롢ʱ�䵽��");
						}else{
							strcpy(warningMsg,"account has been password apply,the end of time");
						}
						break;
				}

				int msglen=strlen(warningMsg);
				if(msglen>0)
				{				
					if(state>=warningTimes_1 && state < (warningTimes_1 + 300) && (remindFlag == 0 || remindFlag == 4))
					{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������", state/60);
							//sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������",state/60);
						}else{
							sprintf(&warningMsg[msglen],",remote access at the end of %d minute!", state/60);
							//sprintf(&warningMsg[msglen],",remote access at the end of %d minute!",state/60);
						}
						remindFlag = 1;

						if(	retval==1 ||retval==9)
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݻ�����ʱ���룬������������ϵ����Ա��");	
							}else{
								strcat(warningMsg,"\nplease backup your data or apply for delay,if you have any questions please contact administrator.");	
							}
						}else
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݣ�������������ϵ����Ա��");		
							}else{
								strcat(warningMsg,"\nplease backup your data,if you have any questions please contact administrator.");	
							}
						}

						char temp[1024] = {0};
						sprintf(temp, "�˺���Ȩʱ���Ҫ���ڣ���ʾ��Ϣ��%s", warningMsg);
						WriteLog(temp);
						ShowWarning(warningMsg);
					}
					else if(state>= warningTimes_2 && state < (warningTimes_2 + 300) && (remindFlag == 0 || remindFlag == 1 || remindFlag == 4))
					{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������", state/60);	
						}else{

							sprintf(&warningMsg[msglen],",remote access at the end of %d minute!", state/60);	
						}
						remindFlag = 2;

						if(	retval==1 ||retval==9)
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݻ�����ʱ���룬������������ϵ����Ա��");	
							}else{
								strcat(warningMsg,"\nplease backup your data or apply for delay,if you have any questions please contact administrator.");	
							}
						}else
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݣ�������������ϵ����Ա��");		
							}else{
								strcat(warningMsg,"\nplease backup your data,if you have any questions please contact administrator.");	
							}
						}

						char temp[1024] = {0};
						sprintf(temp, "�˺���Ȩʱ���Ҫ���ڣ���ʾ��Ϣ��%s", warningMsg);
						WriteLog(temp);
						ShowWarning(warningMsg);
					}
					else if (state>= warningTimes_3 && state < (warningTimes_3 + 300) && (remindFlag == 0 || remindFlag == 2 || remindFlag == 4))
					{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������", state/60);	
						}else{

							sprintf(&warningMsg[msglen],",remote access at the end of %d minute!", state/60);	
						}
						remindFlag = 3;

						if(	retval==1 ||retval==9)
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݻ�����ʱ���룬������������ϵ����Ա��");	
							}else{
								strcat(warningMsg,"\nplease backup your data or apply for delay,if you have any questions please contact administrator.");	
							}
						}else
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݣ�������������ϵ����Ա��");		
							}else{
								strcat(warningMsg,"\nplease backup your data,if you have any questions please contact administrator.");	
							}
						}

						char temp[1024] = {0};
						sprintf(temp, "�˺���Ȩʱ���Ҫ���ڣ���ʾ��Ϣ��%s", warningMsg);
						WriteLog(temp);
						ShowWarning(warningMsg);
					}		
					
				}
			}
			/*if(showmsg == 1 &&  state>0 && warningFlag ==2)
			{
				showmsg=2;
				switch(retval)
				{
					case 1:
					case 9:
					   
					   if(GetLocalLang() == 0){
						   strcpy(warningMsg,"��Դ����ʱ������");
					   }else{
						   strcpy(warningMsg,"resource access time limit");
					   }
						break;
					case 2:
					case 3:
					case 10:
					case 11:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�������ʱ������");
						}else{
							strcpy(warningMsg,"apply access time limit");
						}
						break;
					case 4:
					case 12:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"����ʱ������");
						}else{
							strcpy(warningMsg,"policy time limit");
						}
						break;
					case 8:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"��������ʱ������");
						}else{
							strcpy(warningMsg,"apply password time limit");
						}
						break;
					case 16:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�˺��ѱ��������롢ʱ�䵽��");
						}else{
							strcpy(warningMsg,"account has been password apply��the end of time");
						}
						break;
				}
				int msglen=strlen(warningMsg);
				if(msglen>0)
				{
					if(state>=3600)
					{
					   if(GetLocalLang() == 0){
						   sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%dСʱ�������",state/3600);	
					   }else{
						   sprintf(&warningMsg[msglen],",remote access at the end of %d hours!",state/3600);	
					   }
					}else
					if(state>=60)
					{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������",state/60);
						}else{
							sprintf(&warningMsg[msglen],",remote access at the end of %d minute!",state/60);
						}
					}else{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d��������",state);	
						}else{
							sprintf(&warningMsg[msglen],",remote access at the end of %d second!",state);	
						}
					}

					if(	retval==1 ||retval==9)
					{
						if(GetLocalLang() == 0){
							strcat(warningMsg,"\n�뱸�ݺ����ݻ�����ʱ���룬������������ϵ����Ա��");	
						}else{
							strcat(warningMsg,"\nplease backup your data or apply for delay,if you have any questions please contact administrator.");	
						}
					}else
					{
						if(GetLocalLang() == 0){
							strcat(warningMsg,"\n�뱸�ݺ����ݣ�������������ϵ����Ա��");		
						}else{
							strcat(warningMsg,"\nplease backup your data,if you have any questions please contact administrator.");	
						}
					}
					char temp[1024] = {0};
					sprintf(temp, "�˺���Ȩʱ���Ҫ���ڣ���ʾ��Ϣ��%s", warningMsg);
					WriteLog(temp);
					ShowWarning(warningMsg);
				}
			}
			if(showmsg==2 && state>0 && warningFlag==3)
			{
				showmsg=3;
				switch(retval)
				{
					case 1:
					case 9:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"��Դ����ʱ������");
						}else{
							strcpy(warningMsg,"resource access time limit");
						}
						break;
					case 2:
					case 3:
					case 10:
					case 11:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�������ʱ������");
						}else{
							strcpy(warningMsg,"apply access time limit");
						}
						break;
					case 4:
					case 12:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"����ʱ������");
						}else{
							strcpy(warningMsg,"policy time limit");
						}
						break;
					case 8:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"��������ʱ������");
						}else{
							strcpy(warningMsg,"apply password time limit");
						}
						break;
					case 16:
						if(GetLocalLang() == 0){
							strcpy(warningMsg,"�˺��ѱ��������롢ʱ�䵽��");
						}else{
							strcpy(warningMsg,"account has been password apply��the end of time");
						}
						break;
				}
				int msglen=strlen(warningMsg);
				if(msglen>0)
				{
					if(state>=3600)
					{
						if(GetLocalLang() == 0){
							sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%dСʱ�������",state/3600);	
						}else{
							sprintf(&warningMsg[msglen],",remote access at the end of %d hours!",state/3600);	
						}
					}else
						if(state>=60)
						{
							if(GetLocalLang() == 0){
								sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d���Ӻ������",state/60);
							}else{
								sprintf(&warningMsg[msglen],",remote access at the end of %d minute!",state/60);
							}
						}else{
							if(GetLocalLang() == 0){
								sprintf(&warningMsg[msglen],"��Զ�̷��ʽ���%d��������",state);	
							}else{
								sprintf(&warningMsg[msglen],",remote access at the end of %d second!",state);	
							}
						}

						if(	retval==1 ||retval==9)
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݻ�����ʱ���룬������������ϵ����Ա��");	
							}else{
								strcat(warningMsg,"\nplease backup your data or apply for delay,if you have any questions please contact administrator.");	
							}
						}else
						{
							if(GetLocalLang() == 0){
								strcat(warningMsg,"\n�뱸�ݺ����ݣ�������������ϵ����Ա��");		
							}else{
								strcat(warningMsg,"\nplease backup your data,if you have any questions please contact administrator.");	
							}
						}
						char temp[1024] = {0};
						sprintf(temp, "�˺���Ȩʱ���Ҫ���ڣ���ʾ��Ϣ��%s", warningMsg);
						WriteLog(temp);
						ShowWarning(warningMsg);
				}
			}*/
		}
	}
}

void getChildProcessID(IN DWORD parentProcessID,OUT DWORD *childProcessIDs,IN int MAXIDS,OUT int& childsCount )
{
	int repeatTimes = 0;
	childsCount = 0;
tryAgain:
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return;
	}
	if (!Process32First(hSnap, &procEntry))//��Snapshot�õ���һ�����̼�¼��Ϣ
	{
		return;
	}
	int i = 0;
	do
	{
		if(i > MAXIDS)
			break;
		DWORD winlogonSessId = 0;
		if(procEntry.th32ParentProcessID == parentProcessID)
		{
			childProcessIDs[i++] = procEntry.th32ProcessID;
			childsCount ++;
		}
	} while (Process32Next(hSnap, &procEntry));
	CloseHandle(hSnap);
	if(i <= 0 && repeatTimes <3)
	{
		repeatTimes ++;
		Sleep(100);
		goto tryAgain;
	}
}
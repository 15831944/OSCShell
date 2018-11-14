// OSCConfig.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "OSCConfig.h"
#include "OSCConfigDlg.h"
#include <Shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"shlwapi.lib")

// COSCConfigApp

BEGIN_MESSAGE_MAP(COSCConfigApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COSCConfigApp ����

COSCConfigApp::COSCConfigApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� COSCConfigApp ����

COSCConfigApp theApp;


// COSCConfigApp ��ʼ��

BOOL COSCConfigApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	COSCConfigDlg dlg;

	if(CString(this->m_lpCmdLine) == CString("-uninstall")){
		WinExec("net stop NFS", SW_HIDE);
		WinExec("net stop WMIService", SW_HIDE);
		WinExec("sc delete NFS",SW_HIDE);
		WinExec("sc delete WMIService",SW_HIDE);
		WinExec("sc delete licenseManager",SW_HIDE);

		char winDir[256] = "0";
		GetSystemWindowsDirectory(winDir,256);
		strcat(winDir,"\\System32\\drivers\\CancelSafe.sys");
		DeleteFile(winDir);

		SHDeleteKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\OSC");
		SHDeleteKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\services\\CancelSafe");

		if (MessageBox(NULL,"ж����ɣ��Ƿ���������ϵͳ��","��ʾ",MB_YESNO|MB_ICONQUESTION) == IDYES)
		{
			ExitWindowsEx(EWX_REBOOT,EWX_FORCE);
		}
		return FALSE;
	}else if(CString(this->m_lpCmdLine) == CString("-install")){
		dlg.isinstall = 1;
	}

	
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

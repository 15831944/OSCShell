// TestViewDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestView.h"
#include "TestViewDlg.h"
#include <string>
#include <time.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestViewDlg �Ի���




CTestViewDlg::CTestViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestViewDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CTestViewDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestViewDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTestViewDlg ��Ϣ�������

BOOL CTestViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTestViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTestViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
typedef void (*pOnRecord)(char *szRec);
typedef void (*pOnStop)();

static char* getTime()
{
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
    return tmp;
}

void CTestViewDlg::OnBnClickedButton1()
{
	HMODULE hmodule = LoadLibrary(_T("recordview.dll"));
	pOnRecord OnRecord = (pOnRecord)GetProcAddress(hmodule,"OnRecord");
	(*OnRecord)("c:\\test1.avi");
}

void CTestViewDlg::OnBnClickedButton2()
{
	HMODULE hmodule = LoadLibrary("recordview.dll");
	pOnStop OnStop = (pOnStop)GetProcAddress(hmodule,"OnStop");
	(*OnStop)();
}

DWORD static WINAPI CloseRecord(LPVOID lparam)
{
	OutputDebugString(getTime());
	OutputDebugString("   2--stop record...");
	OutputDebugString("\r\n");
	HMODULE hmodule = LoadLibrary("recordview.dll");
	pOnStop OnStop = (pOnStop)GetProcAddress(hmodule,"OnStop");
	(*OnStop)();
	OutputDebugString(getTime());
	OutputDebugString("   2--stop record end.");
	OutputDebugString("\r\n");
	return 0;
}

void CTestViewDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	DWORD  dwThreadId1;	
	HMODULE hmodule = LoadLibrary("recordview.dll");
	pOnStop OnStop = (pOnStop)GetProcAddress(hmodule,"OnStop");

	CreateThread(NULL,NULL,CloseRecord,NULL,0,&dwThreadId1);

	OutputDebugString(getTime());
	OutputDebugString("   1--stop record...");
	(*OnStop)();
	OutputDebugString(getTime());
	OutputDebugString("   1--stop record end.");
	CDialog::OnClose();
}
// TestIEHelper.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestIEHelperApp:
// �йش����ʵ�֣������ TestIEHelper.cpp
//

class CTestIEHelperApp : public CWinApp
{
public:
	CTestIEHelperApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestIEHelperApp theApp;
// OSCConfig.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// COSCConfigApp:
// �йش����ʵ�֣������ OSCConfig.cpp
//

class COSCConfigApp : public CWinApp
{
public:
	COSCConfigApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern COSCConfigApp theApp;
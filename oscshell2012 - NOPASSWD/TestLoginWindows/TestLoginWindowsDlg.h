// TestLoginWindowsDlg.h : ͷ�ļ�
//

#pragma once


// CTestLoginWindowsDlg �Ի���
class CTestLoginWindowsDlg : public CDialog
{
// ����
public:
	CTestLoginWindowsDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTLOGINWINDOWS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnlogin();
	CString m_strHost;
	CString m_strUser;
	CString m_strPwd;
};

// TestIEHelperDlg.h : ͷ�ļ�
//

#pragma once


// CTestIEHelperDlg �Ի���
class CTestIEHelperDlg : public CDialog
{
// ����
public:
	CTestIEHelperDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTIEHELPER_DIALOG };

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
	CString m_strUsername;
	CString m_strPassword;
	CString m_strParam;
	afx_msg void OnBnClickedBtnok();
	CString m_strUrl;
};

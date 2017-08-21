
// ServerWindowDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <WINSOCK2.H>
#pragma comment(lib,"ws2_32")

#define WM_SERVER_SOCKET WM_USER + 0x0100

// CServerWindowDlg �Ի���
class CServerWindowDlg : public CDialogEx
{
// ����
public:
	CServerWindowDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERWINDOW_DIALOG };
#endif

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
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	CEdit m_portEdit;
	CListBox m_logList;
	CButton m_startButton;
	CButton m_stopButton;

private:
	SOCKET m_serverSocket;
protected:
	afx_msg LRESULT OnServerSocket(WPARAM wParam, LPARAM lParam);
};


// ServerWindowDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <WINSOCK2.H>
#pragma comment(lib,"ws2_32")

#define WM_SERVER_SOCKET WM_USER + 0x0100

// CServerWindowDlg 对话框
class CServerWindowDlg : public CDialogEx
{
// 构造
public:
	CServerWindowDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERWINDOW_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

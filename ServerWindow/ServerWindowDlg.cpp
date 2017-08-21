
// ServerWindowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ServerWindow.h"
#include "ServerWindowDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: hanpchttpd/0.1.0\r\n"

using namespace std;

unsigned int __stdcall accept_request(void *);
void bad_request(SOCKET);
void cat(SOCKET, FILE *);
void cannot_execute(int);
void error_die(const char *);
// void execute_cgi(SOCKET, const char *, const char *, const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(SOCKET);

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
* return.  Process the request appropriately.
* Parameters: the socket connected to the client */
/**********************************************************************/
unsigned int __stdcall accept_request(void * pClient)
{
	SOCKET client = (SOCKET)pClient;

	char buf[1024];
	ZeroMemory(buf, 1024);
	int numchars;
	char method[255];
	ZeroMemory(method, 255);
	char url[255];
	ZeroMemory(url, 255);
	char path[512];
	ZeroMemory(path, 512);
	size_t i, j;
	struct stat st;
	int cgi = 0;      /* becomes true if server decides this is a CGI
					  * program */
	char *query_string = NULL;

	numchars = get_line(client, buf, sizeof(buf));
	i = 0; j = 0;
	//////////////////////////////////////////////////////////////////////////
	// this will cause a bug if buf HAS NOT BEEN INITAILIZED
	// because ISSpace only accept -1 ~ 255
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
		//////////////////////////////////////////////////////////////////////////
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';

	if (stricmp(method, "GET") && stricmp(method, "POST"))
	{
		unimplemented(client);
		return 0;
	}

	if (stricmp(method, "POST") == 0)
		cgi = 1;

	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';

	if (stricmp(method, "GET") == 0)
	{
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if (*query_string == '?')
		{
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}

	sprintf(path, "htdocs%s", url);
	if (path[strlen(path) - 1] == '/')
		strcat(path, "index.html");
	if (stat(path, &st) == -1) {
		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
			numchars = get_line(client, buf, sizeof(buf));
		not_found(client);
	}
	else
	{
		if ((st.st_mode & S_IFMT) == S_IFDIR)
			strcat(path, "/index.html");
		//if ((st.st_mode & S_IXUSR) ||
		//	(st.st_mode & S_IXGRP) ||
		//	(st.st_mode & S_IXOTH))
		//	cgi = 1;
		if (!cgi)
			serve_file(client, path);
		else
			// execute_cgi(client, path, method, query_string);
			unimplemented(client);
	}

	closesocket(client);

	return 0;
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
* implemented.
* Parameter: the client socket */
/**********************************************************************/
void unimplemented(SOCKET client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
* Parameters: client socket */
/**********************************************************************/
void bad_request(SOCKET client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
* is named after the UNIX "cat" command, because it might have been
* easier just to do something like pipe, fork, and exec("cat").
* Parameters: the client socket descriptor
*             FILE pointer for the file to cat */
/**********************************************************************/
void cat(SOCKET client, FILE *resource)
{
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
* Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
* on value of errno, which indicates system call errors) and exit the
* program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
	perror(sc);
	exit(1);
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
* carriage return, or a CRLF combination.  Terminates the string read
* with a null character.  If no newline indicator is found before the
* end of the buffer, the string is terminated with a null.  If any of
* the above three line terminators is read, the last character of the
* string will be a linefeed and the string will be terminated with a
* null character.
* Parameters: the socket descriptor
*             the buffer to save the data in
*             the size of the buffer
* Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
*             the name of the file */
/**********************************************************************/
void headers(int client, const char *filename)
{
	char buf[1024];
	(void)filename;  /* could use filename to determine file type */

	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
* errors to client if they occur.
* Parameters: a pointer to a file structure produced from the socket
*              file descriptor
*             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
	FILE *resource = NULL;
	int numchars = 1;
	char buf[1024];

	buf[0] = 'A'; buf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));

	resource = fopen(filename, "r");
	if (resource == NULL)
		not_found(client);
	else
	{
		headers(client, filename);
		cat(client, resource);
	}
	fclose(resource);
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerWindowDlg 对话框



CServerWindowDlg::CServerWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERVERWINDOW_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PORT, m_portEdit);
	DDX_Control(pDX, IDC_LIST_LOG, m_logList);
	DDX_Control(pDX, IDC_BUTTON_START, m_startButton);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_stopButton);
}

BEGIN_MESSAGE_MAP(CServerWindowDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CServerWindowDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CServerWindowDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_SERVER_SOCKET, &CServerWindowDlg::OnServerSocket)
END_MESSAGE_MAP()


// CServerWindowDlg 消息处理程序

BOOL CServerWindowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	BYTE minorVer = 2, majorVer = 2;
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(minorVer, majorVer);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		AfxMessageBox("Error initial WSA");
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != minorVer || HIBYTE(wsaData.wVersion) != majorVer)
	{
		WSACleanup();
		AfxMessageBox("Error WSA Version is too low");
		return FALSE;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CServerWindowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerWindowDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerWindowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CServerWindowDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR buff[10] = { 0 };
	m_portEdit.GetWindowTextA(buff, sizeof(buff) / sizeof(TCHAR));
	int port = atoi(buff);
	if (port > 0)
	{
		USHORT nPort = port;
		SOCKET m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in addrin;
		addrin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrin.sin_family = AF_INET;
		addrin.sin_port = htons(nPort);
		if (SOCKET_ERROR == bind(m_serverSocket, (sockaddr*)&addrin, sizeof(sockaddr)))
		{
			AfxMessageBox("Bind failed");
			return;
		}
		if (SOCKET_ERROR == WSAAsyncSelect(m_serverSocket, m_hWnd, WM_SERVER_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ))
		{
			AfxMessageBox("Error select message");
			return;
		}
		if (listen(m_serverSocket, 5) < 0)
		{
			AfxMessageBox("Error listen");
			return;
		}
		int namelen = sizeof(addrin);
		if (getsockname(m_serverSocket, (struct sockaddr *)&addrin, &namelen) == -1)
			AfxMessageBox("Error getsockname");
		port = ntohs(addrin.sin_port);
		CString	log;
		log.Format(_T("Listen to port :%d", port));
		m_logList.AddString(log);
	}
	else
	{
		AfxMessageBox("Check your port");
	}

}


void CServerWindowDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
}


afx_msg LRESULT CServerWindowDlg::OnServerSocket(WPARAM wParam, LPARAM lParam)
{
	SOCKET sock = wParam;
	if (WSAGETSELECTERROR(lParam))   //#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
	{
		closesocket(sock);
		return 0;
	}
	//处理发生的事件
	switch (WSAGETSELECTEVENT(lParam)) //#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
	{
		case FD_ACCEPT:                             //监听中的套接字检查到有连接进入
		{
			accept_request((void *)sock);
			
		}
		break;
		case FD_WRITE:
		{
			//do something
		}
		break;
		case FD_READ:
		{
			char szText[] = { 0 };
			if (SOCKET_ERROR == recv(sock, szText, 1024, 0))
			{
				closesocket(sock);
			}
			else
			{
				
			}
		}
		break;
		case FD_CLOSE:
		{
			closesocket(sock);
		}
		break;
	}
}

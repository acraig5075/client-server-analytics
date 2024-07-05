
// DashboardDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Dashboard.h"
#include "DashboardDlg.h"
#include "DatabaseConnectionDlg.h"
#include "afxdialogex.h"
#include <mysql_driver.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:

private:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDashboardDlg dialog



CDashboardDlg::CDashboardDlg(CWnd *pParent /*=nullptr*/)
	: CDialogEx(IDD_DASHBOARD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDashboardDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTIONBTN, m_connectionBtn);
}

BEGIN_MESSAGE_MAP(CDashboardDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECTIONBTN, &CDashboardDlg::OnBnClickedConnectionbtn)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDashboardDlg message handlers

BOOL CDashboardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect rc;
	GetDlgItem(IDC_PROPSHEETFRAME)->GetWindowRect(&rc);
	ScreenToClient(&rc);

	m_propSheet.AddPage(&m_page1);

	m_propSheet.Create(this, WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0);
	m_propSheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	m_propSheet.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	auto pLayoutMgr = GetDynamicLayout();
	if (pLayoutMgr)
		{
		pLayoutMgr->Create(this);
		pLayoutMgr->AddItem(m_propSheet.m_hWnd, CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeHorizontalAndVertical(100, 100));
		}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDashboardDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDashboardDlg::OnPaint()
{
	if (IsIconic())
		{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
		}
	else
		{
		CDialogEx::OnPaint();
		}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDashboardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDashboardDlg::OnBnClickedConnectionbtn()
{
	if (!m_pConnection || m_pConnection->isClosed())
		{
		DatabaseConnect();
		m_page1.SetConnection(m_pConnection);
		}
	else
		{
		m_page1.CloseConnection();
		DatabaseDisconnect();
		m_page1.SetConnection(m_pConnection);
		}
}



void CDashboardDlg::DatabaseConnect()
{
	if (m_pConnection && !m_pConnection->isClosed())
		DatabaseDisconnect();

	std::vector<std::string> params = { m_connHost, m_connUser, m_connPass };
	auto pDlg = std::make_unique<CDatabaseConnectionDlg>(params, this);
	if (IDCANCEL == pDlg->DoModal())
		return;

	try
		{
		sql::Driver *driver = get_driver_instance();
		if (driver)
			{
			sql::ConnectOptionsMap options;
			options["hostName"] = m_connHost = params[0];
			options["userName"] = m_connUser = params[1];
			options["password"] = m_connPass = params[2];
			options["schema"] = "Analytics";
			options["OPT_CONNECT_TIMEOUT"] = 5;

			m_pConnection = std::shared_ptr<sql::Connection>(driver->connect(options));
			}
		m_connectionBtn.SetWindowTextW(_T("MySQL connection ... Yes"));
		}
	catch (sql::SQLException &ex)
		{
		m_connectionBtn.SetWindowTextW(_T("MySQL connection ... No"));
		MessageBox(CStringW{ ex.what() }, _T("MySQL Error"), MB_ICONERROR | MB_OK);
		}
}

void CDashboardDlg::DatabaseDisconnect()
{
	if (m_pConnection && !m_pConnection->isClosed())
		{
		m_pConnection->close();
		m_pConnection.reset();
		m_connectionBtn.SetWindowTextW(_T("MySQL connection ... No"));
		}
}


void CDashboardDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

// DatabaseConnectionDlg.cpp : implementation file
//

#include "pch.h"
#include "Dashboard.h"
#include "afxdialogex.h"
#include "DatabaseConnectionDlg.h"


// CDatabaseConnectionDlg dialog

IMPLEMENT_DYNAMIC(CDatabaseConnectionDlg, CDialogEx)

CDatabaseConnectionDlg::CDatabaseConnectionDlg(std::vector<std::string> &params, CWnd *pParent /*=nullptr*/)
	: CDialogEx(IDD_DBCONNECTION_DLG, pParent)
	, m_params(params)
{
}

CDatabaseConnectionDlg::~CDatabaseConnectionDlg()
{
}

void CDatabaseConnectionDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDatabaseConnectionDlg, CDialogEx)
END_MESSAGE_MAP()


// CDatabaseConnectionDlg message handlers


BOOL CDatabaseConnectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_params.size() > 0)
		SetDlgItemTextW(IDC_HOSTEDIT, CStringW{ m_params[0].c_str() });
	if (m_params.size() > 1)
		SetDlgItemTextW(IDC_USEREDIT, CStringW{ m_params[1].c_str() });
	if (m_params.size() > 2)
		SetDlgItemTextW(IDC_PASSEDIT, CStringW{ m_params[2].c_str() });

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDatabaseConnectionDlg::OnOK()
{
	CString host, user, pass;
	GetDlgItemTextW(IDC_HOSTEDIT, host);
	GetDlgItemTextW(IDC_USEREDIT, user);
	GetDlgItemTextW(IDC_PASSEDIT, pass);

	m_params.clear();
	m_params.resize(3);
	m_params[0] = CW2A(host);
	m_params[1] = CW2A(user);
	m_params[2] = CW2A(pass);

	CDialogEx::OnOK();
}


// DashboardDlg.h : header file
//

#pragma once

#include "MainPropertySheet.h"
#include "Page1.h"
#include <mysql_connection.h>

class CDatabaseThreadMgr;

// CDashboardDlg dialog
class CDashboardDlg : public CDialogEx
{
// Construction
public:
	CDashboardDlg(CWnd *pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DASHBOARD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedConnectionbtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	void DatabaseConnect();
	void DatabaseDisconnect();

// Implementation
protected:
	HICON m_hIcon;
	std::string m_connHost = "localhost";
	std::string m_connUser = "root";
	std::string m_connPass;
	CButton m_connectionBtn;
	CMainPropertySheet m_propSheet;
	CPage1 m_page1;

	std::shared_ptr<sql::Connection> m_pConnection;

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
};

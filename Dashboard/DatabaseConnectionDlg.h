#pragma once
#include "afxdialogex.h"


// CDatabaseConnectionDlg dialog

class CDatabaseConnectionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDatabaseConnectionDlg)

public:
	CDatabaseConnectionDlg(std::vector<std::string> &params, CWnd *pParent = nullptr);   // standard constructor
	virtual ~CDatabaseConnectionDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DBCONNECTION_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	std::vector<std::string> &m_params;

	DECLARE_MESSAGE_MAP()
};

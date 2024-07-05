#pragma once
#include "afxdialogex.h"
#include "FetchedResults.h"

namespace sql
{
	class Connection;
}

class CDatabaseThreadMgr;

// CPage1 dialog

class CPage1 : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CPage1)

public:
	CPage1(CWnd *pParent = nullptr);   // standard constructor
	virtual ~CPage1();

	void SetConnection(std::shared_ptr<sql::Connection> pConnection);
	void CloseConnection();
	void DatabaseFinished();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	std::shared_ptr<sql::Connection> m_pConnection;
	UINT_PTR m_timerID = 0;
	UINT m_timerElapse = 2000;
	CDatabaseThreadMgr *m_pDatabaseThreadMgr = nullptr;
	std::vector<FetchedResults> m_fetchedResults;

	DECLARE_MESSAGE_MAP()
};

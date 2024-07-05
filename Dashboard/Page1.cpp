// Page1.cpp : implementation file
//

#include "pch.h"
#include "Dashboard.h"
#include "afxdialogex.h"
#include "Page1.h"
#include "DatabaseThreadMgr.h"
#include <mysql_connection.h>
#include <cppconn\prepared_statement.h>
#include <cppconn\resultset.h>
#include <thread>

static UINT WorkThread(LPVOID pMgr);
static void PostProcessFunc(CDatabaseThreadMgr *pMgr);

// CPage1 dialog

IMPLEMENT_DYNAMIC(CPage1, CMFCPropertyPage)

CPage1::CPage1(CWnd *pParent /*=nullptr*/)
	: CMFCPropertyPage(IDD_PAGE1, IDS_DASHBOARD)
	, m_pDatabaseThreadMgr(new CDatabaseThreadMgr)
{
}

CPage1::~CPage1()
{
	delete m_pDatabaseThreadMgr;
}

void CPage1::DoDataExchange(CDataExchange *pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPage1, CMFCPropertyPage)
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CPage1::SetConnection(std::shared_ptr<sql::Connection> pConnection)
{
	KillTimer(m_timerID);

	m_pConnection = pConnection;

	if (m_pConnection && !m_pConnection->isClosed())
		{
		m_timerID = SetTimer(1, 0, nullptr);
		}
}

void CPage1::CloseConnection()
{
	KillTimer(m_timerID);

	CWaitCursor csr;

	Sleep(m_timerElapse);

	std::vector<UINT> editCtrls = { IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6, IDC_EDIT7, IDC_EDIT8, };
	for (UINT nID : editCtrls)
		{
		SetDlgItemTextW(nID, _T(""));
		}
}


// CPage1 message handlers


BOOL CPage1::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPage1::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(m_timerID);

	m_pDatabaseThreadMgr->SetOwner(this);
	m_pDatabaseThreadMgr->SetConnection(m_pConnection);
	m_pDatabaseThreadMgr->SetFetchedResultsStore(&m_fetchedResults);
	m_pDatabaseThreadMgr->SetPostProcessFunc(PostProcessFunc);
	m_pDatabaseThreadMgr->BeginThread(WorkThread);

	// Restart timer
	m_timerID = SetTimer(m_timerID, m_timerElapse, nullptr);

	CMFCPropertyPage::OnTimer(nIDEvent);
}

static UINT WorkThread(LPVOID pMgr)
{
	CDatabaseThreadMgr *pDatabaseThreadMgr = static_cast<CDatabaseThreadMgr *>(pMgr);
	if (!pDatabaseThreadMgr)
		return 0;

	auto connection = pDatabaseThreadMgr->GetConnection();
	if (!connection)
		return 0;

	std::vector<FetchedResults> *pFetchedResults = pDatabaseThreadMgr->GetFetchedResultsStore();
	if (!pFetchedResults)
		return 0;

	std::vector<std::string> tableNames =
		{
		"SurveyTbl",
		"TerrainTbl",
		"RoadTbl",
		"SewerTbl",
		"StormTbl",
		"WaterTbl",
		"SignageTbl",
		};

	pFetchedResults->clear();
	pFetchedResults->resize(tableNames.size());

	auto itr = pFetchedResults->begin();

	for (size_t i = 0; i < tableNames.size(); ++i, ++itr)
		{
		std::string table = tableNames[i];

		int limit = 5;
		std::string str = std::format("SELECT Command, SUM(Count) AS Sum FROM {} GROUP BY Command ORDER BY Sum DESC LIMIT {}", table, limit);
		sql::PreparedStatement *stmt = connection->prepareStatement(str);
		sql::ResultSet *res = stmt->executeQuery();

		itr->reserve(res->rowsCount());

		while (res->next())
			{
			itr->push_back({ res->getString(1), res->getInt(2) });
			}

		delete stmt;
		delete res;
		}

	pDatabaseThreadMgr->SetFinished();
	return 1;
}

/// This is called once the work thread is finished
static void PostProcessFunc(CDatabaseThreadMgr *pMgr)
{
	if (pMgr)
		{
		CPage1 *pDlg = static_cast<CPage1 *>(pMgr->GetOwner());
		if (pDlg)
			pDlg->DatabaseFinished();
		}
}

void CPage1::DatabaseFinished()
{
	std::vector<UINT> editCtrls = { IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6, IDC_EDIT7, IDC_EDIT8, };

	size_t count = std::min<size_t>(editCtrls.size(), m_fetchedResults.size());

	for (size_t i = 0; i < count; ++i)
		{
		CString str;
		for (const auto &res : m_fetchedResults[i])
			{
			str.AppendFormat(_T("%ws\t%d\r\n"), CStringW{ res.command.c_str() }.GetString(), res.count);
			}

		SetDlgItemTextW(editCtrls[i], str);
		}
}


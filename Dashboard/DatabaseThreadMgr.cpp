#include "pch.h"
#include "DatabaseThreadMgr.h"

void CDatabaseThreadMgr::SetOwner(CWnd *pWnd)
{
	m_pWnd = pWnd;
}

void CDatabaseThreadMgr::SetConnection(std::shared_ptr<sql::Connection> connection)
{
	m_pConnection = connection;
}

void CDatabaseThreadMgr::SetFetchedResultsStore(std::vector<FetchedResults> *pFetchedResults)
{
	m_pFetchedResults = pFetchedResults;
}

void CDatabaseThreadMgr::SetPostProcessFunc(std::function<void(CDatabaseThreadMgr *)> pPostProcessFunc)
{
	m_pPostProcessFunc = pPostProcessFunc;
}

void CDatabaseThreadMgr::BeginThread(AFX_THREADPROC pfnThreadProc)
{
	AfxBeginThread(pfnThreadProc, this);
}

CWnd *CDatabaseThreadMgr::GetOwner()
{
	return m_pWnd;
}

std::shared_ptr<sql::Connection> CDatabaseThreadMgr::GetConnection() const
{
	return m_pConnection;
}

std::vector<FetchedResults> *CDatabaseThreadMgr::GetFetchedResultsStore()
{
	return m_pFetchedResults;
}

void CDatabaseThreadMgr::SetFinished()
{
	if (m_pPostProcessFunc)
		m_pPostProcessFunc(this);
}

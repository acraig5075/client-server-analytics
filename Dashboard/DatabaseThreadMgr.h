#pragma once

#include <mysql_connection.h>
#include "FetchedResults.h"

class CDatabaseThreadMgr
{
public:
	void SetOwner(CWnd *pWnd);
	void SetConnection(std::shared_ptr<sql::Connection> connection);
	void SetFetchedResultsStore(std::vector<FetchedResults> *pFetchedResults);
	void SetPostProcessFunc(std::function<void(CDatabaseThreadMgr *)> pPostProcessFunc);
	void BeginThread(AFX_THREADPROC pfnThreadProc);
	void SetFinished();

	CWnd *GetOwner();
	std::shared_ptr<sql::Connection> GetConnection() const;
	std::vector<FetchedResults> *GetFetchedResultsStore();

private:
	CWnd *m_pWnd = nullptr;
	std::shared_ptr<sql::Connection> m_pConnection;
	std::vector<FetchedResults> *m_pFetchedResults;
	std::function<void(CDatabaseThreadMgr *)> m_pPostProcessFunc = nullptr;
};


#pragma once


// CMainPropertySheet

class CMainPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CMainPropertySheet)

public:
	CMainPropertySheet();
	virtual ~CMainPropertySheet();

	virtual BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
};



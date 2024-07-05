// MainPropertySheet.cpp : implementation file
//

#include "pch.h"
#include "Dashboard.h"
#include "MainPropertySheet.h"


// CMainPropertySheet

IMPLEMENT_DYNAMIC(CMainPropertySheet, CMFCPropertySheet)

CMainPropertySheet::CMainPropertySheet()
{
	EnableDynamicLayout(TRUE);
}

CMainPropertySheet::~CMainPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CMainPropertySheet, CMFCPropertySheet)
END_MESSAGE_MAP()


// CMainPropertySheet message handlers

BOOL CMainPropertySheet::OnInitDialog()
{
	BOOL bResult = CMFCPropertySheet::OnInitDialog();

	return bResult;
}

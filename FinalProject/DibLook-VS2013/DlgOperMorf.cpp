// DlgOperMorf.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "DlgOperMorf.h"
#include "afxdialogex.h"


// CDlgOperMorf dialog

IMPLEMENT_DYNAMIC(CDlgOperMorf, CDialogEx)

CDlgOperMorf::CDlgOperMorf(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgOperMorf::IDD, pParent)
	, m_oper_morf(0)
{

}

CDlgOperMorf::~CDlgOperMorf()
{
}

void CDlgOperMorf::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_OPER_MORF, m_oper_morf);
}


BEGIN_MESSAGE_MAP(CDlgOperMorf, CDialogEx)
END_MESSAGE_MAP()


// CDlgOperMorf message handlers

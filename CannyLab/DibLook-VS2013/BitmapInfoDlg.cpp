// BitmapInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "BitmapInfoDlg.h"
#include "afxdialogex.h"


// CBitmapInfoDlg dialog

IMPLEMENT_DYNAMIC(CBitmapInfoDlg, CDialogEx)

CBitmapInfoDlg::CBitmapInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBitmapInfoDlg::IDD, pParent)
	, m_Width(_T(""))
	, m_Height(_T(""))
	, m_LUT(_T(""))
{

}

CBitmapInfoDlg::~CBitmapInfoDlg()
{
}

void CBitmapInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Width);
	DDX_Text(pDX, IDC_EDIT2, m_Height);
	DDX_Text(pDX, IDC_EDIT3, m_LUT);
}


BEGIN_MESSAGE_MAP(CBitmapInfoDlg, CDialogEx)
END_MESSAGE_MAP()


// CBitmapInfoDlg message handlers

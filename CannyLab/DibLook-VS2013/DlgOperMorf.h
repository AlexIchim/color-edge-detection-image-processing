#pragma once


// CDlgOperMorf dialog

class CDlgOperMorf : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgOperMorf)

public:
	CDlgOperMorf(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgOperMorf();

// Dialog Data
	enum { IDD = IDD_DIALOG_OP_MORF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_oper_morf;
};

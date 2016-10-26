#pragma once


// CBitmapInfoDlg dialog

class CBitmapInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBitmapInfoDlg)

public:
	CBitmapInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBitmapInfoDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_Width;
	CString m_Height;
	CString m_LUT;
};

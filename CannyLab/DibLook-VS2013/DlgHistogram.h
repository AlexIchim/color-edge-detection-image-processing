#pragma once
#include "Histogram.h"


// CDlgHistogram dialog

class CDlgHistogram : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgHistogram)

public:
	CDlgHistogram(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHistogram();

// Dialog Data
	enum { IDD = IDD_DIALOG_HISTOGRAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CHistogram Histogram;
	afx_msg void OnStnClickedHistogram();
};

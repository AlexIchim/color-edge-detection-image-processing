#pragma once


// CHistogram

class CHistogram : public CStatic
{
	DECLARE_DYNAMIC(CHistogram)

public:
	CHistogram();
	virtual ~CHistogram();
	int values[256];

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};



// Histogram.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "Histogram.h"


// CHistogram

IMPLEMENT_DYNAMIC(CHistogram, CStatic)

CHistogram::CHistogram()
{

}

CHistogram::~CHistogram()
{
}


BEGIN_MESSAGE_MAP(CHistogram, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CHistogram message handlers




void CHistogram::OnPaint()
{
		CPaintDC dc(this); // device context-ul pentru afi�are
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0)); // definirea pen-ului de desenare cu culoare ro�ie
			CPen *pTempPen = dc.SelectObject(&pen); // selectarea pen-ului de afi�are
		CRect rect;
		GetClientRect(rect); // ob�ine zona de afi�are dreptunghiular� disponibil�
		int height = rect.Height(); // �n�l�imea zonei de afi�are
		int width = rect.Width(); // l��imea zonei de afi�are
		// se determin� maximul �irului values[256]
		int i;
		int maxValue = 0;
		for (i = 0; i<256; i++)
			if (values[i]>maxValue)
				maxValue = values[i];
		// se verific� dac� este nevoie de scalare
		double scaleFactor = 1.0;
		if (maxValue >= height)
		{
			// este nevoie de scalare
			scaleFactor = (double)height / maxValue;
		}

		// se afi�eaza histograma (eventual scalata) ca ni�te bare verticale
		for (i = 0; i<256; i++)
		{
			// determinarea lungimii liniei
			int lengthLine = (int)(scaleFactor*values[i]);
			//afi�area liniei
			dc.MoveTo(i, height);
			dc.LineTo(i, height - lengthLine);
		}
		dc.SelectObject(pTempPen); // restaurarea pen-ului de afi�are
}

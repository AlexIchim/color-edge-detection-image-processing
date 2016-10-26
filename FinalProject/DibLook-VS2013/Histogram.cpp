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
		CPaintDC dc(this); // device context-ul pentru afiºare
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0)); // definirea pen-ului de desenare cu culoare roºie
			CPen *pTempPen = dc.SelectObject(&pen); // selectarea pen-ului de afiºare
		CRect rect;
		GetClientRect(rect); // obþine zona de afiºare dreptunghiularã disponibilã
		int height = rect.Height(); // înãlþimea zonei de afiºare
		int width = rect.Width(); // lãþimea zonei de afiºare
		// se determinã maximul ºirului values[256]
		int i;
		int maxValue = 0;
		for (i = 0; i<256; i++)
			if (values[i]>maxValue)
				maxValue = values[i];
		// se verificã dacã este nevoie de scalare
		double scaleFactor = 1.0;
		if (maxValue >= height)
		{
			// este nevoie de scalare
			scaleFactor = (double)height / maxValue;
		}

		// se afiºeaza histograma (eventual scalata) ca niºte bare verticale
		for (i = 0; i<256; i++)
		{
			// determinarea lungimii liniei
			int lengthLine = (int)(scaleFactor*values[i]);
			//afiºarea liniei
			dc.MoveTo(i, height);
			dc.LineTo(i, height - lengthLine);
		}
		dc.SelectObject(pTempPen); // restaurarea pen-ului de afiºare
}

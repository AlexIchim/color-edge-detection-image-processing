// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

//using namespace std;

#include "stdafx.h"
#include <algorithm>
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"

#include "HRTimer.h"
#include "BitmapInfoDlg.h"â
#include "DlgHistogram.h"
#include "DlgOperMorf.h"
#include "dibfft.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <queue>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	


#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format(_TEXT("%s - Proc. time = %.2f ms"), _TEXT(Titlu), elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCTSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);	\

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_PROCESSING_COLOR24, &CDibView::OnProcessingColor24)
	ON_COMMAND(ID_PROCESSING_GREY, &CDibView::OnProcessingGrey)
	ON_COMMAND(ID_LAB2_AFISAREINFO, &CDibView::OnLab2Afisareinfo)
	ON_COMMAND(ID_LAB2_GRAYSCALE, &CDibView::OnLab2Grayscale)
	ON_COMMAND(ID_LAB2_INVGRAYSCALE, &CDibView::OnLab2Invgrayscale)
	ON_COMMAND(ID_LAB2_BLACKWHITE, &CDibView::OnLab2Blackwhite)
	ON_COMMAND(ID_LAB3_AFISAREHISTOGRAMA, &CDibView::OnLab3Afisarehistograma)
	ON_COMMAND(ID_LAB3_REDUCEREGRI, &CDibView::OnLab3Reduceregri)
	ON_COMMAND(ID_LAB3_FLOYD, &CDibView::OnLab3Floyd)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_LAB4_PROIECT, &CDibView::OnLab4Proiect)
	ON_COMMAND(ID_LAB4_PROIECTIEPEVERTICALA, &CDibView::OnLab4Proiectiepeverticala)
	ON_COMMAND(ID_LAB5_ETICHETARE, &CDibView::OnLab5Etichetare)
	ON_COMMAND(ID_LAB6_ALGORITMURMARIRECONTUR, &CDibView::OnLab6Algoritmurmarirecontur)
	ON_COMMAND(ID_LAB6_RECONSTRUCTIECONTUR, &CDibView::OnLab6Reconstructiecontur)
	ON_COMMAND(ID_LAB7_OPERATIIMORFOLOGICE, &CDibView::OnLab7Operatiimorfologice)
	ON_COMMAND(ID_LAB8_MEDIASIDEV, &CDibView::OnLab8Mediasidev)
	ON_COMMAND(ID_LAB8_BINARIZARE32791, &CDibView::OnLab8Binarizare32791)
	ON_COMMAND(ID_LAB8_BINARIZARE, &CDibView::OnLab8Binarizare)
	ON_COMMAND(ID_LAB8_MODIFICARELUMINOZITATE, &CDibView::OnLab8Modificareluminozitate)
	ON_COMMAND(ID_LAB8_MODIFICARECONTRAST, &CDibView::OnLab8Modificarecontrast)
	ON_COMMAND(ID_LAB8_CORECTIEGAMMA, &CDibView::OnLab8Corectiegamma)
	ON_COMMAND(ID_LAB8_EGALIZAREHISTOGRAMA, &CDibView::OnLab8Egalizarehistograma)
	ON_COMMAND(ID_LAB9_FILTRUMEDIEARITMETICA, &CDibView::OnLab9Filtrumediearitmetica)
	ON_COMMAND(ID_LAB9_FILTRUMEDIEARITMETICA5X5, &CDibView::OnLab9Filtrumediearitmetica5x5)
	ON_COMMAND(ID_LAB9_FILTRUGAUSSIAN3X3, &CDibView::OnLab9Filtrugaussian3x3)
	ON_COMMAND(ID_LAB9_FILTRULAPLACE3X3, &CDibView::OnLab9Filtrulaplace3x3)
	ON_COMMAND(ID_LAB9_FILTRUTRECESUS3X3, &CDibView::OnLab9Filtrutrecesus3x3)
	ON_COMMAND(ID_LAB9_LOGARITMMAGNITUDINE, &CDibView::OnLab9Logaritmmagnitudine)
	ON_COMMAND(ID_LAB9_FTSIDEAL, &CDibView::OnLab9Ftsideal)
	ON_COMMAND(ID_LAB9_FTSTAIEREGAUSSIANA32805, &CDibView::OnLab9Ftstaieregaussiana32805)
	ON_COMMAND(ID_LAB9_FTSIDEAL32803, &CDibView::OnLab9Ftsideal32803)
	ON_COMMAND(ID_LAB9_FTSTAIEREGAUSSIANA, &CDibView::OnLab9Ftstaieregaussiana)
	ON_COMMAND(ID_L10_FILTRUGAUSSIAN2D, &CDibView::OnL10Filtrugaussian2d)
	ON_COMMAND(ID_L10_FILTRUGAUSSIAN2D32807, &CDibView::OnL10Filtrugaussian2d32807)
	ON_COMMAND(ID_L10_FILTRUMEDIAN, &CDibView::OnL10Filtrumedian)
	ON_COMMAND(ID_L11_FILTRAREGAUSSIANA, &CDibView::OnL11Filtraregaussiana)
	ON_COMMAND(ID_L11_MODULULORIENTAREAGRADIENTILOR, &CDibView::OnL11Modululorientareagradientilor)
	ON_COMMAND(ID_L11_BINARIZAREAADAPTIVA, &CDibView::OnL11Binarizareaadaptiva)
	ON_COMMAND(ID_L11_PRELUNGIREAMUCHIILOR, &CDibView::OnL11Prelungireamuchiilor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL) hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

void CDibView::OnProcessingParcurgereSimpla() 
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// Makes a grayscale image by equalizing the R, G, B components from the LUT
	for (int k=0;  k < iColors ; k++)
		bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;

	//  Goes through the bitmap pixels and performs their negative	
	for (int i=0;i<dwHeight;i++)
		for (int j=0;j<dwWidth;j++)
		  {	
			lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
	  }

	END_PROCESSING("Negativ imagine");
}



void CDibView::OnProcessingColor24()
{
	// TODO: Add your command handler code here
	int val = 100;
	BEGIN_PROCESSING();
	
	//  Goes through the bitmap pixels and performs their negative	
	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j<dwWidth; j++)
		{
			int R = lpSrc[i*w + j * 3 + 0];
			int G = lpSrc[i*w + j * 3 + 1];
			int B = lpSrc[i*w + j * 3 + 2];

			 //makes image negative
			if (R + val > 255)
				lpDst[i*w + j * 3 + 0] = 255;
			else
				lpDst[i*w + j * 3 + 0] =	R + val;

			if (G + val > 255)
				lpDst[i*w + j * 3 + 1] = 255;
			else
				lpDst[i*w + j * 3 + 1] = G + val;

			if (B + val > 255)
				lpDst[i*w + j * 3 + 2] = 255;
			else
				lpDst[i*w + j * 3 + 2] = B + val;

		}

	END_PROCESSING("Crestere luminozitate");
}

void CDibView::OnProcessingGrey()
{

	BEGIN_PROCESSING();


	// TODO: Add your command handler code here
	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
		{

			int val = lpSrc[i*w + j];
			
			if (val + 50 > 255)
				lpDst[i*w + j] = 255;
			else
				lpDst[i*w + j] = val + 50;
		}

	END_PROCESSING("Crestere luminozitate");

}


void CDibView::OnLab2Afisareinfo()
{
	// TODO: Add your command handler code here
	BEGIN_SOURCE_PROCESSING;
	CBitmapInfoDlg dlgBitmap;
	LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO)lpS;

	dlgBitmap.m_Width.Format(_TEXT("Width: %d"), pBitmapInfoSrc->bmiHeader.biWidth);
	dlgBitmap.m_Height.Format(_TEXT("Height: %d"), pBitmapInfoSrc->bmiHeader.biHeight);
	
	CString buffer;
	for (int i = 0; i < iColors; i++)
	{
		buffer.Format(_TEXT("%3d. \t%3d\t%3d\r\n"), i, 
			bmiColorsSrc[i].rgbRed,
			bmiColorsSrc[i].rgbGreen,
			bmiColorsSrc[i].rgbBlue);
		dlgBitmap.m_LUT += buffer;
	}

	dlgBitmap.DoModal();
	END_SOURCE_PROCESSING;
}


void CDibView::OnLab2Grayscale()
{
	BEGIN_PROCESSING()
		// TODO: Add your command handler code here
	
	BYTE g[256];

	LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO)lpS;
	
	for (int k = 0; k < iColors; ++k)
		{
			g[k] = bmiColorsDst[k].rgbRed;
			bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = k;
		}

		for (int i = 0; i < dwHeight; ++i){
			for (int j = 0; j < dwWidth; ++j){
				int k = lpDst[i*w + j];
				lpDst[i*w + j] = g[k];
			}
		}
	
	END_PROCESSING("Grayscale")
}


void CDibView::OnLab2Invgrayscale()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING()
	BYTE g[256];
	LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO)lpS;

	for (int k = 0; k < iColors; ++k)
	{
		g[k] = bmiColorsSrc[k].rgbRed;
		bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = k;
	}

	for (int i = 0; i < dwHeight; ++i){
		for (int j = 0; j < dwWidth; ++j){
			int k = lpDst[i*w + j];
			lpDst[i*w + j] = k;
		}
	}
	END_PROCESSING("Grayscale")
}


void CDibView::OnLab2Blackwhite()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING()
	LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO)lpS;
	
	int threshold = 150;

		for (int i = 0; i < dwHeight; ++i){
			for (int j = 0; j < dwWidth; ++j){
				if (lpSrc[i*w + j] < threshold){
					lpDst[i*w + j] = 0;
				}
				else {
					lpDst[i*w + j] = 255;
				}
			}
		}
	END_PROCESSING("Black and White")
}

void CDibView::OnLab3Afisarehistograma()
{
	// TODO: Add your command handler code here
	BEGIN_SOURCE_PROCESSING;

	int h[256] = { 0 };
	float FDPValues[256];
	// se va scrie codul pentru calcularea histogramei în ºirul de întregihistValues[256]
		// se va scrie codul pentru calcularea FDP în ºirul de valori reale FDPValues[256]
		// se instanþiazã un dialog box de afiºare ºi se asociazã histograma
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			FDPValues[lpSrc[i*w + j]] = (float)h[lpSrc[i*w + j]] / (dwHeight * dwWidth);
		}
	}


	CDlgHistogram d;
	memcpy(d.Histogram.values, h, sizeof(h));
	// se afiºeazã dialog box-ul
	d.DoModal();
	END_SOURCE_PROCESSING;

}

int getClosestHistogramVal(int nVal, int vSize, int *v){
	int range = 0;
	for (int i = 0; i < vSize; i++){
		if (abs(nVal - v[i]) < abs(nVal - range)){
			range = v[i];
		}
	}
	return range;
}


void CDibView::OnLab3Reduceregri()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING()

	int maxv[256] = { 0 };
	int h[256] = { 0 };
	float FDPValues[256];
	// se va scrie codul pentru calcularea histogramei în ºirul de întregihistValues[256]
	// se va scrie codul pentru calcularea FDP în ºirul de valori reale FDPValues[256]
	// se instanþiazã un dialog box de afiºare ºi se asociazã histograma
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			FDPValues[lpSrc[i*w + j]] = (float)h[lpSrc[i*w + j]] / (dwHeight * dwWidth);
		}
	}

	int WH = 5;
	int frameWidth = 2 * WH + 1;
	double TH = 0.0003;

	for (int i = 0 + WH; i < 255 - WH; i++) {
		float avg = 0;
		int ok = 1;
		for (int j = i - WH; j <= i + WH; j++) {
			avg += FDPValues[j];
			if (FDPValues[j] > FDPValues[i]) {
				ok = 0;
			}
		}
		avg = avg / (2 * WH + 1);
		if ((FDPValues[i] >(avg + TH)) && ok == 1)
			maxv[i] = i;
	}
	maxv[0] = 0;
	maxv[255] = 255;
	
	
	int ok = 1;
	while (ok) {
		ok = 0;
		for (int i = 1; i < 255; i++) {
			if (maxv[i]==0 && maxv[i - 1]!=0) {
				maxv[i] = maxv[i - 1];
				i++;
				ok = 1;
			}
			if (maxv[i]==0 && maxv[i + 1]!=0) {
				maxv[i] = maxv[i + 1];
				i++;
				ok = 1;
			}
		}
	}
	

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			lpDst[i*w + j] = maxv[lpDst[i*w + j]];
			//lpDst[i*w + j] = getClosestHistogramVal(lpDst[i*w + j], 256, maxv);
		}
	}

	END_PROCESSING("ReducereGri")
}


void CDibView::OnLab3Floyd()
{
	// TODO: Add your command handler code here

	// TODO: Add your command handler code here
	BEGIN_PROCESSING()

	int maxv[256] = { 0 };
	int maxv2[50] = { 0 };
	int h[256] = { 0 };
	float FDPValues[256];
	// se va scrie codul pentru calcularea histogramei în ºirul de întregihistValues[256]
	// se va scrie codul pentru calcularea FDP în ºirul de valori reale FDPValues[256]
	// se instanþiazã un dialog box de afiºare ºi se asociazã histograma
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			FDPValues[lpSrc[i*w + j]] = (float)h[lpSrc[i*w + j]] / (dwHeight * dwWidth);
		}
	}

	int WH = 5;
	int frameWidth = 2 * WH + 1;
	double TH = 0.0003;
	int k = 0;
	maxv2[0] = 0;

	for (int i = 0 + WH; i < 255 - WH; i++) {
		float avg = 0;
		int ok = 1;
		for (int j = i - WH; j <= i + WH; j++) {
			avg += FDPValues[j];
			if (FDPValues[j] > FDPValues[i]) {
				ok = 0;
			}
		}
		avg = avg / (2 * WH + 1);
		if ((FDPValues[i] >(avg + TH)) && ok == 1)
			maxv2[k++] = i;
	}
	maxv2[k++] = 255;

	/*
	int ok = 1;
	while (ok) {
		ok = 0;
		for (int i = 1; i < 255; i++) {
			if (maxv[i] == 0 && maxv[i - 1] != 0) {
				maxv[i] = maxv[i - 1];
				i++;
				ok = 1;
			}
			if (maxv[i] == 0 && maxv[i + 1] != 0) {
				maxv[i] = maxv[i + 1];
				i++;
				ok = 1;
			}
		}
	}
	*/


	int error = 0;
	for (int y = 0; y < dwHeight; y++){
		for (int x = 0; x < dwWidth; ++x){
			
			//pixel vechi
			int pVechi = lpDst[y*w + x]; 

			//pixel nou - cel mai apropiat maxim din histograma
			int pNou = getClosestHistogramVal(pVechi, k, maxv2);

			//setare pixel nou
			lpDst[y*w + x] = pNou;

			//calcul eroare > pixel_vechi - pixel_nou
			error =  pVechi - pNou;
			

			//prelucrare pixel din dreapta pixelului curent
			if (x < dwWidth - 1) {
				if (lpDst[y*w + x + 1] + 7 * error / 16 > 255)
					lpDst[y*w + x + 1] = 255;
				else if (lpDst[y*w + x + 1] + 7 * error / 16 < 0)
						lpDst[y*w + x + 1] = 0;
					else 
						lpDst[y*w + x + 1] = lpDst[y*w + x + 1] + 7 * error / 16;
			}
			
			//prelucrare pixel din stanga sus a pixelului  curent
			if ( y < dwHeight-1  && x >0 )
				if (lpDst[(y + 1)*w + x - 1] + 3 * error / 16 > 255)
					lpDst[(y + 1)*w + x - 1] = 255;
				else if (lpDst[(y + 1)*w + x - 1] + 3 * error / 166 < 0)
					lpDst[(y + 1)*w + x - 1] = 0;
				else
					lpDst[(y + 1)*w + x - 1] = lpDst[(y + 1)*w + x - 1] + 3 * error / 16;

			
			
			//prelucrare pixel deasupra pixelului curent
			if (y < dwHeight - 1)

				if (lpDst[(y + 1)*w + x] + 5 * error / 16 > 255)
					lpDst[(y + 1)*w + x] = 255;
				else if (lpDst[(y + 1)*w + x] + 5 * error / 16 < 0)
					lpDst[(y + 1)*w + x] = 0;
				else
					lpDst[(y + 1)*w + x] = lpDst[(y + 1)*w + x] + 5 * error / 16;

			//prelucrare pixel din dreapta sus a pixelului  curent
			if (y < dwHeight -1 && x< dwWidth - 1)
				if (lpDst[(y + 1)*w + x + 1] + error / 16 > 255)
					lpDst[(y + 1)*w + x + 1] = 255;
				else if (lpDst[(y + 1)*w + x + 1] + error / 16 < 0)
					lpDst[(y + 1)*w + x + 1] = 0;
				else
					lpDst[(y + 1)*w + x + 1] = lpDst[(y + 1)*w + x + 1] + error / 16;

		}
	}
	END_PROCESSING("Floyd")
}
#define PI 3.14159265


void CDibView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BEGIN_PROCESSING();

	//axa y trebuie inversatã din cauza reprezentãrii bitmap-urilor în memorie
	CDC dc; //obiectul DC-ul aflat în memorie
	//se creazã astfel încât sã fie compatibil cu ecranul
	dc.CreateCompatibleDC(0);
	//va conþine un DDB compatibil cu ecranul
	CBitmap ddBitmap;
	//creare DDB compatibil cu ecranul ºi iniþializarea datelor acestuia
	//cu datele din imagine DIB sursã
	HBITMAP hDDBitmap =
		CreateDIBitmap(::GetDC(0), &((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT,
		lpSrc, (LPBITMAPINFO)lpS, DIB_RGB_COLORS);

	//ataºazã hanble-ul la obþinut la obiectul CBitmap
	ddBitmap.Attach(hDDBitmap);
	
	//selecteazã bitmap-ul in DC-ul aflat în memorie
	//astfel încât toate desenãrile sã se aibã loc în imagine
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);

	//începând de aici toate desenãrile se vor face pe imagine DDB

	//obþine poziþia curentã de scroll (din cauza barelor de scroll
	//coordonatele pot fi translatate)ºi ajusteazã coordonatele
	CPoint pos = GetScrollPosition() + point;

	int x = pos.x;
	int y = dwHeight - pos.y - 1;
	int Aria = 0;

	int centruDeMasaX = 0;
	int centruDeMasaY = 0;

	//Aria
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == lpSrc[y*w + x]) {
				Aria++;
			}
		}
	}

	//Centru de masa
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == lpSrc[y*w + x]) {
				centruDeMasaX += j;
				centruDeMasaY += i;
			}
		}
	}

	centruDeMasaX = (int) centruDeMasaX / Aria;
	centruDeMasaY = (int) centruDeMasaY / Aria;

	//Axa de alungire
	double A = 0, B = 0, C = 0;
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == lpSrc[y*w + x]) {
				A += (i - centruDeMasaY)*(j - centruDeMasaX);
				B += (j - centruDeMasaX)*(j - centruDeMasaX);
				C += (i - centruDeMasaY)*(i - centruDeMasaY);
			}
		}
	}
	double angle = atan2((2 * A), (B - C));
	if (angle < 0)
		angle = angle + PI;
	angle = angle * 180 / PI / 2;

	//Perimetru
	int perimetru = 0;
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == lpSrc[y*w + x]) {
				if (lpSrc[(i + 1)*w + j] != lpSrc[y*w + x]		||
					lpSrc[(i + 1)*w + j + 1] != lpSrc[y*w + x]	||
					lpSrc[i*w + j - 1] != lpSrc[y*w + x]		||
					lpSrc[i*w + j - 1] != lpSrc[y*w + x]		||
					lpSrc[(i - 1)*w + j] != lpSrc[y*w + x]		||
					lpSrc[(i - 1)*w + j - 1] != lpSrc[y*w + x]	||
					lpSrc[i*w + j - 1] !=	 lpSrc[y*w + x]		||
					lpSrc[(i + 1)*w + j - 1] != lpSrc[y*w + x]
					)

					perimetru++;
			}
		}
	}

	//Factor de subtiere
	float factorSubtiere = 4.0f * PI *  ((float)Aria / (float)(perimetru * perimetru));
	
	//Factor de aspect
	int minX, minY, maxX, maxY;
	maxX = maxY = 0;
	minY = dwHeight;
	minX = dwWidth;
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == lpSrc[y*w + x]) {

				if (i < minY)
					minY = i;
				if (i > maxY)
					maxY = i;
				if (j < minX)
					minX = j;
				if (j > maxX)
					maxX = j;
			}
		}
	}
	float aspectRatio = (float)(maxX - minX + 1) / (float) (maxY - minY + 1);
	

	//Axa de alungire
	double angle2 = angle * PI / 180;
	float yA = centruDeMasaY + tan(angle2)*(minX - centruDeMasaX);
	float yB = centruDeMasaY + tan(angle2)*(maxX - centruDeMasaX);
	
	


	//testeazã dacã punctul este poziþionat în interiorul imaginii
	if (x>0 && x<dwWidth && y>0 && y<dwHeight)
	{
		//pregãteºte un obiect CString pentru formatarea mesajului
		CString info;
		info.Format(_TEXT("Aria=%d  \nCentru de masa: x=%d y=%d \nUnghi=%f\n Perimetru=%d \nFactor de subtiere=%f \nFactor de aspect = %f"), Aria, centruDeMasaX, centruDeMasaY, angle, perimetru, factorSubtiere, aspectRatio);
		AfxMessageBox(info);
	}

	//creare a pen verde pentru desenare
	CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
	//selecteazã pen-ul în DC
	CPen *pTempPen = dc.SelectObject(&pen); //afiºare text
	//dc.TextOut(pos.x, pos.y, "test");
	dc.MoveTo(minX,dwHeight - 1 - yA); //afiºare linie
	dc.LineTo(maxX,dwHeight -1 - yB);

	//selecteazã vechiul pen
	dc.SelectObject(pTempPen);
	//ºi vechiul DDB
	dc.SelectObject(pTempBmp);
	//copiazã pixelii din DDB în DIB-ul destinaþie
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst, (LPBITMAPINFO)lpD,
		DIB_RGB_COLORS);

	END_PROCESSING("linie");
	
}


void CDibView::OnLab4Proiect()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	//Proiectii orizontale
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			lpDst[i*w + j] = 255;
		}
	}
	for (int i = 0; i < dwHeight; i++) {
		int count = 0;
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] == 0)
			{
				lpDst[i*w + count] = 0;
				count++;
			}
		}
	}
	END_PROCESSING("linie");
}


void CDibView::OnLab4Proiectiepeverticala()
{
	// TODO: Add your command handler code here

	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	//Proiectii orizontale
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			lpDst[i*w + j] = 255;
		}
	}
	for (int j = 0; j < dwWidth; j++) {
			
			int count=0;
			for (int i = 0; i < dwHeight; i++) {
				if (lpSrc[i*w + j] == 0)
				{
					lpDst[count*w + j] = 0;
					count++;
				}
			}
	}
	END_PROCESSING("linie");
}


int q[300], top = -1, front = -1, rear = -1, a[20][20], vis[300];
int culoare = 1;
int etichete[9000][9000];
int echivalente[3000][3000];

void add (int item)
{

	if (rear == -1)
	{
		q[++rear] = item;
		front++;
	}
	else
		q[++rear] = item;

}

int delete_n()
{
	int k;
	if ((front > rear) || (front == -1))
		return 0;
	else
	{
		k = q[front++];
		return k;
	}
}

void bfs(int s, int n)
{
	int p, i;

	add(s);
	vis[s] = culoare;
	p = delete_n();  //scoate un element din coada
	while (p != 0)   // adauga vecinii lui p in coada
	{
		for (i = 1; i <n; i++)
		if ((echivalente[p][i] != 0) && (vis[i] == 0))
		{
			add(i);
			vis[i] = culoare;
		}
		p = delete_n();
	}
	for (i = 1; i <n; i++) //ia nodurile nevizitate
		if (vis[i] == 0)
		{
			culoare++;
			bfs(i, n);
		}
}


void CDibView::OnLab5Etichetare()
{
	BEGIN_PROCESSING();

	int eticheta = 1;
	int culoare[9000];
	int x_label, b_label, c_label, d_label, a_label;
	//algoritm
	for (int i = dwHeight - 2; i >= 0; i--)
		for (int j = 1; j<dwWidth - 1; j++)
		if (lpSrc[i*w + j] == 0) //if x==0
		{
			if (lpSrc[(i + 1)*w + j - 1] == 0) //B==0
			if ((lpSrc[(i + 1)*w + j] != 0) && (lpSrc[(i + 1)*w + j + 1] == 0)) /// C!=0 and D==-
			{
				etichete[i][j] = etichete[i + 1][j - 1]; //x_label=b_label
				b_label = etichete[i + 1][j - 1];
				d_label = etichete[i + 1][j + 1];           //new pair(b_label,d_label)
				echivalente[b_label][d_label] = 1;
				echivalente[d_label][b_label] = 1;
			}
			else 
				etichete[i][j] = etichete[i + 1][j - 1];   //x_label=b_label
			else if (lpSrc[i*w + j - 1] == 0) //A==0
			if (lpSrc[(i + 1)*w + j] == 0) //C==0 
			{
				etichete[i][j] = etichete[i][j - 1]; //x_label=a_label
				a_label = etichete[i][j - 1];
				c_label = etichete[i + 1][j];           //new pair(a_label,c_label)
				echivalente[a_label][c_label] = 1;
				echivalente[c_label][a_label] = 1;
			}
			else if (lpSrc[(i + 1)*w + j + 1] != 0) //D!=0
				etichete[i][j] = etichete[i][j - 1];  //x_label=a_label   
			else
			{
				etichete[i][j] = etichete[i][j - 1];  //x_label=a_label   
				a_label = etichete[i][j - 1];
				d_label = etichete[i + 1][j + 1];           //new pair(a_label,d_label)
				echivalente[a_label][d_label] = 1;
				echivalente[d_label][a_label] = 1;
			}
			else
			if (lpSrc[(i + 1)*w + j] == 0)  //c==0
				etichete[i][j] = etichete[i + 1][j];//x_label=c_label
			else
			if (lpSrc[(i + 1)*w + j + 1] == 0) //D==0
				etichete[i][j] = etichete[i + 1][j + 1];
			else
			{
				etichete[i][j] = eticheta;
				eticheta++;
			}
		}

	//clase de ehivalenta
	bfs(0, eticheta);

	for (int r = 0; r<dwHeight; r++)
		for (int c = 0; c<dwWidth; c++)
		{
			int cul;
			cul = vis[etichete[r][c]];
			if (lpSrc[r*w + c] != 255)
				lpDst[r*w + c] = cul;
		}
	//
	for (int k = 1; k <= 254; k++)
	{
		bmiColorsDst[k].rgbRed = rand() % 254 + 1;
		bmiColorsDst[k].rgbGreen = rand() % 254 + 1;
		bmiColorsDst[k].rgbBlue = rand() % 254 + 1;
	}


	END_PROCESSING("etic");
}


void CDibView::OnLab6Algoritmurmarirecontur()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	//{x,y}

	//int d[8][2] {{1, 0}, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }};

	int v8[8][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } };

	int gasit = 0;

	int xv, yv, x, y;
	int dir = 7;

	FILE *f;
	char filename[] = "file.txt";
	fopen_s(&f, filename, "w");

	int x0, y0, x1, y1; // x0, y0, x1, y1
	int xn1, yn1; // xn-1, yn-1
	int xn, yn;   // xn , yn
	int start;
	int p1, p2;
	int k = 0;
	int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	int count = 0, count2 = 0;;

	for (int i = dwHeight - 1; i >= 0 && !gasit; i--){
		for (int j = 0; j < dwWidth - 1 && !gasit; j++)
		{
			if (lpSrc[i*w + j] == 0) {
				x0 = j;
				y0 = i;
				lpDst[i*w + j] = 125;
				gasit = 1;
			}
		}
	}

	CString info;
	info.Format(_TEXT("x=%d y=%d"), x0, y0);
	AfxMessageBox(info);


	if (gasit == 1) {
		x = x0;
		y = y0;

		gasit = 0;
		p1 = 0;
		dir = 7;

		do {
			fprintf(f, "%d ", dir);
			if (dir % 2 == 0)
				dir = (dir + 7) % 8;
			else
				dir = (dir + 6) % 8;


			gasit = 0;
			for (k = 0; k < 8 && !gasit; k++) {
				xv = x + v8[(k + dir) % 8][1];
				yv = y + v8[(k + dir) % 8][0];
				count++;

				if (lpSrc[yv*w + xv] == 0){
					lpDst[yv*w + xv] = 125;
					gasit = 1;
					dir = (k + dir) % 8;
				}
			}
			count2++;

			//xn-1
			xn1 = x;
			yn1 = y;

			if (p1 == 1)
				p1 = 2;

			//x1 - al doilea punct
			if (p1 == 0){
				x1 = xv;
				y1 = yv;
				p1 = 1;
			}

			x = xv;
			y = yv;
		} while (!(x0 == xn1 && y0 == yn1 && x1 == xv && y1 == yv && p1 == 2));
	}


	fclose(f);

	END_PROCESSING("contur");
}


void CDibView::OnLab6Reconstructiecontur()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	int x, y;
	int i, n, dir;
	int v8[8][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } };
	
	FILE *f;
	char filename[] = "reconstruct.txt";
	fopen_s(&f, filename, "r");

	if (f != NULL){
		fscanf_s(f, "%d %d", &x, &y);
		fscanf_s(f, "%d", &n);
		lpDst[x*w + y] = 0;
		for (i = 0; i<n; ++i){
			fscanf_s(f, "%d", &dir);
			x = x + v8[dir][0];
			y = y + v8[dir][1];
			lpDst[x*w + y] = 0;
		}
	}
	else{
		CString info;
		info.Format(_TEXT("Fisierul nu a fost gasit"));
		AfxMessageBox(info);
	
	}

	END_PROCESSING("Reconstructie");
}



void CDibView::OnLab7Operatiimorfologice()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	CDlgOperMorf d;
	d.DoModal();
	
	BYTE *lpTmp = new BYTE[w*dwHeight];
	BYTE *lpCompl = new BYTE[w*dwHeight];

	BYTE *x0 = new BYTE[w*dwHeight];
	BYTE *xK = new BYTE[w*dwHeight];
	BYTE *xK1 = new BYTE[w*dwHeight];

	memcpy(lpTmp, lpSrc, w*dwHeight);

	switch (d.m_oper_morf) {
		case 0:	
				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{
						if (lpSrc[i*w + j] == 0) {
							lpDst[(i + 1)*w + j] = 0;
							lpDst[(i - 1)*w + j] = 0;
							lpDst[i*w + j - 1] = 0;
							lpDst[i*w + j + 1] = 0;
						}
					}
				}
				break;

				
		case 1: for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{
						if (lpSrc[i*w + j] == 0)
							if (lpSrc[(i + 1)*w + j] == 255 || lpSrc[(i - 1)*w + j] == 255 || lpSrc[i*w + j - 1] == 255 || lpSrc[i*w + j + 1] == 255)
								lpDst[i*w + j] = 255;
							else
								lpDst[i*w + j] = 0;
					}
				}
				break;


		case 2: 
	
				for (int i = 1; i<= dwHeight -1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{
						if (lpSrc[i*w + j] == 0) {
							lpTmp[(i + 1)*w + j] = 0;
							lpTmp[(i - 1)*w + j] = 0;
							lpTmp[i*w + j - 1] = 0;
							lpTmp[i*w + j + 1] = 0;
						}

					}
				}

				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{ 
						if (lpTmp[i*w + j] == 0) {
							if (lpTmp[(i + 1)*w + j] == 255 || lpTmp[(i - 1)*w + j] == 255 || lpTmp[i*w + j - 1] == 255 || lpTmp[i*w + j + 1] == 255)
								lpDst[i*w + j] = 255;
							else
								lpDst[i*w + j] = 0;
						}

					}
				}

				//delete[] lpTmp;
				break;
	
	
		case 3:	

				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{
						if (lpSrc[i*w + j] == 0) {
							if (lpSrc[(i + 1)*w + j] == 255 || lpSrc[(i - 1)*w + j] == 255 || lpSrc[i*w + j - 1] == 255 || lpSrc[i*w + j + 1] == 255)
								lpTmp[i*w + j] = 255;
							else
								lpTmp[i*w + j] = 0;
						}
					}
				}
				memcpy(lpDst, lpTmp, w*dwHeight);

				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
						{
							if (lpTmp[i*w + j] == 0) {
								lpDst[(i + 1)*w + j] = 0;
								lpDst[(i - 1)*w + j] = 0;
								lpDst[i*w + j - 1] = 0;
								lpDst[i*w + j + 1] = 0;
							}


						}
				}
				break;

		case 4:
				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++)
					{
						if (lpSrc[i*w + j] == 0)
							if (lpSrc[(i + 1)*w + j] == 255 || lpSrc[(i - 1)*w + j] == 255 || lpSrc[i*w + j - 1] == 255 || lpSrc[i*w + j + 1] == 255)
								lpTmp[i*w + j] = 255;
							else
								lpTmp[i*w + j] = 0;
					}
				}



				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++) {
						if (lpTmp[i*w + j] == 0 && lpSrc[i*w + j] == 0)
							lpDst[i*w + j] = 255;
					}
				}
				break;
		case 5:
				int pX = dwWidth / 2;
				int pY = dwHeight / 2;
				lpDst[pY*w + pY] = 0;
				
				for (int i = 1; i <= dwHeight - 1; i++){
					for (int j = 1; j < dwWidth - 1; j++) {
						if (i == pX && j == pY) {
							x0[i*w + j] = 0;
						}
						else
							x0[i*w + j] = 255;
					}
				}

				int ok = 0;

				while (!ok) {


					for (int i = 1; i <= dwHeight - 1; i++){
						for (int j = 1; j < dwWidth - 1; j++) {
							if (lpSrc[i*w + j] == 0) {
								lpCompl[i*w + j] = 255;
							}
							else {
								lpCompl[i*w + j] = 0;
							}
						}
					}
				}
				break;
	}

	END_PROCESSING("OpMorfologice");
}


void CDibView::OnLab8Mediasidev()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	int h[256] = { 0 };
	float p[256] = { 0 };
	double medie = 0.0;
	double dev = 0.0;
	double M = w * dwHeight;
	
	// se va scrie codul pentru calcularea histogramei în ºirul de întregihistValues[256]
	// se va scrie codul pentru calcularea FDP în ºirul de valori reale FDPValues[256]
	// se instanþiazã un dialog box de afiºare ºi se asociazã histograma
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			p[lpSrc[i*w + j]] = (float)h[lpSrc[i*w + j]] / (dwHeight * dwWidth);
		}
	}


	for (int g = 0; g <= 255; g++) {
		medie += g*h[g];
	}
	medie = medie / M;

	for (int g = 0; g <= 255; g++) {
		dev += (g - medie)*(g - medie) * p[g];
	}
	dev = sqrt(dev);

	CString info;
	info.Format(_TEXT("Media=%f, Deviatia Standard=%f"),medie, dev);
	AfxMessageBox(info);

	END_PROCESSING("Media si Deviata Standarad");
}


void CDibView::OnLab8Binarizare32791()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING("Binarizare alb negru");

	double a[256] = { 0 };
	int h[256] = { 0 };
	double tmax = 0;
	double tnew = 0;
	int tpoz;
	double medie = 0.0;
	double M = w * dwHeight;

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	

	for (int g = 0; g <= 255; g++) {
		medie += g*h[g];
	}
	medie = medie / M;

	double mHigh[256] = { 0 };

	//double mLow[256] = { 0 };

	a[0] = w*dwHeight;
	for (int g = 1; g <= 255; g++) {
		a[g] = a[g - 1] - h[g - 1];
	}


	mHigh[0] = medie * M;
	for (int t = 1; t <= 255; t++) {
		mHigh[t] = mHigh[t - 1] - (t-1)*h[t-1];
	}

	double numitor1 = 0.0;
	double numarator1 = 0.0;
	double numarator2 = 0.0;
	double numitor2 = 0.0;
	
	numarator1 = (medie * M - mHigh[0])*(medie * M - mHigh[0]);
	numarator2 = mHigh[0] * mHigh[0];
	numitor1 = M - a[0];
	numitor2 = a[0];

	if (numitor1 !=0 && numitor2 != 0)
		tmax = numarator1 / numitor1 + numarator2 / numitor2;
	tpoz = 1;
	for (int t = 1; t <= 255; t++) {
		numarator1 = (medie * M - mHigh[t])*(medie * M - mHigh[t]);
		numarator2 = mHigh[t] * mHigh[t];
		numitor1 = M - a[t];
		numitor2 = a[t];

		if (numitor1 != 0 && numitor2 != 0) {
			tnew = numarator1 / numitor1 + numarator2 / numitor2;
			if (tnew > tmax) {
				tmax = tnew;
				tpoz = t;
			}
		}
	
	}


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	double gHigh = 0.0;
	double gLow = 0.0;
	
	numitor1 = 0.0;
	numarator1 = 0.0;
	numarator2 = 0.0;
	numitor2 = 0.0;

	for (int g = 0; g < tpoz; g++) {
		numarator1 += g*h[g];
		numitor1 += h[g];
	}


	for (int g = tpoz; g <=255; g++) {
		numarator2 += g*h[g];
		numitor2 += h[g];
	}

	gLow = numarator1 / numitor1;
	gHigh = numarator2 / numitor2;



	CString info;
	info.Format(_TEXT("Tmax = %d, Ghigh=%f, Glow=%f"), tpoz, gHigh, gLow);
	AfxMessageBox(info);

	
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] < tpoz) {
				lpDst[i*w + j] = gLow;
			}
			else
				lpDst[i*w + j] = gHigh;
		}
	}

	END_PROCESSING("OpMorfologice");
}


void CDibView::OnLab8Binarizare()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING("Binarizare alb negru");

	double a[256] = { 0 };
	int h[256] = { 0 };
	int tmax = 0;
	int tpoz;

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	a[0] = w*dwHeight;
	for (int g = 1; g <= 255; g++) {
		a[g] = a[g - 1] - h[g - 1];
	}

	tmax = a[1]; tpoz = 1;
	for (int g = 2; g <= 255; g++) {
		if (g*a[g] > tmax) {
			tmax = g*a[g];
			tpoz = g;
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if (lpSrc[i*w + j] < tpoz) {
				lpDst[i*w + j] = 0;
			}
			else
				lpDst[i*w + j] = 255;
		}
	}


	CString info;
	info.Format(_TEXT("Tmax = %d"), tpoz);
	AfxMessageBox(info);



	END_PROCESSING("OpMorfologice");
}


void CDibView::OnLab8Modificareluminozitate()
{
	
	BEGIN_PROCESSING("ModifLuminozitate");
	// TODO: Add your command handler code here
	//int gOutMin = 20;
	//int gOutMax = 240;

	int offset = 100;
	int p = 0;
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			
			p = lpSrc[i*w + j] + offset;
			if (p < 0)
				lpDst[i*w + j] = 0;
			else if (p>255)
				lpDst[i*w + j] = 255;
			else
				lpDst[i*w + j] = p;
		}
	}

	END_PROCESSING("ModifLuminozitate");
}


void CDibView::OnLab8Modificarecontrast()
{
	BEGIN_PROCESSING("ModifLuminozitate");

	// TODO: Add your command handler code here
	double gOutMin = 20;
	double gOutMax = 240;
	double gInMin = 0;
	double gInMax = 0;

	int h[256] = { 0 };
	int tmax = 0;
	int tpoz;

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}
	int ok1 = 0;

	for (int g = 0; g <= 255; g++) {
		if (h[g] != 0 && ok1 == 0) {
			ok1 = 1;
			gInMin = g;
		}
		else if (h[g] != 0)
			gInMax = g;
	}


	
	CString info;
	info.Format(_TEXT("gNmax=%f ginMin=%f"), gInMax, gInMin);
	AfxMessageBox(info);


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
				lpDst[i*w + j] = gOutMin + (lpSrc[i*w + j] - gInMin) * ((double) (gOutMax - gOutMin) / (double)(gInMax - gInMin));

		}
	}

	END_PROCESSING("ModifLuminozitate");

}


void CDibView::OnLab8Corectiegamma()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("CorectieGamma");


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			lpDst[i*w + j] = 255 * pow((double)lpSrc[i*w + j] / 255, 2);
		}
	}

	END_PROCESSING("CorectieGamma");

}


//p(g), pc(g)

void CDibView::OnLab8Egalizarehistograma()
{

	BEGIN_PROCESSING("ModifLuminozitate");

	// TODO: Add your command handler code here
	int h[256] = { 0 };
	double pc[256];
	int s = 0;
	int gout[256];

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			h[lpSrc[i*w + j]]++;
		}
	}

	for (int i = 0; i<256; ++i){
		s = 0;
		for (int j = 0; j <= i; ++j)
			s += h[j];
		pc[i] = s / (double)(dwWidth*dwHeight);
	}

	//pc[0] = h[0]/


	for (int i = 0; i <= 255; ++i){
		gout[i] = (int)(255 * pc[i]);
	}


	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j<dwWidth; j++){
			lpDst[i*w + j] = gout[lpSrc[i*w + j]];
		}



	END_PROCESSING("CorectieGamma");

}



void CDibView::OnLab9Filtrumediearitmetica()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Filtru Medie Aritmetica 3x3");

	int H[3][3] = { { 1, 1, 1 },
					{ 1, 1, 1 },
					{ 1, 1, 1 } };

	int s = 9;
	int d = 1;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{		int temp = 0;
				for (int k = -d; k <= d; k++) {
					for  (int m = -d; m <= d; m++) {
						temp += lpSrc[(i + k)*w + (j +m)] * H[k + d][m + d];
					}
				}

				temp = temp / s;
				if (temp < 0)
					lpDst[i*w + j] = 0;
				else 
					if (temp > 255)
						lpDst[i*w + j] = 255;
					else
						lpDst[i*w + j] = temp;
		}
	}

	END_PROCESSING("Filtru Medie Aritmetica 3x3");

}


void CDibView::OnLab9Filtrumediearitmetica5x5()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Filtru Medie Aritmetica 5x5");

	int H[5][5] = { { 1, 1, 1, 1, 1 },
					{ 1, 1, 1, 1, 1, },
					{ 1, 1, 1, 1, 1 },
					{ 1, 1, 1, 1, 1 },
					{ 1, 1, 1, 1, 1 } };

	int s = 25;
	int d = 2;

	for (int i = 2; i < dwHeight - 2; i++) {
		for (int j = 2; j < dwWidth - 2; j++)
		{
			int temp = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {
					temp += lpSrc[(i + k)*w + (j + m)] * H[k + d][m + d];
				}
			}

			temp = temp / s;
			if (temp < 0)
				lpDst[i*w + j] = 0;
			else
				if (temp > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = temp;
		}
	}

	END_PROCESSING("Filtru Medie Aritmetica 5x5");
}


void CDibView::OnLab9Filtrugaussian3x3()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING("Filtru Gaussian");

	int H[3][3] = { { 1, 2, 1 },
					{ 2, 4, 2 },
					{ 1, 2, 1 } };

	int s = 16;
	int d = 1;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			int temp = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {
					temp += lpSrc[(i + k)*w + (j + m)] * H[k + d][m + d];
				}
			}

			temp = temp / s;
			if (temp < 0)
				lpDst[i*w + j] = 0;
			else
				if (temp > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = temp;
		}
	}

	END_PROCESSING("Filtru Gaussian");
}


/*

}
}

*/

void CDibView::OnLab9Filtrulaplace3x3()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING("Filtru Laplace 3x3");

	/*
	int H[3][3] = { { 0, -1, 0 },
	{ -1, 4, -1 },
	{ 0, -1, 0 } };
	*/

	int H[3][3] = { { -1, -1, -1 },
	{ -1, 8, -1 },
	{ -1, -1, -1 } };


	//int s = 16;
	int d = 1;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			int temp = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {
					temp += lpSrc[(i + k)*w + (j + m)] * H[k + d][m + d];
				}
			}

			//temp = temp / s;
			
			if (temp < 0)
				lpDst[i*w + j] = 0;
			else
				if (temp > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = temp;
		}
	}

	END_PROCESSING("Filtru Laplace 3x3");
}


void CDibView::OnLab9Filtrutrecesus3x3()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING("Filtru Trece-SUs");

	/*
	int H[3][3] = { { 0, -1, 0 },
	{ -1, 4, -1 },
	{ 0, -1, 0 } };
	*/

	int H[3][3] = { { -1, -1, -1 },
	{ -1, 9, -1 },
	{ -1, -1, -1 } };


	//int s = 16;
	int d = 1;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			int temp = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {
					temp += lpSrc[(i + k)*w + (j + m)] * H[k + d][m + d];
				}
			}

			//temp = temp / s;
			if (temp < 0)
				lpDst[i*w + j] = 0;
			else
				if (temp > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = temp;
		}
	}

	END_PROCESSING("Filtru Trece-sus");
}


void CDibView::OnLab9Logaritmmagnitudine()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Magnitudine");

	double *srcCentrat = new double[w*dwHeight];
	double *xRe = new double[w*dwHeight];
	double *xIm = new double[w*dwHeight];

	double *logartim = new double[w*dwHeight];


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				srcCentrat[i*w + j] = (-1)*lpSrc[i*w + j];
			else
				srcCentrat[i*w + j] = lpSrc[i*w + j];
		}
	}

	fftimage(dwWidth, dwHeight, srcCentrat, (double*)0, xRe, xIm);
	
	double max = 0;
	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			logartim[i*w + j] = log(sqrt(xRe[i*w + j] * xRe[i*w + j] + xIm[i*w + j] * xIm[i*w + j]) + 1);
			if (logartim[i*w + j] > max)
				max = logartim[i*w + j];
		}
	}

	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			lpDst[i*w + j] = logartim[i*w + j] * 255 / max;
		}
	}

	END_PROCESSING("Magnitudine");

}


void CDibView::OnLab9Ftsideal()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("FTS ideal");
	int R = 10;
	double *srcCentrat = new double[w*dwHeight];
	double *dstCentrat = new double[w*dwHeight];

	double *xRe = new double[w*dwHeight];
	double *xIm = new double[w*dwHeight];

	double *logartim = new double[w*dwHeight];


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				srcCentrat[i*w + j] = (-1)*lpSrc[i*w + j];
			else
				srcCentrat[i*w + j] = lpSrc[i*w + j];
		}
	}

	fftimage(dwWidth, dwHeight, srcCentrat, (double*)0, xRe, xIm);


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
					double val = (i - dwHeight / 2)*(i - dwHeight / 2) + (j - dwWidth / 2)*(j - dwWidth / 2);
					if (val > R*R) {
							xRe[i*w + j] = 0;
							xIm[i*w + j] = 0;
					}		
		}
	}

	ifftimage(dwWidth, dwHeight, xRe, xIm, dstCentrat, (double *) 0);



	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				dstCentrat[i*w + j] = (-1)*dstCentrat[i*w + j];
			else
				dstCentrat[i*w + j] = dstCentrat[i*w + j];
		}
	}


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			if (dstCentrat[i*w + j] < 0) {
				lpDst[i*w + j] = 0;
			}
			else 
				if (dstCentrat[i*w + j] > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = dstCentrat[i*w + j];
		}
	}

	
	END_PROCESSING("FTS ideal");

}



void CDibView::OnLab9Ftsideal32803()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("FTS ideal");
	int R = 10;
	double *srcCentrat = new double[w*dwHeight];
	double *dstCentrat = new double[w*dwHeight];

	double *xRe = new double[w*dwHeight];
	double *xIm = new double[w*dwHeight];

	double *logartim = new double[w*dwHeight];


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				srcCentrat[i*w + j] = (-1)*lpSrc[i*w + j];
			else
				srcCentrat[i*w + j] = lpSrc[i*w + j];
		}
	}

	fftimage(dwWidth, dwHeight, srcCentrat, (double*)0, xRe, xIm);


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			double val = (i - dwHeight / 2)*(i - dwHeight / 2) + (j - dwWidth / 2)*(j - dwWidth / 2);
			if (val > R*R) {
				xRe[i*w + j] = 0;
				xIm[i*w + j] = 0;
			}
		}
	}

	ifftimage(dwWidth, dwHeight, xRe, xIm, dstCentrat, (double *)0);



	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				dstCentrat[i*w + j] = (-1)*dstCentrat[i*w + j];
			else
				dstCentrat[i*w + j] = dstCentrat[i*w + j];
		}
	}


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			if (dstCentrat[i*w + j] < 0) {
				lpDst[i*w + j] = 0;
			}
			else
				if (dstCentrat[i*w + j] > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = dstCentrat[i*w + j];
		}
	}


	END_PROCESSING("FTS ideal");
}


void CDibView::OnLab9Ftstaieregaussiana32805()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Taiere Gausiana FTJ");
	int R = 10;
	double A = 10;
	double *srcCentrat = new double[w*dwHeight];
	double *dstCentrat = new double[w*dwHeight];

	double *xRe = new double[w*dwHeight];
	double *xIm = new double[w*dwHeight];

	double *logartim = new double[w*dwHeight];


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				srcCentrat[i*w + j] = (-1)*lpSrc[i*w + j];
			else
				srcCentrat[i*w + j] = lpSrc[i*w + j];
		}
	}

	fftimage(dwWidth, dwHeight, srcCentrat, (double*)0, xRe, xIm);


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
				 
			double aux = ((i - dwHeight / 2)*(i - dwHeight / 2) + (j - dwWidth / 2)*(j - dwWidth / 2));
				xRe[i*w + j] = xRe[i*w + j] * exp((-1)*aux / (A*A));
				xIm[i*w + j] = xIm[i*w + j] * exp(-(1)*aux/ (A*A));
			
		}
	}

	ifftimage(dwWidth, dwHeight, xRe, xIm, dstCentrat, (double *)0);



	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				dstCentrat[i*w + j] = (-1)*dstCentrat[i*w + j];
			else
				dstCentrat[i*w + j] = dstCentrat[i*w + j];
		}
	}


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			if (dstCentrat[i*w + j] < 0) {
				lpDst[i*w + j] = 0;
			}
			else
				if (dstCentrat[i*w + j] > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = dstCentrat[i*w + j];
		}
	}


	END_PROCESSING("Taiere Gausiana FTJ");

}


void CDibView::OnLab9Ftstaieregaussiana()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Taiere Gausiana FTS");
	int R = 10;
	double A = 10;
	double *srcCentrat = new double[w*dwHeight];
	double *dstCentrat = new double[w*dwHeight];

	double *xRe = new double[w*dwHeight];
	double *xIm = new double[w*dwHeight];

	double *logartim = new double[w*dwHeight];


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				srcCentrat[i*w + j] = (-1)*lpSrc[i*w + j];
			else
				srcCentrat[i*w + j] = lpSrc[i*w + j];
		}
	}

	fftimage(dwWidth, dwHeight, srcCentrat, (double*)0, xRe, xIm);


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			double aux = ((i - dwHeight / 2)*(i - dwHeight / 2) + (j - dwWidth / 2)*(j - dwWidth / 2));
			xRe[i*w + j] = xRe[i*w + j] * (1 - exp((-1)*aux / (A*A)));
			xIm[i*w + j] = xIm[i*w + j] * (1 - exp((-1)*aux / (A*A)));

		}
	}

	ifftimage(dwWidth, dwHeight, xRe, xIm, dstCentrat, (double *)0);



	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {
			if ((i + j) % 2 != 0)
				dstCentrat[i*w + j] = (-1)*dstCentrat[i*w + j];
			else
				dstCentrat[i*w + j] = dstCentrat[i*w + j];
		}
	}


	for (int i = 0; i < dwHeight; i++) {
		for (int j = 0; j < dwWidth; j++) {

			if (dstCentrat[i*w + j] < 0) {
				lpDst[i*w + j] = 0;
			}
			else
				if (dstCentrat[i*w + j] > 255)
					lpDst[i*w + j] = 255;
				else
					lpDst[i*w + j] = dstCentrat[i*w + j];
		}
	}


	END_PROCESSING("Taiere Gausiana FTS");
}


void CDibView::OnL10Filtrugaussian2d()
{
	// TODO: Add your command handler code here

	/**/
	BEGIN_PROCESSING("Filtru Gaussian");
	int wA = 5;
	double sigma = wA / 6.0;
	double result = 0;

	//double G[5][5];
	double *G = new double[w*dwHeight];

	//double *g[] = new double[w*dwHeight];

	for (int x = 0; x < wA; x++) {
		for (int y = 0; y < wA; y++) {
			double numitor = 2.0 * PI*sigma*sigma;
			double numarator = exp(-((x - wA / 2)*(x - wA / 2) + (y - wA / 2)*(y - wA / 2)) / (2 * sigma*sigma));

			G[x*w + y] = numarator / numitor;
		}
	}

	int sum = 0;
	for (int i = wA/2; i < dwHeight - wA/2; i++) {
		for (int j = wA/2; j < dwWidth - wA/2; j++) {

			
			for (int x = -wA / 2; x <= wA / 2; x++) {
				for (int y = -wA / 2; y <= wA / 2; y++) {
					/*
					double numitor = 2 * PI*sigma*sigma;
					double numarator = exp(-((x - w / 2)*(x - w / 2) + (y - w / 2)*(y - w / 2)) / (2 * sigma*sigma));
					double result = numarator / numitor;
					*/

					sum += G[(x + wA/2 )*w + y + wA/2] * lpSrc[(i + x)*w + j + y];
				}
			}
			lpDst[i*w + j] = sum;
			sum = 0;
		}
	}

	END_PROCESSING("Filtru Gaussian");
}

void CDibView::OnL10Filtrugaussian2d32807()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING("Filtru Gaussian");

	int wA = 5;
	double sigma = wA / 6.0;

	double *lpTmp = new double[w*dwHeight];

	double *Gx = new double[w*dwHeight];
	double *Gy = new double[w*dwHeight];


		for (int y = 0; y < wA; y++) {
			double numitor = sqrt(2*PI)*sigma;
			double numarator = exp(-((y - wA / 2)*(y - wA / 2)) / (2 * sigma*sigma));
			Gy[y] = numarator / numitor;
		}

		for (int x = 0; x < wA; x++) {
			double numitor = sqrt(2 * PI)*sigma;
			double numarator2 = exp(-((x - wA / 2)*(x - wA / 2)) / (2 * sigma*sigma));
			Gx[x] = numarator2 / numitor;
		}

	
		double sum = 0;
		for (int i = wA / 2; i < dwHeight - wA / 2; i++) {
			for (int j = wA / 2; j < dwWidth - wA / 2; j++) {
				
				for (int x = -wA / 2; x <= wA / 2; x++) {
					for (int y = -wA / 2; y <= wA / 2; y++) {
						sum += Gx[x*w] * Gy[y];
						//sum += Gx[x*w] * lpSrc[i*w + j];
					}
				}
			lpTmp[i*w + j] = sum;
			sum = 0;
		}
	}


	for (int i = wA / 2; i < dwHeight - wA / 2; i++) {
		for (int j = wA / 2; j < dwWidth - wA / 2; j++) {
			lpDst[i*w + j] = lpTmp[i*w + j] * lpDst[i*w+j];
		}
	}

	/*
	int sum = 0;
	for (int i = wA / 2; i < dwHeight - wA / 2; i++) {
		for (int j = wA / 2; j < dwWidth - wA / 2; j++) {

			for (int x = -wA / 2; x <= wA / 2; x++) {
				for (int y = -wA / 2; y <= wA / 2; y++) {
					sum += Gx[(x + wA / 2)*w + y + wA / 2] * Gy[(x + wA / 2)*w + y + wA / 2];
				}
			}
			lpDst[i*w + j] = sum*lpSrc[i*w+j];
			sum = 0;
			
		}
	}

	*/


	END_PROCESSING("Filtru Gaussian");

}


int cmpfunc(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}

void CDibView::OnL10Filtrumedian()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING();

	int wA = 5;
	int *matrix = new int[wA*wA];


	for (int i = wA/2; i<dwHeight - wA/2; i++)
		for (int j = wA/2; j<dwWidth - wA/2; j++){

			
			int x = 0;
			for (int k = -wA/2; k<=wA/2; k++)
				for (int l = -wA/2; l<=wA/2; l++){
					matrix[x] = lpSrc[(i + k)*w + (j + l)];
					x++;
				}

			/*
			boolean isSorted = false;
			while (isSorted == false){
				isSorted = true;
				for (int l = 0; l<wA*wA - 1; l++)
					if (matrix[l]<matrix[l + 1])
					{
						int aux = matrix[l];
						matrix[l] = matrix[l + 1];
						matrix[l + 1] = aux;
						isSorted = false;
					}
			}
			*/


			/* using algorithm lib */
			std::sort(matrix, matrix + wA*wA);

			//qsort(matrix, wA*wA, sizeof(int), cmpfunc);
			lpDst[i*w + j] = matrix[(wA*wA)/2];

		}
	END_PROCESSING("Salt and Pepper");
}


void CDibView::OnL11Filtraregaussiana()
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here

	/**/
	BEGIN_PROCESSING("Filtru Gaussian");
	int wA = 5;
	double sigma = wA / 6.0;
	double result = 0;

	//double G[5][5];
	double *G = new double[w*dwHeight];

	//double *g[] = new double[w*dwHeight];

	for (int x = 0; x < wA; x++) {
		for (int y = 0; y < wA; y++) {
			double numitor = 2.0 * PI*sigma*sigma;
			double numarator = exp(-((x - wA / 2)*(x - wA / 2) + (y - wA / 2)*(y - wA / 2)) / (2 * sigma*sigma));

			G[x*w + y] = numarator / numitor;
		}
	}

	int sum = 0;
	for (int i = wA / 2; i < dwHeight - wA / 2; i++) {
		for (int j = wA / 2; j < dwWidth - wA / 2; j++) {


			for (int x = -wA / 2; x <= wA / 2; x++) {
				for (int y = -wA / 2; y <= wA / 2; y++) {
					/*
					double numitor = 2 * PI*sigma*sigma;
					double numarator = exp(-((x - w / 2)*(x - w / 2) + (y - w / 2)*(y - w / 2)) / (2 * sigma*sigma));
					double result = numarator / numitor;
					*/

					sum += G[(x + wA / 2)*w + y + wA / 2] * lpSrc[(i + x)*w + j + y];
				}
			}
			lpDst[i*w + j] = sum;
			sum = 0;
		}
	}

	END_PROCESSING("Filtru Gaussian");
}


void CDibView::OnL11Modululorientareagradientilor()
{
	// TODO: Add your command handler code here
	double *G, *GS;
	double *fi;

	BEGIN_PROCESSING("Modul Directie Gradient");

	int Sy[3][3] = { { -1, -2, -1 },
	{ 0, 0, 0 },
	{ 1, 2, 1 } };

	int Sx[3][3] = { { -1, 0, 1 },
	{ -2, 0, 2 },
	{ -1, 0, 1 } };

	//Modul gradient
	G = new double[w*dwHeight];
	GS = new double[w*dwHeight];

	//Unghi fi -- atan2(Gy,Gx) -> (-PI,PI)
	fi = new double[w*dwHeight];
	//lpDst -> modul gradient (minim dintre g si 255)
	//int s = 16;
	int d = 1;



	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			//int dir = 0;
			int temp = 0, temp2 = 0; double modul = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {

					//Gx
					temp += lpSrc[(i + k)*w + (j + m)] * Sx[k + d][m + d];

					//Gy
					temp2 += lpSrc[(i + k)*w + (j + m)] * Sy[k + d][m + d];
				}
			}

			modul = sqrt(temp*temp + temp2*temp2);
			G[i*w + j] = modul;
			fi[i*w + j] = atan2(temp2, temp);

			lpDst[i*w + j] = min(255, modul);
		}
	}

	//directie
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {

			int dir = 0;
			
			if ((fi[i*w + j] >= 0 && fi[i*w + j] <= M_PI / 8) ||
				(fi[i*w + j] <= 0 && fi[i*w + j] >= -M_PI / 8) ||
				(fi[i*w + j] >= 7 * M_PI / 8 && fi[i*w + j] <= M_PI) ||
				(fi[i*w + j] <= -7 * M_PI / 8 && fi[i*w + j] >= -M_PI))
				dir = 0;
			else
				if ((fi[i*w + j] >= M_PI / 8 && fi[i*w + j] <= 3 * M_PI / 8) ||
					(fi[i*w + j] <= -5 * M_PI / 8 && fi[i*w + j] >= -7 * M_PI / 8))
					dir = 1;
				else
					if ((fi[i*w + j] >= 3 * M_PI / 8 && fi[i*w + j] <= 5 * M_PI / 8) ||
						(fi[i*w + j] <= -3 * M_PI / 8 && fi[i*w + j] >= -5 * M_PI / 8))
						dir = 2;
					else
						dir = 3;


			if (dir == 0) {
				if (G[i*w + j + 1] <= G[i*w + j] && G[i*w + j - 1] <= G[i*w + j])
					GS[i*w + j] = G[i*w + j];
				else
					GS[i*w + j] = 0;
			}
			else
				if (dir == 1)
					if (G[(i + 1)*w + j + 1] <= G[i*w + j] && G[(i - 1)*w + j - 1] <= G[i*w + j])
						GS[i*w + j] = G[i*w + j];
					else
						GS[i*w + j] = 0;
					else
						if (dir == 2)
							if (G[(i + 1)*w + j] <= G[i*w + j] && G[(i - 1)*w + j] <= G[i*w + j])
								GS[i*w + j] = G[i*w + j];
							else
								GS[i*w + j] = 0;
							else
								if (dir == 3)
									if (G[(i + 1)*w + j - 1] <= G[i*w + j] && G[(i - 1)*w + j + 1] <= G[i*w+j])
										GS[i*w + j] = G[i*w + j];
									else
										GS[i*w + j] = 0;

			lpDst[i*w + j] = min(GS[i*w + j],255);
		}
	}


	END_PROCESSING("Modul Directie Gradient");
}


void CDibView::OnL11Binarizareaadaptiva()
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	double *G, *GS;
	double *fi;
	double *GSN;

	BEGIN_PROCESSING("Binarizare Adaptiva");

	int Sy[3][3] = { { -1, -2, -1 },
	{ 0, 0, 0 },
	{ 1, 2, 1 } };

	int Sx[3][3] = { { -1, 0, 1 },
	{ -2, 0, 2 },
	{ -1, 0, 1 } };

	//Modul gradient
	G = new double[w*dwHeight];
	GS = new double[w*dwHeight];

	//Unghi fi -- atan2(Gy,Gx) -> (-PI,PI)
	fi = new double[w*dwHeight];
	//lpDst -> modul gradient (minim dintre g si 255)
	//int s = 16;
	int d = 1;



	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			//int dir = 0;
			int temp = 0, temp2 = 0; double modul = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {

					//Gx
					temp += lpSrc[(i + k)*w + (j + m)] * Sx[k + d][m + d];

					//Gy
					temp2 += lpSrc[(i + k)*w + (j + m)] * Sy[k + d][m + d];
				}
			}

			modul = sqrt(temp*temp + temp2*temp2);
			G[i*w + j] = modul;
			fi[i*w + j] = atan2(temp2, temp);

			lpDst[i*w + j] = min(255, modul);
		}
	}

	//directie
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {

			int dir = 0;

			if ((fi[i*w + j] >= 0 && fi[i*w + j] <= M_PI / 8) ||
				(fi[i*w + j] <= 0 && fi[i*w + j] >= -M_PI / 8) ||
				(fi[i*w + j] >= 7 * M_PI / 8 && fi[i*w + j] <= M_PI) ||
				(fi[i*w + j] <= -7 * M_PI / 8 && fi[i*w + j] >= -M_PI))
				dir = 0;
			else
				if ((fi[i*w + j] >= M_PI / 8 && fi[i*w + j] <= 3 * M_PI / 8) ||
					(fi[i*w + j] <= -5 * M_PI / 8 && fi[i*w + j] >= -7 * M_PI / 8))
					dir = 1;
				else
					if ((fi[i*w + j] >= 3 * M_PI / 8 && fi[i*w + j] <= 5 * M_PI / 8) ||
						(fi[i*w + j] <= -3 * M_PI / 8 && fi[i*w + j] >= -5 * M_PI / 8))
						dir = 2;
					else
						dir = 3;


			if (dir == 0) {
				if (G[i*w + j + 1] <= G[i*w + j] && G[i*w + j - 1] <= G[i*w + j])
					GS[i*w + j] = G[i*w + j];
				else
					GS[i*w + j] = 0;
			}
			else
				if (dir == 1)
					if (G[(i + 1)*w + j + 1] <= G[i*w + j] && G[(i - 1)*w + j - 1] <= G[i*w + j])
						GS[i*w + j] = G[i*w + j];
					else
						GS[i*w + j] = 0;
				else
					if (dir == 2)
						if (G[(i + 1)*w + j] <= G[i*w + j] && G[(i - 1)*w + j] <= G[i*w + j])
							GS[i*w + j] = G[i*w + j];
						else
							GS[i*w + j] = 0;
					else
						if (dir == 3)
							if (G[(i + 1)*w + j - 1] <= G[i*w + j] && G[(i - 1)*w + j + 1] <= G[i*w + j])
								GS[i*w + j] = G[i*w + j];
							else
								GS[i*w + j] = 0;

			lpDst[i*w + j] = min(GS[i*w + j], 255);
		}
	}


	//Prelungirea muchiilor
	int dx[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int dy[] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	int histM[256] = { 0 };
	GSN = new double[w*dwHeight];

	//Normalizare gradient
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			GS[i*w + j] = GS[i*w+j]/ (4 * sqrt(2));
		}
	}

	//Calculare histograma gradientului normalizat
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			histM[(int)GS[i*w + j]]++;
		}
	}


	double NrPcMuchie = 0;
	double NrPctGradient = 0;
	double NrPctGradientNull = histM[0];

	//Calculare NrPcteGradient
	for (int i = 0; i < 256; i++)
		NrPctGradient += histM[i];

	//Calculare NrPcteMuchie
	double p = 0.1; //prag
	NrPcMuchie = p*(NrPctGradient - NrPctGradientNull);

	double Thigh = 0;
	double Thighint = 0;
	for (int k = 255; k > 0; k--) {
		if (Thigh < NrPcMuchie) {
			Thigh = Thigh + histM[k];
			Thighint = k;
		}
	}

	double Tlow = 0.4*Thighint;

	int x = 0;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			if (GS[i*w + j] > Thighint)
				lpDst[i*w + j] = 255;
			else
				if (GS[i*w + j] < Tlow)
					lpDst[i*w + j] = 0;
				else
					lpDst[i*w + j] = 128;
		}
	}

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			if (GS[i*w + j] == 255)
			{	
				CPoint p;
				p.x = i;
				p.y = w*j;

				//std:queue<CPoint> Q;


			}
		}
	}

	END_PROCESSING("Binarizare Adaptiva");

}


void CDibView::OnL11Prelungireamuchiilor()
{
	// TODO: Add your command handler code here
	double *G, *GS;
	double *fi;
	double *GSN;
	BEGIN_PROCESSING("Prelungire Muchii");

	int Sy[3][3] = { { -1, -2, -1 },
	{ 0, 0, 0 },
	{ 1, 2, 1 } };

	int Sx[3][3] = { { -1, 0, 1 },
	{ -2, 0, 2 },
	{ -1, 0, 1 } };

	//Modul gradient
	G = new double[w*dwHeight];
	GS = new double[w*dwHeight];

	//Unghi fi -- atan2(Gy,Gx) -> (-PI,PI)
	fi = new double[w*dwHeight];
	//lpDst -> modul gradient (minim dintre g si 255)
	//int s = 16;
	int d = 1;



	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++)
		{
			//int dir = 0;
			int temp = 0, temp2 = 0; double modul = 0;
			for (int k = -d; k <= d; k++) {
				for (int m = -d; m <= d; m++) {

					//Gx
					temp += lpSrc[(i + k)*w + (j + m)] * Sx[k + d][m + d];

					//Gy
					temp2 += lpSrc[(i + k)*w + (j + m)] * Sy[k + d][m + d];
				}
			}

			modul = sqrt(temp*temp + temp2*temp2);
			G[i*w + j] = modul;
			fi[i*w + j] = atan2(temp2, temp);

			lpDst[i*w + j] = min(255, modul);
		}
	}

	//directie
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {

			int dir = 0;

			if ((fi[i*w + j] >= 0 && fi[i*w + j] <= M_PI / 8) ||
				(fi[i*w + j] <= 0 && fi[i*w + j] >= -M_PI / 8) ||
				(fi[i*w + j] >= 7 * M_PI / 8 && fi[i*w + j] <= M_PI) ||
				(fi[i*w + j] <= -7 * M_PI / 8 && fi[i*w + j] >= -M_PI))
				dir = 0;
			else
			if ((fi[i*w + j] >= M_PI / 8 && fi[i*w + j] <= 3 * M_PI / 8) ||
				(fi[i*w + j] <= -5 * M_PI / 8 && fi[i*w + j] >= -7 * M_PI / 8))
				dir = 1;
			else
			if ((fi[i*w + j] >= 3 * M_PI / 8 && fi[i*w + j] <= 5 * M_PI / 8) ||
				(fi[i*w + j] <= -3 * M_PI / 8 && fi[i*w + j] >= -5 * M_PI / 8))
				dir = 2;
			else
				dir = 3;


			if (dir == 0) {
				if (G[i*w + j + 1] <= G[i*w + j] && G[i*w + j - 1] <= G[i*w + j])
					GS[i*w + j] = G[i*w + j];
				else
					GS[i*w + j] = 0;
			}
			else
			if (dir == 1)
			if (G[(i + 1)*w + j + 1] <= G[i*w + j] && G[(i - 1)*w + j - 1] <= G[i*w + j])
				GS[i*w + j] = G[i*w + j];
			else
				GS[i*w + j] = 0;
			else
			if (dir == 2)
			if (G[(i + 1)*w + j] <= G[i*w + j] && G[(i - 1)*w + j] <= G[i*w + j])
				GS[i*w + j] = G[i*w + j];
			else
				GS[i*w + j] = 0;
			else
			if (dir == 3)
			if (G[(i + 1)*w + j - 1] <= G[i*w + j] && G[(i - 1)*w + j + 1] <= G[i*w + j])
				GS[i*w + j] = G[i*w + j];
			else
				GS[i*w + j] = 0;

			lpDst[i*w + j] = min(GS[i*w + j], 255);
		}
	}


	//Prelungirea muchiilor
	int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	int histM[256] = { 0 };
	//GSN = new double[w*dwHeight];

	//Normalizare gradient
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			GS[i*w + j] = GS[i*w + j] / (4 * sqrt(2));
		}
	}

	//Calculare histograma gradientului normalizat
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			histM[(int)GS[i*w + j]]++;
		}
	}


	double NrPcMuchie = 0;
	double NrPctGradient = 0;
	double NrPctGradientNull = histM[0];

	//Calculare NrPcteGradient
	for (int i = 0; i < 256; i++)
		NrPctGradient += histM[i];

	//Calculare NrPcteMuchie
	double p = 0.1; //prag
	NrPcMuchie = p*(NrPctGradient - NrPctGradientNull);

	double Thigh = 0;
	double Thighint = 0;
	for (int k = 255; k > 0; k--) {
		if (Thigh < NrPcMuchie) {
			Thigh = Thigh + histM[k];
			Thighint = k;
		}
	}

	double Tlow = 0.4*Thighint;

	int x = 0;

	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			if (GS[i*w + j] > Thighint)
				GS[i*w + j] = 255;
			else
			if (GS[i*w + j] < Tlow)
				GS[i*w + j] = 0;
			else
				GS[i*w + j] = 128;
		}
	}
	
	for (int i = dwHeight - 1; i >= 1; i--) {
		for (int j = 1; j < dwWidth - 1; j++) {
			if (GS[i*w + j] == 255)
			{
				CPoint p;
				p.x = j;
				p.y = i;
				
				std::queue<CPoint> Q;
				//Adaugam pixelul de muchie tare detectat in coada
				Q.push(p);

				//Prelucram toti vecinii pixelului, curent daca sunt pixeli de muchie slaba ii adaugam
				while (!Q.empty()) {
					CPoint r = Q.front();
					for (int k = -1; k <= 1; k++) {
						for (int m = -1; m <= 1; m++) {
							//E muchie slaba ?
							if (GS[(r.y + k)*w + (r.x + m)] == 128) {
								//Il facem punct de muchie puternica
								GS[(r.y + k)*w + (r.x + m)] = 255;
								
								//Il adaugam in coada
								CPoint p;
								p.x = r.x + m;
								p.y = r.y + k;
								Q.push(p);
							}
						}
					}
					//Scoatem p din coada
					Q.pop();
				}
			}
		}
	}
	for (int i = 1; i < dwHeight - 1; i++) {
		for (int j = 1; j < dwWidth - 1; j++) {
			if (GS[i*w + j] == 128)
				lpDst[i*w + j] = 0;
			else
				lpDst[i*w + j] = GS[i*w + j];
		}
	}
	
	END_PROCESSING("Prelungire Muchii");
}

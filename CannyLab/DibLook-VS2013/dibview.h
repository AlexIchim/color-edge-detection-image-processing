// dibview.h : interface of the CDibView class
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

class CDibView : public CScrollView
{
protected: // create from serialization only
	CDibView();
	DECLARE_DYNCREATE(CDibView)

// Attributes
public:
	CDibDoc* GetDocument()
		{
			ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDibDoc)));
			return (CDibDoc*) m_pDocument;
		}

// Operations
public:

// Implementation
public:
	virtual ~CDibView();
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view

	virtual void OnInitialUpdate();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

	// Printing support
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Generated message map functions
protected:
	//{{AFX_MSG(CDibView)
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM lParam);  // user message
	afx_msg void OnProcessingParcurgereSimpla();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnProcessingColor24();
	afx_msg void OnProcessingGrey();
	afx_msg void OnLab2Afisareinfo();
	afx_msg void OnLab2Grayscale();
	afx_msg void OnLab2Invgrayscale();
	afx_msg void OnLab2Blackwhite();
	afx_msg void OnLab3Afisarehistograma();
	afx_msg void OnLab3Reduceregri();
	afx_msg void OnLab3Floyd();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLab4Proiect();
	afx_msg void OnLab4Proiectiepeverticala();
	afx_msg void OnLab5Etichetare();
	afx_msg void OnLab6Algoritmurmarirecontur();
	afx_msg void OnLab6Reconstructiecontur();
	afx_msg void OnLab7Operatiimorfologice();
	afx_msg void OnLab8Mediasidev();
	afx_msg void OnLab8Binarizare32791();
	afx_msg void OnLab8Binarizare();
	afx_msg void OnLab8Modificareluminozitate();
	afx_msg void OnLab8Modificarecontrast();
	afx_msg void OnLab8Corectiegamma();
	afx_msg void OnLab8Egalizarehistograma();
	afx_msg void OnLab9Filtrumediearitmetica();
	afx_msg void OnLab9Filtrumediearitmetica5x5();
	afx_msg void OnLab9Filtrugaussian3x3();
	afx_msg void OnLab9Filtrulaplace3x3();
	afx_msg void OnLab9Filtrutrecesus3x3();
	afx_msg void OnLab9Logaritmmagnitudine();
	afx_msg void OnLab9Ftsideal();
	afx_msg void OnLab9Ftstaieregaussiana32805();
	afx_msg void OnLab9Ftsideal32803();
	afx_msg void OnLab9Ftstaieregaussiana();
	afx_msg void OnL10Filtrugaussian2d();
	afx_msg void OnL10Filtrugaussian2d32807();
	afx_msg void OnL10Filtrumedian();
	afx_msg void OnL11Filtraregaussiana();
	afx_msg void OnL11Modululorientareagradientilor();
	afx_msg void OnL11Binarizareaadaptiva();
	afx_msg void OnL11Prelungireamuchiilor();
};

/////////////////////////////////////////////////////////////////////////////

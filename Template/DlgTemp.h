#pragma once
#include "afxwin.h"
#include "DlgComposite.h"
#include <list>

using namespace std;

typedef struct _tagStPoint
{
	INT32 x;
	INT32 y;

	double X;
	double Y;
}StPoint;
/* Ax + By + C = 0 */
typedef struct _tagStLine
{
	double A;
	double B;
	double C;
}StLine;

#define POINT_IN		0x00000001
#define LINE_VALID		0x80000000

typedef struct _tagStPointEx
{
	INT32 m_s32Index;
	INT32 m_s32PointFlag;
	INT32 m_s32LineFlag;
	StPoint m_stPoint;
	StLine m_stLine;
}StPointEx;


typedef list<StPoint>  CListPoint;
typedef list<StPoint>::iterator  CListPointIter;

typedef list<StPointEx>  CListPointEx;
typedef list<StPointEx>::iterator  CListPointExIter;


class CPolygonCtrl
{
public:
	CPolygonCtrl()
	{
	}
	~CPolygonCtrl()
	{
	}

	CListPointEx m_csPoints;
	RECT m_stOrgRect;

	EMPointOnRectType GetPointStatus(INT32 x, INT32 y, INT32 *pV = NULL);
	INT32 InsertPoint(INT32 x, INT32 y, INT32 s32Index = ~0);
	INT32 DeletePoint(INT32 x, INT32 y, INT32 s32Index = ~0);
	INT32 MovePolygon(INT32 xOffset, INT32 yOffset);
	INT32 MovePoint(INT32 xOffset, INT32 yOffset, INT32 s32Index = ~0);
	INT32 ReBuildUnionPoint(RECT *pRect);
	INT32 ReloadRelativePoint(RECT *pRect);
	bool IsPolygonValid(void)
	{
		return m_csPoints.size() > 2;
	}

};


typedef struct _tagStPolygon
{
	CListPoint csListPoint;
}StPolygon;

typedef list<StPolygon>  CListPolygon;
typedef list<StPolygon>::iterator  CListPolygonIter;


// CDlgTemp 对话框

class CDlgTemp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTemp)

public:
	CDlgTemp(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTemp();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_Temp };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);


	void ReBuildCtrls(void);

public:
	CString m_csScanFloder;
	CString m_csLoadFile;

	UINT32	m_u32BmpWidth;
	UINT32	m_u32BmpHeight;
	UINT32	*m_pBmpBuf32Bit;

	EMMouseStatus m_emMouseStatus;
	INT32	m_s32OperatoinIndex;


public:
	CStatic m_csStaticPIC;
	CDC		*m_pBMPDC;
	CBitmap *m_pBMPForBMPDC;
	CBitmap *m_pOldBMP;
	CRect m_csDrawRectInPIC;
	CPoint m_csPrevDownPoint;
	CPoint m_csPrevDownMovePoint;


	CPolygonCtrl m_csPolygonCtrl;



public:
	INT32 ReloadBMP(void);
	INT32 ReBuildBMPDC(void);
	INT32 ReDrawStaticPic(void);

};



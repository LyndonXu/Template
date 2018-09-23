// DlgTemp.cpp : 实现文件
//

#include "stdafx.h"
#include "Template.h"
#include "DlgTemp.h"
#include "afxdialogex.h"

#include "utils.h"
#include "DlgComposite.h"

// CDlgTemp 对话框

IMPLEMENT_DYNAMIC(CDlgTemp, CDialogEx)

CDlgTemp::CDlgTemp(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_Temp, pParent)
	, m_csScanFloder(L"")
	, m_csLoadFile(L"")

	, m_u32BmpWidth(0)
	, m_u32BmpHeight(0)
	, m_pBmpBuf32Bit(0)
	, m_emMouseStatus(_Mouse_UP)
	, m_s32OperatoinIndex(0)


	, m_pBMPDC(NULL)
	, m_pBMPForBMPDC(NULL)
	, m_pOldBMP(NULL)

{

}

CDlgTemp::~CDlgTemp()
{

	if (m_pBmpBuf32Bit != NULL)
	{
		free(m_pBmpBuf32Bit);
		m_pBmpBuf32Bit = NULL;
	}

	if (m_pBMPDC != NULL)
	{
		m_pBMPDC->SelectObject(m_pOldBMP);
		delete m_pBMPDC;
		m_pBMPDC = NULL;
	}

	if (m_pBMPForBMPDC != NULL)
	{
		delete m_pBMPForBMPDC;
		m_pBMPForBMPDC = NULL;
	}

}

void CDlgTemp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PicTemp, m_csStaticPIC);
}


BEGIN_MESSAGE_MAP(CDlgTemp, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


// CDlgTemp 消息处理程序


BOOL CDlgTemp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	ReBuildCtrls();

	ReloadBMP();

	SetTimer(1, 40, NULL);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgTemp::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	KillTimer(1);
	return CDialogEx::DestroyWindow();
}


void CDlgTemp::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
}




void CDlgTemp::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	ReBuildCtrls();
}


void CDlgTemp::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x = 480;
	lpMMI->ptMinTrackSize.y = 320;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}
void CDlgTemp::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		ReDrawStaticPic();
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgTemp::ReBuildCtrls(void)
{

	{
		CWnd *pPICShow = GetDlgItem(IDC_STATIC_PicTemp);
		if (pPICShow != NULL && pPICShow->GetSafeHwnd() != NULL)
		{
			CRect csClient;
			GetWindowRect(&csClient);
			csClient.left += 8;
			csClient.top += 31;
			csClient.right -= 8;
			csClient.bottom -= 45;

			ScreenToClient(&csClient);

			pPICShow->MoveWindow(&csClient);
			pPICShow->Invalidate();
		}
	}

	{
#define ID_COUNT	2
		const UINT32 u32ID[ID_COUNT] = { IDOK, IDCANCEL };
		CRect csClient;
		GetWindowRect(&csClient);
		for (UINT32 i = 0; i < ID_COUNT; i++)
		{
			CWnd *pCtrl = GetDlgItem(u32ID[i]);

			if (pCtrl == NULL || pCtrl->GetSafeHwnd() == NULL)
			{
				break;
			}
			CRect csRect;
			pCtrl->GetWindowRect(csRect);

			csRect.MoveToXY(csClient.right - 200 + i * (csRect.Width() + 30), csClient.bottom - 35);

			ScreenToClient(&csRect);

			pCtrl->MoveWindow(csRect);
		}
#undef ID_COUNT
	}

}

INT32 CDlgTemp::ReloadBMP(void)
{
	CString csFile;
	if (m_csScanFloder.GetLength() != 0)
	{
		csFile = m_csScanFloder + L"\\";
	}
	csFile += m_csLoadFile;

	FILE *pFile = NULL;

	int ret = _wfopen_s(&pFile, csFile.GetString(), L"rb");
	if (ret != 0)
	{
		return -1;
	}
	StRGB32Bit stRGB = { (int32_t)m_u32BmpWidth, (int32_t)m_u32BmpHeight, m_pBmpBuf32Bit };
	int32_t s32Ret = LoadBmp(pFile, &stRGB);

	fclose(pFile);
	if (s32Ret != 0)
	{	
		if (stRGB.pRGB == NULL)
		{
			m_u32BmpWidth = 0;
			m_u32BmpHeight = 0;
			m_pBmpBuf32Bit = stRGB.pRGB;
		}

		return -1;
	}

	m_u32BmpWidth = stRGB.s32Width;
	m_u32BmpHeight = stRGB.s32Height;
	m_pBmpBuf32Bit = stRGB.pRGB;
	return ReBuildBMPDC();
}


INT32 CDlgTemp::ReBuildBMPDC(void)
{
	if (m_pBmpBuf32Bit == NULL)
	{
		return -1;
	}

	if (m_pBMPDC != NULL && m_pBMPForBMPDC != NULL)
	{
		m_pBMPDC->SelectObject(m_pOldBMP);
		m_pOldBMP = NULL;
		delete m_pBMPDC;
		m_pBMPDC = NULL;

		delete m_pBMPForBMPDC;
		m_pBMPForBMPDC = NULL;
	}

	if (m_pBMPDC == NULL)
	{
		m_pBMPDC = new CDC;
	}

	if (m_pBMPForBMPDC == NULL)
	{
		m_pBMPForBMPDC = new CBitmap;
	}

	if (m_pBMPDC != NULL && m_pBMPForBMPDC != NULL)
	{
		m_pBMPDC->CreateCompatibleDC(NULL);

		m_pBMPForBMPDC->CreateBitmap(m_u32BmpWidth,
			m_u32BmpHeight, 1, 32,
			NULL);
		m_pOldBMP = m_pBMPDC->SelectObject(m_pBMPForBMPDC);

		BITMAPINFO bmpInfo;
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = m_u32BmpWidth;
		bmpInfo.bmiHeader.biHeight = m_u32BmpHeight;
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = 32;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biSizeImage = 0;
		bmpInfo.bmiHeader.biXPelsPerMeter = 3000;
		bmpInfo.bmiHeader.biYPelsPerMeter = 3000;
		bmpInfo.bmiHeader.biClrUsed = 0;
		bmpInfo.bmiHeader.biClrImportant = 0;

		::SetDIBits(m_pBMPDC->GetSafeHdc(),
			(HBITMAP)(m_pBMPForBMPDC->GetSafeHandle()),
			0, m_u32BmpHeight,
			m_pBmpBuf32Bit,
			&bmpInfo, DIB_RGB_COLORS);

		return ReDrawStaticPic();
	}
	else if (m_pBMPDC != NULL)
	{
		delete m_pBMPDC;
		m_pBMPDC = NULL;
	}
	else
	{
		delete m_pBMPForBMPDC;
		m_pBMPForBMPDC = NULL;
	}
	return -1;
}


INT32 CDlgTemp::ReDrawStaticPic(void)
{
	if (m_pBmpBuf32Bit == NULL || m_pBMPDC == NULL)
	{
		m_csStaticPIC.Invalidate();
		return 0;
	}
	do
	{
		CDC *pDC = m_csStaticPIC.GetDC();
		if (pDC == NULL)
		{
			break;
		}
		CRect csRect;
		pDC->GetWindow()->GetWindowRect(&csRect);

		CDC csMemDC;
		csMemDC.CreateCompatibleDC(pDC);
		CBitmap csBitMap;
		csBitMap.CreateCompatibleBitmap(pDC, csRect.Width(), csRect.Height());

		CBitmap *pOldBMP = (CBitmap *)csMemDC.SelectObject(&csBitMap);

		double d64ImageRadio = (double)m_u32BmpWidth / m_u32BmpHeight;
		double d64DrawRadio = (double)csRect.Width() / csRect.Height();

		CRect csDest = { 0, 0, csRect.Width(), csRect.Height() };
		if (d64ImageRadio > d64DrawRadio)
		{
			INT32 y = (INT32)(csRect.Width() / d64ImageRadio);
			y = csDest.Height() - y;
			csDest.top = y / 2;
			csDest.bottom = csDest.bottom - y / 2;
		}
		else
		{
			INT x = (INT32)(csRect.Height() * d64ImageRadio);
			x = csDest.Width() - x;
			csDest.left = x / 2;
			csDest.right = csDest.right - x / 2;
		}

		m_csDrawRectInPIC = csDest;

		TransparentBlt(csMemDC.GetSafeHdc(),
			csDest.left, csDest.top, csDest.Width(), csDest.Height(),
			m_pBMPDC->GetSafeHdc(),
			0, 0, m_u32BmpWidth, m_u32BmpHeight,
			0);

		if (1)
		do 
		{
			if (m_csPolygonCtrl.m_csPoints.size() != 0)
			{
				if (m_emMouseStatus == _Mouse_UP)
				{
					m_csPolygonCtrl.ReloadRelativePoint(m_csDrawRectInPIC);
				}

				POINT *pPoint = (POINT *)malloc(sizeof(POINT) *m_csPolygonCtrl.m_csPoints.size());
				if (pPoint == NULL)
				{
					break;
				}
				AutoFree csFreePoint(pPoint);
				CListPointExIter iter = m_csPolygonCtrl.m_csPoints.begin();
				UINT32 i = 0;
				for (; iter != m_csPolygonCtrl.m_csPoints.end(); iter++, i++)
				{
					pPoint[i].x = iter->m_stPoint.x;
					pPoint[i].y = iter->m_stPoint.y;
				}

				CPen csPen(PS_SOLID, 1, RGB(255, 0, 0));
				CPen *pOldPen = (CPen *)csMemDC.SelectObject(&csPen);
				csMemDC.SelectStockObject(NULL_BRUSH);
				csMemDC.SetBkMode(TRANSPARENT);

				csMemDC.Polygon(pPoint, m_csPolygonCtrl.m_csPoints.size());

				CBrush brush(RGB(255, 0, 0));
				CBrush *pOldBrush = csMemDC.SelectObject(&brush);
				for (UINT i = 0; i < m_csPolygonCtrl.m_csPoints.size(); i++)
				{
					csMemDC.Ellipse(pPoint[i].x - 2, pPoint[i].y - 2,
						pPoint[i].x + 2, pPoint[i].y + 2);
				}

				csMemDC.SelectObject(pOldBrush);
				csMemDC.SelectObject(pOldPen);
			}

		} while (0);


		pDC->BitBlt(0, 0, csRect.Width(), csRect.Height(), &csMemDC,
			0, 0, SRCCOPY);

		csMemDC.SelectObject(pOldBMP);

		ReleaseDC(pDC);
	} while (0);

	return 0;
}


int32_t DisPointToLine(double x, double y, StLine *pLine, double *pDis)
{
	if (pDis == NULL || pDis == NULL)
	{
		return -1;
	}

	double numerator = abs(pLine->A * x + pLine->B * y + pLine->C);
	double denominator = sqrt(pLine->A * pLine->A + pLine->B * pLine->B);
	
	*pDis = numerator / denominator;

	return 0;
}

int32_t DisPointToPoint(double x1, double y1, double x2, double y2, double *pDis)
{
	if (pDis == NULL || pDis == NULL)
	{
		return -1;
	}

	double dis = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	*pDis = dis;
	return 0;
}


int32_t GetLine(double x1, double y1, double x2, double y2, StLine *pLine)
{
	if (pLine == NULL)
	{
		return -1;
	}
	pLine->A = y2 - y1;
	pLine->B = 0.0 - (x2 - x1);
	pLine->C = (y1 * x2 - y2 * x1);
	
	return 0;
}

bool PtInPolygon(POINT p, LPPOINT ptPolygon, INT32 nCount) 
{
	INT32 nCross = 0; 
	for (INT32 i = 0; i < nCount; i++)
	{
		POINT p1 = ptPolygon[i]; 
		POINT p2 = ptPolygon[(i + 1) % nCount]; // 求解 y=p.y 与 p1p2 的交点 
		if ( p1.y == p2.y ) // p1p2 与 y=p0.y平行 
			continue; 
		if ( p.y < min(p1.y, p2.y) ) // 交点在p1p2延长线上 
			continue; 
		if ( p.y >= max(p1.y, p2.y) ) // 交点在p1p2延长线上 
			continue; // 求交点的 X 坐标
		double x = (double)(p.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x; 
		if ( x > p.x ) 
			nCross++; // 只统计单边交点 
	} // 单边交点为偶数，点在多边形之外 --- 
		
	return (nCross % 2 == 1); 
} 

bool PointInPolygon(CListPointEx *pPoints, INT32 x, INT32 y)
{
	if (pPoints->size() < 3)
	{
		return false;
	}
	POINT *pPoint = (POINT *)malloc(sizeof(POINT) *  pPoints->size());
	if (pPoint == NULL)
	{
		return false;
	}
	AutoFree csFreePoint(pPoint);
	CListPointExIter iter = pPoints->begin();
	INT32 i = 0;
	for (; iter != pPoints->end(); iter++, i++)
	{
		pPoint[i].x = iter->m_stPoint.x;
		pPoint[i].y = iter->m_stPoint.y;
	}

	POINT p = { x, y };
	return PtInPolygon(p, pPoint, i);
}

EMPointOnRectType  CPolygonCtrl:: GetPointStatus(INT32 x, INT32 y, INT32 *pV/* = NULL*/)
{
	if (m_csPoints.size() < 3)
	{
		return _Point_On_Rect_NotIn;
	}

	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		double dis = 100.0;
		DisPointToPoint(x, y, iter->m_stPoint.x, iter->m_stPoint.y, &dis);
		if (dis < 3.0)
		{
			if (pV != NULL)
			{
				*pV = iter->m_s32Index;
			}
			return _Point_On_Rect_POINT_HAND;
		}
	}

	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		double dis = 100.0;
		DisPointToLine(x, y, &(iter->m_stLine), &dis);
		if (dis < 3.0)
		{
			CListPointExIter iterCur = iter;
			CListPointExIter iterNext = iter;
			iterNext++;
			if (iterNext == m_csPoints.end())
			{
				iterNext = m_csPoints.begin();
			}
			INT32 mx = iterCur->m_stPoint.x + iterNext->m_stPoint.x;
			INT32 my = iterCur->m_stPoint.y + iterNext->m_stPoint.y;

			mx /= 2;
			my /= 2;

			double R = 0.0;
			double mAndpDis = 0.0f;

			DisPointToPoint(mx, my, iterCur->m_stPoint.x, iterCur->m_stPoint.y, &R);
			DisPointToPoint(mx, my, x, y, &mAndpDis);

			if (mAndpDis < R)
			{
				if (pV != NULL)
				{
					*pV = iter->m_s32Index;
				}
				return _Point_On_Rect_LINE_HAND;
			}

		}
	}

	if (PointInPolygon(&m_csPoints, x, y))
	{
		return _Point_On_Rect_SIZEALL;
	}

	return _Point_On_Rect_NotIn;
}
INT32 CPolygonCtrl::InsertPoint(INT32 x, INT32 y, INT32 s32Index/* = ~0*/)
{
	if (s32Index >= 0)
	{
		if (m_csPoints.size() > 2 && (UINT32)s32Index <= m_csPoints.size() - 1)
		{
			CListPointExIter iter;
			for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
			{
				if (iter->m_s32Index == s32Index)
				{
					break;
				}
			}
			if (iter != m_csPoints.end())
			{
				iter++;
				if (iter != m_csPoints.end())
				{
					m_csPoints.insert(iter, StPointEx{ iter->m_s32Index, 0, 0,{ x, y } });
				}
				else		/* insert to end */
				{
					m_csPoints.push_back(StPointEx{(INT32)m_csPoints.size(), 0, 0,{ x, y } });
				}
			}
			else
			{
				return -1;
			}

			CListPointExIter iterPrev;
			INT32 i = 0;
			for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++, i++)
			{
				iter->m_s32Index = i;
				if (i == s32Index)
				{
					iterPrev = iter;
				}
			}
			CListPointExIter iterCur = iterPrev;
			iterCur++;

			CListPointExIter iterNext = iterCur;
			iterNext++;
			if (iterNext == m_csPoints.end())
			{
				iterNext = m_csPoints.begin();
			}

			StLine stLine = { 0.0 };

			GetLine(iterPrev->m_stPoint.x, iterPrev->m_stPoint.y,
				iterCur->m_stPoint.x, iterCur->m_stPoint.y, &stLine);

			iterPrev->m_s32LineFlag = LINE_VALID;
			iterPrev->m_stLine = stLine;


			GetLine(iterCur->m_stPoint.x, iterCur->m_stPoint.y,
				iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);

			iterCur->m_s32LineFlag = LINE_VALID;
			iterCur->m_stLine = stLine;

			return 0;
		}
		else
		{
			return -1;
		}

		return -1;
	}

	if (m_csPoints.size() > 2)
	{
		CListPointExIter iter;
		for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
		{
			double dis = 100.0;
			DisPointToLine(x, y, &(iter->m_stLine), &dis);
			if (dis < 3.0)
			{
				CListPointExIter iterCur = iter;
				CListPointExIter iterNext = iter;
				iterNext++;
				if (iterNext == m_csPoints.end())
				{
					iterNext = m_csPoints.begin();
				}

				INT32 mx = iterCur->m_stPoint.x + iterNext->m_stPoint.x;
				INT32 my = iterCur->m_stPoint.y + iterNext->m_stPoint.y;

				mx /= 2;
				my /= 2;

				double R = 0.0;
				double mAndpDis = 0.0f;

				DisPointToPoint(mx, my, iterCur->m_stPoint.x, iterCur->m_stPoint.y, &R);
				DisPointToPoint(mx, my, x, y, &mAndpDis);

				if (mAndpDis < R)
				{
					INT32 ret = InsertPoint(x, y, iter->m_s32Index);
					if (ret == 0)
					{
						return 0;
					}
				}

				/* else push_back */
			}
		}
	}

	if (m_csPoints.size() != 0)
	{
		if (m_csPoints.size() == 1)
		{
			m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0, { x, y } });
			CListPointExIter iter = m_csPoints.begin();
			CListPointExIter next = iter;
			next++;

			StLine stLine = { 0.0 };

			GetLine(iter->m_stPoint.x, iter->m_stPoint.y,
				next->m_stPoint.x, next->m_stPoint.y, &stLine);

			iter->m_s32LineFlag = next->m_s32LineFlag = LINE_VALID;
			iter->m_stLine = next->m_stLine = stLine;
		}
		else
		{
			CListPointExIter iterNext = m_csPoints.begin();

			m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0,{ x, y } });

			CListPointExIter iterCur = m_csPoints.end();
			iterCur--;

			CListPointExIter iterPrev = iterCur;
			iterPrev--;

			StLine stLine = { 0.0 };

			GetLine(iterPrev->m_stPoint.x, iterPrev->m_stPoint.y,
				iterCur->m_stPoint.x, iterCur->m_stPoint.y, &stLine);

			iterPrev->m_s32LineFlag = LINE_VALID;
			iterPrev->m_stLine = stLine;


			GetLine(iterCur->m_stPoint.x, iterCur->m_stPoint.y,
				iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);

			iterCur->m_s32LineFlag = LINE_VALID;
			iterCur->m_stLine = stLine;

		}

	}
	else
	{
		m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0, { x, y } });
	}

	return 0;
}
INT32 CPolygonCtrl::DeletePoint(INT32 x, INT32 y, INT32 s32Index/* = ~0*/)
{
	if (m_csPoints.size() == 0)
	{
		return -1;
	}
	if (s32Index < 0)
	{
		return -1;
	}

	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		if (iter->m_s32Index == s32Index)
		{
			break;
		}
	}
	if (iter == m_csPoints.end())
	{
		return -1;
	}

	CListPointExIter iterCur = iter, iterPrev, iterNext;
	if (iter == m_csPoints.begin())
	{
		iterPrev = m_csPoints.end();
	}
	else
	{
		iterPrev = iter;
	}
	iterPrev--;

	iterNext = iter;
	iterNext++;
	if (iterNext == m_csPoints.end())
	{
		iterNext = m_csPoints.begin();
	}
	StLine stLine = { 0 };
	GetLine(iterPrev->m_stPoint.x, iterPrev->m_stPoint.y,
		iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);

	iterPrev->m_s32LineFlag = LINE_VALID;
	iterPrev->m_stLine = stLine;

	m_csPoints.erase(iterCur);

	s32Index = 0;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++, s32Index++)
	{
		if (iter->m_s32Index = s32Index)
		{
			break;
		}
	}

	return 0;

}
INT32 CPolygonCtrl::MovePolygon(INT32 xOffset, INT32 yOffset)
{
	if (m_csPoints.size() == 0)
	{
		return -1;
	}
	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		iter->m_stPoint.x += xOffset;
		iter->m_stPoint.y += yOffset;
	}

	CListPointExIter iterTmp = m_csPoints.end();
	iterTmp--;
	for (iter = m_csPoints.begin(); iter != iterTmp; iter++)
	{
		CListPointExIter iterNext = iter;
		iterNext++;

		StLine stLine = { 0.0 };

		GetLine(iter->m_stPoint.x, iter->m_stPoint.y,
			iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);
		iter->m_stLine = stLine;
	}

	{
		CListPointExIter iterNext = m_csPoints.begin();
		StLine stLine = { 0.0 };

		GetLine(iter->m_stPoint.x, iter->m_stPoint.y,
			iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);
		iter->m_stLine = stLine;
	}

	return 0;
}

INT32 CPolygonCtrl::MovePoint(INT32 xOffset, INT32 yOffset, INT32 s32Index/* = ~0*/)
{
	if (m_csPoints.size() == 0)
	{
		return -1;
	}
	if (s32Index < 0)
	{
		return -1;
	}

	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		if (iter->m_s32Index == s32Index)
		{
			break;
		}
	}
	if (iter == m_csPoints.end())
	{
		return -1;
	}

	iter->m_stPoint.x += xOffset;
	iter->m_stPoint.y += yOffset;

	CListPointExIter iterCur = iter, iterPrev, iterNext;
	if (iter == m_csPoints.begin())
	{
		iterPrev = m_csPoints.end();
	}
	else
	{
		iterPrev = iter;
	}
	iterPrev--;

	iterNext = iter;
	iterNext++;
	if (iterNext == m_csPoints.end())
	{
		iterNext = m_csPoints.begin();
	}
	StLine stLine = { 0 };
	GetLine(iterPrev->m_stPoint.x, iterPrev->m_stPoint.y,
		iterCur->m_stPoint.x, iterCur->m_stPoint.y, &stLine);

	iterPrev->m_s32LineFlag = LINE_VALID;
	iterPrev->m_stLine = stLine;


	GetLine(iterCur->m_stPoint.x, iterCur->m_stPoint.y,
		iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);

	iterCur->m_s32LineFlag = LINE_VALID;
	iterCur->m_stLine = stLine;

	return 0;

}

INT32 CPolygonCtrl::ReBuildUnionPoint(RECT *pRect)
{
	if (pRect == NULL)
	{
		return -1;
	}

	INT32 s32Width = pRect->right - pRect->left;
	INT32 s32Height = pRect->bottom - pRect->top;

	if (s32Width < 0)
	{
		s32Width = 0 - s32Width;
	}

	if (s32Height < 0)
	{
		s32Height = 0 - s32Height;
	}

	m_stOrgRect = *pRect;

	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		iter->m_stPoint.X = (double)(iter->m_stPoint.x - pRect->left) / s32Width;
		iter->m_stPoint.Y = (double)(iter->m_stPoint.y - pRect->top) / s32Height;
	}


	return 0;
}
INT32 CPolygonCtrl::ReloadRelativePoint(RECT *pRect)
{
	if (pRect == NULL)
	{
		return -1;
	}

	if (memcmp(pRect, &m_stOrgRect, sizeof(RECT)) == 0)
	{
		return 0;
	}

	INT32 s32Width = pRect->right - pRect->left;
	INT32 s32Height = pRect->bottom - pRect->top;

	if (s32Width < 0)
	{
		s32Width = 0 - s32Width;
	}

	if (s32Height < 0)
	{
		s32Height = 0 - s32Height;
	}

	m_stOrgRect = *pRect;

	CListPointExIter iter;
	for (iter = m_csPoints.begin(); iter != m_csPoints.end(); iter++)
	{
		iter->m_stPoint.x = (INT32)(iter->m_stPoint.X * s32Width + pRect->left);
		iter->m_stPoint.y = (INT32)(iter->m_stPoint.Y * s32Height + pRect->top);
	}

	return 0;
}




void CDlgTemp::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//TRACE(L"POINT is %sin\n", PointInPolygon(&m_csPoint, point.x, point.y) ? L"" : L"not");

	//m_csPolygonCtrl.InsertPoint(point.x, point.y);

	CRect csPicRect, csRectWindows;
	m_csStaticPIC.GetWindowRect(csPicRect);

	csRectWindows = m_csDrawRectInPIC;
	csRectWindows.MoveToXY(csPicRect.left + m_csDrawRectInPIC.left,
		csPicRect.top + m_csDrawRectInPIC.top);

	if (PtInRect(&m_csDrawRectInPIC, point))
	{
		m_csPrevDownMovePoint = m_csPrevDownPoint = point;

		EMPointOnRectType emType = _Point_On_Rect_NotIn;
		
		emType = m_csPolygonCtrl.GetPointStatus(point.x, point.y, &m_s32OperatoinIndex);

		if (emType == _Point_On_Rect_NotIn)
		{
			m_csPolygonCtrl.InsertPoint(point.x, point.y);
		}
		else if (emType == _Point_On_Rect_LINE_HAND)
		{
			m_csPolygonCtrl.InsertPoint(point.x, point.y, m_s32OperatoinIndex);
		}
		else if (emType == _Point_On_Rect_POINT_HAND)
		{
			m_s32OperatoinIndex = m_s32OperatoinIndex;
		}

		m_emMouseStatus = c_emMouseStatus[emType];

		ClipCursor(&csRectWindows);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CDlgTemp::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	if (m_emMouseStatus != _Mouse_UP)
	{
		m_csPolygonCtrl.ReBuildUnionPoint(m_csDrawRectInPIC);
	}
	m_emMouseStatus = _Mouse_UP;

	ClipCursor(NULL);

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CDlgTemp::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_emMouseStatus != _Mouse_UP)
	{
		INT32 x = point.x;
		INT32 y = point.y;
		switch (m_emMouseStatus)
		{
		case _Mouse_PIC_New:
		{
			break;
		}
		case _Mouse_PIC_RectLeft:
		{
			break;
		}
		case _Mouse_PIC_RectRight:
		{
			break;
		}
		case _Mouse_PIC_RectUp:
		{
			break;
		}
		case _Mouse_PIC_RectDown:
		{
			break;
		}
		case _Mouse_PIC_RectLeftUp:
		{
			break;
		}
		case _Mouse_PIC_RectRightUp:
		{
			break;
		}
		case _Mouse_PIC_RectLeftDown:
		{
			break;
		}
		case _Mouse_PIC_RectRightDown:
		{
			break;
		}
		case _Mouse_PIC_RectMove:
		{
			m_csPolygonCtrl.MovePolygon(
				x - m_csPrevDownMovePoint.x,
				y - m_csPrevDownMovePoint.y);
			m_csPrevDownMovePoint = point;
			break;
		}
		case _Mouse_PIC_RectPoint:
		{
			m_csPolygonCtrl.MovePoint(
				x - m_csPrevDownMovePoint.x,
				y - m_csPrevDownMovePoint.y, m_s32OperatoinIndex);
			m_csPrevDownMovePoint = point;
			break;
		}

		default:
			break;
		}

	}
	else
	{
		EMPointOnRectType emType = _Point_On_Rect_NotIn;
		emType = m_csPolygonCtrl.GetPointStatus(point.x, point.y);

		SetCursor(LoadCursor(NULL, c_pCursorType[emType]));
		if (emType != _Point_On_Rect_NotIn)
		{
			SetCapture();
		}
		else
		{
			ReleaseCapture();
		}

	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CDlgTemp::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (PtInRect(&m_csDrawRectInPIC, point))
	{
		EMPointOnRectType emType = _Point_On_Rect_NotIn;

		emType = m_csPolygonCtrl.GetPointStatus(point.x, point.y, &m_s32OperatoinIndex);

		if (emType == _Point_On_Rect_NotIn)
		{
		}
		else if (emType == _Point_On_Rect_LINE_HAND)
		{

		}
		else if (emType == _Point_On_Rect_POINT_HAND)
		{
			m_csPolygonCtrl.DeletePoint(point.x, point.y, m_s32OperatoinIndex);
		}
	}

	CDialogEx::OnRButtonUp(nFlags, point);
}

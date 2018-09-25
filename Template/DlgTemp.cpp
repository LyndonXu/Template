// DlgTemp.cpp : 实现文件
//

#include "stdafx.h"
#include "Template.h"
#include "DlgTemp.h"
#include "afxdialogex.h"

#include "utils.h"
#include "DlgComposite.h"


#define RELOAD_BMP_MSG	(WM_USER + 200)

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

	, m_s32SetUpMode(0)

	, m_pBMPDC(NULL)
	, m_pBMPForBMPDC(NULL)
	, m_pOldBMP(NULL)

	, m_pPolygonCtrl(NULL)

	, m_pCreateWnd(NULL)

{

}

CDlgTemp::~CDlgTemp()
{
	if (m_s32SetUpMode != 0)
	{
		DestroyWindow();
	}

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
	ON_MESSAGE(RELOAD_BMP_MSG, &ReloadBmpMessageCtrl)

	ON_BN_CLICKED(IDC_BTN_Save, &CDlgTemp::OnBnClickedBtnSave)
END_MESSAGE_MAP()


// CDlgTemp 消息处理程序


BOOL CDlgTemp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	INT32 s32Width = GetSystemMetrics(SM_CXSCREEN);
	//int s32Height = GetSystemMetrics(SM_CYSCREEN);
	INT32 s32Left = (s32Width - 480) / 2;
	INT32 s32Top = 50;
	CRect csClient(s32Left, s32Top, s32Left + 480, s32Top + 320);

	::SetWindowPos(GetSafeHwnd(), NULL,//HWND_TOPMOST, 
		csClient.left, csClient.top,
		csClient.Width(), csClient.Height(),
		SWP_SHOWWINDOW);

	ReBuildCtrls();

	if (m_s32SetUpMode == 0)
	{
		ReloadBMP();
	}
	else
	{
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL)
		{
			pSysMenu->EnableMenuItem(SC_CLOSE, MF_DISABLED);
		}

		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

	}

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

	{
		CRect csClient;
		GetWindowRect(&csClient);
		CWnd *pCtrl = GetDlgItem(IDC_BTN_Save);

		if (pCtrl != NULL && pCtrl->GetSafeHwnd() != NULL)
		{
			CRect csRect;
			pCtrl->GetWindowRect(csRect);

			csRect.MoveToXY(csClient.left + 20, csClient.bottom - 35);

			ScreenToClient(&csRect);

			pCtrl->MoveWindow(csRect);

		}
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

#if 0
	{
		for (UINT i = 0; i < stRGB.s32Height; i++)
		{
			g_u32Row[i] = *(stRGB.pRGB + i * stRGB.s32Width);
		}
		g_u32Row[0] = g_u32Row[0];
	}

#endif

	return ReBuildBMPDC();
}

INT32 CDlgTemp::ReloadBMP(StRGB32Bit *pRGB)
{
	if (m_s32SetUpMode != 1)
	{
		return -1;
	}
	if (pRGB == NULL)
	{
		return -1;
	}

	if (pRGB->s32Height <= 0 || pRGB->s32Width <= 0 || pRGB->pRGB == NULL)
	{
		return -1;
	}
	char *pTmp = (char *)malloc(pRGB->s32Width * pRGB->s32Height * 4 + 32 + sizeof(StRGB32Bit) + 32);
	if (pTmp == NULL)
	{
		return  -1;
	}
	StRGB32Bit *pRGBNNew = (StRGB32Bit *)(pTmp + pRGB->s32Width * pRGB->s32Height * 4);
	*pRGBNNew = *pRGB;
	pRGBNNew->pRGB = (uint32_t *)(pTmp);

	memcpy(pRGBNNew->pRGB, pRGB->pRGB, pRGB->s32Width * pRGB->s32Height * 4);

	if (PostMessage(RELOAD_BMP_MSG, 0, (LPARAM)pRGBNNew))
	{
		return 0;
	}
	else
	{
		free(pTmp);
		return -1;
	}

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
		bmpInfo.bmiHeader.biHeight = 0 - m_u32BmpHeight;
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

		CListPolygonCtrlIter iterCtrl;
		for (iterCtrl = m_csListPolygonCtrl.begin(); 
			iterCtrl != m_csListPolygonCtrl.end(); iterCtrl++)
		{
			if (iterCtrl->m_csPoints.size() != 0)
			{
				if (m_emMouseStatus == _Mouse_UP)
				{
					iterCtrl->ReloadRelativePoint(m_csDrawRectInPIC);
				}

				POINT *pPoint = (POINT *)malloc(sizeof(POINT) * iterCtrl->m_csPoints.size());
				if (pPoint == NULL)
				{
					break;
				}
				AutoFree csFreePoint(pPoint);
				CListPointExIter iter = iterCtrl->m_csPoints.begin();
				UINT32 i = 0;
				for (; iter != iterCtrl->m_csPoints.end(); iter++, i++)
				{
					pPoint[i].x = iter->m_stPoint.x;
					pPoint[i].y = iter->m_stPoint.y;
				}

				CPen csPen(PS_SOLID, 1, RGB(255, 0, 0));
				CPen *pOldPen = (CPen *)csMemDC.SelectObject(&csPen);
				csMemDC.SelectStockObject(NULL_BRUSH);
				csMemDC.SetBkMode(TRANSPARENT);

				csMemDC.Polygon(pPoint, iterCtrl->m_csPoints.size());

				CBrush brush(RGB(255, 0, 0));
				CBrush *pOldBrush = csMemDC.SelectObject(&brush);
				for (UINT i = 0; i < iterCtrl->m_csPoints.size(); i++)
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

		CListPolygonCtrlIter iter = m_csListPolygonCtrl.begin();
		for (; iter != m_csListPolygonCtrl.end(); iter++)
		{
			emType = iter->GetPointStatus(point.x, point.y, &m_s32OperatoinIndex);
			if (emType != _Point_On_Rect_NotIn)
			{
				break;
			}
		}

		if (emType == _Point_On_Rect_NotIn)
		{
			if (m_pPolygonCtrl == NULL)
			{
				//if (m_csListPolygonCtrl.size() == 0)
				{
					CPolygonCtrl csCtrl;
					m_csListPolygonCtrl.push_back(csCtrl);
				}
				iter = m_csListPolygonCtrl.end();
				iter--;

				m_pPolygonCtrl = &(*iter);
			}
			m_pPolygonCtrl->InsertPoint(point.x, point.y);
		}
		else
		{
			m_pPolygonCtrl = &(*iter);
			if (emType == _Point_On_Rect_SIZEALL)
			{
			}
			else if (emType == _Point_On_Rect_LINE_HAND)
			{
				m_pPolygonCtrl->InsertPoint(point.x, point.y, m_s32OperatoinIndex);
			}
			else if (emType == _Point_On_Rect_POINT_HAND)
			{
				m_s32OperatoinIndex = m_s32OperatoinIndex;
			}
			else
			{

			}
		}
		
		m_emMouseStatus = c_emMouseStatus[emType];

		ClipCursor(&csRectWindows);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CDlgTemp::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	if (m_emMouseStatus != _Mouse_UP &&
		m_pPolygonCtrl != NULL)
	{
		m_pPolygonCtrl->ReBuildUnionPoint(m_csDrawRectInPIC);
		//m_pPolygonCtrl = NULL;
	}
	m_emMouseStatus = _Mouse_UP;

	ClipCursor(NULL);

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CDlgTemp::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_emMouseStatus != _Mouse_UP &&
		m_pPolygonCtrl != NULL)
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
			m_pPolygonCtrl->MovePolygon(
				x - m_csPrevDownMovePoint.x,
				y - m_csPrevDownMovePoint.y);
			m_csPrevDownMovePoint = point;
			break;
		}
		case _Mouse_PIC_RectPoint:
		{
			m_pPolygonCtrl->MovePoint(
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
		CListPolygonCtrlIter iter = m_csListPolygonCtrl.begin();
		for (; iter != m_csListPolygonCtrl.end(); iter++)
		{
			emType = iter->GetPointStatus(point.x, point.y, &m_s32OperatoinIndex);
			if (emType != _Point_On_Rect_NotIn)
			{
				break;
			}
		}

		if (emType == _Point_On_Rect_LINE_HAND)
		{
			SetCursor(LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_CURSOR_Add)));
		}
		else
		{
			SetCursor(LoadCursor(NULL, c_pCursorType[emType]));
		}
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

		CListPolygonCtrlIter iter = m_csListPolygonCtrl.begin();
		for (; iter != m_csListPolygonCtrl.end(); iter++)
		{
			emType = iter->GetPointStatus(point.x, point.y, &m_s32OperatoinIndex);
			if (emType != _Point_On_Rect_NotIn)
			{
				break;
			}
		}


		if (emType == _Point_On_Rect_NotIn)
		{
			m_pPolygonCtrl = NULL;
		}
		else if (emType == _Point_On_Rect_SIZEALL)
		{
			m_csListPolygonCtrl.erase(iter);
			m_pPolygonCtrl = NULL;
		}
		else if (emType == _Point_On_Rect_LINE_HAND)
		{

		}
		else if (emType == _Point_On_Rect_POINT_HAND)
		{
			iter->DeletePoint(point.x, point.y, m_s32OperatoinIndex);
		}
	}

	CDialogEx::OnRButtonUp(nFlags, point);
}

void CDlgTemp::OnBnClickedBtnSave()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_s32SetUpMode != 0) /* savefile */
	{
		if (m_csSaveFile == L"")
		{
			return;
		}

		CImage csImage;

		csImage.Attach((HBITMAP)(m_pBMPForBMPDC->GetSafeHandle()));
		csImage.Save(m_csSaveFile);
	}

	/* save config */

}

LRESULT CDlgTemp::ReloadBmpMessageCtrl(WPARAM wMsg, LPARAM lData)
{
	switch (wMsg)
	{
		case 0:
		{
			StRGB32Bit *pRGB = (StRGB32Bit *)lData;
			if (pRGB != NULL)
			{
				if (m_pBmpBuf32Bit != NULL)
				{
					free(m_pBmpBuf32Bit);
					m_pBmpBuf32Bit = NULL;
				}

				m_u32BmpWidth = pRGB->s32Width;
				m_u32BmpHeight = pRGB->s32Height;
				m_pBmpBuf32Bit = pRGB->pRGB;
				return ReBuildBMPDC();

			}
		}
		default:
			break;
	}
	return 0;
}




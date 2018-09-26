#include "stdafx.h"
#include "utils.h"
#include "PolygonCtrl.h"



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
		if (p1.y == p2.y) // p1p2 与 y=p0.y平行 
			continue;
		if (p.y < min(p1.y, p2.y)) // 交点在p1p2延长线上 
			continue;
		if (p.y >= max(p1.y, p2.y)) // 交点在p1p2延长线上 
			continue; // 求交点的 X 坐标
		double x = (double)(p.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
		if (x > p.x)
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

EMPointOnRectType  CPolygonCtrl::GetPointStatus(INT32 x, INT32 y, INT32 *pV/* = NULL*/)
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
					m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0,{ x, y } });
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
			m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0,{ x, y } });
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
		m_csPoints.push_back(StPointEx{ (INT32)m_csPoints.size(), 0, 0,{ x, y } });
	}

	return 0;
}
INT32 CPolygonCtrl::DeletePoint(INT32 x, INT32 y, INT32 s32Index/* = ~0*/)
{
	if (m_csPoints.size() <= 3)
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
INT32 CPolygonCtrl::ReBuildIndexAndLine(bool boUseUnionPoint/* = false*/)
{
	if (m_csPoints.size() == 0)
	{
		return -1;
	}
	CListPointExIter iter;

	INT32 s32Index = 0;

	CListPointExIter iterTmp = m_csPoints.end();
	iterTmp--;
	for (iter = m_csPoints.begin(); iter != iterTmp; iter++, s32Index++)
	{
		CListPointExIter iterNext = iter;
		iterNext++;

		StLine stLine = { 0.0 };

		iter->m_s32Index = s32Index;
		iter->m_s32LineFlag = LINE_VALID;
		if (!boUseUnionPoint)
		{
			GetLine(iter->m_stPoint.x, iter->m_stPoint.y,
				iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);
		}
		else
		{
			GetLine(iter->m_stPoint.X, iter->m_stPoint.Y,
				iterNext->m_stPoint.X, iterNext->m_stPoint.Y, &stLine);
		}
		iter->m_stLine = stLine;
	}

	{
		CListPointExIter iterNext = m_csPoints.begin();
		StLine stLine = { 0.0 };

		iter->m_s32Index = s32Index;
		iter->m_s32LineFlag = LINE_VALID;
		if (!boUseUnionPoint)
		{
			GetLine(iter->m_stPoint.x, iter->m_stPoint.y,
				iterNext->m_stPoint.x, iterNext->m_stPoint.y, &stLine);
		}
		else
		{
			GetLine(iter->m_stPoint.X, iter->m_stPoint.Y,
				iterNext->m_stPoint.X, iterNext->m_stPoint.Y, &stLine);
		}
		iter->m_stLine = stLine;
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

#if 1
	ReBuildIndexAndLine();
#else
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
#endif
	return 0;
}

INT32 CPolygonCtrl::MovePoint(INT32 xOffset, INT32 yOffset, INT32 s32Index/* = ~0*/)
{
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

	ReBuildIndexAndLine();

	return 0;
}

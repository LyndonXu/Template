#pragma once
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
	INT32 ReBuildIndexAndLine(bool boUseUnionPoint = false);
	bool IsPolygonValid(void)
	{
		return m_csPoints.size() > 2;
	}

};

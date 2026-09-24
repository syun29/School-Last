#include "CRect.h"

CRect::CRect() :  m_left(0), m_top(0), m_right(0), m_bottom(0), m_pos(0, 0), m_size(0, 0)
{
}

CRect::CRect(float l, float t, float r, float b): m_left(l), m_top(t), m_right(r), m_bottom(b), m_pos(l, t), m_size(r-l, b-t)
{
}

CRect::CRect(const CVector2D& pos, const CVector2D& size): m_left(pos.x), m_top(pos.y), m_right(pos.x+size.x), m_bottom(pos.y + size.y), m_pos(pos), m_size(size)
{
}

CRect::CRect(const CRect& rect): m_left(rect.m_left), m_top(rect.m_top), m_right(rect.m_right), m_bottom(rect.m_bottom), m_pos(rect.m_pos), m_size(rect.m_size)
{
}

CRect CRect::operator=(const CRect& rect)
{

	m_left = rect.m_left;
	m_top = rect.m_top;
	m_right = rect.m_right;
	m_bottom = rect.m_bottom;
	m_pos = rect.m_pos;
	m_size = rect.m_size;
	return *this;
}

void CRect::Set(float l, float t, float r, float b)
{

	m_left = l;
	m_top = t;
	m_right = r;
	m_bottom = b;
	m_pos.x = l;
	m_pos.y = t;
	m_size.x = r - l;
	m_size.y = b - t;
}

void CRect::Set(const CVector2D& pos, const CVector2D& size)
{
	m_pos = pos;
	m_size = size;
	m_left = m_pos.x;
	m_top = m_pos.y;
	m_right = m_pos.x+size.x;
	m_bottom = m_pos.y + size.y;
}

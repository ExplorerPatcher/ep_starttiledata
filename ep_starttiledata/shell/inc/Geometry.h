#pragma once

#include <Windows.h>

namespace Geometry
{
    struct CSize : SIZE
    {
        CSize(const SIZE& size) : SIZE(size)
        {
        }

        CSize(long cx, long cy)
        {
            this->cx = cx;
            this->cy = cy;
        }

        CSize() : CSize(0, 0)
        {
        }

        bool IsEmpty() const
        {
            return cx <= 0 || cy <= 0;
        }

        static CSize MulDiv(const SIZE& size, int nNumerator, int nDenominator)
        {
            return { ::MulDiv(size.cx, nNumerator, nDenominator), ::MulDiv(size.cy, nNumerator, nDenominator) };
        }
    };

    inline bool operator==(const CSize& x, const CSize& y)
    {
        return x.cx == y.cx && x.cy == y.cy;
    }

    inline bool operator!=(const CSize& x, const CSize& y)
    {
        return !(x == y);
    }

    struct CPointFloat : POINTFLOAT
    {
        CPointFloat(const POINTFLOAT& ptfl) : POINTFLOAT(ptfl)
        {
        }

        CPointFloat(float x, float y)
        {
            this->x = x;
            this->y = y;
        }

        CPointFloat() : CPointFloat(0, 0)
        {
        }

        void Offset(float dx, float dy)
        {
            x += dx;
            y += dy;
        }
    };

    struct CSegment1D
    {
        CSegment1D()
            : begin(0), end(0)
        {
        }

        CSegment1D(long begin, long end)
            : begin(begin), end(end)
        {
        }

        long begin;
        long end;

        void AlignBegin(long newBegin)
        {
            long size = GetSize();
            begin = newBegin;
            end = begin + size;
        }

        void AlignEnd(long newEnd)
        {
            long size = GetSize();
            end = newEnd;
            begin = end - size;
        }

        void AlignCenter(const CSegment1D& other)
        {
            long size = GetSize();
            begin = other.GetCenter() - (size / 2);
            end = begin + size;
        }

        bool Contains(long nValue) const
        {
            return nValue >= begin && nValue < end;
        }

        bool Contains(const CSegment1D& other) const
        {
            return other.begin >= begin && other.end <= end;
        }

        long GetCenter() const
        {
            return begin + GetSize() / 2;
        }

        long GetDistance(long point) const
        {
            if (point < begin) return begin - point;
            if (point > end) return point - end;
            return 0;
        }

        long GetSize() const
        {
            return end - begin;
        }

        bool Intersects(const CSegment1D& other) const
        {
            return begin < other.end && end > other.begin;
        }

        CSegment1D GetIntersection(const CSegment1D& other) const
        {
            CSegment1D intersection;
            if (Intersects(other))
            {
                intersection.begin = max(begin, other.begin);
                intersection.end = min(end, other.end);
            }
            return intersection;
        }

        void MirrorWithin(long rangeBegin, long rangeEnd)
        {
            begin = rangeEnd - (begin - rangeBegin);
            end = rangeEnd - (end - rangeBegin);
            if (begin > end)
            {
                long temp = begin;
                begin = end;
                end = temp;
            }
        }

        void Offset(long offset)
        {
            begin += offset;
            end += offset;
        }

        void Scale(long factor)
        {
            end = begin + (GetSize() * factor);
        }

        void ScaleF(float factor)
        {
            end = begin + (int)((float)GetSize() * factor);
        }

        void Inflate(long inflateAmount)
        {
            Inflate(inflateAmount, inflateAmount);
        }

        void Inflate(long inflateBegin, long inflateEnd)
        {
            begin -= inflateBegin;
            end += inflateEnd;
        }

        static CSegment1D FromSize(long start, long size)
        {
            return CSegment1D(start, start + size);
        }
    };

    inline bool operator==(const CSegment1D& x, const CSegment1D& y)
    {
        return x.begin == y.begin && x.end == y.end;
    }

    inline bool operator!=(const CSegment1D& x, const CSegment1D& y)
    {
        return !(x == y);
    }

    struct CPoint : POINT
    {
        CPoint(const POINT& pt) : POINT(pt)
        {
        }

        CPoint(long x, long y)
        {
            this->x = x;
            this->y = y;
        }

        CPoint() : CPoint(0, 0)
        {
        }

        void Offset(long dx, long dy)
        {
            x += dx;
            y += dy;
        }

        long GetDistanceSquared(const POINT& other) const
        {
            long dx = x - other.x;
            long dy = y - other.y;
            return dx * dx + dy * dy;
        }

        void BoundToRect(const RECT& rcBounds)
        {
            x = max(rcBounds.left, min(rcBounds.right, x));
            y = max(rcBounds.top, min(rcBounds.bottom, y));
        }

        static CPoint MulDiv(const POINT& pt, int nNumerator, int nDenominator)
        {
            return CPoint(
                ::MulDiv(pt.x, nNumerator, nDenominator),
                ::MulDiv(pt.y, nNumerator, nDenominator)
            );
        }

        static CPoint ClientToScreen(HWND hwnd, const POINT& ptClient)
        {
            CPoint ptScreen = ptClient;
            MapWindowPoints(hwnd, nullptr, &ptScreen, 1);
            return ptScreen;
        }

        static CPoint ScreenToClient(HWND hwnd, const POINT& ptScreen)
        {
            CPoint ptClient = ptScreen;
            MapWindowPoints(nullptr, hwnd, &ptClient, 1);
            return ptClient;
        }
    };

    inline bool operator==(const CPoint& x, const CPoint& y)
    {
        return x.x == y.x && x.y == y.y;
    }

    inline bool operator!=(const CPoint& x, const CPoint& y)
    {
        return !(x == y);
    }

    struct CRect : RECT
    {
        CRect(const RECT& rc) : RECT(rc)
        {
        }

        CRect(long left, long top, long right, long bottom)
        {
            this->left = left;
            this->top = top;
            this->right = right;
            this->bottom = bottom;
        }

        CRect() : CRect(0, 0, 0, 0)
        {
        }

        CRect(const SIZE& size) : CRect(0, 0, size.cx, size.cy)
        {
        }

        CRect(const POINT& pt, const SIZE& size) : CRect(pt.x, pt.y, pt.x + size.cx, pt.y + size.cy)
        {
        }

        bool Intersects(const RECT& other)
        {
            RECT rcIntersection;
            rcIntersection.left = max(this->left, other.left);
            rcIntersection.top = max(this->top, other.top);
            rcIntersection.right = min(this->right, other.right);
            rcIntersection.bottom = min(this->bottom, other.bottom);
            return rcIntersection.left < rcIntersection.right && rcIntersection.top < rcIntersection.bottom;
        }

        bool Contains(const POINT& point) const
        {
            return point.x >= this->left && point.x < this->right && point.y >= this->top && point.y < this->bottom;
        }

        POINT GetCenter() const
        {
            return { left + (right - left) / 2, top + (bottom - top) / 2 };
        }

        POINT GetTopLeft() const { return { left, top }; }
        POINT GetTopRight() const { return { right, top }; }
        POINT GetBottomLeft() const { return { left, bottom }; }
        POINT GetBottomRight() const { return { right, bottom }; }

        CPointFloat GetCenterFloat() const
        {
            return CPointFloat(left + (right - left) / 2.0f, top + (bottom - top) / 2.0f);
        }

        long GetHeight() const { return bottom - top; }
        CSegment1D GetHorizontalSegment() const { return CSegment1D(left, right); }

        POINT GetLocation() const { return { left, top }; }
        CSize GetSize() const { return CSize(GetWidth(), GetHeight()); }

        long GetArea() const { return GetWidth() * GetHeight(); }

        long GetWidth() const { return right - left; }
        CSegment1D GetVerticalSegment() const { return CSegment1D(top, bottom); }

        void Inflate(long dx, long dy) { InflateRect(this, dx, dy); }
        void Deflate(long dx, long dy) { Inflate(-dx, -dy); }
        void Deflate(SIZE size) { Deflate(size.cx, size.cy); }

        bool IsEmpty() const
        {
            return left >= right || top >= bottom;
        }

        // bool IsAnyDimensionLargerThan(const CRect& other) const;
        // bool IsAnyDimensionSmallerThan(const CRect& other) const;
        // void HorizontallyMirrorWithin(long rangeBegin, long rangeEnd);

        void Offset(long dx, long dy)
        {
            left += dx;
            right += dx;
            top += dy;
            bottom += dy;
        }

        // void MoveTo(long x, long y);

        static CRect MulDiv(const RECT& rect, int nNumerator, int nDenominator)
        {
            return CRect(
                ::MulDiv(rect.left, nNumerator, nDenominator),
                ::MulDiv(rect.top, nNumerator, nDenominator),
                ::MulDiv(rect.right, nNumerator, nDenominator),
                ::MulDiv(rect.bottom, nNumerator, nDenominator)
            );
        }

        long GetNearestEdgeToPointDistanceSquared(const CPoint& pt) const
        {
            long distance = 0;

            if (!Contains(pt))
            {
                if (pt.y >= this->top && pt.y <= this->bottom)
                {
                    distance = std::min<long>(abs(pt.x - this->left), abs(pt.x - this->right));
                    distance = distance * distance;
                }
                else if (pt.x >= this->left && pt.x <= this->right)
                {
                    distance = std::min<long>(abs(pt.y - this->top), abs(pt.y - this->bottom));
                    distance = distance * distance;
                }
                else
                {
                    long distanceLeft = (pt.x - this->left) * (pt.x - this->left);
                    long distanceTop = (pt.y - this->top) * (pt.y - this->top);
                    long distanceRight = (pt.x - this->right) * (pt.x - this->right);
                    long distanceBottom = (pt.y - this->bottom) * (pt.y - this->bottom);

                    long distanceTopLeft = distanceLeft + distanceTop;
                    long distanceTopRight = distanceRight + distanceTop;
                    long distanceBottomLeft = distanceLeft + distanceBottom;
                    long distanceBottomRight = distanceRight + distanceBottom;

                    distance = std::min<long>(
                        std::min<long>(distanceTopLeft, distanceTopRight),
                        std::min<long>(distanceBottomLeft, distanceBottomRight)
                    );
                }
            }

            return distance;
        }

        CRect Union(const CRect& other) const
        {
            return CRect(
                min(this->left, other.left),
                min(this->top, other.top),
                max(this->right, other.right),
                max(this->bottom, other.bottom)
            );
        }
    };

    inline bool operator==(const CRect& x, const RECT& y) // @MOD y originally Geometry::CRect
    {
        return x.left == y.left && x.top == y.top && x.right == y.right && x.bottom == y.bottom;
    }

    inline bool operator!=(const CRect& x, const RECT& y) // @MOD y originally Geometry::CRect
    {
        return !(x == y);
    }

    struct CRectFloat
    {
        float left;
        float top;
        float right;
        float bottom;

        CRectFloat(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom) {}

        CRectFloat() : left(0), top(0), right(0), bottom(0) {}

        CPointFloat GetCenter() const
        {
            return CPointFloat(left + GetWidth() * 0.5f, top + GetHeight() * 0.5f);
        }

        CPointFloat GetTopLeft() const
        {
            return CPointFloat(left, top);
        }

        CPointFloat GetTopRight() const
        {
            return CPointFloat(right, top);
        }

        CPointFloat GetBottomLeft() const
        {
            return CPointFloat(left, bottom);
        }

        CPointFloat GetBottomRight() const
        {
            return CPointFloat(right, bottom);
        }

        float GetHeight() const
        {
            return bottom - top;
        }

        float GetWidth() const
        {
            return right - left;
        }

        float GetArea() const
        {
            return GetWidth() * GetHeight();
        }

        CPointFloat GetLocation() const
        {
            return CPointFloat(left, top);
        }

        CPointFloat GetSize() const
        {
            return CPointFloat(GetWidth(), GetHeight());
        }

        bool IsEmpty() const
        {
            return right == left && bottom == top;
        }

        bool IsAnyDimensionLargerThan(const CRectFloat& other) const
        {
            return GetWidth() > other.GetWidth() || GetHeight() > other.GetHeight();
        }

        bool IsAnyDimensionSmallerThan(const CRectFloat& other) const
        {
            return GetWidth() < other.GetWidth() || GetHeight() < other.GetHeight();
        }

        operator CRect() const
        {
            return Round();
        }

        CRect Round() const
        {
            return CRect(_round(left), _round(top), _round(right), _round(bottom));
        }

        void Inflate(float amount)
        {
            Inflate(amount, amount);
        }

        void Inflate(float dx, float dy)
        {
            left -= dx;
            top -= dy;
            right += dx;
            bottom += dy;
        }

        void Inflate(CPointFloat amount)
        {
            Inflate(amount.x, amount.y);
        }

        void Deflate(float amount)
        {
            Inflate(-amount);
        }

        void Deflate(float dx, float dy)
        {
            Inflate(-dx, -dy);
        }

        void Deflate(CPointFloat amount)
        {
            Inflate(-amount.x, -amount.y);
        }

        CRectFloat Union(const CRectFloat& other) const
        {
            return CRectFloat(
                min(this->left, other.left),
                min(this->top, other.top),
                max(this->right, other.right),
                max(this->bottom, other.bottom)
            );
        }

        CRectFloat Intersection(const CRectFloat& other) const
        {
            return CRectFloat(
                max(this->left, other.left),
                max(this->top, other.top),
                min(this->right, other.right),
                min(this->bottom, other.bottom)
            );
        }

        void Offset(float x, float y)
        {
            left += x;
            top += y;
            right += x;
            bottom += y;
        }

        static int _round(float value)
        {
            return (int)(value + 0.5f);
        }
    };
}

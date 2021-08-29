#include "Geometry.hpp"

#include <cmath>

namespace vt
{
	bool PointIsInRegion(Region region, Point point)
	{
		return
			point.x >= region.x && point.y >= region.y &&
			point.x <= region.right() && point.y <= region.top();
	}

	// Helper function only, for testing collinear segments
	bool PointIsOnLine(Line line, Point p)
	{
		return
			(p.x <= line.p0.x || p.x <= line.p1.x) &&
			(p.x >= line.p0.x || p.x >= line.p1.x) &&
			(p.y <= line.p0.y || p.y <= line.p1.y) &&
			(p.y >= line.p0.y || p.y >= line.p1.y);
	}

	int Orientation(Point a, Point b, Point c)
	{
		float o = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);

		return (0.f < o) - (o < 0.f);
	}

	bool LineIntersectsLine(Line l0, Line l1)
	{
		int o1 = Orientation(l0.p0, l0.p1, l1.p0);
		int o2 = Orientation(l0.p0, l0.p1, l1.p1);
		int o3 = Orientation(l1.p0, l1.p1, l0.p0);
		int o4 = Orientation(l1.p0, l1.p1, l0.p1);

		if (o1 != o2 && o3 != o4)
		{
			return true;
		}

		if (o1 == 0 && PointIsOnLine(l0, l1.p0))
		{
			return true;
		}

		if (o2 == 0 && PointIsOnLine(l0, l1.p1))
		{
			return true;
		}

		if (o3 == 0 && PointIsOnLine(l1, l0.p0))
		{
			return true;
		}

		if (o4 == 0 && PointIsOnLine(l1, l0.p1))
		{
			return true;
		}

		return false;
	}

	bool LineIntersectsRegion(Region region, Line line)
	{
		return
			PointIsInRegion(region, line.p0) ||
			PointIsInRegion(region, line.p1) ||
			LineIntersectsLine(region.bottomEdge(), line) ||
			LineIntersectsLine(region.rightEdge(), line) ||
			LineIntersectsLine(region.topEdge(), line) ||
			LineIntersectsLine(region.leftEdge(), line);
	}

	bool CurveIntersectsLine(Line line, BezierCurve curve)
	{
		float dx = line.p1.x - line.p0.x;
		float dy = line.p1.y - line.p0.y;

		float ax = curve.p0.x - 2 * curve.p1.x + curve.p2.x;
		float ay = curve.p0.y - 2 * curve.p1.y + curve.p2.y;

		float bx = -2 * curve.p0.x + 2 * curve.p1.x;
		float by = -2 * curve.p0.y + 2 * curve.p1.y;

		float cx = curve.p0.x;
		float cy = curve.p0.y;

		float a = dy * ax - dx * ay;
		float b = dy * bx - dx * by;
		float c = dy * cx - dx * cy;

		float uc = -b * 0.5f / a;
		float ud = std::sqrt(b * b - 4 * a * c) * 0.5f / a;

		float u1 = uc - ud;
		float u2 = uc + ud;

		if (u1 >= 0.0f && u1 <= 1.0f)
		{
			float x1 =
				(1.0f - u1) * (1.0f - u1) * curve.p0.x +
				2 * u1 * (1.0f - u1) * curve.p1.x +
				u1 * u1 * curve.p2.x;
			float y1 =
				(1.0f - u1) * (1.0f - u1) * curve.p0.y +
				2 * u1 * (1.0f - u1) * curve.p1.y +
				u1 * u1 * curve.p2.y;

			if ((x1 >= line.p0.x || x1 >= line.p1.x) &&
				(x1 <= line.p0.x || x1 <= line.p1.x) &&
				(y1 >= line.p0.y || y1 >= line.p1.y) &&
				(y1 <= line.p0.y || y1 <= line.p1.y))
			{
				return true;
			}
		}

		if (u2 >= 0.0f && u2 <= 1.0f)
		{
			float x2 =
				(1.0f - u2) * (1.0f - u2) * curve.p0.x +
				2 * u2 * (1.0f - u2) * curve.p1.x +
				u2 * u2 * curve.p2.x;
			float y2 =
				(1.0f - u2) * (1.0f - u2) * curve.p0.y +
				2 * u2 * (1.0f - u2) * curve.p1.y +
				u2 * u2 * curve.p2.y;

			if ((x2 >= line.p0.x || x2 >= line.p1.x) &&
				(y2 >= line.p0.y || y2 >= line.p1.y))
			{
				return true;
			}
		}

		return false;
	}

	bool CurveIntersectsRegion(Region region, BezierCurve c)
	{
		return
			PointIsInRegion(region, c.p0) ||
			PointIsInRegion(region, c.p2) ||
			CurveIntersectsLine(region.bottomEdge(), c) ||
			CurveIntersectsLine(region.rightEdge(), c) ||
			CurveIntersectsLine(region.topEdge(), c) ||
			CurveIntersectsLine(region.leftEdge(), c);
	}

	Point NormalizeToRegion(Region region, Point p)
	{
		return { (p.x - region.x) / region.width, (p.y - region.y) / region.height };
	}
}
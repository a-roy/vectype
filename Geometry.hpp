#pragma once

namespace vt
{

	struct Point
	{
		float x;
		float y;
	};

	struct Line
	{
		Point p0;
		Point p1;
	};

	struct BezierCurve
	{
		Point p0;
		Point p1;
		Point p2;
	};

	struct Region
	{
		float x;
		float y;
		float width;
		float height;

		inline float right() { return x + width; }
		inline float top() { return y + height; }
		inline Line bottomEdge() { return { {x, y}, {right(), y} }; }
		inline Line rightEdge() { return { {right(), y}, {right(), top()} }; }
		inline Line topEdge() { return { {right(), top()}, {x, top()} }; }
		inline Line leftEdge() { return { {x, top()}, {x, y} }; }
	};

	bool PointIsInRegion(Region region, Point point);

	// Helper function only, for testing intersection of collinear segments
	bool PointIsOnLine(Line line, Point p);

	// Helper for testing intersection of line segments
	int Orientation(Point a, Point b, Point c);

	bool LineIntersectsLine(Line l0, Line l1);

	bool LineIntersectsRegion(Region region, Line line);

	bool CurveIntersectsLine(Line line, BezierCurve curve);

	bool CurveIntersectsRegion(Region region, BezierCurve c);

	Point NormalizeToRegion(Region region, Point p);
}

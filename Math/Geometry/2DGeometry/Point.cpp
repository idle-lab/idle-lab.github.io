#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr double eps = 1e-15;
struct Point;
double abs(const Point &x);

struct Point
{
    double x, y; // 二维向量，表示一个点
    Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}

    // 向量基本运算的实现+ - · *
    Point operator+(const Point &a) const { return Point{x + a.x, y + a.y}; }
    Point operator-(const Point &a) const { return Point{x - a.x, y - a.y}; }
    Point operator-() const { return Point{-x, -y}; }

    double operator|(const Point &a) const { return x * a.x + y * a.y; }  // 点乘
    double operator*(const Point &a) const { return x * a.y - y * a.x; }  // 叉乘
    Point operator*(const double a) const { return Point{x * a, y * a}; } // 向量数乘
    double pow() const { return x * x + y * y; }                          // 向量取平方

    // 求距离
    // 点到点的距离
    double disPoint(const Point &a) const
    {
        return sqrt((x - a.x) * (x - a.x) + (y - a.y) * (y - a.y));
    }

    // 点到直线的距离
    // a,b直线上两点
    double disline(const Point &a, const Point &b) const
    {
        Point ap = (*this) - a, ab = b - a;
        return abs(ap * ab) / abs(ab);
    }

    // 点到线段的距离
    // a,b线段两端点
    double disSeg(const Point &a, const Point &b) const
    {
        // 判断点和线段的位置关系
        if ((((*this) - a) | (b - a)) <= -eps || (((*this) - b) | (a - b)) <= -eps)
            return min(this->disPoint(a), this->disPoint(b));
        return disline(a, b);
    }

    // 重载 < 方便找线段交点
    bool operator<(const Point &b) const
    {
        if (y == b.y)
            return x < b.x;
        return y < b.y;
    }
};
double triangle(Point &a, Point &b, Point &c) { return (b - a) * (b - c) / 2.0; } // 求三角形面积
double abs(const Point &x) { return sqrt(x.pow()); }                              // 向量取模


/*
line类用来实现直线，线段
记录直线上两点或线段两个端点，和直线的方向向量
*/
struct line
{
    Point x1, x2;
    Point dVec;

    line(const Point &_x1 = {0, 0}, const Point &_x2 = {0, 0}) : x1(_x1),
                                                                 x2(_x2)
    {
        dVec = x2 - x1;
    }

    // 判断直线是否平行
    bool is_parallel(const line &b) const
    {
        return abs(dVec * b.dVec) <= eps;
    }

    // 求两直线交点
    Point line_intersection(const line &b) const
    {
        // 直线平行返回无穷大
        if (is_parallel(b))
            return {INT_MAX, INT_MAX};

        // 带入公式
        Point c = x2 - b.x2;
        double K = (b.dVec * c) / (dVec * b.dVec);
        Point res = x2 + dVec * K;
        return res;
    }

    // 求两线段交点
    // 交点不存在返回 INT_MAX
    Point seg_intersection(const line & b) const
    {
        // 判断平行
        if (is_parallel(b))
        {
            // 判断是否共线
            if ((b.x2 - x1) * (b.x1 - x1) == 0)
            {
                Point mi = max(min(x1, x2), min(b.x1, b.x2));
                Point ma = min(max(x1, x2), max(b.x1, b.x2));
                // 判断两线段是否重合
                if (ma.x >= mi.x && ma.y >= mi.y)
                    return mi;
            }
            return {INT_MAX, INT_MAX};
        }

        // 判断是否分别在线段的两端
        if (((b.x2 - x1) * dVec) * ((b.x1 - x1) * dVec) > 0)
            return {INT_MAX, INT_MAX};
        if (((x1 - b.x1) * b.dVec) * ((x2 - b.x1) * b.dVec) > 0)
            return {INT_MAX, INT_MAX};
        
        return line_intersection(b);
    }
};

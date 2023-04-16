#include "../include/grid_generator/common.h"
#include "../include/grid_generator/common_internal.h"
#include <math.h>
#include <stdexcept>

bool angle_in_arc(double arc_azimuth, double arc_angle, double angle)
{
    if (arc_azimuth + arc_angle > M_PI) return angle >= (arc_azimuth - M_PI) && angle <= (arc_azimuth + arc_angle - M_PI);
    else return angle >= (arc_azimuth) && angle <= (arc_azimuth + arc_angle);
}

gg::Vector::Vector() {}

gg::Vector::Vector(double x, double y) : x(x), y(y) {}

gg::Vector gg::Vector::operator*(double b) const
{
    return Vector(x * b, y * b);
}

gg::Vector gg::Vector::operator/(double b) const
{
    return Vector(x / b, y / b);
}

gg::Vector gg::Vector::operator+(const Vector &b) const
{
    return Vector(x + b.x, y + b.y);
}

gg::Vector gg::Vector::operator-(const Vector &b) const
{
    return Vector(x - b.x, y - b.y);
}

double gg::Vector::dot(const Vector &b) const
{
    return x * b.x + y * b.y;
}

double gg::Vector::norm() const
{
    return sqrt(x * x + y * y);
}

double gg::Vector::squared_norm() const
{
    return x * x + y * y;
}

gg::Figure::~Figure() {}

gg::Intersection::Intersection() : valid(false) {}

gg::Intersection::Intersection(Vector coord, Vector vector, Vector normal) : valid(true), coord(coord), vector(vector/vector.norm()), normal(normal/normal.norm()) {}

gg::Circle::Circle(Vector center, double radius, bool normal_inwards) : _center(center), _radius(radius), _normal_inwards(normal_inwards) {}

gg::Intersection gg::Circle::intersection(Vector a, Vector b) const
{
    double A = 1;
    double B = -2 * (_center - a).dot(b-a) / (b-a).norm();
    double C = (_center - a).squared_norm() - _radius*_radius;
    double determinant = B*B - 4 * A * C;
    if (determinant == 0)
    {
        double L = (-B) / (2 * A);
        if (L >= 0 && L <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L / (b-a).norm();
            return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center));
        }
    }
    else if (determinant > 0)
    {
        double L1 = (-B - sqrt(determinant)) / (2 * A);
        if (L1 >= 0 && L1 <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L1 / (b-a).norm();
            return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center));
        }
        double L2 = (-B + sqrt(determinant)) / (2 * A);
        if (L2 >= 0 && L2 <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L2 / (b-a).norm();
            return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center));
        }
    }
    return Intersection();
}

gg::Arc::Arc(Vector center, double radius, bool normal_inwards, double azimuth, double angle) : _center(center), _radius(radius), _normal_inwards(normal_inwards), _azimuth(azimuth), _angle(angle) {}

gg::Intersection gg::Arc::intersection(Vector a, Vector b) const
{
    const double A = 1;
    const double B = -2 * (_center - a).dot(b - a) / (b - a).norm();
    const double C = (_center - a).squared_norm() - _radius*_radius;
    const double determinant = B*B - 4 * A * C;
    if (determinant == 0)
    {
        const double L = (-B) / (2 * A);
        if (L >= 0 && L <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L / (b-a).norm();
            double angle = atan2((I - _center).y, (I - _center).x);
            if (angle >= _azimuth && angle <= (_azimuth + angle)) return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center)); //Valid azimuth
        }
    }
    else if (determinant > 0)
    {
        const double L1 = (-B - sqrt(determinant)) / (2 * A);
        if (L1 >= 0 && L1 <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L1 / (b-a).norm();
            const double angle = atan2((I - _center).y, (I - _center).x);
            if (angle_in_arc(_azimuth, _angle, angle)) return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center)); //Valid azimuth
        }
        double L2 = (-B + sqrt(determinant)) / (2 * A);
        if (L2 >= 0 && L2 <= (b-a).norm()) //Valid length
        {
            Vector I = a + (b-a) * L2 / (b-a).norm();
            const double angle = atan2((I - _center).y, (I - _center).x);
            if (angle_in_arc(_azimuth, _angle, angle)) return Intersection(I, rotate_ccw(I - _center), _normal_inwards ? (_center - I) : (I - _center)); //Valid azimuth
        }
    }
    return Intersection();
}

gg::Line::Line(Vector a, Vector b, bool normal_cw) : _a(a), _b(b), _normal_cw(normal_cw) {}

gg::Intersection gg::Line::intersection(Vector a, Vector b) const
{
    //Equation "a + (b - a) * t = _a + (_b - _a) * s" transformed into "A * [t s] = b" and solved
    const double A00 = b.x - a.x;
    const double A01 = -_b.x + _a.x;
    const double A10 = b.y - a.y;
    const double A11 = -_b.y + _a.y;
    const double b0 = -a.x + _a.x;
    const double b1 = -a.y + _a.y;
    const double determinant = A00 * A11 - A01 * A10;
    const double t = (A11 * b0 - A01 * b1) / determinant;
    if (t < 0.0 || t > 1.0) return Intersection();
    const double s = (-A10 * b0 - A00 * b1) / determinant;
    if (s < 0.0 || s > 1.0) return Intersection();
    return Intersection(a + (b - a) * t, _b - _a, _normal_cw ? rotate_cw(_b-_a) : rotate_ccw(_b-_a));
}

gg::Boundary::Boundary(const Figure *fig) : _figure(fig)
{
    if (_figure == nullptr) throw std::runtime_error("gg::Boundary::Boundary(): Figure is nullptr");
}

const gg::Figure *gg::Boundary::figure() const
{
    return _figure;
}

gg::Boundary::~Boundary()
{
    if (_figure != nullptr) delete _figure;
}
#include "../include/grid_generator/common_internal.h"
#include <math.h>

bool gg::Position::operator<(const Position &b) const
{
    if (xi != b.xi) return xi < b.xi;
    else if (yi != b.yi) return yi < b.yi;
    else return (!upside_down && b.upside_down);
}

unsigned int gg::get_shape(const Parameters &parameters)
{
    switch (parameters.typ)
    {
        case GridType::triangular: return 3;
        case GridType::hexagonal: return 6;
        default: return 4;
    }
}

double gg::get_area(const Parameters &parameters)
{
    switch (parameters.typ)
    {
        case GridType::triangular: return parameters.size.x * parameters.size.y * sqrt(3) / 4.0;
        case GridType::hexagonal: return 6.0 * parameters.size.x * parameters.size.y * sqrt(3) / 4.0;;
        default: return parameters.size.x * parameters.size.y;
    }
}

gg::Vector gg::get_center(const Parameters &parameters, Position position)
{
    Vector coord;
    switch (parameters.typ)
    {
        case GridType::triangular:
            coord = Vector(0.5 * position.yi + position.xi, 0.5 * sqrt(3) * position.yi);
            if (position.upside_down) coord = coord + Vector(1.0 / 4.0, sqrt(3) / 12);
            else coord = coord - Vector(1.0 / 4.0, sqrt(3) / 12);
            break;
        case GridType::hexagonal:
            coord = Vector(position.yi + 2.0 * position.xi, sqrt(3) * position.yi);
            break;
        default: //case GridType::square:
            coord = Vector(position.xi, position.yi);
            break;
    }
    return parameters.origin + rotate(Vector(parameters.size.x * coord.x, parameters.size.y * coord.y), parameters.inclination);
}

std::array<gg::Vector, 6> gg::get_points(const Parameters &parameters, Position point)
{
    std::array<gg::Vector, 6> coords;
    unsigned int coords_size;
    switch (parameters.typ)
    {
        case GridType::triangular:
        {
            Vector center(0.5 * point.yi + point.xi, 0.5 * sqrt(3) * point.yi);
            std::array<gg::Vector, 3> offsets;
            offsets[0] = Vector(-3.0 / 4.0, -sqrt(3) / 4);
            offsets[1] = Vector(1.0 / 4.0, -sqrt(3) / 4);
            offsets[2] = Vector(-1.0 / 4.0, sqrt(3) / 4);
            for (unsigned int i = 0; i < 3; i++) coords[i] = point.upside_down ? (center - offsets[i]) : (center + offsets[i]);
            coords_size = 3;
            break;
        }
        case GridType::hexagonal:
        {
            Vector center(point.yi + 2.0 * point.xi, sqrt(3) * point.yi);
            coords[0] = center + Vector(0, -1);
            coords[1] = center + Vector(sqrt(3) / 4, -1.0 / 2.0);
            coords[2] = center + Vector(sqrt(3) / 4, 1.0 / 2.0);
            coords[3] = center + Vector(0, 1);
            coords[4] = center + Vector(-sqrt(3) / 4, 1.0 / 2.0);
            coords[5] = center + Vector(-sqrt(3) / 4, -1.0 / 2.0);
            coords_size = 6;
            break;
        }
        default: //case GridType::square:
        {
            Vector center(point.xi, point.yi);
            coords[0] = center + Vector(-0.5, -0.5);
            coords[1] = center + Vector(0.5, -0.5);
            coords[2] = center + Vector(0.5, 0.5);
            coords[3] = center + Vector(-0.5, 0.5);
            coords_size = 4;
            break;
        }
    }
    for (unsigned int i = 0; i < coords_size; i++)
    {
        coords[i] = parameters.origin + rotate(Vector(parameters.size.x * coords[i].x, parameters.size.y * coords[i].y), parameters.inclination);
    }
    return coords;
}

gg::FacePosition gg::get_face_neighbor(const Parameters &parameters, FacePosition face)
{
    int one;
    switch (parameters.typ)
    {
    case GridType::triangular:
        one = (face.position.upside_down ? -1 : 1);
        switch (face.face)
        {
            case 0: face.position.yi -= one; break;
            case 2: face.position.xi -= one; break;
        }
        face.position.upside_down = !face.position.upside_down;
        break;
    case GridType::hexagonal:
        switch (face.face)
        {
            case 0: face.position.yi--; face.position.xi++; break;
            case 1: face.position.xi++; break;
            case 2: face.position.yi++; break;
            case 3: face.position.xi--; face.position.yi++; break;
            case 4: face.position.xi--; break;
            default: face.position.yi--; break;
        }
        face.face = (face.face + 3) % 6;
        break;
    default: //case GridType::square:
        switch (face.face)
        {
            case 0: face.position.yi--; break;
            case 1: face.position.xi++; break;
            case 2: face.position.yi++; break;
            default: face.position.xi--; break;
        }
        face.face = (face.face + 2) % 4;
        break;
    }
    return face;
}

std::array<gg::PointPosition, 6> gg::get_point_neighbors(const Parameters &parameters, PointPosition point)
{
    std::array<gg::PointPosition, 6> result;
    int one;
    switch (parameters.typ)
    {
    case GridType::triangular:
        for (unsigned int i = 0; i < 6; i++) { result[i].position = point.position; result[i].point = (unsigned int)-1; }
        one = (point.position.upside_down ? -1 : 1);
        switch (point.point)
        {
        case 0:
            result[0].point = 2; result[0].position.xi -= one; result[0].position.upside_down = !point.position.upside_down;
            result[1].point = 1; result[1].position.xi -= one;
            result[2].point = 0; result[2].position.xi -= one; result[2].position.yi -= one; result[2].position.upside_down = !point.position.upside_down;
            result[3].point = 2; result[3].position.yi -= one;
            result[4].point = 1; result[4].position.yi -= one; result[4].position.upside_down = !point.position.upside_down;
            break;
        case 1:
            result[0].point = 0; result[0].position.yi -= one; result[0].position.upside_down = !point.position.upside_down;
            result[1].point = 2; result[1].position.xi += one; result[1].position.yi -= one;
            result[2].point = 1; result[2].position.xi += one; result[2].position.yi -= one; result[2].position.upside_down = !point.position.upside_down;
            result[3].point = 0; result[3].position.xi += one;
            result[4].point = 2; result[4].position.upside_down = !point.position.upside_down;
            break;
        default:
            result[0].point = 1; result[0].position.upside_down = !point.position.upside_down;
            result[1].point = 0; result[1].position.yi += one;
            result[2].point = 2; result[2].position.xi -= one; result[2].position.yi += one; result[2].position.upside_down = !point.position.upside_down;
            result[3].point = 1; result[3].position.xi -= one; result[3].position.yi += one;
            result[4].point = 0; result[4].position.xi += one; result[4].position.upside_down = !point.position.upside_down;
            break;
        }
        break;
    case GridType::hexagonal:
        for (unsigned int i = 0; i < 6; i++) { result[i].position = point.position; result[i].point = (unsigned int)-1; }
        switch (point.point)
        {
            case 0: result[0].position.yi--; result[1].position.xi++; result[1].position.yi--; break;
            case 1: result[0].position.xi++; result[0].position.yi--; result[1].position.xi++; break;
            case 2: result[0].position.xi++; result[1].position.yi++; break;
            case 3: result[0].position.yi++; result[1].position.xi--; result[1].position.yi++; break;
            case 4: result[0].position.xi--; result[0].position.yi++; result[1].position.xi--; break;
            default: result[0].position.xi--; result[1].position.yi--; break;
        }
        result[0].point = (point.point + 2) % 6;
        result[1].point = (point.point + 4) % 6;
        break;
    default: //case GridType::square:
        for (unsigned int i = 0; i < 6; i++) { result[i].position = point.position; result[i].point = (unsigned int)-1; }
        switch (point.point)
        {
            case 0: result[0].position.xi--; result[1].position.xi--; result[1].position.yi--; result[2].position.yi--; break;
            case 1: result[0].position.yi--; result[1].position.xi++; result[1].position.yi--; result[2].position.xi++; break;
            case 2: result[0].position.xi++; result[1].position.xi++; result[1].position.yi++; result[2].position.yi++; break;
            default: result[0].position.yi++; result[1].position.xi--; result[1].position.yi++; result[2].position.xi--; break;
        }
        result[0].point = (point.point + 1) % 4;
        result[1].point = (point.point + 2) % 4;
        result[2].point = (point.point + 3) % 4;
        break;
    }
    return result;
}

gg::Vector gg::rotate_ccw(Vector v)
{
    return Vector(v.y, -v.x);
}

gg::Vector gg::rotate_cw(Vector v)
{
    return Vector(-v.y, v.x);
}

gg::Vector gg::rotate(Vector v, double angle)
{
    return Vector(cos(angle) * v.x - sin(angle) * v.y, sin(angle) * v.x + cos(angle) * v.y);
}
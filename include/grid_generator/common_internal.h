/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/

#pragma once
#include "common.h"
#include <array>

namespace gg
{
    ///Two-dimensional integer position of an element on the grid (triangular, square, hexagonal)
    struct Position
    {
        int xi = 0;
        int yi = 0;
        bool upside_down = false;
        bool operator<(const Position &b) const;
    };

    ///Structure that describes element and its face
    struct FacePosition
    {
        Position position;
        unsigned int face;
    };

    ///Structure that describes element and its point
    struct PointPosition
    {
        Position position;
        unsigned int point;
    };

    ///Gets number of points/faces
    unsigned int get_shape(const Parameters &parameters);
    ///Gets center of the element
    Vector get_center(const Parameters &parameters, Position position);
    ///Gets points of the element
    Vector get_point(const Parameters &parameters, PointPosition point);
    ///Gets neighbors of the face
    FacePosition get_face_neighbor(const Parameters &parameters, FacePosition face);
    ///Gets neighbors of the point
    std::array<PointPosition, 6> get_point_neighbors(const Parameters &parameters, PointPosition point);
}
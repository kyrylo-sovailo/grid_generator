/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/ 

#pragma once
#include "common.h"
#include <set>

namespace gg
{
    ///Standalone point that is a part of point grid
    template <class B = Boundary>
    class StandalonePoint
    {
    protected:
        Vector _coord;
        Vector _normal;
        bool _boundary;
        std::vector<StandalonePoint*> _neighbors;
    public:
        ///Creates point
        ///@param coord Coordinate of the point
        StandalonePoint(Vector coord);
        ///Creates point that touches a boundary
        ///@param coord Coordinate of the point
        ///@param intersection INtersection with the boundary
        ///@param boundary boundary
        StandalonePoint(Vector coord, Intersection intersection, B boundary);
        ///Gets point coordinate
        Vector coord() const;
        ///Gets point normal
        Vector normal() const;
        ///Gets whether point touches the boundary
        bool boundary() const;
        ///Gets list of point neighbors
        std::vector<StandalonePoint*> &neighbors();
    };

    ///Point grid parameters
    struct PointGridParameters : Parameters {};

    ///Point grid
    template <class B = Boundary, class P = StandalonePoint<B>>
    class PointGrid
    {
    protected:
        std::set<P*> _points;
    public:
        ///Creates point grid
        ///@param parameters Point grid parameters
        ///@param boundaries Grid boundaries
        PointGrid(const PointGridParameters &parameters, const std::vector<B> &boundaries);
        ///Gets list of points
        std::set<P*> &points();
    };
}
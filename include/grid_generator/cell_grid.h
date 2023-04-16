/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/ 

#pragma once
#include <vector>
#include <set>
#include <array>

namespace gg
{
    ///Standalone point that is a part of point grid
    template <class B = Boundary>
    class Point
    {
    protected:
        Vector _coord;
        Vector _normal;
    public:
        ///Creates point
        ///@param coord Coordinate of the point
        Point(Vector coord);
        ///Creates point on a boundary
        ///@param intersection Intersection with the boundary
        ///@param boundary Boundary
        Point(Intersection intersection, const B *boundary);
        ///Gets point coordinate
        Vector coord() const;
        ///Gets point normal
        Vector normal() const;
    };

    /// Face between two points
    template <class B = Boundary, class P = Point<B>>
    class Face
    {
    protected:
        Vector _center;
        Vector _normal;
        double _length;
        std::array<P*, 2> _points;
    public:
        ///Creates face
        ///@param a First point
        ///@param b Second point
        Face(P *a, P *b);
        ///Creates face that touches a boundary
        ///@param a First point (free point)
        ///@param b Second point (lies on boundary)
        ///@param intersection Intersection with the boundary
        ///@param boundary Boundary
        Face(P *a, P *b, Intersection intersection, const B *boundary);
        ///Gers face center
        Vector center() const;
        ///Gets face normal
        Vector normal() const;
        ///Gets face length
        double length() const;
        ///Gets face points
        std::array<P*, 2> points() const;
    };

    /// Cell is the mein structural element of cellular grid
    template <class B = Boundary, class P = Point<B>, class F = Face<B, P>>
    class Cell
    {
    public:
        ///Information specific for each point, face, or neighbor of the cell
        struct Side
        {
            P *point;       ///< Point (clockwise in respect to face)
            F *face;        ///< Face
            Cell *cell;     ///< Neighbor cell
            bool inwards;   ///< Face normal points inwards, otherwise outwards
        };
    protected:
        Vector _center;
        double _area;
        std::vector<Side> _sides;
    public:
        ///Creates cell
        ///@param center Cell center
        ///@param area Cell area
        Cell(Vector center, double area);
        ///Creates cell that touches a boundary
        ///@param center Cell center
        ///@param area Cell area
        ///@param intersection Intersection with the boundary
        ///@param boundary Boundary
        Cell(Vector center, double area, Intersection intersection, const B *boundary);
        ///Gets coordinate of the center of the cell
        Vector center() const;
        ///Gets area of the cell
        double area() const;
        ///Gets sides of the cell
        std::vector<Side> &sides();
    };

    ///Point grid parameters
    struct CellGridParameters : Parameters
    {
        double threshold_area = 0.5;    ///< Minimal area of the cell by which the cell is created (0.0 <= threshold_area <= 1.0)
    };
    
    ///Cellular grid
    template <class B = Boundary, class P = Point<B>, class F = Face<B, P>, class C = Cell<B, P, F>>
    class CellGrid
    {
    protected:
        std::set<P*> _points;
        std::set<F*> _faces;
        std::set<C*> _cells;
    public:
        ///Creates cellular grid
        ///@param parameters Cell grid parameters
        ///@param boundaries Grid boundaries
        CellGrid(const CellGridParameters &parameters, const std::vector<B> &boundaries);
        ///Gets list of points
        std::set<P*> &points();
        ///Gets list of faces
        std::set<F*> &faces();
        ///Gets list of cells
        std::set<C*> &cells();
    };
}
/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/

#pragma once
#include <vector>

namespace gg
{
    ///Grid type
    enum class GridType
    {
        triangular,
        square,
        hexagonal
    };

    ///2D Vector
    struct Vector
    {
        ///X component of coordinate
        double x;
        ///Y component of coordinate
        double y;
        ///Creates vector
        Vector();
        ///Creates vector from given coordinates
        Vector(double x, double y);
        ///Multiplies vector by scalar
        Vector operator*(double b) const;
        ///Divides vector by scalar
        Vector operator/(double b) const;
        ///Adds vector to vector
        Vector operator+(const Vector &b) const;
        ///Subtracts vector to vector
        Vector operator-(const Vector &b) const;
        ///Performs dot multiplication on two vectors
        double dot(const Vector &b) const;
        ///Gets a norm of a vector
        double norm() const;
        ///Gets squared norm of a vector
        double squared_norm() const;
    };

    ///Intersection between figure and segment
    struct Intersection
    {
        ///Whether the intersection is valid
        bool valid;
        ///Coordinate of the intersection
        Vector coord;
        ///Vector of the surface
        Vector vector;
        ///Normal of the surface
        Vector normal;
        ///Creates invalid intersection
        Intersection();
        ///Creates valid intersection
        Intersection(Vector coord, Vector vector, Vector normal);
    };

    ///Abstract figure
    class Figure
    {
    public:
        ///Searches for intersection between figure and line
        ///@param a Beginning of the line
        ///@param b Ending of the line
        virtual Intersection intersection(Vector a, Vector b) const = 0;
        ///@brief Destroys figure
        virtual ~Figure() = 0;
    };

    ///Circle figure
    class Circle : public Figure
    {
    protected:
        Vector _center;
        double _radius;
        bool _normal_inwards;
    public:
        ///Creates circle
        ///@param center Center of the circle
        ///@param radius Radius of the circle
        ///@param normal_inwards Normal points inwards, otherwise outwards
        Circle(Vector center, double radius, bool normal_inwards);
        ///Searches for intersection between figure and line
        ///@param a Beginning of the line
        ///@param b Ending of the line
        virtual Intersection intersection(Vector a, Vector b) const;
    };

    ///Arc figure
    class Arc : public Figure
    {
    protected:
        Vector _center;
        double _radius;
        bool _normal_inwards;
        double _azimuth;
        double _angle;
    public:
        ///Creates circle
        ///@param center Center of the circle
        ///@param radius Radius of the circle
        ///@param normal_inwards Normal points inwards, otherwise outwards
        ///@param azimuth Azimuth of the beginning of the arc (radians, counterclockwise)
        ///@param angle Angle of the arc (radians, counterclockwise)
        Arc(Vector center, double radius, bool normal_inwards, double azimuth, double angle);
        ///Searches for intersection between figure and line
        ///@param a Beginning of the line
        ///@param b Ending of the line
        virtual Intersection intersection(Vector a, Vector b) const;
    };

    ///Line figure
    class Line : public Figure
    {
    protected:
        Vector _a; 
        Vector _b;
        bool _normal_cw;
    public:
        ///Creates line
        ///@param a Beginning of the line
        ///@param b Ending of the line
        ///@param normal_cw Normal is pointing clockwise of the line vector, otherwise counterclockwise
        Line(Vector a, Vector b, bool normal_cw);
        ///Searches for intersection between figure and line
        ///@param a Beginning of the line
        ///@param b Ending of the line
        virtual Intersection intersection(Vector a, Vector b) const;
    };

    ///Boundary consists of boundary figure and boundary conditions
    class Boundary
    {
    protected:
        const Figure *_figure;
    public:
        ///Creates boundary
        ///@param fig Boundary figure
        Boundary(const Figure *fig);
        ///Transfers boundary
        ///@param other Boundary to be transferred
        Boundary(Boundary &&other);
        ///Returns boundary figure
        const Figure *figure() const;
        ///Destroys boundary and frees figure
        ~Boundary();
    };

    ///Grid parameters
    struct Parameters
    {
        GridType typ = GridType::square;    ///< Grid type
        Vector origin = Vector(0.0, 0.0);   ///< Grid origin
        Vector size = Vector(1.0, 1.0);     ///< Size of element side
        double inclination = 0.0;           ///< Grid inclination (radians, counterclockwise)
    };
}

/** @mainpage Grid generator

This library is used for grid generation. The library can generate two types of grids: point grid and cellular grid. Point grids consist only of points on 2D plane. Cellular grids is a 2D hierarchical grid with points, faces and cells. All underlying classes are template arguments, allowing almost unlimited customization.

@section Build
The build is automated using [CMake](https://cmake.org):
@code{.sh}
mkdir build
cd build
cmake ..							# Configure library
cmake --build .						# Build library
cmake --build . --target doc		# Generate documentation (requires Doxygen)
cmake --build . --target test		# Run test (requires GTest)
cmake --build . --target install	# Install library
cmake --build . --target uninstall	# Uninstall library
@endcode

@section Example
@code{.cpp}
#include <grid_generator/point_grid.hxx>
#include <iostream>

//Initialize square boundaries
std::vector<gg::Boundary> boundaries;
boundaries.push_back(new gg::Line(gg::Vector(0.5, 0.5), gg::Vector(0.5, -0.5), false));
boundaries.push_back(new gg::Line(gg::Vector(0.5, -0.5), gg::Vector(-0.5, -0.5), false));
boundaries.push_back(new gg::Line(gg::Vector(-0.5, -0.5), gg::Vector(-0.5, 0.5), false));
boundaries.push_back(new gg::Line(gg::Vector(-0.5, 0.5), gg::Vector(0.5, 0.5), false));

//Initialize point grid parameters
gg::PointGridParameters point_parameters;
point_parameters.size = gg::Vector(0.3, 0.3);

//Construct point grid
gg::PointGrid<> point_grid(point_parameters, boundaries);

//Print point coordinates
for (std::map<gg::StandalonePoint<>>::iterator i = point_grid->points().begin(); i != point_grid->points().end(); i++)
{
	std::cout << "X: " << i->coord().x << "Y: " << i->coord().y << "\n";
}    
@endcode

@section Reduction of compile time
This project provides a possibility for reduction of compile time. This is achieved by separating template headers (`*.h`) and template implementations (`*.hxx`). The user has an option to include `*.h` headers in their code, and then implement the used template classes in a separate `*.cpp` by using `template class gg::PointGrid<>;` constructions.
*/
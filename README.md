# Welcome to grid_generator

This library is used for grid generation. The library can generate two types of grids: point grid and cellular grid. Point grids consist only of points on 2D plane. Cellular grids is a 2D hierarchical grid with points, faces and cells. All underlying classes are template arguments, allowing almost unlimited customization.

### Build
The build is automated using [CMake](https://cmake.org):
```
mkdir build
cd build
cmake ..							# Configure library
cmake --build .						# Build library
cmake --build . --target doc		# Generate documentation (requires Doxygen)
cmake --build . --target test		# Run test (requires GTest)
cmake --build . --target install	# Install library
cmake --build . --target uninstall	# Uninstall library
```

### Example
```
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
```

### Reduction of compile time
This project provides a possibility for reduction of compile time. This is achieved by separating template headers (`*.h`) and template implementations (`*.hxx`). The user has an option to include `*.h` headers in their code, and then implement the used template classes in a separate `*.cpp` by using `template class gg::PointGrid<>;` constructions.
#include "../include/grid_generator/point_grid.hxx"
#include "../include/grid_generator/cell_grid.hxx"
#include <iostream>

int _main()
{
    std::vector<gg::Boundary> boundaries;
    boundaries.push_back(new gg::Line(gg::Vector(1.0, 1.0), gg::Vector(1.0, -1.0), false));
    boundaries.push_back(new gg::Line(gg::Vector(1.0, -1.0), gg::Vector(-1.0, -1.0), false));
    boundaries.push_back(new gg::Line(gg::Vector(-1.0, -1.0), gg::Vector(-1.0, 1.0), false));
    boundaries.push_back(new gg::Line(gg::Vector(-1.0, 1.0), gg::Vector(1.0, 1.0), false));

    //Point grid
    gg::PointGridParameters point_parameters;
    point_parameters.size = gg::Vector(0.3, 0.3);
    gg::PointGrid<> point_grid(point_parameters, boundaries);
    std::cout << "Constructed point grid has " <<
        point_grid.points().size() << " points" <<
        "(" << (point_grid.points().size() == 9 ? "correct" : "incorrect") << ")\n";

    //Cell grid
    gg::CellGridParameters cell_parameters;
    cell_parameters.size = gg::Vector(0.3, 0.3);
    cell_parameters.threshold_area = 0.0;
    gg::CellGrid<> cell_grid(cell_parameters, boundaries);
    std::cout << "Constructed cellular grid has " <<
        cell_grid.points().size() << " points, " <<
        cell_grid.faces().size() << " faces, " <<
        cell_grid.cells().size() << " cells " <<
        "(" << ((cell_grid.points().size() == 21 && cell_grid.points().size() == 21 && cell_grid.points().size() == 12) ? "correct" : "incorrect") << ")\n";

    return 0;
}

int main()
{
    try
    {
        return _main();
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }    
}
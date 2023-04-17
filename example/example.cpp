#include "../include/grid_generator/point_grid.hxx"
#include "../include/grid_generator/cell_grid.hxx"
#include <gtest/gtest.h>

TEST (GridTest, PointGridTest)
{
    std::vector<gg::Boundary> boundaries;
    boundaries.push_back(new gg::Line(gg::Vector(0.5, 0.5), gg::Vector(0.5, -0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(0.5, -0.5), gg::Vector(-0.5, -0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(-0.5, -0.5), gg::Vector(-0.5, 0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(-0.5, 0.5), gg::Vector(0.5, 0.5), false));
    gg::PointGridParameters point_parameters;
    point_parameters.size = gg::Vector(0.3, 0.3);

    gg::PointGrid<> *point_grid;
    EXPECT_NO_THROW(point_grid = new gg::PointGrid<>(point_parameters, boundaries));
    EXPECT_EQ(point_grid->points().size(), 9);
    EXPECT_NO_THROW(delete point_grid);
}

TEST (GridTest, CellGridTest)
{
    std::vector<gg::Boundary> boundaries;
    boundaries.push_back(new gg::Line(gg::Vector(0.5, 0.5), gg::Vector(0.5, -0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(0.5, -0.5), gg::Vector(-0.5, -0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(-0.5, -0.5), gg::Vector(-0.5, 0.5), false));
    boundaries.push_back(new gg::Line(gg::Vector(-0.5, 0.5), gg::Vector(0.5, 0.5), false));
    gg::CellGridParameters cell_parameters;
    cell_parameters.size = gg::Vector(0.3, 0.3);
    cell_parameters.threshold_area = 0.0;

    gg::CellGrid<> *cell_grid;
    EXPECT_NO_THROW(cell_grid = new gg::CellGrid<>(cell_parameters, boundaries));
    EXPECT_EQ(cell_grid->points().size(), 32);
    EXPECT_EQ(cell_grid->faces().size(), 56);
    EXPECT_EQ(cell_grid->cells().size(), 25);
    EXPECT_NO_THROW(delete cell_grid);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
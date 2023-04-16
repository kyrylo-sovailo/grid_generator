/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/ 

#pragma once
#include "cell_grid.h"
#include "common_internal.h"
#include <map>
#include <math.h>

/*
    Cells consist of points. Each point can be active, passive or unreached
    Unreached points have not yet been reached by the algorithm
    Active and passive points are both "reached" points, they are divided in two for optimization reasons
    Active points should be searched around, while the search around passive points is already complete and will bring no more results
    Active cell is a cell that has active points. The algorithm searches around active cells.
    Because the point is located in multiple cells simultaneously, the search is performed in each cell
    To prevent any inconsistency when searching along faces in different cells, the algorithm performs the search one time, and then saves the result
    
    Because adding/removing elements from set cannot be done while iterating through it, there are sets like "to_be_active", etc.
*/

enum class PointStatus
{
    unreached,
    to_be_active,
    active,
    passive
};

template <class B, class P, class F, class C>
struct TemporaryCell
{
    struct TemporaryPoint
    {
        PointStatus status = PointStatus::unreached;

        P *point = nullptr;
    };
    std::vector<TemporaryPoint> points;

    struct TemporaryFace
    {
        bool probed = false;            //Face was probed and intersections were searched
        gg::Intersection intersection;  //Found intersection, includes coordinate and normal (result of probing)
        const B *boundary = nullptr;    //Found boundary (result of probing)

        P *point = nullptr;
        F *face = nullptr;
    };
    std::vector<TemporaryFace> faces;

    gg::Intersection intersection;  //Found intersection (also propagated due to failed cells)
    const B *boundary = nullptr;    //Found boundary (also propagated due to failed cells)

    bool complete;
    double area;
    gg::Vector center;
    
    C *cell = nullptr;

    TemporaryCell(const gg::CellGridParameters &parameters) : points(gg::get_shape(parameters)), faces(gg::get_shape(parameters)) {}
};

template <class B> gg::Point<B>::Point(Vector coord) :
    _coord(coord), _normal(0,0) {}

template <class B> gg::Point<B>::Point(Intersection intersection, const B *boundary) :
    _coord(intersection.coord), _normal(intersection.normal) {}

template <class B> gg::Vector gg::Point<B>::coord() const
{
    return _coord;
}

template <class B> gg::Vector gg::Point<B>::normal() const
{
    return _normal;
}

template <class B, class P> gg::Face<B, P>::Face(P *a, P *b) :
    _center((a->coord() + b->coord()) * 0.5), _normal(gg::rotate_ccw(a->coord() - b->coord())), _length((a->coord() - b->coord()).norm()), _points({a, b}) {}

template <class B, class P> gg::Face<B, P>::Face(P *a, P *b, Intersection intersection, const B *boundary) :
    _center((a->coord() + b->coord()) * 0.5), _normal(gg::rotate_ccw(a->coord() - b->coord())), _length((a->coord() - b->coord()).norm()), _points({a, b}) {}

template <class B, class P> gg::Vector gg::Face<B, P>::center() const
{
    return _center;
}

template <class B, class P> gg::Vector gg::Face<B, P>::normal() const
{
    return _normal;
}

template <class B, class P> double gg::Face<B, P>::length() const
{
    return _length;
}

template <class B, class P> std::array<P*, 2> gg::Face<B, P>::points() const
{
    return _points;
}

template <class B, class P, class F> gg::Cell<B, P, F>::Cell(Vector center, double area) :
    _center(center), _area(area) {}

template <class B, class P, class F> gg::Cell<B, P, F>::Cell(Vector center, double area, Intersection intersection, const B *boundary) :
    _center(center), _area(area) {}

template <class B, class P, class F> gg::Vector gg::Cell<B, P, F>::center() const
{
    return _center;
}

template <class B, class P, class F> double gg::Cell<B, P, F>::area() const
{
    return _area;
}

template <class B, class P, class F> std::vector<typename gg::Cell<B, P, F>::Side> &gg::Cell<B, P, F>::sides()
{
    return _sides;
}

template <class B, class P, class F, class C> gg::CellGrid<B, P, F, C>::CellGrid(const CellGridParameters &parameters, const std::vector<B> &boundaries)
{
    //STAGE 0: declare sets and variables
    const double area = get_area(parameters);
    std::map<Position, TemporaryCell<B, P, F, C>> active, passive;

    //STAGE 1: add first cell
    {
        TemporaryCell<B, P, F, C> new_cell(parameters);
        new_cell.points[0].status = PointStatus::active;
        active.insert({Position(), new_cell});
        const std::array<PointPosition, 6> neighbors = get_point_neighbors(parameters, { Position(), 0 });
        for (std::array<PointPosition, 6>::const_iterator neighbor = neighbors.begin(); neighbor != neighbors.end() && neighbor->point < 100; neighbor++)
        {
            new_cell = TemporaryCell<B, P, F, C>(parameters);
            new_cell.points[neighbor->point].status = PointStatus::active;
            active.insert({neighbor->position, new_cell});
        }
    }

    //STAGE 2: add all cells
    while (!active.empty())
    {
        //Iterate through active, probe unprobed faces, add new cells to to_be_active
        std::map<Position, TemporaryCell<B, P, F, C>> to_be_active;
        for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = active.begin(); cell != active.end(); cell++)
        {
            //For each point
            std::array<Vector, 6> points = get_points(parameters, cell->first);
            for (unsigned int p = 0; p < get_shape(parameters); p++)
            {
                if (cell->second.points[p].status != PointStatus::active) continue;

                //Trying to probe counterclockwise
                const unsigned int next_ccw = ((p == (get_shape(parameters) - 1)) ? 0 : (p + 1));
                if (!cell->second.faces[p].probed)
                {
                    Intersection intersection;
                    const B* pboundary;
                    for (typename std::vector<B>::const_iterator boundary = boundaries.begin(); boundary != boundaries.end(); boundary++)
                    {
                        Intersection new_intersection = boundary->figure()->intersection(points[p], points[next_ccw]);
                        if (new_intersection.valid && (!intersection.valid || ((new_intersection.coord-points[p]).squared_norm() < (intersection.coord-points[p]).squared_norm())))
                        {
                            intersection = new_intersection;
                            pboundary = &(*boundary);
                        }
                    }
                    const FacePosition neighbor = get_face_neighbor(parameters, { cell->first, p });
                    typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator find = active.find(neighbor.position);
                    cell->second.faces[p].probed = true;
                    find->second.faces[neighbor.face].probed = true;
                    if (intersection.valid)
                    {
                        cell->second.faces[p].intersection  = find->second.faces[neighbor.face].intersection= intersection;
                        cell->second.faces[p].boundary      = find->second.faces[neighbor.face].boundary    = pboundary;
                        cell->second.intersection           = find->second.intersection                     = intersection;
                        cell->second.boundary               = find->second.boundary                         = pboundary;
                    }
                }
                if (!cell->second.faces[p].intersection.valid && cell->second.points[next_ccw].status == PointStatus::unreached)
                {
                    cell->second.points[next_ccw].status = PointStatus::to_be_active;
                    const std::array<PointPosition, 6> neighbors = get_point_neighbors(parameters, { cell->first, next_ccw });
                    for (std::array<PointPosition, 6>::const_iterator neighbor = neighbors.begin(); neighbor != neighbors.end() && neighbor->point < 100; neighbor++)
                    {
                        if (to_be_active.find(neighbor->position) != to_be_active.end()) //to_be_active points remain to_be_active
                        {
                            to_be_active.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                        }
                        else if (active.find(neighbor->position) != active.end()) //Active points remain active
                        {
                            active.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                        }
                        else if (passive.find(neighbor->position) != passive.end()) //Passive points become to_be_active
                        {
                            passive.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                            to_be_active.insert(*passive.find(neighbor->position));
                            passive.erase(neighbor->position);
                        }
                        else //Create to_be_active point
                        {
                            TemporaryCell<B, P, F, C> new_cell(parameters);
                            new_cell.points[neighbor->point].status = PointStatus::to_be_active;
                            to_be_active.insert({neighbor->position, new_cell});
                        }
                    }
                }

                //Trying to probe clockwise
                unsigned int next_cw = ((p == 0) ? (get_shape(parameters) - 1) : (p - 1));
                if (!cell->second.faces[next_cw].probed)
                {
                    Intersection intersection;
                    const B* pboundary;
                    for (typename std::vector<B>::const_iterator boundary = boundaries.begin(); boundary != boundaries.end(); boundary++)
                    {
                        Intersection new_intersection = boundary->figure()->intersection(points[p], points[next_cw]);
                        if (new_intersection.valid && (!intersection.valid || ((new_intersection.coord-points[p]).squared_norm() < (intersection.coord-points[p]).squared_norm())))
                        {
                            intersection = new_intersection;
                            pboundary = &(*boundary);
                        }
                    }
                    const FacePosition neighbor = get_face_neighbor(parameters, { cell->first, next_cw });
                    typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator find = active.find(neighbor.position);
                    cell->second.faces[next_cw].probed = true;
                    find->second.faces[neighbor.face].probed = true;
                    if (intersection.valid)
                    {
                        cell->second.faces[next_cw].intersection    = find->second.faces[neighbor.face].intersection= intersection;
                        cell->second.faces[next_cw].boundary        = find->second.faces[neighbor.face].boundary    = pboundary;
                        cell->second.intersection                   = find->second.intersection                     = intersection;
                        cell->second.boundary                       = find->second.boundary                         = pboundary;
                    }
                }
                if (!cell->second.faces[next_cw].intersection.valid && cell->second.points[next_cw].status == PointStatus::unreached)
                {
                    cell->second.points[next_cw].status = PointStatus::to_be_active;
                    const std::array<PointPosition, 6> neighbors = get_point_neighbors(parameters, { cell->first, next_cw });
                    for (std::array<PointPosition, 6>::const_iterator neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++)
                    {
                        if (to_be_active.find(neighbor->position) != to_be_active.end()) //to_be_active points remain to_be_active
                        {
                            to_be_active.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                        }
                        else if (active.find(neighbor->position) != active.end()) //Active points remain active
                        {
                            active.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                        }
                        else if (passive.find(neighbor->position) != passive.end()) //Passive points become to_be_active
                        {
                            passive.find(neighbor->position)->second.points[neighbor->point].status = PointStatus::to_be_active;
                            to_be_active.insert(*passive.find(neighbor->position));
                            passive.erase(neighbor->position);
                        }
                        else //Create to_be_active point
                        {
                            TemporaryCell<B, P, F, C> new_cell(parameters);
                            new_cell.points[neighbor->point].status = PointStatus::to_be_active;
                            to_be_active.insert({neighbor->position, new_cell});
                        }
                    }
                }
            }
        }

        //Add fresh active cells
        active.insert(to_be_active.begin(), to_be_active.end());

        //Make to_be_active points active, make active points passive, divide active into new_active and passive
        std::map<Position, TemporaryCell<B, P, F, C>> new_active;
        for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = active.begin(); cell != active.end(); cell++)
        {
            bool active = false;
            for (unsigned int p = 0; p < get_shape(parameters); p++)
            {
                if (cell->second.points[p].status == PointStatus::to_be_active)
                {
                    cell->second.points[p].status = PointStatus::active;
                    active = true;
                }
                else if (cell->second.points[p].status == PointStatus::active)
                {
                    cell->second.points[p].status = PointStatus::passive;
                }
            }
            if (active) new_active.insert(*cell);
            else passive.insert(*cell);
        }
        active = new_active;
    }

    //STAGE 3: calculate area
    for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = passive.begin(); cell != passive.end(); cell++)
    {
        bool complete = true;
        for (unsigned int p = 0; p < get_shape(parameters); p++)
        {
            if (cell->second.points[p].status == PointStatus::passive && !cell->second.faces[p].intersection.valid) { complete = false; break; }
        }
        if (complete)
        {
            //Complete cell, no calculations needed
            cell->second.complete = true;
            cell->second.area = area;
            cell->second.center = get_center(parameters, cell->first);
        }
        else
        {
            //Create list of points
            std::array<Vector, 6> points = get_points(parameters, cell->first);
            std::array<Vector, 12> point_list;
            unsigned int point_list_size = 0;
            for (unsigned int p = 0; p < get_shape(parameters); p++)
            {
                unsigned int next_ccw = ((p == (get_shape(parameters) - 1)) ? 0 : (p + 1));
                if (cell->second.points[p].status == PointStatus::passive) point_list[point_list_size++] = points[p];
                if (cell->second.points[p].status != cell->second.points[next_ccw].status) point_list[point_list_size++] = cell->second.faces[p].intersection.coord;
            }

            //Calculate area
            cell->second.area = 0;
            cell->second.center = Vector(0, 0);
            for (unsigned int p = 1; p < (point_list_size - 1); p++)
            {
                double a = (point_list[p] - point_list[0]).norm();
                double b = (point_list[p+1] - point_list[0]).norm();
                double c = (point_list[p+1] - point_list[p]).norm();
                double s = 0.5 * (a + b + c);
                double local_area = sqrt(s * (s - a) * (s - b) * (s - c));
                Vector local_center = (point_list[0] + point_list[p] + point_list[p+1]) / 3;
                cell->second.area = cell->second.area + local_area;
                cell->second.center = cell->second.center + (local_center * local_area);
            }
            cell->second.center = cell->second.center / cell->second.area;
            if (parameters.threshold_area <= 0.0) cell->second.complete = true;
            else if (parameters.threshold_area >= 1.0) cell->second.complete = false;
            else cell->second.complete = (cell->second.area > (parameters.threshold_area * area));
        }
    }

    //STAGE 4: apply failed cells
    for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = passive.begin(); cell != passive.end(); cell++)
    {
        if (!cell->second.complete)
        {
            for (unsigned int f = 0; f < get_shape(parameters); f++)
            {
                const FacePosition neighbor = get_face_neighbor(parameters, { cell->first, f });
                typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator find = passive.find(neighbor.position);
                if (find != passive.end() && find->second.complete)
                {
                    find->second.intersection = cell->second.intersection;
                    find->second.boundary = cell->second.boundary;
                }
            }
        }
    }

    //STAGE 5: create cells
    for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = passive.begin(); cell != passive.end(); cell++)
    {
        if (!cell->second.complete) continue;

        if (cell->second.boundary == nullptr) _cells.insert(cell->second.cell = new C(cell->second.center, cell->second.area));
        else _cells.insert(cell->second.cell = new C(cell->second.center, cell->second.area, cell->second.intersection, cell->second.boundary));
    }

    //STAGE 6: create points
    for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = passive.begin(); cell != passive.end(); cell++)
    {
        if (cell->second.cell == nullptr) continue;

        //For each point
        std::array<Vector, 6> points = get_points(parameters, cell->first);
        for (unsigned int p = 0; p < get_shape(parameters); p++)
        {
            //Regular points
            if (cell->second.points[p].status == PointStatus::passive)
            {
                if (cell->second.points[p].point == nullptr)
                {
                    _points.insert(cell->second.points[p].point = new P(points[p]));
                    const std::array<PointPosition, 6> neighbors = get_point_neighbors(parameters, { cell->first, p });
                    for (typename std::array<PointPosition, 6>::const_iterator neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++)
                        passive.find(neighbor->position)->second.points[neighbor->point].point = cell->second.points[p].point;
                }
                cell->second.cell->sides().push_back({ cell->second.points[p].point, nullptr, nullptr, false });
            }

            //Points on faces
            unsigned int next_ccw = ((p == (get_shape(parameters) - 1)) ? 0 : (p + 1));
            if (cell->second.points[p].status != cell->second.points[next_ccw].status) //same as cell->second.points[p].intersection.valid ?
            {
                if (cell->second.faces[p].point == nullptr)
                {
                    _points.insert(cell->second.faces[p].point = new P(cell->second.faces[p].intersection, cell->second.faces[p].boundary));
                    const FacePosition neighbor = get_face_neighbor(parameters, { cell->first, p });
                    passive.find(neighbor.position)->second.faces[neighbor.face].point = cell->second.faces[p].point;
                }
                cell->second.cell->sides().push_back({ cell->second.faces[p].point, nullptr, nullptr, false });
            }
        }
    }

    //STAGE 7: create faces
    for (typename std::map<Position, TemporaryCell<B, P, F, C>>::iterator cell = passive.begin(); cell != passive.end(); cell++)
    {
        if (cell->second.cell == nullptr) continue;

        //Creating faces
        P *irregular_face_start = nullptr;
        unsigned int side_counter = 0;
        for (unsigned int p = 0; p < get_shape(parameters); p++)
        {
            unsigned int next_ccw = ((p == (get_shape(parameters) - 1)) ? 0 : (p + 1));
            if (cell->second.points[p].status == PointStatus::passive || cell->second.points[next_ccw].status == PointStatus::passive)
            {
                //At least one of the points is passive, the face should exist
                const FacePosition neighbor = get_face_neighbor(parameters, { cell->first, p });
                if (cell->second.faces[p].face == nullptr)
                {
                    //The face doesn't exist and needs to be created
                    F *face;
                    if (cell->second.points[p].status == PointStatus::passive && cell->second.points[next_ccw].status != PointStatus::passive)
                    {
                        //First point is normal point, second point is face point
                        face = new F(cell->second.points[p].point, cell->second.faces[p].point, cell->second.faces[p].intersection, cell->second.faces[p].boundary);
                    }
                    else if (cell->second.points[p].status != PointStatus::passive && cell->second.points[next_ccw].status == PointStatus::passive)
                    {
                        //First point is face point, second point is normal point
                        face = new F(cell->second.points[next_ccw].point, cell->second.faces[p].point, cell->second.faces[p].intersection, cell->second.faces[p].boundary);
                    }
                    else
                    {
                        //Normal regular face
                        face = new F(cell->second.points[p].point, cell->second.points[next_ccw].point);
                    }
                    _faces.insert(face);
                    cell->second.faces[p].face = face;
                    passive.find(neighbor.position)->second.faces[neighbor.face].face = face;;
                }

                //Now when regular faces are created, one can decide what to do with it
                if (cell->second.points[p].status != PointStatus::passive && cell->second.points[next_ccw].status == PointStatus::passive)
                {
                    //First point is face point, second point is normal point -> Close irregular face and add normal face
                    if (irregular_face_start != nullptr)
                    {
                        F *irregular_face = new F(irregular_face_start, cell->second.faces[p].point, cell->second.faces[p].intersection, cell->second.faces[p].boundary);
                        _faces.insert(irregular_face);
                        cell->second.cell->sides()[side_counter].face = irregular_face;
                        side_counter++;
                    }
                    cell->second.cell->sides()[side_counter].face = cell->second.faces[p].face;
                    cell->second.cell->sides()[side_counter].cell = passive.find(neighbor.position)->second.cell;
                    side_counter++;
                }
                else if (cell->second.points[p].status == PointStatus::passive && cell->second.points[next_ccw].status != PointStatus::passive)
                {
                    //First point is normal point, second point is face point -> Add normal face and open irregular face
                    cell->second.cell->sides()[side_counter].face = cell->second.faces[p].face;
                    cell->second.cell->sides()[side_counter].cell = passive.find(neighbor.position)->second.cell;
                    side_counter++;

                    irregular_face_start = cell->second.faces[p].point;
                    irregular_face_start = nullptr;
                }
                else
                {
                    //Normal regular face -> just add it
                    cell->second.cell->sides()[side_counter].face = cell->second.faces[p].face;
                    cell->second.cell->sides()[side_counter].cell = passive.find(neighbor.position)->second.cell;
                    side_counter++;
                }
            }
        }
        //Closing open irregular face
        if (irregular_face_start != nullptr)
        {
            for (unsigned int p = 0; p < get_shape(parameters); p++)
            {
                if (cell->second.faces[p].point != nullptr)
                {
                    F *irregular_face = new F(irregular_face_start, cell->second.faces[p].point, cell->second.faces[p].intersection, cell->second.faces[p].boundary);
                    _faces.insert(irregular_face);
                    cell->second.cell->sides()[side_counter].face = irregular_face;
                    break;
                }
            }
        }
    }

    //STAGE 8: calculating if faces are flipped
    for (typename std::set<C*>::iterator icell = _cells.begin(); icell != _cells.end(); icell++)
    {
        C *cell = *icell;
        for (unsigned int f = 0; f < cell->sides().size(); f++) cell->sides()[f].inwards = ((cell->center() - cell->sides()[f].face->center()).dot(cell->sides()[f].face->normal()) >= 0.0);
    }
}

template <class B, class P, class F, class C>std::set<P*> &gg::CellGrid<B, P, F, C>::points()
{
    return _points;
}

template <class B, class P, class F, class C>std::set<F*> &gg::CellGrid<B, P, F, C>::faces()
{
    return _faces;
}

template <class B, class P, class F, class C>std::set<C*> &gg::CellGrid<B, P, F, C>::cells()
{
    return _cells;
}
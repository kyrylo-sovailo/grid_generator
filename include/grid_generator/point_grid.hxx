/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/

#pragma once
#include "point_grid.h"
#include "common_internal.h"
#include <map>

/*
    Points are divided into active, passive and unreached
    Unreached points have not yet been reached by the algorithm
    Active and passive points are both "reached" points, they are divided in two categories for optimization reasons
    The algorithm actively searches around "active" point, but does not search around "passive" points

    Because adding/removing elements from set cannot be done while iterating through it, there are sets like "to_be_active", etc.
    
    In this version of the algorithm active points with guaranty become passive after one iteration, so no checks are done
*/

namespace gg
{
    template <class B, class P>
    struct TemporaryStandalonePoint
    {
        P *point = nullptr;
        Intersection intersection;
        const B *boundary = nullptr;
    };
}

template <class B> gg::StandalonePoint<B>::StandalonePoint(Vector coord) : _coord(coord), _normal(Vector(0,0)) {}

template <class B> gg::StandalonePoint<B>::StandalonePoint(Vector coord, Intersection intersection, const B *boundary) : _coord(coord), _normal(intersection.normal) {}

template <class B> gg::Vector gg::StandalonePoint<B>::coord() const
{
    return _coord;
}

template <class B> gg::Vector gg::StandalonePoint<B>::normal() const
{
    return _normal;
}

template <class B> std::vector<gg::StandalonePoint<B>*> &gg::StandalonePoint<B>::neighbors()
{
    return _neighbors;
}

template <class B, class P> gg::PointGrid<B, P>::PointGrid(const PointGridParameters &parameters, const std::vector<B> &boundaries)
{
    //STAGE 0: declare sets
    std::map<Position, TemporaryStandalonePoint<B, P>> active, passive;

    //STAGE 1: add first point
    active.insert({ Position(), TemporaryStandalonePoint<B, P>() });

    //STAGE 2: add all points
    while (!active.empty())
    {
        //Iterate through active, add new points to "to_be_active"
        std::map<Position, TemporaryStandalonePoint<B, P>> to_be_active;
        for (typename std::map<Position, TemporaryStandalonePoint<B, P>>::iterator point = active.begin(); point != active.end(); point++)
        {
            const Vector active_coord = get_center(parameters, point->first);
            for (unsigned int f = 0; f < get_shape(parameters); f++)    //Look on every neighbor
            {
                const Position neighbor = get_face_neighbor(parameters, { point->first, f }).position;
                if (passive.find(neighbor) != passive.end()) continue;  //Already passive, skip
                if (active.find(neighbor) != active.end()) continue;    //Already active, skip
                const Vector to_be_active_coord = get_center(parameters, neighbor);
                
                Intersection intersection;
                const B *pboundary;
                for (typename std::vector<B>::const_iterator boundary = boundaries.begin(); boundary != boundaries.end(); boundary++)
                {
                    Intersection new_intersection = boundary->figure()->intersection(active_coord, to_be_active_coord);
                    if (new_intersection.valid && (!intersection.valid || ((new_intersection.coord-active_coord).squared_norm() < (intersection.coord-active_coord).squared_norm())))
                    {
                        intersection = new_intersection;
                        pboundary = &(*boundary);
                    }
                }
                if (intersection.valid) //Boundary found, remember conditions
                {
                    point->second.intersection = intersection;
                    point->second.boundary = pboundary;
                }
                else  //Boundary not found, create point
                {
                    if (to_be_active.find(neighbor) == to_be_active.end()) to_be_active.insert({neighbor, TemporaryStandalonePoint<B, P>()});
                }
            }
        }
        passive.insert(active.begin(), active.end());   //All active are now passive, no checks needed
        active = to_be_active;                          //All to_be_active are now active
    }

    //STAGE 3: create point objects
    for (typename std::map<Position, TemporaryStandalonePoint<B, P>>::iterator point = passive.begin(); point != passive.end(); point++)
    {
        if (point->second.boundary == nullptr)
        {
            _points.insert(point->second.point = new P(get_center(parameters, point->first)));
        }
        else
        {
            _points.insert(point->second.point = new P(get_center(parameters, point->first), point->second.intersection, point->second.boundary));
        }
    }

    //STAGE 4: interconnect points
    for (typename std::map<Position, TemporaryStandalonePoint<B, P>>::iterator point = passive.begin(); point != passive.end(); point++)
    {
        for (unsigned int f = 0; f < get_shape(parameters); f++)
        {
            const Position neighbor = get_face_neighbor(parameters, { point->first, f }).position;
            typename std::map<Position, TemporaryStandalonePoint<B, P>>::iterator find = passive.find(neighbor);
            if (find != passive.end()) point->second.point->neighbors().push_back(find->second.point);
        }
    }
}

template <class B, class P> std::set<P*> &gg::PointGrid<B, P>::points()
{
    return _points;
}
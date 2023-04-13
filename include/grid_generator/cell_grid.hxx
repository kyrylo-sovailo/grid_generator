/*
    Part of the GridGenerator Project. Distributed under MIT License, which means:
        - Do whatever you want
        - Keep this notice and include the license file to your project
        - I provide no warranty
    Created by Kyrylo Sovailo, github.com/kyrylo-sovailo, k.sovailo@gmail.com
*/ 

#pragma once
#include "cell_grid.h"

template <class B> gg::Point<B>::Point(Vector coord)
{}
template <class B> gg::Point<B>::Point(Vector coord, Intersection intersection, B boundary)
{}
template <class B> gg::Vector gg::Point<B>::coord() const
{}
template <class B> gg::Vector gg::Point<B>::normal() const
{}
template <class B> bool gg::Point<B>::boundary() const
{}

template <class B, class P> gg::Face<B, P>::Face(P *a, P *b)
{}
template <class B, class P> gg::Vector gg::Face<B, P>::center() const
{}

template <class B, class P> gg::Vector gg::Face<B, P>::normal() const
{}

template <class B, class P> double gg::Face<B, P>::length() const
{}

template <class B, class P> std::array<P*, 2> gg::Face<B, P>::points() const
{}

template <class B, class P, class F> gg::Vector gg::Cell<B, P, F>::center() const
{}
template <class B, class P, class F> double gg::Cell<B, P, F>::area() const
{}
template <class B, class P, class F> std::vector<typename gg::Cell<B, P, F>::Side> &gg::Cell<B, P, F>::sides()
{}

template <class B, class P, class F, class C> gg::CellGrid<B, P, F, C>::CellGrid(const CellGridParameters &parameters, const std::vector<B> &boundaries)
{}
template <class B, class P, class F, class C>std::set<P*> &gg::CellGrid<B, P, F, C>::points()
{}
template <class B, class P, class F, class C>std::set<F*> &gg::CellGrid<B, P, F, C>::faces()
{}
template <class B, class P, class F, class C>std::set<C*> &gg::CellGrid<B, P, F, C>::cells()
{}
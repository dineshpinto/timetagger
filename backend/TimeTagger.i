/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef WRAPPER_SWIG_I
#define WRAPPER_SWIG_I
%module(docstring="The TimeTagger module to measure timing event very accurate.") TimeTagger
%feature("autodoc", "4");
// first section
%{
#include "TimeTagger.h"
#include "Iterator.cpp"
#include "StartStop.cpp"
#include "TimeDifferences.cpp"
#include "SSRTimeTrace.cpp"
#include "CountBetweenMarkers.cpp"
#include "Counter.cpp"
#include "SingTag.cpp"

#include "numpy/arrayobject.h"
%}
// second section

%include "numpy.i"
%include "std_string.i"
%include "std_vector.i"

%template(IntVector) std::vector<int>;
%template(UIntVector) std::vector<unsigned int>;
%template(LongVector) std::vector<long long>;
%template(ULongVector) std::vector<unsigned long long>;

%init %{
	import_array();
%}

%include "TimeTagger.h"
%include "Iterator.cpp"
%include "StartStop.cpp"
%include "TimeDifferences.cpp"
%include "SSRTimeTrace.cpp"
%include "CountBetweenMarkers.cpp"
%include "Counter.cpp"
%include "SingTag.cpp"

#endif


from libcpp.string cimport string
from libcpp.vector cimport vector
from libc.stdint cimport uint32_t, int64_t
from libc.math cimport sqrt
from libcpp cimport bool

from ttcrpy.common cimport sxz, sxyz, siv, siv2, sijv, Node3Dc, Node3Dcsp, \
Node3Dn, Node3Dnsp, Cell, Node2Dcsp, Node2Dn, Node2Dnsp

cdef extern from "typedefs.h" namespace "ttcr":
    cdef cppclass node2d:
        pass
    cdef cppclass cell2d:
        pass
    cdef cppclass cell2dvti:
        pass
    cdef cppclass cell2dtti:
        pass

cdef extern from "Grid3D.h" namespace "ttcr" nogil:
    cdef cppclass Grid3D[T1,T2]:
        size_t getNthreads()
        void setSlowness(vector[T1]&) except +
        T1 computeSlowness(sxyz[T1]&) except +
        void getTT(vector[T1]& tt, size_t threadNo) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      size_t thread_no) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      vector[vector[sxyz[T1]]]& r_data,
                      size_t thread_no) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      vector[vector[siv[T1]]]& l_data,
                      size_t thread_no) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      vector[vector[sxyz[T1]]]& r_data,
                      vector[vector[siv[T1]]]& l_data,
                      size_t thread_no) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      vector[vector[sijv[T1]]]& m_data,
                      size_t thread_no) except +
        void raytrace(vector[sxyz[T1]]& Tx,
                      vector[T1]& t0,
                      vector[sxyz[T1]]& Rx,
                      vector[T1]& tt,
                      vector[vector[sxyz[T1]]]& r_data,
                      vector[vector[sijv[T1]]]& m_data,
                      size_t thread_no) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes,
                      vector[vector[vector[sxyz[T1]]]]& r_data) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes,
                      vector[vector[vector[sijv[T1]]]]& m_data) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes,
                      vector[vector[vector[sxyz[T1]]]]& r_data,
                      vector[vector[vector[sijv[T1]]]]& m_data) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes,
                      vector[vector[vector[siv[T1]]]]& l_data) except +
        void raytrace(vector[vector[sxyz[T1]]]& Tx,
                      vector[vector[T1]]& t0,
                      vector[vector[sxyz[T1]]]& Rx,
                      vector[vector[T1]]& traveltimes,
                      vector[vector[vector[sxyz[T1]]]]& r_data,
                      vector[vector[vector[siv[T1]]]]& l_data) except +


cdef extern from "Grid3Drn.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drn[T1,T2,N](Grid3D[T1,T2]):
        pass

cdef extern from "Grid3Drc.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drc[T1,T2,N](Grid3D[T1,T2]):
        pass

cdef extern from "Grid3Drnfs.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drnfs[T1,T2](Grid3Drn[T1,T2,Node3Dn[T1,T2]]):
        Grid3Drnfs(T2, T2, T2, T1, T1, T1, T1, T1, int, bool, bool, bool,
                   size_t) except +

cdef extern from "Grid3Drnsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drnsp[T1,T2](Grid3Drn[T1,T2,Node3Dnsp[T1,T2]]):
        Grid3Drnsp(T2, T2, T2, T1, T1, T1, T1, T1, T1, T2, T2, T2, bool, bool,
                   size_t) except +

cdef extern from "Grid3Drndsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drndsp[T1,T2](Grid3Drn[T1,T2,Node3Dn[T1,T2]]):
        Grid3Drndsp(T2, T2, T2, T1, T1, T1, T1, T1, T1, T2, bool, T2, T1, bool,
                    size_t) except +

cdef extern from "Grid3Drcfs.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drcfs[T1,T2](Grid3Drn[T1,T2,Node3Dn[T1,T2]]):
        Grid3Drcfs(T2, T2, T2, T1, T1, T1, T1, T1, int, bool, bool, bool,
                   size_t) except +

cdef extern from "Grid3Drcsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drcsp[T1,T2,CELL](Grid3Drc[T1,T2,Node3Dcsp[T1,T2]]):
        Grid3Drcsp(T2, T2, T2, T1, T1, T1, T1, T1, T1, T2, T2, T2, bool,
                   size_t) except +

cdef extern from "Grid3Drcdsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid3Drcdsp[T1,T2,CELL](Grid3Drc[T1,T2,Node3Dc[T1,T2]]):
        Grid3Drcdsp(T2, T2, T2, T1, T1, T1, T1, T1, T1, T2, bool, T2, T1,
                    size_t) except +


cdef extern from "Grid2D.h" namespace "ttcr" nogil:
    cdef cppclass Grid2D[T1,T2,S]:
        size_t getNthreads()
        void setSlowness(vector[T1]&) except +
        void raytrace(vector[S]& Tx,
                      vector[T1]& t0,
                      vector[S]& Rx,
                      vector[T1]& traveltimes,
                      size_t threadNo) except +
        void raytrace(vector[S]& Tx,
                      vector[T1]& t0,
                      vector[S]& Rx,
                      vector[T1]& traveltimes,
                      vector[vector[sxz[T1]]]& r_data,
                      size_t threadNo) except +
        void raytrace(vector[S]& Tx,
                      vector[T1]& t0,
                      vector[S]& Rx,
                      vector[T1]& traveltimes,
                      vector[vector[siv2[T1]]]& l_data,
                      size_t threadNo) except +
        void raytrace(vector[S]& Tx,
                      vector[T1]& t0,
                      vector[S]& Rx,
                      vector[T1]& traveltimes,
                      vector[vector[S]]& r_data,
                      vector[vector[siv2[T1]]]& l_data,
                      size_t threadNo) except +

cdef extern from "Grid2Drc.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drc[T1,T2,S,NODE,CELL](Grid2D[T1,T2,S]):
        pass

cdef extern from "Grid2Drn.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drn[T1,T2,S,NODE](Grid2D[T1,T2,S]):
        pass

cdef extern from "Grid2Drcsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drcsp[T1,T2,S,CELL](Grid2Drc[T1,T2,S,node2d,CELL]):
        Grid2Drcsp(T2, T2, T1, T1, T1, T1, T2, T2, size_t) except +

cdef extern from "Grid2Drcfs.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drcfs[T1,T2,S](Grid2Drn[T1,T2,S,Node2Dn[T1,T2]]):
        Grid2Drcfs(T2, T2, T1, T1, T1, T1, T1, int, bool, bool, size_t) except +

cdef extern from "Grid2Drnsp.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drnsp[T1,T2,S](Grid2Drn[T1,T2,S,node2d]):
        Grid2Drnsp(T2, T2, T1, T1, T1, T1, T2, T2, size_t) except +

cdef extern from "Grid2Drnfs.h" namespace "ttcr" nogil:
    cdef cppclass Grid2Drnfs[T1,T2,S](Grid2Drn[T1,T2,S,Node2Dn[T1,T2]]):
        Grid2Drnfs(T2, T2, T1, T1, T1, T1, T1, int, bool, bool, size_t) except +

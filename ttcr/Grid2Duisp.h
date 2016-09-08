//
//  Grid2Duisp.h
//  ttcr
//
//  Created by Bernard Giroux on 2014-04-11.
//  Copyright (c) 2014 Bernard Giroux. All rights reserved.
//

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ttcr_Grid2Duisp_h
#define ttcr_Grid2Duisp_h

#include <array>
#include <fstream>
#include <queue>
#include <set>

#include "Grid2Dui.h"

namespace ttcr {
    
    template<typename T1, typename T2, typename NODE, typename S>
    class Grid2Duisp : public Grid2Dui<T1,T2,NODE,S> {
    public:
        Grid2Duisp(const std::vector<S>& no,
                   const std::vector<triangleElem<T2>>& tri,
                   const T2 ns, const size_t nt=1) :
        Grid2Dui<T1,T2,NODE,S>(no, tri, nt), nsecondary(ns)
        {
            buildGridNodes(no, nt);
            this->buildGridNeighbors();
        }
        
        ~Grid2Duisp() {
        }
        
        int setSlowness(const std::vector<T1>& s) {
            if ( this->nPrimary != s.size() ) {
                std::cerr << "Error: slowness vectors of incompatible size.";
                return 1;
            }
            for ( size_t n=0; n<this->nPrimary; ++n ) {
                this->nodes[n].setNodeSlowness( s[n] );
            }
            if ( nsecondary>0 ) interpSlownessSecondary();
            return 0;
        }
        
        int setSlowness(const T1 *s, size_t ns) {
            if ( this->nPrimary != ns ) {
                std::cerr << "Error: slowness vectors of incompatible size.";
                return 1;
            }
            for ( size_t n=0; n<this->nPrimary; ++n ) {
                this->nodes[n].setNodeSlowness( s[n] );
            }
            if ( nsecondary>0 ) interpSlownessSecondary();
            return 0;
        }
        
        int raytrace(const std::vector<S>&,
                     const std::vector<T1>&,
                     const std::vector<S>&,
                     std::vector<T1>&,
                     const size_t=0) const;
        
        int raytrace(const std::vector<S>&,
                     const std::vector<T1>&,
                     const std::vector<const std::vector<S>*>&,
                     std::vector<std::vector<T1>*>&,
                     const size_t=0) const;
        
        int raytrace(const std::vector<S>&,
                     const std::vector<T1>& ,
                     const std::vector<S>&,
                     std::vector<T1>&,
                     std::vector<std::vector<S>>&,
                     const size_t=0) const;
        
        int raytrace(const std::vector<S>&,
                     const std::vector<T1>&,
                     const std::vector<const std::vector<S>*>&,
                     std::vector<std::vector<T1>*>&,
                     std::vector<std::vector<std::vector<S>>*>&,
                     const size_t=0) const;
        
        int raytrace(const std::vector<S>&,
                     const std::vector<T1>&,
                     const std::vector<S>&,
                     std::vector<T1>&,
                     std::vector<std::vector<S>>&,
                     std::vector<std::vector<siv<T1>>>&,
                     const size_t=0) const;

        int raytrace(const std::vector<S>&,
                     const std::vector<T1>&,
                     const std::vector<S>&,
                     std::vector<T1>&,
                     std::vector<std::vector<S>>&,
                     std::vector<std::vector<sijv<T1>>>&,
                     const size_t=0) const;

    private:
        T2 nsecondary;
        
        void buildGridNodes(const std::vector<S>&, const size_t);
        
        void interpSlownessSecondary();
        
        void initQueue(const std::vector<S>& Tx,
                       const std::vector<T1>& t0,
                       std::priority_queue<NODE*,
                       std::vector<NODE*>,
                       CompareNodePtr<T1>>& queue,
                       std::vector<NODE>& txNodes,
                       std::vector<bool>& inQueue,
                       std::vector<bool>& frozen,
                       const size_t threadNo) const;
        
        void propagate(std::priority_queue<NODE*,
                       std::vector<NODE*>,
                       CompareNodePtr<T1>>& queue,
                       std::vector<bool>& inQueue,
                       std::vector<bool>& frozen,
                       const size_t threadNo) const;
        
    };
    
    template<typename T1, typename T2, typename NODE, typename S>
    void Grid2Duisp<T1,T2,NODE,S>::buildGridNodes(const std::vector<S>& no,
                                                  const size_t nt) {
        
        // primary nodes
        for ( T2 n=0; n<no.size(); ++n ) {
            this->nodes[n].setXYZindex( no[n], n );
            this->nodes[n].setPrimary(5);
        }
        T2 nNodes = static_cast<T2>(this->nodes.size());
        
        std::map<std::array<T2,2>,std::vector<T2>> lineMap;
        std::array<T2,2> lineKey;
        typename std::map<std::array<T2,2>,std::vector<T2>>::iterator lineIt;
        
        size_t estLineNo = (this->triangles.size()+this->triangles.size()/10) * 3/2;
        this->nodes.reserve( nNodes + estLineNo*nsecondary );
        
        // edge nodes
        NODE tmpNode(nt);
        for ( T2 ntri=0; ntri<this->triangles.size(); ++ntri ) {
            
            for ( size_t nl=0; nl<3; ++nl ) {
                
                // push owner for primary nodes
                this->nodes[ this->triangles[ntri].i[nl] ].pushOwner( ntri );
                
                if ( nsecondary>0 ) {
                    
                    lineKey = { this->triangles[ntri].i[nl],
                        this->triangles[ntri].i[(nl+1)%3] };
                    std::sort(lineKey.begin(), lineKey.end());
                    
                    lineIt = lineMap.find( lineKey );
                    if ( lineIt == lineMap.end() ) {
                        // not found, insert new pair
                        lineMap[ lineKey ] = std::vector<T2>(nsecondary);
                    } else {
                        for ( size_t n=0; n<lineIt->second.size(); ++n ) {
                            // setting owners
                            this->nodes[ lineIt->second[n] ].pushOwner( ntri );
                        }
                        continue;
                    }
                    
                    S d = (no[lineKey[1]]-no[lineKey[0]])/static_cast<T1>(nsecondary+1);
                    
                    for ( size_t n2=0; n2<nsecondary; ++n2 ) {
                        tmpNode.setXYZindex(no[lineKey[0]]+static_cast<T1>(1+n2)*d,
                                            nNodes );
                        lineMap[lineKey][n2] = nNodes++;
                        this->nodes.push_back( tmpNode );
                        this->nodes.back().pushOwner( ntri );
                    }
                }
            }
        }
        
        this->nodes.shrink_to_fit();
    }
    
    
    template<typename T1, typename T2, typename NODE, typename S>
    void Grid2Duisp<T1,T2,NODE,S>::interpSlownessSecondary() {
        
        T2 nNodes = this->nPrimary;
        
        std::map<std::array<T2,2>,std::vector<T2>> lineMap;
        std::array<T2,2> lineKey;
        typename std::map<std::array<T2,2>,std::vector<T2>>::iterator lineIt;
        
        for ( T2 ntri=0; ntri<this->triangles.size(); ++ntri ) {
            
            for ( size_t nl=0; nl<3; ++nl ) {
                
                lineKey = { this->triangles[ntri].i[nl],
                    this->triangles[ntri].i[(nl+1)%3] };
                std::sort(lineKey.begin(), lineKey.end());
                
                lineIt = lineMap.find( lineKey );
                if ( lineIt == lineMap.end() ) {
                    // not found, insert new pair
                    lineMap[ lineKey ] = std::vector<T2>(nsecondary);
                } else {
                    // already done
                    continue;
                }
                
                T1 slope = (this->nodes[lineKey[1]].getNodeSlowness() - this->nodes[lineKey[0]].getNodeSlowness())/
                this->nodes[lineKey[1]].getDistance(this->nodes[lineKey[0]]);
                
                for ( size_t n2=0; n2<nsecondary; ++n2 ) {
                    
                    T1 s = this->nodes[lineKey[0]].getNodeSlowness() + slope * this->nodes[nNodes].getDistance(this->nodes[lineKey[0]]);
                    this->nodes[nNodes].setNodeSlowness( s );
                    lineMap[lineKey][n2] = nNodes++;
                    
                }
            }
        }
    }
    
    
    
    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<S>& Rx,
                                           std::vector<T1>& traveltimes,
                                           const size_t threadNo) const {
        
        if ( this->checkPts(Tx) == 1 ) return 1;
        if ( this->checkPts(Rx) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        
        for (size_t n=0; n<Rx.size(); ++n) {
            traveltimes[n] = this->getTraveltime(Rx[n], this->nodes, threadNo);
        }
        return 0;
    }
    
    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<const std::vector<S>*>& Rx,
                                           std::vector<std::vector<T1>*>& traveltimes,
                                           const size_t threadNo) const {
        
        if ( this->checkPts(Tx) == 1 ) return 1;
        for ( size_t n=0; n<Rx.size(); ++n )
            if ( this->checkPts(*Rx[n]) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        
        for (size_t nr=0; nr<Rx.size(); ++nr) {
            traveltimes[nr]->resize( Rx[nr]->size() );
            for (size_t n=0; n<Rx[nr]->size(); ++n)
                (*traveltimes[nr])[n] = this->getTraveltime((*Rx[nr])[n],
                                                            this->nodes, threadNo);
        }
        return 0;
    }
    
    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<S>& Rx,
                                           std::vector<T1>& traveltimes,
                                           std::vector<std::vector<S>>& r_data,
                                           const size_t threadNo) const {
        
        //	std::cout << "   running in thread no " << threadNo << std::endl;
        if ( this->checkPts(Tx) == 1 ) return 1;
        if ( this->checkPts(Rx) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<r_data.size(); ++ni ) {
            r_data[ni].resize( 0 );
        }
        T2 nodeParentRx;
        T2 cellParentRx;
        
        for (size_t n=0; n<Rx.size(); ++n) {
            
            traveltimes[n] = this->getTraveltime(Rx[n], this->nodes, nodeParentRx,
                                                 cellParentRx, threadNo);
            
            bool flag=false;
            for ( size_t ns=0; ns<Tx.size(); ++ns ) {
                if ( Rx[n] == Tx[ns] ) {
                    
                    r_data[n].resize( 1 );
                    r_data[n][0] = Rx[n];
                    
                    flag = true;
                    break;
                }
            }
            if ( flag ) continue;
            
            // Rx are in nodes (not txNodes)
            std::vector<NODE> *node_p;
            node_p = &(this->nodes);
            
            std::vector<S> r_tmp;
            T2 iChild, iParent = nodeParentRx;
            S child;
            
            // store the son's coord
            child = Rx[n];
            while ( (*node_p)[iParent].getNodeParent(threadNo) !=
                   std::numeric_limits<T2>::max() ) {
                
                r_tmp.push_back( child );
                
                // we now go up in time - parent becomes the child of grand'pa
                iChild = iParent;
                child = (*node_p)[iChild];
                
                // grand'pa is now papa
                iParent = (*node_p)[iChild].getNodeParent(threadNo);
                if ( iParent >= this->nodes.size() ) {
                    node_p = &txNodes;
                    iParent -= this->nodes.size();
                }
                else {
                    node_p = &(this->nodes);
                }
            }
            
            // parent is now at Tx
            r_tmp.push_back( child );
            
            // finally, store Tx position
            child = (*node_p)[iParent];
            r_tmp.push_back( child );
            
            // the order should be from Tx to Rx, so we reorder...
            iParent = static_cast<T2>(r_tmp.size());
            r_data[n].resize( r_tmp.size() );
            for ( size_t nn=0; nn<r_data[n].size(); ++nn ) {
                r_data[n][nn] = r_tmp[ iParent-1-nn ];
            }
        }
        return 0;
    }
    
    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<const std::vector<S>*>& Rx,
                                           std::vector<std::vector<T1>*>& traveltimes,
                                           std::vector<std::vector<std::vector<S>>*>& r_data,
                                           const size_t threadNo) const {
        
        if ( this->checkPts(Tx) == 1 ) return 1;
        for ( size_t n=0; n<Rx.size(); ++n )
            if ( this->checkPts(*Rx[n]) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        
        for (size_t nr=0; nr<Rx.size(); ++nr) {
            
            traveltimes[nr]->resize( Rx[nr]->size() );
            r_data[nr]->resize( Rx[nr]->size() );
            for ( size_t ni=0; ni<r_data[nr]->size(); ++ni ) {
                (*r_data[nr])[ni].resize( 0 );
            }
            
            T2 nodeParentRx;
            T2 cellParentRx;
            
            for (size_t n=0; n<Rx[nr]->size(); ++n) {
                
                (*traveltimes[nr])[n] = this->getTraveltime((*Rx[nr])[n], this->nodes,
                                                            nodeParentRx, cellParentRx,
                                                            threadNo);
                
                bool flag=false;
                for ( size_t ns=0; ns<Tx.size(); ++ns ) {
                    if ( (*Rx[nr])[n] == Tx[ns] ) {
                        
                        (*r_data[nr])[n].resize( 1 );
                        (*r_data[nr])[n][0] = (*Rx[nr])[n];
                        
                        flag = true;
                        break;
                    }
                }
                if ( flag ) continue;
                
                // Rx are in nodes (not txNodes)
                std::vector<NODE> *node_p;
                node_p = &(this->nodes);
                
                std::vector<S> r_tmp;
                T2 iChild, iParent = nodeParentRx;
                S child;
                
                // store the son's coord
                child = (*Rx[nr])[n];
                while ( (*node_p)[iParent].getNodeParent(threadNo) !=
                       std::numeric_limits<T2>::max() ) {
                    
                    r_tmp.push_back( child );
                    
                    // we now go up in time - parent becomes the child of grand'pa
                    iChild = iParent;
                    child = (*node_p)[iChild];
                    
                    // grand'pa is now papa
                    iParent = (*node_p)[iChild].getNodeParent(threadNo);
                    if ( iParent >= this->nodes.size() ) {
                        node_p = &txNodes;
                        iParent -= this->nodes.size();
                    }
                    else {
                        node_p = &(this->nodes);
                    }
                }
                
                // parent is now at Tx
                r_tmp.push_back( child );
                
                // finally, store Tx position
                child = (*node_p)[iParent];
                r_tmp.push_back( child );
                
                // the order should be from Tx to Rx, so we reorder...
                iParent = static_cast<T2>(r_tmp.size());
                (*r_data[nr])[n].resize( r_tmp.size() );
                for ( size_t nn=0; nn<(*r_data[nr])[n].size(); ++nn ) {
                    (*r_data[nr])[n][nn] = r_tmp[ iParent-1-nn ];
                }
            }
        }
        return 0;
    }
    
    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<S>& Rx,
                                           std::vector<T1>& traveltimes,
                                           std::vector<std::vector<S>>& r_data,
                                           std::vector<std::vector<siv<T1>>>& l_data,
                                           const size_t threadNo) const {
        
        //	std::cout << "   running in thread no " << threadNo << std::endl;
        if ( this->checkPts(Tx) == 1 ) return 1;
        if ( this->checkPts(Rx) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        if ( l_data.size() != Rx.size() ) {
            l_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<l_data.size(); ++ni ) {
            l_data[ni].resize( 0 );
        }
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<r_data.size(); ++ni ) {
            r_data[ni].resize( 0 );
        }
        T2 nodeParentRx;
        T2 cellParentRx;
        
        for (size_t n=0; n<Rx.size(); ++n) {
            
            traveltimes[n] = this->getTraveltime(Rx[n], this->nodes, nodeParentRx, cellParentRx,
                                                 threadNo);
            
            bool flag=false;
            for ( size_t ns=0; ns<Tx.size(); ++ns ) {
                if ( Rx[n] == Tx[ns] ) {
                    
                    r_data[n].resize( 1 );
                    r_data[n][0] = Rx[n];
                    
                    // no need to update l_data: ray length is zero
                    
                    flag = true;
                }
            }
            if ( flag ) continue;
            
            // Rx are in nodes (not txNodes)
            std::vector<NODE> *node_p;
            node_p = &(this->nodes);
            
            std::vector<S> r_tmp;
            T2 iChild, iParent = nodeParentRx;
            S child;
            siv<T1> cell;
            
            // store the son's coord
            child = Rx[n];
            cell.i = cellParentRx;
            while ( (*node_p)[iParent].getNodeParent(threadNo) !=
                   std::numeric_limits<T2>::max() ) {
                
                r_tmp.push_back( child );
                
                cell.v = (*node_p)[iParent].getDistance( child );
                bool found=false;
                for (size_t nc=0; nc<l_data[n].size(); ++nc) {
                    if ( l_data[n][nc].i == cell.i ) {
                        l_data[n][nc].v += cell.v;  // must add in case we pass through secondary nodes along edge
                        found = true;
                        break;
                    }
                }
                if ( found == false ) {
                    l_data[n].push_back( cell );
                }
                
                // we now go up in time - parent becomes the child of grand'pa
                iChild = iParent;
                child = (*node_p)[iChild];
                cell.i = (*node_p)[iChild].getCellParent(threadNo);
                
                // grand'pa is now papa
                iParent = (*node_p)[iChild].getNodeParent(threadNo);
                if ( iParent >= this->nodes.size() ) {
                    node_p = &txNodes;
                    iParent -= this->nodes.size();
                }
                else {
                    node_p = &(this->nodes);
                }
            }
            
            // parent is now at Tx
            r_tmp.push_back( child );
            
            cell.v = (*node_p)[iParent].getDistance( child );
            bool found=false;
            for (size_t nc=0; nc<l_data[n].size(); ++nc) {
                if ( l_data[n][nc].i == cell.i ) {
                    l_data[n][nc].v += cell.v;  // must add in case we pass through secondary nodes along edge
                    found = true;
                    break;
                }
            }
            if ( found == false ) {
                l_data[n].push_back( cell );
            }
            
            // finally, store Tx position
            child = (*node_p)[iParent];
            r_tmp.push_back( child );
            
            //  must be sorted to build matrix L
            sort(l_data[n].begin(), l_data[n].end(), CompareSiv_i<T1>());
            
            // the order should be from Tx to Rx, so we reorder...
            iParent = static_cast<T2>(r_tmp.size());
            r_data[n].resize( r_tmp.size() );
            for ( size_t nn=0; nn<r_data[n].size(); ++nn ) {
                r_data[n][nn] = r_tmp[ iParent-1-nn ];
            }
        }
        return 0;
    }

    template<typename T1, typename T2, typename NODE, typename S>
    int Grid2Duisp<T1,T2,NODE,S>::raytrace(const std::vector<S>& Tx,
                                           const std::vector<T1>& t0,
                                           const std::vector<S>& Rx,
                                           std::vector<T1>& traveltimes,
                                           std::vector<std::vector<S>>& r_data,
                                           std::vector<std::vector<sijv<T1>>>& m_data,
                                           const size_t threadNo) const {
        
        //	std::cout << "   running in thread no " << threadNo << std::endl;
        if ( this->checkPts(Tx) == 1 ) return 1;
        if ( this->checkPts(Rx) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        CompareNodePtr<T1> cmp(threadNo);
        std::priority_queue< NODE*, std::vector<NODE*>,
        CompareNodePtr<T1>> queue( cmp );
        
        std::vector<NODE> txNodes;
        std::vector<bool> inQueue( this->nodes.size(), false );
        std::vector<bool> frozen( this->nodes.size(), false );
        
        initQueue(Tx, t0, queue, txNodes, inQueue, frozen, threadNo);
        
        propagate(queue, inQueue, frozen, threadNo);
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        if ( m_data.size() != Rx.size() ) {
            m_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<m_data.size(); ++ni ) {
            m_data[ni].resize( 0 );
        }
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<r_data.size(); ++ni ) {
            r_data[ni].resize( 0 );
        }
        T2 nodeParentRx;
        T2 cellParentRx;
        sijv<T1> m;
        S mid_pt;
        
        //std::cout << "\nTx: " << Tx[0].x << ' ' << Tx[0].y << ' ' << Tx[0].z << std::endl;
        for (size_t n=0; n<Rx.size(); ++n) {
            //std::cout << "  Rx: " << Rx[n].x << ' ' << Rx[n].y << ' ' << Rx[n].z << std::endl;
            m.i = n;
            
            traveltimes[n] = this->getTraveltime(Rx[n], this->nodes, nodeParentRx, cellParentRx,
                                                 threadNo);
            
            bool flag=false;
            for ( size_t ns=0; ns<Tx.size(); ++ns ) {
                if ( Rx[n] == Tx[ns] ) {
                    
                    r_data[n].resize( 1 );
                    r_data[n][0] = Rx[n];
                    
                    // no need to update l_data: ray length is zero
                    
                    flag = true;
                }
            }
            if ( flag ) continue;
            
            // Rx are in nodes (not txNodes)
            std::vector<NODE> *node_p;
            node_p = &(this->nodes);
            
            std::vector<S> r_tmp;
            T2 iChild, iParent = nodeParentRx;
            S child;
            
            // store the son's coord
            child = Rx[n];
            while ( (*node_p)[iParent].getNodeParent(threadNo) !=
                   std::numeric_limits<T2>::max() ) {
                
                r_tmp.push_back( child );
				
				if (r_tmp.size() > 1 ) {
					// compute terms of matrix M
					mid_pt = static_cast<T1>(0.5)*((*node_p)[iParent] + child );
                    //std::cout << "    mid_pt = " << mid_pt.x << ' ' << mid_pt.y << ' ' << mid_pt.z << "    - " << iParent << ' ' << iChild << std::endl;
                    T1 v = 2./((*node_p)[iParent].getNodeSlowness() + (*node_p)[iChild].getNodeSlowness());
					v *= v;  // square
					T1 ds = (*node_p)[iParent].getDistance( child );
					
					std::set<T2> neib;
					std::vector<T2> cells;
					for ( size_t nc1=0; nc1<(*node_p)[iParent].getOwners().size(); ++nc1 ) {
						for ( size_t nc2=0; nc2<(*node_p)[iChild].getOwners().size(); ++nc2 ) {
							if ( (*node_p)[iParent].getOwners()[nc1] == (*node_p)[iChild].getOwners()[nc2] ) {
								cells.push_back( (*node_p)[iParent].getOwners()[nc1] );
							}
						}
					}
					
					for ( size_t nc=0; nc<cells.size(); ++nc ) {
						for ( size_t nn=0; nn<this->neighbors[cells[nc]].size(); ++nn ) {
							if ( (*node_p)[this->neighbors[cells[nc]][nn]].isPrimary() ) {
								neib.insert( this->neighbors[cells[nc]][nn] );
							}
						}
					}
					std::vector<T1> w;
					T1 sum_w = 0.0;
					for (typename std::set<T2>::iterator it=neib.begin(); it!=neib.end(); ++it) {
						w.push_back( 1./(*node_p)[*it].getDistance( mid_pt ) );  //  weight is inverse distance
						sum_w += w.back();
					}
					size_t nn=0;
					for (typename std::set<T2>::iterator it=neib.begin(); it!=neib.end(); ++it) {
                        //std::cout << "       *it = " << *it << std::endl;
						m.j = *it;
						m.v = -1.0 / v * ds * w[nn++]/sum_w;
                        bool found = false;
                        for ( size_t nn=0; nn<m_data[n].size(); ++nn ) {
                            if ( m_data[n][nn].j == m.j ) {
                                m_data[n][nn].v += m.v;
                                found = true;
                                break;
                            }
                        }
                        if ( found == false ) {
                            m_data[n].push_back(m);
                        }
					}
					
				}
                // we now go up in time - parent becomes the child of grand'pa
                iChild = iParent;
                child = (*node_p)[iChild];
                
                // grand'pa is now papa
                iParent = (*node_p)[iChild].getNodeParent(threadNo);
                if ( iParent >= this->nodes.size() ) {
                    node_p = &txNodes;
                    iParent -= this->nodes.size();
                }
                else {
                    node_p = &(this->nodes);
                }
            }
            //std::cout << "Outside loop" << std::endl;
            // parent is now at Tx
            r_tmp.push_back( child );
            
            // finally, store Tx position
            child = (*node_p)[iParent];
            r_tmp.push_back( child );
            
            
            // compute last term of matrix M
            mid_pt = static_cast<T1>(0.5)*(r_tmp.back() + r_tmp[ r_tmp.size()-2 ] );
            T1 v = 1./this->computeSlowness( r_tmp.back() );
            v *= v;  // square
            T1 ds = r_tmp.back().getDistance( r_tmp[ r_tmp.size()-2 ] );
            
            T2 cell = this->getCellNo( mid_pt);
            if ( cell == -1 ) {
                std::cerr << "Error while building M" << std::endl;
                return 1;
            }

            T1 w[3];
            
            w[0] =  1./this->nodes[ this->triangles[cell].i[0] ].getDistance( mid_pt );  //  weight is inverse distance
            w[1] =  1./this->nodes[ this->triangles[cell].i[1] ].getDistance( mid_pt );  //  weight is inverse distance
            w[2] =  1./this->nodes[ this->triangles[cell].i[2] ].getDistance( mid_pt );  //  weight is inverse distance
            
            T1 sum_w = w[0] + w[1] + w[2];
            
            for ( size_t nn=0; nn<3; ++nn ) {
                m.j = this->triangles[cell].i[nn];
                m.v = -1.0 / v * ds * w[nn]/sum_w;
                bool found = false;
                for ( size_t nnn=0; nnn<m_data[n].size(); ++nnn ) {
                    if ( m_data[n][nnn].j == m.j ) {
                        m_data[n][nnn].v += m.v;
                        found = true;
                        break;
                    }
                }
                if ( found == false ) {
                    m_data[n].push_back(m);
                }
            }
            
            // the order should be from Tx to Rx, so we reorder...
            iParent = static_cast<T2>(r_tmp.size());
            r_data[n].resize( r_tmp.size() );
            for ( size_t nn=0; nn<r_data[n].size(); ++nn ) {
                r_data[n][nn] = r_tmp[ iParent-1-nn ];
            }
        }
        return 0;
    }

    
    template<typename T1, typename T2, typename NODE, typename S>
    void Grid2Duisp<T1,T2,NODE,S>::initQueue(const std::vector<S>& Tx,
                                             const std::vector<T1>& t0,
                                             std::priority_queue<NODE*,
                                             std::vector<NODE*>,
                                             CompareNodePtr<T1>>& queue,
                                             std::vector<NODE>& txNodes,
                                             std::vector<bool>& inQueue,
                                             std::vector<bool>& frozen,
                                             const size_t threadNo) const {
        
        for (size_t n=0; n<Tx.size(); ++n) {
            bool found = false;
            for ( size_t nn=0; nn<this->nodes.size(); ++nn ) {
                if ( this->nodes[nn] == Tx[n] ) {
                    found = true;
                    this->nodes[nn].setTT( t0[n], threadNo );
                    queue.push( &(this->nodes[nn]) );
                    inQueue[nn] = true;
                    frozen[nn] = true;
                    break;
                }
            }
            if ( found==false ) {
                txNodes.push_back( NODE(t0[n], Tx[n], this->nThreads, threadNo) );
                // we belong to cell index no
                txNodes.back().pushOwner( this->getCellNo(Tx[n]) );
                txNodes.back().setGridIndex( static_cast<T2>(this->nodes.size()+
                                                             txNodes.size()-1) );
                
                queue.push( &(txNodes.back()) );
                inQueue.push_back( true );
                frozen.push_back( true );
            }
        }
    }
    
    
    template<typename T1, typename T2, typename NODE, typename S>
    void Grid2Duisp<T1,T2,NODE,S>::propagate(std::priority_queue<NODE*,
                                             std::vector<NODE*>,
                                             CompareNodePtr<T1>>& queue,
                                             std::vector<bool>& inQueue,
                                             std::vector<bool>& frozen,
                                             const size_t threadNo) const {
        //    size_t n=1;
        while ( !queue.empty() ) {
            
            const NODE* source = queue.top();
            queue.pop();
            inQueue[ source->getGridIndex() ] = false;
            frozen[ source->getGridIndex() ] = true;
            
            for ( size_t no=0; no<source->getOwners().size(); ++no ) {
                
                T2 cellNo = source->getOwners()[no];
                
                for ( size_t k=0; k< this->neighbors[cellNo].size(); ++k ) {
                    T2 neibNo = this->neighbors[cellNo][k];
                    if ( neibNo == source->getGridIndex() || frozen[neibNo] ) {
                        continue;
                    }
                    
                    // compute dt
                    T1 dt = this->computeDt(*source, this->nodes[neibNo]);
                    
                    if (source->getTT(threadNo)+dt < this->nodes[neibNo].getTT(threadNo)) {
                        this->nodes[neibNo].setTT( source->getTT(threadNo)+dt, threadNo );
                        this->nodes[neibNo].setnodeParent(source->getGridIndex(),threadNo);
                        this->nodes[neibNo].setCellParent(cellNo, threadNo );
                        
                        if ( !inQueue[neibNo] ) {
                            queue.push( &(this->nodes[neibNo]) );
                            inQueue[neibNo] = true;
                        }
                    }
                }
            }
        }
    }
    
}

#endif

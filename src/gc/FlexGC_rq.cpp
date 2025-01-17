/* Authors: Lutong Wang and Bangqi Xu */
/*
 * Copyright (c) 2019, The Regents of the University of California
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "gc/FlexGC_impl.h"
#include "frRTree.h"

using namespace std;
using namespace fr;

struct FlexGCWorkerRegionQuery::Impl
{
  void addPolygonEdge(gcSegment* edge, std::vector<std::vector<std::pair<segment_t, gcSegment*> > > &allShapes);
  void addMaxRectangle(gcRect* rect, std::vector<std::vector<rq_box_value_t<gcRect*>>> &allShapes);
  void init(int numLayers);

  FlexGCWorker* gcWorker;
  std::vector<bgi::rtree<std::pair<segment_t, gcSegment*>, bgi::quadratic<16> > > polygon_edges; // merged
  std::vector<bgi::rtree<rq_box_value_t<gcRect*>,          bgi::quadratic<16> > > max_rectangles; // merged

};

FlexGCWorkerRegionQuery::FlexGCWorkerRegionQuery(FlexGCWorker* in)
  : impl(make_unique<Impl>())
{
  impl->gcWorker = in;
}
FlexGCWorkerRegionQuery::~FlexGCWorkerRegionQuery() = default;

FlexGCWorker* FlexGCWorkerRegionQuery::getGCWorker() const {
  return impl->gcWorker;
}

void FlexGCWorkerRegionQuery::addPolygonEdge(gcSegment* edge) {
  segment_t boosts(point_t(edge->low().x(), edge->low().y()), point_t(edge->high().x(), edge->high().y()));
  impl->polygon_edges[edge->getLayerNum()].insert(make_pair(boosts, edge));
}

void FlexGCWorkerRegionQuery::Impl::addPolygonEdge(gcSegment* edge, vector<vector<pair<segment_t, gcSegment*> > > &allShapes) {
  segment_t boosts(point_t(edge->low().x(), edge->low().y()), point_t(edge->high().x(), edge->high().y()));
  allShapes[edge->getLayerNum()].push_back(make_pair(boosts, edge));
}

void FlexGCWorkerRegionQuery::addMaxRectangle(gcRect* rect) {
  box_t boostb(point_t(gtl::xl(*rect), gtl::yl(*rect)), point_t(gtl::xh(*rect), gtl::yh(*rect)));
  impl->max_rectangles[rect->getLayerNum()].insert(make_pair(boostb, rect));
}

void FlexGCWorkerRegionQuery::Impl::addMaxRectangle(gcRect* rect, vector<vector<rq_box_value_t<gcRect*>>> &allShapes) {
  box_t boostb(point_t(gtl::xl(*rect), gtl::yl(*rect)), point_t(gtl::xh(*rect), gtl::yh(*rect)));
  allShapes[rect->getLayerNum()].push_back(make_pair(boostb, rect));
}

void FlexGCWorkerRegionQuery::removePolygonEdge(gcSegment* edge) {
  segment_t boosts(point_t(edge->low().x(), edge->low().y()), point_t(edge->high().x(), edge->high().y()));
  impl->polygon_edges[edge->getLayerNum()].remove(make_pair(boosts, edge));
}

void FlexGCWorkerRegionQuery::removeMaxRectangle(gcRect* rect) {
  box_t boostb(point_t(gtl::xl(*rect), gtl::yl(*rect)), point_t(gtl::xh(*rect), gtl::yh(*rect)));
  impl->max_rectangles[rect->getLayerNum()].remove(make_pair(boostb, rect));
}

void FlexGCWorkerRegionQuery::queryPolygonEdge(const box_t &box, frLayerNum layerNum, vector<pair<segment_t, gcSegment*> > &result) {
  impl->polygon_edges[layerNum].query(bgi::intersects(box), back_inserter(result));
}

void FlexGCWorkerRegionQuery::queryPolygonEdge(const frBox &box, frLayerNum layerNum, vector<pair<segment_t, gcSegment*> > &result) {
  box_t boostb(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  queryPolygonEdge(boostb, layerNum, result);
}

void FlexGCWorkerRegionQuery::queryMaxRectangle(const box_t &box, frLayerNum layerNum, std::vector<rq_box_value_t<gcRect*> > &result) {
  impl->max_rectangles[layerNum].query(bgi::intersects(box), back_inserter(result));
}

void FlexGCWorkerRegionQuery::queryMaxRectangle(const frBox &box, frLayerNum layerNum, std::vector<rq_box_value_t<gcRect*> > &result) {
  box_t boostb(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  queryMaxRectangle(boostb, layerNum, result);
}

void FlexGCWorkerRegionQuery::queryMaxRectangle(const gtl::rectangle_data<frCoord> &box, frLayerNum layerNum, std::vector<rq_box_value_t<gcRect*> > &result) {
  box_t boostb(point_t(gtl::xl(box), gtl::yl(box)), point_t(gtl::xh(box), gtl::yh(box)));
  queryMaxRectangle(boostb, layerNum, result);
}

void FlexGCWorkerRegionQuery::init(int numLayers) {
  impl->init(numLayers);
}

void FlexGCWorkerRegionQuery::Impl::init(int numLayers) {
  bool enableOutput = false;
  //bool enableOutput = true;
  polygon_edges.clear();
  polygon_edges.resize(numLayers);
  max_rectangles.clear();
  max_rectangles.resize(numLayers);
  
  vector<vector<pair<segment_t, gcSegment*>>> allPolygonEdges(numLayers);
  vector<vector<rq_box_value_t<gcRect*>>> allMaxRectangles(numLayers);

  int cntPolygonEdge  = 0;
  int cntMaxRectangle = 0;
  for (auto &net: gcWorker->getNets()) {
    for (auto &pins: net->getPins()) {
      for (auto &pin: pins) {
        for (auto &edges: pin->getPolygonEdges()) {
          for (auto &edge: edges) {
            addPolygonEdge(edge.get(), allPolygonEdges);
            cntPolygonEdge++;
          }
        }  
        for (auto &rect: pin->getMaxRectangles()) {
          addMaxRectangle(rect.get(), allMaxRectangles);
          cntMaxRectangle++;
        }
      }
    }
  }

  int cntRTPolygonEdge  = 0;
  int cntRTMaxRectangle = 0;
  for (int i = 0; i < numLayers; i++) {
    polygon_edges[i]  = boost::move(bgi::rtree<pair<segment_t, gcSegment*>, bgi::quadratic<16>>(allPolygonEdges[i]));
    max_rectangles[i] = boost::move(bgi::rtree<rq_box_value_t<gcRect*>, bgi::quadratic<16>>(allMaxRectangles[i]));
    cntRTPolygonEdge  += polygon_edges[i].size();
    cntRTMaxRectangle += max_rectangles[i].size();
  }
  if (enableOutput) {
    for (int i = 0; i < numLayers; i++) {
      frPoint bp, ep;
      double dbu = gcWorker->getDesign()->getTopBlock()->getDBUPerUU();
      for (auto &[seg, ptr]: polygon_edges[i]) {
        //ptr->getPoints(bp, ep);
        cout << __FILE__ << ":" << __LINE__ << ": " <<"polyEdge ";
        if (ptr->isFixed()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"FIXED";
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"ROUTE";
        }
        cout << __FILE__ << ":" << __LINE__ << ": " <<" @(" <<ptr->low().x() / dbu <<", " <<ptr->low().y() / dbu <<") (" <<ptr->high().x() / dbu <<", " <<ptr->high().y() / dbu <<") "
             <<gcWorker->getDesign()->getTech()->getLayer(i)->getName() <<" ";
        auto owner = ptr->getNet()->getOwner();
        if (owner == nullptr) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" FLOATING";
        } else {
          if (owner->typeId() == frcNet) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frNet*>(owner)->getName();
          } else if (owner->typeId() == frcInstTerm) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
                 <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
          } else if (owner->typeId() == frcTerm) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
          } else if (owner->typeId() == frcInstBlockage) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
          } else if (owner->typeId() == frcBlockage) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"PIN/OBS";
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"UNKNOWN";
          }
        }
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
      }
    }
  }
  
  if (enableOutput) {
    for (int i = 0; i < numLayers; i++) {
      double dbu = gcWorker->getDesign()->getTopBlock()->getDBUPerUU();
      for (auto &[box, ptr]: max_rectangles[i]) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"maxRect ";
        if (ptr->isFixed()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"FIXED";
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"ROUTE";
        }
        cout << __FILE__ << ":" << __LINE__ << ": " <<" @(" <<gtl::xl(*ptr) / dbu <<", " <<gtl::yl(*ptr) / dbu <<") (" <<gtl::xh(*ptr) / dbu <<", " <<gtl::yh(*ptr) / dbu <<") "
             <<gcWorker->getDesign()->getTech()->getLayer(i)->getName() <<" ";
        auto owner = ptr->getNet()->getOwner();
        if (owner == nullptr) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" FLOATING";
        } else {
          if (owner->typeId() == frcNet) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frNet*>(owner)->getName();
          } else if (owner->typeId() == frcInstTerm) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
                 <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
          } else if (owner->typeId() == frcTerm) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
          } else if (owner->typeId() == frcInstBlockage) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
          } else if (owner->typeId() == frcBlockage) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"PIN/OBS";
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"UNKNOWN";
          }
        }
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
      }
    }
  }

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"gc region query #poly_edges/max_rects/rt_poly_edges/rt_max_rect = " 
         <<cntPolygonEdge <<"/" <<cntMaxRectangle <<"/" <<cntRTPolygonEdge <<"/" <<cntRTMaxRectangle 
         <<endl;
  }
}

void FlexGCWorkerRegionQuery::addToRegionQuery(gcNet* net) {
  for (auto &pins: net->getPins()) {
    for (auto &pin: pins) {
      for (auto &edges: pin->getPolygonEdges()) {
        for (auto &edge: edges) {
          addPolygonEdge(edge.get());
        }
      }  
      for (auto &rect: pin->getMaxRectangles()) {
        addMaxRectangle(rect.get());
      }
    }
  }
}

void FlexGCWorkerRegionQuery::removeFromRegionQuery(gcNet* net) {
  for (auto &pins: net->getPins()) {
    for (auto &pin: pins) {
      for (auto &edges: pin->getPolygonEdges()) {
        for (auto &edge: edges) {
          removePolygonEdge(edge.get());
        }
      }  
      for (auto &rect: pin->getMaxRectangles()) {
        removeMaxRectangle(rect.get());
      }
    }
  }
}


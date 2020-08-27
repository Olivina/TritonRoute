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
#include "global.h"
#include "frDesign.h"
#include "frRegionQuery.h"
using namespace std;
using namespace fr;

void frRegionQuery::add(frShape* shape, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;
  if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect) {
    shape->getBBox(frb);
    boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
    allShapes.at(shape->getLayerNum()).push_back(make_pair(boostb, shape));
  } else {
    cout <<"Error: unsupported region query add" <<endl;
  }
}

void frRegionQuery::addDRObj(frShape* shape) {
  frBox frb;
  box_t boostb;
  if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect || shape->typeId() == frcPatchWire) {
    shape->getBBox(frb);
    boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
    drObjs.at(shape->getLayerNum()).insert(make_pair(boostb, shape));
  } else {
    cout <<"Error: unsupported region query add" <<endl;
  }
}

void frRegionQuery::addMarker(frMarker* in) {
  frBox frb;
  box_t boostb;
  in->getBBox(frb);
  boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  markers.at(in->getLayerNum()).insert(make_pair(boostb, in));
}

void frRegionQuery::addDRObj(frShape* shape, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;
  if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect || shape->typeId() == frcPatchWire) {
    shape->getBBox(frb);
    boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
    allShapes.at(shape->getLayerNum()).push_back(make_pair(boostb, shape));
  } else {
    cout <<"Error: unsupported region query add" <<endl;
  }
}

void frRegionQuery::removeDRObj(frShape* shape) {
  frBox frb;
  box_t boostb;
  if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect || shape->typeId() == frcPatchWire) {
    shape->getBBox(frb);
    boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
    drObjs.at(shape->getLayerNum()).remove(make_pair(boostb, shape));
  } else {
    cout <<"Error: unsupported region query add" <<endl;
  }
}

void frRegionQuery::removeMarker(frMarker* in) {
  frBox frb;
  box_t boostb;
  in->getBBox(frb);
  boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  markers.at(in->getLayerNum()).remove(make_pair(boostb, in));
}

void frRegionQuery::add(frVia* via, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  frTransform xform;
  frPoint origin;
  via->getOrigin(origin);
  xform.set(origin);
  box_t boostb;
  for (auto &uShape: via->getViaDef()->getLayer1Figs()) {
    auto shape = uShape.get();
    if (shape->typeId() == frcRect) {
      shape->getBBox(frb);
      frb.transform(xform);
      boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
      allShapes.at(via->getViaDef()->getLayer1Num()).push_back(make_pair(boostb, via));
    } else {
      cout <<"Error: unsupported region query add" <<endl;
    }
  }
  for (auto &uShape: via->getViaDef()->getLayer2Figs()) {
    auto shape = uShape.get();
    if (shape->typeId() == frcRect) {
      shape->getBBox(frb);
      frb.transform(xform);
      boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
      allShapes.at(via->getViaDef()->getLayer2Num()).push_back(make_pair(boostb, via));
    } else {
      cout <<"Error: unsupported region query add" <<endl;
    }
  }
  for (auto &uShape: via->getViaDef()->getCutFigs()) {
    auto shape = uShape.get();
    if (shape->typeId() == frcRect) {
      shape->getBBox(frb);
      frb.transform(xform);
      boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
      allShapes.at(via->getViaDef()->getCutLayerNum()).push_back(make_pair(boostb, via));
    } else {
      cout <<"Error: unsupported region query add" <<endl;
    }
  }
}

void frRegionQuery::addDRObj(frVia* via) {
  frBox frb;
  via->getBBox(frb);
  box_t boostb(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  drObjs.at(via->getViaDef()->getCutLayerNum()).insert(make_pair(boostb, via));
}

void frRegionQuery::addDRObj(frVia* via, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  via->getBBox(frb);
  box_t boostb(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  allShapes.at(via->getViaDef()->getCutLayerNum()).push_back(make_pair(boostb, via));
}

void frRegionQuery::removeDRObj(frVia* via) {
  frBox frb;
  via->getBBox(frb);
  box_t boostb(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  drObjs.at(via->getViaDef()->getCutLayerNum()).remove(make_pair(boostb, via));
}

void frRegionQuery::add(frInstTerm* instTerm, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;

  frTransform xform;
  instTerm->getInst()->getUpdatedXform(xform);

  for (auto &pin: instTerm->getTerm()->getPins()) {
    for (auto &uFig: pin->getFigs()) {
      auto shape = uFig.get();
      if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect) {
        shape->getBBox(frb);
        frb.transform(xform);
        boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
        allShapes.at(static_cast<frShape*>(shape)->getLayerNum()).push_back(make_pair(boostb, instTerm));
      } else {
        cout <<"Error: unsupported region query add" <<endl;
      }
    }
  }
}

void frRegionQuery::add(frTerm* term, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;
  for (auto &pin: term->getPins()) {
    for (auto &uFig: pin->getFigs()) {
      auto shape = uFig.get();
      if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect) {
        shape->getBBox(frb);
        boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
        allShapes.at(static_cast<frShape*>(shape)->getLayerNum()).push_back(make_pair(boostb, term));
      } else {
        cout <<"Error: unsupported region query add" <<endl;
      }
    }
  }
}

void frRegionQuery::add(frInstBlockage* instBlk, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;

  frTransform xform;
  instBlk->getInst()->getUpdatedXform(xform);
  auto blk = instBlk->getBlockage();
  auto pin = blk->getPin();
  for (auto &uFig: pin->getFigs()) {
    auto shape = uFig.get();
    if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect) {
      shape->getBBox(frb);
      frb.transform(xform);
      boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
      allShapes.at(static_cast<frShape*>(shape)->getLayerNum()).push_back(make_pair(boostb, instBlk));
    } else {
      cout <<"Error: unsupported region query add" <<endl;
    }
  }
}

void frRegionQuery::add(frBlockage* blk, ObjectsByLayer<frBlockObject> &allShapes) {
  frBox frb;
  box_t boostb;
  auto pin = blk->getPin();
  for (auto &uFig: pin->getFigs()) {
    auto shape = uFig.get();
    if (shape->typeId() == frcPathSeg || shape->typeId() == frcRect) {
      shape->getBBox(frb);
      boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
      allShapes.at(static_cast<frShape*>(shape)->getLayerNum()).push_back(make_pair(boostb, blk));
    } else {
      cout <<"Error: unsupported region query add" <<endl;
    }
  }
}

void frRegionQuery::addGuide(frGuide* guide, ObjectsByLayer<frGuide> &allShapes) {
  frBox frb;
  box_t boostb;
  guide->getBBox(frb);
  boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  for (int i = guide->getBeginLayerNum(); i <= guide->getEndLayerNum(); i++) {
    allShapes.at(i).push_back(make_pair(boostb, guide));
  }
}

void frRegionQuery::addOrigGuide(frNet* net, const frRect &rect, ObjectsByLayer<frNet> &allShapes) {
  frBox frb;
  rect.getBBox(frb);
  box_t boostb;
  boostb = box_t(point_t(frb.left(), frb.bottom()), point_t(frb.right(), frb.top()));
  allShapes.at(rect.getLayerNum()).push_back(make_pair(boostb, net));
}

void frRegionQuery::query(const frBox &box, frLayerNum layerNum, Objects<frBlockObject> &result) {
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  shapes.at(layerNum).query(bgi::intersects(boostb), back_inserter(result));
}

void frRegionQuery::queryGuide(const frBox &box, frLayerNum layerNum, Objects<frGuide> &result) {
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  guides.at(layerNum).query(bgi::intersects(boostb), back_inserter(result));
}

void frRegionQuery::queryGuide(const frBox &box, frLayerNum layerNum, vector<frGuide*> &result) {
  Objects<frGuide> temp;
  queryGuide(box, layerNum, temp);
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryGuide(const frBox &box, vector<frGuide*> &result) {
  Objects<frGuide> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  for (auto &m: guides) {
    m.query(bgi::intersects(boostb), back_inserter(temp));
  }
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryOrigGuide(const frBox &box, frLayerNum layerNum, Objects<frNet> &result) {
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  origGuides.at(layerNum).query(bgi::intersects(boostb), back_inserter(result));
}

void frRegionQuery::queryGRPin(const frBox &box, vector<frBlockObject*> &result) {
  Objects<frBlockObject> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  grPins.query(bgi::intersects(boostb), back_inserter(temp));
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryDRObj(const frBox &box, frLayerNum layerNum, Objects<frBlockObject> &result) {
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  drObjs.at(layerNum).query(bgi::intersects(boostb), back_inserter(result));
}

void frRegionQuery::queryDRObj(const frBox &box, frLayerNum layerNum, vector<frBlockObject*> &result) {
  Objects<frBlockObject> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  drObjs.at(layerNum).query(bgi::intersects(boostb), back_inserter(temp));
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryDRObj(const frBox &box, vector<frBlockObject*> &result) {
  Objects<frBlockObject> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  for (auto &m: drObjs) {
    m.query(bgi::intersects(boostb), back_inserter(temp));
  }
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryMarker(const frBox &box, frLayerNum layerNum, vector<frMarker*> &result) {
  Objects<frMarker> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  markers.at(layerNum).query(bgi::intersects(boostb), back_inserter(temp));
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::queryMarker(const frBox &box, vector<frMarker*> &result) {
  Objects<frMarker> temp;
  box_t boostb = box_t(point_t(box.left(), box.bottom()), point_t(box.right(), box.top()));
  for (auto &m: markers) {
    m.query(bgi::intersects(boostb), back_inserter(temp));
  }
  transform(temp.begin(), temp.end(), back_inserter(result), [](auto &kv) {return kv.second;});
}

void frRegionQuery::init(frLayerNum numLayers) {
  shapes.clear();
  shapes.resize(numLayers);

  markers.clear();
  markers.resize(numLayers);

  ObjectsByLayer<frBlockObject> allShapes(numLayers);

  int cnt = 0;
  for (auto &inst: design->getTopBlock()->getInsts()) {
    for (auto &instTerm: inst->getInstTerms()) {
      add(instTerm.get(), allShapes);
    }
    for (auto &instBlk: inst->getInstBlockages()) {
      add(instBlk.get(), allShapes);
    }
    cnt++;
    if (VERBOSE > 0) {
      if (cnt < 100000) {
        if (cnt % 10000 == 0) {
          cout <<"  complete " <<cnt <<" insts" <<endl;
        }
      } else {
        if (cnt % 100000 == 0) {
          cout <<"  complete " <<cnt <<" insts" <<endl;
        }
      }
    }
  }
  cnt = 0;
  for (auto &term: design->getTopBlock()->getTerms()) {
    add(term.get(), allShapes);
    cnt++;
    if (VERBOSE > 0) {
      if (cnt < 100000) {
        if (cnt % 10000 == 0) {
          cout <<"  complete " <<cnt <<" terms" <<endl;
        }
      } else {
        if (cnt % 100000 == 0) {
          cout <<"  complete " <<cnt <<" terms" <<endl;
        }
      }
    }
  }
  /*
  cnt = 0;
  for (auto &net: design->getTopBlock()->getNets()) {
    for (auto &shape: net->getShapes()) {
      add(shape.get(), allShapes);
    }
    for (auto &via: net->getVias()) {
      add(via.get(), allShapes);
    }
    cnt++;
    if (VERBOSE > 0) {
      if (cnt % 10000 == 0) {
        cout <<"  complete " <<cnt <<" nets" <<endl;
      }
    }
  }
  */
  cnt = 0;
  for (auto &net: design->getTopBlock()->getSNets()) {
    for (auto &shape: net->getShapes()) {
      add(shape.get(), allShapes);
    }
    for (auto &via: net->getVias()) {
      add(via.get(), allShapes);
    }
    cnt++;
    if (VERBOSE > 0) {
      if (cnt % 10000 == 0) {
        cout <<"  complete " <<cnt <<" snets" <<endl;
      }
    }
  }

  cnt = 0;
  for (auto &blk: design->getTopBlock()->getBlockages()) {
    add(blk.get(), allShapes);
    cnt++;
    if (VERBOSE > 0) {
      if (cnt % 10000 == 0) {
        cout <<"  complete " <<cnt <<" blockages" <<endl;
      }
    }
  }

  for (auto i = 0; i < numLayers; i++) {
    shapes.at(i) = boost::move(rtree<frBlockObject>(allShapes.at(i)));
    allShapes.at(i).clear();
    allShapes.at(i).shrink_to_fit();
    if (VERBOSE > 0) {
      cout <<"  complete " <<design->getTech()->getLayer(i)->getName() <<endl;
    }
  }

}

void frRegionQuery::initOrigGuide(frLayerNum numLayers, map<frNet*, vector<frRect>, frBlockObjectComp> &tmpGuides) {
  origGuides.clear();
  origGuides.resize(numLayers);
  ObjectsByLayer<frNet> allShapes(numLayers);

  int cnt = 0;
  for (auto &[net, rects]: tmpGuides) {
    for (auto &rect: rects) {
      addOrigGuide(net, rect, allShapes);
      cnt++;
      if (VERBOSE > 0) {
        if (cnt < 100000) {
          if (cnt % 10000 == 0) {
            cout <<"  complete " <<cnt <<" orig guides" <<endl;
          }
        } else {
          if (cnt % 100000 == 0) {
            cout <<"  complete " <<cnt <<" orig guides" <<endl;
          }
        }
      }
    }
  }
  for (auto i = 0; i < numLayers; i++) {
    origGuides.at(i) = boost::move(rtree<frNet>(allShapes.at(i)));
    allShapes.at(i).clear();
    allShapes.at(i).shrink_to_fit();
    if (VERBOSE > 0) {
      cout <<"  complete " <<design->getTech()->getLayer(i)->getName() <<endl;
    }
  }
}

void frRegionQuery::initGuide(frLayerNum numLayers) {
  guides.clear();
  guides.resize(numLayers);
  ObjectsByLayer<frGuide> allGuides(numLayers);

  int cnt = 0;
  for (auto &net: design->getTopBlock()->getNets()) {
    for (auto &guide: net->getGuides()) {
      addGuide(guide.get(), allGuides);
    }
    cnt++;
    if (VERBOSE > 0) {
      if (cnt < 100000) {
        if (cnt % 10000 == 0) {
          cout <<"  complete " <<cnt <<" nets (guide)" <<endl;
        }
      } else {
        if (cnt % 100000 == 0) {
          cout <<"  complete " <<cnt <<" nets (guide)" <<endl;
        }
      }
    }
  }
  for (auto i = 0; i < numLayers; i++) {
    guides.at(i) = boost::move(rtree<frGuide>(allGuides.at(i)));
    allGuides.at(i).clear();
    allGuides.at(i).shrink_to_fit();
    if (VERBOSE > 0) {
      cout <<"  complete " <<design->getTech()->getLayer(i)->getName() <<" (guide)"<<endl;
    }
  }
}

void frRegionQuery::initGRPin(vector<pair<frBlockObject*, frPoint> > &in) {
  grPins.clear();
  Objects<frBlockObject> allGRPins;
  box_t boostb;
  for (auto &[obj, pt]: in) {
    boostb = box_t(point_t(pt.x(), pt.y()), point_t(pt.x(), pt.y()));
    allGRPins.push_back(make_pair(boostb, obj));
  }
  in.clear();
  in.shrink_to_fit();
  grPins = boost::move(rtree<frBlockObject>(allGRPins));
}

void frRegionQuery::initDRObj(frLayerNum numLayers) {
  drObjs.clear();
  drObjs.resize(numLayers);

  ObjectsByLayer<frBlockObject> allShapes(numLayers);

  for (auto &net: design->getTopBlock()->getNets()) {
    for (auto &shape: net->getShapes()) {
      addDRObj(shape.get(), allShapes);
    }
    for (auto &via: net->getVias()) {
      addDRObj(via.get(), allShapes);
    }
  }

  for (auto i = 0; i < numLayers; i++) {
    drObjs.at(i) = boost::move(rtree<frBlockObject>(allShapes.at(i)));
    allShapes.at(i).clear();
    allShapes.at(i).shrink_to_fit();
  }

}

void frRegionQuery::print() {
  cout <<endl;
  for (int i = 0; i < (int)(design->getTech()->getLayers().size()); i++) {
    frString layerName;
    design->getTech()->getLayers().at(i)->getName(layerName);
    cout <<layerName <<" shape region query size = " <<shapes.at(i).size() <<endl <<flush;
  }
}

void frRegionQuery::printGuide() {
  cout <<endl;
  for (int i = 0; i < (int)(design->getTech()->getLayers().size()); i++) {
    frString layerName;
    design->getTech()->getLayers().at(i)->getName(layerName);
    cout <<layerName <<" guide region query size = " <<guides.at(i).size() <<endl <<flush;
  }
}

void frRegionQuery::printDRObj() {
  cout <<endl;
  for (int i = 0; i < (int)(design->getTech()->getLayers().size()); i++) {
    frString layerName;
    design->getTech()->getLayers().at(i)->getName(layerName);
    cout <<layerName <<" drObj region query size = " <<drObjs.at(i).size() <<endl <<flush;
  }
}

void frRegionQuery::clearGuides() {
  for (auto &m: guides) {
    m.clear();
  }
}

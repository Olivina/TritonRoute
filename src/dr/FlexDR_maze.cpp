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

#include "frProfileTask.h"
#include "dr/FlexDR.h"
#include "gc/FlexGC.h"
#include <chrono>
#include <algorithm>
#include <random>
#include <boost/polygon/polygon.hpp>

using namespace std;
using namespace fr;
namespace gtl = boost::polygon;

inline frCoord FlexDRWorker::pt2boxDistSquare(const frPoint &pt, const frBox &box) {
  frCoord dx = max(max(box.left()   - pt.x(), pt.x() - box.right()), 0);
  frCoord dy = max(max(box.bottom() - pt.y(), pt.y() - box.top()),   0);
  return dx * dx + dy * dy;
}

inline frCoord FlexDRWorker::box2boxDistSquare(const frBox &box1, const frBox &box2, frCoord &dx, frCoord &dy) {
  dx = max(max(box1.left(), box2.left())     - min(box1.right(), box2.right()), 0);
  dy = max(max(box1.bottom(), box2.bottom()) - min(box1.top(), box2.top()),     0);
  return dx * dx + dy * dy;
}

// prlx = -dx, prly = -dy
// dx > 0 : disjoint in x; dx = 0 : touching in x; dx < 0 : overlap in x
inline frCoord FlexDRWorker::box2boxDistSquareNew(const frBox &box1, const frBox &box2, frCoord &dx, frCoord &dy) {
  dx = max(box1.left(), box2.left())     - min(box1.right(), box2.right());
  dy = max(box1.bottom(), box2.bottom()) - min(box1.top(), box2.top());
  return max(dx, 0) * max(dx, 0) + max(dy, 0) * max(dy, 0);
}

void FlexDRWorker::modViaForbiddenThrough(const FlexMazeIdx &bi, const FlexMazeIdx &ei, int type) {
  bool isHorz = (bi.y() == ei.y());

  bool isLowerViaForbidden = getTech()->isViaForbiddenThrough(bi.z(), true, isHorz);
  bool isUpperViaForbidden = getTech()->isViaForbiddenThrough(bi.z(), false, isHorz);

  if (isHorz) {
    for (int xIdx = bi.x(); xIdx < ei.x(); xIdx++) {
      if (isLowerViaForbidden) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(xIdx, bi.y(), bi.z() - 1); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(xIdx, bi.y(), bi.z() - 1); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(xIdx, bi.y(), bi.z() - 1); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(xIdx, bi.y(), bi.z() - 1); // safe access
            break;
          default:
            ;
        }
      }

      if (isUpperViaForbidden) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(xIdx, bi.y(), bi.z()); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(xIdx, bi.y(), bi.z()); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(xIdx, bi.y(), bi.z()); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(xIdx, bi.y(), bi.z()); // safe access
            break;
          default:
            ;
        }
      }
    }
  } else {
    for (int yIdx = bi.y(); yIdx < ei.y(); yIdx++) {
      if (isLowerViaForbidden) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(bi.x(), yIdx, bi.z() - 1); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(bi.x(), yIdx, bi.z() - 1); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(bi.x(), yIdx, bi.z() - 1); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(bi.x(), yIdx, bi.z() - 1); // safe access
            break;
          default:
            ;
        }
      }

      if (isUpperViaForbidden) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(bi.x(), yIdx, bi.z()); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(bi.x(), yIdx, bi.z()); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(bi.x(), yIdx, bi.z()); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(bi.x(), yIdx, bi.z()); // safe access
            break;
          default:
            ;
        }
      }
    }
  }
}

void FlexDRWorker::modBlockedPlanar(const frBox &box, frMIdx z, bool setBlock) {
  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  gridGraph.getIdxBox(mIdx1, mIdx2, box);
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      if (setBlock) {
        gridGraph.setBlocked(i, j, z, frDirEnum::E);
        gridGraph.setBlocked(i, j, z, frDirEnum::N);
        gridGraph.setBlocked(i, j, z, frDirEnum::W);
        gridGraph.setBlocked(i, j, z, frDirEnum::S);
      } else {
        gridGraph.resetBlocked(i, j, z, frDirEnum::E);
        gridGraph.resetBlocked(i, j, z, frDirEnum::N);
        gridGraph.resetBlocked(i, j, z, frDirEnum::W);
        gridGraph.resetBlocked(i, j, z, frDirEnum::S);
      }
    }
  }
}

void FlexDRWorker::modBlockedVia(const frBox &box, frMIdx z, bool setBlock) {
  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  gridGraph.getIdxBox(mIdx1, mIdx2, box);
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      if (setBlock) {
        gridGraph.setBlocked(i, j, z, frDirEnum::U);
        gridGraph.setBlocked(i, j, z, frDirEnum::D);
      } else {
        gridGraph.resetBlocked(i, j, z, frDirEnum::U);
        gridGraph.resetBlocked(i, j, z, frDirEnum::D);
      }
    }
  }
}

/*inline*/ void FlexDRWorker::modMinSpacingCostPlaner(const frBox &box, frMIdx z, int type, bool isBlockage) {
  auto lNum = gridGraph.getLayerNum(z);
  frCoord width1  = box.width();
  frCoord length1 = box.length();
  // layer default width
  frCoord width2     = getDesign()->getTech()->getLayer(lNum)->getWidth();
  frCoord halfwidth2 = width2 / 2;
  // spacing value needed
  frCoord bloatDist = 0;
  auto con = getDesign()->getTech()->getLayer(lNum)->getMinSpacing();
  if (con) {
    if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
      bloatDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
      bloatDist = (isBlockage && USEMINSPACING_OBS) ? static_cast<frSpacingTablePrlConstraint*>(con)->findMin() :
                               static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2), length1);
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
      bloatDist = (isBlockage && USEMINSPACING_OBS) ? static_cast<frSpacingTableTwConstraint*>(con)->findMin() :
                               static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2, length1);
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: min spacing rule not supporterd" <<endl;
      return;
    }
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: no min spacing rule" <<endl;
    return;
  }
  frCoord bloatDistSquare = bloatDist * bloatDist;

  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  // assumes width always > 2
  frBox bx(box.left()   - bloatDist - halfwidth2 + 1, box.bottom() - bloatDist - halfwidth2 + 1,
           box.right()  + bloatDist + halfwidth2 - 1, box.top()    + bloatDist + halfwidth2 - 1);
  gridGraph.getIdxBox(mIdx1, mIdx2, bx);
  //if (!isInitDR()) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" box " <<box <<" bloatDist " <<bloatDist <<" bx " <<bx <<endl;
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" midx1/2 (" <<mIdx1.x() <<", " <<mIdx1.y() <<") ("
  //                      <<mIdx2.x() <<", " <<mIdx2.y() <<") (" <<endl;
  //}

  frPoint pt, pt1, pt2, pt3, pt4;
  frCoord distSquare = 0;
  int cnt = 0;
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      gridGraph.getPoint(pt, i, j);
      pt1.set(pt.x() + halfwidth2, pt.y() - halfwidth2);
      pt2.set(pt.x() + halfwidth2, pt.y() + halfwidth2);
      pt3.set(pt.x() - halfwidth2, pt.y() - halfwidth2);
      pt4.set(pt.x() - halfwidth2, pt.y() + halfwidth2);
      distSquare = min(pt2boxDistSquare(pt1, box), pt2boxDistSquare(pt2, box));
      distSquare = min(pt2boxDistSquare(pt3, box), distSquare);
      distSquare = min(pt2boxDistSquare(pt4, box), distSquare);
      if (distSquare < bloatDistSquare) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostPlanar(i, j, z); // safe access
            break;
          case 1:
            gridGraph.addDRCCostPlanar(i, j, z); // safe access
            break;
          case 2:
            gridGraph.subShapeCostPlanar(i, j, z); // safe access
            break;
          case 3:
            gridGraph.addShapeCostPlanar(i, j, z); // safe access
            break;
          default:
            ;
        }
        if (QUICKDRCTEST) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") minSpc planer" <<endl;
        }
        cnt++;
        //if (!isInitDR()) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<" planer find viol mIdx (" <<i <<", " <<j <<") " <<pt <<endl;
        //}
      }
    }
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"planer mod " <<cnt <<" edges" <<endl;
}


void FlexDRWorker::modMinSpacingCost(drNet* net, const frBox &box, frMIdx z, int type, bool isCurrPs) {
  auto lNum = gridGraph.getLayerNum(z);
  frCoord width1  = box.width();
  frCoord length1 = box.length();
  // layer default width
  frCoord width2planar     = getDesign()->getTech()->getLayer(lNum)->getWidth();
  frCoord halfwidth2planar = width2planar / 2;
  frViaDef* viaDefL = (lNum > getDesign()->getTech()->getBottomLayerNum()) ? 
                      getDesign()->getTech()->getLayer(lNum-1)->getDefaultViaDef() : 
                      nullptr;
  frVia viaL(viaDefL);
  frBox viaBoxL(0,0,0,0);
  if (viaDefL) {
    viaL.getLayer2BBox(viaBoxL);
  }
  frCoord width2viaL  = viaBoxL.width();
  frCoord length2viaL = viaBoxL.length();
  // obj2 viaU = other obj
  frViaDef* viaDefU = (lNum < getDesign()->getTech()->getTopLayerNum()) ? 
                      getDesign()->getTech()->getLayer(lNum+1)->getDefaultViaDef() : 
                      nullptr;
  frVia viaU(viaDefU);
  frBox viaBoxU(0,0,0,0);
  if (viaDefU) {
    viaU.getLayer1BBox(viaBoxU);
  }
  frCoord width2viaU  = viaBoxU.width();
  frCoord length2viaU = viaBoxU.length();

  // spacing value needed
  frCoord bloatDistPlanar = 0;
  frCoord bloatDistViaL   = 0;
  frCoord bloatDistViaU   = 0;
  auto con = getDesign()->getTech()->getLayer(lNum)->getMinSpacing();
  if (con) {
    if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
      bloatDistPlanar = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
      bloatDistViaL   = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
      bloatDistViaU   = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
      bloatDistPlanar = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2planar), length1);
      bloatDistViaL   = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2viaL), isCurrPs ? length2viaL : min(length1, length2viaL));
      bloatDistViaU   = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2viaU), isCurrPs ? length2viaU : min(length1, length2viaU));
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
      bloatDistPlanar = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2planar, length1);
      bloatDistViaL   = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2viaL, isCurrPs ? length2viaL : min(length1, length2viaL));
      bloatDistViaU   = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2viaU, isCurrPs ? length2viaU : min(length1, length2viaU));
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: min spacing rule not supporterd" <<endl;
      return;
    }
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: no min spacing rule" <<endl;
    return;
  }

  // other obj eol spc to curr obj
  // no need to bloat eolWithin because eolWithin always < minSpacing
  frCoord bloatDistEolX = 0;
  frCoord bloatDistEolY = 0;
  for (auto con: getDesign()->getTech()->getLayer(lNum)->getEolSpacing()) {
    auto eolSpace  = con->getMinSpacing();
    auto eolWidth  = con->getEolWidth();
    // eol up and down
    if (viaDefL && viaBoxL.right() - viaBoxL.left() < eolWidth) {
      bloatDistEolY = max(bloatDistEolY, eolSpace);
    } 
    if (viaDefU && viaBoxU.right() - viaBoxU.left() < eolWidth) {
      bloatDistEolY = max(bloatDistEolY, eolSpace);
    } 
    // eol left and right
    if (viaDefL && viaBoxL.top() - viaBoxL.bottom() < eolWidth) {
      bloatDistEolX = max(bloatDistEolX, eolSpace);
    }
    if (viaDefU && viaBoxU.top() - viaBoxU.bottom() < eolWidth) {
      bloatDistEolX = max(bloatDistEolX, eolSpace);
    }
  }

  frCoord bloatDist = max(max(bloatDistPlanar, bloatDistViaL), bloatDistViaU);

  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  // assumes width always > 2
  frBox bx(box.left()   - max(bloatDist, bloatDistEolX) - max(max(halfwidth2planar, viaBoxL.right() ), viaBoxU.right() ) + 1, 
           box.bottom() - max(bloatDist, bloatDistEolY) - max(max(halfwidth2planar, viaBoxL.top()   ), viaBoxU.top()   ) + 1,
           box.right()  + max(bloatDist, bloatDistEolX) + max(max(halfwidth2planar, viaBoxL.left()  ), viaBoxU.left()  ) - 1, 
           box.top()    + max(bloatDist, bloatDistEolY) + max(max(halfwidth2planar, viaBoxL.bottom()), viaBoxU.bottom()) - 1);
  gridGraph.getIdxBox(mIdx1, mIdx2, bx);
  //if (!isInitDR()) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" box " <<box <<" bloatDist " <<bloatDist <<" bx " <<bx <<endl;
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" midx1/2 (" <<mIdx1.x() <<", " <<mIdx1.y() <<") ("
  //                      <<mIdx2.x() <<", " <<mIdx2.y() <<") (" <<endl;
  //}

  frPoint pt;
  frBox tmpBx;
  frCoord dx, dy, prl;
  frTransform xform;
  frCoord reqDist = 0;
  frCoord distSquare = 0;
  int cnt = 0;
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      gridGraph.getPoint(pt, i, j);
      xform.set(pt);
      // planar
      tmpBx.set(pt.x() - halfwidth2planar, pt.y() - halfwidth2planar,
                pt.x() + halfwidth2planar, pt.y() + halfwidth2planar);
      distSquare = box2boxDistSquare(box, tmpBx, dx, dy);
      prl = max(dx, dy);
      if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
        reqDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
      } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
        reqDist = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2planar), prl > 0 ? length1 : 0);
      } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
        reqDist = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2planar, prl > 0 ? length1 : 0);
      }
      if (distSquare < reqDist * reqDist) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostPlanar(i, j, z); // safe access
            break;
          case 1:
            gridGraph.addDRCCostPlanar(i, j, z); // safe access
            break;
          case 2:
            gridGraph.subShapeCostPlanar(i, j, z);
            break;
          case 3:
            gridGraph.addShapeCostPlanar(i, j, z);
            break;
          default:
            ;
        }
        if (QUICKDRCTEST) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") minSpc planer" <<endl;
        }
        cnt++;
      }
      // viaL
      if (viaDefL) {
        tmpBx.set(viaBoxL);
        tmpBx.transform(xform);
        distSquare = box2boxDistSquare(box, tmpBx, dx, dy);
        prl = max(dx, dy);
        // curr is ps
        if (isCurrPs) {
          if (dx == 0 && dy > 0) {
            prl = viaBoxL.right() - viaBoxL.left();
          } else if (dx > 0 && dy == 0) {
            prl = viaBoxL.top() - viaBoxL.bottom();
          }
        }
        if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
          reqDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
        } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
          reqDist = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2viaL), prl);
        } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
          reqDist = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2viaL, prl);
        }
        if (distSquare < reqDist * reqDist) {
          switch(type) {
            case 0:
              gridGraph.subDRCCostVia(i, j, z - 1);
              break;
            case 1:
              gridGraph.addDRCCostVia(i, j, z - 1);
              break;
            case 2:
              gridGraph.subShapeCostVia(i, j, z - 1);
              break;
            case 3:
              gridGraph.addShapeCostVia(i, j, z - 1);
              break;
            default:
              ;
          }
          if (QUICKDRCTEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z - 1 <<") U minSpc via" <<endl;
          }
        } else {
          modMinSpacingCostVia_eol(box, tmpBx, type, false, i, j, z);
        }
      }
      if (viaDefU) {
        tmpBx.set(viaBoxU);
        tmpBx.transform(xform);
        distSquare = box2boxDistSquare(box, tmpBx, dx, dy);
        prl = max(dx, dy);
        // curr is ps
        if (isCurrPs) {
          if (dx == 0 && dy > 0) {
            prl = viaBoxU.right() - viaBoxU.left();
          } else if (dx > 0 && dy == 0) {
            prl = viaBoxU.top() - viaBoxU.bottom();
          }
        }
        if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
          reqDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
        } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
          reqDist = static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2viaU), prl);
        } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
          reqDist = static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2viaU, prl);
        }
        if (distSquare < reqDist * reqDist) {
          switch(type) {
            case 0:
              gridGraph.subDRCCostVia(i, j, z);
              break;
            case 1:
              gridGraph.addDRCCostVia(i, j, z);
              break;
            case 2:
              gridGraph.subShapeCostVia(i, j, z); // safe access
              break;
            case 3:
              gridGraph.addShapeCostVia(i, j, z); // safe access
              break;
            default:
              ;
          }
          if (QUICKDRCTEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U minSpc via" <<endl;
          }
        } else {
          modMinSpacingCostVia_eol(box, tmpBx, type, true, i, j, z);
        }
      }
    }
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"planer mod " <<cnt <<" edges" <<endl;
}

void FlexDRWorker::modMinSpacingCostVia_eol_helper(const frBox &box, const frBox &testBox, int type, bool isUpperVia,
                                                          frMIdx i, frMIdx j, frMIdx z) {
  if (testBox.overlaps(box, false)) {
    if (isUpperVia) {
      switch(type) {
        case 0:
          gridGraph.subDRCCostVia(i, j, z);
          break;
        case 1:
          gridGraph.addDRCCostVia(i, j, z);
          break;
        case 2:
          gridGraph.subShapeCostVia(i, j, z); // safe access
          break;
        case 3:
          gridGraph.addShapeCostVia(i, j, z); // safe access
          break;
        default:
          ;
      }
      if (QUICKDRCTEST) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U minSpc eol helper" <<endl;
      }
    } else {
      switch(type) {
        case 0:
          gridGraph.subDRCCostVia(i, j, z - 1);
          break;
        case 1:
          gridGraph.addDRCCostVia(i, j, z - 1);
          break;
        case 2:
          gridGraph.subShapeCostVia(i, j, z - 1); // safe access
          break;
        case 3:
          gridGraph.addShapeCostVia(i, j, z - 1); // safe access
          break;
        default:
          ;
      }
      if (QUICKDRCTEST) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z - 1 <<") U minSpc eol helper" <<endl;
      }
    }
  }
}

void FlexDRWorker::modMinSpacingCostVia_eol(const frBox &box, const frBox &tmpBx, int type, bool isUpperVia,
                                                   frMIdx i, frMIdx j, frMIdx z) {
  auto lNum = gridGraph.getLayerNum(z);
  frBox testBox;
  if (getDesign()->getTech()->getLayer(lNum)->hasEolSpacing()) {
    for (auto eolCon: getDesign()->getTech()->getLayer(lNum)->getEolSpacing()) {
      auto eolSpace  = eolCon->getMinSpacing();
      auto eolWidth  = eolCon->getEolWidth();
      auto eolWithin = eolCon->getEolWithin();
      // eol to up and down
      if (tmpBx.right() - tmpBx.left() < eolWidth) {
        testBox.set(tmpBx.left() - eolWithin, tmpBx.top(),               tmpBx.right() + eolWithin, tmpBx.top() + eolSpace);
        modMinSpacingCostVia_eol_helper(box, testBox, type, isUpperVia, i, j, z);

        testBox.set(tmpBx.left() - eolWithin, tmpBx.bottom() - eolSpace, tmpBx.right() + eolWithin, tmpBx.bottom());
        modMinSpacingCostVia_eol_helper(box, testBox, type, isUpperVia, i, j, z);
      }
      // eol to left and right
      if (tmpBx.top() - tmpBx.bottom() < eolWidth) {
        testBox.set(tmpBx.right(),           tmpBx.bottom() - eolWithin, tmpBx.right() + eolSpace, tmpBx.top() + eolWithin);
        modMinSpacingCostVia_eol_helper(box, testBox, type, isUpperVia, i, j, z);

        testBox.set(tmpBx.left() - eolSpace, tmpBx.bottom() - eolWithin, tmpBx.left(),             tmpBx.top() + eolWithin);
        modMinSpacingCostVia_eol_helper(box, testBox, type, isUpperVia, i, j, z);
      }
    }
  }
}

void FlexDRWorker::modMinimumcutCostVia(const frBox &box, frMIdx z, int type, bool isUpperVia) {
  auto lNum = gridGraph.getLayerNum(z);
  frCoord width1  = box.width();
  frCoord length1 = box.length();
  // default via dimension
  frViaDef* viaDef = nullptr;
  if (isUpperVia) {
    viaDef = (lNum < getDesign()->getTech()->getTopLayerNum()) ? 
             getDesign()->getTech()->getLayer(lNum+1)->getDefaultViaDef() : 
             nullptr;
  } else {
    viaDef = (lNum > getDesign()->getTech()->getBottomLayerNum()) ? 
             getDesign()->getTech()->getLayer(lNum-1)->getDefaultViaDef() : 
             nullptr;
  }
  if (viaDef == nullptr) {
    return;
  }
  frVia via(viaDef);
  frBox viaBox(0,0,0,0);
  if (isUpperVia) {
    via.getCutBBox(viaBox);
  } else {
    via.getCutBBox(viaBox);
  }
  
  FlexMazeIdx mIdx1, mIdx2;
  frBox bx, tmpBx, sViaBox;
  frTransform xform;
  frPoint pt;
  frCoord dx ,dy;
  frVia sVia;
  for (auto &con: getDesign()->getTech()->getLayer(lNum)->getMinimumcutConstraints()) {
    // check via2cut to box
    // check whether via can be placed on the pin
    if ((!con->hasLength() || (con->hasLength() && length1 > con->getLength())) && width1 > con->getWidth()) {
      bool checkVia2 = false;
      if (!con->hasConnection()) {
        checkVia2 = true;
      } else {
        if (con->getConnection() == frMinimumcutConnectionEnum::FROMABOVE && isUpperVia) {
          checkVia2 = true;
        } else if (con->getConnection() == frMinimumcutConnectionEnum::FROMBELOW && !isUpperVia) {
          checkVia2 = true;
        }
      }
      if (!checkVia2) {
        continue;
      }
      // block via on pin
      frCoord dist = 0;
      if (con->hasLength()) {
        dist = con->getDistance();
        // conservative for macro pin
        // TODO: revert the += to be more accurate and check qor change
        dist += getDesign()->getTech()->getLayer(lNum)->getPitch();
      }
      // assumes width always > 2
      bx.set(box.left()   - dist - (viaBox.right() - 0) + 1, 
             box.bottom() - dist - (viaBox.top() - 0) + 1,
             box.right()  + dist + (0 - viaBox.left())  - 1, 
             box.top()    + dist + (0 - viaBox.bottom()) - 1);
      gridGraph.getIdxBox(mIdx1, mIdx2, bx);

      for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
        for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
          gridGraph.getPoint(pt, i, j);
          xform.set(pt);
          tmpBx.set(viaBox);
          if (gridGraph.isSVia(i, j, isUpperVia ? z : z - 1)) {
            auto sViaDef = apSVia[FlexMazeIdx(i, j, isUpperVia ? z : z - 1)]->getAccessViaDef();
            sVia.setViaDef(sViaDef);
            if (isUpperVia) {
              sVia.getCutBBox(sViaBox);
            } else {
              sVia.getCutBBox(sViaBox);
            }
            tmpBx.set(sViaBox);
          }
          tmpBx.transform(xform);
          box2boxDistSquareNew(box, tmpBx, dx, dy);
          if (!con->hasLength()) {
            if (dx <= 0 && dy <= 0) {
              ;
            } else {
              continue;
            }
          } else {
            if (dx > 0 && dy > 0 && dx + dy < dist) {
              ;
            } else {
              continue;
            }
          }
          if (isUpperVia) {
            switch(type) {
              case 0:
                gridGraph.subDRCCostVia(i, j, z); // safe access
                break;
              case 1:
                gridGraph.addDRCCostVia(i, j, z); // safe access
                break;
              case 2:
                gridGraph.subShapeCostVia(i, j, z); // safe access
                break;
              case 3:
                gridGraph.addShapeCostVia(i, j, z); // safe access
                break;
              default:
                ;
            }
            if (QUICKDRCTEST) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U minSpc via" <<endl;
            }
          } else {
            switch(type) {
              case 0:
                gridGraph.subDRCCostVia(i, j, z - 1); // safe access
                break;
              case 1:
                gridGraph.addDRCCostVia(i, j, z - 1); // safe access
                break;
              case 2:
                gridGraph.subShapeCostVia(i, j, z - 1); // safe access
                break;
              case 3:
                gridGraph.addShapeCostVia(i, j, z - 1); // safe access
                break;
              default:
                ;
            }
            if (QUICKDRCTEST) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z - 1 <<") U minSpc via" <<endl;
            }
          }
        }
      }

    }
  }
}

void FlexDRWorker::modMinSpacingCostVia(const frBox &box, frMIdx z, int type, bool isUpperVia, bool isCurrPs, bool isBlockage) {
  auto lNum = gridGraph.getLayerNum(z);
  frCoord width1  = box.width();
  frCoord length1 = box.length();
  // default via dimension
  frViaDef* viaDef = nullptr;
  if (isUpperVia) {
    viaDef = (lNum < getDesign()->getTech()->getTopLayerNum()) ? 
             getDesign()->getTech()->getLayer(lNum+1)->getDefaultViaDef() : 
             nullptr;
  } else {
    viaDef = (lNum > getDesign()->getTech()->getBottomLayerNum()) ? 
             getDesign()->getTech()->getLayer(lNum-1)->getDefaultViaDef() : 
             nullptr;
  }
  if (viaDef == nullptr) {
    return;
  }
  frVia via(viaDef);
  frBox viaBox(0,0,0,0);
  if (isUpperVia) {
    via.getLayer1BBox(viaBox);
  } else {
    via.getLayer2BBox(viaBox);
  }
  frCoord width2  = viaBox.width();
  frCoord length2 = viaBox.length();
  
  // via prl should check min area patch metal if not fat via
  frCoord defaultWidth = getDesign()->getTech()->getLayer(lNum)->getWidth();
  bool isH = (getDesign()->getTech()->getLayer(lNum)->getDir() == frPrefRoutingDirEnum::frcHorzPrefRoutingDir);
  bool isFatVia = (isH) ? 
                  (viaBox.top() - viaBox.bottom() > defaultWidth) : 
                  (viaBox.right() - viaBox.left() > defaultWidth);

  frCoord length2_mar = length2;
  frCoord patchLength = 0;
  if (!isFatVia) {
    auto minAreaConstraint = getDesign()->getTech()->getLayer(lNum)->getAreaConstraint();
    auto minArea           = minAreaConstraint ? minAreaConstraint->getMinArea() : 0;
    patchLength            = frCoord(ceil(1.0 * minArea / defaultWidth / getDesign()->getTech()->getManufacturingGrid())) * 
                             frCoord(getDesign()->getTech()->getManufacturingGrid());
    length2_mar = max(length2_mar, patchLength);
  }

  // spacing value needed
  frCoord bloatDist = 0;
  auto con = getDesign()->getTech()->getLayer(lNum)->getMinSpacing();
  if (con) {
    if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
      bloatDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
      bloatDist = (isBlockage && USEMINSPACING_OBS && !isFatVia) ? static_cast<frSpacingTablePrlConstraint*>(con)->findMin() :
                               static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2), isCurrPs ? 
                                                                                                         (length2_mar) : 
                                                                                                         min(length1, length2_mar));
    } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
      bloatDist = (isBlockage && USEMINSPACING_OBS && !isFatVia) ? static_cast<frSpacingTableTwConstraint*>(con)->findMin() :
                               static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2, isCurrPs ? 
                                                                                                   (length2_mar) : 
                                                                                                   min(length1, length2_mar));
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: min spacing rule not supporterd" <<endl;
      return;
    }
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: no min spacing rule" <<endl;
    return;
  }
  // other obj eol spc to curr obj
  // no need to bloat eolWithin because eolWithin always < minSpacing
  frCoord bloatDistEolX = 0;
  frCoord bloatDistEolY = 0;
  for (auto con: getDesign()->getTech()->getLayer(lNum)->getEolSpacing()) {
    auto eolSpace  = con->getMinSpacing();
    auto eolWidth  = con->getEolWidth();
    // eol up and down
    if (viaBox.right() - viaBox.left() < eolWidth) {
      bloatDistEolY = max(bloatDistEolY, eolSpace);
    } 
    // eol left and right
    if (viaBox.top() - viaBox.bottom() < eolWidth) {
      bloatDistEolX = max(bloatDistEolX, eolSpace);
    }
  }
  //frCoord bloatDistSquare = bloatDist * bloatDist;
  
  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  // assumes width always > 2
  frBox bx(box.left()   - max(bloatDist, bloatDistEolX) - (viaBox.right() - 0) + 1, 
             box.bottom() - max(bloatDist, bloatDistEolY) - (viaBox.top() - 0) + 1,
           box.right()  + max(bloatDist, bloatDistEolX) + (0 - viaBox.left())  - 1, 
             box.top()    + max(bloatDist, bloatDistEolY) + (0 - viaBox.bottom()) - 1);
  gridGraph.getIdxBox(mIdx1, mIdx2, bx);

  frPoint pt;
  frBox tmpBx;
  frCoord distSquare = 0;
  frCoord dx, dy, prl;
  frTransform xform;
  frCoord reqDist = 0;
  frBox sViaBox;
  frVia sVia;
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      gridGraph.getPoint(pt, i, j);
      xform.set(pt);
      tmpBx.set(viaBox);
      if (gridGraph.isSVia(i, j, isUpperVia ? z : z - 1)) {
        auto sViaDef = apSVia[FlexMazeIdx(i, j, isUpperVia ? z : z - 1)]->getAccessViaDef();
        sVia.setViaDef(sViaDef);
        if (isUpperVia) {
          sVia.getLayer1BBox(sViaBox);
        } else {
          sVia.getLayer2BBox(sViaBox);
        }
        tmpBx.set(sViaBox);
      }
      tmpBx.transform(xform);
      distSquare = box2boxDistSquareNew(box, tmpBx, dx, dy);
      prl = max(-dx, -dy);
      // curr is ps
      if (isCurrPs) {
        if (-dy >= 0 && prl == -dy) {
          prl = viaBox.top() - viaBox.bottom();
          // ignore svia effect here...
          if (!isH && !isFatVia) {
            prl = max(prl, patchLength);
          }
        } else if (-dx >= 0 && prl == -dx) {
          prl = viaBox.right() - viaBox.left();
          if (isH && !isFatVia) {
            prl = max(prl, patchLength);
          }
        }
      } else {
        ;
      }
      if (con->typeId() == frConstraintTypeEnum::frcSpacingConstraint) {
        reqDist = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
      } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTablePrlConstraint) {
        reqDist = (isBlockage && USEMINSPACING_OBS && !isFatVia) ? static_cast<frSpacingTablePrlConstraint*>(con)->findMin() :
                               static_cast<frSpacingTablePrlConstraint*>(con)->find(max(width1, width2), prl);
      } else if (con->typeId() == frConstraintTypeEnum::frcSpacingTableTwConstraint) {
        reqDist = (isBlockage && USEMINSPACING_OBS && !isFatVia) ? static_cast<frSpacingTableTwConstraint*>(con)->findMin() :
                               static_cast<frSpacingTableTwConstraint*>(con)->find(width1, width2, prl);
      }
      if (distSquare < reqDist * reqDist) {
        if (isUpperVia) {
          switch(type) {
            case 0:
              gridGraph.subDRCCostVia(i, j, z); // safe access
              break;
            case 1:
              gridGraph.addDRCCostVia(i, j, z); // safe access
              break;
            case 2:
              gridGraph.subShapeCostVia(i, j, z); // safe access
              break;
            case 3:
              gridGraph.addShapeCostVia(i, j, z); // safe access
              break;
            default:
              ;
          }
          if (QUICKDRCTEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U minSpc via" <<endl;
          }
        } else {
          switch(type) {
            case 0:
              gridGraph.subDRCCostVia(i, j, z - 1); // safe access
              break;
            case 1:
              gridGraph.addDRCCostVia(i, j, z - 1); // safe access
              break;
            case 2:
              gridGraph.subShapeCostVia(i, j, z - 1); // safe access
              break;
            case 3:
              gridGraph.addShapeCostVia(i, j, z - 1); // safe access
              break;
            default:
              ;
          }
          if (QUICKDRCTEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z - 1 <<") U minSpc via" <<endl;
          }
        }
      }
      // eol, other obj to curr obj
      modMinSpacingCostVia_eol(box, tmpBx, type, isUpperVia, i, j, z);
    }
  }

}


// eolType == 0: planer
// eolType == 1: down
// eolType == 2: up
/*inline*/ void FlexDRWorker::modEolSpacingCost_helper(const frBox &testbox, frMIdx z, int type, int eolType) {
  auto lNum = gridGraph.getLayerNum(z);
  frBox bx;
  if (eolType == 0) {
    // layer default width
    frCoord width2     = getDesign()->getTech()->getLayer(lNum)->getWidth();
    frCoord halfwidth2 = width2 / 2;
    // assumes width always > 2
    bx.set(testbox.left()   - halfwidth2 + 1, testbox.bottom() - halfwidth2 + 1,
           testbox.right()  + halfwidth2 - 1, testbox.top()    + halfwidth2 - 1);
  } else {
    // default via dimension
    frViaDef* viaDef = nullptr;
    if (eolType == 1) {
      viaDef = (lNum > getDesign()->getTech()->getBottomLayerNum()) ? 
               getDesign()->getTech()->getLayer(lNum-1)->getDefaultViaDef() : 
               nullptr;
    } else if (eolType == 2) {
      viaDef = (lNum < getDesign()->getTech()->getTopLayerNum()) ? 
               getDesign()->getTech()->getLayer(lNum+1)->getDefaultViaDef() : 
               nullptr;
    }
    if (viaDef == nullptr) {
      return;
    }
    frVia via(viaDef);
    frBox viaBox(0,0,0,0);
    if (eolType == 2) { // upper via
      via.getLayer1BBox(viaBox);
    } else {
      via.getLayer2BBox(viaBox);
    }
    // assumes via bbox always > 2
    bx.set(testbox.left()  - (viaBox.right() - 0) + 1, testbox.bottom() - (viaBox.top() - 0) + 1,
           testbox.right() + (0 - viaBox.left())  - 1, testbox.top()    + (0 - viaBox.bottom()) - 1);
  }

  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  gridGraph.getIdxBox(mIdx1, mIdx2, bx); // >= bx

  frVia sVia;
  frBox sViaBox;
  frTransform xform;
  frPoint pt;
  
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      if (eolType == 0) {
        switch(type) {
          case 0:
            gridGraph.subDRCCostPlanar(i, j, z); // safe access
            break;
          case 1:
            gridGraph.addDRCCostPlanar(i, j, z); // safe access
            break;
          case 2:
            gridGraph.subShapeCostPlanar(i, j, z); // safe access
            break;
          case 3:
            gridGraph.addShapeCostPlanar(i, j, z); // safe access
            break;
          default:
            ;
        }
        if (QUICKDRCTEST) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") N eolSpc" <<endl;
        }
      } else if (eolType == 1) {
        if (gridGraph.isSVia(i, j, z - 1)) {
          gridGraph.getPoint(pt, i, j);
          auto sViaDef = apSVia[FlexMazeIdx(i, j, z - 1)]->getAccessViaDef();
          sVia.setViaDef(sViaDef);
          sVia.setOrigin(pt);
          sVia.getLayer2BBox(sViaBox);
          if (!sViaBox.overlaps(testbox, false)) {
            continue;
          }
        }
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(i, j, z - 1); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(i, j, z - 1); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(i, j, z - 1); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(i, j, z - 1); // safe access
            break;
          default:
            ;
        }
        if (QUICKDRCTEST) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z - 1 <<") U eolSpc" <<endl;
        }
      } else if (eolType == 2) {
        if (gridGraph.isSVia(i, j, z)) {
          gridGraph.getPoint(pt, i, j);
          auto sViaDef = apSVia[FlexMazeIdx(i, j, z)]->getAccessViaDef();
          sVia.setViaDef(sViaDef);
          sVia.setOrigin(pt);
          sVia.getLayer1BBox(sViaBox);
          if (!sViaBox.overlaps(testbox, false)) {
            continue;
          }
        }
        switch(type) {
          case 0:
            gridGraph.subDRCCostVia(i, j, z); // safe access
            break;
          case 1:
            gridGraph.addDRCCostVia(i, j, z); // safe access
            break;
          case 2:
            gridGraph.subShapeCostVia(i, j, z); // safe access
            break;
          case 3:
            gridGraph.addShapeCostVia(i, j, z); // safe access
            break;
          default:
            ;
        }
        if (QUICKDRCTEST) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U eolSpc" <<endl;
        }
      }
    }
  }
}

void FlexDRWorker::modEolSpacingCost(const frBox &box, frMIdx z, int type, bool isSkipVia) {
  auto lNum = gridGraph.getLayerNum(z);
  frBox testBox;
  if (getDesign()->getTech()->getLayer(lNum)->hasEolSpacing()) {
    for (auto con: getDesign()->getTech()->getLayer(lNum)->getEolSpacing()) {
      auto eolSpace  = con->getMinSpacing();
      auto eolWidth  = con->getEolWidth();
      auto eolWithin = con->getEolWithin();
      // eol to up and down
      if (box.right() - box.left() < eolWidth) {
        testBox.set(box.left() - eolWithin, box.top(),               box.right() + eolWithin, box.top() + eolSpace);
        //if (!isInitDR()) {
        modEolSpacingCost_helper(testBox, z, type, 0);
        if (!isSkipVia) {
          modEolSpacingCost_helper(testBox, z, type, 1);
          modEolSpacingCost_helper(testBox, z, type, 2);
        }
        testBox.set(box.left() - eolWithin, box.bottom() - eolSpace, box.right() + eolWithin, box.bottom());
        modEolSpacingCost_helper(testBox, z, type, 0);
        if (!isSkipVia) {
          modEolSpacingCost_helper(testBox, z, type, 1);
          modEolSpacingCost_helper(testBox, z, type, 2);
        }
      }
      // eol to left and right
      if (box.top() - box.bottom() < eolWidth) {
        testBox.set(box.right(),           box.bottom() - eolWithin, box.right() + eolSpace, box.top() + eolWithin);
        modEolSpacingCost_helper(testBox, z, type, 0);
        if (!isSkipVia) {
          modEolSpacingCost_helper(testBox, z, type, 1);
          modEolSpacingCost_helper(testBox, z, type, 2);
        }
        testBox.set(box.left() - eolSpace, box.bottom() - eolWithin, box.left(),             box.top() + eolWithin);
        modEolSpacingCost_helper(testBox, z, type, 0);
        if (!isSkipVia) {
          modEolSpacingCost_helper(testBox, z, type, 1);
          modEolSpacingCost_helper(testBox, z, type, 2);
        }
      }
      // other obj to curr obj eol
    }
  }
}

// forbid via if it would trigger violation
void FlexDRWorker::modAdjCutSpacingCost_fixedObj(const frBox &origCutBox, frVia *origVia) {
  if (origVia->getNet()->getType() != frNetEnum::frcPowerNet && origVia->getNet()->getType() != frNetEnum::frcGroundNet) {
    return;
  }
  auto lNum = origVia->getViaDef()->getCutLayerNum();
  for (auto con: getDesign()->getTech()->getLayer(lNum)->getCutSpacing()) {
    if (con->getAdjacentCuts() == -1) {
      continue;
    }
    bool hasFixedViol = false;

    gtl::point_data<frCoord> origCenter((origCutBox.left() + origCutBox.right()) / 2, (origCutBox.bottom() + origCutBox.top()) / 2);
    gtl::rectangle_data<frCoord> origCutRect(origCutBox.left(), origCutBox.bottom(), origCutBox.right(), origCutBox.top());

    frBox viaBox;
    origVia->getCutBBox(viaBox);

    auto reqDistSquare = con->getCutSpacing();
    reqDistSquare *= reqDistSquare;

    auto cutWithin = con->getCutWithin();
    frBox queryBox;
    viaBox.bloat(cutWithin, queryBox);

    frRegionQuery::Objects<frBlockObject> result;
    getRegionQuery()->query(queryBox, lNum, result);

    for (auto &[box, obj]:result) {
      if (obj->typeId() == frcVia) {
        auto via = static_cast<frVia*>(obj);
        if (via->getNet()->getType() != frNetEnum::frcPowerNet && via->getNet()->getType() != frNetEnum::frcGroundNet) {
          continue;
        }
        if (origCutBox == box) {
          continue;
        }

        gtl::rectangle_data<frCoord> cutRect(box.left(), box.bottom(), box.right(), box.top());
        gtl::point_data<frCoord> cutCenterPt((box.left() + box.right()) / 2, (box.bottom() + box.top()) / 2);

        frCoord distSquare = 0;
        if (con->hasCenterToCenter()) {
          distSquare = gtl::distance_squared(origCenter, cutCenterPt);
        } else {
          distSquare = gtl::square_euclidean_distance(origCutRect, cutRect);
        }

        if (distSquare < reqDistSquare) {
          hasFixedViol = true;
          break;
        }
      }
    }

    // block adjacent via idx if will trigger violation
    // pessimistic since block a box
    if (hasFixedViol) {
      FlexMazeIdx mIdx1, mIdx2;
      frBox spacingBox;
      auto reqDist = con->getCutSpacing();
      auto cutWidth = getDesign()->getTech()->getLayer(lNum)->getWidth();
      if (con->hasCenterToCenter()) {
        spacingBox.set(origCenter.x() - reqDist, origCenter.y() - reqDist, origCenter.x() + reqDist, origCenter.y() + reqDist);
      } else {
        origCutBox.bloat(reqDist + cutWidth / 2, spacingBox);
      }
      // cout << __FILE__ << ":" << __LINE__ << ": " << "  @@@ debug: blocking for adj (" << spacingBox.left() / 2000.0 << ", " << spacingBox.bottom() / 2000.0
      //      << ") -- (" << spacingBox.right() / 2000.0 << ", " << spacingBox.top() / 2000.0 << ")\n";
      gridGraph.getIdxBox(mIdx1, mIdx2, spacingBox);

      frMIdx zIdx = gridGraph.getMazeZIdx(origVia->getViaDef()->getLayer1Num());
      for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
        for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
          gridGraph.setBlocked(i, j, zIdx, frDirEnum::U);
        }
      }
    }

  }
}


/*inline*/ void FlexDRWorker::modCutSpacingCost(const frBox &box, frMIdx z, int type, bool isBlockage) {
  auto lNum = gridGraph.getLayerNum(z) + 1;
  if (!getDesign()->getTech()->getLayer(lNum)->hasCutSpacing()) {
    return;
  }
  // obj1 = curr obj
  // obj2 = other obj
  // default via dimension
  frViaDef* viaDef = getDesign()->getTech()->getLayer(lNum)->getDefaultViaDef();
  frVia via(viaDef);
  frBox viaBox(0,0,0,0);
  via.getCutBBox(viaBox);

  // spacing value needed
  frCoord bloatDist = 0;
  for (auto con: getDesign()->getTech()->getLayer(lNum)->getCutSpacing()) {
    bloatDist = max(bloatDist, con->getCutSpacing());
    if (con->getAdjacentCuts() != -1 && isBlockage) {
      bloatDist = max(bloatDist, con->getCutWithin());
    }
  }
  //frCoord bloatDistSquare = bloatDist * bloatDist;
  
  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  // assumes width always > 2
  frBox bx(box.left()   - bloatDist - (viaBox.right() - 0) + 1, 
           box.bottom() - bloatDist - (viaBox.top() - 0) + 1,
           box.right()  + bloatDist + (0 - viaBox.left())  - 1, 
           box.top()    + bloatDist + (0 - viaBox.bottom()) - 1);
  gridGraph.getIdxBox(mIdx1, mIdx2, bx);

  frPoint pt;
  frBox tmpBx;
  frCoord distSquare = 0;
  frCoord c2cSquare = 0;
  frCoord dx, dy, prl;
  frTransform xform;
  //frCoord reqDist = 0;
  frCoord reqDistSquare = 0;
  frPoint boxCenter, tmpBxCenter;
  boxCenter.set((box.left() + box.right()) / 2, (box.bottom() + box.top()) / 2);
  frCoord currDistSquare = 0;
  bool hasViol = false;
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      for (auto &uFig: via.getViaDef()->getCutFigs()) {
        auto obj = static_cast<frRect*>(uFig.get());
        gridGraph.getPoint(pt, i, j);
        xform.set(pt);
        obj->getBBox(tmpBx);
        tmpBx.transform(xform);
        tmpBxCenter.set((tmpBx.left() + tmpBx.right()) / 2, (tmpBx.bottom() + tmpBx.top()) / 2);
        distSquare = box2boxDistSquareNew(box, tmpBx, dx, dy);
        c2cSquare = (boxCenter.x() - tmpBxCenter.x()) * (boxCenter.x() - tmpBxCenter.x()) + 
                    (boxCenter.y() - tmpBxCenter.y()) * (boxCenter.y() - tmpBxCenter.y()); 
        prl = max(-dx, -dy);
        for (auto con: getDesign()->getTech()->getLayer(lNum)->getCutSpacing()) {
          hasViol = false;
          reqDistSquare = con->getCutSpacing() * con->getCutSpacing();
          currDistSquare = con->hasCenterToCenter() ? c2cSquare : distSquare;
          if (con->hasSameNet()) {
            continue;
          }
          if (con->isLayer()) {
            ;
          } else if (con->isAdjacentCuts()) {
            // OBS always count as within distance instead of cut spacing
            if (isBlockage) {
              reqDistSquare = con->getCutWithin() * con->getCutWithin();
            }
            if (currDistSquare < reqDistSquare) {
              hasViol = true;
              // should disable hasViol and modify this part to new grid graph
            }
          } else if (con->isParallelOverlap()) {
            if (prl > 0 && currDistSquare < reqDistSquare) {
              hasViol = true;
            }
          } else if (con->isArea()) {
            auto currArea = max(box.length() * box.width(), tmpBx.length() * tmpBx.width());
            if (currArea >= con->getCutArea() && currDistSquare < reqDistSquare) {
              hasViol = true;
            }
          } else {
            if (currDistSquare < reqDistSquare) {
              hasViol = true;
            }
          }
          if (hasViol) {
            switch(type) {
              case 0:
                gridGraph.subDRCCostVia(i, j, z); // safe access
                break;
              case 1:
                gridGraph.addDRCCostVia(i, j, z); // safe access
                break;
              case 2:
                gridGraph.subShapeCostVia(i, j, z); // safe access
                break;
              case 3:
                gridGraph.addShapeCostVia(i, j, z); // safe access
                break;
              default:
                ;
            }
            if (QUICKDRCTEST) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U cutSpc" <<endl;
            }
            break;
          }
        }
      }
    }
  }
}

void FlexDRWorker::modInterLayerCutSpacingCost(const frBox &box, frMIdx z, int type, bool isUpperVia, bool isBlockage) {
  auto cutLayerNum1 = gridGraph.getLayerNum(z) + 1;
  auto cutLayerNum2 = isUpperVia ? cutLayerNum1 + 2 : cutLayerNum1 - 2;
  auto z2 = isUpperVia ? z + 1 : z - 1;

  frViaDef* viaDef = nullptr;
  if (isUpperVia) {
    viaDef = (cutLayerNum2 <= getDesign()->getTech()->getTopLayerNum()) ? 
             getDesign()->getTech()->getLayer(cutLayerNum2)->getDefaultViaDef() : 
             nullptr;
  } else {
    viaDef = (cutLayerNum2 >= getDesign()->getTech()->getBottomLayerNum()) ? 
             getDesign()->getTech()->getLayer(cutLayerNum2)->getDefaultViaDef() : 
             nullptr;
  }
  if (viaDef == nullptr) {
    return;
  }
  frCutSpacingConstraint* con = getDesign()->getTech()->getLayer(cutLayerNum1)->getInterLayerCutSpacing(cutLayerNum2, false);
  if (con == nullptr) {
    con = getDesign()->getTech()->getLayer(cutLayerNum2)->getInterLayerCutSpacing(cutLayerNum1, false);
  }
  if (con == nullptr) {
    return;
  }
  // obj1 = curr obj
  // obj2 = other obj
  // default via dimension
  frVia via(viaDef);
  frBox viaBox(0,0,0,0);
  via.getCutBBox(viaBox);

  // spacing value needed
  frCoord bloatDist = con->getCutSpacing();
  
  FlexMazeIdx mIdx1;
  FlexMazeIdx mIdx2;
  // assumes width always > 2
  frBox bx(box.left()   - bloatDist - (viaBox.right() - 0) + 1, 
           box.bottom() - bloatDist - (viaBox.top() - 0) + 1,
           box.right()  + bloatDist + (0 - viaBox.left())  - 1, 
           box.top()    + bloatDist + (0 - viaBox.bottom()) - 1);
  gridGraph.getIdxBox(mIdx1, mIdx2, bx);

  frPoint pt;
  frBox tmpBx;
  frCoord distSquare = 0;
  frCoord c2cSquare = 0;
  frCoord dx, dy/*, prl*/;
  frTransform xform;
  frCoord reqDistSquare = 0;
  frPoint boxCenter, tmpBxCenter;
  boxCenter.set((box.left() + box.right()) / 2, (box.bottom() + box.top()) / 2);
  frCoord currDistSquare = 0;
  bool hasViol = false;
  for (int i = mIdx1.x(); i <= mIdx2.x(); i++) {
    for (int j = mIdx1.y(); j <= mIdx2.y(); j++) {
      for (auto &uFig: via.getViaDef()->getCutFigs()) {
        auto obj = static_cast<frRect*>(uFig.get());
        gridGraph.getPoint(pt, i, j);
        xform.set(pt);
        obj->getBBox(tmpBx);
        tmpBx.transform(xform);
        tmpBxCenter.set((tmpBx.left() + tmpBx.right()) / 2, (tmpBx.bottom() + tmpBx.top()) / 2);
        distSquare = box2boxDistSquareNew(box, tmpBx, dx, dy);
        c2cSquare = (boxCenter.x() - tmpBxCenter.x()) * (boxCenter.x() - tmpBxCenter.x()) + 
                    (boxCenter.y() - tmpBxCenter.y()) * (boxCenter.y() - tmpBxCenter.y()); 
        hasViol = false;
        reqDistSquare = con->getCutSpacing() * con->getCutSpacing();
        currDistSquare = con->hasCenterToCenter() ? c2cSquare : distSquare;
        if (currDistSquare < reqDistSquare) {
          hasViol = true;
        }
        if (hasViol) {
          switch(type) {
            case 0:
              gridGraph.subDRCCostVia(i, j, z2); // safe access
              break;
            case 1:
              gridGraph.addDRCCostVia(i, j, z2); // safe access
              break;
            case 2:
              gridGraph.subShapeCostVia(i, j, z2); // safe access
              break;
            case 3:
              gridGraph.addShapeCostVia(i, j, z2); // safe access
              break;
            default:
              ;
          }
          if (QUICKDRCTEST) {
            if (isUpperVia) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") U inter layer cutSpc" <<endl;
            } else {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<i <<", " <<j <<", " <<z <<") D inter layer cutSpc" <<endl;
            }
          }
          break;
        }
      }
    }
  }
}

void FlexDRWorker::addPathCost(drConnFig *connFig) {
  modPathCost(connFig, 1);
}

void FlexDRWorker::subPathCost(drConnFig *connFig) {
  modPathCost(connFig, 0);
}

void FlexDRWorker::modPathCost(drConnFig *connFig, int type) {
  if (connFig->typeId() == drcPathSeg) {
    auto obj = static_cast<drPathSeg*>(connFig);
    FlexMazeIdx bi, ei;
    obj->getMazeIdx(bi, ei);
    if (QUICKDRCTEST) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  ";
      if (type) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"add";
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"sub";
      }
      cout << __FILE__ << ":" << __LINE__ << ": " <<"PsCost for " <<bi <<" -- " <<ei <<endl;
    }
    // new 
    frBox box;
    obj->getBBox(box);
    
    modMinSpacingCostPlaner(box, bi.z(), type);
    modMinSpacingCostVia(box, bi.z(), type, true,  true);
    modMinSpacingCostVia(box, bi.z(), type, false, true);
    modViaForbiddenThrough(bi, ei, type);
    // wrong way wire cannot have eol problem: (1) with via at end, then via will add eol cost; (2) with pref-dir wire, then not eol edge
    bool isHLayer = (getDesign()->getTech()->getLayer(gridGraph.getLayerNum(bi.z()))->getDir() == frPrefRoutingDirEnum::frcHorzPrefRoutingDir);
    if (isHLayer == (bi.y() == ei.y())) {
      modEolSpacingCost(box, bi.z(), type);
    }
  } else if (connFig->typeId() == drcPatchWire) {
    auto obj = static_cast<drPatchWire*>(connFig);
    frMIdx zIdx = gridGraph.getMazeZIdx(obj->getLayerNum());
    // FlexMazeIdx bi, ei;
    // obj->getMazeIdx(bi, ei);
    // if (TEST) {
    //   cout << __FILE__ << ":" << __LINE__ << ": " <<"  ";
    //   if (isAddPathCost) {
    //     cout << __FILE__ << ":" << __LINE__ << ": " <<"add";
    //   } else {
    //     cout << __FILE__ << ":" << __LINE__ << ": " <<"sub";
    //   }
    //   cout << __FILE__ << ":" << __LINE__ << ": " <<"PsCost for " <<bi <<" -- " <<ei <<endl;
    // }
    // new 
    frBox box;
    obj->getBBox(box);
    
    modMinSpacingCostPlaner(box, zIdx, type);
    modMinSpacingCostVia(box, zIdx, type, true,  true);
    modMinSpacingCostVia(box, zIdx, type, false, true);
    modEolSpacingCost(box, zIdx, type);
  } else if (connFig->typeId() == drcVia) {
    auto obj = static_cast<drVia*>(connFig);
    FlexMazeIdx bi, ei;
    obj->getMazeIdx(bi, ei);
    if (QUICKDRCTEST) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  ";
      if (type) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"add";
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"sub";
      }
      cout << __FILE__ << ":" << __LINE__ << ": " <<"ViaCost for " <<bi <<" -- " <<ei <<endl;
    }
    // new
    
    frBox box;
    obj->getLayer1BBox(box); // assumes enclosure for via is always rectangle

    modMinSpacingCostPlaner(box, bi.z(), type);
    modMinSpacingCostVia(box, bi.z(), type, true,  false);
    modMinSpacingCostVia(box, bi.z(), type, false, false);
    modEolSpacingCost(box, bi.z(), type);
    
    obj->getLayer2BBox(box); // assumes enclosure for via is always rectangle

    modMinSpacingCostPlaner(box, ei.z(), type);
    modMinSpacingCostVia(box, ei.z(), type, true,  false);
    modMinSpacingCostVia(box, ei.z(), type, false, false);
    modEolSpacingCost(box, ei.z(), type);

    frTransform xform;
    frPoint pt;
    obj->getOrigin(pt);
    xform.set(pt);
    for (auto &uFig: obj->getViaDef()->getCutFigs()) {
      auto rect = static_cast<frRect*>(uFig.get());
      rect->getBBox(box);
      box.transform(xform);
      modCutSpacingCost(box, bi.z(), type);
      modInterLayerCutSpacingCost(box, bi.z(), type, true);
      modInterLayerCutSpacingCost(box, bi.z(), type, false);
    }
  }
}

bool FlexDRWorker::mazeIterInit_sortRerouteNets(int mazeIter, vector<drNet*> &rerouteNets) {
  auto rerouteNetsComp1 = [](drNet* const &a, drNet* const &b) {
                         frBox boxA, boxB;
                         a->getPinBox(boxA);
                         b->getPinBox(boxB);
                         auto areaA = (boxA.right() - boxA.left()) * (boxA.top() - boxA.bottom());
                         auto areaB = (boxB.right() - boxB.left()) * (boxB.top() - boxB.bottom());
                         return (a->getNumPinsIn() == b->getNumPinsIn() ? (areaA == areaB ? a->getId() < b->getId() : areaA < areaB) : 
                                                          a->getNumPinsIn() < b->getNumPinsIn());
                         };
  auto rerouteNetsComp2 = [](drNet* const &a, drNet* const &b) {
                         return (a->getMarkerDist() == b->getMarkerDist() ? a->getId() < b->getId() : a->getMarkerDist() < b->getMarkerDist());
                         };
  auto rerouteNetsComp3 = [mazeIter](drNet* const &a, drNet* const &b) {
                         frBox boxA, boxB;
                         a->getPinBox(boxA);
                         b->getPinBox(boxB);
                         auto areaA = (boxA.right() - boxA.left()) * (boxA.top() - boxA.bottom());
                         auto areaB = (boxB.right() - boxB.left()) * (boxB.top() - boxB.bottom());
                         bool sol = (a->getNumPinsIn() == b->getNumPinsIn() ? (areaA == areaB ? a->getId() < b->getId() : areaA < areaB) : 
                                                          a->getNumPinsIn() < b->getNumPinsIn());
                         return (a->getMarkerDist() == b->getMarkerDist() ? sol : a->getMarkerDist() < b->getMarkerDist());
                         };
  bool sol = false;
  // sort
  if (mazeIter == 0) {
    switch(getFixMode()) {
      case 0:
      case 9: 
        sort(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp1);
        break;
      case 1:
      case 2:
        sort(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp2);
        break;
      case 3:
      case 4:
      case 5:
        sort(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp3);
        break;
      default:
        ;
    }
    // to be removed
    if (OR_SEED != -1 && rerouteNets.size() >= 2) {
      uniform_int_distribution<int> distribution(0, rerouteNets.size() - 1);
      default_random_engine generator(OR_SEED);
      int numSwap = (double)(rerouteNets.size()) * OR_K;
      for (int i = 0; i < numSwap; i++) {
        int idx = distribution(generator);
        swap(rerouteNets[idx], rerouteNets[(idx + 1) % rerouteNets.size()]);
      }
    }

    sol = true;
  } else {
    switch(getFixMode()) {
      case 0: 
        sol = next_permutation(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp1);
        break;
      case 1:
      case 2:
        break;
      case 3:
      case 4:
      case 5:
        break;
      default:
        ;
    }
    sol = true;
  }
  return sol;
}

// temporary settings to test search and repair
bool FlexDRWorker::mazeIterInit_searchRepair(int mazeIter, vector<drNet*> &rerouteNets) {
  auto &workerRegionQuery = getWorkerRegionQuery();
  int cnt = 0;
  if (mazeIter == 0) {
    if (getRipupMode() == 0) {
      for (auto &net: nets) {
        if (net->isRipup()) {
          rerouteNets.push_back(net.get());
        }
      }
    } else if (getRipupMode() == 1) {
      for (auto &net: nets) {
        rerouteNets.push_back(net.get());
      }
    }
  } else {
    if (getFixMode() == 1 || getFixMode() == 2 || getFixMode() == 3 || getFixMode() == 4 || getFixMode() == 5) {
      rerouteNets.clear();
      for (auto &net: nets) {
        if (net->isRipup()) {
          rerouteNets.push_back(net.get());
        }
      }
    }
  }
  auto sol = mazeIterInit_sortRerouteNets(mazeIter, rerouteNets);
  if (sol) {
    for (auto &net: rerouteNets) {
      net->setModified(true);
      if (net->getFrNet()) {
        net->getFrNet()->setModified(true);
      }
      net->setNumMarkers(0);
      for (auto &uConnFig: net->getRouteConnFigs()) {
        subPathCost(uConnFig.get());
        workerRegionQuery.remove(uConnFig.get()); // worker region query
        cnt++;
      }
      // add via access cost when net is not routed
      if (RESERVE_VIA_ACCESS) {
        initMazeCost_via_helper(net, true);
      }
      net->clear();
    }
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"sr sub " <<cnt <<" connfig costs" <<endl;
  return sol;
}

void FlexDRWorker::mazeIterInit_drcCost() {
  switch(getFixMode()) {
    case 0:
      break;
    case 1:
    case 2:
      workerDRCCost *= 2;
      break;
    default:
      ;
  }
}

void FlexDRWorker::mazeIterInit_resetRipup() {
  if (getFixMode() == 1 || getFixMode() == 2 || getFixMode() == 3 || getFixMode() == 4 || getFixMode() == 5) {
    for (auto &net: nets) {
      net->resetRipup();
      net->resetMarkerDist();
    }
  }
}

bool FlexDRWorker::mazeIterInit(int mazeIter, vector<drNet*> &rerouteNets) {
  mazeIterInit_resetRipup(); // reset net ripup
  initMazeCost_marker(); // add marker cost, set net ripup
  mazeIterInit_drcCost();
  return mazeIterInit_searchRepair(mazeIter, rerouteNets); //add to rerouteNets
}

void FlexDRWorker::route_2_init_getNets_sort(vector<drNet*> &rerouteNets) {
  auto rerouteNetsComp1 = [](drNet* const &a, drNet* const &b) {
                         frBox boxA, boxB;
                         a->getPinBox(boxA);
                         b->getPinBox(boxB);
                         auto areaA = (boxA.right() - boxA.left()) * (boxA.top() - boxA.bottom());
                         auto areaB = (boxB.right() - boxB.left()) * (boxB.top() - boxB.bottom());
                         return (a->getNumPinsIn() == b->getNumPinsIn() ? (areaA == areaB ? a->getId() < b->getId() : areaA < areaB) : 
                                                          a->getNumPinsIn() < b->getNumPinsIn());
                         };
  auto rerouteNetsComp2 = [](drNet* const &a, drNet* const &b) {
                         return (a->getMarkerDist() == b->getMarkerDist() ? a->getId() < b->getId() : a->getMarkerDist() < b->getMarkerDist());
                         };
  // sort
  if (getRipupMode() == 1) {
    sort(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp1);
  } else {
    sort(rerouteNets.begin(), rerouteNets.end(), rerouteNetsComp2);
  }
}



void FlexDRWorker::route_2_init_getNets(vector<drNet*> &tmpNets) {
  initMazeCost_marker();
  for (auto &net: nets) {
    if (getRipupMode() == 1 || net->isRipup()) {
      tmpNets.push_back(net.get());
    }
  }
  route_2_init_getNets_sort(tmpNets);
}

void FlexDRWorker::route_2_ripupNet(drNet* net) {
  auto &workerRegionQuery = getWorkerRegionQuery();
  for (auto &uConnFig: net->getRouteConnFigs()) {
    subPathCost(uConnFig.get()); // sub quick drc cost
    workerRegionQuery.remove(uConnFig.get()); // sub worker region query
  }
  // add via access cost when net is not routed
  if (RESERVE_VIA_ACCESS) {
    initMazeCost_via_helper(net, true);
  }
  net->clear(); // delete connfigs
}


void FlexDRWorker::route_2_pushNet(deque<drNet*> &rerouteNets, drNet* net, bool ripUp, bool isPushFront) {
  if (net->isInQueue() || net->getNumReroutes() >= getMazeEndIter()) {
    return;
  }
  if (isPushFront) {
    rerouteNets.push_front(net);
  } else {
    rerouteNets.push_back(net);
  }
  if (ripUp) {
    route_2_ripupNet(net);
  }
  net->setInQueue();
}

drNet* FlexDRWorker::route_2_popNet(deque<drNet*> &rerouteNets) {
  auto net = rerouteNets.front();
  rerouteNets.pop_front();
  if (net->isRouted()) {
    route_2_ripupNet(net);
  }
  net->resetInQueue();
  if (net->getNumReroutes() < getMazeEndIter()) {
    net->addNumReroutes();
    net->setRouted();
  } else {
    net = nullptr;
  }
  return net;
}

void FlexDRWorker::route_2_init(deque<drNet*> &rerouteNets) {
  vector<drNet*> tmpNets;
  route_2_init_getNets(tmpNets);
  for (auto &net: tmpNets) {
    route_2_pushNet(rerouteNets, net, true);
  }
}



void FlexDRWorker::mazeNetInit(drNet* net) {
  gridGraph.resetStatus();
  // sub term / instterm cost when net is about to route
  initMazeCost_terms(net->getFrNetTerms(), false, true);
  // sub via access cost when net is about to route
  // route_queue does not need to reserve
  if (getFixMode() < 9 && RESERVE_VIA_ACCESS) {
    initMazeCost_via_helper(net, false);
  }
  if (isFollowGuide()) {
    initMazeCost_guide_helper(net, true);
  }
  // add minimum cut cost from objs in ext ring when the net is about to route
  initMazeCost_minCut_helper(net, true);
  initMazeCost_ap_helper(net, false);
  initMazeCost_boundary_helper(net, false);
}

void FlexDRWorker::mazeNetEnd(drNet* net) {
  // add term / instterm cost back when net is about to end
  initMazeCost_terms(net->getFrNetTerms(), true, true);
  if (isFollowGuide()) {
    initMazeCost_guide_helper(net, false);
  }
  // sub minimum cut cost from vias in ext ring when the net is about to end
  initMazeCost_minCut_helper(net, false);
  initMazeCost_ap_helper(net, true);
  initMazeCost_boundary_helper(net, true);
}

void FlexDRWorker::route_drc() {
  using namespace std::chrono;
  // new gcWorker
  FlexGCWorker gcWorker(getDesign(), this);
  gcWorker.setExtBox(getExtBox());
  gcWorker.setDrcBox(getDrcBox());
  gcWorker.setEnableSurgicalFix(true);
  high_resolution_clock::time_point t0x = high_resolution_clock::now();
  gcWorker.init();
  high_resolution_clock::time_point t1x = high_resolution_clock::now();
  gcWorker.main();
  high_resolution_clock::time_point t2x = high_resolution_clock::now();

  // write back GC patches
  for (auto &pwire: gcWorker.getPWires()) {
    auto net = pwire->getNet();
    if (!net) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Error: pwire with no net\n";
      exit(1);
    }
    auto tmpPWire = make_unique<drPatchWire>();
    tmpPWire->setLayerNum(pwire->getLayerNum());
    frPoint origin;
    pwire->getOrigin(origin);
    tmpPWire->setOrigin(origin);
    frBox box;
    pwire->getOffsetBox(box);
    tmpPWire->setOffsetBox(box);
    tmpPWire->addToNet(net);

    unique_ptr<drConnFig> tmp(std::move(tmpPWire));
    auto &workerRegionQuery = getWorkerRegionQuery();
    workerRegionQuery.add(tmp.get());
    net->addRoute(std::move(tmp));
  }

  gcWorker.end();
  setMarkers(gcWorker.getMarkers());
  high_resolution_clock::time_point t3x = high_resolution_clock::now();
  
  duration<double> time_span0x = duration_cast<duration<double>>(t1x - t0x);

  duration<double> time_span1x = duration_cast<duration<double>>(t2x - t1x);

  duration<double> time_span2x = duration_cast<duration<double>>(t3x - t2x);
  if (VERBOSE > 1) {
    stringstream ss;
    ss   <<"GC  (INIT/MAIN/END) "   <<time_span0x.count() <<" " 
                                    <<time_span1x.count() <<" "
                                    <<time_span2x.count() <<" "
                                    <<endl;
    ss <<"#viol = " <<markers.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<ss.str() <<flush;
  }
}

void FlexDRWorker::route_postRouteViaSwap() {
  auto &workerRegionQuery = getWorkerRegionQuery();
  set<FlexMazeIdx> modifiedViaIdx;
  frBox box;
  vector<drConnFig*> results;
  frPoint bp;
  FlexMazeIdx bi, ei;
  bool flag = false;
  for (auto &marker: getMarkers()) {
    results.clear();
    marker.getBBox(box);
    auto lNum = marker.getLayerNum();
    workerRegionQuery.query(box, lNum, results);
    for (auto &connFig: results) {
      if (connFig->typeId() == drcVia) {
        auto obj = static_cast<drVia*>(connFig);
        obj->getMazeIdx(bi, ei);
        obj->getOrigin(bp);
        bool condition1 = isInitDR() ? (bp.x() < getRouteBox().right()) : (bp.x() <= getRouteBox().right());
        bool condition2 = isInitDR() ? (bp.y() < getRouteBox().top())   : (bp.y() <= getRouteBox().top());
        // only swap vias when the net is marked modified
        if (bp.x() >= getRouteBox().left() && bp.y() >= getRouteBox().bottom() && condition1 && condition2 &&
            obj->getNet()->isModified()) {
          auto it = apSVia.find(bi);
          if (modifiedViaIdx.find(bi) == modifiedViaIdx.end() && it != apSVia.end()) {
            auto ap = it->second;
            if (ap->nextAccessViaDef()) {
              auto newViaDef = ap->getAccessViaDef();
              workerRegionQuery.remove(obj);
              obj->setViaDef(newViaDef);
              workerRegionQuery.add(obj);
              modifiedViaIdx.insert(bi);
              flag = true;
            }
          }
        }
      }
    }
  }
  if (flag) {
    route_drc();
  }
}

bool FlexDRWorker::route_2_x2_addHistoryCost(const frMarker &marker) {
  bool enableOutput = false;
  //bool enableOutput = true;

  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_box_value_t<drConnFig*> > results;
  frBox mBox, bloatBox;
  FlexMazeIdx mIdx1, mIdx2;

  marker.getBBox(mBox);
  auto lNum   = marker.getLayerNum();

  workerRegionQuery.query(mBox, lNum, results);
  frPoint bp, ep;
  frCoord width;
  frSegStyle segStyle;
  FlexMazeIdx objMIdx1, objMIdx2;
  bool fixable = false;
  for (auto &[objBox, connFig]: results) {
    if (connFig->typeId() == drcPathSeg) {
      auto obj = static_cast<drPathSeg*>(connFig);
      // skip if unfixable obj
      obj->getPoints(bp, ep);
      if (!(getRouteBox().contains(bp) && getRouteBox().contains(ep))) {
        continue;
      }
      fixable = true;
      // add history cost
      // get points to mark up, markup up to "width" grid points to the left and right of pathseg
      obj->getStyle(segStyle);
      width = segStyle.getWidth();
      mBox.bloat(width, bloatBox);
      gridGraph.getIdxBox(mIdx1, mIdx2, bloatBox);
      obj->getMazeIdx(objMIdx1, objMIdx2);
      bool isH = (objMIdx1.y() == objMIdx2.y());
      if (isH) {
        for (int i = max(objMIdx1.x(), mIdx1.x()); i <= min(objMIdx2.x(), mIdx2.x()); i++) {
          gridGraph.addMarkerCostPlanar(i, objMIdx1.y(), objMIdx1.z());
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"add marker cost planar @(" <<i <<", " <<objMIdx1.y() <<", " <<objMIdx1.z() <<")" <<endl;
          }
          planarHistoryMarkers.insert(FlexMazeIdx(i, objMIdx1.y(),objMIdx1.z()));
        }
      } else {
        for (int i = max(objMIdx1.y(), mIdx1.y()); i <= min(objMIdx2.y(), mIdx2.y()); i++) {
          gridGraph.addMarkerCostPlanar(objMIdx1.x(), i, objMIdx1.z());
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"add marker cost planar @(" <<objMIdx1.x() <<", " <<i <<", " <<objMIdx1.z() <<")" <<endl;
          }
          planarHistoryMarkers.insert(FlexMazeIdx(objMIdx1.x(), i, objMIdx1.z()));
        }
      }
    } else if (connFig->typeId() == drcVia) {
      auto obj = static_cast<drVia*>(connFig);
      obj->getOrigin(bp);
      // skip if unfixable obj
      if (!getRouteBox().contains(bp)) {
        continue;
      }
      fixable = true;
      // add history cost
      obj->getMazeIdx(objMIdx1, objMIdx2);
      gridGraph.addMarkerCostVia(objMIdx1.x(), objMIdx1.y(), objMIdx1.z());
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"add marker cost via @(" <<objMIdx1.x() <<", " <<objMIdx1.y() <<", " <<objMIdx1.z() <<")" <<endl;
      }
      viaHistoryMarkers.insert(objMIdx1);
    } else if (connFig->typeId() == drcPatchWire) {
      ;
    }
  }
  return fixable;
}

void FlexDRWorker::route_2_x2_ripupNets(const frMarker &marker, drNet* net) {
  bool enableOutput = false;

  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_box_value_t<drConnFig*> > results;
  frBox mBox, bloatBox;
  FlexMazeIdx mIdx1, mIdx2;

  marker.getBBox(mBox);
  auto lNum   = marker.getLayerNum();

  // ripup all nets within bloatbox
  frCoord bloatDist = 0;
  if (getDesign()->getTech()->getLayer(lNum)->getType() == frLayerTypeEnum::CUT) {
    if (getDesign()->getTech()->getTopLayerNum() >= lNum + 1 && 
        getDesign()->getTech()->getLayer(lNum + 1)->getType() == frLayerTypeEnum::ROUTING) {
      bloatDist = getDesign()->getTech()->getLayer(lNum + 1)->getWidth() * workerMarkerBloatWidth;
    } else if (getDesign()->getTech()->getBottomLayerNum() <= lNum - 1 && 
               getDesign()->getTech()->getLayer(lNum - 1)->getType() == frLayerTypeEnum::ROUTING) {
      bloatDist = getDesign()->getTech()->getLayer(lNum - 1)->getWidth() * workerMarkerBloatWidth;
    }
  } else if (getDesign()->getTech()->getLayer(lNum)->getType() == frLayerTypeEnum::ROUTING) {
    bloatDist = getDesign()->getTech()->getLayer(lNum)->getWidth() * workerMarkerBloatWidth;
  }
  mBox.bloat(bloatDist, bloatBox);
  if (enableOutput) {
    double dbu = getDesign()->getTopBlock()->getDBUPerUU();
    cout << __FILE__ << ":" << __LINE__ << ": " <<"marker @(" <<mBox.left()  / dbu <<", " <<mBox.bottom() / dbu <<") ("
                       <<mBox.right() / dbu <<", " <<mBox.top()    / dbu <<") "
         <<getDesign()->getTech()->getLayer(lNum)->getName() <<" " <<bloatDist
         <<endl;
  }

  workerRegionQuery.query(bloatBox, lNum, results);
  for (auto &[objBox, connFig]: results) {
    // for pathseg-related marker, bloat marker by half width and add marker cost planar
    if (connFig->typeId() == drcPathSeg) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"@@pathseg" <<endl;
      // update marker dist
      auto dx = max(max(objBox.left(),   mBox.left())   - min(objBox.right(), mBox.right()), 0);
      auto dy = max(max(objBox.bottom(), mBox.bottom()) - min(objBox.top(),   mBox.top()),   0);
      connFig->getNet()->updateMarkerDist(dx * dx + dy * dy);

      if (connFig->getNet() != net) {
        connFig->getNet()->setRipup();
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"ripup pathseg from " <<connFig->getNet()->getFrNet()->getName() <<endl;
        }
      }
    // for via-related marker, add marker cost via
    } else if (connFig->typeId() == drcVia) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"@@via" <<endl;
      // update marker dist
      auto dx = max(max(objBox.left(),   mBox.left())   - min(objBox.right(), mBox.right()), 0);
      auto dy = max(max(objBox.bottom(), mBox.bottom()) - min(objBox.top(),   mBox.top()),   0);
      connFig->getNet()->updateMarkerDist(dx * dx + dy * dy);

      auto obj = static_cast<drVia*>(connFig);
      obj->getMazeIdx(mIdx1, mIdx2);
      if (connFig->getNet() != net) {
        connFig->getNet()->setRipup();
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"ripup via from " <<connFig->getNet()->getFrNet()->getName() <<endl;
        }
      }
    } else if (connFig->typeId() == drcPatchWire) {
      // TODO: could add marker // for now we think the other part in the violation would not be patchWire
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupporterd dr type" <<endl;
    }
  }
}

void FlexDRWorker::route_queue() {
  // bool enableOutput = true;
  bool enableOutput = false;
  queue<RouteQueueEntry> rerouteQueue;

  if (skipRouting) {
    return;
  }

  // init GC
  FlexGCWorker gcWorker(getDesign(), this);
  gcWorker.setExtBox(getExtBox());
  gcWorker.setDrcBox(getDrcBox());
  gcWorker.init();
  gcWorker.setEnableSurgicalFix(true);
  if (needRecheck) {
    gcWorker.main();
    setMarkers(gcWorker.getMarkers());
  }

  setGCWorker(&gcWorker);

  // init net status
  route_queue_resetRipup();
  // init marker cost
  route_queue_addMarkerCost();
  // init reroute queue
  // route_queue_init_queue(rerouteNets);
  route_queue_init_queue(rerouteQueue);


  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "init. #nets in rerouteQueue = " << rerouteQueue.size() << "\n";
  }

  // route
  route_queue_main(rerouteQueue);

  // end
  gcWorker.resetTargetNet();
  gcWorker.setEnableSurgicalFix(true);
  gcWorker.main();

  // write back GC patches
  for (auto &pwire: gcWorker.getPWires()) {
    auto net = pwire->getNet();
    if (!net) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Error: pwire with no net\n";
      exit(1);
    }
    auto tmpPWire = make_unique<drPatchWire>();
    tmpPWire->setLayerNum(pwire->getLayerNum());
    frPoint origin;
    pwire->getOrigin(origin);
    tmpPWire->setOrigin(origin);
    frBox box;
    pwire->getOffsetBox(box);
    tmpPWire->setOffsetBox(box);
    tmpPWire->addToNet(net);

    unique_ptr<drConnFig> tmp(std::move(tmpPWire));
    auto &workerRegionQuery = getWorkerRegionQuery();
    workerRegionQuery.add(tmp.get());
    net->addRoute(std::move(tmp));
  }

  gcWorker.end();

  setMarkers(gcWorker.getMarkers());

  for (auto &net: nets) {
    net->setBestRouteConnFigs();
  }
  setBestMarkers();
}

void FlexDRWorker::route_queue_main(queue<RouteQueueEntry> &rerouteQueue) {
  auto &workerRegionQuery = getWorkerRegionQuery();
  while (!rerouteQueue.empty()) {
    // cout << __FILE__ << ":" << __LINE__ << ": " << "rerouteQueue size = " << rerouteQueue.size() << endl;
    auto& entry = rerouteQueue.front();
    frBlockObject* obj = entry.block;
    bool doRoute = entry.doRoute;
    int numReroute = entry.numReroute;

    rerouteQueue.pop();
    bool didRoute = false;
    bool didCheck = false;

    if (obj->typeId() == drcNet && doRoute) {
      auto net = static_cast<drNet*>(obj);
      if (numReroute != net->getNumReroutes()) {
        continue;
      }
      // init
      net->setModified(true);
      if (net->getFrNet()) {
        net->getFrNet()->setModified(true);
      }
      net->setNumMarkers(0);
      for (auto &uConnFig: net->getRouteConnFigs()) {
        subPathCost(uConnFig.get());
        workerRegionQuery.remove(uConnFig.get()); // worker region query
      }
      // route_queue need to unreserve via access if all nets are ripupped (i.e., not routed)
      // see route_queue_init_queue RESERVE_VIA_ACCESS
      // this is unreserve via 
      // via is reserved only when drWorker starts from nothing and via is reserved
      if (RESERVE_VIA_ACCESS && net->getNumReroutes() == 0 && getRipupMode() == 1) {
        initMazeCost_via_helper(net, false);
      }
      net->clear();


      // route
      mazeNetInit(net);
      bool isRouted = routeNet(net);
      if (isRouted == false) {
        frBox routeBox = getRouteBox();
        // TODO: output maze area
        cout << __FILE__ << ":" << __LINE__ << ": " << "Fatal error: Maze Route cannot find path (" << net->getFrNet()->getName() << ") in " 
             << "(" << routeBox.left() / 2000.0 << ", " << routeBox.bottom() / 2000.0
             << ") - (" << routeBox.right() / 2000.0 << ", " << routeBox.top() / 2000.0
             << "). Connectivity Changed.\n";
        if (OUT_MAZE_FILE == string("")) {
          if (VERBOSE > 0) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Waring: no output maze log specified, skipped writing maze log" <<endl;
          }
        } else {
          gridGraph.print();
        }
        exit(1);
      }
      mazeNetEnd(net);
      net->addNumReroutes();
      didRoute = true;

      // if (routeBox.left() == 462000 && routeBox.bottom() == 81100) {
      //   cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ debug net: " << net->getFrNet()->getName() << ", numPins = " << net->getNumPinsIn() << "\n";
      //   for (auto &uConnFig: net->getRouteConnFigs()) {
      //     if (uConnFig->typeId() == drcPathSeg) {
      //       auto ps = static_cast<drPathSeg*>(uConnFig.get());
      //       frPoint bp , ep;
      //       ps->getPoints(bp, ep);
      //       cout << __FILE__ << ":" << __LINE__ << ": " << "  pathseg: (" << bp.x() / 1000.0 << ", " << bp.y() / 1000.0 << ") - ("
      //                              << ep.x() / 1000.0 << ", " << ep.y() / 1000.0 << ") layerNum = " 
      //                              << ps->getLayerNum() << "\n";  
      //     }
      //   }
      //   // sanity check
      //   auto vptr = getDRNets(net->getFrNet());
      //   if (vptr) {
      //     bool isFound = false;
      //     for (auto dnet: *vptr) {
      //       if (dnet == net) {
      //         isFound = true;
      //       }
      //     }
      //     if (!isFound) {
      //       cout << __FILE__ << ":" << __LINE__ << ": " << "Error: drNet does not exist in frNet-to-drNets map\n";
      //     }
      //   }
      // }

      // gc
      if (gcWorker->setTargetNet(net->getFrNet())) {
        gcWorker->updateDRNet(net);
        gcWorker->setEnableSurgicalFix(true);
        gcWorker->main();

        // write back GC patches
        for (auto &pwire: gcWorker->getPWires()) {
          auto net = pwire->getNet();
          auto tmpPWire = make_unique<drPatchWire>();
          tmpPWire->setLayerNum(pwire->getLayerNum());
          frPoint origin;
          pwire->getOrigin(origin);
          tmpPWire->setOrigin(origin);
          frBox box;
          pwire->getOffsetBox(box);
          tmpPWire->setOffsetBox(box);
          tmpPWire->addToNet(net);

          unique_ptr<drConnFig> tmp(std::move(tmpPWire));
          auto &workerRegionQuery = getWorkerRegionQuery();
          workerRegionQuery.add(tmp.get());
          net->addRoute(std::move(tmp));
        }

        didCheck = true;
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " << "Error: fail to setTargetNet\n";
      }
    } else {
      // if (isRouteSkipped == false) {
        gcWorker->setEnableSurgicalFix(false);
        if (obj->typeId() == frcNet) {
          auto net = static_cast<frNet*>(obj);
          if (gcWorker->setTargetNet(net)) {
            gcWorker->main();
            didCheck = true;
            // cout << __FILE__ << ":" << __LINE__ << ": " << "do check " << net->getName() << "\n";
          }
        } else {
          if (gcWorker->setTargetNet(obj)) {
            gcWorker->main();
            didCheck = true;
            // cout << __FILE__ << ":" << __LINE__ << ": " << "do check\n";
          }
        }
      // }
    }

    

    // end
    if (didCheck) {
      route_queue_update_queue(gcWorker->getMarkers(), rerouteQueue);
    }
    if (didRoute) {
      route_queue_markerCostDecay();
    }
    if (didCheck) {
      route_queue_addMarkerCost(gcWorker->getMarkers());
    }
  }
}

void FlexDRWorker::route() {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "start Maze route #nets = " <<nets.size() <<endl;
  }
  if (!DRCTEST && isEnableDRC() && getRipupMode() == 0 && getInitNumMarkers() == 0) {
    return;
  }
  if (DRCTEST) {
    //DRCWorker drcWorker(getDesign(), fixedObjs);
    //drcWorker.addDRNets(nets);
    using namespace std::chrono;
    //high_resolution_clock::time_point t0 = high_resolution_clock::now();
    //drcWorker.init();
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();
    //drcWorker.setup();
    //high_resolution_clock::time_point t2 = high_resolution_clock::now();
    //// drcWorker.main();
    //drcWorker.check();
    //high_resolution_clock::time_point t3 = high_resolution_clock::now();
    //drcWorker.report();
    //
    //duration<double> time_span0 = duration_cast<duration<double>>(t1 - t0);

    //duration<double> time_span1 = duration_cast<duration<double>>(t2 - t1);

    //duration<double> time_span2 = duration_cast<duration<double>>(t3 - t2);
    //stringstream ss;
    //ss   <<"time (INIT/SETUP/MAIN) " <<time_span0.count() <<" " 
    //                                 <<time_span1.count() <<" "
    //                                 <<time_span2.count() <<" "
    //                                 <<endl;
    //cout << __FILE__ << ":" << __LINE__ << ": " <<ss.str() <<flush;
    FlexGCWorker gcWorker(getDesign(), this);
    gcWorker.setExtBox(getExtBox());
    gcWorker.setDrcBox(getDrcBox());
    high_resolution_clock::time_point t0x = high_resolution_clock::now();
    gcWorker.init();
    high_resolution_clock::time_point t1x = high_resolution_clock::now();
    gcWorker.main();
    high_resolution_clock::time_point t2x = high_resolution_clock::now();
    // drcWorker.main();
    gcWorker.end();
    setMarkers(gcWorker.getMarkers());
    high_resolution_clock::time_point t3x = high_resolution_clock::now();
    //drcWorker.report();
    
    duration<double> time_span0x = duration_cast<duration<double>>(t1x - t0x);

    duration<double> time_span1x = duration_cast<duration<double>>(t2x - t1x);

    duration<double> time_span2x = duration_cast<duration<double>>(t3x - t2x);
    if (VERBOSE > 1) {
      stringstream ss;
      ss   <<"GC  (INIT/MAIN/END) "   <<time_span0x.count() <<" " 
                                      <<time_span1x.count() <<" "
                                      <<time_span2x.count() <<" "
                                      <<endl;
      ss <<"#viol = " <<markers.size() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<ss.str() <<flush;
    }

  } else {
    vector<drNet*> rerouteNets;
    for (int i = 0; i < mazeEndIter; ++i) {
      if (!mazeIterInit(i, rerouteNets)) {
        return;
      }
      for (auto net: rerouteNets) {
        mazeNetInit(net);
        bool isRouted = routeNet(net);
        if (isRouted == false) {
          // TODO: output maze area
          cout << __FILE__ << ":" << __LINE__ << ": " << "Fatal error: Maze Route cannot find path (" << net->getFrNet()->getName() << ") in " 
             << "(" << routeBox.left() / 2000.0 << ", " << routeBox.bottom() / 2000.0
             << ") - (" << routeBox.right() / 2000.0 << ", " << routeBox.top() / 2000.0
             << "). Connectivity Changed.\n";
          cout << __FILE__ << ":" << __LINE__ << ": " << "#local pin = " << net->getPins().size() << endl;
          for (auto &pin: net->getPins()) {
            if (pin->hasFrTerm()) {
              if (pin->getFrTerm()->typeId() == frcInstTerm) {
                auto instTerm = static_cast<frInstTerm*>(pin->getFrTerm());
                cout << __FILE__ << ":" << __LINE__ << ": " << "  instTerm " << instTerm->getInst()->getName() << "/" << instTerm->getTerm()->getName() << "\n";
              } else {
                cout << __FILE__ << ":" << __LINE__ << ": " << "  term\n";
              }
            } else {
              cout << __FILE__ << ":" << __LINE__ << ": " << "  boundary pin\n";
            }
          }
          if (OUT_MAZE_FILE == string("")) {
            if (VERBOSE > 0) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<"Waring: no output maze log specified, skipped writing maze log" <<endl;
            }
          } else {
            gridGraph.print();
          }
          exit(1);
        }
        mazeNetEnd(net);
      }
      // drc worker here
      if (!rerouteNets.empty() && isEnableDRC()) {
        route_drc();
      }
      
      // quick drc
      int violNum = getNumQuickMarkers();
      if (VERBOSE > 1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"#quick viol = " <<getNumQuickMarkers() <<endl;
      }

      // save to best drc
      if (i == 0 || (isEnableDRC() && (getMarkers().size() < getBestMarkers().size() || workerMarkerBloatWidth > 0))) {
        for (auto &net: nets) {
          net->setBestRouteConnFigs();
        }
        //double dbu = getDesign()->getTopBlock()->getDBUPerUU();
        //if (getRouteBox().left()    == 63     * dbu && 
        //    getRouteBox().right()   == 84     * dbu && 
        //    getRouteBox().bottom()  == 139.65 * dbu && 
        //    getRouteBox().top()     == 159.6  * dbu) { 
        //  for (auto &net: nets) {
        //    if (net->getFrNet()->getName() == string("net144") || net->getFrNet()->getName() == string("net221")) {
        //      cout << __FILE__ << ":" << __LINE__ << ": " <<net->getFrNet()->getName() <<": " <<endl;
        //      cout << __FILE__ << ":" << __LINE__ << ": " <<"routeConnFigs" <<endl;
        //      for (auto &uConnFig: net->getRouteConnFigs()) {
        //        if (uConnFig->typeId() == drcPathSeg) {
        //          auto obj = static_cast<drPathSeg*>(uConnFig.get());
        //          frPoint bp, ep;
        //          obj->getPoints(bp, ep);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  ps (" 
        //               <<bp.x() / dbu <<", " <<bp.y() / dbu <<") ("
        //               <<ep.x() / dbu <<", " <<ep.y() / dbu <<") "
        //               <<getDesign()->getTech()->getLayer(obj->getLayerNum())->getName()
        //               <<endl;

        //        } else if (uConnFig->typeId() == drcVia) {
        //          auto obj = static_cast<drVia*>(uConnFig.get());
        //          frPoint pt;
        //          obj->getOrigin(pt);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  via (" 
        //               <<pt.x() / dbu <<", " <<pt.y() / dbu <<") "
        //               <<obj->getViaDef()->getName() 
        //               <<endl;
        //        } else if (uConnFig->typeId() == drcPatchWire) {
        //          auto obj = static_cast<drPatchWire*>(uConnFig.get());
        //          frPoint pt;
        //          obj->getOrigin(pt);
        //          frBox offsetBox;
        //          obj->getOffsetBox(offsetBox);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  pWire (" <<pt.x() / dbu <<", " <<pt.y() / dbu <<") RECT ("
        //               <<offsetBox.left()   / dbu <<" "
        //               <<offsetBox.bottom() / dbu <<" "
        //               <<offsetBox.right()  / dbu <<" "
        //               <<offsetBox.top()    / dbu <<") " 
        //               <<getDesign()->getTech()->getLayer(obj->getLayerNum())->getName()
        //               <<endl;
        //        }
        //      }
        //      cout << __FILE__ << ":" << __LINE__ << ": " <<"extConnFigs" <<endl;
        //      for (auto &uConnFig: net->getExtConnFigs()) {
        //        if (uConnFig->typeId() == drcPathSeg) {
        //          auto obj = static_cast<drPathSeg*>(uConnFig.get());
        //          frPoint bp, ep;
        //          obj->getPoints(bp, ep);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  ps (" 
        //               <<bp.x() / dbu <<", " <<bp.y() / dbu <<") ("
        //               <<ep.x() / dbu <<", " <<ep.y() / dbu <<") "
        //               <<getDesign()->getTech()->getLayer(obj->getLayerNum())->getName()
        //               <<endl;

        //        } else if (uConnFig->typeId() == drcVia) {
        //          auto obj = static_cast<drVia*>(uConnFig.get());
        //          frPoint pt;
        //          obj->getOrigin(pt);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  via (" 
        //               <<pt.x() / dbu <<", " <<pt.y() / dbu <<") "
        //               <<obj->getViaDef()->getName() 
        //               <<endl;
        //        } else if (uConnFig->typeId() == drcPatchWire) {
        //          auto obj = static_cast<drPatchWire*>(uConnFig.get());
        //          frPoint pt;
        //          obj->getOrigin(pt);
        //          frBox offsetBox;
        //          obj->getOffsetBox(offsetBox);
        //          cout << __FILE__ << ":" << __LINE__ << ": " <<"  pWire (" <<pt.x() / dbu <<", " <<pt.y() / dbu <<") RECT ("
        //               <<offsetBox.left()   / dbu <<" "
        //               <<offsetBox.bottom() / dbu <<" "
        //               <<offsetBox.right()  / dbu <<" "
        //               <<offsetBox.top()    / dbu <<") " 
        //               <<getDesign()->getTech()->getLayer(obj->getLayerNum())->getName()
        //               <<endl;
        //        }
        //      }
        //    }
        //  }
        //}
        setBestMarkers();
        if (VERBOSE > 1 && i > 0) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"best iter = " <<i <<endl;
        }
      }

      if (isEnableDRC() && getMarkers().empty()) {
        break;
      } else if (!isEnableDRC() && violNum == 0) {
        break;
      }
    }
  }
}

void FlexDRWorker::routeNet_prep(drNet* net, set<drPin*, frBlockObjectComp> &unConnPins, 
                                 map<FlexMazeIdx, set<drPin*, frBlockObjectComp> > &mazeIdx2unConnPins,
                                 set<FlexMazeIdx> &apMazeIdx,
                                 set<FlexMazeIdx> &realPinAPMazeIdx/*,
                                 map<FlexMazeIdx, frViaDef*> &apSVia*/) {
  //bool enableOutput = true;
  bool enableOutput = false;
  for (auto &pin: net->getPins()) {
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"pin set target@";
    }
    unConnPins.insert(pin.get());
    for (auto &ap: pin->getAccessPatterns()) {
      FlexMazeIdx mi;
      ap->getMazeIdx(mi);
      mazeIdx2unConnPins[mi].insert(pin.get());
      if (pin->hasFrTerm()) {
        realPinAPMazeIdx.insert(mi);
        // if (net->getFrNet()->getName() == string("pci_devsel_oe_o")) {
        //   cout << __FILE__ << ":" << __LINE__ << ": " <<"apMazeIdx (" <<mi.x() <<", " <<mi.y() <<", " <<mi.z() <<")\n";
        //   auto routeBox = getRouteBox();
        //   double dbu = getDesign()->getTopBlock()->getDBUPerUU();
        //   std::cout << __FILE__ << ":" << __LINE__ << ": " <<"routeBox (" <<routeBox.left() / dbu <<", " <<routeBox.bottom() / dbu <<") ("
        //                            <<routeBox.right()/ dbu <<", " <<routeBox.top()    / dbu <<")" <<std::endl;
        // }
      }
      apMazeIdx.insert(mi);
      gridGraph.setDst(mi);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" (" <<mi.x() <<", " <<mi.y() <<", " <<mi.z() <<")";
      }
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
    }
  }
}

void FlexDRWorker::routeNet_setSrc(set<drPin*, frBlockObjectComp> &unConnPins, 
                                   map<FlexMazeIdx, set<drPin*, frBlockObjectComp> > &mazeIdx2unConnPins,
                                   vector<FlexMazeIdx> &connComps, 
                                   FlexMazeIdx &ccMazeIdx1, FlexMazeIdx &ccMazeIdx2, frPoint &centerPt) {
  frMIdx xDim, yDim, zDim;
  gridGraph.getDim(xDim, yDim, zDim);
  ccMazeIdx1.set(xDim - 1, yDim - 1, zDim - 1);
  ccMazeIdx2.set(0, 0, 0);
  // first pin selection algorithm goes here
  // choose the center pin
  centerPt.set(0, 0);
  int totAPCnt = 0;
  frCoord totX = 0;
  frCoord totY = 0;
  frCoord totZ = 0;
  FlexMazeIdx mi;
  frPoint bp;
  for (auto &pin: unConnPins) {
    for (auto &ap: pin->getAccessPatterns()) {
      ap->getMazeIdx(mi);
      ap->getPoint(bp);
      totX += bp.x();
      totY += bp.y();
      centerPt.set(centerPt.x() + bp.x(), centerPt.y() + bp.y());
      totZ += gridGraph.getZHeight(mi.z());
      totAPCnt++;
      break;
    }
  }
  totX /= totAPCnt;
  totY /= totAPCnt;
  totZ /= totAPCnt;
  centerPt.set(centerPt.x() / totAPCnt, centerPt.y() / totAPCnt);

  // select the farmost pin 
  drPin* currPin = nullptr;

  frCoord currDist = 0;
  for (auto &pin: unConnPins) {
    for (auto &ap: pin->getAccessPatterns()) {
      ap->getMazeIdx(mi);
      ap->getPoint(bp);
      frCoord dist = abs(totX - bp.x()) + abs(totY - bp.y()) + abs(totZ - gridGraph.getZHeight(mi.z()));
      if (dist >= currDist) {
        currDist = dist;
        currPin  = pin;
      }
    }
  }

  unConnPins.erase(currPin);

  // first pin selection algorithm ends here
  for (auto &ap: currPin->getAccessPatterns()) {
    ap->getMazeIdx(mi);
    connComps.push_back(mi);
    ccMazeIdx1.set(min(ccMazeIdx1.x(), mi.x()),
                   min(ccMazeIdx1.y(), mi.y()),
                   min(ccMazeIdx1.z(), mi.z()));
    ccMazeIdx2.set(max(ccMazeIdx2.x(), mi.x()),
                   max(ccMazeIdx2.y(), mi.y()),
                   max(ccMazeIdx2.z(), mi.z()));
    auto it = mazeIdx2unConnPins.find(mi);
    if (it == mazeIdx2unConnPins.end()) {
      continue;
    }
    auto it2 = it->second.find(currPin);
    if (it2 == it->second.end()) {
      continue;
    }
    it->second.erase(it2);

    gridGraph.setSrc(mi);
    // remove dst label only when no other pins share the same loc
    if (it->second.empty()) {
      mazeIdx2unConnPins.erase(it);
      gridGraph.resetDst(mi);
    }
  }

}

drPin* FlexDRWorker::routeNet_getNextDst(FlexMazeIdx &ccMazeIdx1, FlexMazeIdx &ccMazeIdx2, 
                                         map<FlexMazeIdx, set<drPin*, frBlockObjectComp> > &mazeIdx2unConnPins) {
  frPoint pt;
  frPoint ll, ur;
  gridGraph.getPoint(ll, ccMazeIdx1.x(), ccMazeIdx1.y());
  gridGraph.getPoint(ur, ccMazeIdx2.x(), ccMazeIdx2.y());
  frCoord currDist = std::numeric_limits<frCoord>::max();
  drPin* nextDst = nullptr;
  
  if (!nextDst)
    for (auto &[mazeIdx, setS]: mazeIdx2unConnPins) {
      gridGraph.getPoint(pt, mazeIdx.x(), mazeIdx.y());
      frCoord dx = max(max(ll.x() - pt.x(), pt.x() - ur.x()), 0);
      frCoord dy = max(max(ll.y() - pt.y(), pt.y() - ur.y()), 0);
      frCoord dz = max(max(gridGraph.getZHeight(ccMazeIdx1.z()) - gridGraph.getZHeight(mazeIdx.z()), 
                           gridGraph.getZHeight(mazeIdx.z()) - gridGraph.getZHeight(ccMazeIdx2.z())), 0);
      if (dx + dy + dz < currDist) {
        currDist = dx + dy + dz;
        nextDst = *(setS.begin());
      }
      if (currDist == 0) {
        break;
      }
    }
  return nextDst;
}

void FlexDRWorker::mazePinInit() {
  gridGraph.resetAStarCosts();
  gridGraph.resetPrevNodeDir();
}

void FlexDRWorker::routeNet_postAstarUpdate(vector<FlexMazeIdx> &path, vector<FlexMazeIdx> &connComps,
                                            set<drPin*, frBlockObjectComp> &unConnPins, 
                                            map<FlexMazeIdx, set<drPin*, frBlockObjectComp> > &mazeIdx2unConnPins,
                                            bool isFirstConn) {
  // first point is dst
  set<FlexMazeIdx> localConnComps;
  if (!path.empty()) {
    auto mi = path[0];
    vector<drPin*> tmpPins;
    for (auto pin: mazeIdx2unConnPins[mi]) {
      tmpPins.push_back(pin);
    }
    for (auto pin: tmpPins) {
      unConnPins.erase(pin);
      for (auto &ap: pin->getAccessPatterns()) {
        FlexMazeIdx mi;
        ap->getMazeIdx(mi);
        auto it = mazeIdx2unConnPins.find(mi);
        if (it == mazeIdx2unConnPins.end()) {
          continue;
        }
        auto it2 = it->second.find(pin);
        if (it2 == it->second.end()) {
          continue;
        }
        it->second.erase(it2);
        if (it->second.empty()) {
          mazeIdx2unConnPins.erase(it);
          gridGraph.resetDst(mi);
        }
        if (ALLOW_PIN_AS_FEEDTHROUGH) {
          localConnComps.insert(mi);
          gridGraph.setSrc(mi);
        }
      }
    }
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: routeNet_postAstarUpdate path is empty" <<endl;
  }
  // must be before comment line ABC so that the used actual src is set in gridgraph
  if (isFirstConn && (!ALLOW_PIN_AS_FEEDTHROUGH)) {
    for (auto &mi: connComps) {
      gridGraph.resetSrc(mi);
    }
    connComps.clear();
    if ((int)path.size() == 1) {
      connComps.push_back(path[0]);
      gridGraph.setSrc(path[0]);
    }
  }
  // line ABC
  // must have >0 length
  for (int i = 0; i < (int)path.size() - 1; ++i) {
    auto start = path[i];
    auto end = path[i + 1];
    auto startX = start.x(), startY = start.y(), startZ = start.z();
    auto endX = end.x(), endY = end.y(), endZ = end.z();
    // horizontal wire
    if (startX != endX && startY == endY && startZ == endZ) {
      for (auto currX = std::min(startX, endX); currX <= std::max(startX, endX); ++currX) {
        localConnComps.insert(FlexMazeIdx(currX, startY, startZ));
        gridGraph.setSrc(currX, startY, startZ);
      }
    // vertical wire
    } else if (startX == endX && startY != endY && startZ == endZ) {
      for (auto currY = std::min(startY, endY); currY <= std::max(startY, endY); ++currY) {
        localConnComps.insert(FlexMazeIdx(startX, currY, startZ));
        gridGraph.setSrc(startX, currY, startZ);
      }
    // via
    } else if (startX == endX && startY == endY && startZ != endZ) {
      for (auto currZ = std::min(startZ, endZ); currZ <= std::max(startZ, endZ); ++currZ) {
        localConnComps.insert(FlexMazeIdx(startX, startY, currZ));
        gridGraph.setSrc(startX, startY, currZ);
      }
    // zero length
    } else if (startX == endX && startY == endY && startZ == endZ) {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: zero-length path in updateFlexPin\n";
    } else {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << "Error: non-colinear path in updateFlexPin\n";
    }
  }
  for (auto &mi: localConnComps) {
    if (isFirstConn && !ALLOW_PIN_AS_FEEDTHROUGH) {
      connComps.push_back(mi);
    } else {
      if (!(mi == *(path.cbegin()))) {
        connComps.push_back(mi);
      }
    }
  }
}

void FlexDRWorker::routeNet_postAstarWritePath(drNet* net, vector<FlexMazeIdx> &points,
                                               const set<FlexMazeIdx> &apMazeIdx) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (points.empty()) {
    if (enableOutput) {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: path is empty in writeMazePath\n";
    }
    return;
  }
  if (TEST && points.size()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"path";
    for (auto &mIdx: points) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" (" <<mIdx.x() <<", " <<mIdx.y() <<", " <<mIdx.z() <<")";
    }
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  }
  auto &workerRegionQuery = getWorkerRegionQuery();
  for (int i = 0; i < (int)points.size() - 1; ++i) {
    FlexMazeIdx start, end;
    if (points[i + 1] < points[i]) {
      start = points[i + 1];
      end = points[i];
    } else {
      start = points[i];
      end = points[i + 1];
    }
    auto startX = start.x(), startY = start.y(), startZ = start.z();
    auto endX = end.x(), endY = end.y(), endZ = end.z();
    // horizontal wire
    if (startX != endX && startY == endY && startZ == endZ) {
      frPoint startLoc, endLoc;
      frLayerNum currLayerNum = gridGraph.getLayerNum(startZ);
      gridGraph.getPoint(startLoc, startX, startY);
      gridGraph.getPoint(endLoc, endX, endY);
      auto currPathSeg = make_unique<drPathSeg>();
      currPathSeg->setPoints(startLoc, endLoc);
      currPathSeg->setLayerNum(currLayerNum);
      currPathSeg->addToNet(net);
      auto currStyle = getTech()->getLayer(currLayerNum)->getDefaultSegStyle();
      if (apMazeIdx.find(start) != apMazeIdx.end()) {
        currStyle.setBeginStyle(frcTruncateEndStyle, 0);
      }
      if (apMazeIdx.find(end) != apMazeIdx.end()) {
        currStyle.setEndStyle(frcTruncateEndStyle, 0);
      }
      currPathSeg->setStyle(currStyle);
      currPathSeg->setMazeIdx(start, end);
      unique_ptr<drConnFig> tmp(std::move(currPathSeg));
      workerRegionQuery.add(tmp.get());
      net->addRoute(std::move(tmp));
      // if (/*startLoc.x() == 1834400 && */startLoc.y() == 2095500 && currLayerNum == 6 && net->getFrNet()->getName() == string("net74729")) {
      //   if (currStyle.getBeginStyle() == frEndStyle(frcTruncateEndStyle)) {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: begin point has 0 ext at x = " << startLoc.x() / 2000.0 << "\n";
      //   } else {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: begin point has non-0 ext at x = " << startLoc.x() / 2000.0 << "\n";
      //   }
      //   auto routeBox = getRouteBox();
      //   double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      //   std::cout << __FILE__ << ":" << __LINE__ << ": " <<"routeBox (" <<routeBox.left() / dbu <<", " <<routeBox.bottom() / dbu <<") ("
      //                            <<routeBox.right()/ dbu <<", " <<routeBox.top()    / dbu <<")" <<std::endl;
      // }
      // if (/*endLoc.x() == 1834400 && */endLoc.y() == 2095500 && currLayerNum == 6 && net->getFrNet()->getName() == string("net74729")) {
      //   if (currStyle.getEndStyle() == frEndStyle(frcTruncateEndStyle)) {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: end point has 0 ext at x = " << endLoc.x() / 2000.0 << "\n";
      //   } else {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: end point has non-0 ext at x = " << endLoc.x() / 2000.0 << "\n";
      //   }
      //   auto routeBox = getRouteBox();
      //   double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      //   std::cout << __FILE__ << ":" << __LINE__ << ": " <<"routeBox (" <<routeBox.left() / dbu <<", " <<routeBox.bottom() / dbu <<") ("
      //                            <<routeBox.right()/ dbu <<", " <<routeBox.top()    / dbu <<")" <<std::endl;
      // }
      // quick drc cnt
      bool prevHasCost = false;
      for (int i = startX; i < endX; i++) {
        if (gridGraph.hasDRCCost(i, startY, startZ, frDirEnum::E)) {
          if (!prevHasCost) {
            net->addMarker();
            prevHasCost = true;
          }
          if (TEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" pass marker @(" <<i <<", " <<startY <<", " <<startZ <<") E" <<endl;
          }
        } else {
          prevHasCost = false;
        }
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" write horz pathseg (" 
             <<startLoc.x() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<", " 
             <<startLoc.y() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<") (" 
             <<endLoc.x()   * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<", " 
             <<endLoc.y()   * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<") " 
             <<getTech()->getLayer(currLayerNum)->getName() <<endl;
      }
    // vertical wire
    } else if (startX == endX && startY != endY && startZ == endZ) {
      frPoint startLoc, endLoc;
      frLayerNum currLayerNum = gridGraph.getLayerNum(startZ);
      gridGraph.getPoint(startLoc, startX, startY);
      gridGraph.getPoint(endLoc, endX, endY);
      auto currPathSeg = make_unique<drPathSeg>();
      currPathSeg->setPoints(startLoc, endLoc);
      currPathSeg->setLayerNum(currLayerNum);
      currPathSeg->addToNet(net);
      auto currStyle = getTech()->getLayer(currLayerNum)->getDefaultSegStyle();
      if (apMazeIdx.find(start) != apMazeIdx.end()) {
        currStyle.setBeginStyle(frcTruncateEndStyle, 0);
      }
      if (apMazeIdx.find(end) != apMazeIdx.end()) {
        currStyle.setEndStyle(frcTruncateEndStyle, 0);
      }
      currPathSeg->setStyle(currStyle);
      currPathSeg->setMazeIdx(start, end);
      unique_ptr<drConnFig> tmp(std::move(currPathSeg));
      workerRegionQuery.add(tmp.get());
      net->addRoute(std::move(tmp));
      // if (startLoc.x() == 127300 && currLayerNum == 2 && net->getFrNet()->getName() == string("pci_devsel_oe_o")) {
      //   if (currStyle.getBeginStyle() == frEndStyle(frcTruncateEndStyle)) {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: begin point has 0 ext at y = " << startLoc.y() / 1000.0 << "(mazeIdx (x, y) = (" << startX << "," << startY << ")\n";
      //   } else {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: begin point has non-0 ext at y = " << startLoc.y() / 1000.0 << "(mazeIdx (x, y) = (" << startX << "," << startY << ")\n";
      //   }
      //   auto routeBox = getRouteBox();
      //   double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      //   std::cout << __FILE__ << ":" << __LINE__ << ": " <<"routeBox (" <<routeBox.left() / dbu <<", " <<routeBox.bottom() / dbu <<") ("
      //                            <<routeBox.right()/ dbu <<", " <<routeBox.top()    / dbu <<")" <<std::endl;
      // }
      // if (endLoc.x() == 127300 && currLayerNum == 2 && net->getFrNet()->getName() == string("pci_devsel_oe_o")) {
      //   if (currStyle.getEndStyle() == frEndStyle(frcTruncateEndStyle)) {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: end point has 0 ext at y = " << endLoc.y() / 1000.0 << "(mazeIdx (x, y) = (" << endX << "," << endY << ")\n";
      //   } else {
      //     cout << __FILE__ << ":" << __LINE__ << ": " << "@@@ DEBUG: end point has non-0 ext at x = " << endLoc.y() / 1000.0 << "(mazeIdx (x, y) = (" << endX << "," << endY << ")\n";
      //   }
      //   auto routeBox = getRouteBox();
      //   double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      //   std::cout << __FILE__ << ":" << __LINE__ << ": " <<"routeBox (" <<routeBox.left() / dbu <<", " <<routeBox.bottom() / dbu <<") ("
      //                            <<routeBox.right()/ dbu <<", " <<routeBox.top()    / dbu <<")" <<std::endl;
      // }
      // quick drc cnt
      bool prevHasCost = false;
      for (int i = startY; i < endY; i++) {
        if (gridGraph.hasDRCCost(startX, i, startZ, frDirEnum::E)) {
          if (!prevHasCost) {
            net->addMarker();
            prevHasCost = true;
          }
          if (TEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" pass marker @(" <<startX <<", " <<i <<", " <<startZ <<") N" <<endl;
          }
        } else {
          prevHasCost = false;
        }
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" write vert pathseg (" 
             <<startLoc.x() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<", " 
             <<startLoc.y() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<") (" 
             <<endLoc.x()   * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<", " 
             <<endLoc.y()   * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<") " 
             <<getTech()->getLayer(currLayerNum)->getName() <<endl;
      }
    // via
    } else if (startX == endX && startY == endY && startZ != endZ) {
      for (auto currZ = startZ; currZ < endZ; ++currZ) {
        frPoint loc;
        frLayerNum startLayerNum = gridGraph.getLayerNum(currZ);
        //frLayerNum endLayerNum = gridGraph.getLayerNum(currZ + 1);
        gridGraph.getPoint(loc, startX, startY);
        FlexMazeIdx mi(startX, startY, currZ); 
        auto cutLayerDefaultVia = getTech()->getLayer(startLayerNum + 1)->getDefaultViaDef();
        if (gridGraph.isSVia(startX, startY, currZ)) {
          cutLayerDefaultVia = apSVia.find(mi)->second->getAccessViaDef();
        }
        auto currVia = make_unique<drVia>(cutLayerDefaultVia);
        currVia->setOrigin(loc);
        currVia->setMazeIdx(FlexMazeIdx(startX, startY, currZ), FlexMazeIdx(startX, startY, currZ+1));
        unique_ptr<drConnFig> tmp(std::move(currVia));
        workerRegionQuery.add(tmp.get());
        net->addRoute(std::move(tmp));
        if (gridGraph.hasDRCCost(startX, startY, currZ, frDirEnum::U)) {
          net->addMarker();
          if (TEST) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" pass marker @(" <<startX <<", " <<startY <<", " <<currZ <<") U" <<endl;
          }
        }
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" write via (" 
               <<loc.x() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<", " 
               <<loc.y() * 1.0 / getDesign()->getTopBlock()->getDBUPerUU() <<") " 
               <<cutLayerDefaultVia->getName() <<endl;
        }
      }
    // zero length
    } else if (startX == endX && startY == endY && startZ == endZ) {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: zero-length path in updateFlexPin\n";
    } else {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << "Error: non-colinear path in updateFlexPin\n";
    }
  }
}

void FlexDRWorker::routeNet_postRouteAddPathCost(drNet* net) {
  int cnt = 0;
  for (auto &connFig: net->getRouteConnFigs()) {
    addPathCost(connFig.get());
    cnt++;
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"updated " <<cnt <<" connfig costs" <<endl;
}

void FlexDRWorker::routeNet_prepAreaMap(drNet* net, map<FlexMazeIdx, frCoord> &areaMap) {
  FlexMazeIdx mIdx;
  for (auto &pin: net->getPins()) {
    for (auto &ap: pin->getAccessPatterns()) {
      ap->getMazeIdx(mIdx);
      auto it = areaMap.find(mIdx);
      if (it != areaMap.end()) {
        it->second = max(it->second, ap->getBeginArea());
      } else {
        areaMap[mIdx] = ap->getBeginArea();
      }
    }
  }
}

bool FlexDRWorker::routeNet(drNet* net) {
  ProfileTask profile("DR:routeNet");
  //bool enableOutput = true;
  bool enableOutput = false;
  if (net->getPins().size() <= 1) {
    return true;
  }
  
  if (TEST || enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"route " <<net->getFrNet()->getName() <<endl;
  }

  set<drPin*, frBlockObjectComp> unConnPins;
  map<FlexMazeIdx, set<drPin*, frBlockObjectComp> > mazeIdx2unConnPins;
  set<FlexMazeIdx> apMazeIdx;
  set<FlexMazeIdx> realPinAPMazeIdx; // 
  routeNet_prep(net, unConnPins, mazeIdx2unConnPins, apMazeIdx, realPinAPMazeIdx);
  // prep for area map
  map<FlexMazeIdx, frCoord> areaMap;
  if (ENABLE_BOUNDARY_MAR_FIX) {
    routeNet_prepAreaMap(net, areaMap);
  }

  FlexMazeIdx ccMazeIdx1, ccMazeIdx2; // connComps ll, ur flexmazeidx
  frPoint centerPt;
  vector<FlexMazeIdx> connComps;
  routeNet_setSrc(unConnPins, mazeIdx2unConnPins, connComps, ccMazeIdx1, ccMazeIdx2, centerPt);

  vector<FlexMazeIdx> path; // astar must return with >= 1 idx
  bool isFirstConn = true;
  while(!unConnPins.empty()) {
    mazePinInit();
    auto nextPin = routeNet_getNextDst(ccMazeIdx1, ccMazeIdx2, mazeIdx2unConnPins);
    path.clear();
    if (gridGraph.search(connComps, nextPin, path, ccMazeIdx1, ccMazeIdx2, centerPt)) {
      routeNet_postAstarUpdate(path, connComps, unConnPins, mazeIdx2unConnPins, isFirstConn);
      routeNet_postAstarWritePath(net, path, realPinAPMazeIdx/*, apSVia*/);
      routeNet_postAstarPatchMinAreaVio(net, path, areaMap);
      isFirstConn = false;
    } else {
      return false;
    }
  }
  routeNet_postRouteAddPathCost(net);
  return true;
}

void FlexDRWorker::routeNet_postAstarPatchMinAreaVio(drNet* net, const vector<FlexMazeIdx> &path, const map<FlexMazeIdx, frCoord> &areaMap) {
  if (path.empty()) {
    return;
  }
  // get path with separated (stacked vias)
  vector<FlexMazeIdx> points;
  for (int i = 0; i < (int)path.size() - 1; ++i) {
    auto currIdx = path[i];
    auto nextIdx = path[i+1];
    if (currIdx.z() == nextIdx.z()) {
      points.push_back(currIdx);
    } else {
      if (currIdx.z() < nextIdx.z()) {
        for (auto z = currIdx.z(); z < nextIdx.z(); ++z) {
          FlexMazeIdx tmpIdx(currIdx.x(), currIdx.y(), z);
          points.push_back(tmpIdx);
        }
      } else {
        for (auto z = currIdx.z(); z > nextIdx.z(); --z) {
          FlexMazeIdx tmpIdx(currIdx.x(), currIdx.y(), z);
          points.push_back(tmpIdx);
        }
      }
    }
  }
  points.push_back(path.back());


  auto layerNum = gridGraph.getLayerNum(points.front().z());
  auto minAreaConstraint = getDesign()->getTech()->getLayer(layerNum)->getAreaConstraint();

  frCoord currArea = 0;
  if (ENABLE_BOUNDARY_MAR_FIX) {
    if (areaMap.find(points[0]) != areaMap.end()) {
      currArea = areaMap.find(points[0])->second;
      //if (TEST) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[0] = " <<currArea <<", from areaMap" <<endl;
      //}
    } else {
      currArea = (minAreaConstraint) ? minAreaConstraint->getMinArea() : 0;
      //if (TEST) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[0] = " <<currArea <<", from rule" <<endl;
      //}
    }
  } else {
    currArea = (minAreaConstraint) ? minAreaConstraint->getMinArea() : 0;
  }
  frCoord startViaHalfEncArea = 0, endViaHalfEncArea = 0;
  FlexMazeIdx prevIdx = points[0], currIdx;
  int i;
  int prev_i = 0; // path start point
  for (i = 1; i < (int)points.size(); ++i) {
    currIdx = points[i];
    // check minAreaViolation when change layer, or last segment
    if (currIdx.z() != prevIdx.z()) {
      layerNum = gridGraph.getLayerNum(prevIdx.z());
      minAreaConstraint = getDesign()->getTech()->getLayer(layerNum)->getAreaConstraint();
      frCoord reqArea = (minAreaConstraint) ? minAreaConstraint->getMinArea() : 0;
      // add next via enclosure
      if (currIdx.z() < prevIdx.z()) {
        currArea += gridGraph.getHalfViaEncArea(prevIdx.z() - 1, false);
        endViaHalfEncArea = gridGraph.getHalfViaEncArea(prevIdx.z() - 1, false);
      } else {
        currArea += gridGraph.getHalfViaEncArea(prevIdx.z(), true);
        endViaHalfEncArea = gridGraph.getHalfViaEncArea(prevIdx.z(), true);
      }
      // push to minArea violation
      if (currArea < reqArea) {
        //if (TEST) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[" <<i <<"] = " <<currArea <<", add pwire " <<i - 2 <<", " <<i - 1 <<endl;
        //}
        FlexMazeIdx bp, ep;
        frCoord gapArea = reqArea - (currArea - startViaHalfEncArea - endViaHalfEncArea) - std::min(startViaHalfEncArea, endViaHalfEncArea);
        // new
        bool bpPatchStyle = true; // style 1: left only; 0: right only
        bool epPatchStyle = false;
        // stack via
        if (i - 1 == prev_i) {
          bp = points[i-1];
          ep = points[i-1];
          bpPatchStyle = true;
          epPatchStyle = false;
        // planar
        } else {
          bp = points[prev_i];
          ep = points[i-1];
          if (getDesign()->getTech()->getLayer(layerNum)->getDir() == frPrefRoutingDirEnum::frcHorzPrefRoutingDir) {
            if (points[prev_i].x() < points[prev_i+1].x()) {
              bpPatchStyle = true;
            } else if (points[prev_i].x() > points[prev_i+1].x()) {
              bpPatchStyle = false;
            } else {
              if (points[prev_i].x() < points[i-1].x()) {
                bpPatchStyle = true;
              } else {
                bpPatchStyle = false;
              }
            }
            if (points[i-1].x() < points[i-2].x()) {
              epPatchStyle = true;
            } else if (points[i-1].x() > points[i-2].x()) {
              epPatchStyle = false;
            } else {
              if (points[i-1].x() < points[prev_i].x()) {
                epPatchStyle = true;
              } else {
                epPatchStyle = false;
              }
            }
          } else {
            if (points[prev_i].y() < points[prev_i+1].y()) {
              bpPatchStyle = true;
            } else if (points[prev_i].y() > points[prev_i+1].y()) {
              bpPatchStyle = false;
            } else {
              if (points[prev_i].y() < points[i-1].y()) {
                bpPatchStyle = true;
              } else {
                bpPatchStyle = false;
              }
            }
            if (points[i-1].y() < points[i-2].y()) {
              epPatchStyle = true;
            } else if (points[i-1].y() > points[i-2].y()) {
              epPatchStyle = false;
            } else {
              if (points[i-1].y() < points[prev_i].y()) {
                epPatchStyle = true;
              } else {
                epPatchStyle = false;
              }
            }
          }
        }
        auto patchWidth = getDesign()->getTech()->getLayer(layerNum)->getWidth();
        routeNet_postAstarAddPatchMetal(net, bp, ep, gapArea, patchWidth, bpPatchStyle, epPatchStyle);
      } else {
        //if (TEST) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[" <<i <<"] = " <<currArea <<", no pwire" <<endl;
        //}
      }
      // init for next path
      if (currIdx.z() < prevIdx.z()) {
        currArea = gridGraph.getHalfViaEncArea(prevIdx.z() - 1, true);
        startViaHalfEncArea = gridGraph.getHalfViaEncArea(prevIdx.z() - 1, true);
      } else {
        currArea = gridGraph.getHalfViaEncArea(prevIdx.z(), false);
        startViaHalfEncArea = gridGraph.getHalfViaEncArea(prevIdx.z(), false);
      }
      prev_i = i;
    } 
    // add the wire area
    else {
      layerNum = gridGraph.getLayerNum(prevIdx.z());
      minAreaConstraint = getDesign()->getTech()->getLayer(layerNum)->getAreaConstraint();
      frCoord reqArea = (minAreaConstraint) ? minAreaConstraint->getMinArea() : 0;
      auto pathWidth = getDesign()->getTech()->getLayer(layerNum)->getWidth();
      frPoint bp, ep;
      gridGraph.getPoint(bp, prevIdx.x(), prevIdx.y());
      gridGraph.getPoint(ep, currIdx.x(), currIdx.y());
      frCoord pathLength = abs(bp.x() - ep.x()) + abs(bp.y() - ep.y());
      if (currArea < reqArea) {
        currArea += pathLength * pathWidth;
      }
      //if (TEST) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[" <<i <<"] = " <<currArea <<", no pwire planar" <<endl;
      //}
    }
    prevIdx = currIdx;
  }
  // add boundary area for last segment
  if (ENABLE_BOUNDARY_MAR_FIX) {
    layerNum = gridGraph.getLayerNum(prevIdx.z());
    minAreaConstraint = getDesign()->getTech()->getLayer(layerNum)->getAreaConstraint();
    frCoord reqArea = (minAreaConstraint) ? minAreaConstraint->getMinArea() : 0;
    if (areaMap.find(prevIdx) != areaMap.end()) {
      currArea += areaMap.find(prevIdx)->second;
    }
    endViaHalfEncArea = 0;
    if (currArea < reqArea) {
      //if (TEST) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[" <<i <<"] = " <<currArea <<", add pwire end" <<endl;
      //}
      FlexMazeIdx bp, ep;
      frCoord gapArea = reqArea - (currArea - startViaHalfEncArea - endViaHalfEncArea) - std::min(startViaHalfEncArea, endViaHalfEncArea);
      // new
      bool bpPatchStyle = true; // style 1: left only; 0: right only
      bool epPatchStyle = false;
      // stack via
      if (i - 1 == prev_i) {
        bp = points[i-1];
        ep = points[i-1];
        bpPatchStyle = true;
        epPatchStyle = false;
      // planar
      } else {
        bp = points[prev_i];
        ep = points[i-1];
        if (getDesign()->getTech()->getLayer(layerNum)->getDir() == frPrefRoutingDirEnum::frcHorzPrefRoutingDir) {
          if (points[prev_i].x() < points[prev_i+1].x()) {
            bpPatchStyle = true;
          } else if (points[prev_i].x() > points[prev_i+1].x()) {
            bpPatchStyle = false;
          } else {
            if (points[prev_i].x() < points[i-1].x()) {
              bpPatchStyle = true;
            } else {
              bpPatchStyle = false;
            }
          }
          if (points[i-1].x() < points[i-2].x()) {
            epPatchStyle = true;
          } else if (points[i-1].x() > points[i-2].x()) {
            epPatchStyle = false;
          } else {
            if (points[i-1].x() < points[prev_i].x()) {
              epPatchStyle = true;
            } else {
              epPatchStyle = false;
            }
          }
        } else {
          if (points[prev_i].y() < points[prev_i+1].y()) {
            bpPatchStyle = true;
          } else if (points[prev_i].y() > points[prev_i+1].y()) {
            bpPatchStyle = false;
          } else {
            if (points[prev_i].y() < points[i-1].y()) {
              bpPatchStyle = true;
            } else {
              bpPatchStyle = false;
            }
          }
          if (points[i-1].y() < points[i-2].y()) {
            epPatchStyle = true;
          } else if (points[i-1].y() > points[i-2].y()) {
            epPatchStyle = false;
          } else {
            if (points[i-1].y() < points[prev_i].y()) {
              epPatchStyle = true;
            } else {
              epPatchStyle = false;
            }
          }
        }
      }
      auto patchWidth = getDesign()->getTech()->getLayer(layerNum)->getWidth();
      routeNet_postAstarAddPatchMetal(net, bp, ep, gapArea, patchWidth, bpPatchStyle, epPatchStyle);
    } else {
      //if (TEST) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"currArea[" <<i <<"] = " <<currArea <<", no pwire end" <<endl;
      //}
    }
  }
}

// assumes patchWidth == defaultWidth
// the cost checking part is sensitive to how cost is stored (1) planar + via; or (2) N;E;U
int FlexDRWorker::routeNet_postAstarAddPathMetal_isClean(const FlexMazeIdx &bpIdx,
                                                         bool isPatchHorz, bool isPatchLeft, 
                                                         frCoord patchLength) {
  //bool enableOutput = true;
  bool enableOutput = false;
  int cost = 0;
  frPoint origin, patchEnd;
  gridGraph.getPoint(origin, bpIdx.x(), bpIdx.y());
  frLayerNum layerNum = gridGraph.getLayerNum(bpIdx.z());
  if (isPatchHorz) {
    if (isPatchLeft) {
      patchEnd.set(origin.x() - patchLength, origin.y());
    } else {
      patchEnd.set(origin.x() + patchLength, origin.y());
    }
  } else {
    if (isPatchLeft) {
      patchEnd.set(origin.x(), origin.y() - patchLength);
    } else {
      patchEnd.set(origin.x(), origin.y() + patchLength);
    }
  }
  if (enableOutput) {
    double dbu = getDesign()->getTopBlock()->getDBUPerUU();
    cout << __FILE__ << ":" << __LINE__ << ": " <<"    patchOri@(" <<origin.x() / dbu   <<", " <<origin.y() / dbu   <<")" <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"    patchEnd@(" <<patchEnd.x() / dbu <<", " <<patchEnd.y() / dbu <<")" <<endl;
  }
  // for wire, no need to bloat width
  frPoint patchLL = min(origin, patchEnd);
  frPoint patchUR = max(origin, patchEnd);
  if (!getRouteBox().contains(patchEnd)) {
    cost = std::numeric_limits<int>::max();
  } else {
    FlexMazeIdx startIdx, endIdx;
    startIdx.set(0, 0, layerNum);
    endIdx.set(0, 0, layerNum);
    frBox patchBox(patchLL, patchUR);
    gridGraph.getIdxBox(startIdx, endIdx, patchBox);
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    patchOriIdx@(" <<startIdx.x() <<", " <<startIdx.y() <<")" <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    patchEndIdx@(" <<endIdx.x()   <<", " <<endIdx.y()   <<")" <<endl;
    }
    if (isPatchHorz) {
      // in gridgraph, the planar cost is checked for xIdx + 1
      for (auto xIdx = max(0, startIdx.x() - 1); xIdx < endIdx.x(); ++xIdx) {
        if (gridGraph.hasDRCCost(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E)) {
          cost += gridGraph.getEdgeLength(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E) * workerDRCCost;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<xIdx <<", " <<bpIdx.y() <<", " <<bpIdx.z() <<") drc cost" <<endl;
          }
        }
        if (gridGraph.hasShapeCost(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E)) {
          cost += gridGraph.getEdgeLength(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E) * SHAPECOST;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<xIdx <<", " <<bpIdx.y() <<", " <<bpIdx.z() <<") shape cost" <<endl;
          }
        }
        if (gridGraph.hasMarkerCost(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E)) {
          cost += gridGraph.getEdgeLength(xIdx, bpIdx.y(), bpIdx.z(), frDirEnum::E) * workerMarkerCost;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<xIdx <<", " <<bpIdx.y() <<", " <<bpIdx.z() <<") marker cost" <<endl;
          }
        }
      }
    } else {
      // in gridgraph, the planar cost is checked for yIdx + 1
      for (auto yIdx = max(0, startIdx.y() - 1); yIdx < endIdx.y(); ++yIdx) {
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    check (" <<bpIdx.x() <<", " <<yIdx <<", " <<bpIdx.z() <<") N" <<endl;
        }
        if (gridGraph.hasDRCCost(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N)) {
          cost += gridGraph.getEdgeLength(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N) * workerDRCCost;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<bpIdx.x() <<", " <<yIdx <<", " <<bpIdx.z() <<") drc cost" <<endl;
          }
        }
        if (gridGraph.hasShapeCost(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N)) {
          cost += gridGraph.getEdgeLength(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N) * SHAPECOST;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<bpIdx.x() <<", " <<yIdx <<", " <<bpIdx.z() <<") shape cost" <<endl;
          }
        }
        if (gridGraph.hasMarkerCost(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N)) {
          cost += gridGraph.getEdgeLength(bpIdx.x(), yIdx, bpIdx.z(), frDirEnum::N) * workerMarkerCost;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"    (" <<bpIdx.x() <<", " <<yIdx <<", " <<bpIdx.z() <<") marker cost" <<endl;
          }
        }
      }
    }
  }
  return cost;
}


void FlexDRWorker::routeNet_postAstarAddPatchMetal_addPWire(drNet* net, const FlexMazeIdx &bpIdx, 
                                                            bool isPatchHorz, bool isPatchLeft, 
                                                            frCoord patchLength, frCoord patchWidth) {
  frPoint origin, patchEnd;
  gridGraph.getPoint(origin, bpIdx.x(), bpIdx.y());
  frLayerNum layerNum = gridGraph.getLayerNum(bpIdx.z());
  // actual offsetbox
  frPoint patchLL, patchUR;
  if (isPatchHorz) {
    if (isPatchLeft) {
      patchLL.set(0 - patchLength, 0 - patchWidth / 2);
      patchUR.set(0,               0 + patchWidth / 2);
    } else {
      patchLL.set(0,               0 - patchWidth / 2);
      patchUR.set(0 + patchLength, 0 + patchWidth / 2);
    }
  } else {
    if (isPatchLeft) {
      patchLL.set(0 - patchWidth / 2, 0 - patchLength);
      patchUR.set(0 + patchWidth / 2, 0              );
    } else {
      patchLL.set(0 - patchWidth / 2, 0              );
      patchUR.set(0 + patchWidth / 2, 0 + patchLength);
    }
  }

  auto tmpPatch = make_unique<drPatchWire>();
  tmpPatch->setLayerNum(layerNum);
  tmpPatch->setOrigin(origin);
  tmpPatch->setOffsetBox(frBox(patchLL, patchUR));
  tmpPatch->addToNet(net);
  unique_ptr<drConnFig> tmp(std::move(tmpPatch));
  auto &workerRegionQuery = getWorkerRegionQuery();
  workerRegionQuery.add(tmp.get());
  net->addRoute(std::move(tmp));
  //if (TEST) {
  //  double dbu = getDesign()->getTopBlock()->getDBUPerUU();
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"pwire@(" <<origin.x() / dbu <<", " <<origin.y() / dbu <<"), (" 
  //       <<patchLL.x() / dbu <<", " <<patchLL.y() / dbu <<", "
  //       <<patchUR.x() / dbu <<", " <<patchUR.y() / dbu 
  //       <<endl;
  //}
}

void FlexDRWorker::routeNet_postAstarAddPatchMetal(drNet* net, 
                                                   const FlexMazeIdx &bpIdx,
                                                   const FlexMazeIdx &epIdx,
                                                   frCoord gapArea,
                                                   frCoord patchWidth,
                                                   bool bpPatchStyle, bool epPatchStyle) {
  bool isPatchHorz;
  //bool isLeftClean = true;
  frLayerNum layerNum = gridGraph.getLayerNum(bpIdx.z());
  frCoord patchLength = frCoord(ceil(1.0 * gapArea / patchWidth / getDesign()->getTech()->getManufacturingGrid())) * 
                        getDesign()->getTech()->getManufacturingGrid();

  // always patch to pref dir
  if (getDesign()->getTech()->getLayer(layerNum)->getDir() == frcHorzPrefRoutingDir) {
    isPatchHorz = true;
  } else {
    isPatchHorz = false;
  }

  //if (QUICKDRCTEST) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  pwire L" <<endl;
  //}
  auto costL = routeNet_postAstarAddPathMetal_isClean(bpIdx, isPatchHorz, bpPatchStyle, patchLength);
  //if (QUICKDRCTEST) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  pwire R" <<endl;
  //}
  auto costR = routeNet_postAstarAddPathMetal_isClean(epIdx, isPatchHorz, epPatchStyle, patchLength);
  //if (QUICKDRCTEST) {
  //  double dbu = getDesign()->getTopBlock()->getDBUPerUU();
  //  frPoint bp, ep;
  //  gridGraph.getPoint(bp, bpIdx.x(), bpIdx.y());
  //  gridGraph.getPoint(ep, epIdx.x(), epIdx.y());
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  pwire L@(" <<bpIdx.x()    <<", " <<bpIdx.y()           <<", " <<bpIdx.z() <<") ("
  //                       <<bp.x() / dbu <<", " <<bp.y() / dbu <<") " <<getDesign()->getTech()->getLayer(layerNum)->getName() <<", cost="
  //                       <<costL <<endl;
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  pwire R@(" <<epIdx.x()    <<", " <<epIdx.y()           <<", " <<epIdx.z() <<") (" 
  //                       <<ep.x() / dbu <<", " <<ep.y() / dbu <<") " <<getDesign()->getTech()->getLayer(layerNum)->getName() <<", cost="
  //                       <<costR <<endl;
  //}
  if (costL <= costR) {
    routeNet_postAstarAddPatchMetal_addPWire(net, bpIdx, isPatchHorz, bpPatchStyle, patchLength, patchWidth);
    //if (TEST) {
    //  cout << __FILE__ << ":" << __LINE__ << ": " <<"pwire added L" <<endl;
    //}
  } else {
    routeNet_postAstarAddPatchMetal_addPWire(net, epIdx, isPatchHorz, epPatchStyle, patchLength, patchWidth);
    //if (TEST) {
    //  cout << __FILE__ << ":" << __LINE__ << ": " <<"pwire added R" <<endl;
    //}
  }
}

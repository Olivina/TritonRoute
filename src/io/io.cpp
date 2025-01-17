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
#include <fstream>
#include <sstream>
#include <exception>

#include "frProfileTask.h"
#include "global.h"
#include "io/io.h"
#include "db/tech/frConstraint.h"

#include "defrReader.hpp"
#include "lefrReader.hpp"

using namespace std;
using namespace fr;

// This class hides the dependency on the Si2 parsing libraries from clients
class io::Parser::Callbacks
{
public:
      static int getDefDieArea(defrCallbackType_e type, defiBox* box, defiUserData data);
      static int getDefTracks(defrCallbackType_e type, defiTrack* track, defiUserData data);
      static int getDefVias(defrCallbackType_e type, defiVia* comp, defiUserData data);
      static int getDefComponents(defrCallbackType_e type, defiComponent* comp, defiUserData data);
      static int getDefTerminals(defrCallbackType_e type, defiPin* term, defiUserData data);
      static int getDefNets(defrCallbackType_e type, defiNet* net, defiUserData data);
      static int getDefInteger(defrCallbackType_e type, int number, defiUserData data);
      static int getDefString(defrCallbackType_e type, const char* str, defiUserData data);
      static int getDefVoid(defrCallbackType_e type, void* variable, defiUserData data);
      static int getDefUnits(defrCallbackType_e type, double number, defiUserData data);
      static int getDefBlockages(defrCallbackType_e type, defiBlockage* blockage, defiUserData data);
      static int getLefMacros(lefrCallbackType_e type, lefiMacro* macro, lefiUserData data);
      static int getLefPins(lefrCallbackType_e type, lefiPin* pin, lefiUserData data);
      static int getLefObs(lefrCallbackType_e type, lefiObstruction* obs, lefiUserData data);
      static int getLefString(lefrCallbackType_e type, const char* string, lefiUserData data);
      static int getLefUnits(lefrCallbackType_e type, lefiUnits* units, lefiUserData data);
      static int getLefManufacturingGrid(lefrCallbackType_e type, double number, lefiUserData data);
      static int getLefLayers(lefrCallbackType_e type, lefiLayer* layer, lefiUserData data);
      static int getLefVias(lefrCallbackType_e type, lefiVia* via, lefiUserData data);
      static int getLefViaRules(lefrCallbackType_e type, lefiViaRule* via, lefiUserData data);
      static int getLefUseMinSpacing(lefrCallbackType_e type, lefiUseMinSpacing* spacing, lefiUserData data);
};

int io::Parser::Callbacks::getDefBlockages(defrCallbackType_e type, defiBlockage* blockage, defiUserData data) {
  // bool enableOutput = true;
  bool enableOutput = false;
  if ((type != defrBlockageCbkType)) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Type is not defrBlockageCbkType!" << endl;
    exit(1);
  }

  io::Parser* parser = (io::Parser*) data;
  frLayerNum layerNum = -1;
  string layerName;
  // blockage
  auto blkIn = make_unique<frBlockage>();
  blkIn->setId(parser->numBlockages);
  parser->numBlockages++;
  // pin
  auto pinIn = make_unique<frPin>();
  pinIn->setId(0);

  if (blockage->hasLayer()) {
    layerName = blockage->layerName();
    if (parser->tech->name2layer.find(layerName) != parser->tech->name2layer.end()) {
      layerNum = parser->tech->name2layer[layerName]->getLayerNum();
    } else {
        cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: DEF OBS on layer " << layerName <<" is skipped..." << endl; 
      // }
      return 0;
    }
    if (blockage->hasComponent()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasSlots()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasFills()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasPushdown()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasExceptpgnet()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasMask()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasSpacing()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
    if (blockage->hasDesignRuleWidth()) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
      return 0;
    }
  } else if (blockage->hasPlacement()) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: DEF placement OBS is skipped..." << endl;
    return 0;
  }

  for (int i = 0; i < blockage->numRectangles(); i++) {
    // DEF is already DBU
    frCoord xl = blockage->xl(i);
    frCoord yl = blockage->yl(i);
    frCoord xh = blockage->xh(i);
    frCoord yh = blockage->yh(i);
    // pinFig
    unique_ptr<frRect> pinFig = make_unique<frRect>();
    pinFig->setBBox(frBox(xl, yl, xh, yh));
    pinFig->addToPin(pinIn.get());
    pinFig->setLayerNum(layerNum);
    // pinFig completed
    unique_ptr<frPinFig> uptr(std::move(pinFig));
    pinIn->addPinFig(std::move(uptr));
    // pin completed
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"      RECT " <<blockage->xl(i) <<" " <<blockage->yl(i) <<" " <<blockage->xh(i) <<" " <<blockage->yh(i) <<" ;" <<endl;
    }
  }

  for (int i = 0; i < blockage->numPolygons(); i++) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Unsupported syntax in DEF BLOCKAGE, skipped...\n";
    continue;
  }

  blkIn->setPin(std::move(pinIn));
  parser->tmpBlock->addBlockage(std::move(blkIn));

  return 0;

}

int io::Parser::Callbacks::getDefVias(defrCallbackType_e type, defiVia* via, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if ((type != defrViaCbkType)) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defrViaCbkType!" <<endl;
    exit(1);
  }

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"- " <<via->name() <<endl;
    //cout << __FILE__ << ":" << __LINE__ << ": " <<"  numLayers = " <<via->numLayers() <<endl;
  }
  io::Parser* parser = (io::Parser*) data;

  // viaRule defined via
  if (via->hasViaRule()) {
    char* viaRuleName;
    char* botLayer;
    char* cutLayer;
    char* topLayer;
    int xSize, ySize, xCutSpacing, yCutSpacing, xBotEnc, yBotEnc, xTopEnc, yTopEnc;
    via->viaRule(&viaRuleName, &xSize, &ySize, &botLayer, &cutLayer, &topLayer,
                 &xCutSpacing, &yCutSpacing, &xBotEnc, &yBotEnc, &xTopEnc, &yTopEnc);
    int xOffset = 0;
    int yOffset = 0;
    if (via->hasOrigin()) {
      via->origin(&xOffset, &yOffset);
    }
    int xBotOffset = 0;
    int yBotOffset = 0;
    int xTopOffset = 0;
    int yTopOffset = 0;
    if (via->hasOffset()) {
      via->offset(&xBotOffset, &yBotOffset, &xTopOffset, &yTopOffset);
    }
    int numCutRows = 1;
    int numCutCols = 1;
    if (via->hasRowCol()) {
      via->rowCol(&numCutRows, &numCutCols);
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + VIARULE "    <<viaRuleName <<endl;
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" + CUTSIZE " <<xSize * 1.0 / parser->tmpBlock->getDBUPerUU() <<" " 
      //                     <<ySize * 1.0 / parser->tmpBlock->getDBUPerUU() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + CUTSIZE "    <<xSize       <<" " <<ySize       <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + LAYERS "     <<botLayer    <<" " <<cutLayer    <<" "   <<topLayer <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + CUTSPACING " <<xCutSpacing <<" " <<yCutSpacing <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + ENCLOSURE "  <<xBotEnc     <<" " <<yBotEnc     <<" "
                              <<xTopEnc     <<" " <<yTopEnc     <<endl;
      if (via->hasRowCol()) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" + ROWCOL " <<numCutRows <<" " <<numCutCols <<endl;
      }
      if (via->hasOrigin()) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" + ORIGIN " <<xOffset <<" " <<yOffset <<endl;
      }
      if (via->hasOffset()) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" + OFFSET " <<xBotOffset <<" " <<yBotOffset <<" " <<xTopOffset <<" " <<yTopOffset <<endl;
      }
      cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;

    }

    // create cut figs
    frLayerNum cutLayerNum = 0;
    if (parser->tech->name2layer.find(cutLayer) == parser->tech->name2layer.end()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find cut layer" <<endl;
      exit(1);
    } else {
      cutLayerNum = parser->tech->name2layer.find(cutLayer)->second->getLayerNum();
    }
    frLayerNum botLayerNum = 0;
    if (parser->tech->name2layer.find(botLayer) == parser->tech->name2layer.end()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find bot layer" <<endl;
      exit(1);
    } else {
      botLayerNum = parser->tech->name2layer.find(botLayer)->second->getLayerNum();
    }
    // create cut figs
    frLayerNum topLayerNum = 0;
    if (parser->tech->name2layer.find(topLayer) == parser->tech->name2layer.end()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find top layer" <<endl;
      exit(1);
    } else {
      topLayerNum = parser->tech->name2layer.find(topLayer)->second->getLayerNum();
    }

    frCoord currX = 0;
    frCoord currY = 0;
    vector<unique_ptr<frShape> > cutFigs;
    for (int i = 0; i < numCutRows; i++) {
      currX = 0;
      for (int j = 0; j < numCutCols; j++) {
        auto rect = make_unique<frRect>();
        frBox tmpBox(currX, currY, currX + xSize, currY + ySize);
        rect->setBBox(tmpBox);
        rect->setLayerNum(cutLayerNum);
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"cutFig (" <<currX <<", " <<currY <<") (" <<currX+xSize <<"," <<currY+ySize <<") " <<cutLayer <<endl;

        cutFigs.push_back(std::move(rect));

        currX += xSize + xCutSpacing;
      }
      currY += ySize + yCutSpacing;
    }
    currX -= xCutSpacing; // max cut X
    currY -= yCutSpacing; // max cut Y
    //cout << __FILE__ << ":" << __LINE__ << ": " <<"max x/y " <<currX <<" " <<currY <<endl;

    frTransform cutXform(-currX / 2 + xOffset, -currY / 2 + yOffset);
    for (auto &uShape: cutFigs) {
      auto rect = static_cast<frRect*>(uShape.get());
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"orig " <<*rect <<endl;
      rect->move(cutXform);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"move " <<*rect <<endl;
    }

    unique_ptr<frShape> uBotFig = make_unique<frRect>();
    auto botFig = static_cast<frRect*>(uBotFig.get());
    unique_ptr<frShape> uTopFig = make_unique<frRect>();
    auto topFig = static_cast<frRect*>(uTopFig.get());

    frBox botBox(0 - xBotEnc, 0 - yBotEnc, currX + xBotEnc, currY + yBotEnc);
    frBox topBox(0 - xTopEnc, 0 - yTopEnc, currX + xTopEnc, currY + yTopEnc);

    frTransform botXform(-currX / 2 + xOffset + xBotOffset, -currY / 2 + yOffset + yBotOffset);
    frTransform topXform(-currX / 2 + xOffset + xTopOffset, -currY / 2 + yOffset + yTopOffset);
    botBox.transform(botXform);
    topBox.transform(topXform);

    botFig->setBBox(botBox);
    topFig->setBBox(topBox);
    botFig->setLayerNum(botLayerNum);
    topFig->setLayerNum(topLayerNum);

    // create via
    auto viaDef = make_unique<frViaDef>(via->name());
    viaDef->addLayer1Fig(std::move(uBotFig));
    viaDef->addLayer2Fig(std::move(uTopFig));
    for (auto &uShape: cutFigs) {
      viaDef->addCutFig(std::move(uShape));
    }
    parser->tech->addVia(std::move(viaDef));
  // RECT defined via
  } else {
    if (via->numPolygons()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupport polygon in def via" <<endl;
      exit(1);
    }
    char* layerName;
    int xl;
    int yl;
    int xh;
    int yh;
    map<frLayerNum, set<int> > lNum2Int;
    for (int i = 0; i < via->numLayers(); ++i) {
      via->layer(i, &layerName, &xl, &yl, &xh, &yh);
      if (parser->tech->name2layer.find(layerName) == parser->tech->name2layer.end()) {
        if (VERBOSE > -1) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: layer " <<layerName <<" is skipiped for " <<via->name() <<endl;
        }
        return 0;
      }
      auto layerNum = parser->tech->name2layer.at(layerName)->getLayerNum();
      lNum2Int[layerNum].insert(i);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"layerNum " <<layerNum <<" i " <<i <<endl;
    }
    if ((int)lNum2Int.size() != 3) {
      if (VERBOSE > -1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported via" <<endl;
      }
      exit(1);
    }
    if (lNum2Int.begin()->first + 2 != (--lNum2Int.end())->first) {
      if (VERBOSE > -1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: non-consecutive layers" <<endl;
      }
      exit(1);
    }
    auto viaDef = make_unique<frViaDef>(via->name());
    int cnt = 0;
    for (auto &[layerNum, intS]: lNum2Int) {
      for (auto i: intS) {
        via->layer(i, &layerName, &xl, &yl, &xh, &yh);
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" + RECT " <<layerName <<" ( " <<xl <<" " <<yl <<" ) ( " <<xh <<" " <<yh <<" )" <<endl;
        }
        unique_ptr<frRect> pinFig = make_unique<frRect>();
        pinFig->setBBox(frBox(xl, yl, xh, yh));
        pinFig->setLayerNum(layerNum);
        switch(cnt) {
          case 0 :
            viaDef->addLayer1Fig(std::move(pinFig));
            break;
          case 1 :
            viaDef->addCutFig(std::move(pinFig));
            break;
          default:
            viaDef->addLayer2Fig(std::move(pinFig));
            break;
        }
      }
      //for (int j = 0; j < via->numPolygons(i); ++j) {
      //  if (enableOutput) {
      //    cout << __FILE__ << ":" << __LINE__ << ": " <<"    POLYGON"; 
      //  }
      //  vector<frPoint> tmpPoints;
      //  for (int k = 0; k < via->getPolygon(i, j).numPoints; k++) {
      //    frCoord x = round(via->getPolygon(i, j).x[k] * parser->tech->getDBUPerUU());
      //    frCoord y = round(via->getPolygon(i, j).y[k] * parser->tech->getDBUPerUU());
      //    tmpPoints.push_back(frPoint(x, y));
      //    if (enableOutput) {
      //       cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<x * 1.0 / parser->tech->getDBUPerUU() <<" " 
      //                  <<y * 1.0 / parser->tech->getDBUPerUU();
      //    }
      //  }
      //  unique_ptr<frPolygon> pinFig = make_unique<frPolygon>();
      //  pinFig->setPoints(tmpPoints);
      //  pinFig->setLayerNum(layerNum);
      //  if (enableOutput) {
      //    cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
      //  }
      //  switch(cnt) {
      //    case 0 :
      //      viaDef->addLayer1Fig(pinFig);
      //      break;
      //    case 1 :
      //      viaDef->addCutFig(pinFig);
      //      break;
      //    default:
      //      viaDef->addLayer2Fig(pinFig);
      //      break;
      //  }
      //}
      cnt++;
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
    }
    parser->tech->addVia(std::move(viaDef));
  }

  return 0;
}

int io::Parser::Callbacks::getDefComponents(defrCallbackType_e type, defiComponent* comp, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if ((type != defrComponentCbkType)) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defrComponentCbkType!" <<endl;
    exit(1);
  }

  io::Parser* parser = (io::Parser*) data;
  if (parser->design->name2refBlock.find(comp->name()) == parser->design->name2refBlock.end()) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: library cell not found!" <<endl;
    }
    exit(1);
  }
  
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"- " <<comp->id() <<" " <<comp->name() <<" + STATUS ( " <<comp->placementX()
         <<" " <<comp->placementY() <<" ) " <<comp->placementOrient() <<endl;
  }

  
  frBlock* refBlock = parser->design->name2refBlock.at(comp->name());
  auto uInst = make_unique<frInst>(comp->id(), refBlock);
  auto tmpInst = uInst.get();
  tmpInst->setId(parser->numInsts);
  parser->numInsts++;
  tmpInst->setOrigin(frPoint(comp->placementX(), comp->placementY()));
  tmpInst->setOrient(frOrientEnum(comp->placementOrient()));
  for (auto &uTerm: tmpInst->getRefBlock()->getTerms()) {
    auto term = uTerm.get();
    unique_ptr<frInstTerm> instTerm = make_unique<frInstTerm>(tmpInst, term);
    instTerm->setId(parser->numTerms);
    parser->numTerms++;
    int pinCnt = term->getPins().size();
    instTerm->setAPSize(pinCnt);
    tmpInst->addInstTerm(std::move(instTerm));
  }
  for (auto &uBlk: tmpInst->getRefBlock()->getBlockages()) {
    auto blk = uBlk.get();
    unique_ptr<frInstBlockage> instBlk = make_unique<frInstBlockage>(tmpInst, blk);
    instBlk->setId(parser->numBlockages);
    parser->numBlockages++;
    tmpInst->addInstBlockage(std::move(instBlk));
  }

  if (parser->tmpBlock->name2inst.find(comp->id()) != parser->tmpBlock->name2inst.end()) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: same cell name!" <<endl;
    }
    exit(1);
  }
  parser->tmpBlock->addInst(std::move(uInst));
  if (parser->tmpBlock->insts.size() < 100000) {
    if (parser->tmpBlock->insts.size() % 10000 == 0) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->insts.size() <<" components" <<endl;
    }
  } else {
    if (parser->tmpBlock->insts.size() % 100000 == 0) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->insts.size() <<" components" <<endl;
    }
  }

  return 0;
}

int io::Parser::Callbacks::getDefString(defrCallbackType_e type, const char* str, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (type == defrDesignStartCbkType) {
    io::Parser* parser = (io::Parser*) data;
    auto &tmpBlock = parser->tmpBlock;
    tmpBlock = make_unique<frBlock>(string(str));
    tmpBlock->trackPatterns.clear();
    tmpBlock->trackPatterns.resize(parser->tech->layers.size());
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"DESIGN " <<tmpBlock->getName() <<" ;" <<endl;
    }
  }
  return 0;
}

int io::Parser::Callbacks::getDefVoid(defrCallbackType_e type, void* variable, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (type == defrDesignEndCbkType) {
    io::Parser* parser = (io::Parser*) data;
    parser->tmpBlock->setId(0);
    parser->design->setTopBlock(
                     std::move(parser->tmpBlock));
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"END DESIGN" <<endl;
    }
  }
  return 0;
}

int io::Parser::Callbacks::getDefDieArea(defrCallbackType_e type, defiBox* box, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (type != defrDieAreaCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defrDieAreaCbkType!" <<endl;
    exit(1);
  }
  vector<frBoundary> bounds;
  frBoundary bound;
  vector<frPoint> points;
  points.push_back(frPoint(box->xl(), box->yl()));
  points.push_back(frPoint(box->xh(), box->yl()));
  points.push_back(frPoint(box->xh(), box->yh()));
  points.push_back(frPoint(box->xl(), box->yh()));
  bound.setPoints(points);
  bounds.push_back(bound);
  io::Parser* parser = (io::Parser*) data;
  parser->tmpBlock->setBoundaries(bounds);
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"DIEAREA ( " <<box->xl() <<" " <<box->yl() <<" ) ( " <<box->xh() <<" " <<box->yh() <<" ) ;" <<endl;
  }
  return 0;
}

int io::Parser::Callbacks::getDefNets(defrCallbackType_e type, defiNet* net, defiUserData data) {
  bool enableOutput = false;
  // bool enableOutput = true;
  bool isSNet = false;

  if (type != defrNetCbkType && type != defrSNetCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defr(S)NetCbkType!" <<endl;
    exit(1);
  }

  if (type == defrSNetCbkType) {
    //cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is Special Net: " <<net->name() <<endl;
    //exit(1);
    isSNet = true;
  }

  io::Parser* parser = (io::Parser*) data;
  unique_ptr<frNet> uNetIn = make_unique<frNet>(net->name());
  auto netIn = uNetIn.get();
  netIn->setId(parser->numNets);
  parser->numNets++;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"- " <<net->name();
  }
  for (int i = 0; i < net->numConnections(); i++) {
    if (enableOutput) {
      if (i % 4 == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<" ";
      }
      cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<net->instance(i) <<" " <<net->pin(i) <<" )";
    }
    
    if (!strcmp(net->instance(i), "PIN")) {
      // IOs
      if (parser->tmpBlock->name2term.find(net->pin(i)) == parser->tmpBlock->name2term.end()) {
        if (VERBOSE > -1) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: term not found!" <<endl;
        }
        exit(1);
      }
      auto term = parser->tmpBlock->name2term[net->pin(i)]; // frTerm*
      term->addToNet(netIn);
      netIn->addTerm(term);
    } else {
      // Instances
      if (!strcmp(net->instance(i), "*")) {
        for (auto &inst: parser->tmpBlock->getInsts()) {
          for (auto &uInstTerm: inst->getInstTerms()) {
            auto instTerm = uInstTerm.get();
            auto name = instTerm->getTerm()->getName();
            if (name == frString(net->pin(i))) {
              instTerm->addToNet(netIn);
              netIn->addInstTerm(instTerm);
              break;
            }
          }
        }
      } else {
        if (parser->tmpBlock->name2inst.find(net->instance(i)) == parser->tmpBlock->name2inst.end()) {
          if (VERBOSE > -1) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: component not found!" <<endl;
          }
          exit(1);
        }
        auto inst = parser->tmpBlock->name2inst[net->instance(i)]; //frInst*
        bool flag =   false;
        for (auto &uInstTerm: inst->getInstTerms()) {
          auto instTerm = uInstTerm.get();
          auto name = instTerm->getTerm()->getName();
          if (name == frString(net->pin(i))) {
            flag = true;
            instTerm->addToNet(netIn);
            netIn->addInstTerm(instTerm);
            break;
          }
        }
        if (!flag) {
          if (VERBOSE > -1) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: component pin not found!" <<endl;
          }
          exit(1);
        }
      }
    }
  }
  // read pre-route
  //cout << __FILE__ << ":" << __LINE__ << ": " << "Net " << net->name() << " has " << net->numWires() << " wires\n";
  //cout << __FILE__ << ":" << __LINE__ << ": " << "Net " << net->name() << " has " << net->numPaths() << " paths\n"; // no paths
  //cout << __FILE__ << ":" << __LINE__ << ": " << "Net " << net->name() << " has " << net->numVpins() << " vpins\n"; // no vpins
  
  // initialize
  string layerName   = "";
  string viaName     = "";
  string shape       = "";
  bool hasBeginPoint = false;
  bool hasEndPoint   = false;
  frCoord beginX     = -1;
  frCoord beginY     = -1;
  frCoord beginExt   = -1;
  frCoord endX       = -1;
  frCoord endY       = -1;
  frCoord endExt     = -1;
  bool hasRect       = false;
  frCoord left       = -1;
  frCoord bottom     = -1;
  frCoord right      = -1;
  frCoord top        = -1;
  frCoord width      = 0;
  for (int i = 0; i < (int)net->numWires(); i++) {
    defiWire* tmpWire = net->wire(i);
    //cout << __FILE__ << ":" << __LINE__ << ": " << "Wire " << i << "\n";
    //cout << __FILE__ << ":" << __LINE__ << ": " << "  Type: " << tmpWire->wireType() << endl;
    //cout << __FILE__ << ":" << __LINE__ << ": " << "  has " << tmpWire->numPaths() << " paths\n";
    
    if (enableOutput) {
       cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  + " <<tmpWire->wireType();
    }
    // each path is a def line
    for (int j = 0; j < (int)tmpWire->numPaths(); j++) {
      defiPath* path     = tmpWire->path(j);
      path->initTraverse();
      // initialize
      layerName     = "";
      viaName       = "";
      shape         = "";
      hasBeginPoint = false;
      hasEndPoint   = false;
      beginX        = -1;
      beginY        = -1;
      beginExt      = -1;
      endX          = -1;
      endY          = -1;
      endExt        = -1;
      hasRect       = false;
      left          = -1;
      bottom        = -1;
      right         = -1;
      top           = -1;
      width         = 0;
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"path here" <<endl;
      
      int pathId;
      while ((pathId = path->next()) != DEFIPATH_DONE) {
        //cout << __FILE__ << ":" << __LINE__ << ": " << "  pathId = " << pathId << endl;
        switch(pathId) {
          case DEFIPATH_LAYER:
            layerName = string(path->getLayer());
            if (parser->tech->name2layer.find(layerName) == parser->tech->name2layer.end()) {
              if (VERBOSE > -1) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported layer: " <<layerName <<endl;
              }
              exit(1);
            }
            if (enableOutput) {
              if (!j) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<layerName;
              } else {
                cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    NEW " <<layerName;
              }
            }
            break;
          case DEFIPATH_VIA:
            viaName = string(path->getVia());
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<viaName;
            }
            break;
          case DEFIPATH_WIDTH:
            width = path->getWidth();
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<width;
            }
            break;
          case DEFIPATH_POINT:
            if (!hasBeginPoint) {
              path->getPoint(&beginX, &beginY);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<beginX <<" " <<beginY <<" )";
              }
              hasBeginPoint = true;
            } else {
              path->getPoint(&endX, &endY);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<endX <<" " <<endY <<" )";
              }
              hasEndPoint = true;
            }
            break;
          case DEFIPATH_FLUSHPOINT:
            if (!hasBeginPoint) {
              path->getFlushPoint(&beginX, &beginY, &beginExt);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<beginX <<" " <<beginY <<" " <<beginExt <<" )";
              }
              hasBeginPoint = true;
            } else {
              path->getFlushPoint(&endX, &endY, &endExt);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<endX <<" " <<endY <<" " <<endExt <<" )";
              }
              hasEndPoint = true;
            }
            break;
          case DEFIPATH_SHAPE:
            shape = path->getShape();
            beginExt = 0;
            endExt = 0;
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" + SHAPE " <<shape;
            }
            break;
          case DEFIPATH_RECT:
            path->getViaRect(&left, &bottom, &right, &top);
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" RECT ( " <<left <<" " <<bottom <<" " <<right <<" " <<top <<" )";
            }
            hasRect = true;
            break;
          case DEFIPATH_VIRTUALPOINT:
            if (!hasBeginPoint) {
              path->getVirtualPoint(&beginX, &beginY);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<beginX <<" " <<beginY <<" )";
              }
              hasBeginPoint = true;
            } else {
              path->getVirtualPoint(&endX, &endY);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " <<" ( " <<endX <<" " <<endY <<" )";
              }
              hasEndPoint = true;
            }
            break;
          default : cout << __FILE__ << ":" << __LINE__ << ": " <<" net " <<net->name() <<" unknown pathId " <<pathId <<endl; break;
        }
      }


      auto layerNum = parser->tech->name2layer[layerName]->getLayerNum();
      // add rect
      if (hasRect) {
        continue;
      }

      // add wire, currently do not consider extension
      if (hasEndPoint) {
        // route
        auto tmpP = make_unique<frPathSeg>();

        // avoid begin > end case
        if (beginX > endX || beginY > endY) {
          tmpP->setPoints(frPoint(endX, endY), frPoint(beginX, beginY));
          swap(beginExt, endExt);
        } else {
          tmpP->setPoints(frPoint(beginX, beginY), frPoint(endX, endY));
        }
        tmpP->addToNet(netIn);
        tmpP->setLayerNum(layerNum);

        width = (width) ? width : parser->tech->name2layer[layerName]->getWidth();
        auto defaultBeginExt = width / 2;
        auto defaultEndExt   = width / 2;

        frEndStyleEnum tmpBeginEnum;
        if (beginExt == -1) {
          tmpBeginEnum = frcExtendEndStyle;
        } else if (beginExt == 0) {
          tmpBeginEnum = frcTruncateEndStyle;
        } else {
          tmpBeginEnum = frcVariableEndStyle;
        }
        frEndStyle tmpBeginStyle(tmpBeginEnum);

        frEndStyleEnum tmpEndEnum;
        if (endExt == -1) {
          tmpEndEnum = frcExtendEndStyle;
        } else if (endExt == 0) {
          tmpEndEnum = frcTruncateEndStyle;
        } else {
          tmpEndEnum = frcVariableEndStyle;
        }
        frEndStyle tmpEndStyle(tmpEndEnum);

        frSegStyle tmpSegStyle;
        tmpSegStyle.setWidth(width);
        tmpSegStyle.setBeginStyle(tmpBeginStyle, tmpBeginEnum == frcExtendEndStyle ? defaultBeginExt : beginExt);
        tmpSegStyle.setEndStyle(tmpEndStyle, tmpEndEnum == frcExtendEndStyle ? defaultEndExt : endExt);
        tmpP->setStyle(tmpSegStyle);
        netIn->addShape(std::move(tmpP));
      }

      // add via
      if (viaName != "") {
        if (parser->tech->name2via.find(viaName) == parser->tech->name2via.end()) {
          if (VERBOSE > -1) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported via: " <<viaName <<endl;
          }
        } else {
          frPoint p;
          if (hasEndPoint) {
            p.set(endX, endY);
          } else {
            p.set(beginX, beginY);
          }
          auto viaDef = parser->tech->name2via[viaName];
          auto tmpP = make_unique<frVia>(viaDef);
          tmpP->setOrigin(p);
          tmpP->addToNet(netIn);
          netIn->addVia(std::move(tmpP));
        }
      }
    } // end path
  } // end wire
 
  frNetEnum netType = frNetEnum::frcNormalNet;
  if (net->hasUse()) {
    string str(net->use());
    if (str == "SIGNAL") {
      ;
    } else if (str == "CLOCK") {
      netType = frNetEnum::frcClockNet;
    } else if (str == "POWER") {
      netType = frNetEnum::frcPowerNet;
    } else if (str == "GROUND") {
      netType = frNetEnum::frcGroundNet;
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported NET USE in def" <<endl;
      exit(1);
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" + USE " <<str <<endl;
    }
  }
  netIn->setType(netType);

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<" ;" <<endl;
  }

  if (isSNet) {
    parser->tmpBlock->addSNet(std::move(uNetIn)); 
    if (parser->tmpBlock->snets.size() < 100000) {
      if (parser->tmpBlock->snets.size() % 10000 == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->snets.size() <<" snets" <<endl;
      }
    } else {
      if (parser->tmpBlock->snets.size() % 10000 == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->snets.size() <<" snets" <<endl;
      }
    }
  } else {
    parser->tmpBlock->addNet(std::move(uNetIn)); 
    if (parser->tmpBlock->nets.size() < 100000) {
      if (parser->tmpBlock->nets.size() % 10000 == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->nets.size() <<" nets" <<endl;
      }
    } else {
      if (parser->tmpBlock->nets.size() % 100000 == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->nets.size() <<" nets" <<endl;
      }
    }
  }

  return 0;
}

int io::Parser::Callbacks::getDefTerminals(defrCallbackType_e type, defiPin* term, defiUserData data) {
  bool enableOutput = false;
  //bool enableOutput = true;
  if (type != defrPinCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defrPinCbkType!" <<endl;
    exit(1);
  }

  frTermEnum termType = frTermEnum::frcNormalTerm;

  if (term->hasUse()) {
    string str(term->use());
    if (str == "SIGNAL") {
      ;
    } else if (str == "CLOCK") {
      termType = frTermEnum::frcClockTerm;
    } else if (str == "POWER") {
      termType = frTermEnum::frcPowerTerm;
    } else if (str == "GROUND") {
      termType = frTermEnum::frcGroundTerm;
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported PIN USE in lef" <<endl;
      exit(1);
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    USE " <<str <<" ;" <<endl;
    }
  }

  io::Parser* parser = (io::Parser*) data;
  if (term->hasPort()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: multiple pin ports existing in DEF" <<endl;
    exit(1);
  } else {
    // term
    auto uTermIn = make_unique<frTerm>(term->pinName());
    auto termIn = uTermIn.get();
    termIn->setId(parser->numTerms);
    parser->numTerms++;
    termIn->setType(termType);
    // term should add pin
    // pin
    auto pinIn  = make_unique<frPin>();
    pinIn->setId(0);
    for (int i = 0; i < term->numLayer(); ++i) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerName= " <<term->layer(i) <<endl;
      string layer = term->layer(i);
      if (parser->tech->name2layer.find(layer) == parser->tech->name2layer.end()) {
        if (VERBOSE > -1) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported layer: " <<layer <<endl;
        }
        //continue;
        exit(1);
      }

      frLayerNum layerNum = parser->tech->name2layer[layer]->getLayerNum();
      frCoord xl = 0;
      frCoord yl = 0;
      frCoord xh = 0;
      frCoord yh = 0;
      term->bounds(i, &xl, &yl, &xh, &yh);
      // pinFig
      unique_ptr<frRect> pinFig = make_unique<frRect>();
      pinFig->setBBox(frBox(xl, yl, xh, yh));
      pinFig->addToPin(pinIn.get());
      pinFig->setLayerNum(layerNum);
      pinFig->move(frTransform(term->placementX(), term->placementY(), frOrientEnum(term->orient())));
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"move" <<endl;
      frBox transformedBBox;
      pinFig->getBBox(transformedBBox);
      // pinFig completed
      // pin
      unique_ptr<frPinFig> uptr(std::move(pinFig));
      pinIn->addPinFig(std::move(uptr));
      // pin completed
    }
    // polygon
    for (int i = 0; i < term->numPolygons(); ++i) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"  polyName= " <<term->polygonName(i) <<endl;
      string layer = term->polygonName(i);
      if (parser->tech->name2layer.find(layer) == parser->tech->name2layer.end()) {
        if (VERBOSE > -1) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported layer: " <<layer <<endl;
        }
        //continue;
        exit(1);
      }

      frLayerNum layerNum = parser->tech->name2layer[layer]->getLayerNum();
      auto polyPoints = term->getPolygon(i);
      frCollection<frPoint> tmpPoints;
      for (int j = 0; j < polyPoints.numPoints; j++) {
        tmpPoints.push_back(frPoint((polyPoints.x)[j], (polyPoints.y)[j]));
      }

      // pinFig
      unique_ptr<frPolygon> pinFig = make_unique<frPolygon>();
      pinFig->setPoints(tmpPoints);
      pinFig->addToPin(pinIn.get());
      pinFig->setLayerNum(layerNum);
      pinFig->move(frTransform(term->placementX(), term->placementY(), frOrientEnum(term->orient())));
      // pinFig completed
      // pin
      unique_ptr<frPinFig> uptr(std::move(pinFig));
      pinIn->addPinFig(std::move(uptr));
      // pin completed
    }
    termIn->addPin(std::move(pinIn));
    //cout << __FILE__ << ":" << __LINE__ << ": " <<"  placeXY  = (" <<term->placementX() <<"," <<term->placementY() <<")" <<endl;
    parser->tmpBlock->addTerm(std::move(uTermIn));
  }

  if (parser->tmpBlock->terms.size() % 1000 == 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"defIn read " <<parser->tmpBlock->terms.size() <<" pins" <<endl;
  }

  return 0;

}

int io::Parser::Callbacks::getDefTracks(defrCallbackType_e type, defiTrack* track, defiUserData data) {
  bool enableOutput = false;
  //bool enableOutput = true;
  if (type != defrTrackCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not defrTrackCbkType!" <<endl;
    exit(1);
  }

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"TRACKS " <<track->macro() <<" " <<track->x() 
         <<" DO " <<track->xNum() <<" STEP " <<track->xStep() 
         <<" LAYER " <<track->layer(0) <<endl;
  }

  io::Parser* parser = (io::Parser*) data;
  unique_ptr<frTrackPattern> tmpTrackPattern = make_unique<frTrackPattern>();
  if (parser->tech->name2layer.find(track->layer(0)) == parser->tech->name2layer.end()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find layer: " <<track->layer(0) <<endl;
    exit(2);
  }
  tmpTrackPattern->setLayerNum(parser->tech->name2layer.at(track->layer(0))->getLayerNum());
  if (!strcmp(track->macro(), "X")) {
    tmpTrackPattern->setHorizontal(true);
  } else if (!strcmp(track->macro(), "Y")) {
    tmpTrackPattern->setHorizontal(false);
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupporterd direction: " <<track->macro() <<endl;
    exit(2);
  }
  tmpTrackPattern->setStartCoord(track->x());
  tmpTrackPattern->setNumTracks(track->xNum());
  tmpTrackPattern->setTrackSpacing(track->xStep());
  parser->tmpBlock->trackPatterns.at(tmpTrackPattern->getLayerNum()).push_back(std::move(tmpTrackPattern));
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"here" <<endl;

  return 0;
}

int io::Parser::Callbacks::getDefUnits(defrCallbackType_e type, double number, defiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  io::Parser* parser = (io::Parser*) data;
  parser->tmpBlock->setDBUPerUU(static_cast<frUInt4>(number));
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"UNITS DISTANCE MICRONS " <<parser->tmpBlock->getDBUPerUU() <<" ;" <<endl;
  }
  return 0;
}

void io::Parser::readDef() {
  ProfileTask profile("IO:readDef");
  FILE* f;
  int res;
  
  defrInit();
  defrReset();

  defrInitSession(1);
  
  defrSetUserData((defiUserData)this);

  defrSetDesignCbk(Callbacks::getDefString);
  defrSetDesignEndCbk(Callbacks::getDefVoid);
  defrSetDieAreaCbk(Callbacks::getDefDieArea);
  defrSetUnitsCbk(Callbacks::getDefUnits);
  defrSetTrackCbk(Callbacks::getDefTracks);
  defrSetComponentCbk(Callbacks::getDefComponents);
  defrSetPinCbk(Callbacks::getDefTerminals);
  defrSetSNetCbk(Callbacks::getDefNets);
  defrSetNetCbk(Callbacks::getDefNets);
  defrSetAddPathToNet();
  defrSetViaCbk(Callbacks::getDefVias);
  defrSetBlockageCbk(Callbacks::getDefBlockages);

  if ((f = fopen(DEF_FILE.c_str(),"r")) == 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Couldn't open def file" <<endl;
    exit(2);
  }

  res = defrRead(f, DEF_FILE.c_str(), (defiUserData)this, 1);
  if (res != 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"DEF parser returns an error!" <<endl;
    exit(2);
  }
  fclose(f);

  defrClear();

  // add fake nets for floating PG
  addFakeNets();
}

void io::Parser::addFakeNets() {
  // add VSS fake net
  auto vssFakeNet = make_unique<frNet>(string("frFakeVSS"));
  vssFakeNet->setType(frNetEnum::frcGroundNet);
  vssFakeNet->setIsFake(true);
  design->getTopBlock()->addFakeSNet(std::move(vssFakeNet));
  // add VDD fake net
  auto vddFakeNet = make_unique<frNet>(string("frFakeVDD"));
  vddFakeNet->setType(frNetEnum::frcPowerNet);
  vddFakeNet->setIsFake(true);
  design->getTopBlock()->addFakeSNet(std::move(vddFakeNet));
}

int io::Parser::getLef58SpacingTable_parallelRunLength(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  SPACINGTABLE" <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  PARALLELRUNLENGTH";
  }

  bool    isWrongDirection    = false;
  bool    isSameMask          = false;

  bool    exceptEol           = false;
  frCoord eolWidth            = 0;

  frCoord lowExcludeSpacing   = 0;
  frCoord highExcludeSpacing  = 0;

  // 2d spacing table
  frCollection<frCoord> rowVals, colVals;
  frCollection<frCollection<frCoord> > tblVals;
  frCollection<frCoord> tblRowVals;
  
  // except within
  map<frCoord, pair<frCoord, frCoord> > ewVals;

  int stage = 0;
  io::Parser* parser = (io::Parser*) data;

  istringstream istr(sIn);
  string word;
  while (istr >> word) {
    if (word == string("WRONGDIRECTION")) {
      //isWrongDirection = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WRONGDIRECTION";
      }
    } else if (word == string("SAMEMASK")) {
      isSameMask = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" SAMEMASK";
      }
    } else if (word == string("EXCEPTEOL")) {
      double tmp;
      if (istr >> tmp) {
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" EXCEPTEOL " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58SpacingTable_parallelRunLength" <<endl;
      }
    } else if (word == string("EXCEPTWITHIN")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" EXCEPTWITHIN";
      }
      double tmp;
      if (istr >> tmp) {
        lowExcludeSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58SpacingTable_parallelRunLength" <<endl;
      }
      if (istr >> tmp) {
        highExcludeSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58SpacingTable_parallelRunLength" <<endl;
      }
      ewVals[rowVals.size()-1] = make_pair(lowExcludeSpacing, highExcludeSpacing);
    } else if (word == string("WIDTH")) {
      if (tblRowVals.size()) {
        tblVals.push_back(tblRowVals);
        tblRowVals.clear();
      }
      stage = 1;
      double tmp;
      if (istr >> tmp) {
        rowVals.push_back(frCoord(round(tmp * parser->tech->getDBUPerUU())));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  WIDTH " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58SpacingTable_parallelRunLength" <<endl;
      }
    } else if (word == string(";")) {
      if (stage == 1 && tblRowVals.size()) {
        tblVals.push_back(tblRowVals);
        tblRowVals.clear();
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;";
      }
    } else {
      // get length
      if (stage == 0) {
        colVals.push_back(frCoord(round(stod(word) * parser->tech->getDBUPerUU())));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
      }
      if (stage == 1) {
        tblRowVals.push_back(frCoord(round(stod(word) * parser->tech->getDBUPerUU())));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
      }
    }
  }

  string rowName("WIDTH");
  string colName("PARALLELRUNLENGTH");
  shared_ptr<fr2DLookupTbl<frCoord, frCoord, frCoord> > prlTbl = make_shared<
    fr2DLookupTbl<frCoord, frCoord, frCoord> >(rowName, rowVals, colName, colVals, tblVals);
  shared_ptr<frLef58SpacingTableConstraint> spacingTableConstraint = make_shared<frLef58SpacingTableConstraint>(prlTbl, ewVals);
  spacingTableConstraint->setWrongDirection(isWrongDirection);
  spacingTableConstraint->setSameMask(isSameMask);
  if (exceptEol) {
    spacingTableConstraint->setEolWidth(eolWidth);
  }
  
  parser->tech->addConstraint(spacingTableConstraint);
  tmpLayer->addConstraint(spacingTableConstraint);

  return 0;
}

int io::Parser::getLef58SpacingTable(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_SPACINGTABLE \"";
  }
  istringstream istr(sIn);
  string word;
  stringstream ss;

  string keyword;
  while (istr >> word) {
    if (word == string("SPACINGTABLE")) {
      ss.str("");
    } else if (word == string("PARALLELRUNLENGTH")) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"found PARALLELRUNLENGTH" <<endl;
      keyword = "PARALLELRUNLENGTH";
    } else if (word == string(";")) {
      ss <<" " <<word;
      if (keyword == string("PARALLELRUNLENGTH")) {
        getLef58SpacingTable_parallelRunLength(data, tmpLayer, ss.str());
      }
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"found ;" <<endl;
    } else {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"found " <<word <<endl;
      ss <<" " <<word;
    }
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<ss.str() <<endl;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  }

  return 0;
}


int io::Parser::getLef58Spacing_endOfLineWithin(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  // minspacing
  frCoord eolSpace           = 0;
  frCoord eolWidth           = 0;

  bool    hasExactWidth      = false;

  bool    hasWrongDirSpacing = false;
  frCoord wrongDirSpace      = 0;

  bool    hasOppositeWidth   = false;
  frCoord oppositeWidth      = 0;

  frCoord eolWithin          = 0;

  bool    hasWrongDirWithin  = false;
  frCoord wrongDirWithin     = 0;

  bool    hasSameMask        = false;

  // ENDTOEND
  bool    hasEndToEnd          = false;
  frCoord endToEndSpace        = 0;
  bool    hasCutSpace          = false;
  frCoord oneCutSpace          = 0;
  frCoord twoCutSpace          = 0;
  bool    hasExtension         = false;
  frCoord extension            = 0;
  bool    hasWrongDirExtension = false;
  frCoord wrongDirExtension    = 0;
  bool    hasOtherEndWidth     = false;
  frCoord otherEndWidth        = 0;
  
  // MINLENGTH/MAXLENGTH
  bool    hasLength          = false;
  bool    isMax              = false;
  frCoord length             = 0;
  bool    hasTwoSides        = false;

  // PARALLELEDGE
  bool    hasParallelEdge          = false;
  bool    hasSubtractEolWidth      = false;
  frCoord parSpace                 = 0;
  frCoord parWithin                = 0;
  bool    hasPrl                   = false;
  frCoord prl                      = 0;
  bool    hasParallelEdgeMinLength = false;
  frCoord parallelEdgeMinLength    = 0;
  bool    hasTwoEdges              = false;
  bool    hasSameMetal             = false;
  bool    hasNonEolCornerOnly      = false;
  bool    hasParallelSameMask      = false;

  bool    skip = false;
  io::Parser* parser = (io::Parser*) data;

  istringstream istr(sIn);
  string word;
  int stage = 0;
  while (istr >> word) {
    if (word == string("SPACING")) {
      double tmp;
      if (istr >> tmp) {
        eolSpace = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  SPACING " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (word == string("ENDOFLINE")) {
      double tmp;
      if (istr >> tmp) {
        eolWidth = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ENDOFLINE " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (word == string("EXACTWIDTH")) {
      hasExactWidth = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" EXACTWIDTH";
      }
      stage = 0;
    } else if (word == string("WRONGDIRSPACING")) {
      hasWrongDirSpacing = true;
      double tmp;
      if (istr >> tmp) {
        wrongDirSpace = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" WRONGDIRSPACING " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (word == string("OPPOSITEWIDTH")) {
      hasOppositeWidth = true;
      double tmp;
      if (istr >> tmp) {
        oppositeWidth = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" OPPOSITEWIDTH " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (word == string("WITHIN")) {
      double tmp;
      if (istr >> tmp) {
        eolWithin = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" WITHIN " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 1;
    } else if (word == string("SAMEMASK")) {
      hasSameMask = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" SAMEMASK";
      }
      stage = 0;
    } else if (word == string("EXCEPTEXACTWIDTH")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" EXCEPTEXACTWIDTH(SKIP)";
      }
      stage = 0;
      skip = true;
    } else if (word == string("FILLCONCAVECORNER")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" FILLCONCAVECORNER(SKIP)";
      }
      stage = 0;
      skip = true;
    } else if (word == string("WITHCUT")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WITHCUT(SKIP)";
      }
      stage = 0;
    } else if (word == string("ENDPRLSPACING")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ENDPRLSPACING(SKIP)";
      }
      stage = 0;
      skip = true;
    } else if (word == string("ENDTOEND")) {
      hasEndToEnd = true;
      double tmp;
      if (istr >> tmp) {
        endToEndSpace = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ENDTOEND " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 2;
    } else if (word == string("MAXLENGTH")) {
      hasLength = true;
      isMax     = true;
      double tmp;
      if (istr >> tmp) {
        length = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" MAXLENGTH " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (stage != 3 && word == string("MINLENGTH")) {
      hasLength = true;
      isMax     = false;
      double tmp;
      if (istr >> tmp) {
        length = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" MINLENGTH " <<tmp;
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      stage = 0;
    } else if (word == string("TWOSIDES")) {
      hasTwoSides = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" TWOSIDES";
      }
      stage = 0;
    } else if (word == string("EQUALRECTWIDTH")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" EQUALRECTWIDTH(SKIP)";
      }
      stage = 0;
      skip = true;
    } else if (word == string("PARALLELEDGE")) {
      hasParallelEdge = true;
      string tmp;
      // read to parSpace
      if (istr >> tmp) {
        if (tmp == string("SUBTRACTEOLWIDTH")) {
          hasSubtractEolWidth = true;
          double tmp2;
          if (istr >> tmp2) {
            parSpace = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" PARALLELEDGE SUBTRACTEOLWIDTH " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        } else {
          parSpace = frCoord(round(stod(tmp) * parser->tech->getDBUPerUU()));
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" PARALLELEDGE " <<tmp;
          }
        }
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
      }
      // read to parWithin
      if (istr >> tmp) {
        if (tmp == string("WITHIN")) {
          double tmp2;
          if (istr >> tmp2) {
            parWithin = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" (PE)WITHIN " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
        }
      }
      stage = 3;
    } else if (word == string("ENCLOSECUT")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ENCLOSECUT(SKIP)";
      }
      stage = 0;
      skip = true;
    } else if (word == string(";")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;";
      }
      stage = 0;
    } else {
      // stage = 1, read wrongDirWithin
      if (stage == 1) {
        hasWrongDirWithin = true;
        cout << __FILE__ << ":" << __LINE__ << ": " <<flush;
        wrongDirWithin = frCoord(round(stod(word) * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
        stage = 0;
      // stage = 2, read end to end from onecutspace
      } else if (stage == 2) {
        auto tmp = word;
        if (tmp == string("EXTENSION")) {
          hasExtension = true;
          double tmp2;
          if (istr >> tmp2) {
            extension = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" EXTENSION " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
          stage = 20;
        } else if (tmp == string("OTHERENDWIDTH")) {
          hasOtherEndWidth = true;
          double tmp2;
          if (istr >> tmp2) {
            otherEndWidth = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" OTHERENDWIDTH " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        } else {
          hasCutSpace = true;
          oneCutSpace = frCoord(round(stod(tmp) * parser->tech->getDBUPerUU()));
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tmp;
          }
          double tmp2;
          if (istr >> tmp2) {
            twoCutSpace = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        }
      // stage = 20, read end to end wrongDirExtension
      } else if (stage == 20) {
        hasWrongDirExtension = true;
        wrongDirExtension = frCoord(round(stod(word) * parser->tech->getDBUPerUU()));
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
        stage = 2;
      // stage = 3, read paralleledge from prl
      } else if (stage == 3) {
        if (word == string("PRL")) {
          hasPrl = true;
          double tmp2;
          if (istr >> tmp2) {
            prl = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" PRL " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        } else if (word == string("MINLENGTH")) {
          hasParallelEdgeMinLength = true;
          double tmp2;
          if (istr >> tmp2) {
            parallelEdgeMinLength = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" (PE)MINLENGTH " <<tmp2;
            }
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58Spacing_eolSpace" <<endl;
          }
        } else if (word == string("TWOEDGES")) {
          hasTwoEdges = true;
        } else if (word == string("SAMEMETAL")) {
          hasSameMetal = true;
        } else if (word == string("NONEOLCORNERONLY")) {
          hasNonEolCornerOnly = true;
        } else if (word == string("PARALLELSAMEMASK")) {
          hasParallelSameMask = true;
        } else {
          ;
        }
      } else {
        ;
      }
    }
  }

  if (skip) {
    ;
  } else {
    auto con = make_shared<frLef58SpacingEndOfLineConstraint>();
    con->setEol(eolSpace, eolWidth, hasExactWidth);
    if (hasWrongDirSpacing) {
      con->setWrongDirSpace(wrongDirSpace);
    }

    auto within = make_shared<frLef58SpacingEndOfLineWithinConstraint>();
    con->setWithinConstraint(within);
    if (hasOppositeWidth) {
      within->setOppositeWidth(oppositeWidth);
    }
    within->setEolWithin(eolWithin);
    if (hasWrongDirWithin) {
      within->setWrongDirWithin(wrongDirWithin);
    }
    if (hasSameMask) {
      within->setSameMask(hasSameMask);
    }
    if (hasEndToEnd) {
      auto endToEnd = make_shared<frLef58SpacingEndOfLineWithinEndToEndConstraint>();
      within->setEndToEndConstraint(endToEnd);
      endToEnd->setEndToEndSpace(endToEndSpace);
      if (hasCutSpace) {
        endToEnd->setCutSpace(oneCutSpace, twoCutSpace);
      }
      if (hasExtension) {
        if (hasWrongDirExtension) {
          endToEnd->setExtension(extension, wrongDirExtension);
        } else {
          endToEnd->setExtension(extension);
        }
      }
      if (hasOtherEndWidth) {
        endToEnd->setOtherEndWidth(otherEndWidth);
      }
    }
    if (hasParallelEdge) {
      auto parallelEdge = make_shared<frLef58SpacingEndOfLineWithinParallelEdgeConstraint>();
      within->setParallelEdgeConstraint(parallelEdge);
      if (hasSubtractEolWidth) {
        parallelEdge->setSubtractEolWidth(hasSubtractEolWidth);
      }
      parallelEdge->setPar(parSpace, parWithin);
      if (hasPrl) {
        parallelEdge->setPrl(prl);
      }
      if (hasParallelEdgeMinLength) {
        parallelEdge->setMinLength(parallelEdgeMinLength);
      }
      if (hasTwoEdges) {
        parallelEdge->setTwoEdges(hasTwoEdges);
      }
      if (hasSameMetal) {
        parallelEdge->setSameMetal(hasSameMetal);
      }
      if (hasNonEolCornerOnly) {
        parallelEdge->setNonEolCornerOnly(hasNonEolCornerOnly);
      }
      if (hasParallelSameMask) {
        parallelEdge->setParallelSameMask(hasParallelSameMask);
      }
    }
    if (hasLength) {
      auto len = make_shared<frLef58SpacingEndOfLineWithinMaxMinLengthConstraint>();
      within->setMaxMinLengthConstraint(len);
      len->setLength(isMax, length, hasTwoSides);
    }

    parser->tech->addConstraint(con);
    tmpLayer->lef58SpacingEndOfLineConstraints.push_back(con);
  }

  //if (enableOutput) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  //}

  return 0;
}


int io::Parser::getLef58Spacing(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_SPACING \"";
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<sIn <<endl;
  istringstream istr(sIn);
  string word;
  stringstream ss;

  string keyword;
  string keyword2;
  while (istr >> word) {
    if (word == string("SPACING")) {
      ss.str("");
      ss <<word;
      keyword = "";
    } else if (word == string("EOLPERPENDICULAR")) {
      keyword = "EOLPERPENDICULAR";
      ss <<" " <<word;
    } else if (word == string("AREA")) {
      keyword = "AREA";
      ss <<" " <<word;
    } else if (word == string("LAYER")) {
      keyword = "LAYER";
      ss <<" " <<word;
    } else if (word == string("NOTCHLENGTH")) {
      keyword = "NOTCHLENGTH";
      ss <<" " <<word;
    } else if (word == string("NOTCHSPAN")) {
      keyword = "NOTCHSPAN";
      ss <<" " <<word;
    } else if (word == string("ENDOFLINE")) {
      keyword = "ENDOFLINE";
      ss <<" " <<word;
    } else if (word == string("CONVEXCORNERS")) {
      keyword = "CONVEXCORNERS";
      ss <<" " <<word;
    } else if (word == string("TOCONCAVECORNER")) {
      keyword2 = "TOCONCAVECORNER";
      ss <<" " <<word;
    } else if (word == string("TONOTCHLENGTH")) {
      keyword2 = "TONOTCHLENGTH";
      ss <<" " <<word;
    } else if (word == string(";")) {
      ss <<" " <<word;
      if (keyword == string("ENDOFLINE")) {
        if (keyword2 == string("")) {
          getLef58Spacing_endOfLineWithin(data, tmpLayer, ss.str());
        } else if (keyword2 == string("TOCONCAVECORNER")) {
          ;
        } else if (keyword2 == string("TONOTCHLENGTH")) {
          ;
        } else {
          ;
        }
      } else {
        ; // has keyword, or SAMEMASK, or WRONGDIRECTION, or nothing
      }
    } else {
      ss <<" " <<word;
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  }
  return 0;
}

// only support GF14 related content
int io::Parser::getLef58MinStep(void *data, frLayer* tmpLayer, const string &sIn) {
  bool enableOutput = true;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << endl << "  PROPERTY LEF58_MINSTEP \n";
  }

  bool isSkip = false;
  istringstream istr(sIn);
  string word;

  frCoord minStepLength = -1;
  int maxEdges = -1;
  frCoord minAdjLength = -1;
  frCoord eolWidth = -1;
  io::Parser* parser = (io::Parser*) data;

  while (istr >> word) {
    if (word == string("MINSTEP")) {
      isSkip = false;
      double tmp;
      if (istr >> tmp) {
        minStepLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        isSkip = true;
      }
    } else if (word == string("MAXEDGES")) {
      int tmp;
      if (istr >> tmp) {
        maxEdges = tmp;
      } else {
        isSkip = true;
      }
    } else if (word == string("MINADJACENTLENGTH")) {
      double tmp;
      if (istr >> tmp) {
        minAdjLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        isSkip = true;
      }
    } else if (word == string("NOBETWEENEOL")) {
      double tmp;
      if (istr >> tmp) {
        eolWidth = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        isSkip = true;
      }
    } else if (word == ";") {
      if (!isSkip) {
        auto con = make_unique<frLef58MinStepConstraint>();
        con->setMinStepLength(minStepLength);
        con->setMaxEdges(maxEdges);
        con->setMinAdjacentLength(minAdjLength);
        con->setEolWidth(eolWidth);
        tmpLayer->addLef58MinStepConstraint(con.get());
        parser->tech->addUConstraint(std::move(con));
        // cout << __FILE__ << ":" << __LINE__ << ": " << "Adding lef58MinStep con for layer " << tmpLayer->getName() << "\n";
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: unsupported LEF58_MINSTEP rule branch...\n";
        cout << __FILE__ << ":" << __LINE__ << ": " << sIn << endl;
      }
    } else {
      isSkip = true;
    }
  }

  
  return 0;
}

int io::Parser::getLef58CutClass(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_CUTCLASS \"";
  }
  istringstream istr(sIn);
  string word;

  string  name       = "";
  frCoord viaWidth   = 0;
  bool    hViaLength = false;
  frCoord viaLength  = 0;
  bool    hNumCut    = false;
  frUInt4 numCut     = 0;
  io::Parser* parser = (io::Parser*) data;

  while (istr >> word) {
    if (word == string("CUTCLASS")) {
      // initialization
      name       = "";
      viaWidth   = 0;
      hViaLength = false;
      viaLength  = 0;
      hNumCut    = false;
      numCut     = 0;
      if (istr >> name) {
        ;
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutClass" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  CUTCLASS " <<name;
      }
    } else if (word == "WIDTH") {
      double tmp;
      if (istr >> tmp) {
        viaWidth = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutClass" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WIDTH " <<tmp;
      }
    } else if (word == "LENGTH") {
      double tmp;
      if (istr >> tmp) {
        hViaLength = true;
        viaLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutClass" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" LENGTH " <<tmp;
      }
    } else if (word == "CUTS") {
      if (istr >> numCut) {
        hNumCut = true;
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutClass" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" CUTS " <<numCut;
      }
    } else if (word == ";") {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;";
      }
      // push rule here;
      auto cutClass = make_unique<frLef58CutClass>();
      cutClass->setName(name);
      cutClass->setViaWidth(viaWidth);
      if (hViaLength) {
        cutClass->setViaLength(viaLength);
      } else {
        cutClass->setViaLength(viaWidth);
      }
      if (hNumCut) {
        cutClass->setNumCut(numCut);
      } else {
        cutClass->setNumCut(1);
      }
      parser->tech->addCutClass(tmpLayer->getLayerNum(), std::move((cutClass)));
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  }
  return 0;
}

int io::Parser::getLef58CutSpacing_helper(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  //bool enableOutput = false;

  string keyword = "";

  istringstream istr(sIn);
  string word;
  stringstream ss;
  while (istr >> word) {
    if (word == string("SPACING")) {
      keyword = "";
      ss.str("");
      ss <<word;
    } else if (keyword == "" && word == string("SAMEMASK")) {
      keyword = "SAMEMASK";
      ss <<" " <<word;
    } else if (word == string("MAXXY")) {
      keyword = "MAXXY";
      ss <<" " <<word;
    } else if (word == string("LAYER")) {
      keyword = "LAYER";
      ss <<" " <<word;
    } else if (word == string("ADJACENTCUTS")) {
      keyword = "ADJACENTCUTS";
      ss <<" " <<word;
    } else if (word == string("PARALLELOVERLAP")) {
      keyword = "PARALLELOVERLAP";
      ss <<" " <<word;
    } else if (word == string("PARALLELWITHIN")) {
      keyword = "PARALLELWITHIN";
      ss <<" " <<word;
    } else if (word == string("SAMEMETALSHAREDEDGE")) {
      keyword = "SAMEMETALSHAREDEDGE";
      ss <<" " <<word;
    } else if (word == string("AREA")) {
      keyword = "AREA";
      ss <<" " <<word;
    } else {
      ss <<" " <<word;
    }
  }

  if (keyword == "LAYER") {
    getLef58CutSpacing_layer(data, tmpLayer, ss.str());
  } else if (keyword == "ADJACENTCUTS") {
    getLef58CutSpacing_adjacentCuts(data, tmpLayer, ss.str());
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: unsupported LEF58_SPACING branch" << keyword << ", skipped...\n"; //skip unsupported rules
    cout << __FILE__ << ":" << __LINE__ << ": " << sIn << endl;
  }
  
  return 0;

}

int io::Parser::getLef58CutSpacing(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_SPACING \"";
  }
  istringstream istr(sIn);
  string word;
  stringstream ss;
  while (istr >> word) {
    if (word == string("SPACING")) {
      ss.str("");
      ss <<word;
    } else if (word == ";") {
      ss <<" " <<word;
      getLef58CutSpacing_helper(data, tmpLayer, ss.str());
    } else {
      ss <<" " <<word;
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  }
  return 0;
}

int io::Parser::getLef58CutSpacing_layer(void *data, frLayer* tmpLayer, const string &sIn) {
  // bool enableOutput = true;
  bool enableOutput = false;
  bool isSkip = false;

  frCoord cutSpacing = -1;
  bool isCenterToCenter = false;
  bool isSameNet = false;
  bool isSameMetal = false;
  bool isSameVia = false;

  string secondLayerName;
  bool isStack = false;
  frCoord orthogonalSpacing = -1;
  string className;
  bool isShortEdgeOnly = false;
  frCoord prl = -1;
  bool isConcaveCorner = false;;
  frCoord width = -1;
  frCoord enclosure = -1;
  frCoord edgeLength = -1;
  frCoord parLength = -1;
  frCoord parWithin = -1;
  frCoord edgeEnclosure = -1;
  frCoord adjEnclosure = -1;
  frCoord extension = -1;
  frCoord eolWidth = -1;
  frCoord minLength = -1;
  bool isMaskOverlap = false;
  bool isWrongDirection = false;

  istringstream istr(sIn);
  string word;

  string keyword = "";
  io::Parser* parser = (io::Parser*) data;

  auto con = make_unique<frLef58CutSpacingConstraint>();

  while (istr >> word) {
    if (word == string("SPACING")) {
      double tmp;
      if (istr >> tmp) {
        cutSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        con->setCutSpacing(cutSpacing);
      } else {
        isSkip = true;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  SPACING " <<tmp;
      }
    } else if (word == string("CENTERTOCENTER")) {
      isCenterToCenter = true;
      con->setCenterToCenter(isCenterToCenter);
    } else if (word == string("SAMENET")) {
      isSameNet = true;
      con->setSameNet(isSameNet);
    } else if (word == string("SAMEMETAL")) {
      isSameMetal = true;
      con->setSameMetal(isSameMetal);
    } else if (word == string("SAMEVIA")) {
      isSameVia = true;
      con->setSameVia(isSameVia);
    } else if (word == string("LAYER")) {
      if (istr >> secondLayerName) {
        con->setSecondLayerName(secondLayerName);
      } else {
        isSkip = true;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" LAYER " <<secondLayerName;
      }
      if (istr >> word) {
        if (word == string("STACK")) {
          isStack = true;
          con->setStack(isStack);
        } else if (word == string("ORTHOGONALSPACING")) {
          double tmp;
          if (istr >> tmp) {
            orthogonalSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
            con->setOrthogonalSpacing(orthogonalSpacing);
          } else {
            isSkip = true;
          }
        } else if (word == string("CUTCLASS")) {
          if (istr >> className) {
            con->setCutClassName(className);
            auto cutClassIdx = tmpLayer->getCutClassIdx(className);
            if (cutClassIdx != -1) {
              // cout << __FILE__ << ":" << __LINE__ << ": " << "cutClassIdx = " << cutClassIdx << endl;
              con->setCutClassIdx(cutClassIdx);
            } else {
              isSkip = true;
            }
          } else {
            isSkip = true;
          }
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" CUTCLASS " <<className;
          }
          // cut class branch
          if (istr >> word) {
            if (word == string("SHORTEDGEONLY")) {
              isShortEdgeOnly = true;
              con->setShortEdgeOnly(isShortEdgeOnly);
              if (istr >> word) {
                if (word == string("PRL")) {
                  double tmp;
                  if (istr >> tmp) {
                    prl = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                    con->setPrl(prl);
                  } else {
                    isSkip = true;
                  }
                }
              }
            } else if (word == string("CONCAVECORNER")) {
              isConcaveCorner = true;
              con->setConcaveCorner(isConcaveCorner);
              if (enableOutput) {
                cout << __FILE__ << ":" << __LINE__ << ": " << " CONCAVECORNER";
              }
              if (istr >> word) {
                if (word == string("WIDTH")) {
                  double tmp;
                  if (istr >> tmp) {
                    width = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                    con->setWidth(width);
                    if (istr >> word) {
                      if (word == string("ENCLOSURE")) {
                        if (istr >> tmp) {
                          enclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                          con->setEnclosure(enclosure);
                          if (istr >> word) {
                            if (word == string("EDGELENGTH")) {
                              if (istr >> tmp) {
                                edgeLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                                con->setEdgeLength(edgeLength);
                              } else {
                                isSkip = true;
                              }
                            } else {
                              isSkip = true;
                            }
                          } else {
                            isSkip = true;
                          }
                        } else {
                          isSkip = true;
                        }
                      } else {
                        isSkip = true;
                      }
                    } else {
                      isSkip = true;
                    }
                  } else {
                    isSkip = true;
                  }
                } else if (word == string("PARALLEL")) {
                  double tmp;
                  if (istr >> tmp) {
                    parLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                    con->setParLength(parLength);
                    if (istr >> word) {
                      if (word == string("WITHIN")) {
                        if (istr >> tmp) {
                          parWithin = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                          con->setParWithin(parWithin);
                          if (istr >> word) {
                            if (word == string("ENCLOSURE")) {
                              if (istr >> tmp) {
                                enclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                                con->setEnclosure(enclosure);
                              } else {
                                isSkip = true;
                              }
                            } else {
                              isSkip = true;
                            }
                          } else {
                            isSkip = true;
                          }
                        } else {
                          isSkip = true;
                        }
                      } else {
                        isSkip = true;
                      }
                    } else {
                      isSkip = true;
                    }
                  } else {
                    isSkip = true;
                  }
                } else if (word == string("EDGELENGTH")) {
                  double tmp;
                  if (istr >> tmp) {
                    edgeLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                    con->setEdgeLength(edgeLength);
                    if (istr >> word) {
                      if (word == string("ENCLOSURE")) {
                        if (istr >> tmp) {
                          edgeEnclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                          con->setEdgeEnclosure(edgeEnclosure);
                          if (istr >> tmp) {
                            adjEnclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                            con->setAdjEnclosure(adjEnclosure);
                          } else {
                            isSkip = true;
                          }
                        } else {
                          isSkip = true;
                        }
                      } else {
                        isSkip = true;
                      }
                    } else {
                      isSkip = true;
                    }
                  } else {
                    isSkip = true;
                  }
                }
              } else {
                isSkip = true;
              }
            } else if (word == string("EXTENSION")) {
              double tmp;
              if (istr >> tmp) {
                extension = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                con->setExtension(extension);
              } else {
                isSkip = true;
              }
            } else if (word == string("NONEOLCONVEXCORNER")) {
              double tmp;
              if (istr >> tmp) {
                if (enableOutput) {
                  cout << __FILE__ << ":" << __LINE__ << ": " << " NONEOLCONVEXCORNER " << tmp;
                }
                eolWidth = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                con->setEolWidth(eolWidth);
                if (istr >> word) {
                  if (word == string("MINLENGTH")) {
                    if (istr >> tmp) {
                      if (enableOutput) {
                        cout << __FILE__ << ":" << __LINE__ << ": " << " MINLENGTH " << tmp;
                      }
                      minLength = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                      con->setMinLength(minLength);
                    } else {
                      isSkip = true;
                    }
                  } else {
                    ;
                  }
                }
              }
            } else if (word == string("ABOVEWIDTH")) {
              double tmp;
              if (istr >> tmp) {
                width = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                con->setWidth(width);
                if (istr >> word) {
                  if (word == string("ENCLOSURE")) {
                    if (istr >> tmp) {
                      enclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                      con->setEnclosure(enclosure);
                    } else {
                      isSkip = true;
                    }
                  } else {
                    isSkip = true;
                  }
                }
              } else {
                isSkip = true;
              }
            } else if (word == string("MASKOVERLAP")) {
              isMaskOverlap = true;
              con->setMaskOverlap(isMaskOverlap);
            } else if (word == string("WRONGDIRECTION")) {
              isWrongDirection = true;
              con->setWrongDirection(isWrongDirection);
            } else {
              isSkip = true;
            }
          }
        }
      }
    } 
  }

  if (isSkip) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Error: getLef58CutSpacing_layer" << endl;
    cout << __FILE__ << ":" << __LINE__ << ": " << sIn << endl;
  } else {
    // cout << __FILE__ << ":" << __LINE__ << ": " << "addLef58CutSpacingConstraint\n";
    tmpLayer->addLef58CutSpacingConstraint(con.get());
    parser->tech->addUConstraint(std::move(con));

  }
  return 0;
}

// lefdef ref spacing
int io::Parser::getLef58CutSpacing_adjacentCuts(void *data, frLayer* tmpLayer, const string &sIn) {
  bool enableOutput = false;
  // bool enableOutput = true;

  bool isSkip = false;

  frCoord  cutSpacing = -1;
  bool isCenterToCenter = false;
  bool isSameNet = false;
  bool isSameMetal = false;
  bool isSameVia = false;
  int numAdjCuts = -1;
  int exactAlignedCut = -1;
  // two cuts
  int twoCuts = -1;
  frCoord twoCutsSpacing = -1;
  bool isSameCut = false;
  // within
  frCoord cutWithin2 = -1;

  bool isExceptSamePGNet;
  frCoord exceptAllWithin;
  bool isAbove = false;
  bool isBelow = false;
  frCoord enclosure = -1;
  // cutclass
  string cutClassName = "";
  bool isToAll = false;
  bool isNoPrl = false;
  bool isSideParallelOverlap = false;
  bool isSameMask = false;

  istringstream istr(sIn);
  string word;

  string keyword = "";
  io::Parser* parser = (io::Parser*) data;

  auto con = make_unique<frLef58CutSpacingConstraint>();

  while (istr >> word) {
    if (word == string("SPACING")) {
      double tmp;
      if (istr >> tmp) {
        cutSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
        con->setCutSpacing(cutSpacing);
      } else {
        isSkip = true;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  SPACING " <<tmp;
      }
    } else if (word == string("CENTERTOCENTER")) {
      isCenterToCenter = true;
      con->setCenterToCenter(isCenterToCenter);
    } else if (word == string("SAMENET")) {
      isSameNet = true;
      con->setSameNet(isSameNet);
    } else if (word == string("SAMEMETAL")) {
      isSameMetal = true;
      con->setSameMetal(isSameMetal);
    } else if (word == string("SAMEVIA")) {
      isSameVia = true;
      con->setSameVia(isSameVia);
    } else if (word == string("ADJACENTCUTS")) {
      if (istr >> numAdjCuts) {
        con->setAdjacentCuts(numAdjCuts);
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " << " ADJACENTCUTS " << numAdjCuts;
        }
      } else {
        isSkip = true;
      }
      while (istr >> word) {
        if (word == string("EXACTALIGNED")) {
          if (istr >> exactAlignedCut) {
            con->setExactAlignedCut(exactAlignedCut);
          } else {
            isSkip = true;
          }
        } else if (word == string("TWOCUTS")) {
          if (istr >> twoCuts) {
            con->setTwoCuts(twoCuts);
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " << " TWOCUTS " << twoCuts;
            }
            streampos pos = istr.tellg();
            bool done = false;
            while (!done) {
              if (istr >> word) {
                if (word == string("TWOCUTSSPACING")) {
                  double tmp;
                  if (istr >> tmp) {
                    twoCutsSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
                    con->setTwoCutsSpacing(twoCutsSpacing);
                    pos = istr.tellg();
                  } else {
                    isSkip = true;
                  }
                } else if (word == string("SAMECUT")) {
                  isSameCut = true;
                  con->setSameCut(isSameCut);
                  pos = istr.tellg();
                } else {
                  done = true;
                }
              }
            }
            istr.seekg(pos);
          } else {
            isSkip = true;
          }
        } else if (word == string("WITHIN")) {
          streampos pos = istr.tellg();
          double tmp1;
          if (istr >> tmp1) {
            pos = istr.tellg();
          } else {
            isSkip = true;
          }
          // TODO: bring this back after figure out how to correctly use seekg / tellg
          // if (istr >> tmp2) {
          //   cutWithin1 = frCoord(round(tmp1 * parser->tech->getDBUPerUU()));
          //   cutWithin2 = frCoord(round(tmp2 * parser->tech->getDBUPerUU()));
          //   con->setCutWithin1(cutWithin1);
          //   con->setCutWithin2(cutWithin2);
          //   pos = istr.tellg();
          // } else {
            cutWithin2 = frCoord(round(tmp1 * parser->tech->getDBUPerUU()));
            con->setCutWithin(cutWithin2);
            // istr.seekg(pos);
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " << " WITHIN " << tmp1;
            }
          // }
        } else if (word == string("EXCEPTSAMEPGNET")) {
          isExceptSamePGNet = true;
          con->setExceptSamePGNet(isExceptSamePGNet);
        } else if (word == string("EXCEPTALLWITHIN")) {
          double tmp;
          if (istr >> tmp) {
            exceptAllWithin = frCoord(round(tmp * parser->tech->getDBUPerUU()));
            con->setExceptAllWithin(exceptAllWithin);
          } else {
            isSkip = true;
          }
        } else if (word == string("ENCLOSURE")) {
          streampos pos = istr.tellg();
          if (istr >> word) {
            if (word == string("ABOVE")) {
              isAbove = true;
              con->setAbove(isAbove);
            } else if (word == string("BELOW")) {
              isBelow = true;
              con->setBelow(isBelow);
            } else {
              istr.seekg(pos);
            }
          }
          double tmp;
          if (istr >> tmp) {
            enclosure = frCoord(round(tmp * parser->tech->getDBUPerUU()));
            con->setEnclosure(enclosure);
          } else {
            isSkip = true;
          }
        } else if (word == string("CUTCLASS")) {
          if (istr >> cutClassName) {
            con->setCutClassName(cutClassName);
            auto cutClassIdx = tmpLayer->getCutClassIdx(cutClassName);
            if (cutClassIdx != -1) {
              // cout << __FILE__ << ":" << __LINE__ << ": " << "cutClassIdx = " << cutClassIdx << endl;
              con->setCutClassIdx(cutClassIdx);
            } else {
              isSkip = true;
            }
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " << " CUTCLASS " << cutClassName;
            }
          } else {
            isSkip = true;
          }
          streampos pos = istr.tellg();
          string word1, word2;
          if (istr >> word1 && istr >> word2) {
            if (word1 == string("TO") && word2 == string("ALL")) {
              isToAll = true;
              con->setToAll(isToAll);
            } else {
              istr.seekg(pos);
            }
          }
        } else if (word == string("NOPRL")) {
          isNoPrl = true;
          con->setNoPrl(isNoPrl);
        } else if (word == string("SIDEPARALLELOVERLAP")) {
          isSideParallelOverlap = true;
          con->setSideParallelOverlap(isSideParallelOverlap);
        } else if (word == string("SAMEMASK")) {
          isSameMask = true;
          con->setSameMask(isSameMask);
        } else {
          isSkip = true;
        }
      }
    }
  }

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << endl;
  }

  if (isSkip) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Error: getLef58CutSpacing_adjacentCuts" << endl;
  } else {
    // cout << __FILE__ << ":" << __LINE__ << ": " << "addLef58CutSpacingConstraint\n";
    tmpLayer->addLef58CutSpacingConstraint(con.get());
    parser->tech->addUConstraint(std::move(con));
  }
  return 0;
}

int io::Parser::getLef58CutSpacingTable(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_SPACINGTABLE \"";
  }
  istringstream istr(sIn);
  string word;
  stringstream ss;
  while (istr >> word) {
    if (word == string("SPACINGTABLE")) {
      ss.str("");
      ss <<word;
    } else if (word == ";") {
      ss <<" " <<word;
      getLef58CutSpacingTable_helper(data, tmpLayer, ss.str());
    } else {
      ss <<" " <<word;
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  }
  return 0;
}

int io::Parser::getLef58CutSpacingTable_helper(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  //bool enableOutput = false;
  //if (enableOutput) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  PROPERTY LEF58_SPACINGTABLE \"" <<endl;
  //}

  string keyword = "";

  istringstream istr(sIn);
  string word;
  stringstream ss;
  while (istr >> word) {
    if (word == string("SPACINGTABLE")) {
      keyword = "";
      ss.str("");
      ss <<word;
      //if (enableOutput) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACINGTABLE";
      //}
    } else if (word == string("CENTERSPACING")) {
      //if (enableOutput) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<" CENTERSPACING";
      //}
      keyword = "CENTERSPACING";
      ss <<" " <<word;
    } else if (word == string("ORTHOGONAL")) {
      //if (enableOutput) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<" CENTERSPACING";
      //}
      keyword = "ORTHOGONAL";
      ss <<" " <<word;
    } else {
      ss <<" " <<word;
    }
  }

  if (keyword == "CENTERSPACING") {
    ; //skip center spacing rules
  } else if (keyword == "ORTHOGONAL") {
    ; //skip orthogonal rules
  } else {
    getLef58CutSpacingTable_others(data, tmpLayer, ss.str());
  }
  
  //if (enableOutput) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"\" ;" <<endl;
  //}

  return 0;

}

int io::Parser::getLef58CutSpacingTable_default(void *data, frLayer* tmpLayer, const string &sIn, 
  const shared_ptr<frLef58CutSpacingTableConstraint> &con) {
  //cout << __FILE__ << ":" << __LINE__ << ": " <<sIn <<endl;
  //bool enableOutput = true;
  bool enableOutput = false;
  
  frCoord defaultCutSpacing = 0;

  istringstream istr(sIn);
  string word;
  io::Parser* parser = (io::Parser*) data;

  while (istr >> word) {
    if (word == string("DEFAULT")) {
      double tmp;
      if (istr >> tmp) {
        defaultCutSpacing = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutSpacingTable_default" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  DEFAULT " <<tmp;
      }
    } else {
      ; // skip unknown rules
    }
  }

  con->setDefaultCutSpacing(defaultCutSpacing);
  return 0;
}

int io::Parser::getLef58CutSpacingTable_prl(void *data, frLayer* tmpLayer, const string &sIn, 
    const shared_ptr<frLef58CutSpacingTableConstraint> &con) {
  //cout << __FILE__ << ":" << __LINE__ << ": " <<sIn <<endl;
  //bool enableOutput = true;
  bool enableOutput = false;
  
  frCoord prl          = 0;
  bool    isHorizontal = false;
  bool    isVertical   = false;
  bool    isMaxXY      = false;

  istringstream istr(sIn);
  string word;
  io::Parser* parser = (io::Parser*) data;

  while (istr >> word) {
    if (word == string("PRL")) {
      double tmp;
      if (istr >> tmp) {
        prl = frCoord(round(tmp * parser->tech->getDBUPerUU()));
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutSpacingTable_prl" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" PRL " <<tmp;
      }
    } else if (word == string("HORIZONTAL")) {
      isHorizontal = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" HORIZONTAL";
      }
    } else if (word == string("VERTICAL")) {
      isVertical = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" VERTICAL";
      }
    } else if (word == string("MAXXY")) {
      isMaxXY = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" MAXXY";
      }
    } else {
      ; // skip unknown rules
    }
  }

  auto ptr = make_shared<frLef58CutSpacingTablePrlConstraint>();
  ptr->setPrl(prl);
  ptr->setHorizontal(isHorizontal);
  ptr->setVertical(isVertical);
  ptr->setMaxXY(isMaxXY);
  con->setPrlConstraint(ptr);

  return 0;
}

int io::Parser::getLef58CutSpacingTable_layer(void *data, frLayer* tmpLayer, const string &sIn, 
    const shared_ptr<frLef58CutSpacingTableConstraint> &con, frLayerNum &secondLayerNum) {

  //bool enableOutput = true;
  bool enableOutput = false;

  frString secondLayerName    = "";
  bool     isNonZeroEnclosure = false;

  istringstream istr(sIn);
  string word;

  while (istr >> word) {
    if (word == string("LAYER")) {
      if (istr >> secondLayerName) {
        ;
      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutSpacingTable_layer" <<endl;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" LAYER " <<secondLayerName;
      }
    } else if (word == string("NONZEROENCLOSURE")) {
      isNonZeroEnclosure = true;
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  NONZEROENCLOSURE";
      }
    } else {
      ; // skip unknown rules
    }
  }

  io::Parser* parser = (io::Parser*) data;
  auto ptr = make_shared<frLef58CutSpacingTableLayerConstraint>();
  //cout << __FILE__ << ":" << __LINE__ << ": " <<secondLayerName <<endl <<flush;
  secondLayerNum = parser->tech->name2layer.at(secondLayerName)->getLayerNum();
  ptr->setSecondLayerNum(secondLayerNum);
  ptr->setNonZeroEnc(isNonZeroEnclosure);
  con->setLayerConstraint(ptr);

  return 0;
}


int io::Parser::getLef58CutSpacingTable_cutClass(void *data, frLayer* tmpLayer, const string &sIn, 
    const shared_ptr<frLef58CutSpacingTableConstraint> &con, bool hasSecondLayer, frLayerNum secondLayerNum) {
  //bool enableOutput = true;
  bool enableOutput = false;

  auto defaultCutSpacing = con->getDefaultCutSpacing();
  // 2d spacing table
  frCollection<frCollection<pair<frCoord, frCoord> > > tblVals;

  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<sIn <<endl;
  // check numRows and numCols
  istringstream istr1(sIn);
  string word;
  int numCols = 0;
  int numRows = 0;
  bool isPrevNum   = false;
  while (istr1 >> word) {
    // "-" is treated as number 0
    if (word == "-") {
      word = "0";
    }
    stringstream tmpss(word);
    double tmpd;
    // is a number
    if (tmpss >> tmpd) {
      isPrevNum = true;
      numCols++;
    // is a string
    } else {
      if (word == ";") {
        numRows++;
      } else if (isPrevNum) {
        numRows++;
        numCols = 0;
      }
      isPrevNum = false;
    }
  }

  numCols /= 2;
  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"#rows/cols = " <<numRows <<"/" <<numCols <<endl;
  
  vector<frString> colNames;
  vector<int>      dupColNames; //duplicate side and all
  
  vector<frString> rowNames;
  vector<int>      dupRowNames; //duplicate side and all
  
  vector<vector<pair<frCoord, frCoord> > > tmpTbl;
  vector<pair<frCoord, frCoord> > tmpTblRow;
  
  io::Parser* parser = (io::Parser*) data;
  istringstream istr2(sIn);
  word = "";
  int stage = 0; // 0 = read columns; 1 = read rows
  int readNum = 0; // numbers read in a row
  while (istr2 >> word) {
    // "-" is treated as number 0
    //if (word == ";") {
    //  cout << __FILE__ << ":" << __LINE__ << ": " <<"found ;" <<endl;
    //}
    if (word == "-") {
      word = to_string(defaultCutSpacing * 1.0 / parser->tech->getDBUPerUU());
    }
    if (word == "CUTCLASS") {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  CUTCLASS";
      }
    // read numCols times
    } else if ((int)colNames.size() < numCols) {
      if (word == "SIDE" || word == "END") {
        *(--colNames.end()) = *(--colNames.end()) + word;
        *(--dupColNames.end()) = 1;
      } else {
        colNames.push_back(word);
        dupColNames.push_back(2);
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
      }
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"testX" <<endl;
    } else if (stage == 0 && (int)colNames.size() == numCols) {
      // last word of column
      if (word == "SIDE" || word == "END") {
        *(--colNames.end()) = *(--colNames.end()) + word;
        *(--dupColNames.end()) = 1;
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
      // first word of row
      } else {
        rowNames.push_back(word);
        dupRowNames.push_back(2);
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  " <<word;
        }
      }
      stage = 1;
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"testXX" <<endl;
    } else if (word == ";") {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;";
      }
      tmpTbl.push_back(tmpTblRow);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"testXXX" <<endl;
    } else if (stage == 1) {
      if (word == "SIDE" || word == "END") {
        *(--rowNames.end()) = *(--rowNames.end()) + word;
        *(--dupRowNames.end()) = 1;
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
        }
      } else {
        stringstream ss(word);
        double firstNum;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"test: " <<word <<endl;
        // number
        if (ss >> firstNum) {
          frCoord val1 = frCoord(round(firstNum * parser->tech->getDBUPerUU()));
          string tmpS;
          if (istr2 >> tmpS) {
            ;
          } else {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLef58CutSpacingTable_cutClass" <<endl;
          }
          stringstream tmpSS(tmpS);
          double secondNum;
          if (tmpSS >> secondNum) {
            frCoord val2 = frCoord(round(secondNum * parser->tech->getDBUPerUU()));
            tmpTblRow.push_back(make_pair(val1, val2));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<firstNum <<" " <<secondNum;
            }
          } else {
            // the number is "-", use default spacing
            frCoord val2 = defaultCutSpacing;
            tmpTblRow.push_back(make_pair(val1, val2));
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<firstNum <<" " <<defaultCutSpacing * 1.0 / parser->tech->getDBUPerUU();
            }
          }
          readNum += 1;
        // first word
        } else {
          rowNames.push_back(word);
          dupRowNames.push_back(2);
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  " <<word;
          }
          if (readNum) {
            tmpTbl.push_back(tmpTblRow);
            tmpTblRow.clear();
          }
          readNum = 0;
        }
      }
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"testXXXX" <<endl;
    }
  }

  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"column:";
  //for (auto &str: colNames) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<str;
  //}
  //
  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"row:";
  //for (auto &str: rowNames) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<str;
  //}

  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"table: ";
  //for (auto &v1: tmpTbl) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"test here";
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    ";
  //  for (auto &v2: v1) {
  //    cout << __FILE__ << ":" << __LINE__ << ": " <<"test here";
  //    cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<v2.first <<" " <<v2.second;
  //  }
  //}
  //cout << __FILE__ << ":" << __LINE__ << ": " <<flush;

  vector<frString> expColNames;
  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"new expand column:";
  for (int i = 0; i < (int)colNames.size(); i++) {
    if (dupColNames.at(i) == 2) {
      string name1 = colNames.at(i) + "SIDE";
      string name2 = colNames.at(i) + "END";
      expColNames.push_back(name1);
      expColNames.push_back(name2);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<name1 <<" " <<name2;
    } else {
      string name = colNames.at(i);
      expColNames.push_back(name);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<name;
    };
  }
  
  vector<frString> expRowNames;
  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"new expand rows:";
  for (int i = 0; i < (int)rowNames.size(); i++) {
    if (dupRowNames.at(i) == 2) {
      string name1 = rowNames.at(i) + "SIDE";
      string name2 = rowNames.at(i) + "END";
      expRowNames.push_back(name1);
      expRowNames.push_back(name2);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<name1 <<" " <<name2;
    } else {
      string name = rowNames.at(i);
      expRowNames.push_back(name);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<name;
    };
  }

  vector<vector<pair<frCoord, frCoord> > > expTmpTbl;
  for (int i = 0; i < (int)rowNames.size(); i++) {
    vector<pair<frCoord, frCoord> > expTmpTblRow;
    for (int j = 0; j < (int)colNames.size(); j++) {
      expTmpTblRow.push_back(tmpTbl.at(i).at(j));
      if (dupColNames.at(j) == 2) {
        expTmpTblRow.push_back(tmpTbl.at(i).at(j));
      }
    }
    expTmpTbl.push_back(expTmpTblRow);
    if (dupRowNames.at(i) == 2) {
      expTmpTbl.push_back(expTmpTblRow);
    }
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<"new expand table: ";
  //for (auto &v1: expTmpTbl) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    ";
  //  for (auto &v2: v1) {
  //    cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<v2.first <<" " <<v2.second;
  //  }
  //}

  vector<pair<frString, int> > expColNames_helper;
  for (int i = 0; i < (int)expColNames.size(); i++) {
    expColNames_helper.push_back(make_pair(expColNames.at(i), i));
  }
  sort(expColNames_helper.begin(), expColNames_helper.end(), 
       [](const pair<frString, int> &a, const pair<frString, int> &b) 
       {return a.first < b.first;});
  sort(expColNames.begin(), expColNames.end());

  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"sorted expand column:";
  //for (auto &it: expColNames_helper) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<it.first;
  //}

  vector<pair<frString, int> > expRowNames_helper;
  for (int i = 0; i < (int)expRowNames.size(); i++) {
    expRowNames_helper.push_back(make_pair(expRowNames.at(i), i));
  }
  sort(expRowNames_helper.begin(), expRowNames_helper.end(), 
       [](const pair<frString, int> &a, const pair<frString, int> &b) 
       {return a.first < b.first;});
  sort(expRowNames.begin(), expRowNames.end());

  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"sorted expand row:";
  //for (auto &it: expRowNames_helper) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<it.first;
  //}

  tblVals = expTmpTbl;
  //cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"sorted tbl:";
  for (int i = 0; i < (int)expRowNames_helper.size(); i++) {
    //cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
    for (int j = 0; j < (int)expColNames_helper.size(); j++) {
      int orig_i = expRowNames_helper.at(i).second;
      int orig_j = expColNames_helper.at(j).second;
      tblVals.at(i).at(j) = expTmpTbl.at(orig_i).at(orig_j);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tblVals.at(i).at(j).first <<" " <<tblVals.at(i).at(j).second;
    }
  }

  string rowName("CUTCLASS");
  string colName("CUTCLASS");
  auto ptr = make_shared<fr2DLookupTbl<frString, frString, pair<frCoord, frCoord> > >(rowName, 
      expRowNames, colName, expColNames, tblVals);
  con->setCutClassTbl(ptr);

  return 0;
}

// lefdef ref 2nd spacing table, no orthogonal case
int io::Parser::getLef58CutSpacingTable_others(void *data, frLayer* tmpLayer, const string &sIn) {
  //bool enableOutput = true;
  bool enableOutput = false;

  istringstream istr(sIn);
  string word;

  stringstream ssDefault;
  stringstream ssLayer;    
  stringstream ssPrl;
  stringstream ssCutClass; 
  
  string keyword = "";
  while (istr >> word) {
    if (word == string("SPACINGTABLE")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  SPACINGTABLE";
      }
    } else if (word == string("DEFAULT")) {
      keyword = "DEFAULT";
      ssDefault <<word;
    } else if (word == string("SAMEMASK")) {
      keyword = "SAMEMASK";
    } else if (word == string("SAMENET") || word == string("SAMEMETAL") || word == string("SAMEVIA")) {
      keyword = "SAMENETMETALVIA";
    } else if (word == string("LAYER")) {
      keyword = "LAYER";
      ssLayer <<word;
    } else if (word == string("CENTERTOCENTER")) {
      keyword = "CENTERTOCENTER";
    } else if (word == string("CENTERANDEDGE")) {
      keyword = "CENTERANDEDGE";
    } else if (word == string("PRL")) {
      keyword = "PRL";
      ssPrl <<"PRL";
    } else if (word == string("PRLTWOSIDES")) {
      keyword = "PRLTWOSIDES";
    } else if (word == string("ENDEXTENSION")) {
      keyword = "ENDEXTENSION";
    } else if (word == string("EXACTALIGNEDSPACING")) {
      keyword = "EXACTALIGNEDSPACING";
    } else if (word == string("NONOPPOSITEENCLOSURESPACING")) {
      keyword = "NONOPPOSITEENCLOSURESPACING";
    } else if (word == string("OPPOSITEENCLOSURERESIZESPACING")) {
      keyword = "OPPOSITEENCLOSURERESIZESPACING";
    } else if (word == string("CUTCLASS")) {
      keyword = "CUTCLASS";
      ssCutClass <<word;
    } else {
      if (keyword == "DEFAULT") {
        ssDefault <<" " <<word;
      } else if (keyword == "CUTCLASS") {
        ssCutClass <<" " <<word;
      } else if (keyword == "PRL") {
        ssPrl <<" " <<word;
      } else if (keyword == "LAYER") {
        ssLayer <<" " <<word;
      } else {
        ;
      }
    }
  }

  auto con = make_shared<frLef58CutSpacingTableConstraint>();

  bool hasSecondLayer       = false;
  frLayerNum secondLayerNum = 0;

  bool isFirstViaLayerHavingSecondLayerNum = false;

  if (ssDefault.str() != "") {
    getLef58CutSpacingTable_default(data, tmpLayer, ssDefault.str(), con);
  }
  if (ssPrl.str() != "") {
    getLef58CutSpacingTable_prl(data, tmpLayer, ssPrl.str(), con);
  }
  if (ssLayer.str() != "") {
    if (tmpLayer->getLayerNum() == 1) {
      isFirstViaLayerHavingSecondLayerNum = true;
    } else {
      getLef58CutSpacingTable_layer(data, tmpLayer, ssLayer.str(), con, secondLayerNum);
    }
  }
  if (ssCutClass.str() != "" && !isFirstViaLayerHavingSecondLayerNum) {
    getLef58CutSpacingTable_cutClass(data, tmpLayer, ssCutClass.str(), con, hasSecondLayer, secondLayerNum);
  }

  if (isFirstViaLayerHavingSecondLayerNum) {
    ;
  } else {
    tmpLayer->lef58CutSpacingTableConstraints.push_back(con);
    io::Parser* parser = (io::Parser*) data;
    parser->tech->addConstraint(con);
  }

  return 0;
}

int io::Parser::getLef58RightWayOnGridOnly(void *data, frLayer* tmpLayer, const string &sIn) {
  // bool enableOutput = true;
  bool enableOutput = false;
  bool checkMask = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << endl << "  PROPERTY LEF58_RIGHTWAYONGRIDONLY \"";
  }
  istringstream istr(sIn);
  string word;
  while (istr >> word) {
    if (word == string("RIGHTWAYONGRIDONLY")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "RIGHTWAYONGRIDONLY ";
      }
    } else if (word == string("CHECKMASK")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "CHECKMASK ";
      }
      checkMask = true;
    } else if (word == string(";")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << " ;";
      }
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "\" ;" << endl;
  }
  auto rightWayOnGridOnlyConstraint = make_unique<frLef58RightWayOnGridOnlyConstraint>(checkMask);
  tmpLayer->setLef58RightWayOnGridOnlyConstraint(rightWayOnGridOnlyConstraint.get());
  io::Parser* parser = (io::Parser*) data;
  parser->tech->addUConstraint(std::move(rightWayOnGridOnlyConstraint));
  return 0;
}

int io::Parser::getLef58RectOnly(void *data, frLayer* tmpLayer, const string &sIn) {
  // bool enableOutput = true;
  bool enableOutput = false;
  bool exceptNonCorePins = false;
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << endl << "  PROPERTY LEF58_RECTONLY \"";
  }
  istringstream istr(sIn);
  string word;
  while (istr >> word) {
    if (word == string("RECTONLY")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "RECTONLY ";
      }
    } else if (word == string("EXCEPTNONCOREPINS")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "EXCEPTNONCOREPINS ";
      }
      exceptNonCorePins = true;
    } else if (word == string(";")) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << " ;";
      }
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " << "\" ;" << endl;
  }
  auto rectOnlyConstraint = make_unique<frLef58RectOnlyConstraint>(exceptNonCorePins);
  tmpLayer->setLef58RectOnlyConstraint(rectOnlyConstraint.get());
  io::Parser* parser = (io::Parser*) data;
  parser->tech->addUConstraint(std::move(rectOnlyConstraint));
  return 0;
}

int io::Parser::getLef58CornerSpacing(void *data, frLayer *tmpLayer, const string &stringIn) {
  istringstream istr(stringIn);
  string word;
  
  int numSpacingEntry = 0;

  bool hasConvexCorner    = false;
  bool hasSameMask        = false;
  bool hasCornerOnly      = false;
  frUInt4 within          = 0;
  bool hasExceptEol       = false;
  bool hasEdgeLength      = false;
  bool hasIncludeLShape   = false;
  bool hasExceptJogLength = false;
  frUInt4 eolWidth        = 0;
  frUInt4 length          = 0;
 
  bool hasConcaveCorner     = false;
  bool hasMinLength         = false;
  frUInt4 minLength         = 0;
  bool hasExceptNotch       = false;
  bool hasExceptNotchLength = false;
  frUInt4 notchLength       = -1;
 
  bool hasExceptSameNet   = false;
  bool hasExceptSameMetal = false;
 
  std::vector< std::vector<frUInt4> > widthSpacing;
  std::vector<frUInt4> tmpWidthSpacing;
  bool hasSameXY = true;
 
  bool doCornerSpacing = false;
  bool doConvexCorner  = false;
  bool doConcaveCorner = false;
  bool doWidthSpacing  = false;
  io::Parser* parser = (io::Parser*) data;
  
  // check whether has notchLength specified
  istringstream testIstr(stringIn);
  while (testIstr >> word) {
    if (word == "EXCEPTNOTCH") {
      std::string tempNotchLength;
      std::string::size_type sz;
      testIstr >> tempNotchLength;
      try {
        std::stod(tempNotchLength, &sz);
        hasExceptNotchLength = true;
      }
      catch (std::exception& e) {
        hasExceptNotchLength = false;
      }
      break;
    }
  }

  while (istr >> word) {
    //cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<word;
 
    if (word == ";") {
      doCornerSpacing = false;
      doConvexCorner  = false;
      doConcaveCorner = false;
      doWidthSpacing  = false;
      continue;
    }
 
    if (!doCornerSpacing && word == "CORNERSPACING") {
      doCornerSpacing = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<"CORNERSPACING";
      continue;
    }
    if (doCornerSpacing &&  word == "CONVEXCORNER") {
      doConvexCorner  = true;
      hasConvexCorner = true;
      doConcaveCorner = false;
      doWidthSpacing  = false;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  CONVEXCORNER";
      continue;
    }
    if (doCornerSpacing &&  word == "CONCAVECORNER") {
      doConvexCorner  = false;
      doConcaveCorner = true;
      hasConcaveCorner = true;
      doWidthSpacing  = false;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  CONCAVECORNER";
      continue;
    }
 
    if (doCornerSpacing &&  word == "EXCEPTSAMENET") {
      doConvexCorner  = false;
      doConcaveCorner = false;
      doWidthSpacing  = false;
      hasExceptSameNet   = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  EXCEPTSAMENET";
      continue;
    }
    
    if (doCornerSpacing &&  word == "EXCEPTSAMEMETAL") {
      doConvexCorner  = false;
      doConcaveCorner = false;
      doWidthSpacing  = false;
      hasExceptSameMetal = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  EXCEPTSAMEMETAL";
      continue;
    }
 
    if (doCornerSpacing &&  word == "WIDTH") {
      doConvexCorner  = false;
      doConcaveCorner = false;
      doWidthSpacing  = true;
      if (!tmpWidthSpacing.empty()) {
        widthSpacing.push_back(tmpWidthSpacing);
        if (numSpacingEntry == 0) {
          numSpacingEntry = std::min(int(tmpWidthSpacing.size()), 3);
        } else {
          if (numSpacingEntry != (int)tmpWidthSpacing.size()) {
            cout << __FILE__ << ":" << __LINE__ << ": " << "Error: LEF58_CORNERSPACING rule not well defined, please check..." << endl;
            return 1;
          }
        }
      }
      tmpWidthSpacing.clear();
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  WIDTH";
      continue;
    }
    
    if (doConvexCorner && word == "SAMEMASK") {
      hasSameMask = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " <<" SAMEMASK";
    }
    if (doConvexCorner && word == "CORNERONLY") {
      hasCornerOnly = true;
      double tmpWithin;
      istr >> tmpWithin;
      within = round(tmpWithin * parser->tech->getDBUPerUU());
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    CORNERONLY " <<within * 1.0 / parser->tech->getDBUPerUU();
    }

    if (doConvexCorner && word == "EXCEPTEOL") {
      hasExceptEol = true;
      double tmpEolWidth;
      istr >> tmpEolWidth;
      eolWidth = round(tmpEolWidth * parser->tech->getDBUPerUU());
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    EXCEPTEOL " <<eolWidth * 1.0 / parser->tech->getDBUPerUU();
    }

    if (doConcaveCorner && hasExceptEol && word == "EXCEPTJOGLENGTH") {
      hasExceptJogLength = true;
      double tmpLength;
      istr >> tmpLength;
      length = round(tmpLength * parser->tech->getDBUPerUU());
      // cout << __FILE__ << ":" << __LINE__ << ": " << " EXCEPTJOGLENGTH " << length * 1.0 / parser->tech->getDBUPerUU();
    }

    if (doConcaveCorner && hasExceptEol && hasExceptJogLength && word == "EDGELENGTH") {
      hasEdgeLength = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " << " EDGELENGTH" << endl;
    }

    if (doConcaveCorner && hasExceptEol && hasExceptJogLength && word == "INCLUDELSHAPE") {
      hasIncludeLShape = true;
      // cout << __FILE__ << ":" << __LINE__ << ": " << " INCLUDELSHAPE"  << endl;
    }
    
    if (doConcaveCorner && word == "MINLENGTH") {
      hasMinLength = true;
      double tmpMinLength;
      istr >> tmpMinLength;
      minLength = round(tmpMinLength * parser->tech->getDBUPerUU());
      // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    MINLENGTH " <<minLength * 1.0 / parser->tech->getDBUPerUU();
    }
    if (doConcaveCorner && word == "EXCEPTNOTCH") {
      hasExceptNotch = true;
      if (hasExceptNotchLength) {
        double tmpNotchLength;
        istr >> tmpNotchLength;
        notchLength = round(tmpNotchLength * parser->tech->getDBUPerUU());
        // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    EXCEPTNOTCH " <<notchLength * 1.0 / parser->tech->getDBUPerUU();
      } else {
        // cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    EXCEPTNOTCH ";
      }
    }
 
    if (doWidthSpacing &&  word == "SPACING") {
      // cout << __FILE__ << ":" << __LINE__ << ": " <<" SPACING";
      continue;
    }
 
    if (doWidthSpacing &&  word != "SPACING") {
      frUInt4 tmp = round(stod(word) * parser->tech->getDBUPerUU());
      tmpWidthSpacing.push_back(tmp);
      // cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tmp * 1.0 / parser->tech->getDBUPerUU();
      continue;
    }
 
  }
 
  if (!tmpWidthSpacing.empty()) {
    widthSpacing.push_back(tmpWidthSpacing);
  }

  if (numSpacingEntry == 3) {
    hasSameXY = false;
  }

  // cout << __FILE__ << ":" << __LINE__ << ": " << endl << " numSpacingEntry = " << numSpacingEntry << endl;

  // create 1D lookup table
  std::string widthName("WIDTH");
  std::vector<frCoord> widths;
  std::vector<std::pair<frCoord, frCoord> > spacings;
  for (int i = 0; i < (int)widthSpacing.size(); i++) {
    widths.push_back(widthSpacing[i][0]);
    if (numSpacingEntry == 3) {
      spacings.push_back(std::make_pair(widthSpacing[i][1], widthSpacing[i][2]));
    } else {
      spacings.push_back(std::make_pair(widthSpacing[i][1], widthSpacing[i][1]));
    }
  }
  fr1DLookupTbl<frCoord, std::pair<frCoord, frCoord> > cornerSpacingTbl(widthName, widths, spacings);
  // cout << __FILE__ << ":" << __LINE__ << ": " << std::min(cornerSpacingTbl.findMin().first, cornerSpacingTbl.findMin().second) 
  //      << " " << std::max(cornerSpacingTbl.findMax().first, cornerSpacingTbl.findMax().second) << "\n";

  unique_ptr<frConstraint> uCon = make_unique<frLef58CornerSpacingConstraint>(cornerSpacingTbl);
  auto rptr = static_cast<frLef58CornerSpacingConstraint*>(uCon.get());
  if (hasConvexCorner) {
    rptr->setCornerType(frCornerTypeEnum::CONVEX);
    rptr->setSameMask(hasSameMask);
    if (hasCornerOnly) {
      rptr->setWithin(within);
    }
    if (hasExceptEol) {
      rptr->setEolWidth(eolWidth);
      if (hasExceptJogLength) {
        rptr->setLength(length);
        rptr->setEdgeLength(hasEdgeLength);
        rptr->setIncludeLShape(hasIncludeLShape);
      }
    }
    rptr->setExceptSameNet(hasExceptSameNet);
    rptr->setExceptSameMetal(hasExceptSameMetal);
    rptr->setSameXY(hasSameXY);
  } else if (hasConcaveCorner) {
    rptr->setCornerType(frCornerTypeEnum::CONCAVE);
    if (hasMinLength) {
      rptr->setMinLength(minLength);
    }
    rptr->setExceptNotch(hasExceptNotch);
    if (hasExceptNotchLength) {
      rptr->setExceptNotchLength(notchLength);
    }
    rptr->setExceptSameNet(hasExceptSameNet);
    rptr->setExceptSameMetal(hasExceptSameMetal);
    rptr->setSameXY(hasSameXY);
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " << "Error: getLef58CornerSpacing rule is not well defined, please check...\n";
    return 1;
  }
  
  parser->tech->addUConstraint(std::move(uCon));
  tmpLayer->addLef58CornerSpacingConstraint(rptr);

  return 0;
}

int io::Parser::Callbacks::getLefLayers(lefrCallbackType_e type, lefiLayer* layer, lefiUserData data) {
  // bool enableOutput = true;
  bool enableOutput = false;
  //bool enableDoubleCheck = true;
  //bool enableDoubleCheck = false;
  std::string masterSliceLayerName("FR_MASTERSLICE"); // default masterslice
  std::string viaLayerName("FR_VIA"); // default via above masterslice
  if (type != lefrLayerCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrLayerCbkType!" <<endl;
    exit(1);
  }
  unique_ptr<frLayer> uLayer = make_unique<frLayer>();
  auto tmpLayer = uLayer.get();
  io::Parser* parser = (io::Parser*) data;

  if (!strcmp(layer->type(), "ROUTING")) {
    // add default masterslice and via layers if LEF does not have one
    if (parser->readLayerCnt == 0) {
      unique_ptr<frLayer> uMSLayer = make_unique<frLayer>();
      auto tmpMSLayer = uMSLayer.get();
      tmpMSLayer->setLayerNum(parser->readLayerCnt++);
      tmpMSLayer->setName(masterSliceLayerName);
      parser->tech->addLayer(std::move(uMSLayer));
      tmpMSLayer->setType(frLayerTypeEnum::MASTERSLICE);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"\n";
        cout << __FILE__ << ":" << __LINE__ << ": " <<"LAYER     " <<tmpMSLayer->getName() <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  TYPE      " <<"MASTERSLICE" <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerNum  " <<tmpMSLayer->getLayerNum() <<endl;
      }

      unique_ptr<frLayer> uCutLayer = make_unique<frLayer>();
      auto tmpCutLayer = uCutLayer.get();
      tmpCutLayer->setLayerNum(parser->readLayerCnt++);
      tmpCutLayer->setName(viaLayerName);
      parser->tech->addLayer(std::move(uCutLayer));
      tmpCutLayer->setType(frLayerTypeEnum::CUT);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"\n";
        cout << __FILE__ << ":" << __LINE__ << ": " <<"LAYER     " <<tmpCutLayer->getName() <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  TYPE      " <<"CUT" <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerNum  " <<tmpCutLayer->getLayerNum() <<endl;
      }
    }
    if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"\n";
      cout << __FILE__ << ":" << __LINE__ << ": " <<"LAYER "       <<layer->name() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  TYPE      " <<layer->type() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  DIRECTION " <<layer->direction() <<endl;
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"  MINWIDTH  " <<layer->minwidth() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  AREA      " <<layer->area() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  WIDTH     " <<layer->width() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerNum  " <<parser->readLayerCnt <<endl;
    }
    tmpLayer->setLayerNum(parser->readLayerCnt++);
    tmpLayer->setName(layer->name());
    parser->tech->addLayer(std::move(uLayer));

    tmpLayer->setWidth(round(layer->width() * parser->tech->getDBUPerUU()));
    if (layer->hasMinwidth()) {
      tmpLayer->setMinWidth(round(layer->minwidth() * parser->tech->getDBUPerUU()));
      if (tmpLayer->getMinWidth() > tmpLayer->getWidth()) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: minWidth on layer " << layer->name() << " is larger than width, setting minWidth to width\n";
        tmpLayer->setMinWidth(tmpLayer->getWidth());
      }
    } else {
      tmpLayer->setMinWidth(tmpLayer->getWidth());
    }
    // add minWidth constraint
    auto minWidthConstraint = make_unique<frMinWidthConstraint>(tmpLayer->getMinWidth());
    tmpLayer->setMinWidthConstraint(minWidthConstraint.get());
    parser->tech->addUConstraint(std::move(minWidthConstraint));

    tmpLayer->setType(frLayerTypeEnum::ROUTING);
    if (!strcmp(layer->direction(), "HORIZONTAL")) {
      tmpLayer->setDir(frcHorzPrefRoutingDir);
    } else if (!strcmp(layer->direction(), "VERTICAL")) {
      tmpLayer->setDir(frcVertPrefRoutingDir);
    }
    tmpLayer->setPitch(round(layer->pitch() * parser->tech->getDBUPerUU()));

    // Add off grid rule for every layer
    auto recheckConstraint = make_unique<frRecheckConstraint>();
    tmpLayer->setRecheckConstraint(recheckConstraint.get());
    parser->tech->addUConstraint(std::move(recheckConstraint));

    // Add short rule for every layer
    auto shortConstraint = make_unique<frShortConstraint>();
    tmpLayer->setShortConstraint(shortConstraint.get());
    parser->tech->addUConstraint(std::move(shortConstraint));

    // Add off grid rule for every layer
    auto offGridConstraint = make_unique<frOffGridConstraint>();
    tmpLayer->setOffGridConstraint(offGridConstraint.get());
    parser->tech->addUConstraint(std::move(offGridConstraint));
    
    // Add nsmetal rule for every layer
    auto nsmetalConstraint = make_unique<frNonSufficientMetalConstraint>();
    tmpLayer->setNonSufficientMetalConstraint(nsmetalConstraint.get());

    parser->tech->addUConstraint(std::move(nsmetalConstraint));

    //cout << __FILE__ << ":" << __LINE__ << ": " <<"number of props " <<layer->numProps() <<endl;
    for (int i = 0; i < layer->numProps(); i++) {
      if (string(layer->propName(i)) == string("LEF58_PROTRUSIONWIDTH") ||
          // string(layer->propName(i)) == string("LEF58_MINSTEP") ||
          string(layer->propName(i)) == string("LEF58_ENCLOSURESPACING") ||
          string(layer->propName(i)) == string("LEF58_VOLTAGESPACING") ||
          string(layer->propName(i)) == string("LEF58_ANTENNAGATEPLUSDIFF") ||
          string(layer->propName(i)) == string("LEF58_ANTENNAGATEPWL") ||
          string(layer->propName(i)) == string("LEF58_ANTENNAGATEPWL") ||
          string(layer->propName(i)) == string("LEF58_FORBIDDENSPACING") 
         ) {
        ;
      } else {
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"name:     " <<layer->propName(i) <<endl;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"value:    " <<layer->propValue(i) <<endl;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"number:   " <<layer->propNumber(i) <<endl;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"type:     " <<layer->propType(i) <<endl;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"isNumber: " <<layer->propIsNumber(i) <<endl;
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"isString: " <<layer->propIsString(i) <<endl;
        if (!strcmp(layer->propName(i), "LEF58_CORNERSPACING") && layer->propIsString(i)) {
         //cout << __FILE__ << ":" << __LINE__ << ": " <<"start parsing LEF58_CORNERSPACING" <<endl;
         getLef58CornerSpacing(data, tmpLayer, layer->propValue(i));
        }
        if (!strcmp(layer->propName(i), "LEF58_SPACING") && layer->propIsString(i)) {
          //cout << __FILE__ << ":" << __LINE__ << ": " <<"name:     " <<layer->propName(i) <<endl;
          //cout << __FILE__ << ":" << __LINE__ << ": " <<"value:    " <<layer->propValue(i) <<endl;
          getLef58Spacing(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF57_SPACING") && layer->propIsString(i)) {
          getLef58Spacing(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_SPACINGTABLE") && layer->propIsString(i)) {
          //cout << __FILE__ << ":" << __LINE__ << ": " <<"name:     " <<layer->propName(i) <<endl;
          //cout << __FILE__ << ":" << __LINE__ << ": " <<"value:    " <<layer->propValue(i) <<endl;
          getLef58SpacingTable(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_RIGHTWAYONGRIDONLY") && layer->propIsString(i)) {
          getLef58RightWayOnGridOnly(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_RECTONLY") && layer->propIsString(i)) {
          getLef58RectOnly(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_MINSTEP") && layer->propIsString(i)) {
          // cout << __FILE__ << ":" << __LINE__ << ": " <<"name:     " <<layer->propName(i) <<endl;
          // cout << __FILE__ << ":" << __LINE__ << ": " <<"value:    " <<layer->propValue(i) <<endl;
          getLef58MinStep(data, tmpLayer, layer->propValue(i));
        } else {
          //cout << __FILE__ << ":" << __LINE__ << ": " <<"  name:     " <<layer->propName(i) <<endl;
        }
      }
    }

    // read minArea rule
    if (layer->hasArea()) {
      frCoord minArea = frCoord(round(layer->area() * parser->tech->getDBUPerUU() * parser->tech->getDBUPerUU()));
      unique_ptr<frConstraint> uCon = make_unique<frAreaConstraint>(minArea);
      auto rptr = static_cast<frAreaConstraint*>(uCon.get());
      parser->tech->addUConstraint(std::move(uCon));
      //std::cout << __FILE__ << ":" << __LINE__ << ": " << "Add minArea constraint to " << tmpLayer->getName() << "\n";
      tmpLayer->setAreaConstraint(rptr);
    }

    if (layer->hasMinstep()) {
      if (layer->numMinstep() > 1) {
        std::cout << __FILE__ << ":" << __LINE__ << ": " << "ERROR: only one minStep rule should be defined for a given layer. Only the last one is checked\n";
      }
      for (int i = 0; i < layer->numMinstep(); ++i) {
        unique_ptr<frConstraint> uCon = make_unique<frMinStepConstraint>();
        auto rptr = static_cast<frMinStepConstraint*>(uCon.get());
        if (layer->hasMinstepType(i)) {
          if (strcmp(layer->minstepType(i), "INSIDECORNER") == 0) {
            rptr->setInsideCorner(true);
            rptr->setOutsideCorner(false);
            rptr->setStep(false);
            rptr->setMinstepType(frMinstepTypeEnum::INSIDECORNER);
          } else if (strcmp(layer->minstepType(i), "OUTSIDECORNER") == 0) {
            rptr->setInsideCorner(false);
            rptr->setOutsideCorner(true);
            rptr->setStep(false);
            rptr->setMinstepType(frMinstepTypeEnum::OUTSIDECORNER);
          } else if (strcmp(layer->minstepType(i), "STEP") == 0) {
            rptr->setInsideCorner(false);
            rptr->setOutsideCorner(false);
            rptr->setStep(true);
            rptr->setMinstepType(frMinstepTypeEnum::STEP);
          }
        } else {
           rptr->setMinstepType(frMinstepTypeEnum::OUTSIDECORNER);
        }
        if (layer->hasMinstepLengthsum(i)) {
          rptr->setMaxLength(frCoord(layer->minstepLengthsum(i) * parser->tech->getDBUPerUU()));
        }
        if (layer->hasMinstepMaxedges(i)) {
          rptr->setMaxEdges(layer->minstepMaxedges(i));
          rptr->setInsideCorner(true);
          rptr->setOutsideCorner(true);
          rptr->setStep(true);
          rptr->setMinstepType(frMinstepTypeEnum::UNKNOWN);
        }
        rptr->setMinStepLength(layer->minstep(i) * parser->tech->getDBUPerUU());
        parser->tech->addUConstraint(std::move(uCon));
        //std::cout << __FILE__ << ":" << __LINE__ << ": " << "Add minStep constraint to " << tmpLayer->getName() << "\n";
        tmpLayer->setMinStepConstraint(rptr);
      }
    }

    // read minHole rule
    for (int i = 0; i < layer->numMinenclosedarea(); ++i) {
      frCoord minEnclosedArea = frCoord(round(layer->minenclosedarea(i) * parser->tech->getDBUPerUU() * parser->tech->getDBUPerUU()));
      frCoord minEnclosedWidth = -1;
      if (layer->hasMinenclosedareaWidth(i)) {
        minEnclosedWidth = frCoord(round(layer->minenclosedareaWidth(i) * parser->tech->getDBUPerUU()));
        cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: minEnclosedArea constraint with width is not supported, skipped\n";
        continue;
      }
      auto minEnclosedAreaConstraint = make_unique<frMinEnclosedAreaConstraint>(minEnclosedArea);
      if (minEnclosedWidth != -1) {
        minEnclosedAreaConstraint->setWidth(minEnclosedWidth);
      }
      tmpLayer->addMinEnclosedAreaConstraint(minEnclosedAreaConstraint.get());
      parser->tech->addUConstraint(std::move(minEnclosedAreaConstraint));
    }

    // read spacing rule
    for (int i = 0; i < layer->numSpacing(); ++i) {
      //std::shared_ptr<frSpacingConstraint> minSpacingCosntraint;
      frCoord minSpacing = frCoord(round(layer->spacing(i) * parser->tech->getDBUPerUU()));
      if (layer->hasSpacingRange(i)) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WARNING: hasSpacing Range unsupported" <<endl;
      } else if (layer->hasSpacingLengthThreshold(i)) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WARNING: hasSpacingLengthThreshold unsupported" <<endl;
      } else if (layer->hasSpacingEndOfLine(i)) {
        // new
        unique_ptr<frConstraint> uCon = make_unique<frSpacingEndOfLineConstraint>();
        auto rptr = static_cast<frSpacingEndOfLineConstraint*>(uCon.get());
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACING " <<layer->spacing(i) <<" ENDOFLINE " <<layer->spacingEolWidth(i)
               <<" WITHIN " <<layer->spacingEolWithin(i);
        }
        frCoord eolWidth = frCoord(round(layer->spacingEolWidth(i) * parser->tech->getDBUPerUU()));
        frCoord eolWithin = frCoord(round(layer->spacingEolWithin(i) * parser->tech->getDBUPerUU()));
        rptr->setMinSpacing(minSpacing);
        rptr->setEolWidth(eolWidth);
        rptr->setEolWithin(eolWithin);
        if (layer->hasSpacingParellelEdge(i)) {
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" PARALLELEDGE " <<layer->spacingParSpace(i) <<" WITHIN " <<layer->spacingParWithin(i);
            if (layer->hasSpacingTwoEdges(i)) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" TWOEDGES";
            }
          }
          frCoord parSpace = frCoord(round(layer->spacingParSpace(i) * parser->tech->getDBUPerUU()));
          frCoord parWithin = frCoord(round(layer->spacingParWithin(i) * parser->tech->getDBUPerUU()));
          rptr->setParSpace(parSpace);
          rptr->setParWithin(parWithin);
          rptr->setTwoEdges(layer->hasSpacingTwoEdges(i));
        }
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
        }
        parser->tech->addUConstraint(std::move(uCon));
        tmpLayer->addEolSpacing(rptr);
      } else if (layer->hasSpacingSamenet(i)) {
        bool pgOnly = layer->hasSpacingSamenetPGonly(i);
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACING " <<layer->spacing(i) <<" SAMENET ";
          if (pgOnly) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"PGONLY ";
          }
          cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
        }
        unique_ptr<frConstraint> uCon = make_unique<frSpacingSamenetConstraint>(minSpacing, pgOnly);
        auto rptr = uCon.get();
        parser->tech->addUConstraint(std::move(uCon));
        if (tmpLayer->hasSpacingSamenet()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: new SPACING SAMENET overrides old SPACING SAMENET rule" <<endl;
        }
        tmpLayer->setSpacingSamenet(static_cast<frSpacingSamenetConstraint*>(rptr));
      } else if (layer->hasSpacingNotchLength(i)) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WARNING: hasSpacingNotchLength unsupported" <<endl;
      } else if (layer->hasSpacingEndOfNotchWidth(i)) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WARNING: hasSpacingEndOfNotchWidth unsupported" <<endl;
      } else { // min spacing
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACING " <<layer->spacing(i) <<" ;" <<endl;
        }
        
        frCollection<frCoord> rowVals(1, 0), colVals(1, 0);
        frCollection<frCollection<frCoord> > tblVals(1, {minSpacing});
        frString rowName("WIDTH"), colName("PARALLELRUNLENGTH");
        unique_ptr<frConstraint> uCon = make_unique<frSpacingTablePrlConstraint>(fr2DLookupTbl(rowName, rowVals, colName, colVals, tblVals));
        auto rptr = static_cast<frSpacingTablePrlConstraint*>(uCon.get());
        parser->tech->addUConstraint(std::move(uCon));
        if (tmpLayer->getMinSpacing()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: new SPACING overrides old SPACING rule" <<endl;
        }
        tmpLayer->setMinSpacing(rptr);
      }
    }

    // read spacingTable
    for (int i = 0; i < layer->numSpacingTable(); ++i) {
      // old
      std::shared_ptr<frSpacingTableConstraint> spacingTableConstraint;
      auto spTable = layer->spacingTable(i);
      if (spTable->isInfluence()) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" WARNING: SPACINGTABLE INFLUENCE unsupported" <<endl;
      } else if (spTable->isParallel()) {
        // old
        shared_ptr<fr2DLookupTbl<frCoord, frCoord, frCoord> > prlTbl;
        auto parallel = spTable->parallel();
        frCollection<frCoord> rowVals, colVals;
        frCollection<frCollection<frCoord> > tblVals;
        frCollection<frCoord> tblRowVals;
        frString rowName("WIDTH"), colName("PARALLELRUNLENGTH");
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACINGTABLE" <<endl;
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  PARALLELRUNLENGTH";
        }
        for (int j = 0; j < parallel->numLength(); ++j) {
          frCoord prl = frCoord(round(parallel->length(j) * parser->tech->getDBUPerUU()));
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<prl * 1.0 / parser->tech->getDBUPerUU();
          }
          colVals.push_back(prl);
        }
        for (int j = 0; j < parallel->numWidth(); ++j) {
          frCoord width = frCoord(round(parallel->width(j) * parser->tech->getDBUPerUU()));
          rowVals.push_back(width);
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"  WIDTH " <<width * 1.0 / parser->tech->getDBUPerUU();
          }
          tblRowVals.clear();
          for (int k = 0; k < parallel->numLength(); ++k) {
            frCoord spacing = frCoord(round(parallel->widthSpacing(j, k) * parser->tech->getDBUPerUU()));
            tblRowVals.push_back(spacing);
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<spacing * 1.0 / parser->tech->getDBUPerUU();
            }
          }
          tblVals.push_back(tblRowVals);
        }
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
        }
        // old
        prlTbl = make_shared<fr2DLookupTbl<frCoord, frCoord, frCoord> >(rowName, rowVals, colName, colVals, tblVals);
        spacingTableConstraint = make_shared<frSpacingTableConstraint>(prlTbl);
        parser->tech->addConstraint(spacingTableConstraint);
        tmpLayer->addConstraint(spacingTableConstraint);

        // new
        unique_ptr<frConstraint> uCon = make_unique<frSpacingTablePrlConstraint>(fr2DLookupTbl(rowName, rowVals, colName, colVals, tblVals));
        auto rptr = static_cast<frSpacingTablePrlConstraint*>(uCon.get());
        parser->tech->addUConstraint(std::move(uCon));
        if (tmpLayer->getMinSpacing()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: new SPACINGTABLE PARALLELRUNLENGTH overrides old SPACING rule" <<endl;
        }
        tmpLayer->setMinSpacing(rptr);
      } else { // two width spacing rule
        auto tw = spTable->twoWidths();
        frCoord defaultPrl = -abs(frCoord(round(tw->widthSpacing(0,0) * parser->tech->getDBUPerUU())));
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"default prl: " <<defaultPrl <<endl;
        frCollection<frSpacingTableTwRowType> rowVals, colVals;
        frCollection<frCollection<frCoord> > tblVals;
        frCollection<frCoord> tblRowVals;
        frString rowName("WIDTH1PRL"), colName("WIDTH2PRL");
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"  SPACINGTABLE TWOWIDTHS";
        }
        for (int j = 0; j < tw->numWidth(); ++j) {
          frCoord width = frCoord(round(tw->width(j) * parser->tech->getDBUPerUU()));
          frCoord prl   = defaultPrl;
          if (enableOutput) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"    WIDTH " <<tw->width(j);
          }
          if (tw->hasWidthPRL(j)) {
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" PRL " <<tw->widthPRL(j);
            }
            prl = frCoord(round(tw->widthPRL(j) * parser->tech->getDBUPerUU()));
            defaultPrl = prl;
            //hasPrl = true;
          } else {
            //if (!hasPrl) {
            //  defaultPrl = -abs(frCoord(round(tw->widthSpacing(j,0) * parser->tech->getDBUPerUU())));
            //  prl = defaultPrl;
            //}
          }
          colVals.push_back(frSpacingTableTwRowType(width, prl));
          rowVals.push_back(frSpacingTableTwRowType(width, prl));
          tblRowVals.clear();
          for (int k = 0; k < tw->numWidthSpacing(j); k++) {
            if (enableOutput) {
              cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<tw->widthSpacing(j, k);
            }
            frCoord spacing = frCoord(round(tw->widthSpacing(j, k) * parser->tech->getDBUPerUU()));
            tblRowVals.push_back(spacing);
          }
          tblVals.push_back(tblRowVals);
        }
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
        }
        unique_ptr<frConstraint> uCon = make_unique<frSpacingTableTwConstraint>(fr2DLookupTbl(rowName, rowVals, colName, colVals, tblVals));
        auto rptr = static_cast<frSpacingTableTwConstraint*>(uCon.get());
        parser->tech->addUConstraint(std::move(uCon));
        if (tmpLayer->getMinSpacing()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: new SPACINGTABLE TWOWIDTHS overrides old SPACING rule" <<endl;
        }
        tmpLayer->setMinSpacing(rptr);
      }
    }

    for (int i = 0; i < layer->numMinimumcut(); i++) {
      double dbu = parser->tech->getDBUPerUU();
      unique_ptr<frConstraint> uCon = make_unique<frMinimumcutConstraint>();
      auto rptr = static_cast<frMinimumcutConstraint*>(uCon.get());
      
      //if (enableOutput) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<"  MINIMUMCUT " <<layer->minimumcut(i) <<" WIDTH " <<layer->minimumcutWidth(i);
      //}
      rptr->setNumCuts(layer->minimumcut(i));
      rptr->setWidth(frCoord(round(layer->minimumcutWidth(i) * dbu)));

      if (layer->hasMinimumcutWithin(i)) {
        //if (enableOutput) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<" WITHIN " <<layer->minimumcutWithin(i);
        //}
        rptr->setWithin(frCoord(round(layer->minimumcutWithin(i) * dbu)));
      }

      if (layer->hasMinimumcutConnection(i)) {
        //if (enableOutput) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<layer->minimumcutConnection(i);
        //}
        if (strcmp(layer->minimumcutConnection(i), "FROMABOVE") == 0) {
          rptr->setConnection(frMinimumcutConnectionEnum::FROMABOVE);
        } else if (strcmp(layer->minimumcutConnection(i), "FROMBELOW") == 0) {
          rptr->setConnection(frMinimumcutConnectionEnum::FROMBELOW);
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: layer minimumcut unsupporterd connection type" <<endl;
          exit(1);
        }
      }
      
      // hasMinimumcutNumCuts actually outputs whether there is [LENGTH length WITHIN distance]
      if (layer->hasMinimumcutNumCuts(i)) {
        //if (enableOutput) {
        //  cout << __FILE__ << ":" << __LINE__ << ": " <<" LENGTH " <<layer->minimumcutLength(i) <<" WITHIN " <<layer->minimumcutDistance(i);
        //}
        rptr->setLength(frCoord(round(layer->minimumcutLength(i) * dbu)), frCoord(round(layer->minimumcutDistance(i) * dbu)));
      }

      parser->tech->addUConstraint(std::move(uCon));
      tmpLayer->addMinimumcutConstraint(rptr);
      //if (enableOutput) {
      //  cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
      //}
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  MINIMUMCUT " <<rptr->getNumCuts() <<" WIDTH " <<rptr->getWidth() / dbu;
        if (rptr->hasWithin()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" WITHIN " <<rptr->getCutDistance() / dbu;
        }
        if (rptr->hasConnection()) {
          switch(rptr->getConnection()) {
            case frMinimumcutConnectionEnum::FROMABOVE: 
              cout << __FILE__ << ":" << __LINE__ << ": " <<" FROMABOVE"; 
              break;
            case frMinimumcutConnectionEnum::FROMBELOW: 
              cout << __FILE__ << ":" << __LINE__ << ": " <<" FROMBELOW";
              break;
            default: 
              cout << __FILE__ << ":" << __LINE__ << ": " <<" UNKNOWN";
          }
        }
        if (rptr->hasLength()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" LENGTH " <<rptr->getLength() / dbu <<" WITHIN " <<rptr->getDistance() / dbu;
        }
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
      }
    }
  } else if (strcmp(layer->type(), "CUT") == 0) {
    // add default masterslice layer if LEF does not have one
    if (parser->readLayerCnt == 0) {
      unique_ptr<frLayer> uMSLayer = make_unique<frLayer>();
      auto tmpMSLayer = uMSLayer.get();
      tmpMSLayer->setLayerNum(parser->readLayerCnt++);
      tmpMSLayer->setName(masterSliceLayerName);
      parser->tech->addLayer(std::move(uMSLayer));
      tmpMSLayer->setType(frLayerTypeEnum::MASTERSLICE);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"\n";
        cout << __FILE__ << ":" << __LINE__ << ": " <<"LAYER     " <<tmpMSLayer->getName() <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  TYPE      " <<"MASTERSLICE" <<endl;
        cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerNum  " <<tmpMSLayer->getLayerNum() <<endl;
      }
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"\n";
      cout << __FILE__ << ":" << __LINE__ << ": " <<"LAYER "       <<layer->name() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  TYPE      " <<layer->type() <<endl;
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  layerNum  " <<parser->readLayerCnt <<endl;
    }
    tmpLayer->setLayerNum(parser->readLayerCnt++);
    tmpLayer->setName(layer->name());
    tmpLayer->setType(frLayerTypeEnum::CUT);
    parser->tech->addLayer(std::move(uLayer));

    auto shortConstraint = make_shared<frShortConstraint>();
    // std::cout << __FILE__ << ":" << __LINE__ << ": " << "add shortConstraint to layer " <<tmpLayer->getName() << "\n";
    parser->tech->addConstraint(shortConstraint);
    tmpLayer->addConstraint(shortConstraint);
    tmpLayer->setShortConstraint(shortConstraint.get());

    // read spacing constraint
    for (int i = 0; i < layer->numSpacing(); ++i) {
      std::shared_ptr<frCutSpacingConstraint> cutSpacingConstraint;
      frCoord cutArea = frCoord(round(layer->spacingArea(i) * parser->tech->getDBUPerUU()));
      frCoord cutSpacing = frCoord(round(layer->spacing(i) * parser->tech->getDBUPerUU()));
      bool centerToCenter = layer->hasSpacingCenterToCenter(i);
      bool sameNet = layer->hasSpacingSamenet(i);
      bool stack = layer->hasSpacingLayerStack(i);
      bool exceptSamePGNet = layer->hasSpacingSamenetPGonly(i);
      bool parallelOverlap = layer->hasSpacingParallelOverlap(i);
      frString secondLayerName = layer->hasSpacingName(i) ? string(layer->spacingName(i)) : string("");
      int adjacentCuts = layer->spacingAdjacentCuts(i);
      frCoord cutWithin = frCoord(round(layer->spacingAdjacentWithin(i) * parser->tech->getDBUPerUU()));

      // std::cout << __FILE__ << ":" << __LINE__ << ": " << cutSpacing << " " << centerToCenter << " " << sameNet << " " << stack << " " << exceptSamePGNet 
      //           << " " << parallelOverlap << " " << secondLayerName << " " << adjacentCuts << " " << cutWithin << "\n";

      // std::cout << __FILE__ << ":" << __LINE__ << ": " << "raw cutArea = " << layer->spacingArea(i) << "\n";
      // std::cout << __FILE__ << ":" << __LINE__ << ": " << "cutArea = " << cutArea << "\n";
      // initialize for invalid variables
      cutArea = (cutArea == 0) ? -1 : cutArea;
      cutWithin = (cutWithin == 0) ? -1 : cutWithin;
      adjacentCuts = (adjacentCuts == 0) ? -1 : adjacentCuts;
      // std::cout << __FILE__ << ":" << __LINE__ << ": " << "cutArea = " << cutArea << "\n";

      if (cutWithin != -1 && cutWithin < cutSpacing) {
        cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: cutWithin is smaller than cutSpacing for ADJACENTCUTS on layer " << layer->name() 
             << ", please check your rule definition\n";
      }

      cutSpacingConstraint = make_shared<frCutSpacingConstraint>(cutSpacing,
                                                              centerToCenter, 
                                                              sameNet, 
                                                              secondLayerName, 
                                                              stack,
                                                              adjacentCuts,
                                                              cutWithin,
                                                              exceptSamePGNet,
                                                              parallelOverlap,
                                                              cutArea);

      parser->tech->addConstraint(cutSpacingConstraint);
      tmpLayer->addConstraint(cutSpacingConstraint);
      tmpLayer->addCutSpacingConstraint(cutSpacingConstraint.get());

    }

    // lef58
    for (int i = 0; i < layer->numProps(); i++) {
      if (string(layer->propName(i)) == string("LEF58_ENCLOSUREEDGE") ||
          string(layer->propName(i)) == string("LEF58_ENCLOSURE") ||
          string(layer->propName(i)) == string("LEF58_ENCLOSURETABLE")
         ) {
        ;
      } else {
        if (!strcmp(layer->propName(i), "LEF58_CUTCLASS") && layer->propIsString(i)) {
          getLef58CutClass(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_SPACING") && layer->propIsString(i)) {
          getLef58CutSpacing(data, tmpLayer, layer->propValue(i));
        } else if (!strcmp(layer->propName(i), "LEF58_SPACINGTABLE") && layer->propIsString(i)) {
          getLef58CutSpacingTable(data, tmpLayer, layer->propValue(i));
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" Unsupported property name:     " <<layer->propName(i) <<endl;
        }
      }
    }
  } else if (strcmp(layer->type(), "MASTERSLICE") == 0) {
    bool hasWell = false;
    for (int i = 0; i < layer->numProps(); i++) {
      if (string(layer->propName(i)) == string("LEF58_TYPE")) {
        if (layer->propValue(i) == string("TYPE PWELL") || 
            layer->propValue(i) == string("TYPE NWELL")) {
          hasWell = true;
        }
      }
    }
    if (!hasWell) {
      masterSliceLayerName = string(layer->name());
    }
  } else {
    ;
  }

  return 0;
}

int io::Parser::Callbacks::getLefMacros(lefrCallbackType_e type, lefiMacro* macro, lefiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (type != lefrMacroCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrMacroCbkType!" <<endl;
    exit(2);
  }

  io::Parser* parser = (io::Parser*) data;
  frCoord originX = round(macro->originX() * parser->tech->getDBUPerUU()); 
  frCoord originY = round(macro->originY() * parser->tech->getDBUPerUU());
  frCoord sizeX   = round(macro->sizeX()   * parser->tech->getDBUPerUU());
  frCoord sizeY   = round(macro->sizeY()   * parser->tech->getDBUPerUU());
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  ORIGIN " <<originX * 1.0 / parser->tech->getDBUPerUU() <<" " 
                       <<originY * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  SIZE   " <<sizeX   * 1.0 / parser->tech->getDBUPerUU() <<" " 
                       <<sizeY   * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
  }
  vector<frBoundary> bounds;
  frBoundary bound;
  vector<frPoint> points;
  points.push_back(frPoint(originX, originY));
  points.push_back(frPoint(sizeX,   originY));
  points.push_back(frPoint(sizeX,   sizeY));
  points.push_back(frPoint(originX, sizeY));
  bound.setPoints(points);
  bounds.push_back(bound);

  parser->tmpBlock->setBoundaries(bounds);

  if (enableOutput) {
    if (macro->hasClass()) {
      std::cout << __FILE__ << ":" << __LINE__ << ": " << macro->macroClass() << "\n";
    }
  }
  if (macro->hasClass()) {
    if (strcmp(macro->macroClass(), "CORE") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE);
    } else if (strcmp(macro->macroClass(), "CORE TIEHIGH") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE_TIEHIGH);
    } else if (strcmp(macro->macroClass(), "CORE TIELOW") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE_TIELOW);
    } else if (strcmp(macro->macroClass(), "CORE WELLTAP") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE_WELLTAP);
    } else if (strcmp(macro->macroClass(), "CORE SPACER") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE_SPACER);
    } else if (strcmp(macro->macroClass(), "CORE ANTENNACELL") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::CORE_ANTENNACELL);
    } else if (strcmp(macro->macroClass(), "COVER") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::COVER);
    } else if (strcmp(macro->macroClass(), "ENDCAP PRE") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::ENDCAP_PRE);
    } else if (strcmp(macro->macroClass(), "BLOCK") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::BLOCK);
    } else if (strcmp(macro->macroClass(), "PAD") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::PAD);
    } else if (strcmp(macro->macroClass(), "RING") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::RING);
    } else if (strcmp(macro->macroClass(), "PAD POWER") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::PAD_POWER);
    } else if (strcmp(macro->macroClass(), "PAD SPACER") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::PAD_SPACER);
    } else if (strcmp(macro->macroClass(), "ENDCAP BOTTOMLEFT") == 0) {
      parser->tmpBlock->setMacroClass(MacroClassEnum::ENDCAP_BOTTOMLEFT);
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: unknown macroClass " << macro->macroClass() << ", skipped macroClass property\n"; 
    }
  }


  return 0;
}

int io::Parser::Callbacks::getLefPins(lefrCallbackType_e type, lefiPin* pin, lefiUserData data) {
  bool enableOutput = false;
  //bool enableOutput = true;
  if (type != lefrPinCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrPinCbkType!" <<endl;
    exit(1);
  }
  io::Parser* parser = (io::Parser*) data;

  // term
  unique_ptr<frTerm>         uTerm = make_unique<frTerm>(pin->name());
  auto term = uTerm.get();
  term->setId(parser->numTerms);
  parser->numTerms++;

  // inst 
  parser->tmpBlock->addTerm(std::move(uTerm));
  // inst completed
  
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  PIN " <<pin->name() <<endl;
  }
  
  frTermEnum termType = frTermEnum::frcNormalTerm;
  if (pin->hasUse()) {
    string str(pin->use());
    if (str == "SIGNAL") {
      ;
    } else if (str == "CLOCK") {
      termType = frTermEnum::frcClockTerm;
    } else if (str == "POWER") {
      termType = frTermEnum::frcPowerTerm;
    } else if (str == "GROUND") {
      termType = frTermEnum::frcGroundTerm;
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported PIN USE in lef" <<endl;
      exit(1);
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    USE " <<str <<" ;" <<endl;
    }
  }
  term->setType(termType);

  int numPorts = pin->numPorts();
  int numItems = 0;
  int itemType = 0;
  // cout << __FILE__ << ":" << __LINE__ << ": " <<"pin->numPorts: " <<numPorts <<endl;
  for (int i = 0; i < numPorts; ++i) {
    numItems = pin->port(i)->numItems();
    // cout << __FILE__ << ":" << __LINE__ << ": " <<"pin->ports(" <<i <<")->numItems: " <<numItems <<endl;
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    PORT" <<endl;
    }

    // pin
    auto pinIn = make_unique<frPin>();
    pinIn->setId(i);

    // term
    frLayerNum layerNum = -1;
    for (int j = 0; j < numItems; ++j) {
      itemType = pin->port(i)->itemType(j);
      if (itemType == lefiGeomLayerE) {
        string layer = pin->port(i)->getLayer(j);
        if (parser->tech->name2layer.find(layer) == parser->tech->name2layer.end()) {
          if (VERBOSE > 1) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: layer " <<layer <<" is skipped for " <<parser->tmpBlock->getName() <<"/" <<pin->name() <<endl;
          }
          layerNum = -1;
          continue;
        }

        layerNum = parser->tech->name2layer.at(layer)->getLayerNum();
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"  layer: " <<pin->port(i)->getLayer(j) <<endl;
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"    LAYER " <<layer <<" ;" <<endl;
        }
        //cout << __FILE__ << ":" << __LINE__ << ": " <<"    LAYERNUM " <<layerNum <<" ;" <<endl;
      } else if (itemType == lefiGeomRectE) {
        if (layerNum == -1) {
          continue;
        }
        frCoord xl = round(pin->port(i)->getRect(j)->xl * parser->tech->getDBUPerUU());
        frCoord yl = round(pin->port(i)->getRect(j)->yl * parser->tech->getDBUPerUU());
        frCoord xh = round(pin->port(i)->getRect(j)->xh * parser->tech->getDBUPerUU());
        frCoord yh = round(pin->port(i)->getRect(j)->yh * parser->tech->getDBUPerUU());

        // pinFig
        unique_ptr<frRect> pinFig = make_unique<frRect>();
        pinFig->setBBox(frBox(xl, yl, xh, yh));
        pinFig->addToPin(pinIn.get());
        pinFig->setLayerNum(layerNum);
        // pinFig completed
        // pin
        unique_ptr<frPinFig> uptr(std::move(pinFig));
        pinIn->addPinFig(std::move(uptr));
        // pin completed

        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"      RECT " <<xl * 1.0 / parser->tech->getDBUPerUU() <<" " 
                               <<yl * 1.0 / parser->tech->getDBUPerUU() <<" " 
                               <<xh * 1.0 / parser->tech->getDBUPerUU() <<" " 
                               <<yh * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
        }
             
      } else if (itemType == lefiGeomPolygonE) {
        if (layerNum == -1) {
          continue;
        }
        //Polygon polygon;
        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"      POLYGON"; 
        }
        frCollection<frPoint> tmpPoints;
        for (int k = 0; k < pin->port(i)->getPolygon(j)->numPoints; k++) {
          frCoord x = round(pin->port(i)->getPolygon(j)->x[k] * parser->tech->getDBUPerUU());
          frCoord y = round(pin->port(i)->getPolygon(j)->y[k] * parser->tech->getDBUPerUU());
          tmpPoints.push_back(frPoint(x, y));
          if (enableOutput) {
             cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<x * 1.0 / parser->tech->getDBUPerUU() <<" " 
                        <<y * 1.0 / parser->tech->getDBUPerUU();
          }
        }
        // pinFig
        unique_ptr<frPolygon> pinFig = make_unique<frPolygon>();
        pinFig->setPoints(tmpPoints);
        pinFig->addToPin(pinIn.get());
        pinFig->setLayerNum(layerNum);
        // pinFig completed
        // pin
        unique_ptr<frPinFig> uptr(std::move(pinFig));
        pinIn->addPinFig(std::move(uptr));
        // pin completed

        if (enableOutput) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
        }
      } else {
        if (VERBOSE > -1) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported lefiGeometries in getLefPins!" <<endl;
        }
        continue;
      }
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"  enum: " <<pin->port(i)->itemType(j) <<endl;
    }
    term->addPin(std::move(pinIn));
    // term completed
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"    END" <<endl;
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  END " <<pin->name() <<endl;
  }

  return 0;
}


int io::Parser::Callbacks::getLefObs(lefrCallbackType_e type, lefiObstruction* obs, lefiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;

  if (type != lefrObstructionCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrObstructionCbkType!" <<endl;
    exit(1);
  }

  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  OBS" <<endl;
  }
  
  auto geometry = obs->geometries();
  int numItems  = geometry->numItems();
  io::Parser* parser = (io::Parser*) data;

  // blockage
  auto blkIn = make_unique<frBlockage>();
  blkIn->setId(parser->numBlockages);
  parser->numBlockages++;
  // pin
  auto pinIn = make_unique<frPin>();
  pinIn->setId(0);
  
  string layer = "";
  frLayerNum layerNum = -1;
  for (int i = 0; i < numItems; ++i) {
    if (geometry->itemType(i) == lefiGeomLayerE) {
      layer = geometry->getLayer(i);
      if (parser->tech->name2layer.find(layer) != parser->tech->name2layer.end()) {
        layerNum = parser->tech->name2layer[layer]->getLayerNum();
      } else {
        if (VERBOSE > 2) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: layer " <<geometry->getLayer(i) <<" is skipped for " <<parser->tmpBlock->getName() <<"/OBS" <<endl; 
        }
        layerNum = -1;
        continue;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    LAYER " <<layer <<" ;" <<endl;
      }
    } else if (geometry->itemType(i) == lefiGeomRectE) {
      if (layerNum == -1) {
        // cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: OBS on undefined layer " <<" is skipped... " <<endl; 
        continue;
      }
      auto rect = geometry->getRect(i);
      frCoord xl = round(rect->xl * parser->tech->getDBUPerUU());
      frCoord yl = round(rect->yl * parser->tech->getDBUPerUU());
      frCoord xh = round(rect->xh * parser->tech->getDBUPerUU());
      frCoord yh = round(rect->yh * parser->tech->getDBUPerUU());
      // pinFig
      unique_ptr<frRect> pinFig = make_unique<frRect>();
      pinFig->setBBox(frBox(xl, yl, xh, yh));
      pinFig->addToPin(pinIn.get());
      pinFig->setLayerNum(layerNum);
      // pinFig completed
      // pin
      unique_ptr<frPinFig> uptr(std::move(pinFig));
      pinIn->addPinFig(std::move(uptr));
      // pin completed
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"      RECT " <<rect->xl <<" " <<rect->yl <<" " <<rect->xh <<" " <<rect->yh <<" ;" <<endl;
      }
    } else if (geometry->itemType(i) == lefiGeomPolygonE) {
      if (layerNum == -1) {
        // cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: OBS on undefined layer " <<" is skipped... " <<endl; 
        continue;
      }
      std::vector<frPoint> tmpPoints;
      for (int k = 0; k < geometry->getPolygon(i)->numPoints; k++) {
        frCoord x = round(geometry->getPolygon(i)->x[k] * parser->tech->getDBUPerUU());
        frCoord y = round(geometry->getPolygon(i)->y[k] * parser->tech->getDBUPerUU());
        tmpPoints.push_back(frPoint(x, y));
        if (enableOutput) {
           cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<x * 1.0 / parser->tech->getDBUPerUU() <<" " 
                      <<y * 1.0 / parser->tech->getDBUPerUU();
        }
      }
      // pinFig
      unique_ptr<frPolygon> pinFig = make_unique<frPolygon>();
      pinFig->setPoints(tmpPoints);
      pinFig->addToPin(pinIn.get());
      pinFig->setLayerNum(layerNum);
      // pinFig completed
      // pin
      unique_ptr<frPinFig> uptr(std::move(pinFig));
      pinIn->addPinFig(std::move(uptr));
      // pin completed
    } else if (geometry->itemType(i) == lefiGeomLayerMinSpacingE) {
      if (layerNum == -1) {
        // cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: OBS on undefined layer " <<" is skipped... " <<endl; 
        continue;
      }
      frCoord x = round(geometry->getLayerMinSpacing(i) * parser->tech->getDBUPerUU());
      if (enableOutput) {
         cout << __FILE__ << ":" << __LINE__ << ": " <<"      MINSPACING " <<x * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
      }
    } else if (geometry->itemType(i) == lefiGeomLayerRuleWidthE) {
      if (layerNum == -1) {
        // cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: OBS on undefined layer " <<" is skipped... " <<endl; 
        continue;
      }
      frCoord x = round(geometry->getLayerRuleWidth(i) * parser->tech->getDBUPerUU());
      if (enableOutput) {
         cout << __FILE__ << ":" << __LINE__ << ": " <<"      DESIGNRULEWIDTH " <<x * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
      }
    } else {
      if (VERBOSE > -1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported lefiGeometries in getLefObs" <<endl;
      }
      continue;
    }
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"  END" <<endl;
  }
  blkIn->setPin(std::move(pinIn));
  parser->tmpBlock->addBlockage(std::move(blkIn));
  return 0;
}

int io::Parser::Callbacks::getLefString(lefrCallbackType_e type, const char* str, lefiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  io::Parser* parser = (io::Parser*) data;
  if (type == lefrMacroBeginCbkType) {
    auto &tmpBlock = parser->tmpBlock;
    tmpBlock = make_unique<frBlock>(string(str));
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"MACRO " <<tmpBlock->getName() <<endl;
    }
  } else if (type == lefrMacroEndCbkType) {
    auto &tmpBlock = parser->tmpBlock;
    tmpBlock->setId(parser->numRefBlocks + 1);
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"END " <<tmpBlock->getName() <<" " <<parser->numRefBlocks + 1 <<endl;
    }
    parser->design->addRefBlock(std::move(parser->tmpBlock));
    parser->numRefBlocks++;
    parser->numTerms     = 0;
    parser->numBlockages = 0;
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not supported!" <<endl;
    // exit(2);
  }
  return 0;
}

int io::Parser::Callbacks::getLefUnits(lefrCallbackType_e type, lefiUnits* units, lefiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  io::Parser* parser = (io::Parser*) data;
  parser->tech->setDBUPerUU(frUInt4(units->databaseNumber()));
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"DATABASE MICRONS " <<parser->tech->getDBUPerUU() <<endl;
  }
  return 0;
}

int io::Parser::Callbacks::getLefUseMinSpacing(lefrCallbackType_e type, lefiUseMinSpacing* spacing, lefiUserData data) {
  bool enableOutput = true;
  if (!strcmp(spacing->lefiUseMinSpacing::name(), "OBS")) {
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " << "USEMINSPACING OBS";
    }
    if (spacing->lefiUseMinSpacing::value()) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << " ON";
      }
      USEMINSPACING_OBS = true;
    } else {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " << " OFF";
      }
      USEMINSPACING_OBS = false;
    }
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " << endl;
    }
  }
  return 0;
}

int io::Parser::Callbacks::getLefManufacturingGrid(lefrCallbackType_e type, double number, lefiUserData data) {
  //bool enableOutput = true;
  bool enableOutput = false;
  io::Parser* parser = (io::Parser*) data;
  parser->tech->setManufacturingGrid(frUInt4(round(number * parser->tech->getDBUPerUU())));
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"MANUFACTURINGGRID " <<number <<endl;
  }
  return 0;
}

int io::Parser::Callbacks::getLefVias(lefrCallbackType_e type, lefiVia* via, lefiUserData data) {
  bool enableOutput = false;
  // bool enableOutput = true;
  if (type != lefrViaCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrViaCbkType!" <<endl;
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"VIA " <<via->name();
    if (via->hasDefault()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" DEFAULT";
    }
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  }
  if (via->numLayers() != 3) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported via" <<endl;
    }
    exit(1);
  }
  io::Parser* parser = (io::Parser*) data;
  map<frLayerNum, int> lNum2Int;
  for (int i = 0; i < via->numLayers(); ++i) {
    if (parser->tech->name2layer.find(via->layerName(i)) == parser->tech->name2layer.end()) {
      if (VERBOSE > -1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: layer " <<via->layerName(i) <<" is skipiped for " <<via->name() <<endl;
      }
      return 0;
    }
    lNum2Int[parser->tech->name2layer.at(via->layerName(i))->getLayerNum()] = i;
  }
  //for (auto &m: lNum2Int) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<"print " <<m.first <<" " <<m.second <<endl;
  //}
  if (lNum2Int.begin()->first + 2 != (--lNum2Int.end())->first) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: non-consecutive layers" <<endl;
    }
    exit(1);
  }

  auto viaDef = make_unique<frViaDef>(via->name());
  if (via->hasDefault()) {
    viaDef->setDefault(true);
  }
  int cnt = 0;
  for (auto &m: lNum2Int) {
    int i = m.second;
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  LAYER " <<via->layerName(i) <<" ;" <<endl;
    }
    auto layerNum = m.first;
    for (int j = 0; j < via->numRects(i); ++j) {
      frCoord xl = round(via->xl(i, j) * parser->tech->getDBUPerUU());
      frCoord yl = round(via->yl(i, j) * parser->tech->getDBUPerUU());
      frCoord xh = round(via->xh(i, j) * parser->tech->getDBUPerUU());
      frCoord yh = round(via->yh(i, j) * parser->tech->getDBUPerUU());
      unique_ptr<frRect> pinFig = make_unique<frRect>();
      pinFig->setBBox(frBox(xl, yl, xh, yh));
      pinFig->setLayerNum(layerNum);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    RECT "   <<xl * 1.0 / parser->tech->getDBUPerUU() <<" " 
                             <<yl * 1.0 / parser->tech->getDBUPerUU() <<" " 
                             <<xh * 1.0 / parser->tech->getDBUPerUU() <<" " 
                             <<yh * 1.0 / parser->tech->getDBUPerUU() <<" ;" <<endl;
      }
      switch(cnt) {
        case 0 :
          viaDef->addLayer1Fig(std::move(pinFig));
          break;
        case 1 :
          viaDef->addCutFig(std::move(pinFig));
          break;
        default:
          viaDef->addLayer2Fig(std::move(pinFig));
          break;
      }
    }
    for (int j = 0; j < via->numPolygons(i); ++j) {
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    POLYGON"; 
      }
      vector<frPoint> tmpPoints;
      for (int k = 0; k < via->getPolygon(i, j).numPoints; k++) {
        frCoord x = round(via->getPolygon(i, j).x[k] * parser->tech->getDBUPerUU());
        frCoord y = round(via->getPolygon(i, j).y[k] * parser->tech->getDBUPerUU());
        tmpPoints.push_back(frPoint(x, y));
        if (enableOutput) {
           cout << __FILE__ << ":" << __LINE__ << ": " <<" " <<x * 1.0 / parser->tech->getDBUPerUU() <<" " 
                      <<y * 1.0 / parser->tech->getDBUPerUU();
        }
      }
      unique_ptr<frPolygon> pinFig = make_unique<frPolygon>();
      pinFig->setPoints(tmpPoints);
      pinFig->setLayerNum(layerNum);
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<" ;" <<endl;
      }
      switch(cnt) {
        case 0 :
          viaDef->addLayer1Fig(std::move(pinFig));
          break;
        case 1 :
          viaDef->addCutFig(std::move(pinFig));
          break;
        default:
          viaDef->addLayer2Fig(std::move(pinFig));
          break;
      }
    }
    cnt++;
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"END " <<via->name() <<endl;
  }

  // add via class information
  auto cutLayerNum = viaDef->getCutLayerNum();
  auto cutLayer    = parser->tech->getLayer(cutLayerNum);
  int cutClassIdx = -1;
  frLef58CutClass *cutClass = nullptr;

  for (auto &cutFig: viaDef->getCutFigs()) {
    if (cutFig->typeId() == frcRect) {
      frBox box;
      cutFig->getBBox(box);
      auto width  = box.width();
      auto length = box.length();
      cutClassIdx = cutLayer->getCutClassIdx(width, length);
      if (cutClassIdx != -1) {
        cutClass = cutLayer->getCutClass(cutClassIdx);
        break;
      }
    }
  }

  if (cutClass) {
    viaDef->setCutClass(cutClass);
    viaDef->setCutClassIdx(cutClassIdx);
  }

  parser->tech->addVia(std::move(viaDef));
  return 0;
}

int io::Parser::Callbacks::getLefViaRules(lefrCallbackType_e type, lefiViaRule* viaRule, lefiUserData data) {
  bool enableOutput = false;
  //bool enableOutput = true;
  if (type != lefrViaRuleCbkType) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Type is not lefrViaRuleCbkType!" <<endl;
    // exit(1);
  }
  if (enableOutput) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"VIARULE " <<viaRule->name();
    if (viaRule->hasGenerate()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" GENERATE";
    } else {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLefViaRules does not support non-generate rules" <<endl;
      exit(1);
    }
    if (viaRule->hasDefault()) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<" DEFAULT";
    }
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  }
  if (viaRule->numLayers() != 3) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: unsupported via" <<endl;
    }
    exit(1);
  }
  io::Parser* parser = (io::Parser*) data;
  map<frLayerNum, int> lNum2Int;
  for (int i = 0; i < viaRule->numLayers(); ++i) {
    auto viaRuleLayer = viaRule->layer(i);
    if (parser->tech->name2layer.find(viaRuleLayer->name()) == parser->tech->name2layer.end()) {
      if (VERBOSE > -1) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Warning: layer " <<viaRuleLayer->name() <<" is skipiped for " <<viaRule->name() <<endl;
      }
      return 0;
    }
    lNum2Int[parser->tech->name2layer.at(viaRuleLayer->name())->getLayerNum()] = i;
  }
  if (lNum2Int.begin()->first + 2!= (--lNum2Int.end())->first) {
    if (VERBOSE > -1) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: non-consecutive layers" <<endl;
    }
    exit(1);
  }

  if (!viaRule->hasGenerate()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getLefViaRules does not support non-generate rules" <<endl;
    exit(1);
  }
  auto viaRuleGen = make_unique<frViaRuleGenerate>(viaRule->name());
  if (viaRule->hasDefault()) {
    viaRuleGen->setDefault(1);
  }
  int cnt = 0;
  for (auto &[lNum, i]: lNum2Int) {
    auto viaRuleLayer = viaRule->layer(i);
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"  LAYER " <<viaRuleLayer->name() <<" ;" <<endl;
    }
    if (viaRuleLayer->hasEnclosure()) {
      frCoord x = round(viaRuleLayer->enclosureOverhang1() * parser->tech->getDBUPerUU());
      frCoord y = round(viaRuleLayer->enclosureOverhang2() * parser->tech->getDBUPerUU());
      frPoint enc(x, y);
      switch(cnt) {
        case 0:
          viaRuleGen->setLayer1Enc(enc);
          break;
        case 1:
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getViaRuleGenerates cutLayer cannot have overhands" <<endl;
          break;
        default:
          viaRuleGen->setLayer2Enc(enc);
          break;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    ENCLOSURE " <<viaRuleLayer->enclosureOverhang1() <<" " <<viaRuleLayer->enclosureOverhang1() <<" ;" <<endl;
      }
    }
    if (viaRuleLayer->hasRect()) {
      frCoord xl = round(viaRuleLayer->xl() * parser->tech->getDBUPerUU());
      frCoord yl = round(viaRuleLayer->yl() * parser->tech->getDBUPerUU());
      frCoord xh = round(viaRuleLayer->xh() * parser->tech->getDBUPerUU());
      frCoord yh = round(viaRuleLayer->yh() * parser->tech->getDBUPerUU());
      frBox box(xl, yl, xh, yh);
      switch(cnt) {
        case 0:
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getViaRuleGenerates botLayer cannot have rect" <<endl;
          break;
        case 1:
          viaRuleGen->setCutRect(box);
          break;
        default:
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getViaRuleGenerates topLayer cannot have rect" <<endl;
          break;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    RECT " <<viaRuleLayer->xl() <<" " 
                           <<viaRuleLayer->yl() <<" " 
                           <<viaRuleLayer->xh() <<" " 
                           <<viaRuleLayer->yh() 
                           <<" ;" <<endl;
      }
    }
    if (viaRuleLayer->hasSpacing()) {
      frCoord x = round(viaRuleLayer->spacingStepX() * parser->tech->getDBUPerUU());
      frCoord y = round(viaRuleLayer->spacingStepY() * parser->tech->getDBUPerUU());
      frPoint pt(x, y);
      switch(cnt) {
        case 0:
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getViaRuleGenerates botLayer cannot have spacing" <<endl;
          break;
        case 1:
          viaRuleGen->setCutSpacing(pt);
          break;
        default:
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: getViaRuleGenerates topLayer cannot have spacing" <<endl;
          break;
      }
      if (enableOutput) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"    SPACING " <<viaRuleLayer->spacingStepX() <<" BY " <<viaRuleLayer->spacingStepY() <<" ;" <<endl;
      }
    }
    cnt++;
  }

  parser->tech->addViaRuleGenerate(std::move(viaRuleGen));
  return 0;
}

void io::Parser::readLef() {
  ProfileTask profile("IO:readLef");
  FILE* f;
  int res;

  lefrInitSession(1);

  lefrSetUserData ((lefiUserData)this);

  lefrSetMacroCbk(Callbacks::getLefMacros);
  lefrSetMacroBeginCbk(Callbacks::getLefString);
  lefrSetMacroEndCbk(Callbacks::getLefString);
  lefrSetUnitsCbk(Callbacks::getLefUnits);
  lefrSetManufacturingCbk(Callbacks::getLefManufacturingGrid);
  lefrSetUseMinSpacingCbk(Callbacks::getLefUseMinSpacing);
  lefrSetPinCbk(Callbacks::getLefPins);
  lefrSetObstructionCbk(Callbacks::getLefObs);
  lefrSetLayerCbk(Callbacks::getLefLayers);
  lefrSetViaCbk(Callbacks::getLefVias);
  lefrSetViaRuleCbk(Callbacks::getLefViaRules);

  if ((f = fopen(LEF_FILE.c_str(),"r")) == 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Couldn't open lef file" <<endl;
    exit(2);
  }

  res = lefrRead(f, LEF_FILE.c_str(), (lefiUserData)this);
  if (res != 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"LEF parser returns an error!" <<endl;
    exit(2);
  }
  fclose(f);

  lefrClear();
}

void io::Parser::readLefDef() {
  bool enableOutput = false;
  // bool enableOutput = true;

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"reading lef ..." <<endl;
  }

  readLef();

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"units:       " <<tech->getDBUPerUU()      <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#layers:     " <<tech->layers.size()      <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#macros:     " <<design->refBlocks.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#vias:       " <<tech->vias.size()        <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#viarulegen: " <<tech->viaRuleGenerates.size() <<endl;
  }

  auto numLefVia = tech->vias.size();

  //tech->printAllConstraints();
  
  if (enableOutput) {
    //design->printAllMacros();
    // printAllLayers();
    //tech->printAllVias();
    //printLayerMaps();
  }

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"reading def ..." <<endl;
  }

  readDef();



  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
    frBox dieBox;
    design->getTopBlock()->getBoundaryBBox(dieBox);
    cout << __FILE__ << ":" << __LINE__ << ": " <<"design:      " <<design->getTopBlock()->getName()    <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"die area:    " <<dieBox                              <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"trackPts:    " <<design->getTopBlock()->getTrackPatterns().size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"defvias:     " <<tech->vias.size() - numLefVia       <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#components: " <<design->getTopBlock()->insts.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#terminals:  " <<design->getTopBlock()->terms.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#snets:      " <<design->getTopBlock()->snets.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#nets:       " <<design->getTopBlock()->nets.size()  <<endl;
    //cout << __FILE__ << ":" << __LINE__ << ": " <<"#pins:       " <<numPins <<endl;
  }
  //cout << __FILE__ << ":" << __LINE__ << ": " <<flush;

  if (enableOutput) {
    //tech->printAllVias();
    //design->printAllComps();
    //printCompMaps();
    //design->printAllTerms();
    //printTermMaps();
    //printAllNets();
    //printAllTrackGens();
    //printAllTrackPatterns();
  }
  //exit(1);

}

void io::Parser::readGuide() {
  ProfileTask profile("IO:readGuide");

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"reading guide ..." <<endl;
  }

  int numGuides = 0;

  string netName = "";
  frNet* net = nullptr;

  ifstream fin(GUIDE_FILE.c_str());
  string line;
  frBox  box;
  frLayerNum layerNum;

  if (fin.is_open()){
    while (fin.good()) {
      getline(fin, line);
      //cout << __FILE__ << ":" << __LINE__ << ": " <<line <<endl <<line.size() <<endl;
      if (line == "(" || line == "") continue;
      if (line == ")") {
        continue;
      }

      stringstream ss(line);
      string word = "";
      vector<string> vLine;
      while (!ss.eof()) {
        ss >>word;
        vLine.push_back(word);
        //cout << __FILE__ << ":" << __LINE__ << ": " <<word <<" ";
      }
      //cout << __FILE__ << ":" << __LINE__ << ": " <<endl;

      if (vLine.size() == 0) {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: reading guide file!" <<endl;
        exit(2);
      } else if (vLine.size() == 1) {
        netName = vLine[0];
        if (design->topBlock->name2net.find(vLine[0]) == design->topBlock->name2net.end()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find net: " <<vLine[0] <<endl;
          exit(2);
        }
        net = design->topBlock->name2net[netName]; 
      } else if (vLine.size() == 5) {
        if (tech->name2layer.find(vLine[4]) == tech->name2layer.end()) {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: cannot find layer: " <<vLine[4] <<endl;
          exit(2);
        }
        layerNum = tech->name2layer[vLine[4]]->getLayerNum();

        if (layerNum < BOTTOM_ROUTING_LAYER && layerNum != VIA_ACCESS_LAYERNUM
            || layerNum > TOP_ROUTING_LAYER) {
          cout << __FILE__ << ":" << __LINE__ << ": " << "Error: guide in net " << netName
               << " uses layer " << vLine[4]
               << " (" << layerNum << ")"
               << " that is outside the allowed routing range "
               << "[" << tech->getLayer(BOTTOM_ROUTING_LAYER)->getName()
               << " (" << BOTTOM_ROUTING_LAYER << "), "
               << tech->getLayer(TOP_ROUTING_LAYER)->getName()
               << " (" << TOP_ROUTING_LAYER << ")]"
               << endl;
          exit(2);
        }

        box.set(stoi(vLine[0]), stoi(vLine[1]), stoi(vLine[2]), stoi(vLine[3]));
        frRect rect;
        rect.setBBox(box);
        rect.setLayerNum(layerNum);
        tmpGuides[net].push_back(rect);
        ++numGuides;
        if (numGuides < 1000000) {
          if (numGuides % 100000 == 0) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"guideIn read " <<numGuides <<" guides" <<endl;
          }
        } else {
          if (numGuides % 1000000 == 0) {
            cout << __FILE__ << ":" << __LINE__ << ": " <<"guideIn read " <<numGuides <<" guides" <<endl;
          }
        }

      } else {
        cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: reading guide file!" <<endl;
        exit(2);
      }
    }
    fin.close();
  } else {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: failed to open guide file" <<endl;
    exit(2);
  }


  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#guides:     " <<numGuides <<endl;
  }

}

void io::Writer::fillConnFigs_net(frNet* net, bool isTA) {
  //bool enableOutput = true;
  bool enableOutput = false;
  auto netName = net->getName();
  if (isTA) {
    for (auto &uGuide: net->getGuides()) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"find guide" <<endl;
      for (auto &uConnFig: uGuide->getRoutes()) {
        auto connFig = uConnFig.get();
        if (connFig->typeId() == frcPathSeg) {
          connFigs[netName].push_back(make_shared<frPathSeg>(*static_cast<frPathSeg*>(connFig)));
        } else if (connFig->typeId() == frcVia) {
          connFigs[netName].push_back(make_shared<frVia>(*static_cast<frVia*>(connFig)));
        } else {
          cout << __FILE__ << ":" << __LINE__ << ": " <<"Error: io::Writer::filliConnFigs does not support this type" <<endl;
        }
      }
    }
  } else {
    if (enableOutput) {
      cout << __FILE__ << ":" << __LINE__ << ": " << netName << ":\n";
    }
    for (auto &shape: net->getShapes()) {
      if (shape->typeId() == frcPathSeg) {
        auto pathSeg = *static_cast<frPathSeg*>(shape.get());
        frPoint start, end;
        pathSeg.getPoints(start, end);

        if (enableOutput) {
          frLayerNum currLayerNum = pathSeg.getLayerNum();
          cout << __FILE__ << ":" << __LINE__ << ": " << "  connfig pathseg (" << start.x() / 2000.0<< ", " << start.y() / 2000.0 
               << ") - (" << end.x() / 2000.0 << ", " << end.y() / 2000.0 << ") " << currLayerNum  <<"\n"; 
        }
        connFigs[netName].push_back(make_shared<frPathSeg>(pathSeg));
      }
    }
    for (auto &via: net->getVias()) {
      connFigs[netName].push_back(make_shared<frVia>(*via));
    }
    for (auto &shape: net->getPatchWires()) {
      auto pwire = static_cast<frPatchWire*>(shape.get());
      connFigs[netName].push_back(make_shared<frPatchWire>(*pwire));
    }
  }
}

void io::Writer::splitVia_helper(frLayerNum layerNum, int isH, frCoord trackLoc, frCoord x, frCoord y, 
  vector< vector< map<frCoord, vector<shared_ptr<frPathSeg> > > > > &mergedPathSegs) {
  if (layerNum >= 0 && layerNum < (int)(getTech()->getLayers().size()) &&
      mergedPathSegs.at(layerNum).at(isH).find(trackLoc) != mergedPathSegs.at(layerNum).at(isH).end()) {
    for (auto &pathSeg: mergedPathSegs.at(layerNum).at(isH).at(trackLoc)) {
      frPoint begin, end;
      pathSeg->getPoints(begin, end);
      if ((isH == 0 && (begin.x() < x) && (end.x() > x)) ||
          (isH == 1 && (begin.y() < y) && (end.y() > y))) {
        frSegStyle style1, style2, style_default;
        pathSeg->getStyle(style1);
        pathSeg->getStyle(style2);
        style_default = getTech()->getLayer(layerNum)->getDefaultSegStyle();
        shared_ptr<frPathSeg> newPathSeg = make_shared<frPathSeg>(*pathSeg);
        pathSeg->setPoints(begin, frPoint(x,y));
        style1.setEndStyle(style_default.getEndStyle(), style_default.getEndExt());
        pathSeg->setStyle(style1);
        newPathSeg->setPoints(frPoint(x,y), end);
        style2.setBeginStyle(style_default.getBeginStyle(), style_default.getBeginExt());
        newPathSeg->setStyle(style2);
        mergedPathSegs.at(layerNum).at(isH).at(trackLoc).push_back(newPathSeg);
        // via can only intersect at most one merged pathseg on one track
        break;
      }
    }
  }
}

// merge pathseg, delete redundant via
void io::Writer::mergeSplitConnFigs(list<shared_ptr<frConnFig> > &connFigs) {
  //if (VERBOSE > 0) {
  //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"merge and split ..." <<endl;
  //}
  // initialzie pathseg and via map
  map < tuple<frLayerNum, bool, frCoord>,
        map<frCoord, vector< tuple<shared_ptr<frPathSeg>, bool> > 
           >
      > pathSegMergeMap;
  map < tuple<frCoord, frCoord, frLayerNum>, shared_ptr<frVia> > viaMergeMap;
  for (auto &connFig: connFigs) {
    if (connFig->typeId() == frcPathSeg) {
      auto pathSeg = dynamic_pointer_cast<frPathSeg>(connFig);
      frPoint begin, end;
      pathSeg->getPoints(begin, end);
      frLayerNum layerNum = pathSeg->getLayerNum();
      if (begin == end) {
        // std::cout << __FILE__ << ":" << __LINE__ << ": " << "Warning: 0 length connfig\n";
        continue; // if segment length = 0, ignore
      } else {
        // std::cout << __FILE__ << ":" << __LINE__ << ": " << "xxx\n";
        bool isH = (begin.x() == end.x()) ? false : true;
        frCoord trackLoc   = isH ? begin.y() : begin.x();
        frCoord beginCoord = isH ? begin.x() : begin.y();
        frCoord endCoord   = isH ? end.x()   : end.y();
        pathSegMergeMap[make_tuple(layerNum, isH, trackLoc)][beginCoord].push_back(make_tuple(pathSeg, true));
        pathSegMergeMap[make_tuple(layerNum, isH, trackLoc)][endCoord].push_back(make_tuple(pathSeg, false));
      }
    } else if (connFig->typeId() == frcVia) {
      auto via = dynamic_pointer_cast<frVia>(connFig);
      auto cutLayerNum = via->getViaDef()->getCutLayerNum();
      frPoint viaPoint;
      via->getOrigin(viaPoint);
      viaMergeMap[make_tuple(viaPoint.x(), viaPoint.y(), cutLayerNum)] = via;
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"found via" <<endl;
    }
  }

  // merge pathSeg
  map<frCoord, vector<shared_ptr<frPathSeg> > > tmp1;
  vector< map<frCoord, vector<shared_ptr<frPathSeg> > > > tmp2(2, tmp1);
  vector< vector< map<frCoord, vector<shared_ptr<frPathSeg> > > > > mergedPathSegs(getTech()->getLayers().size(), tmp2);

  for (auto &it1: pathSegMergeMap) {
    auto layerNum = get<0>(it1.first);
    int  isH = get<1>(it1.first);
    auto trackLoc = get<2>(it1.first);
    bool hasSeg = false;
    int cnt = 0;
    shared_ptr<frPathSeg> newPathSeg;
    frSegStyle style;
    frPoint begin, end;
    for (auto &it2: it1.second) {
      //cout << __FILE__ << ":" << __LINE__ << ": " <<"coord " <<coord <<endl;
      for (auto &pathSegTuple: it2.second) {
        cnt += get<1>(pathSegTuple)? 1 : -1;
      }
      // newPathSeg begin
      if (!hasSeg && cnt > 0) {
        style.setBeginStyle(frcTruncateEndStyle, 0);
        style.setEndStyle(frcTruncateEndStyle, 0);
        newPathSeg = make_shared<frPathSeg>(*(get<0>(*(it2.second.begin()))));
        for (auto &pathSegTuple: it2.second) {
          auto pathSeg = get<0>(pathSegTuple);
          auto isBegin = get<1>(pathSegTuple);
          if (isBegin) {
            pathSeg->getPoints(begin, end);
            frSegStyle tmpStyle;
            pathSeg->getStyle(tmpStyle);
            if (tmpStyle.getBeginExt() > style.getBeginExt()) {
              style.setBeginStyle(tmpStyle.getBeginStyle(), tmpStyle.getBeginExt());
            }
          }
        }
        newPathSeg->setStyle(style);
        hasSeg = true;
      // newPathSeg end
      } else if (hasSeg && cnt == 0) {
        newPathSeg->getPoints(begin, end);
        for (auto &pathSegTuple: it2.second) {
          auto pathSeg = get<0>(pathSegTuple);
          auto isBegin = get<1>(pathSegTuple);
          if (!isBegin) {
            frPoint tmp;
            pathSeg->getPoints(tmp, end);
            frSegStyle tmpStyle;
            pathSeg->getStyle(tmpStyle);
            if (tmpStyle.getEndExt() > style.getEndExt()) {
              style.setEndStyle(tmpStyle.getEndStyle(), tmpStyle.getEndExt());
            }
          }
        }
        newPathSeg->setPoints(begin, end);
        newPathSeg->setStyle(style);
        hasSeg = false;
        (mergedPathSegs.at(layerNum).at(isH))[trackLoc].push_back(newPathSeg);
      }
    }
  }

  // split pathseg from via
  // mergedPathSegs[layerNum][isHorizontal] is a map<frCoord, vector<shared_ptr<frPathSeg> > >
  //map < tuple<frCoord, frCoord, frLayerNum>, shared_ptr<frVia> > viaMergeMap;
  for (auto &it1: viaMergeMap) {
    auto x           = get<0>(it1.first);
    auto y           = get<1>(it1.first);
    auto cutLayerNum = get<2>(it1.first);
    frCoord trackLoc;

    auto layerNum = cutLayerNum - 1;
    int  isH      = 1;
    trackLoc = (isH == 1) ? y : x;
    splitVia_helper(layerNum, isH, trackLoc, x, y, mergedPathSegs);

    layerNum = cutLayerNum - 1;
    isH = 0;
    trackLoc = (isH == 1) ? y : x;
    splitVia_helper(layerNum, isH, trackLoc, x, y, mergedPathSegs);

    layerNum = cutLayerNum + 1;
    trackLoc = (isH == 1) ? y : x;
    splitVia_helper(layerNum, isH, trackLoc, x, y, mergedPathSegs);

    layerNum = cutLayerNum + 1;
    isH = 0;
    trackLoc = (isH == 1) ? y : x;
    splitVia_helper(layerNum, isH, trackLoc, x, y, mergedPathSegs);
  }

  // split intersecting pathSegs
  for (auto &it1: mergedPathSegs) {
    // vertical for mapIt1
    for (auto &mapIt1: it1.at(0)) {
      // horizontal for mapIt2
      for (auto &mapIt2: it1.at(1)) {
        // at most split once
        // seg1 is vertical
        for (auto &seg1: mapIt1.second) {
          bool skip = false;
          // seg2 is horizontal
          frPoint seg1Begin, seg1End;
          seg1->getPoints(seg1Begin, seg1End);
          for (auto &seg2: mapIt2.second) {
            frPoint seg2Begin, seg2End;
            seg2->getPoints(seg2Begin, seg2End);
            bool pushNewSeg1 = false;
            bool pushNewSeg2 = false;
            shared_ptr<frPathSeg> newSeg1;
            shared_ptr<frPathSeg> newSeg2;
            // check whether seg1 needs to be split, break seg1
            if (seg2Begin.y() > seg1Begin.y() && seg2Begin.y() < seg1End.y()) {
              pushNewSeg1 = true;
              newSeg1 = make_shared<frPathSeg>(*seg1);
              // modify seg1
              seg1->setPoints(seg1Begin, frPoint(seg1End.x(), seg2End.y()));
              // modify newSeg1
              newSeg1->setPoints(frPoint(seg1End.x(), seg2Begin.y()), seg1End);
              // modify endstyle
              auto layerNum = seg1->getLayerNum();
              frSegStyle tmpStyle1;
              frSegStyle tmpStyle2;
              frSegStyle style_default;
              seg1->getStyle(tmpStyle1);
              seg1->getStyle(tmpStyle2);
              style_default = getTech()->getLayer(layerNum)->getDefaultSegStyle();
              tmpStyle1.setEndStyle(frcExtendEndStyle, style_default.getEndExt());
              seg1->setStyle(tmpStyle1);
              tmpStyle2.setBeginStyle(frcExtendEndStyle, style_default.getBeginExt());
              newSeg1->setStyle(tmpStyle2);
            }
            // check whether seg2 needs to be split, break seg2
            if (seg1Begin.x() > seg2Begin.x() && seg1Begin.x() < seg2End.x()) {
              pushNewSeg2 = true;
              newSeg2 = make_shared<frPathSeg>(*seg1);
              // modify seg2
              seg2->setPoints(seg2Begin, frPoint(seg1End.x(), seg2End.y()));
              // modify newSeg2
              newSeg2->setPoints(frPoint(seg1End.x(), seg2Begin.y()), seg2End);
              // modify endstyle
              auto layerNum = seg2->getLayerNum();
              frSegStyle tmpStyle1;
              frSegStyle tmpStyle2;
              frSegStyle style_default;
              seg2->getStyle(tmpStyle1);
              seg2->getStyle(tmpStyle2);
              style_default = getTech()->getLayer(layerNum)->getDefaultSegStyle();
              tmpStyle1.setEndStyle(frcExtendEndStyle, style_default.getEndExt());
              seg2->setStyle(tmpStyle1);
              tmpStyle2.setBeginStyle(frcExtendEndStyle, style_default.getBeginExt());
              newSeg2->setStyle(tmpStyle2);
            }
            if (pushNewSeg1) {
              mapIt1.second.push_back(newSeg1);
            }
            if (pushNewSeg2) {
              mapIt2.second.push_back(newSeg2);
            }
            if (pushNewSeg1 || pushNewSeg2) {
              skip = true;
              break;
            }
            //cout << __FILE__ << ":" << __LINE__ << ": " <<"found" <<endl;
          }
          if (skip) break;
        }
      }
    }
  }


  // write back pathseg
  connFigs.clear();
  for (auto &it1: mergedPathSegs) {
    for (auto &it2: it1) {
      for (auto &it3: it2) {
        for (auto &it4: it3.second) {
          connFigs.push_back(it4);
        }
      }
    }
  }

  // write back via
  //map < tuple<frCoord, frCoord, frLayerNum>, shared_ptr<frVia> > viaMergeMap;
  for (auto &it: viaMergeMap) {
    connFigs.push_back(it.second);
  }

}

void io::Writer::fillViaDefs() {
  viaDefs.clear();
  for (auto &uViaDef: getDesign()->getTech()->getVias()) {
    auto viaDef = uViaDef.get();
    if (viaDef->isAddedByRouter()) {
      viaDefs.push_back(viaDef);
    }
  }
}

void io::Writer::fillConnFigs(bool isTA) {
  connFigs.clear();
  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"post processing ..." <<endl;
  }
  for (auto &net: getDesign()->getTopBlock()->getNets()) {
    fillConnFigs_net(net.get(), isTA);
  }
  if (isTA) {
    for (auto &it: connFigs) {
      mergeSplitConnFigs(it.second);
    }
  }
}

void io::Writer::writeFromTA() {
  ProfileTask profile("IO:writeFromTA");
  if (OUTTA_FILE == string("")) {
    if (VERBOSE > 0) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Waring: no output def specified, skipped writing track assignment def" <<endl;
    }
  } else {
    //if (VERBOSE > 0) {
    //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"start writing track assignment def" <<endl;
    //}
    fillConnFigs(true);
    fillViaDefs();
    writeDef(true);
  }
}

void io::Writer::writeFromDR(const string &str) {
  ProfileTask profile("IO:writeFromDR");
  if (OUT_FILE == string("")) {
    if (VERBOSE > 0) {
      cout << __FILE__ << ":" << __LINE__ << ": " <<"Waring: no output def specified, skipped writing routed def" <<endl;
    }
  } else {
    //if (VERBOSE > 0) {
    //  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"start writing routed def" <<endl;
    //}
  }
  fillConnFigs(false);
  fillViaDefs();
  writeDef(false, str);
}

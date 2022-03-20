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
#include <sstream>
#include <chrono>
#include "frProfileTask.h"
#include "FlexPA.h"
#include "db/infra/frTime.h"
#include "gc/FlexGC.h"

using namespace std;
using namespace fr;

void FlexPA::init() {
  ProfileTask profile("PA:init");
  initViaRawPriority();
  initTrackCoords();

  initUniqueInstance();
  initPinAccess();
}

void FlexPA::prep() {
  ProfileTask profile("PA:prep");
  using namespace std::chrono;
  high_resolution_clock::time_point t0 = high_resolution_clock::now();
  prepPoint();
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  revertAccessPoints();
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  prepPattern();
  high_resolution_clock::time_point t3 = high_resolution_clock::now();

  duration<double> time_span1 = duration_cast<duration<double>>(t1 - t0);
  duration<double> time_span2 = duration_cast<duration<double>>(t3 - t2);
  cout << __FILE__ << ":" << __LINE__ << ": " << "Expt1 runtime (pin-level access point gen): " << time_span1.count() << endl;
  cout << __FILE__ << ":" << __LINE__ << ": " << "Expt2 runtime (design-level access pattern gen): " << time_span2.count() << endl;
}

int FlexPA::main() {
  ProfileTask profile("PA:main");

  //bool enableOutput = true;
  frTime t;
  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<endl <<"start pin access" <<endl;
  }

  init();
  prep();

  int stdCellPinCnt = 0;
  for (auto &inst: getDesign()->getTopBlock()->getInsts()) {
    if (inst->getRefBlock()->getMacroClass() != MacroClassEnum::CORE) {
      continue;
    }
    for (auto &instTerm: inst->getInstTerms()) {
      if (isSkipInstTerm(instTerm.get())) {
        continue;
      }
      if (instTerm->hasNet()) {
        stdCellPinCnt++;
      }
    }
  }

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#scanned instances     = " <<inst2unique.size()     <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#unique  instances     = " <<uniqueInstances.size() <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#stdCellGenAp          = " <<stdCellPinGenApCnt           <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#stdCellValidPlanarAp  = " <<stdCellPinValidPlanarApCnt   <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#stdCellValidViaAp     = " <<stdCellPinValidViaApCnt      <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#stdCellPinNoAp        = " <<stdCellPinNoApCnt            <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#stdCellPinCnt         = " <<stdCellPinCnt                <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#instTermValidViaApCnt = " <<instTermValidViaApCnt        <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#macroGenAp            = " <<macroCellPinGenApCnt         <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#macroValidPlanarAp    = " <<macroCellPinValidPlanarApCnt <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#macroValidViaAp       = " <<macroCellPinValidViaApCnt    <<endl;
    cout << __FILE__ << ":" << __LINE__ << ": " <<"#macroNoAp             = " <<macroCellPinNoApCnt          <<endl;
  }

  if (VERBOSE > 0) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"complete pin access" <<endl;
    t.print();
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  }
  return 0;
}

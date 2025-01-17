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

//#include "FlexRoute.h"
#include <iostream>
#include <fstream>
#include "global.h"
#include "frDesign.h"
using namespace std;
using namespace fr;

void frDesign::printAllComps() {
  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"print all components: ";
  for (auto &m: topBlock->getInsts()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<*m <<endl;
  }
}

void frDesign::printAllMacros() {
  cout << __FILE__ << ":" << __LINE__ << ": " <<endl;
  cout << __FILE__ << ":" << __LINE__ << ": " <<"print all macros: " <<endl;
  for (auto &m: refBlocks) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<*(m) <<endl;
  }
}

void frDesign::printAllTerms() {
  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"print all terminals: ";
  for (auto &m: topBlock->getTerms()) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<*m <<endl;
  }
}

void frTechObject::printAllVias() {
  cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<"print all vias: " <<endl;
  for (auto &m: vias) {
    cout << __FILE__ << ":" << __LINE__ << ": " <<endl <<*(m) <<endl;
  }
}


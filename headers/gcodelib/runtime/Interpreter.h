/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#ifndef GCODELIB_RUNTIME_INTERPRETER_H_
#define GCODELIB_RUNTIME_INTERPRETER_H_

#include "gcodelib/runtime/IR.h"
#include "gcodelib/runtime/Runtime.h"
#include <stack>
#include <map>

namespace GCodeLib::Runtime {

  class GCodeInterpreter {
   public:
    GCodeInterpreter(GCodeIRModule &);
    virtual ~GCodeInterpreter() = default;
    virtual void execute();
   protected:
    GCodeFunctionScope &getFunctions();
    GCodeRuntimeState &getState();
    void interpret();
    void stop();

    virtual void syscall(GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &) = 0;
    virtual GCodeVariableScope &getSystemScope() = 0;
    
    GCodeIRModule &module;
    std::optional<GCodeRuntimeState> state;
    GCodeFunctionScope functions;
    GCodeRuntimeConfig config;
  };
}

#endif
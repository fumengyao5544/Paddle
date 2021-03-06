//   Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "paddle/fluid/framework/details/computation_op_handle.h"

#include <string>

namespace paddle {
namespace framework {
namespace details {
ComputationOpHandle::ComputationOpHandle(const OpDesc &op_desc, Scope *scope,
                                         platform::Place place)
    : op_(framework::OpRegistry::CreateOp(op_desc)),
      scope_(scope),
      place_(place) {}

void ComputationOpHandle::RunImpl() {
  auto *cur_ctx = dev_ctxes_[place_];
  for (auto *in : inputs_) {
    bool need_wait = in->generated_op_ &&
                     in->generated_op_->DeviceContext(place_) != cur_ctx;
    if (need_wait) {
      in->generated_op_->Wait(cur_ctx);
    }
  }

  this->RunAndRecordEvent([this] {
    op_->Run(*scope_->FindVar(kLocalExecScopeName)->Get<Scope *>(), place_);
  });
}

std::string ComputationOpHandle::Name() const { return op_->Type(); }
}  // namespace details
}  // namespace framework
}  // namespace paddle

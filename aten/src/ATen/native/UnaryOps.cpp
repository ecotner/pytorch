#include "ATen/ATen.h"
#include "ATen/Dispatch.h"
#include "ATen/ExpandUtils.h"
#include "ATen/NativeFunctions.h"
#include "ATen/WrapDimUtils.h"

#include "ATen/CPUApplyUtils.h"
#include "ATen/Parallel.h"
#include "ATen/native/cpu/UnaryOpsKernel.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

#include <map>

// NOTE:
// YOU ARE NOT OBLIGED TO USE THESE MACROS
// If you're writing something more specialized, please don't try to make them
// work for your case, but just write something new instead.

namespace at {
namespace native {

Tensor& fill_(Tensor& self, Scalar value) {
  return self._fill_(value);
}

Tensor& fill_(Tensor& self, const Tensor& value) {
  return self._fill_(value);
}

// NB: If you use this macro, you may also need to add a CUDA forwarding
// stub in CUDAUnaryOps

#define IMPLEMENT_UNARY_OP_VEC(op, opfn)                        \
  Tensor op(const Tensor& self) {                               \
    Tensor result = self.type().tensor();                       \
    return at::op##_out(result, self);                          \
  }                                                             \
  Tensor& _##op##__cpu(Tensor& self_) {                         \
    if (self_.numel() > 0) {                                    \
      Tensor self = sort_strides(self_);                        \
      if (self.is_contiguous()) {                               \
        op##Impl(self, self);                                   \
      } else {                                                  \
        AT_DISPATCH_FLOATING_TYPES(self.type(), op, [&] {       \
          CPU_tensor_parallel_apply1<scalar_t>(                 \
              self, [](scalar_t& y) { y = opfn(y); }, 2048);    \
        });                                                     \
      }                                                         \
    }                                                           \
    return self_;                                               \
  }                                                             \
  Tensor& _##op##_out_cpu(Tensor& result, const Tensor& self) { \
    result.resize_(self.sizes());                               \
    if (result.numel() > 0) {                                   \
      if (result.is_contiguous() && self.is_contiguous()) {     \
        op##Impl(result, self);                                 \
      } else {                                                  \
        AT_DISPATCH_FLOATING_TYPES(self.type(), op, [&] {       \
          CPU_tensor_parallel_apply2<scalar_t, scalar_t>(       \
              result,                                           \
              self,                                             \
              [](scalar_t& y, scalar_t& x) { y = opfn(x); },    \
              2048);                                            \
        });                                                     \
      }                                                         \
    }                                                           \
    return result;                                              \
  }

IMPLEMENT_UNARY_OP_VEC(abs, std::abs)
IMPLEMENT_UNARY_OP_VEC(acos, std::acos)
IMPLEMENT_UNARY_OP_VEC(asin, std::asin)
IMPLEMENT_UNARY_OP_VEC(atan, std::atan)
IMPLEMENT_UNARY_OP_VEC(ceil, std::ceil)
IMPLEMENT_UNARY_OP_VEC(cos, std::cos)
IMPLEMENT_UNARY_OP_VEC(cosh, std::cosh)
IMPLEMENT_UNARY_OP_VEC(erf, std::erf)
IMPLEMENT_UNARY_OP_VEC(exp, std::exp)
IMPLEMENT_UNARY_OP_VEC(expm1, std::expm1)
IMPLEMENT_UNARY_OP_VEC(floor, std::floor)
IMPLEMENT_UNARY_OP_VEC(log, std::log)
IMPLEMENT_UNARY_OP_VEC(log10, std::log10)
IMPLEMENT_UNARY_OP_VEC(log1p, std::log1p)
IMPLEMENT_UNARY_OP_VEC(log2, std::log2)
IMPLEMENT_UNARY_OP_VEC(round, std::round)
IMPLEMENT_UNARY_OP_VEC(rsqrt, 1 / std::sqrt)
IMPLEMENT_UNARY_OP_VEC(sin, std::sin)
IMPLEMENT_UNARY_OP_VEC(sinh, std::sinh)
IMPLEMENT_UNARY_OP_VEC(sqrt, std::sqrt)
IMPLEMENT_UNARY_OP_VEC(tan, std::tan)
IMPLEMENT_UNARY_OP_VEC(tanh, std::tanh)
IMPLEMENT_UNARY_OP_VEC(trunc, std::trunc)

}
} // namespace at

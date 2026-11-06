/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "../src/omega_attn_host_policy.h"
#ifndef HOST
#define HOST(x) om_##x
#endif
uint64_t HOST(plan)(uint64_t q[32],uint32_t op){return ah_policy(q,op);}
size_t HOST(check)(uint32_t op,const uint32_t*w,const uint32_t*x,size_t n){return ah_check(op,w,x,n);}
uint64_t HOST(clock)(uint64_t s,uint64_t n){return ah_clock(s,n);}

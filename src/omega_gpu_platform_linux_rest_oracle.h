/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Migration oracle only; production must not include this header. */
#ifndef OMEGA_GPU_PLATFORM_LINUX_REST_ORACLE_H
#define OMEGA_GPU_PLATFORM_LINUX_REST_ORACLE_H
#include <stdint.h>
static uint64_t plat_rest_c(uint64_t mode, const uint64_t m[16], uint64_t q[16]) {
    switch (mode) {
    case 0: {
        uint64_t pages=(m[0]+4095ull)&~4095ull, cursor=m[1];
        if(pages<4096ull || pages>(1ull<<40)) return 0;
        if(cursor<65536ull || cursor>(1ull<<40)-pages) cursor=268435456ull;
        if(cursor>(1ull<<40)-pages) return 0;
        q[0]=pages; q[1]=cursor; q[2]=cursor+pages; return 1;
    }
    case 1: {
        const unsigned first = m[1]=='/' ? 2 : 1;
        const unsigned need = first==2 ? 7 : 6;
        const char *prefix="memfd:";
        if(m[0]<need) return 0;
        for(unsigned i=0;i<6;++i) if(m[first+i]!=(unsigned char)prefix[i]) return 0;
        return 1;
    }
    case 2:
        if(m[0]>=m[5] || m[1]==0) return 5;
        if(!m[2] || m[3]!=m[1]) return 6;
        if(m[4]!=2 && m[4]!=3 && m[4]!=4) return 5;
        return 0;
    case 3: return m[0]*1000000000ull+m[1];
    case 4: q[0]=m[0]/1000u; q[1]=(m[0]%1000u)*1000000ull; return 0;
    case 5: return m[0]>=m[1] ? 1 : 0;
    case 6:
        for(unsigned i=0;i<3 && i+1<m[0];++i) if(!m[i+1]) return i+1;
        return 0;
    case 7: return m[0]!=0 ? 1 : m[1]!=0 ? 11 : 0;
    case 8: return m[0] ? 5 : 0;
    case 9: return m[0]/1000000ull;
    case 10: q[0]=m[0]*4; q[1]=(m[0]+m[1])*4; q[2]=m[0]+m[1]; return 0;
    case 11: return (uint32_t)(m[0]+1);
    case 12: return m[0] && m[1] && (int64_t)m[2]>=0 && (m[3]&m[4]) && !(m[3]&m[5]);
    case 13: return m[0] ? m[0] : 1;
    default: return 0;
    }
}
#endif

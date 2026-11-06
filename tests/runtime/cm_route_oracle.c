/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Exercise the retained route oracle with canonical Omega computation. */
#define CUDAPP_MIGRATION_PENDING_C_CM_ROUTE 1
#include "../../src/runtime/rx_costmodel.c"
#include <assert.h>
#include <stdio.h>
int main(void) {
    const uint64_t values[]={0,1,63,64,65,UINT32_MAX-1,UINT32_MAX,
                             (uint64_t)UINT32_MAX+1,UINT64_MAX-1,UINT64_MAX};
    for(size_t i=0;i<sizeof values/sizeof values[0];++i)
        for(size_t j=0;j<sizeof values/sizeof values[0];++j) {
            assert(cm_route2(2,values[i],values[j])==(values[i]<values[j]));
            assert(cm_route2(3,values[i],values[j])==values[i]+values[j]);
            assert(cm_route2(4,values[i],values[j])==values[i]-values[j]);
        }
    uint8_t data[130]={0}, got[32], expected[32];
    for(size_t i=0;i<sizeof data;++i)data[i]=(uint8_t)i;
    for(size_t n=0;n<=sizeof data;++n) {
        cm_sha256(data,n,got); sha256_hash(data,n,expected);
        assert(!memcmp(got,expected,sizeof got));
    }
    RxCostModel model, decoded;
    rx_cm_init(&model,2);
    size_t n=rx_cm_blob_size(), written=0;
    uint8_t *blob=malloc(n); assert(blob);
    assert(rx_cm_serialize(&model,blob,n,&written)==RX_CM_OK && written==n);
    assert(rx_cm_deserialize(&decoded,blob,n)==RX_CM_OK);
    assert(!memcmp(&decoded,&model,sizeof model));
    assert(rx_cm_serialize(&model,blob,n,NULL)==RX_CM_OK);
    assert(rx_cm_serialize(NULL,blob,n,NULL)==RX_CM_ERR_ARG);
    assert(rx_cm_serialize(&model,NULL,n,NULL)==RX_CM_ERR_ARG);
    assert(rx_cm_serialize(&model,blob,n-1,NULL)==RX_CM_ERR_ARG);
    assert(rx_cm_deserialize(NULL,blob,n)==RX_CM_ERR_FORMAT);
    assert(rx_cm_deserialize(&decoded,NULL,n)==RX_CM_ERR_FORMAT);
    assert(rx_cm_deserialize(&decoded,blob,n-1)==RX_CM_ERR_FORMAT);
    assert(rx_cm_deserialize(&decoded,blob,n+1)==RX_CM_ERR_FORMAT);
    blob[0]^=1;
    assert(rx_cm_deserialize(&decoded,blob,n)==RX_CM_ERR_DIGEST);
    for(unsigned field=0;field<7;++field) {
        assert(rx_cm_serialize(&model,blob,n,NULL)==RX_CM_OK);
        uint32_t header[7]; memcpy(header,blob,sizeof header);
        header[field]=field==2 ? 0 : header[field]+1;
        memcpy(blob,header,sizeof header);
        sha256_hash(blob,n-32,blob+n-32);
        assert(rx_cm_deserialize(&decoded,blob,n)==RX_CM_ERR_FORMAT);
    }
    const double badscales[]={0.0,1000000.0,NAN,INFINITY};
    for(size_t i=0;i<sizeof badscales/sizeof badscales[0];++i) {
        assert(rx_cm_serialize(&model,blob,n,NULL)==RX_CM_OK);
        memcpy(blob+n-32-sizeof(double),badscales+i,sizeof(double));
        sha256_hash(blob,n-32,blob+n-32);
        assert(rx_cm_deserialize(&decoded,blob,n)==RX_CM_ERR_FORMAT);
    }
    free(blob);
    puts("PASS cm route C oracle, SHA block boundaries and blob transport");
    return 0;
}

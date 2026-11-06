/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_core.h"
#include "omega_canonical.h"
#include "omega_core_binding.h"
#include <stdlib.h>
#include <string.h>
_Static_assert(sizeof(OmegaObject)==25972 && sizeof(OmegaGraph)==6648836, "core bootstrap graph ABI");
_Static_assert(sizeof(TypePayload)==44 && offsetof(TypePayload,elem_type)==12, "type ABI");
_Static_assert(sizeof(ValuePayload)==98 && offsetof(ValuePayload,bytes)==34, "value ABI");
_Static_assert(sizeof(OperationPayload)==204 && offsetof(OperationPayload,input_types)==41 && offsetof(OperationPayload,output_type)==169,"operation ABI");
_Static_assert(sizeof(ApplyPayload)==161 && offsetof(ApplyPayload,operands)==33,"apply ABI");
_Static_assert(sizeof(EffectPayload)==184 && offsetof(EffectPayload,param_bytes)==50,"effect ABI");
static void core_status(uint32_t status) { if(status != OMEGA_OK) CORE_STOP(); }
OmegaGraph *omega_graph_create(void) { return calloc(1,sizeof(OmegaGraph)); }
void omega_graph_destroy(OmegaGraph *g) { free(g); }
OmegaObject *omega_graph_add_object(OmegaGraph *g,SemanticKind kind) {
    if(!g)return NULL;
    uint8_t id[32]={0}; uint64_t index;
    core_status(CORE_CALL(omgraph, &index,g,sizeof *g,id,sizeof id,kind,0));
    return index==256?NULL:&g->objects[index];
}
OmegaObject *omega_graph_find_object(OmegaGraph *g,const SemanticId *id) {
    if(!g||!id)return NULL;
    uint8_t key[32];memcpy(key,id->bytes,sizeof key);
    uint64_t index;core_status(CORE_CALL(omgraph, &index,g,sizeof *g,key,32,0,1));
    return index==256?NULL:&g->objects[index];
}
const OmegaObject *omega_graph_find_object_const(const OmegaGraph *g,const SemanticId *id) {
    return omega_graph_find_object((OmegaGraph *)g,id);
}
int omega_object_add_attribute(OmegaObject *o,const char *key,const uint8_t *value,uint16_t n) {
    if(!o||!key)return -1;
    uint8_t data[1024]={0};size_t k=strlen(key);uint64_t v[8]={k,n,value!=NULL},r;
    if(k<64)memcpy(data,key,k);
    if(value&&n<=512)memcpy(data+64,value,n);

    core_status(CORE_CALL(omobject, &r,o,sizeof *o,data,sizeof data,v,8,0));return -(int)r;
}
int omega_object_add_relation(OmegaObject *o,RelationKind kind,const SemanticId *target) {
    if(!o||!target)return -1;
    uint8_t data[1024]={0};uint64_t v[8]={kind},r;memcpy(data,target->bytes,32);
    core_status(CORE_CALL(omobject, &r,o,sizeof *o,data,sizeof data,v,8,1));return -(int)r;
}
int omega_object_add_constraint(OmegaObject *o,ConstraintKind kind,const uint8_t *data,uint16_t n) {
    if(!o)return -1;
    uint8_t d[1024]={0};uint64_t v[8]={kind,n,data!=NULL},r;
    if(data&&n<=128)memcpy(d,data,n);
    core_status(CORE_CALL(omobject, &r,o,sizeof *o,d,sizeof d,v,8,2));return -(int)r;
}
static OmegaObject *core_build(OmegaGraph *g,SemanticKind kind,uint64_t mode,const SemanticId *a,const SemanticId *b,const SemanticId *c,uint64_t x,uint64_t y,uint64_t z) {
    OmegaObject *o=omega_graph_add_object(g,kind);if(!o)return NULL;
    uint8_t ids[96]={0};uint64_t v[8]={x,y,z},n;
    if(a)memcpy(ids,a->bytes,32);
    if(b)memcpy(ids+32,b->bytes,32);
    if(c)memcpy(ids+64,c->bytes,32);
    core_status(CORE_CALL(ompay, &n,o->payload,1024,ids,96,v,8,mode));
    o->payload_len=(uint32_t)n;omega_compute_semantic_id(o);return o;
}
OmegaObject *omega_build_type_unit(OmegaGraph *g){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_UNIT,0,0);}
OmegaObject *omega_build_type_bool(OmegaGraph *g){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_BOOL,0,0);}
OmegaObject *omega_build_type_uint(OmegaGraph *g,uint16_t w){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_UNSIGNED_INT,w,0);}
OmegaObject *omega_build_type_signed_int(OmegaGraph *g,uint16_t w){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_SIGNED_INT,w,0);}
OmegaObject *omega_build_type_fp32(OmegaGraph *g){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_FP32,32,0);}
OmegaObject *omega_build_type_bitvector(OmegaGraph *g,uint16_t w){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_BITVECTOR,w,0);}
OmegaObject *omega_build_type_byte(OmegaGraph *g){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_BYTE,8,0);}
OmegaObject *omega_build_type_sequence(OmegaGraph *g,const SemanticId *id,uint32_t n){return core_build(g,KIND_TYPE,0,id,NULL,NULL,TYPE_SEQUENCE,0,n);}
OmegaObject *omega_build_type_cap_ref(OmegaGraph *g){return core_build(g,KIND_TYPE,0,NULL,NULL,NULL,TYPE_CAPABILITY_REF,0,0);}
OmegaObject *omega_build_val_bool(OmegaGraph *g,const SemanticId *id,bool v){return core_build(g,KIND_VALUE,1,id,NULL,NULL,0,v,0);}
OmegaObject *omega_build_val_uint(OmegaGraph *g,const SemanticId *id,uint16_t w,uint64_t v){return core_build(g,KIND_VALUE,2,id,NULL,NULL,w,v,0);}
OmegaObject *omega_build_op_binary(OmegaGraph *g,OpCode op,OverflowPolicy ov,const SemanticId *id){return core_build(g,KIND_OPERATION,4,id,NULL,NULL,op,ov,id!=NULL);}
OmegaObject *omega_build_apply(OmegaGraph *g,const SemanticId *op,const SemanticId *a,const SemanticId *b){return core_build(g,KIND_OPERATION,5,op,a,b,0,0,0);}
OmegaObject *omega_build_param(OmegaGraph *g,const SemanticId *id,uint16_t index){
    OmegaObject *o=omega_graph_add_object(g,KIND_VALUE);if(!o)return NULL;
    uint8_t type[32]={0};if(id)memcpy(type,id->bytes,sizeof type);
    core_status(CORE_VOID(omparam,o,sizeof *o,type,sizeof type,index));
    omega_compute_semantic_id(o);return o;
}
int omega_effect_payload_encode(const EffectPayload *p,uint8_t out[OMEGA_EFFECT_PAYLOAD_LEN]){
    if(!p||!out)return OMEGA_EFFECT_ERR_ARG;
    uint64_t r;core_status(CORE_CALL(omeffect, &r,(void *)p,sizeof *p,out,178,178,0));return (int)(int64_t)r;
}
int omega_effect_payload_decode(const uint8_t *data,size_t n,EffectPayload *out){
    if(out)memset(out,0,sizeof *out);
    if(!data||!out)return OMEGA_EFFECT_ERR_ARG;
    uint8_t wire[178]={0};uint64_t r;
    if(n==sizeof wire)memcpy(wire,data,n);
    core_status(CORE_CALL(omeffect, &r,out,sizeof *out,wire,sizeof wire,n,1));return (int)(int64_t)r;
}
int omega_effect_write(OmegaObject *o,const EffectPayload *p){
    if(!o||!p||o->kind!=KIND_EFFECT)return OMEGA_EFFECT_ERR_ARG;
    int r=omega_effect_payload_encode(p,o->payload);if(!r)o->payload_len=178;return r;
}
int omega_effect_read(const OmegaObject *o,EffectPayload *p){
    if(p)memset(p,0,sizeof *p);
    if(!o||!p||o->kind!=KIND_EFFECT)return OMEGA_EFFECT_ERR_ARG;
    return omega_effect_payload_decode(o->payload,o->payload_len,p);
}
const char *omega_effect_strerror(int code){
    static const char *const messages[]={"ok","not an effect object","effect payload length is not 178 bytes","legacy v1 effect payload (32-bit generation) refused","effect resource class is 0 (invalid resource)","effect param_len exceeds 128","effect parameter padding is not zero"};
    return code<=0&&code>=-6?messages[-code]:"unknown effect error";
}
OmegaObject *omega_build_effect(OmegaGraph *g,uint16_t res,uint16_t op,uint32_t slot,uint64_t gen){
    OmegaObject *o=omega_graph_add_object(g,KIND_EFFECT);if(!o)return NULL;
    EffectPayload p={.resource_class=res,.operation_code=op,.capability_slot=slot,.capability_generation=gen};
    omega_effect_write(o,&p);omega_compute_semantic_id(o);return o;
}
int omega_eval_pure_binary_uint(OpCode op,OverflowPolicy ov,uint16_t w,uint64_t a,uint64_t b,uint64_t *out){
    if(!out)return -1;
    uint64_t v[6]={op,ov,w,a,b,0},r;
    core_status(CORE_CALL(omeval, &r,v,6,out,1));return (int)(int64_t)r;
}

/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef MODEL_GRAPHS_FIXTURE_H
#define MODEL_GRAPHS_FIXTURE_H
#include <stddef.h>
#include <stdint.h>
/* Real graph translation units are compiled twice. Nested component recorders
 * are deterministic boundary adapters; the same actual command-list packer is
 * used on both sides. No model weights or device are opened by the fixture. */
typedef struct {uint64_t seed;uint32_t pos,layers,layer,heads,attn,flags,mutant,mode;} MgCase;
int mg_record(uint64_t kind,const uint64_t *words,size_t n);
void mg_fixture_setup(void *cl);
void *mg_fixture_list(void);
int mg_q36_run(const MgCase *c);
int mg_q38_run(const MgCase *c);
int mg_decode_run(const MgCase *c);
#endif

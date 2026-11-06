/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_native_artifact.h"
#include "omega_native_artifact_snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
int artifact_c_export(const OmegaNativeArtifact *, const char *, char *, char *, size_t);
int artifact_c_admit(const char *, const char *, char *, char *, size_t);
int artifact_c_snapshot(const char *, const char *, char *, OmegaNativeArtifactSnapshot *, char *, size_t);
void artifact_c_snapshot_free(OmegaNativeArtifactSnapshot *);
static const char *const suffix[]={".manifest",".text.bin",".cubin",".qmd0.bin",".qmd1.bin",".cbank_args.bin",".json",".sha256"};
static void fail(const char *what,int om,int c) { fprintf(stderr,"ARTIFACT differential FAIL %s om=%d c=%d\n",what,om,c); exit(90); }
static unsigned char *load(const char *p,size_t *n) {
 FILE *f=fopen(p,"rb"); if(!f)return NULL;
 if(fseek(f,0,SEEK_END))exit(91);
 long z=ftell(f);
 if(z<0 || fseek(f,0,SEEK_SET))exit(91);
 unsigned char *b=malloc((size_t)z+1); if(!b)exit(91);
 if(fread(b,1,(size_t)z,f)!=(size_t)z || fclose(f))exit(91);
 *n=(size_t)z;
 return b;
}
int artifact_diff_export(const OmegaNativeArtifact *a,const char *p,char *id,char *err,size_t cap) {
 char alt[2048],x[2200],y[2200],cid[65],ce[256];
 const char *cp=p;
 if(p && strlen(p)<sizeof alt) { strcpy(alt,p); size_t z=strlen(alt); if(z)alt[z-1]='~'; cp=alt;
  for(unsigned i=0;i<8;++i) {snprintf(x,sizeof x,"%s%s",p,suffix[i]);snprintf(y,sizeof y,"%s%s",cp,suffix[i]);unlink(y);
   struct stat st; if(lstat(x,&st)==0) {FILE *f=fopen(y,"wb");if(!f)exit(91);fclose(f);}
  }
 }
 int c=artifact_c_export(a,cp,id?cid:NULL,ce,sizeof ce);
 int om=omega_native_artifact_export(a,p,id,err,cap);
 if(c!=om)fail("export rc",om,c);
 if(!om) {
  if(strcmp(cid,id))fail("id",1,0);
  for(unsigned i=0;i<8;++i) {size_t nx=0,ny=0;snprintf(x,sizeof x,"%s%s",p,suffix[i]);snprintf(y,sizeof y,"%s%s",cp,suffix[i]);
   unsigned char *bx=load(x,&nx),*by=load(y,&ny);
   if(!bx||!by||nx!=ny||memcmp(bx,by,nx))fail(suffix[i],1,0);
   free(bx);free(by);
  }
 }
 if(cp==alt)for(unsigned i=0;i<8;++i){snprintf(y,sizeof y,"%s%s",cp,suffix[i]);unlink(y);}
 return om;
}
int artifact_diff_admit(const char *p,const char *d,char *id,char *err,size_t cap) {
 char cid[65],ce[256];int c=artifact_c_admit(p,d,id?cid:NULL,ce,sizeof ce);
 int om=omega_native_artifact_admit(p,d,id,err,cap);
 if(c!=om)fail("admit rc",om,c);
 if(id&&strcmp(cid,id))fail("admit id",1,0);
 return om;
}
int artifact_diff_snapshot(const char *p,const char *d,char *id,OmegaNativeArtifactSnapshot *out,char *err,size_t cap) {
 char cid[65],ce[256];OmegaNativeArtifactSnapshot cs={0};
 int c=artifact_c_snapshot(p,d,id?cid:NULL,out?&cs:NULL,ce,sizeof ce);
 int om=omega_native_artifact_snapshot(p,d,id,out,err,cap);
 if(c!=om)fail("snapshot rc",om,c);
 if(id&&strcmp(cid,id))fail("snapshot id",1,0);
 if(!om)for(unsigned i=0;i<6;++i)if(cs.sizes[i]!=out->sizes[i]||memcmp(cs.bytes[i],out->bytes[i],cs.sizes[i]))fail("snapshot bytes",1,0);
 artifact_c_snapshot_free(&cs);return om;
}

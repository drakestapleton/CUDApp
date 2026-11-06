/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Independent source-loop oracles: no generated implementation used as oracle. */
static uint32_t rawf(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static void data_parts(void){
 const unsigned sizes[]={0,1,3,4,5,31,32,33,65535,65536,65537};
 uint32_t *x=calloc(65537,4),*y=calloc(65537,4);uint16_t *h=calloc(65537,2);float *a=calloc(65537,4),*b=calloc(65537,4);unsigned char *bytes=calloc(65537,1);if(!x||!y||!h||!a||!b||!bytes)abort();
 for(unsigned i=0;i<65537;i++){x[i]=i%3?UINT32_MAX:0;h[i]=(uint16_t)(0x3f00+i%128);a[i]=(float)(i%127);bytes[i]=(unsigned char)i;}
 for(unsigned z=0;z<sizeof sizes/sizeof*sizes;z++){
  unsigned n=sizes[z];uint64_t r=0,nan[2]={UINT64_MAX,0};
  EQ(omega_arfill(&r,UINT32_MAX,y,n),0);EQ(r,0);for(unsigned i=0;i<n;i++)EQ(y[i],UINT32_MAX);
  EQ(omega_arcount(&r,UINT32_MAX,x,n),0);unsigned c=0;for(unsigned i=0;i<n;i++)c+=x[i]==UINT32_MAX;EQ(r,c);
  EQ(omega_arpad(&r,x,n,y,n),0);EQ(r,0);EQ(memcmp(x,y,n*4),0);
  EQ(omega_arcopy(&r,bytes,n,y,n),0);EQ(r,0);EQ(memcmp(bytes,y,n),0);
  EQ(omega_arnan(&r,bytes,n,nan,2),0);EQ(r,0);unsigned nn=0,first=0;for(unsigned i=0;i<n;i++)if(omega_api_isnanb(bytes[i])){if(!nn)first=i;nn++;}EQ(nan[1],nn);if(nn)EQ(nan[0],first);
  EQ(omega_arwiden(&r,h,n,b,n),0);EQ(r,0);for(unsigned i=0;i<n;i++)EQ(rawf(b[i]),(uint32_t)h[i]<<16);
  EQ(omega_arfold(&r,a,n,b,n),0);EQ(r,0);for(unsigned i=0;i<n;i++)EQ(rawf(b[i]),omega_api_xfold(rawf(a[i])));
  EQ(omega_arfoldh(&r,h,n,b,n),0);EQ(r,0);for(unsigned i=0;i<n;i++)EQ(rawf(b[i]),omega_api_xfold16(h[i]));
  EQ(omega_arscale(&r,0,h,n,b,n),0);EQ(r,0);for(unsigned i=0;i<n;i++){uint32_t u;EQ(omega_api_scalef(h[i],0,&u),0);EQ(rawf(b[i]),u);}
  EQ(omega_arwiden(&r,h,n,b,n+1),0);EQ(r,1); /* red extent mismatch */
 }
 uint64_t r=0;uint32_t rec[12]={0};EQ(omega_aridle(&r,rec,12),0);EQ(r,1);rec[7]=1;EQ(omega_aridle(&r,rec,12),0);EQ(r,0);rec[7]=0;rec[6]=1;EQ(omega_aridle(&r,rec,12),0);EQ(r,0);EQ(omega_aridle(&r,rec,11),0);EQ(r,0);
 uint64_t va[]={0,UINT32_MAX,UINT64_MAX};uint32_t words[7]={0};EQ(omega_arargs(&r,va,3,words,6),0);EQ(r,0);for(unsigned i=0;i<3;i++){EQ(words[i*2],(uint32_t)va[i]);EQ(words[i*2+1],va[i]>>32);}EQ(omega_arargs(&r,va,3,words,5),0);EQ(r,1);
 for(unsigned n=0;n<34;n++){
  uint32_t rows[34],xx[34*5],yy[34*5];uint16_t hx[34*5],hy[34*5];for(unsigned i=0;i<34*5;i++){xx[i]=i%2?UINT32_MAX:i;hx[i]=(uint16_t)i;}for(unsigned i=0;i<n;i++)rows[i]=33-i;
  EQ(omega_argath32(&r,rows,n,xx,170,yy,n*5),0);EQ(r,0);EQ(omega_argath16(&r,rows,n,hx,170,hy,n*5),0);EQ(r,0);for(unsigned i=0;i<n;i++)for(unsigned j=0;j<5;j++){EQ(yy[i*5+j],xx[rows[i]*5+j]);EQ(hy[i*5+j],hx[rows[i]*5+j]);}
 }
 for(unsigned e=0;e<5;e++){
  float xx[160],yy[180];for(unsigned i=0;i<160;i++)xx[i]=(float)i;EQ(omega_arroute(&r,e,xx,e*32,yy,e*36),0);EQ(r,0);if(e){for(unsigned j=0;j<8;j++)for(unsigned i=0;i<e;i++)for(unsigned k=0;k<4;k++)EQ(rawf(yy[j*e*4+i*4+k]),rawf(xx[i*32+j*4+k]));for(unsigned i=e*32;i<e*36;i++)EQ(rawf(yy[i]),0);}
 }
 for(unsigned lb=8;lb<=16;lb+=8){float xx[512],yy[512];uint16_t hy[512];for(unsigned i=0;i<512;i++)xx[i]=(float)(i%17);unsigned n=32*lb;EQ(omega_arperm(&r,lb,xx,n,yy,n),0);EQ(r,0);for(unsigned i=0;i<n;i++){unsigned p=(i/(32*lb))*(32*lb)+((i/4)%32)*lb+((i%(32*lb))/128)*4+i%4;float scale;uint32_t u=1602224128;memcpy(&scale,&u,4);EQ(rawf(yy[i]),rawf(xx[p]*scale));}EQ(omega_artcperm(&r,xx,n,hy,n),0);EQ(r,0);}
 float rev[4];for(unsigned n=0;n<=4;n++){EQ(omega_arrev(&r,a,n,rev,4),0);EQ(r,0);for(unsigned i=0;i<4;i++)EQ(rawf(rev[i]),i<4-n?0:rawf(a[3-i]));}
 free(x);free(y);free(h);free(a);free(b);free(bytes);
}
static void tile_parts(void){
 const unsigned ns[]={0,1,15,16,17,31,32,33};uint64_t r=0,out[128];
 for(unsigned z=0;z<sizeof ns/sizeof*ns;z++){
  unsigned n=ns[z];uint32_t ids[33],rows[33];for(unsigned i=0;i<n;i++){ids[i]=i/16;rows[i]=i;}
  uint64_t q[8]={3,34,16};EQ(omega_argroups(&r,q,4,ids,n,rows,n),0);EQ(r,(uint64_t)((n+15)/16)|((uint64_t)((n+15)/16)<<32));
  for(unsigned i=0;i<n;i++){unsigned j=i;while(j<n&&ids[j]==ids[i])j++;EQ(omega_arrun(&r,i,ids,n),0);EQ(r,j);}
  if(n){ids[0]=UINT32_MAX;EQ(omega_argroups(&r,q,4,ids,n,rows,n),0);EQ(r,UINT64_MAX);ids[0]=0;}
  uint32_t counts[3]={n,1,0};q[0]=32;q[1]=UINT32_MAX;q[2]=UINT64_C(4294967296);q[3]=128;q[4]=256;q[5]=16;
  EQ(omega_artiles(&r,q,8,counts,3,out,128),0);unsigned nt=0,row=0;for(unsigned e=0;e<3;e++){for(unsigned i=0;i<counts[e];i+=16){unsigned nn=counts[e]-i;if(nn>16)nn=16;EQ(out[nt*4],q[1]+e*128);EQ(out[nt*4+1],q[2]+e*256);EQ(out[nt*4+2],(uint64_t)(row+i)|((uint64_t)nn<<32));EQ(out[nt*4+3],0);nt++;}row+=counts[e];}EQ(r,nt);
  uint64_t tile[8]={0,17,n,16,UINT32_MAX,UINT64_C(4294967296)};EQ(omega_artile(&r,tile,8,out,128),0);EQ(r,(n+15)/16);for(unsigned i=0;i<(n+15)/16;i++){unsigned nn=n-i*16;if(nn>16)nn=16;EQ(out[i*4],tile[4]);EQ(out[i*4+1],tile[5]);EQ(out[i*4+2],(uint64_t)(17+i*16)|((uint64_t)nn<<32));EQ(out[i*4+3],0);}
  q[0]=n;q[1]=UINT64_MAX;q[2]=128;q[3]=UINT32_MAX;EQ(omega_artg(&r,q,8,out,128),0);if(n<=32){EQ(r,n);for(unsigned i=0;i<n;i++){EQ(out[i*4],UINT64_MAX);EQ(out[i*4+1],i*128);EQ(out[i*4+2],UINT32_MAX);EQ(out[i*4+3],0);}}else EQ(r,0);
 }
}
static void state_parts(void){
 uint64_t r=0;
 uint32_t four[4]={1,2,3,4};float fs[4]={1,1,1,1};EQ(omega_arsegok(&r,4,4,four,4,fs,4),0);EQ(r,1);
 uint32_t nanbits=0x7fc00000;memcpy(fs+3,&nanbits,4);EQ(omega_arsegok(&r,4,4,four,4,fs,4),0);EQ(r,0);
 const uint32_t vals[]={0,1,7,8,9,15,16,17,127,128,129,UINT32_MAX-1,UINT32_MAX};
 for(unsigned v=0;v<sizeof vals/sizeof*vals;v++){
  uint32_t n=vals[v],end[4]={n,n,n,n},got[3];float sc[4]={1,1,1,1};
  for(unsigned c=0;c<=4;c++){
   unsigned valid=!omega_api_segend(0,c,1,1,0);uint32_t prev=0;
   if(valid)for(unsigned j=0;j<c;j++){if(omega_api_segend(j,c,n,end[j],prev))valid=0;prev=end[j];}
   EQ(omega_arsegok(&r,n,c,end,4,sc,4),0);EQ(r,valid);
   EQ(omega_arsegb(&r,n,c,0,end,3,got,3),0);
   if(c>=1&&c<=4){EQ(r,0);for(unsigned j=0;j<3;j++)EQ(got[j],omega_api_segb(j,c,n,end[j],0));}else EQ(r,1);
  }
  uint64_t q[12]={0},out[12],st[7]={1,2,3,4,5,6,7};uint32_t cnt[4]={0,n,0,0};
  EQ(omega_arnext(&r,q,12,cnt,4),0);EQ(r,n!=0);if(n){EQ(q[6],1);EQ(q[1],0);EQ(q[2],n);EQ(q[7],n);EQ(q[8],2);}EQ(omega_arnext(&r,q,12,cnt,4),0);EQ(r,0);
  uint32_t spec[8]={n,128,0,0,9,0,0,0},sp[6];
  uint32_t defw;omega_api_fp8_defaults(0,0,NULL,&defw);
  EQ(omega_argspec(&r,spec,8,sp,6),0);EQ(r,0);EQ(sp[0],n);EQ(sp[1],omega_api_lane(n));EQ(sp[2],omega_api_fp8mt(0,128));EQ(sp[3],defw);EQ(sp[4],9);EQ(sp[5],0);
  uint64_t ss[8]={n,128,128,5,6};
  EQ(omega_arsstats(&r,ss,8,st,7,out,12),0);EQ(r,0);EQ(out[0],(uint64_t)n|((uint64_t)128<<32));EQ(out[1],128);EQ(out[2],(uint64_t)n*(128*128+4));for(unsigned i=0;i<7;i++)EQ(out[i+3],st[i]);EQ(out[10],5);EQ(out[11],6);
 }
 uint32_t rows[5]={0,1,1,2,0};unsigned char have[3]={0};uint64_t q[16]={0,UINT32_MAX,1};
 for(unsigned i=0;i<5;i++){EQ(omega_arxnext(&r,q,16,rows,5,have,3),0);EQ(r,1);EQ(q[5],i);EQ(q[6],rows[i]);EQ(q[7],i!=2&&i!=4);EQ(q[8],(uint64_t)rows[i]*UINT32_MAX);EQ(q[9],(uint64_t)i*UINT32_MAX);EQ(q[10],UINT32_MAX);EQ(q[11],(uint64_t)UINT32_MAX*4);EQ(q[12],(uint64_t)UINT32_MAX*2);}EQ(omega_arxnext(&r,q,16,rows,5,have,3),0);EQ(r,0);
 for(unsigned z=0;z<5;z++){
  unsigned n=(unsigned[]){0,1,127,128,129}[z];unsigned char w[129];uint16_t scale[2]={0x3f80,0x4000};float x[129];double sum=0,mag=0;for(unsigned i=0;i<n;i++){w[i]=0x38;x[i]=i%2?-1:1;}
  for(unsigned i=0;i<n/128*128;i++){uint32_t wb=omega_api_refb(w[i]);float wf;memcpy(&wf,&wb,4);double term=(double)wf*x[i];float f;uint32_t bits=(uint32_t)scale[i/128]<<16;memcpy(&f,&bits,4);term*=f;sum+=term;mag+=term<0?-term:term;}
  uint64_t bits;memcpy(&bits,&sum,8);EQ(omega_aracc(&r,w,n,scale,2,x,n),0);EQ(r,bits);memcpy(&bits,&mag,8);EQ(omega_armag(&r,w,n,scale,2,x,n),0);EQ(r,bits);
 }
}
static void calc_parts(void){
 const uint32_t v[]={0,1,7,8,9,15,16,17,127,128,129,255,256,257,UINT32_MAX-1,UINT32_MAX};
 for(unsigned z=0;z<sizeof v/sizeof*v;z++)for(unsigned op=1;op<=12;op++){
  uint64_t q[32]={op,v[z],128,256,8,2,4,2,1,3},e[16]={0},r=0,rc=0;
  uint64_t a=q[1],b=q[2],c=q[3],d=q[4],f=q[5],g=q[6],h=q[7],i=q[8],j=q[9];
  switch(op){
  case 1:e[0]=(uint32_t)((uint32_t)a+(uint32_t)b-1)/(uint32_t)b;e[1]=c/4;break;
  case 2:e[0]=a/d;e[1]=b/f;e[2]=32*g;e[3]=e[2]*e[1]*e[0];e[4]=(e[0]*32+4095)&~UINT64_C(4095);e[5]=c/128;break;
  case 3:{uint64_t kb=c/128,sc=b/128*kb,wb=b*c,rb=b/8*(kb+1)*4;e[0]=sc;e[1]=wb;e[2]=rb;e[3]=a*wb;e[4]=a*rb;e[5]=(a*rb+15)&~UINT64_C(15);e[6]=d*sc;e[7]=d*wb;e[8]=d*rb;e[9]=sc*4;e[10]=d*rb/4;e[11]=rb/4;e[12]=a*rb+16;e[13]=a*wb&~UINT64_C(15);break;}
  case 4:{uint32_t rr=(uint32_t)b*2;if((uint32_t)a%rr){rc=1;break;}uint32_t u=(uint32_t)a/rr,gx=(u+(uint32_t)c-1)/(uint32_t)c;if(gx>d)gx=d;if(!gx)gx=1;e[0]=u;e[1]=gx;e[2]=(uint32_t)((size_t)gx*(uint32_t)c);e[3]=(uint32_t)(0-(uint32_t)f);e[4]=(uint32_t)(0-(uint32_t)g);e[5]=(uint32_t)(0-(uint32_t)h);e[6]=(uint32_t)(0-(uint32_t)i);e[7]=32*c;break;}
  case 5:e[0]=32*a;e[1]=b*c*d;e[2]=a*b*32;break;
  case 6:e[0]=a*(b*c+b/128*(c/128)*4);break;
  case 7:e[0]=16*f;e[1]=h*8*g;e[2]=32*h;e[3]=i*c*2;e[4]=a*b*4;e[5]=d*32;e[6]=(a+e[0])*4;e[7]=a*b;e[8]=i*c;e[9]=b/e[1];e[10]=e[0]*256;e[11]=d*4;e[12]=a+e[0];break;
  case 8:{uint32_t gx=((uint32_t)b+(uint32_t)f-1)/(uint32_t)f;if(gx>g)gx=g;if(!gx)gx=1;e[0]=gx;e[1]=(uint32_t)((size_t)gx*(uint32_t)f);e[2]=32*(uint32_t)f;e[3]=a*b;e[4]=(a+d)*c;e[5]=(a+d)*c*4;e[6]=a*b*4;e[7]=h*c;e[8]=h*c*4;e[9]=c*4;e[10]=a*c;e[11]=d*c;e[12]=j-i;e[13]=(j-i)*c;e[14]=i*b;e[15]=a*c*4;break;}
  case 9:{q[4]=a<16?0:1;q[5]=1;e[0]=a/16;e[1]=b/8;if(a<16)break;uint64_t rows=a;if(a/16>c)rows=c*16;uint64_t gx=c/(rows/16);if(!gx)gx=1;if(gx>b/8)gx=b/8;e[2]=rows;e[3]=gx;e[4]=rows/16;break;}
  case 10:e[0]=(uint32_t)((uint32_t)b*16*(uint32_t)c);e[1]=a*b*128;break;
  case 11:{uint64_t u=b;while(u>1&&a/16/u<2)u/=2;e[0]=u;break;}
  case 12:{uint64_t mp=(uint32_t)((uint32_t)a+15)/16*16,kp=(uint32_t)((uint32_t)b+15)/16*16,np=(uint32_t)((uint32_t)c+7)/8*8;e[0]=mp;e[1]=kp;e[2]=np;e[3]=mp*kp*2;e[4]=mp*np*4;e[5]=kp*np*2;e[6]=b*c;e[7]=b*c*2;e[8]=np/8;e[9]=64*kp*16;break;}
  }
  EQ(omega_arcalc(&r,q,32),0);EQ(r,rc);if(!rc)for(unsigned i=0;i<16;i++)EQ(q[16+i],e[i]);
 }
 uint64_t q[32]={0},r;EQ(omega_arcalc(&r,q,32),0);EQ(r,1);
}
static void misc_parts(void){
 uint64_t r=0;uint32_t vals[]={0,1,15,16,17,31,32,33,UINT32_MAX-1,UINT32_MAX};
 for(unsigned z=0;z<sizeof vals/sizeof*vals;z++){
  for(unsigned cols=1;cols<=8;cols+=7){uint64_t window[16]={0,1,cols,UINT32_MAX};window[15]=3;EQ(omega_armstep(&r,window,16,vals+z,1),0);EQ(r,omega_api_wok(vals[z],cols,UINT32_MAX)?1:0);}
  uint32_t spec[3]={vals[z],0,UINT32_MAX},key[5];
  EQ(omega_arkey(&r,vals[z],spec,3,key,5),0);EQ(r,0);EQ(key[0],vals[z]);EQ(memcmp(key+1,spec,12),0);EQ(key[4],0);
  EQ(omega_arkey(&r,vals[z],spec,3,key,3),0);EQ(r,1);
  uint64_t tq[8]={0,vals[z],vals[z],16,UINT64_MAX,UINT32_MAX},out[8];EQ(omega_argtile(&r,tq,8,out,8),0);EQ(r,0);EQ(tq[2],0);
  uint64_t sq[16]={0,vals[z],16,32,4,0,0,0,UINT64_MAX,UINT32_MAX,0};
  EQ(omega_argseg(&r,sq,16,out,8),0);EQ(r,0);EQ(out[0],UINT64_MAX);EQ(out[1],UINT32_MAX);EQ(out[2],(uint64_t)vals[z]<<32);EQ(out[3],0);
  uint64_t p=((uint64_t)vals[z]+15)/16;EQ(sq[5],p);EQ(sq[6],p*128);EQ(sq[7],128);EQ(sq[0],vals[z]);EQ(sq[10],1);
 }
 const uint32_t sizes[]={0,1,15,16,17,31,32,33};
 for(unsigned z=0;z<sizeof sizes/sizeof*sizes;z++){
  unsigned n=sizes[z];float f[36];uint16_t bf[36];const uint32_t bits[]={0,1,0x3f807fff,0x3f808000,0x3f808001,0x7f800000,0x7f800001,UINT32_MAX-1,UINT32_MAX};for(unsigned i=0;i<36;i++)memcpy(f+i,bits+i%9,4);
  EQ(omega_ar_bf16(&r,f,n,bf,n),0);EQ(r,0);for(unsigned i=0;i<n;i++)EQ(bf[i],omega_fp32_to_bf16(f[i]));
  EQ(omega_ar_bf16(&r,f,n,bf,n+1),0);EQ(r,1);
  for(unsigned i=0;i<36;i++)f[i]=(float)(i*3);
  unsigned m=(n+3)/4*4;EQ(omega_artcperm(&r,f,m,bf,m),0);EQ(r,0);uint32_t u=1602224128;float scale;memcpy(&scale,&u,4);for(unsigned i=0;i<m;i++){unsigned off=i%4;if(off==1)off=2;else if(off==2)off=1;EQ(bf[i],omega_fp32_to_bf16(f[i-i%4+off]*scale));}
  unsigned char trit[33];for(unsigned i=0;i<n;i++)trit[i]=(unsigned char[]){0,1,255}[i%3];EQ(omega_artrits(&r,trit,n),0);EQ(r,n);if(n){trit[n-1]=2;EQ(omega_artrits(&r,trit,n),0);EQ(r,n-1);}
 }
 for(unsigned layout=0;layout<2;layout++)for(unsigned n=0;n<=17;n++){
  unsigned np=(n+7)/8*8;int8_t w[17*32];uint32_t out[24*2],expect[24*2]={0};uint64_t q[4]={n,32,layout,2};
  for(unsigned i=0;i<n*32;i++)w[i]=(int8_t[]){0,1,-1}[i%3];
  for(unsigned row=0;row<n;row++)for(unsigned c=0;c<2;c++){uint32_t word=0;for(unsigned i=0;i<16;i++){int t=w[row*32+c*16+i];word|=(uint32_t)(t==1?1:t==-1?2:0)<<(i*2);}unsigned p=layout?(row/4*4*2+c*4+row%4):(row*2+c);expect[p]=word;}
  EQ(omega_artern(&r,q,4,w,n*32,out,np*2),0);EQ(r,0);EQ(memcmp(out,expect,np*8),0);
 }
}
static void slot_parts(void){
 const uint32_t pins[]={0,1,UINT32_MAX};
 for(unsigned op=0;op<=7;op++)for(unsigned state=0;state<3;state++)for(unsigned p=0;p<3;p++)for(unsigned published=0;published<2;published++)for(unsigned match=0;match<2;match++){
  uint64_t gen=match?11:13,q[8]={5,7,published,op,11},r[3]={state|((uint64_t)pins[p]<<32),11,gen},stats[7]={0,0,0,0,0,0,UINT64_C(0x1234567800000009)},rec[16]={state,pins[p],11,gen,5,7,published},ret;
  rec[14]=9;rec[15]=0x12345678;int32_t expected=omega_api_slot(op,11,rec);
  EQ(omega_arslot(&ret,q,8,r,3,stats,7),0);EQ(ret,(uint64_t)(int64_t)expected);EQ(r[0],(uint32_t)rec[0]|((uint64_t)(uint32_t)rec[1]<<32));EQ(r[1],rec[2]);EQ(r[2],rec[3]);EQ(q[0],rec[4]);for(unsigned i=0;i<6;i++)EQ(stats[i],rec[8+i]);EQ(stats[6],(uint32_t)rec[14]|((uint64_t)(uint32_t)rec[15]<<32));
  uint64_t gate[16]={state,pins[p],11,gen};expected=omega_api_slot(7,11,gate);uint64_t rr[3]={state|((uint64_t)pins[p]<<32),11,gen};EQ(omega_arsgate(&ret,11,rr,3),0);EQ(ret,(uint64_t)(int64_t)expected);
 }
}

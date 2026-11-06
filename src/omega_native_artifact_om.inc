/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* The default artifact implementation. All canonical bytes and admission
 * decisions come from seeded Omega. This file owns pointers, memory and I/O. */
static const char *const af_suffix[] = {".manifest", ".text.bin", ".cubin", ".qmd0.bin", ".qmd1.bin", ".cbank_args.bin", ".json", ".sha256"};
typedef struct { unsigned char b[65536]; uint64_t s[128]; } ArtifactWire;
const char *omega_native_artifact_rc_name(int rc) {
    static const char *const names[] = {"OK","ARG","FORMAT","CONTRACT","ISA","DIGEST","IO","RELOC","DRIVER","UNSUPPORTED","MEM"};
    return rc >= 0 && (size_t)rc < sizeof names / sizeof names[0] ? names[rc] : "UNKNOWN";
}
static int af_paths(char path[8][1024], const char *prefix, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        int n = snprintf(path[i], 1024, "%s%s", prefix, af_suffix[i]);
        if (n < 0 || n >= 1024) return OMEGA_ARTIFACT_E_ARG;
    }
    return 0;
}
static int af_write(const char *path, const void *data, size_t n) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644);
    if (fd < 0) return OMEGA_ARTIFACT_E_IO;
    FILE *f = fdopen(fd, "wb");
    if (!f) { close(fd); return OMEGA_ARTIFACT_E_IO; }
    int rc = n && fwrite(data, 1, n, f) != n ? OMEGA_ARTIFACT_E_IO : 0;
    if (fclose(f)) rc = OMEGA_ARTIFACT_E_IO;
    return rc;
}
static uint8_t *af_read(const char *path, size_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END)) { fclose(f); return NULL; }
    long z = ftell(f);
    if (z < 0 || z > (long)(1u << 26) || fseek(f, 0, SEEK_SET)) { fclose(f); return NULL; }
    uint8_t *b = malloc((size_t)z + 1);
    if (!b) { fclose(f); return NULL; }
    if (z && fread(b, 1, (size_t)z, f) != (size_t)z) { free(b); fclose(f); return NULL; }
    b[z] = 0; *n = (size_t)z; fclose(f); return b;
}
static void af_hex(const void *data, size_t n, char out[65]) {
    uint8_t d[32]; uint64_t st[8] = {0}, pair;
    sha256_hash(data, n, d);
    for (unsigned i = 0; i < 32; ++i) {
        if (omega_artifact_parse(&pair, 4, d[i], st, 8) != OMEGA_OK) abort();
        out[2*i] = (char)pair; out[2*i+1] = (char)(pair >> 8);
    }
    out[64] = 0;
}
static int af_pack(ArtifactWire *w, unsigned k, const char *v, size_t *off) {
    size_t n = strlen(v);
    if (n > 20000 - *off) return OMEGA_ARTIFACT_E_ARG;
    w->s[8+k] = *off; w->s[48+k] = n;
    memcpy(w->b + *off, v, n); *off += n;
    return 0;
}
static int af_policy(ArtifactWire *w) {
    uint64_t rc = 0;
    if (omega_artifact_admit(&rc, w->b, sizeof w->b, w->s, 128) != OMEGA_OK) return OMEGA_ARTIFACT_E_FORMAT;
    return (int)rc;
}
static int af_build(ArtifactWire *w, unsigned mode, size_t *n) {
    uint64_t ok = 0; w->s[0] = mode;
    if (omega_artifact_format(&ok, w->b, sizeof w->b, w->s, 128) != OMEGA_OK || !ok) return OMEGA_ARTIFACT_E_ARG;
    *n = (size_t)w->s[1]; return 0;
}
static int af_elf(const uint8_t *cub, size_t cn, const uint8_t *text, size_t tn, const char *fn) {
    unsigned char b[512] = {0}; uint64_t s[32] = {0}, result = 2;
    s[1] = cn; s[2] = tn; s[3] = strlen(fn);
    memcpy(b + 256, fn, (size_t)s[3]);
    if (cn >= 64) memcpy(b, cub, 64);
    while (result == 2) {
        if (omega_artifact_elf(&result, b, sizeof b, s, 32) != OMEGA_OK) return 0;
        if (result != 2) break;
        memcpy(b, cub + s[4], (size_t)s[5]);
        if (s[6]) memcpy(b + 128, text + s[14], (size_t)s[5]);
    }
    return (int)result;
}
static int af_error(int rc, char *err, size_t cap) {
    if (err && cap) snprintf(err, cap, "artifact refused: %s", omega_native_artifact_rc_name(rc));
    return rc;
}
int omega_native_artifact_export(const OmegaNativeArtifact *a, const char *prefix,
    char out_id[OMEGA_ARTIFACT_ID_MAX], char *err, size_t err_cap) {
    char path[8][1024], hashes[8][65]; SemanticId machine;
    uint8_t *cub = NULL, *manifest = NULL; size_t cn = 0, mn = 0, n = 0, off = 0;
    ArtifactWire w = {0}; int rc = 0;
    if (err && err_cap) err[0] = 0;
    if (out_id) out_id[0] = 0;
    if (!a || !prefix || !out_id || !a->op || !a->shape || !a->bounds || !a->contract_name || !a->kernel_function ||
        !a->text || !a->text_len || !a->params || !a->qmd0 || !a->qmd1 || !a->cbank_args ||
        !a->semantic_spec_id_hex || !a->contract_id_hex || !a->min_driver || !a->requires || !a->precision ||
        !a->accumulation || !a->tolerance || !a->source_commit || !a->tool_version || !a->qualification)
        return af_error(OMEGA_ARTIFACT_E_ARG, err, err_cap);
    const char *values[35] = {"",a->op,a->shape,a->bounds,a->contract_name,a->contract_id_hex,a->semantic_spec_id_hex,
        "sm_121","","nvidia-cubin-elf64",a->kernel_function,"","","","","","","zero-va-v1","","","template","","","","","",
        a->precision,a->accumulation,a->tolerance,a->min_driver,a->requires,a->qualification,a->source_commit,a->tool_version,""};
    for (unsigned k = 0; k < 35; ++k) if ((rc = af_pack(&w,k,values[k],&off))) return af_error(rc,err,err_cap);
    w.s[88] = a->text_len; w.s[90] = a->cbank_offset; w.s[91] = a->cbank_words;
    w.s[92] = a->grid_x; w.s[93] = a->grid_y; w.s[94] = a->block_x; w.s[95] = a->block_y;
    w.s[96] = a->block_z; w.s[97] = a->shared_bytes; w.s[98] = a->gpr_count;
    memcpy(w.b+24576,a->qmd0,256); memcpy(w.b+24832,a->qmd1,256);
    w.s[0] = 10;
    if ((rc = af_policy(&w))) return af_error(rc,err,err_cap);
    if ((rc = af_paths(path,prefix,8))) return af_error(rc,err,err_cap);
    w.s[0] = 12;
    if ((rc = af_policy(&w))) return af_error(rc,err,err_cap);
    if (omega_blackwell_get_machine_id(&machine)) return af_error(OMEGA_ARTIFACT_E_ARG,err,err_cap);
    /* The cubin packer's policy remains in its own seeded Omega unit. */
    w.s[115] = OMEGA_CUBIN_PARAM_BASE;
    for (size_t i = 0; i < a->nparams; ++i) {
        w.s[0] = 14; w.s[113] = a->params[i].offset; w.s[114] = a->params[i].size;
        if ((rc = af_policy(&w))) return af_error(rc,err,err_cap);
    }
    rc = omega_cubin_pack(a->kernel_function,a->text,a->text_len,a->params,a->nparams,(uint32_t)w.s[115],a->gpr_count,64,&cub,&cn,err,err_cap);
    w.s[0] = 13; w.s[5] = (uint64_t)rc;
    if ((rc = af_policy(&w))) return af_error(rc,err,err_cap);
    w.s[0] = 11;
    if ((rc = af_policy(&w))) goto done;
    w.s[89] = cn;
    af_hex(a->text,a->text_len,hashes[0]); af_hex(cub,cn,hashes[1]);
    af_hex(a->qmd0,384,hashes[2]); af_hex(a->qmd1,384,hashes[3]); af_hex(a->cbank_args,a->cbank_words*4,hashes[4]);
    /* Hex encoding of an existing semantic id uses the same Omega byte codec. */
    { uint64_t st[8]={0},pair;
      for (unsigned i=0;i<32;++i) { if (omega_artifact_parse(&pair,4,machine.bytes[i],st,8)!=OMEGA_OK) abort(); hashes[5][2*i]=(char)pair; hashes[5][2*i+1]=(char)(pair>>8); }
      hashes[5][64]=0; }
    values[8]=hashes[5]; values[11]=hashes[0]; values[13]=hashes[1]; values[15]=hashes[2]; values[16]=hashes[3]; values[21]=hashes[4];
    const unsigned hashed[] = {8,11,13,15,16,21};
    for (unsigned i=0;i<sizeof hashed/sizeof hashed[0];++i)
        if ((rc=af_pack(&w,hashed[i],values[hashed[i]],&off))) goto done;
    if ((rc=af_build(&w,0,&n))) goto done;
    af_hex(w.b+32768,n,hashes[6]); values[0]=hashes[6];
    if ((rc=af_pack(&w,0,values[0],&off))) goto done;
    if ((rc=af_build(&w,1,&mn))) goto done;
    manifest=malloc(mn); if (!manifest) { rc=OMEGA_ARTIFACT_E_MEM; goto done; }
    memcpy(manifest,w.b+32768,mn);
    if ((rc=af_write(path[1],a->text,a->text_len)) || (rc=af_write(path[2],cub,cn)) ||
        (rc=af_write(path[3],a->qmd0,384)) || (rc=af_write(path[4],a->qmd1,384)) ||
        (rc=af_write(path[5],a->cbank_args,a->cbank_words*4))) goto done;
    if ((rc=af_build(&w,2,&n)) || (rc=af_write(path[6],w.b+32768,n)) || (rc=af_write(path[0],manifest,mn))) goto done;
    af_hex(manifest,mn,hashes[7]);
    if ((rc=af_pack(&w,35,hashes[7],&off)) || (rc=af_build(&w,4,&n)) || (rc=af_write(path[7],w.b+32768,n))) goto done;
    memcpy(out_id,hashes[6],65);
done:
    free(cub); free(manifest);
    if (rc) af_error(rc,err,err_cap);
    return rc;
}
static int artifact_admit(const char *prefix, const char *driver, char out_id[65],
    OmegaNativeArtifactSnapshot *snapshot, char *err, size_t err_cap) {
    char path[8][1024], kernel[64]={0}; uint8_t *files[6]={0}; size_t sizes[6]={0};
    ArtifactWire w={0}; int rc=0;
    if (err && err_cap) err[0]=0;
    if (out_id) out_id[0]=0;
    if (snapshot) memset(snapshot,0,sizeof *snapshot);
    if (!prefix) return OMEGA_ARTIFACT_E_ARG;
    if ((rc=af_paths(path,prefix,6))) return af_error(rc,err,err_cap);
    files[0]=af_read(path[0],&sizes[0]);
    if (!files[0]) return af_error(OMEGA_ARTIFACT_E_IO,err,err_cap);
    w.s[3]=sizes[0];
    /* Bound the copy; Omega alone decides whether the manifest fits. */
    memcpy(w.b,files[0],sizes[0]<8192?sizes[0]:8192);
    if (driver) {
        size_t n=strnlen(driver,512); w.s[106]=1; w.s[107]=n;
        memcpy(w.b+25088,driver,n);
    }
    if ((rc=af_policy(&w))) goto done;
    while (w.s[0]!=5) {
        if (w.s[0]==1) {
            char hash[65];
            size_t start=(size_t)w.s[108];
            size_t end=(size_t)w.s[109];
            af_hex(files[0]+start,end-start,hash); memcpy(w.b+26000,hash,64);
        } else if (w.s[0]==2) {
            unsigned f=(unsigned)w.s[6]; char hash[65];
            files[f]=af_read(path[f],&sizes[f]); w.s[5]=files[f]!=NULL; w.s[100+f]=sizes[f];
            if (files[f]) { af_hex(files[f],sizes[f],hash); memcpy(w.b+26000,hash,64); }
        } else if (w.s[0]==3) {
            memcpy(kernel,files[0]+w.s[18],(size_t)w.s[58]);
            w.s[5]=af_elf(files[2],sizes[2],files[1],sizes[1],kernel);
            memcpy(w.b+24576,files[3],256); memcpy(w.b+24832,files[4],256);
        }
        if ((rc=af_policy(&w))) goto done;
    }
    if (out_id) { memcpy(out_id,files[0]+w.s[8],64); out_id[64]=0; }
done:
    if (!rc && snapshot) {
        memcpy(snapshot->bytes,files,sizeof files); memcpy(snapshot->sizes,sizes,sizeof sizes);
        memcpy(snapshot->kernel_function,kernel,sizeof kernel);
        memcpy(snapshot->id,files[0]+w.s[8],64); snapshot->id[64]=0;
    } else for (unsigned i=0;i<6;++i) free(files[i]);
    if (rc) af_error(rc,err,err_cap);
    return rc;
}
int omega_native_artifact_admit(const char *prefix,const char *driver,char out_id[65],char *err,size_t cap) {
    return artifact_admit(prefix,driver,out_id,NULL,err,cap);
}
int omega_native_artifact_snapshot(const char *prefix,const char *driver,char out_id[65],OmegaNativeArtifactSnapshot *out,char *err,size_t cap) {
    if (!out) return OMEGA_ARTIFACT_E_ARG;
    return artifact_admit(prefix,driver,out_id,out,err,cap);
}
void omega_native_artifact_snapshot_free(OmegaNativeArtifactSnapshot *snapshot) {
    if (!snapshot) return;
    for (unsigned i=0;i<OMEGA_SNAPSHOT_FILES;++i) free(snapshot->bytes[i]);
    memset(snapshot,0,sizeof *snapshot);
}

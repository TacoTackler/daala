/*Daala video codec
Copyright (c) 2006-2010 Daala project contributors.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#if !defined(_state_H)
# define _state_H (1)

typedef struct od_state_opt_vtbl od_state_opt_vtbl;
typedef struct od_state          od_state;
typedef struct od_yuv_dumpfile   od_yuv_dumpfile;
typedef struct od_adapt_ctx      od_adapt_ctx;

# include <stdio.h>
# include "internal.h"
# include "dct.h"
# include "mc.h"
# include "filter.h"
# include "dering.h"
# include "pvq.h"
# include "generic_code.h"
# include "util.h"
# include "intra.h"

extern const od_coeff OD_DC_QM[OD_NBSIZES - 1][2];

extern const int OD_HAAR_QM[2][OD_LOG_BSIZE_MAX];

/*Adaptation speed of scalar Laplace encoding.*/
# define OD_SCALAR_ADAPT_SPEED (4)

#define OD_MAX_CODED_REFS (2)

/*The golden reference frame.*/
# define OD_FRAME_GOLD (0)
/*The previous reference frame.*/
# define OD_FRAME_PREV (1)
/*The next reference frame.*/
# define OD_FRAME_NEXT (2)
/*The current frame.*/
# define OD_FRAME_SELF (3)

# define OD_FRAME_MAX  (3)

/*Frame types.*/
# define OD_I_FRAME (0)
# define OD_P_FRAME (1)
# define OD_B_FRAME (2)

/*Frame subtypes that need to be tracked separately by rate control.
  Keep these contiguous with but past the end of the main frame types above.*/
# define OD_GOLDEN_P_FRAME (3)
# define OD_FRAME_NSUBTYPES (4)

/*Prediction modes.*/
# define OD_FORWARD_PRED (1)
# define OD_BACKWARD_PRED (2)
# define OD_BIDIR_PRED (3)

/*Constants for the packet state machine common between encoder and decoder.*/

/*Next packet to emit/read: Codec info header.*/
# define OD_PACKET_INFO_HDR    (-3)
/*Next packet to emit/read: Comment header.*/
# define OD_PACKET_COMMENT_HDR (-2)
/*Next packet to emit/read: Codec setup header.*/
# define OD_PACKET_SETUP_HDR   (-1)
/*Next more packets to emit/read.*/
# define OD_PACKET_DONE        (INT_MAX)

extern const int        OD_VERT_D[];
/*The vector offsets in the X direction for each motion compensation block
   vertex from the upper-left.*/
# define OD_VERT_DX (OD_VERT_D+1)
/*The vector offsets in the Y direction for each motion compensation block
   vertex from the upper-left.*/
# define OD_VERT_DY (OD_VERT_D+0)
extern const int *const OD_VERT_SETUP_DX[4][4];
extern const int *const OD_VERT_SETUP_DY[4][4];

/*This should be a power of 2, and at least 8.*/
# define OD_UMV_CLAMP (32)
/*Half the value of OD_SUBPEL_FILTER_TAP_SIZE in mc.h.*/
# define OD_RESAMPLE_PADDING (3)
/*Buffer padding alignment dictated by SIMD.*/
# define OD_PADDING_ALIGN (32)
/*Actual size of padding on all sides of reference buffers.*/
# define OD_BUFFER_PADDING \
 ((OD_UMV_CLAMP + OD_RESAMPLE_PADDING + OD_PADDING_ALIGN - 1) \
 /OD_PADDING_ALIGN*OD_PADDING_ALIGN)

/*The shared (encoder and decoder) functions that have accelerated variants.*/
struct od_state_opt_vtbl{
  void (*mc_predict1fmv)(od_state *state, unsigned char *_dst,
   const unsigned char *_src, int _systride, int32_t _mvx, int32_t _mvy,
   int _log_xblk_sz, int _log_yblk_sz);
  void (*mc_blend_full)(unsigned char *_dst, int _dystride,
   const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
  void (*mc_blend_full_split)(unsigned char *_dst, int _dystride,
   const unsigned char *_src[4], int _c, int _s,
   int _log_xblk_sz, int _log_yblk_sz);
  void (*mc_blend_multi)(unsigned char *_dst, int _dystride,
   const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
  void (*mc_blend_multi_split)(unsigned char *_dst, int _dystride,
   const unsigned char *_src[4], int _c, int _s,
   int _log_xblk_sz, int _log_yblk_sz);
  struct od_dering_opt_vtbl dering;
  void (*restore_fpu)(void);
  od_dct_func_2d fdct_2d[OD_NBSIZES + 1];
  od_dct_func_2d idct_2d[OD_NBSIZES + 1];
  od_copy_nxn_func od_copy_nxn[OD_LOG_COPYBSIZE_MAX + 1];
};

# if defined(OD_DUMP_IMAGES) || defined(OD_DUMP_RECONS)
struct od_yuv_dumpfile{
  char tag[16];
  FILE *fd;
};
# endif


struct od_adapt_ctx {
  /* Support for PVQ encode/decode */
  od_pvq_adapt_ctx pvq;
  /* Motion vectors */
  generic_encoder mv_model;
  uint16_t mv_ref_cdf[5][16];
  int mv_ex[OD_MC_NLEVELS];
  int mv_ey[OD_MC_NLEVELS];
  uint16_t mv_small_cdf[5][16];
  int mv_small_increment;
  uint16_t split_flag_cdf[OD_MC_LEVEL_MAX][9][2];
  int split_flag_increment;

  generic_encoder model_dc[OD_NPLANES_MAX];

  int ex_sb_dc[OD_NPLANES_MAX];
  int ex_dc[OD_NPLANES_MAX][OD_NBSIZES][3];
  int ex_g[OD_NPLANES_MAX][OD_NBSIZES];

  /* Joint skip flag for DC and AC */
  uint16_t skip_cdf[OD_NBSIZES*2][5];
  int skip_increment;
  uint16_t haar_coeff_cdf[15*3*(OD_NBSIZES + 1)][16];
  int haar_coeff_increment;
  uint16_t haar_split_cdf[15*2*5][16];
  int haar_split_increment;
  uint16_t haar_bits_cdf[3][16];
  int haar_bits_increment;
  uint16_t dering_cdf[2*OD_DERING_LEVELS - 1][OD_DERING_LEVELS];
  int dering_increment;
  /* 4 possible values for the sblock above (or skip), 4 for the sblock to the
     left (or skip). */
  uint16_t q_cdf[4*4][4];
  int q_increment;
};

typedef struct od_output_frame od_output_frame;

struct od_output_frame {
  daala_image *img;
  int number;
};

typedef struct od_output_queue od_output_queue;

struct od_output_queue {
  unsigned char *output_img_data;

  /* Circular queue of frame images in decode order. */
  daala_image images[OD_MAX_REORDER];
  int decode_index;
  int decode_used[OD_MAX_REORDER];

  /* Circular queue of frame indices in output order. */
  od_output_frame frames[OD_MAX_REORDER];
  int output_index;
  int output_used[OD_MAX_REORDER];
};

int od_output_queue_init(od_output_queue *out, od_state *state);
void od_output_queue_clear(od_output_queue *out);
int od_output_queue_add(od_output_queue *out, daala_image *img, int number);
#define od_output_queue_has_next(out) \
 ((out)->output_used[(out)->output_index])
od_output_frame *od_output_queue_next(od_output_queue *out);

struct od_state{
  od_adapt_ctx        adapt;
  daala_info          info;
  unsigned char  *mc_buf[5];
  od_state_opt_vtbl   opt_vtbl;
  uint32_t        cpu_flags;
  int32_t         frame_width;
  int32_t         frame_height;
  /** Buffer for the reference images. */
  int                 ref_imgi[OD_FRAME_MAX+1];
  /** Pointers to the ref images so one can move them around without coping
      them. */
  daala_image         ref_imgs[OD_FRAME_MAX+1];
  /* ----------------------------------------------------- */
  /** I,P,B frame type of current frame. */
  int frame_type;
  /** Tail pointer of out_imgs[]. */
  int out_buff_ptr;
  /** Head pointer of out_imgs[]. */
  int out_buff_head;
  /** # of frames left in output buffer to display. */
  int frames_in_out_buff;
  /* ----------------------------------------------------- */
  unsigned char *ref_img_data;
  /** Increments by 1 for each frame. */
  int64_t         cur_time;
  od_mv_grid_pt **mv_grid;

  /** Number of horizontal motion-vector blocks. */
  int                 nhmvbs;
  /** Number of vertical motion-vector blocks. */
  int                 nvmvbs;
  int                 nhsb;
  int                 nvsb;
  /** Each 8x8 block of pixels in the image (+ one superblock of
      padding on each side) has a corresponding byte in this array, and
      every 64x64 superblock is represented by 64 (8 by 8) entries
      ((8 * 8) * (8 * 8) == 64 * 64) that encode the block size decisions
      for the superblock. The entry format is:
      - 0 means the 8x8 block has been split into 4x4 blocks
      - 1 means the 8x8 block is an 8x8 block
      - 2 means the 8x8 block is part of a 16x16 block
      - 3 means the 8x8 block is part of a 32x32 block.
      - 4 means the 8x8 block is part of a 64x64 block.
      The padding is filled as though it consisted of 64x64 blocks.

      E.g., `state->bsize[j * state->bstride + i]` accesses the i'th 8x8
      block in the j'th row of 8x8 blocks.

      The `bstride` member has the distance between vertically adjacent
      entries (horizontally adjacent entries are adjacent in memory). */
  unsigned char *bsize;
  int                 bstride;
  unsigned char *bskip[3];
  int                 skip_stride;
  od_coeff           *(sb_dc_mem[OD_NPLANES_MAX]);
  int                 mv_res; /* 0: 1/8, 1:1/4, 2: 1/2 pel */
# if defined(OD_DUMP_IMAGES) || defined(OD_DUMP_RECONS)
  int                 dump_tags;
  od_yuv_dumpfile    *dump_files;
# endif
  od_coeff *ctmp[OD_NPLANES_MAX];
  od_coeff *dtmp[OD_NPLANES_MAX];
  int16_t *etmp[OD_NPLANES_MAX];
  od_coeff *mctmp[OD_NPLANES_MAX];
  od_coeff *mdtmp[OD_NPLANES_MAX];
  od_coeff *ltmp[OD_NPLANES_MAX];
  od_coeff *lbuf[OD_NPLANES_MAX];
  /** The quantizer field is the actual value used for quantization. */
  int quantizer;
  /** The coded_quantizer is the quantizer index coded to the bitstream. */
  int coded_quantizer;
  unsigned char pvq_qm_q4[OD_NPLANES_MAX][OD_QM_SIZE];
  /*Array of flags to enable the dering filter per block.
    1 to enable (default), 0 to disable.*/
  unsigned char *dering_level;
  /*This provides context for the quantizer CDF.*/
  unsigned char *sb_q_scaling;
  /*Magnitude compensated quantization matrices and their inverses.
   1 per block-size and decimation factor (i.e. OD_NBSIZES*2*(OD_BSIZE_MAX^2)),
   assuming 2 possible decimation values (see OD_BASIS_MAG).*/
  int16_t *qm;
  int16_t *qm_inv;
};

void *od_aligned_malloc(size_t _sz,size_t _align);
void od_aligned_free(void *_ptr);
int od_state_init(od_state *_state, const daala_info *_info);
void od_state_clear(od_state *_state);

void od_img_plane_copy(daala_image *dest, daala_image *src, int pli);
void od_img_copy(daala_image *dest, daala_image *src);
void od_adapt_ctx_reset(od_adapt_ctx *state, int is_keyframe);
void od_state_set_mv_res(od_state *state, int mv_res);
void od_state_pred_block_from_setup(od_state *state, unsigned char *buf,
 int ystride, int pli, int vx, int vy, int c, int s, int log_mvb_sz);
void od_state_pred_block(od_state *state, unsigned char *buf,
 int ystride, int xstride, int pli, int vx, int vy, int log_mvb_sz);
void od_state_mc_predict(od_state *state, daala_image *dst);
void od_state_init_border(od_state *state);
void od_state_init_superblock_split(od_state *state, unsigned char bsize);
int od_state_dump_yuv(od_state *state, daala_image *img, const char *tag);
void od_img_edge_ext(daala_image* src);
void od_ref_buf_to_coeff(od_state *state,
 od_coeff *dst, int dst_ystride, int lossless_p,
 unsigned char *src, int src_xstride, int src_ystride,
 int w, int h);
void od_ref_plane_to_coeff(od_state *state, od_coeff *dst, int lossless_p,
 daala_image *src, int pli);
void od_coeff_to_ref_buf(od_state *state,
 unsigned char *dst, int dst_xstride, int dst_ystride,
 od_coeff *src, int src_ystride, int lossless_p,
 int w, int h);
void od_coeff_to_ref_plane(od_state *state, daala_image *dst, int pli,
 od_coeff *src, int lossless_p);
# if defined(OD_DUMP_IMAGES)
int od_state_dump_img(od_state *state, daala_image *img, const char *tag);
# endif
void od_init_skipped_coeffs(od_coeff *d, od_coeff *pred, int is_keyframe,
 int bo, int n, int w);


/*Shared accelerated functions.*/

/*Default pure-C implementations.*/
void od_mc_predict1fmv8_c(od_state *state, unsigned char *_dst,
 const unsigned char *_src, int _systride, int32_t _mvx, int32_t _mvy,
 int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_full8_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_full_split8_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _c, int _s, int _log_xblk_sz,
 int _log_yblk_sz);
void od_mc_blend_multi8_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_multi_split8_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _c, int _s, int _log_xblk_sz,
 int _log_yblk_sz);
void od_mc_predict1fmv16_c(od_state *state, unsigned char *_dst,
 const unsigned char *_src, int _systride, int32_t _mvx, int32_t _mvy,
 int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_full16_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_full_split16_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _c, int _s, int _log_xblk_sz,
 int _log_yblk_sz);
void od_mc_blend_multi16_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _log_xblk_sz, int _log_yblk_sz);
void od_mc_blend_multi_split16_c(unsigned char *_dst, int _dystride,
 const unsigned char *_src[4], int _c, int _s, int _log_xblk_sz,
 int _log_yblk_sz);
void od_restore_fpu(od_state *state);

void od_state_opt_vtbl_init_c(od_state *_state);

#endif

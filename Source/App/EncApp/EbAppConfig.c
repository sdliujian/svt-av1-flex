﻿/*
* Copyright(c) 2019 Intel Corporation
*
* This source code is subject to the terms of the BSD 3-Clause Clear License and
* the Alliance for Open Media Patent License 1.0. If the BSD 3-Clause Clear License
* was not distributed with this source code in the LICENSE file, you can
* obtain it at https://www.aomedia.org/license. If the Alliance for Open
* Media Patent License 1.0 was not distributed with this source code in the
* PATENTS file, you can obtain it at https://www.aomedia.org/license/patent-license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "EbSvtAv1Metadata.h"
#include "EbAppConfig.h"
#include "EbAppContext.h"
#include "EbAppInputy4m.h"
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

#include "EbAppOutputivf.h"

#if !defined(_WIN32) || !defined(HAVE_STRNLEN_S)
#include "third_party/safestringlib/safe_str_lib.h"
#endif

/**********************************
 * Defines
 **********************************/
#define HELP_TOKEN "--help"
#define VERSION_TOKEN "--version"
#define CHANNEL_NUMBER_TOKEN "--nch"
#define COMMAND_LINE_MAX_SIZE 2048
#define CONFIG_FILE_TOKEN "-c"
#define CONFIG_FILE_LONG_TOKEN "--config"
#define INPUT_FILE_TOKEN "-i"
#define OUTPUT_BITSTREAM_TOKEN "-b"
#define OUTPUT_RECON_TOKEN "-o"
#define ERROR_FILE_TOKEN "--errlog"

/* two pass token */
#define PASS_TOKEN "--pass"
#define TWO_PASS_STATS_TOKEN "--stats"
#define PASSES_TOKEN "--passes"
#define STAT_FILE_TOKEN "--stat-file"
#if !REMOVE_MANUAL_PRED
#define INPUT_PREDSTRUCT_FILE_TOKEN "--pred-struct-file"
#endif
#define WIDTH_TOKEN "-w"
#define HEIGHT_TOKEN "-h"
#define NUMBER_OF_PICTURES_TOKEN "-n"
#define BUFFERED_INPUT_TOKEN "--nb"
#define NO_PROGRESS_TOKEN "--no-progress" // tbd if it should be removed
#define PROGRESS_TOKEN "--progress"
#define QP_TOKEN "-q"
#define USE_QP_FILE_TOKEN "--use-q-file"
#define FORCE_KEY_FRAMES_TOKEN "--force-key-frames"

#define USE_FIXED_QINDEX_OFFSETS_TOKEN "--use-fixed-qindex-offsets"
#define QINDEX_OFFSETS_TOKEN "--qindex-offsets"
#define KEY_FRAME_QINDEX_OFFSET_TOKEN "--key-frame-qindex-offset"
#define KEY_FRAME_CHROMA_QINDEX_OFFSET_TOKEN "--key-frame-chroma-qindex-offset"
#define CHROMA_QINDEX_OFFSETS_TOKEN "--chroma-qindex-offsets"
#define LUMA_Y_DC_QINDEX_OFFSET_TOKEN "--luma-y-dc-qindex-offset"
#define CHROMA_U_DC_QINDEX_OFFSET_TOKEN "--chroma-u-dc-qindex-offset"
#define CHROMA_U_AC_QINDEX_OFFSET_TOKEN "--chroma-u-ac-qindex-offset"
#define CHROMA_V_DC_QINDEX_OFFSET_TOKEN "--chroma-v-dc-qindex-offset"
#define CHROMA_V_AC_QINDEX_OFFSET_TOKEN "--chroma-v-ac-qindex-offset"

#define FRAME_RATE_TOKEN "--fps"
#define FRAME_RATE_NUMERATOR_TOKEN "--fps-num"
#define FRAME_RATE_DENOMINATOR_TOKEN "--fps-denom"
#define ENCODER_COLOR_FORMAT "--color-format"
#define INPUT_COMPRESSED_TEN_BIT_FORMAT "--compressed-ten-bit-format"
#define HIERARCHICAL_LEVELS_TOKEN "--hierarchical-levels" // no Eval
#define PRED_STRUCT_TOKEN "--pred-struct"
#define PROFILE_TOKEN "--profile"
#define INTRA_PERIOD_TOKEN "--intra-period"
#define TIER_TOKEN "--tier"
#define LEVEL_TOKEN "--level"
#define FILM_GRAIN_TOKEN "--film-grain"
#define FILM_GRAIN_DENOISE_APPLY_TOKEN "--film-grain-denoise"
#define INTRA_REFRESH_TYPE_TOKEN "--irefresh-type" // no Eval
#define CDEF_ENABLE_TOKEN "--enable-cdef"
#define SCREEN_CONTENT_TOKEN "--scm"
// --- start: ALTREF_FILTERING_SUPPORT
#define ENABLE_TF_TOKEN "--enable-tf"
#define ENABLE_OVERLAYS "--enable-overlays"
#define TUNE_TOKEN "--tune"
// --- end: ALTREF_FILTERING_SUPPORT
// --- start: SUPER-RESOLUTION SUPPORT
#define SUPERRES_MODE_INPUT "--superres-mode"
#define SUPERRES_DENOM "--superres-denom"
#define SUPERRES_KF_DENOM "--superres-kf-denom"
#define SUPERRES_QTHRES "--superres-qthres"
#define SUPERRES_KF_QTHRES "--superres-kf-qthres"
// --- end: SUPER-RESOLUTION SUPPORT
// --- start: REFERENCE SCALING SUPPORT
#define RESIZE_MODE_INPUT "--resize-mode"
#define RESIZE_DENOM "--resize-denom"
#define RESIZE_KF_DENOM "--resize-kf-denom"
// --- end: REFERENCE SCALING SUPPORT
#define RATE_CONTROL_ENABLE_TOKEN "--rc"
#define TARGET_BIT_RATE_TOKEN "--tbr"
#define MAX_BIT_RATE_TOKEN "--mbr"
#define MAX_QP_TOKEN "--max-qp"
#define MIN_QP_TOKEN "--min-qp"
#define VBR_BIAS_PCT_TOKEN "--bias-pct"
#define VBR_MIN_SECTION_PCT_TOKEN "--minsection-pct"
#define VBR_MAX_SECTION_PCT_TOKEN "--maxsection-pct"
#define UNDER_SHOOT_PCT_TOKEN "--undershoot-pct"
#define OVER_SHOOT_PCT_TOKEN "--overshoot-pct"
#define MBR_OVER_SHOOT_PCT_TOKEN "--mbr-overshoot-pct"
#define GOP_CONSTRAINT_RC_TOKEN "--gop-constraint-rc"
#define BUFFER_SIZE_TOKEN "--buf-sz"
#define BUFFER_INITIAL_SIZE_TOKEN "--buf-initial-sz"
#define BUFFER_OPTIMAL_SIZE_TOKEN "--buf-optimal-sz"
#define RECODE_LOOP_TOKEN "--recode-loop"
#define ENABLE_TPL_LA_TOKEN "--enable-tpl-la"
#define SUPER_BLOCK_SIZE_TOKEN "--sb-size"
#define TILE_ROW_TOKEN "--tile-rows"
#define TILE_COL_TOKEN "--tile-columns"

#define SCENE_CHANGE_DETECTION_TOKEN "--scd"
#define INJECTOR_TOKEN "--inj" // no Eval
#define INJECTOR_FRAMERATE_TOKEN "--inj-frm-rt" // no Eval
#define ASM_TYPE_TOKEN "--asm"
#define THREAD_MGMNT "--lp"
#define PIN_TOKEN "--pin"
#define TARGET_SOCKET "--ss"
#define RESTRICTED_MOTION_VECTOR "--rmv"
#define CONFIG_FILE_COMMENT_CHAR '#'
#define CONFIG_FILE_NEWLINE_CHAR '\n'
#define CONFIG_FILE_RETURN_CHAR '\r'
#define CONFIG_FILE_VALUE_SPLIT ':'
#define CONFIG_FILE_SPACE_CHAR ' '
#define CONFIG_FILE_ARRAY_SEP_CHAR CONFIG_FILE_SPACE_CHAR
#define CONFIG_FILE_TAB_CHAR '\t'
#define CONFIG_FILE_NULL_CHAR '\0'
#define CONFIG_FILE_MAX_ARG_COUNT 256
#define CONFIG_FILE_MAX_VAR_LEN 128
#define EVENT_FILE_MAX_ARG_COUNT 20
#define EVENT_FILE_MAX_VAR_LEN 256
#define BUFFER_FILE_MAX_ARG_COUNT 320
#define BUFFER_FILE_MAX_VAR_LEN 128

//double dash
#define PRESET_TOKEN "--preset"
#define QP_FILE_NEW_TOKEN "--qpfile"
#define INPUT_DEPTH_TOKEN "--input-depth"
#define KEYINT_TOKEN "--keyint"
#define LOOKAHEAD_NEW_TOKEN "--lookahead"
#define SVTAV1_PARAMS "--svtav1-params"

#define STAT_REPORT_NEW_TOKEN "--enable-stat-report"
#define ENABLE_RESTORATION_TOKEN "--enable-restoration"
#define MFMV_ENABLE_NEW_TOKEN "--enable-mfmv"
#define FAST_DECODE_TOKEN "--fast-decode"
#define HDR_INPUT_NEW_TOKEN "--enable-hdr"
#define ADAPTIVE_QP_ENABLE_NEW_TOKEN "--aq-mode"
#define INPUT_FILE_LONG_TOKEN "--input"
#define OUTPUT_BITSTREAM_LONG_TOKEN "--output"
#define OUTPUT_RECON_LONG_TOKEN "--recon"
#define WIDTH_LONG_TOKEN "--width"
#define HEIGHT_LONG_TOKEN "--height"
#define NUMBER_OF_PICTURES_LONG_TOKEN "--frames"
#define QP_LONG_TOKEN "--qp"
#define CRF_LONG_TOKEN "--crf"
#define LOOP_FILTER_ENABLE "--enable-dlf"
#define FORCED_MAX_FRAME_WIDTH_TOKEN "--forced-max-frame-width"
#define FORCED_MAX_FRAME_HEIGHT_TOKEN "--forced-max-frame-height"

#define COLOR_PRIMARIES_NEW_TOKEN "--color-primaries"
#define TRANSFER_CHARACTERISTICS_NEW_TOKEN "--transfer-characteristics"
#define MATRIX_COEFFICIENTS_NEW_TOKEN "--matrix-coefficients"
#define COLOR_RANGE_NEW_TOKEN "--color-range"
#define CHROMA_SAMPLE_POSITION_TOKEN "--chroma-sample-position"
#define MASTERING_DISPLAY_TOKEN "--mastering-display"
#define CONTENT_LIGHT_LEVEL_TOKEN "--content-light"

#define SFRAME_DIST_TOKEN "--sframe-dist"
#define SFRAME_MODE_TOKEN "--sframe-mode"

#define ENABLE_QM_TOKEN "--enable-qm"
#define MIN_QM_LEVEL_TOKEN "--qm-min"
#define MAX_QM_LEVEL_TOKEN "--qm-max"
#ifdef _WIN32
static HANDLE get_file_handle(FILE *fp) { return (HANDLE)_get_osfhandle(_fileno(fp)); }
#endif

static Bool fopen_and_lock(FILE **file, const char *name, Bool write) {
    if (!file || !name)
        return FALSE;

    const char *mode = write ? "wb" : "rb";
    FOPEN(*file, name, mode);
    if (!*file)
        return FALSE;

#ifdef _WIN32
    HANDLE handle = get_file_handle(*file);
    if (handle == INVALID_HANDLE_VALUE)
        return FALSE;
    if (LockFile(handle, 0, 0, MAXDWORD, MAXDWORD))
        return TRUE;
#else
    int fd = fileno(*file);
    if (flock(fd, LOCK_EX | LOCK_NB) == 0)
        return TRUE;
#endif
    fprintf(stderr, "ERROR: locking %s failed, is it used by other encoder?\n", name);
    return FALSE;
}

/**********************************
 * Set Cfg Functions
 **********************************/
static void set_cfg_input_file(const char *filename, EbConfig *cfg) {
    if (cfg->input_file && !cfg->input_file_is_fifo)
        fclose(cfg->input_file);

    if (!filename) {
        cfg->input_file = NULL;
        return;
    }

    if (!strcmp(filename, "stdin") || !strcmp(filename, "-")) {
        cfg->input_file         = stdin;
        cfg->input_file_is_fifo = TRUE;
    } else
        FOPEN(cfg->input_file, filename, "rb");

    if (cfg->input_file == NULL) {
        return;
    }
    if (cfg->input_file != stdin) {
#ifdef _WIN32
        HANDLE handle = (HANDLE)_get_osfhandle(_fileno(cfg->input_file));
        if (handle == INVALID_HANDLE_VALUE)
            return;
        cfg->input_file_is_fifo = GetFileType(handle) == FILE_TYPE_PIPE;
#else
        int         fd = fileno(cfg->input_file);
        struct stat statbuf;
        fstat(fd, &statbuf);
        cfg->input_file_is_fifo = S_ISFIFO(statbuf.st_mode);
#endif
    }

    cfg->y4m_input = check_if_y4m(cfg);
};

#if !REMOVE_MANUAL_PRED
static void set_pred_struct_file(const char *value, EbConfig *cfg) {
    if (cfg->input_pred_struct_filename)
        free(cfg->input_pred_struct_filename);
#ifndef _WIN32
    cfg->input_pred_struct_filename = strdup(value);
#else
    cfg->input_pred_struct_filename = _strdup(value);
#endif
    cfg->config.enable_manual_pred_struct = TRUE;
}
#endif

static void set_cfg_stream_file(const char *value, EbConfig *cfg) {
    if (cfg->bitstream_file && cfg->bitstream_file != stdout) {
        fclose(cfg->bitstream_file);
    }

    if (!strcmp(value, "stdout") || !strcmp(value, "-")) {
        cfg->bitstream_file = stdout;
    } else {
        FOPEN(cfg->bitstream_file, value, "wb");
    }
};
static void set_cfg_error_file(const char *value, EbConfig *cfg) {
    if (cfg->error_log_file && cfg->error_log_file != stderr) {
        fclose(cfg->error_log_file);
    }
    FOPEN(cfg->error_log_file, value, "w+");
};
static void set_cfg_recon_file(const char *value, EbConfig *cfg) {
    if (cfg->recon_file) {
        fclose(cfg->recon_file);
    }
    FOPEN(cfg->recon_file, value, "wb");
};
static void set_cfg_qp_file(const char *value, EbConfig *cfg) {
    if (cfg->qp_file) {
        fclose(cfg->qp_file);
    }
    FOPEN(cfg->qp_file, value, "r");
};
static void set_pass(const char *value, EbConfig *cfg) {
    cfg->config.pass = strtol(value, NULL, 0);
}
static void set_two_pass_stats(const char *value, EbConfig *cfg) {
#ifndef _WIN32
    cfg->stats = strdup(value);
#else
    cfg->stats                      = _strdup(value);
#endif
}

static void set_passes(const char *value, EbConfig *cfg) {
    (void)value;
    (void)cfg;
    /* empty function, we will handle passes at higher level*/
    return;
}
static void set_cfg_stat_file(const char *value, EbConfig *cfg) {
    if (cfg->stat_file) {
        fclose(cfg->stat_file);
    }
    FOPEN(cfg->stat_file, value, "wb");
};
static void set_stat_report(const char *value, EbConfig *cfg) {
    cfg->config.stat_report = (uint8_t)strtoul(value, NULL, 0);
};
static void set_cfg_source_width(const char *value, EbConfig *cfg) {
    cfg->config.source_width = strtoul(value, NULL, 0);
};
static void set_cfg_source_height(const char *value, EbConfig *cfg) {
    cfg->config.source_height = strtoul(value, NULL, 0);
};
static void set_cfg_forced_max_frame_width(const char *value, EbConfig *cfg) {
    cfg->config.forced_max_frame_width = strtoul(value, NULL, 0);
}
static void set_cfg_forced_max_frame_height(const char *value, EbConfig *cfg) {
    cfg->config.forced_max_frame_height = strtoul(value, NULL, 0);
}
static void set_cfg_frames_to_be_encoded(const char *value, EbConfig *cfg) {
    cfg->frames_to_be_encoded = strtol(value, NULL, 0);
};
static void set_buffered_input(const char *value, EbConfig *cfg) {
    cfg->buffered_input = strtol(value, NULL, 0);
};
static void set_cfg_force_key_frames(const char *value, EbConfig *cfg) {
    struct forced_key_frames fkf;
    fkf.specifiers = NULL;
    fkf.frames     = NULL;
    fkf.count      = 0;

    if (!value)
        return;
    const char *p = value;
    while (p) {
        const size_t len       = strcspn(p, ",");
        char        *specifier = (char *)calloc(sizeof(*specifier), len + 1);
        if (!specifier)
            goto err;
        memcpy(specifier, p, len);
        char **tmp = (char **)realloc(fkf.specifiers, sizeof(*fkf.specifiers) * (fkf.count + 1));
        if (!tmp) {
            free(specifier);
            goto err;
        }
        fkf.specifiers            = tmp;
        fkf.specifiers[fkf.count] = specifier;
        fkf.count++;
        if ((p = strchr(p, ',')))
            ++p;
    }

    if (!fkf.count)
        goto err;

    fkf.frames = (uint64_t *)calloc(sizeof(*fkf.frames), fkf.count);
    if (!fkf.frames)
        goto err;
    cfg->forced_keyframes        = fkf;
    cfg->config.force_key_frames = true;
    return;
err:
    fputs("Error parsing forced key frames list\n", stderr);
    for (size_t i = 0; i < fkf.count; ++i) free(fkf.specifiers[i]);
    free(fkf.specifiers);
}
static void set_no_progress(const char *value, EbConfig *cfg) {
    switch (value ? *value : '1') {
    case '0': cfg->progress = 1; break; // equal to --progress 1
    default: cfg->progress = 0; break; // equal to --progress 0
    }
}
static void set_progress(const char *value, EbConfig *cfg) {
    switch (value ? *value : '1') {
    case '0': cfg->progress = 0; break; // no progress printed
    case '2': cfg->progress = 2; break; // aomenc style progress
    default: cfg->progress = 1; break; // default progress
    }
}
static void set_frame_rate(const char *value, EbConfig *cfg) {
    cfg->config.frame_rate_numerator   = strtoul(value, NULL, 0);
    cfg->config.frame_rate_denominator = 1;
}

static void set_frame_rate_numerator(const char *value, EbConfig *cfg) {
    cfg->config.frame_rate_numerator = strtoul(value, NULL, 0);
};
static void set_frame_rate_denominator(const char *value, EbConfig *cfg) {
    cfg->config.frame_rate_denominator = strtoul(value, NULL, 0);
};
static void set_encoder_bit_depth(const char *value, EbConfig *cfg) {
    cfg->config.encoder_bit_depth = strtoul(value, NULL, 0);
}
static void set_encoder_color_format(const char *value, EbConfig *cfg) {
    cfg->config.encoder_color_format = (EbColorFormat)strtoul(value, NULL, 0);
}
static void set_compressed_ten_bit_format(const char *value, EbConfig *cfg) {
    cfg->config.compressed_ten_bit_format = strtoul(value, NULL, 0);
}
static void set_enc_mode(const char *value, EbConfig *cfg) {
    cfg->config.enc_mode = (uint8_t)strtoul(value, NULL, 0);
};
static void set_enable_qm(const char *value, EbConfig *cfg) {
    cfg->config.enable_qm = !!strtoul(value, NULL, 0);
};
static void set_min_qm_level(const char *value, EbConfig *cfg) {
    cfg->config.min_qm_level = (uint8_t)strtoul(value, NULL, 0);
};
static void set_max_qm_level(const char *value, EbConfig *cfg) {
    cfg->config.max_qm_level = (uint8_t)strtoul(value, NULL, 0);
};
/**
 * @brief split colon separated string into key=value pairs
 *
 * @param[in]  str colon separated string of key=val
 * @param[out] opt key and val, both need to be freed
 */
struct ParseOpt {
    char *key;
    char *val;
};

static struct ParseOpt split_colon_keyequalval_pairs(const char **p) {
    const char     *str        = *p;
    struct ParseOpt opt        = {NULL, NULL};
    const size_t    string_len = strcspn(str, ":");

    const char *val = strchr(str, '=');
    if (!val || !*++val)
        return opt;

    const size_t key_len = val - str - 1;
    const size_t val_len = string_len - key_len - 1;
    if (!key_len || !val_len)
        return opt;

    opt.key = (char *)malloc(key_len + 1);
    opt.val = (char *)malloc(val_len + 1);
    if (!opt.key || !opt.val) {
        free(opt.key);
        opt.key = NULL;
        free(opt.val);
        opt.val = NULL;
        return opt;
    }

    memcpy(opt.key, str, key_len);
    memcpy(opt.val, val, val_len);
    opt.key[key_len] = '\0';
    opt.val[val_len] = '\0';
    str += string_len;
    if (*str)
        str++;
    *p = str;
    return opt;
}

static void parse_svtav1_params(const char *value, EbConfig *cfg) {
    const char *p = value;
    while (*p) {
        struct ParseOpt opt = split_colon_keyequalval_pairs(&p);
        if (!opt.key || !opt.val)
            continue;
        if (EB_ErrorBadParameter == svt_av1_enc_parse_parameter(&cfg->config, opt.key, opt.val))
            fprintf(
                stderr, "Warning: failed to set parameter '%s' with key '%s'\n", opt.key, opt.val);
        free(opt.key);
        free(opt.val);
    }
}

static void set_cfg_intra_period(const char *value, EbConfig *cfg) {
    cfg->config.intra_period_length = strtol(value, NULL, 0);
};
static void set_keyint(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "keyint", value);
}
static void set_cfg_intra_refresh_type(const char *value, EbConfig *cfg) {
    switch (strtol(value, NULL, 0)) {
    case 1: cfg->config.intra_refresh_type = SVT_AV1_FWDKF_REFRESH; break;
    default: cfg->config.intra_refresh_type = SVT_AV1_KF_REFRESH; break;
    }
};
static void set_look_ahead_distance(const char *value, EbConfig *cfg) {
    cfg->config.look_ahead_distance = strtol(value, NULL, 0);
};
static void set_hierarchical_levels(const char *value, EbConfig *cfg) {
    cfg->config.hierarchical_levels = strtol(value, NULL, 0);
};
static void set_cfg_pred_structure(const char *value, EbConfig *cfg) {
    cfg->config.pred_structure = (uint8_t)strtol(value, NULL, 0);
};
static void set_cfg_qp(const char *value, EbConfig *cfg) {
    cfg->config.qp = strtoul(value, NULL, 0);
};
static void set_cfg_crf(const char *value, EbConfig *cfg) {
    cfg->config.qp                           = strtoul(value, NULL, 0);
    cfg->config.rate_control_mode            = SVT_AV1_RC_MODE_CQP_OR_CRF;
    cfg->config.enable_adaptive_quantization = 2;
}
static void set_cfg_use_qp_file(const char *value, EbConfig *cfg) {
    cfg->config.use_qp_file = (Bool)strtol(value, NULL, 0);
};

static void set_cfg_use_fixed_qindex_offsets(const char *value, EbConfig *cfg) {
    cfg->config.use_fixed_qindex_offsets = (uint8_t)strtol(value, NULL, 0);
}

static void set_cfg_key_frame_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.key_frame_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_key_frame_chroma_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.key_frame_chroma_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_luma_y_dc_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.luma_y_dc_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_chroma_u_dc_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.chroma_u_dc_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_chroma_u_ac_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.chroma_u_ac_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_chroma_v_dc_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.chroma_v_dc_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

static void set_cfg_chroma_v_ac_qindex_offset(const char *value, EbConfig *cfg) {
    cfg->config.chroma_v_ac_qindex_offset = (int32_t)strtol(value, NULL, 0);
}

//assume the input list of values are in the format of "[v1,v2,v3,...]"
int arg_parse_list(const char *value, int *list, int n) {
    const char *ptr = value;
    char       *endptr;
    int         i = 0;
    memset(list, 0, n);
    while (ptr[0] != '\0') {
        if (ptr[0] == '[' || ptr[0] == ']') {
            ptr++;
            continue;
        }

        int32_t rawval = (int32_t)strtol(ptr, &endptr, 10);
        if (i >= n) {
            fprintf(stderr, "List has more than %d entries\n", n);
            exit(1);
        } else if (*endptr == ',' || *endptr == ']') {
            endptr++;
        } else if (*endptr != '\0') {
            fprintf(stderr, "Bad list separator '%c'\n", *endptr);
            exit(1);
        }
        list[i++] = (int)rawval;
        ptr       = endptr;
    }
    return i;
}

static void set_cfg_qindex_offsets(const char *value, EbConfig *cfg) {
    arg_parse_list(value, cfg->config.qindex_offsets, EB_MAX_TEMPORAL_LAYERS);
}

static void set_cfg_chroma_qindex_offsets(const char *value, EbConfig *cfg) {
    arg_parse_list(value, cfg->config.chroma_qindex_offsets, EB_MAX_TEMPORAL_LAYERS);
}

static void set_cfg_film_grain(const char *value, EbConfig *cfg) {
    cfg->config.film_grain_denoise_strength = strtol(value, NULL, 0);
}; //not bool to enable possible algorithm extension in the future
static void set_cfg_film_grain_denoise_apply(const char *value, EbConfig *cfg) {
    cfg->config.film_grain_denoise_apply = (Bool)strtol(value, NULL, 0);
};
static void set_enable_dlf_flag(const char *value, EbConfig *cfg) {
    cfg->config.enable_dlf_flag = !!strtoul(value, NULL, 0);
}
static void set_cdef_enable(const char *value, EbConfig *cfg) {
    // Set CDEF to either DEFAULT or 0
    cfg->config.cdef_level = -!!strtoul(value, NULL, 0);
};
static void set_enable_restoration_flag(const char *value, EbConfig *cfg) {
    // Set loop restoration to either DEFAULT(1) or 0
    cfg->config.enable_restoration_filtering = !!strtoul(value, NULL, 0);
};
static void set_enable_mfmv_flag(const char *value, EbConfig *cfg) {
    cfg->config.enable_mfmv = strtol(value, NULL, 0);
};
static void set_fast_decode_flag(const char *value, EbConfig *cfg) {
    cfg->config.fast_decode = (Bool)strtol(value, NULL, 0);
};
static void set_tile_row(const char *value, EbConfig *cfg) {
    cfg->config.tile_rows = strtoul(value, NULL, 0);
};
static void set_tile_col(const char *value, EbConfig *cfg) {
    cfg->config.tile_columns = strtoul(value, NULL, 0);
};
static void set_scene_change_detection(const char *value, EbConfig *cfg) {
    cfg->config.scene_change_detection = strtoul(value, NULL, 0);
}
static void set_enable_tpl_la(const char *value, EbConfig *cfg) {
    cfg->config.enable_tpl_la = (uint8_t)strtoul(value, NULL, 0);
};
static void set_rate_control_mode(const char *value, EbConfig *cfg) {
    if (svt_av1_enc_parse_parameter(&cfg->config, "rc", value) != EB_ErrorNone)
        fprintf(stderr, "Invalid value for rate control mode\n");
};
static void set_target_bit_rate(const char *value, EbConfig *cfg) {
    if (svt_av1_enc_parse_parameter(&cfg->config, "tbr", value) != EB_ErrorNone)
        fprintf(stderr, "Invalid value for target bit rate\n");
};
static void set_max_bit_rate(const char *value, EbConfig *cfg) {
    if (svt_av1_enc_parse_parameter(&cfg->config, "mbr", value) != EB_ErrorNone)
        fprintf(stderr, "Invalid value for max bit rate\n");
};
static void set_max_qp_allowed(const char *value, EbConfig *cfg) {
    cfg->config.max_qp_allowed = strtoul(value, NULL, 0);
};
static void set_min_qp_allowed(const char *value, EbConfig *cfg) {
    cfg->config.min_qp_allowed = strtoul(value, NULL, 0);
};
static void set_vbr_bias_pct(const char *value, EbConfig *cfg) {
    cfg->config.vbr_bias_pct = strtoul(value, NULL, 0);
};
static void set_vbr_min_section_pct(const char *value, EbConfig *cfg) {
    cfg->config.vbr_min_section_pct = strtoul(value, NULL, 0);
};
static void set_vbr_max_section_pct(const char *value, EbConfig *cfg) {
    cfg->config.vbr_max_section_pct = strtoul(value, NULL, 0);
};
static void set_under_shoot_pct(const char *value, EbConfig *cfg) {
    cfg->config.under_shoot_pct = strtoul(value, NULL, 0);
};
static void set_over_shoot_pct(const char *value, EbConfig *cfg) {
    cfg->config.over_shoot_pct = strtoul(value, NULL, 0);
};
static void set_mbr_over_shoot_pct(const char *value, EbConfig *cfg) {
    cfg->config.mbr_over_shoot_pct = strtoul(value, NULL, 0);
}
static void set_gop_constraint_rc(const char *value, EbConfig *cfg) {
    cfg->config.gop_constraint_rc = (Bool)strtoul(value, NULL, 0);
}
static void set_buf_sz(const char *value, EbConfig *cfg) {
    cfg->config.maximum_buffer_size_ms = strtoul(value, NULL, 0);
};
static void set_buf_initial_sz(const char *value, EbConfig *cfg) {
    cfg->config.starting_buffer_level_ms = strtoul(value, NULL, 0);
};
static void set_buf_optimal_sz(const char *value, EbConfig *cfg) {
    cfg->config.optimal_buffer_level_ms = strtoul(value, NULL, 0);
};
static void set_recode_loop(const char *value, EbConfig *cfg) {
    cfg->config.recode_loop = strtoul(value, NULL, 0);
};
static void set_adaptive_quantization(const char *value, EbConfig *cfg) {
    cfg->config.enable_adaptive_quantization = (Bool)strtol(value, NULL, 0);
};
static void set_screen_content_mode(const char *value, EbConfig *cfg) {
    cfg->config.screen_content_mode = strtoul(value, NULL, 0);
};
// --- start: ALTREF_FILTERING_SUPPORT
static void set_enable_tf(const char *value, EbConfig *cfg) {
    cfg->config.enable_tf = !!strtol(value, NULL, 0);
};

static void set_enable_overlays(const char *value, EbConfig *cfg) {
    cfg->config.enable_overlays = (Bool)strtoul(value, NULL, 0);
};

static void set_tune(const char *value, EbConfig *cfg) {
    cfg->config.tune = (Bool)strtoul(value, NULL, 0);
};
// --- end: ALTREF_FILTERING_SUPPORT
// --- start: SUPER-RESOLUTION SUPPORT
static void set_superres_mode(const char *value, EbConfig *cfg) {
    cfg->config.superres_mode = (SUPERRES_MODE)strtoul(value, NULL, 0);
};
static void set_superres_denom(const char *value, EbConfig *cfg) {
    cfg->config.superres_denom = (uint8_t)strtoul(value, NULL, 0);
};
static void set_superres_kf_denom(const char *value, EbConfig *cfg) {
    cfg->config.superres_kf_denom = (uint8_t)strtoul(value, NULL, 0);
};
static void set_superres_qthres(const char *value, EbConfig *cfg) {
    cfg->config.superres_qthres = (uint8_t)strtoul(value, NULL, 0);
};
static void set_superres_kf_qthres(const char *value, EbConfig *cfg) {
    cfg->config.superres_kf_qthres = (uint8_t)strtoul(value, NULL, 0);
};
// --- end: SUPER-RESOLUTION SUPPORT
// --- start: REFERENCE SCALING SUPPORT
static void set_resize_mode(const char *value, EbConfig *cfg) {
    cfg->config.resize_mode = (RESIZE_MODE)strtoul(value, NULL, 0);
};
static void set_resize_denom(const char *value, EbConfig *cfg) {
    cfg->config.resize_denom = (uint8_t)strtoul(value, NULL, 0);
};
static void set_resize_kf_denom(const char *value, EbConfig *cfg) {
    cfg->config.resize_kf_denom = (uint8_t)strtoul(value, NULL, 0);
};
// --- end: REFERENCE SCALING SUPPORT
static void set_high_dynamic_range_input(const char *value, EbConfig *cfg) {
    cfg->config.high_dynamic_range_input = !!strtol(value, NULL, 0);
};
static void set_profile(const char *value, EbConfig *cfg) {
    switch (strtol(value, NULL, 0)) {
    case 1: cfg->config.profile = HIGH_PROFILE; break;
    case 2: cfg->config.profile = PROFESSIONAL_PROFILE; break;
    default: cfg->config.profile = MAIN_PROFILE; break;
    }
};
static void set_tier(const char *value, EbConfig *cfg) {
    cfg->config.tier = strtol(value, NULL, 0);
};
static void set_level(const char *value, EbConfig *cfg) {
    if (strtoul(value, NULL, 0) != 0 || strcmp(value, "0") == 0)
        cfg->config.level = (uint32_t)(10 * strtod(value, NULL));
    else
        cfg->config.level = 9999999;
};
static void set_injector(const char *value, EbConfig *cfg) {
    cfg->injector = strtol(value, NULL, 0);
};

static void set_injector_frame_rate(const char *value, EbConfig *cfg) {
    cfg->injector_frame_rate = strtoul(value, NULL, 0);
}
static void set_asm_type(const char *value, EbConfig *cfg) {
    const struct {
        const char *name;
        EbCpuFlags  flags;
    } param_maps[] = {
        {"c", 0},
        {"0", 0},
        {"mmx", (EB_CPU_FLAGS_MMX << 1) - 1},
        {"1", (EB_CPU_FLAGS_MMX << 1) - 1},
        {"sse", (EB_CPU_FLAGS_SSE << 1) - 1},
        {"2", (EB_CPU_FLAGS_SSE << 1) - 1},
        {"sse2", (EB_CPU_FLAGS_SSE2 << 1) - 1},
        {"3", (EB_CPU_FLAGS_SSE2 << 1) - 1},
        {"sse3", (EB_CPU_FLAGS_SSE3 << 1) - 1},
        {"4", (EB_CPU_FLAGS_SSE3 << 1) - 1},
        {"ssse3", (EB_CPU_FLAGS_SSSE3 << 1) - 1},
        {"5", (EB_CPU_FLAGS_SSSE3 << 1) - 1},
        {"sse4_1", (EB_CPU_FLAGS_SSE4_1 << 1) - 1},
        {"6", (EB_CPU_FLAGS_SSE4_1 << 1) - 1},
        {"sse4_2", (EB_CPU_FLAGS_SSE4_2 << 1) - 1},
        {"7", (EB_CPU_FLAGS_SSE4_2 << 1) - 1},
        {"avx", (EB_CPU_FLAGS_AVX << 1) - 1},
        {"8", (EB_CPU_FLAGS_AVX << 1) - 1},
        {"avx2", (EB_CPU_FLAGS_AVX2 << 1) - 1},
        {"9", (EB_CPU_FLAGS_AVX2 << 1) - 1},
        {"avx512", (EB_CPU_FLAGS_AVX512VL << 1) - 1},
        {"10", (EB_CPU_FLAGS_AVX512VL << 1) - 1},
        {"max", EB_CPU_FLAGS_ALL},
        {"11", EB_CPU_FLAGS_ALL},
    };
    const uint32_t para_map_size = sizeof(param_maps) / sizeof(param_maps[0]);
    uint32_t       i;

    for (i = 0; i < para_map_size; ++i) {
        if (strcmp(value, param_maps[i].name) == 0) {
            cfg->config.use_cpu_flags = param_maps[i].flags;
            return;
        }
    }

    cfg->config.use_cpu_flags = EB_CPU_FLAGS_INVALID;
};
static void set_logical_processors(const char *value, EbConfig *cfg) {
    cfg->config.logical_processors = (uint32_t)strtoul(value, NULL, 0);
};
static void set_pinned_execution(const char *value, EbConfig *cfg) {
    cfg->config.pin_threads = (uint32_t)strtoul(value, NULL, 0);
};
static void set_target_socket(const char *value, EbConfig *cfg) {
    cfg->config.target_socket = (int32_t)strtol(value, NULL, 0);
};
static void set_restricted_motion_vector(const char *value, EbConfig *cfg) {
    cfg->config.restricted_motion_vector = !!strtol(value, NULL, 0);
};
static void set_cfg_color_primaries(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "color-primaries", value);
}
static void set_cfg_transfer_characteristics(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "transfer-characteristics", value);
}
static void set_cfg_matrix_coefficients(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "matrix-coefficients", value);
}
static void set_cfg_color_range(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "color-range", value);
}
static void set_cfg_chroma_sample_position(const char *value, EbConfig *cfg) {
    svt_av1_enc_parse_parameter(&cfg->config, "chroma-sample-position", value);
}
static void set_cfg_mastering_display(const char *value, EbConfig *cfg) {
    if (!svt_aom_parse_mastering_display(&cfg->config.mastering_display, value))
        fprintf(stderr, "Failed to parse mastering-display info properly\n");
}
static void set_cfg_content_light(const char *value, EbConfig *cfg) {
    if (!svt_aom_parse_content_light_level(&cfg->config.content_light_level, value))
        fprintf(stderr, "Failed to parse content light level info properly\n");
}
static void set_cfg_sframe_dist(const char *value, EbConfig *cfg) {
    cfg->config.sframe_dist = (int32_t)strtol(value, NULL, 0);
}
static void set_cfg_sframe_mode(const char *value, EbConfig *cfg) {
    cfg->config.sframe_mode = (EbSFrameMode)strtoul(value, NULL, 0);
}

enum CfgType {
    SINGLE_INPUT, // Configuration parameters that have only 1 value input
    ARRAY_INPUT // Configuration parameters that have multiple values as input
};

/**********************************
 * Config Entry Struct
 **********************************/
typedef struct config_entry_s {
    enum CfgType type;
    const char  *token;
    const char  *name;
    void (*scf)(const char *, EbConfig *);
} ConfigEntry;

/**********************************
 * Config Entry Array
 **********************************/
ConfigEntry config_entry_options[] = {
    // File I/O
    {SINGLE_INPUT,
     HELP_TOKEN,
     "Shows the command line options currently available",
     set_cfg_input_file},
    {SINGLE_INPUT,
     VERSION_TOKEN,
     "Shows the version of the library that's linked to the library",
     set_cfg_input_file},
    {SINGLE_INPUT,
     INPUT_FILE_TOKEN,
     "Input raw video (y4m and yuv) file path, use `stdin` or `-` to read from pipe",
     set_cfg_input_file},
    {SINGLE_INPUT,
     INPUT_FILE_LONG_TOKEN,
     "Input raw video (y4m and yuv) file path, use `stdin` or `-` to read from pipe",
     set_cfg_input_file},

    {SINGLE_INPUT,
     OUTPUT_BITSTREAM_TOKEN,
     "Output compressed (ivf) file path, use `stdout` or `-` to write to pipe",
     set_cfg_stream_file},
    {SINGLE_INPUT,
     OUTPUT_BITSTREAM_LONG_TOKEN,
     "Output compressed (ivf) file path, use `stdout` or `-` to write to pipe",
     set_cfg_stream_file},

    {SINGLE_INPUT, CONFIG_FILE_TOKEN, "Configuration file path", set_cfg_input_file},
    {SINGLE_INPUT, CONFIG_FILE_LONG_TOKEN, "Configuration file path", set_cfg_input_file},

    {SINGLE_INPUT, ERROR_FILE_TOKEN, "Error file path, defaults to stderr", set_cfg_error_file},
    {SINGLE_INPUT, OUTPUT_RECON_TOKEN, "Reconstructed yuv file path", set_cfg_recon_file},
    {SINGLE_INPUT, OUTPUT_RECON_LONG_TOKEN, "Reconstructed yuv file path", set_cfg_recon_file},

    {SINGLE_INPUT,
     STAT_FILE_TOKEN,
     "PSNR / SSIM per picture stat output file path, requires `--enable-stat-report 1`",
     set_cfg_stat_file},

#if !REMOVE_MANUAL_PRED
    {SINGLE_INPUT,
     INPUT_PREDSTRUCT_FILE_TOKEN,
     "Manual prediction structure file path",
     set_pred_struct_file},
#endif
    {SINGLE_INPUT,
     PROGRESS_TOKEN,
     "Verbosity of the output, default is 1 [0: no progress is printed, 2: aomenc style output]",
     set_progress},
    {SINGLE_INPUT,
     NO_PROGRESS_TOKEN,
     "Do not print out progress, default is 0 [1: `" PROGRESS_TOKEN " 0`, 0: `" PROGRESS_TOKEN
     " 1`]",
     set_no_progress},

    {SINGLE_INPUT,
     PRESET_TOKEN,
     "Encoder preset, presets < 0 are for debugging. Higher presets means faster encodes, but with "
     "a quality tradeoff, default is 10 [-2-13]",
     set_enc_mode},

    {SINGLE_INPUT,
     SVTAV1_PARAMS,
     "colon separated list of key=value pairs of parameters with keys based on config file options",
     parse_svtav1_params},

    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_global_options[] = {
    // Picture Dimensions
    {SINGLE_INPUT,
     WIDTH_TOKEN,
     "Frame width in pixels, inferred if y4m, default is 0 [64-16384]",
     set_cfg_source_width},
    {SINGLE_INPUT,
     WIDTH_LONG_TOKEN,
     "Frame width in pixels, inferred if y4m, default is 0 [64-16384]",
     set_cfg_source_width},

    {SINGLE_INPUT,
     HEIGHT_TOKEN,
     "Frame height in pixels, inferred if y4m, default is 0 [64-8704]",
     set_cfg_source_height},
    {SINGLE_INPUT,
     HEIGHT_LONG_TOKEN,
     "Frame height in pixels, inferred if y4m, default is 0 [64-8704]",
     set_cfg_source_height},

    {SINGLE_INPUT,
     FORCED_MAX_FRAME_WIDTH_TOKEN,
     "Maximum frame width value to force, default is 0 [64-16384]",
     set_cfg_forced_max_frame_width},

    {SINGLE_INPUT,
     FORCED_MAX_FRAME_HEIGHT_TOKEN,
     "Maximum frame height value to force, default is 0 [64-8704]",
     set_cfg_forced_max_frame_height},

    {SINGLE_INPUT,
     NUMBER_OF_PICTURES_TOKEN,
     "Number of frames to encode. If `n` is larger than the input, the encoder will loop back and "
     "continue encoding, default is 0 [0: until EOF, 1-`(2^63)-1`]",
     set_cfg_frames_to_be_encoded},
    {SINGLE_INPUT,
     NUMBER_OF_PICTURES_LONG_TOKEN,
     "Number of frames to encode. If `n` is larger than the input, the encoder will loop back and "
     "continue encoding, default is 0 [0: until EOF, 1-`(2^63)-1`]",
     set_cfg_frames_to_be_encoded},

    {SINGLE_INPUT,
     BUFFERED_INPUT_TOKEN,
     "Buffer `n` input frames into memory and use them to encode, default is -1 [-1: no frames "
     "buffered, 1-`(2^31)-1`]",
     set_buffered_input},
    {SINGLE_INPUT,
     ENCODER_COLOR_FORMAT,
     "Color format, only yuv420 is supported at this time, default is 1 [0: yuv400, 1: yuv420, 2: "
     "yuv422, 3: yuv444]",
     set_encoder_color_format},
    {SINGLE_INPUT,
     PROFILE_TOKEN,
     "Bitstream profile, default is 0 [0: main, 1: high, 2: professional]",
     set_profile},
    {SINGLE_INPUT,
     LEVEL_TOKEN,
     "Bitstream level, defined in A.3 of the av1 spec, default is 0 [0: autodetect from input, "
     "2.0-7.3]",
     set_level},
    {SINGLE_INPUT,
     HDR_INPUT_NEW_TOKEN,
     "Enable writing of HDR metadata in the bitstream, default is 0 [0-1]",
     set_high_dynamic_range_input},
    {SINGLE_INPUT,
     FRAME_RATE_TOKEN,
     "Input video frame rate, integer values only, inferred if y4m, default is 60 [1-240]",
     set_frame_rate},
    {SINGLE_INPUT,
     FRAME_RATE_NUMERATOR_TOKEN,
     "Input video frame rate numerator, default is 60000 [0-2^32-1]",
     set_frame_rate_numerator},
    {SINGLE_INPUT,
     FRAME_RATE_DENOMINATOR_TOKEN,
     "Input video frame rate denominator, default is 1000 [0-2^32-1]",
     set_frame_rate_denominator},
    {SINGLE_INPUT,
     INPUT_DEPTH_TOKEN,
     "Input video file and output bitstream bit-depth, default is 8 [8, 10]",
     set_encoder_bit_depth},
    {SINGLE_INPUT,
     INPUT_COMPRESSED_TEN_BIT_FORMAT,
     "Depreciated and ignored, default is 0 [0-1]",
     set_compressed_ten_bit_format},
    // Latency
    {SINGLE_INPUT,
     INJECTOR_TOKEN,
     "Inject pictures to the library at defined frame rate, default is 0 [0-1]",
     set_injector},
    {SINGLE_INPUT,
     INJECTOR_FRAMERATE_TOKEN,
     "Set injector frame rate, only applicable with `--inj 1`, default is 60 [0-240]",
     set_injector_frame_rate},
    {SINGLE_INPUT,
     STAT_REPORT_NEW_TOKEN,
     "Calculates and outputs PSNR SSIM metrics at the end of encoding, default is 0 [0-1]",
     set_stat_report},

    // Asm Type
    {SINGLE_INPUT,
     ASM_TYPE_TOKEN,
     "Limit assembly instruction set, only applicable to x86, default is max [c, mmx, sse, sse2, "
     "sse3, ssse3, sse4_1, sse4_2, avx, avx2, avx512, max]",
     set_asm_type},
    {SINGLE_INPUT,
     THREAD_MGMNT,
     "Target (best effort) number of logical cores to be used. 0 means all. Refer to Appendix A.1 "
     "of the user "
     "guide, default is 0 [0, core count of the machine]",
     set_logical_processors},
    {SINGLE_INPUT,
     PIN_TOKEN,
     "Pin the execution to the first --lp cores. Overwritten to 0 when `--ss` is set. Refer to "
     "Appendix "
     "A.1 of the user guide, default is 1 [0-1]",
     set_pinned_execution},
    {SINGLE_INPUT,
     TARGET_SOCKET,
     "Specifies which socket to run on, assumes a max of two sockets. Refer to Appendix A.1 of the "
     "user guide, default is -1 [-1, 0, -1]",
     set_target_socket},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_rc[] = {
    // Rate Control
    {SINGLE_INPUT,
     RATE_CONTROL_ENABLE_TOKEN,
     "Rate control mode, default is 0 [0: CRF or CQP (if `--aq-mode` is 0), 1: VBR, 2: CBR]",
     set_rate_control_mode},
    {SINGLE_INPUT, QP_TOKEN, "Initial QP level value, default is 35 [1-63]", set_cfg_qp},
    {SINGLE_INPUT, QP_LONG_TOKEN, "Initial QP level value, default is 35 [1-63]", set_cfg_qp},
    {SINGLE_INPUT,
     CRF_LONG_TOKEN,
     "Constant Rate Factor value, setting this value is equal to `--rc 0 --aq-mode 2 --qp "
     "x`, default is 35 [1-63]",
     set_cfg_crf},

    {SINGLE_INPUT,
     TARGET_BIT_RATE_TOKEN,
     "Target Bitrate (kbps), only applicable for VBR and CBR encoding, default is 7000 [1-100000]",
     set_target_bit_rate},
    {SINGLE_INPUT,
     MAX_BIT_RATE_TOKEN,
     "Maximum Bitrate (kbps) only applicable for CRF encoding, default is 0 [1-100000]",
     set_max_bit_rate},
    {SINGLE_INPUT,
     USE_QP_FILE_TOKEN,
     "Overwrite the encoder default picture based QP assignments and use QP values from "
     "`--qp-file`, default is 0 [0-1]",
     set_cfg_use_qp_file},
    {SINGLE_INPUT,
     QP_FILE_NEW_TOKEN,
     "Path to a file containing per picture QP value separated by newlines",
     set_cfg_qp_file},
    {SINGLE_INPUT,
     MAX_QP_TOKEN,
     "Maximum (highest) quantizer, only applicable for VBR and CBR, default is 63 [1-63]",
     set_max_qp_allowed},
    {SINGLE_INPUT,
     MIN_QP_TOKEN,
     "Minimum (lowest) quantizer, only applicable for VBR and CBR, default is 1 [1-63]",
     set_min_qp_allowed},

    {SINGLE_INPUT,
     ADAPTIVE_QP_ENABLE_NEW_TOKEN,
     "Set adaptive QP level, default is 2 [0: off, 1: variance base using AV1 segments, 2: deltaq "
     "pred efficiency]",
     set_adaptive_quantization},
    {SINGLE_INPUT,
     USE_FIXED_QINDEX_OFFSETS_TOKEN,
     "Overwrite the encoder default hierarchical layer based QP assignment and use fixed Q index "
     "offsets, default is 0 [0-2]",
     set_cfg_use_fixed_qindex_offsets},
    {SINGLE_INPUT,
     KEY_FRAME_QINDEX_OFFSET_TOKEN,
     "Overwrite the encoder default keyframe Q index assignment, default is 0 [-256-255]",
     set_cfg_key_frame_qindex_offset},
    {SINGLE_INPUT,
     KEY_FRAME_CHROMA_QINDEX_OFFSET_TOKEN,
     "Overwrite the encoder default chroma keyframe Q index assignment, default is 0 [-256-255]",
     set_cfg_key_frame_chroma_qindex_offset},
    {SINGLE_INPUT,
     QINDEX_OFFSETS_TOKEN,
     "list of luma Q index offsets per hierarchical layer, separated by `,` with each offset in "
     "the range of [-256-255], default is `0,0,..,0`",
     set_cfg_qindex_offsets},
    {SINGLE_INPUT,
     CHROMA_QINDEX_OFFSETS_TOKEN,
     "list of chroma Q index offsets per hierarchical layer, separated by `,` with each offset in "
     "the range of [-256-255], default is `0,0,..,0`",
     set_cfg_chroma_qindex_offsets},
    {SINGLE_INPUT,
     LUMA_Y_DC_QINDEX_OFFSET_TOKEN,
     "Luma Y DC Qindex Offset",
     set_cfg_luma_y_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_U_DC_QINDEX_OFFSET_TOKEN,
     "Chroma U DC Qindex Offset",
     set_cfg_chroma_u_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_U_AC_QINDEX_OFFSET_TOKEN,
     "Chroma U AC Qindex Offset",
     set_cfg_chroma_u_ac_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_V_DC_QINDEX_OFFSET_TOKEN,
     "Chroma V DC Qindex Offset",
     set_cfg_chroma_v_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_V_AC_QINDEX_OFFSET_TOKEN,
     "Chroma V AC Qindex Offset",
     set_cfg_chroma_v_ac_qindex_offset},
    {SINGLE_INPUT,
     UNDER_SHOOT_PCT_TOKEN,
     "Only for VBR and CBR, allowable datarate undershoot (min) target (percentage), default is "
     "25, but can change based on rate control [0-100]",
     set_under_shoot_pct},
    {SINGLE_INPUT,
     OVER_SHOOT_PCT_TOKEN,
     "Only for VBR and CBR, allowable datarate overshoot (max) target (percentage), default is 25, "
     "but can change based on rate control [0-100]",
     set_over_shoot_pct},
    {SINGLE_INPUT,
     MBR_OVER_SHOOT_PCT_TOKEN,
     "Only for Capped CRF, allowable datarate overshoot (max) target (percentage), default is 50, "
     "but can change based on rate control [0-100]",
     set_mbr_over_shoot_pct},
    {SINGLE_INPUT,
     GOP_CONSTRAINT_RC_TOKEN,
     "Enable GoP constraint rc.  When enabled, the rate control matches the target rate for each "
     "GoP, default is 0 [0-1]",
     set_gop_constraint_rc},
    {SINGLE_INPUT,
     BUFFER_SIZE_TOKEN,
     "Client buffer size (ms), only applicable for CBR, default is 6000 [0-10000]",
     set_buf_sz},
    {SINGLE_INPUT,
     BUFFER_INITIAL_SIZE_TOKEN,
     "Client initial buffer size (ms), only applicable for CBR, default is 4000 [0-10000]",
     set_buf_initial_sz},
    {SINGLE_INPUT,
     BUFFER_OPTIMAL_SIZE_TOKEN,
     "Client optimal buffer size (ms), only applicable for CBR, default is 5000 [0-10000]",
     set_buf_optimal_sz},
    {SINGLE_INPUT,
     RECODE_LOOP_TOKEN,
     "Recode loop level, refer to \"Recode loop level table\" in the user guide for more info [0: "
     "off, 4: preset based]",
     set_recode_loop},
    {SINGLE_INPUT,
     VBR_BIAS_PCT_TOKEN,
     "CBR/VBR bias, default is 50 [0: CBR-like, 1-99, 100: VBR-like]",
     set_vbr_bias_pct},
    {SINGLE_INPUT,
     VBR_MIN_SECTION_PCT_TOKEN,
     "GOP min bitrate (expressed as a percentage of the target rate), default is 0 [0-100]",
     set_vbr_min_section_pct},
    {SINGLE_INPUT,
     VBR_MAX_SECTION_PCT_TOKEN,
     "GOP max bitrate (expressed as a percentage of the target rate), default is 2000 [0-10000]",
     set_vbr_max_section_pct},
    {SINGLE_INPUT,
     ENABLE_QM_TOKEN,
     "Enable quantisation matrices, default is 0 [0-1]",
     set_enable_qm},
    {SINGLE_INPUT,
     MIN_QM_LEVEL_TOKEN,
     "Min quant matrix flatness, default is 8 [0-15]",
     set_min_qm_level},
    {SINGLE_INPUT,
     MAX_QM_LEVEL_TOKEN,
     "Max quant matrix flatness, default is 15 [0-15]",
     set_max_qm_level},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_2p[] = {
    // 2 pass
    {SINGLE_INPUT,
     PASS_TOKEN,
     "Multi-pass selection, pass 3 is only available for VBR, default is 0 [0: single pass encode, "
     "1: first pass, 2: second pass, 3: third pass]",
     set_pass},
    {SINGLE_INPUT,
     TWO_PASS_STATS_TOKEN,
     "Filename for multi-pass encoding, default is \"svtav1_2pass.log\"",
     set_two_pass_stats},
    {SINGLE_INPUT,
     PASSES_TOKEN,
     "Number of encoding passes, default is preset dependent but generally 1 [1: one pass encode, "
     "2: multi-pass encode]",
     set_passes},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_intra_refresh[] = {
    {SINGLE_INPUT,
     KEYINT_TOKEN,
     "GOP size (frames), default is -2 [-2: ~5 seconds, -1: \"infinite\" and only applicable for "
     "CRF, 0: same as -1]",
     set_keyint},
    {SINGLE_INPUT,
     INTRA_REFRESH_TYPE_TOKEN,
     "Intra refresh type, default is 2 [1: FWD Frame (Open GOP), 2: KEY Frame (Closed GOP)]",
     set_cfg_intra_refresh_type},
    {SINGLE_INPUT,
     SCENE_CHANGE_DETECTION_TOKEN,
     "Scene change detection control, default is 0 [0-1]",
     set_scene_change_detection},
    {SINGLE_INPUT,
     LOOKAHEAD_NEW_TOKEN,
     "Number of frames in the future to look ahead, not including minigop, temporal filtering, and "
     "rate "
     "control, default is -1 [-1: auto, 0-120]",
     set_look_ahead_distance},
    {SINGLE_INPUT,
     HIERARCHICAL_LEVELS_TOKEN,
     "Set hierarchical levels beyond the base layer, default is <=M12: 5, else: 4 [2: 3 temporal "
     "layers, 3: 4 "
     "temporal layers, 4: 5 "
     "layers, 5: 6 layers]",
     set_hierarchical_levels},
    {SINGLE_INPUT,
     PRED_STRUCT_TOKEN,
     "Set prediction structure, default is 2 [1: low delay frames, 2: "
     "random access]",
     set_cfg_pred_structure},
    {SINGLE_INPUT,
     FORCE_KEY_FRAMES_TOKEN,
     "Force key frames at the comma separated specifiers. `#f` for frames, `#.#s` for seconds",
     set_cfg_force_key_frames},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_specific[] = {
    {SINGLE_INPUT,
     TILE_ROW_TOKEN,
     "Number of tile rows to use, `TileRow == log2(x)`, default changes per resolution but is 1 "
     "[0-6]",
     set_tile_row},
    {SINGLE_INPUT,
     TILE_COL_TOKEN,
     "Number of tile columns to use, `TileCol == log2(x)`, default changes per resolution but is 1 "
     "[0-4]",
     set_tile_col},

    // DLF
    {SINGLE_INPUT,
     LOOP_FILTER_ENABLE,
     "Deblocking loop filter control, default is 1 [0-1]",
     set_enable_dlf_flag},
    // CDEF
    {SINGLE_INPUT,
     CDEF_ENABLE_TOKEN,
     "Enable Constrained Directional Enhancement Filter, default is 1 [0-1]",
     set_cdef_enable},
    // RESTORATION
    {SINGLE_INPUT,
     ENABLE_RESTORATION_TOKEN,
     "Enable loop restoration filter, default is 1 [0-1]",
     set_enable_restoration_flag},
    {SINGLE_INPUT,
     ENABLE_TPL_LA_TOKEN,
     "Temporal Dependency model control, currently forced on library side, only applicable for "
     "CRF/CQP, default is 1 [0-1]",
     set_enable_tpl_la},
    {SINGLE_INPUT,
     MFMV_ENABLE_NEW_TOKEN,
     "Motion Field Motion Vector control, default is -1 [-1: auto, 0-1]",
     set_enable_mfmv_flag},
    {SINGLE_INPUT,
     FAST_DECODE_TOKEN,
     "Fast Decoder levels, default is 0 [0-1]",
     set_fast_decode_flag},
    // --- start: ALTREF_FILTERING_SUPPORT
    {SINGLE_INPUT,
     ENABLE_TF_TOKEN,
     "Enable ALT-REF (temporally filtered) frames, default is 1 [0-1]",
     set_enable_tf},

    {SINGLE_INPUT,
     ENABLE_OVERLAYS,
     "Enable the insertion of overlayer pictures which will be used as an additional reference "
     "frame for the base layer picture, default is 0 [0-1]",
     set_enable_overlays},
    // --- end: ALTREF_FILTERING_SUPPORT
    {SINGLE_INPUT,
     TUNE_TOKEN,
     "Specifies whether to use PSNR or VQ as the tuning metric [0 = VQ, 1 = PSNR], default is 1 "
     "[0-1]",
     set_tune},
    // MD Parameters
    {SINGLE_INPUT,
     SCREEN_CONTENT_TOKEN,
     "Set screen content detection level, default is 2 [0: off, 1: on, 2: content adaptive]",
     set_screen_content_mode},
    // Optional Features
    {SINGLE_INPUT,
     RESTRICTED_MOTION_VECTOR,
     "Restrict motion vectors from reaching outside the picture boundary, default is 0 [0-1]",
     set_restricted_motion_vector},

    // Annex A parameters
    {SINGLE_INPUT,
     FILM_GRAIN_TOKEN,
     "Enable film grain, default is 0 [0: off, 1-50: level of denoising for film grain]",
     set_cfg_film_grain},

    {SINGLE_INPUT,
     FILM_GRAIN_DENOISE_APPLY_TOKEN,
     "Apply denoising when film grain is ON, default is 1 [0: no denoising, film grain data is "
     "still in frame header, "
     "1: level of denoising is set by the film-grain parameter]",
     set_cfg_film_grain_denoise_apply},

    // --- start: SUPER-RESOLUTION SUPPORT
    {SINGLE_INPUT,
     SUPERRES_MODE_INPUT,
     "Enable super-resolution mode, refer to the super-resolution section in the user guide, "
     "default is 0 [0: off, 1-3, 4: auto-select mode]",
     set_superres_mode},
    {SINGLE_INPUT,
     SUPERRES_DENOM,
     "Super-resolution denominator, only applicable for mode == 1, default is 8 [8: no scaling, "
     "9-15, 16: half-scaling]",
     set_superres_denom},
    {SINGLE_INPUT,
     SUPERRES_KF_DENOM,
     "Super-resolution denominator for key frames, only applicable for mode == 1, default is 8 [8: "
     "no scaling, 9-15, 16: half-scaling]",
     set_superres_kf_denom},
    {SINGLE_INPUT,
     SUPERRES_QTHRES,
     "Super-resolution q-threshold, only applicable for mode == 3, default is 43 [0-63]",
     set_superres_qthres},
    {SINGLE_INPUT,
     SUPERRES_KF_QTHRES,
     "Super-resolution q-threshold for key frames, only applicable for mode == 3, default is 43 "
     "[0-63]",
     set_superres_kf_qthres},
    // --- end: SUPER-RESOLUTION SUPPORT

    // --- start: SWITCH_FRAME SUPPORT
    {SINGLE_INPUT,
     SFRAME_DIST_TOKEN,
     "S-Frame interval (frames) (0: OFF[default], > 0: ON)",
     set_cfg_sframe_dist},
    {SINGLE_INPUT,
     SFRAME_MODE_TOKEN,
     "S-Frame insertion mode ([1-2], 1: the considered frame will be made into an S-Frame only if "
     "it is an altref frame,"
     " 2: the next altref frame will be made into an S-Frame[default])",
     set_cfg_sframe_mode},
    // --- end: SWITCH_FRAME SUPPORT
    // --- start: REFERENCE SCALING SUPPORT
    {SINGLE_INPUT,
     RESIZE_MODE_INPUT,
     "Enable resize mode [0: none, 1: fixed scale, 2: random scale, 3: dynamic scale, 4: random "
     "access]",
     set_resize_mode},
    {SINGLE_INPUT,
     RESIZE_DENOM,
     "Resize denominator, only applicable for mode == 1 [8-16]",
     set_resize_denom},
    {SINGLE_INPUT,
     RESIZE_KF_DENOM,
     "Resize denominator for key frames, only applicable for mode == 1 [8-16]",
     set_resize_kf_denom},
    // --- end: REFERENCE SCALING SUPPORT

    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry_color_description[] = {
    // Color description
    {SINGLE_INPUT,
     COLOR_PRIMARIES_NEW_TOKEN,
     "Color primaries, refer to Appendix A.2 of the user guide, default is 2 [0-12, 22]",
     set_cfg_color_primaries},
    {SINGLE_INPUT,
     TRANSFER_CHARACTERISTICS_NEW_TOKEN,
     "Transfer characteristics, refer to Appendix A.2 of the user guide, default is 2 [0-22]",
     set_cfg_transfer_characteristics},
    {SINGLE_INPUT,
     MATRIX_COEFFICIENTS_NEW_TOKEN,
     "Matrix coefficients, refer to Appendix A.2 of the user guide, default is 2 [0-14]",
     set_cfg_matrix_coefficients},
    {SINGLE_INPUT,
     COLOR_RANGE_NEW_TOKEN,
     "Color range, default is 0 [0: Studio, 1: Full]",
     set_cfg_color_range},
    {SINGLE_INPUT,
     CHROMA_SAMPLE_POSITION_TOKEN,
     "Chroma sample position, default is 'unknown' ['unknown', 'vertical'/'left', "
     "'colocated'/'topleft']",
     set_cfg_chroma_sample_position},

    {SINGLE_INPUT,
     MASTERING_DISPLAY_TOKEN,
     "Mastering display metadata in the format of \"G(x,y)B(x,y)R(x,y)WP(x,y)L(max,min)\", refer "
     "to the user guide Appendix A.2",
     set_cfg_mastering_display},

    {SINGLE_INPUT,
     CONTENT_LIGHT_LEVEL_TOKEN,
     "Set content light level in the format of \"max_cll,max_fall\", refer to the user guide "
     "Appendix A.2",
     set_cfg_content_light},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

ConfigEntry config_entry[] = {
    // Options
    {SINGLE_INPUT, INPUT_FILE_TOKEN, "InputFile", set_cfg_input_file},
    {SINGLE_INPUT, INPUT_FILE_LONG_TOKEN, "InputFile", set_cfg_input_file},
    {SINGLE_INPUT, OUTPUT_BITSTREAM_TOKEN, "StreamFile", set_cfg_stream_file},
    {SINGLE_INPUT, OUTPUT_BITSTREAM_LONG_TOKEN, "StreamFile", set_cfg_stream_file},
    {SINGLE_INPUT, ERROR_FILE_TOKEN, "ErrorFile", set_cfg_error_file},
    {SINGLE_INPUT, OUTPUT_RECON_TOKEN, "ReconFile", set_cfg_recon_file},
    {SINGLE_INPUT, OUTPUT_RECON_LONG_TOKEN, "ReconFile", set_cfg_recon_file},
    {SINGLE_INPUT, STAT_FILE_TOKEN, "StatFile", set_cfg_stat_file},
#if !REMOVE_MANUAL_PRED
    {SINGLE_INPUT, INPUT_PREDSTRUCT_FILE_TOKEN, "PredStructFile", set_pred_struct_file},
#endif
    {SINGLE_INPUT, PROGRESS_TOKEN, "Progress", set_progress},
    {SINGLE_INPUT, NO_PROGRESS_TOKEN, "NoProgress", set_no_progress},
    {SINGLE_INPUT, PRESET_TOKEN, "EncoderMode", set_enc_mode},
    {SINGLE_INPUT, SVTAV1_PARAMS, "SvtAv1Params", parse_svtav1_params},

    // Encoder Global Options
    //   Picture Dimensions
    {SINGLE_INPUT, WIDTH_TOKEN, "SourceWidth", set_cfg_source_width},
    {SINGLE_INPUT, WIDTH_LONG_TOKEN, "SourceWidth", set_cfg_source_width},
    {SINGLE_INPUT, HEIGHT_TOKEN, "SourceHeight", set_cfg_source_height},
    {SINGLE_INPUT, HEIGHT_LONG_TOKEN, "SourceHeight", set_cfg_source_height},
    {SINGLE_INPUT,
     FORCED_MAX_FRAME_WIDTH_TOKEN,
     "ForcedMaximumFrameWidth",
     set_cfg_forced_max_frame_width},
    {SINGLE_INPUT,
     FORCED_MAX_FRAME_HEIGHT_TOKEN,
     "ForcedMaximumFrameHeight",
     set_cfg_forced_max_frame_height},
    // Prediction Structure
    {SINGLE_INPUT, NUMBER_OF_PICTURES_TOKEN, "FrameToBeEncoded", set_cfg_frames_to_be_encoded},
    {SINGLE_INPUT, NUMBER_OF_PICTURES_LONG_TOKEN, "FrameToBeEncoded", set_cfg_frames_to_be_encoded},
    {SINGLE_INPUT, BUFFERED_INPUT_TOKEN, "BufferedInput", set_buffered_input},

    //   Annex A parameters
    {SINGLE_INPUT, TIER_TOKEN, "Tier", set_tier}, // Lacks a command line flag for now
    {SINGLE_INPUT, ENCODER_COLOR_FORMAT, "EncoderColorFormat", set_encoder_color_format},
    {SINGLE_INPUT, PROFILE_TOKEN, "Profile", set_profile},
    {SINGLE_INPUT, LEVEL_TOKEN, "Level", set_level},
    {SINGLE_INPUT, HDR_INPUT_NEW_TOKEN, "HighDynamicRangeInput", set_high_dynamic_range_input},

    //   Frame Rate tokens
    {SINGLE_INPUT, FRAME_RATE_TOKEN, "FrameRate", set_frame_rate},
    {SINGLE_INPUT, FRAME_RATE_NUMERATOR_TOKEN, "FrameRateNumerator", set_frame_rate_numerator},
    {SINGLE_INPUT,
     FRAME_RATE_DENOMINATOR_TOKEN,
     "FrameRateDenominator",
     set_frame_rate_denominator},

    //   Bit depth tokens
    {SINGLE_INPUT, INPUT_DEPTH_TOKEN, "EncoderBitDepth", set_encoder_bit_depth},

    {SINGLE_INPUT,
     INPUT_COMPRESSED_TEN_BIT_FORMAT,
     "CompressedTenBitFormat",
     set_compressed_ten_bit_format},

    //   Latency
    {SINGLE_INPUT, INJECTOR_TOKEN, "Injector", set_injector},
    {SINGLE_INPUT, INJECTOR_FRAMERATE_TOKEN, "InjectorFrameRate", set_injector_frame_rate},

    {SINGLE_INPUT, STAT_REPORT_NEW_TOKEN, "StatReport", set_stat_report},

    //   Asm Type
    {SINGLE_INPUT, ASM_TYPE_TOKEN, "Asm", set_asm_type},

    //   Thread Management
    {SINGLE_INPUT, THREAD_MGMNT, "LogicalProcessors", set_logical_processors},
    {SINGLE_INPUT, PIN_TOKEN, "PinnedExecution", set_pinned_execution},
    {SINGLE_INPUT, TARGET_SOCKET, "TargetSocket", set_target_socket},

    // Rate Control Options
    {SINGLE_INPUT, RATE_CONTROL_ENABLE_TOKEN, "RateControlMode", set_rate_control_mode},
    {SINGLE_INPUT, QP_TOKEN, "QP", set_cfg_qp},
    {SINGLE_INPUT, QP_LONG_TOKEN, "QP", set_cfg_qp},
    {SINGLE_INPUT, CRF_LONG_TOKEN, "CRF", set_cfg_crf},
    {SINGLE_INPUT, TARGET_BIT_RATE_TOKEN, "TargetBitRate", set_target_bit_rate},
    {SINGLE_INPUT, MAX_BIT_RATE_TOKEN, "MaxBitRate", set_max_bit_rate},

    {SINGLE_INPUT, USE_QP_FILE_TOKEN, "UseQpFile", set_cfg_use_qp_file},
    {SINGLE_INPUT, QP_FILE_NEW_TOKEN, "QpFile", set_cfg_qp_file},

    {SINGLE_INPUT, MAX_QP_TOKEN, "MaxQpAllowed", set_max_qp_allowed},
    {SINGLE_INPUT, MIN_QP_TOKEN, "MinQpAllowed", set_min_qp_allowed},

    {SINGLE_INPUT, ADAPTIVE_QP_ENABLE_NEW_TOKEN, "AdaptiveQuantization", set_adaptive_quantization},

    //   qindex offsets
    {SINGLE_INPUT,
     USE_FIXED_QINDEX_OFFSETS_TOKEN,
     "UseFixedQIndexOffsets",
     set_cfg_use_fixed_qindex_offsets},
    {SINGLE_INPUT,
     KEY_FRAME_QINDEX_OFFSET_TOKEN,
     "KeyFrameQIndexOffset",
     set_cfg_key_frame_qindex_offset},
    {SINGLE_INPUT,
     KEY_FRAME_CHROMA_QINDEX_OFFSET_TOKEN,
     "KeyFrameChromaQIndexOffset",
     set_cfg_key_frame_chroma_qindex_offset},
    {SINGLE_INPUT, QINDEX_OFFSETS_TOKEN, "QIndexOffsets", set_cfg_qindex_offsets},
    {SINGLE_INPUT,
     CHROMA_QINDEX_OFFSETS_TOKEN,
     "ChromaQIndexOffsets",
     set_cfg_chroma_qindex_offsets},
    {SINGLE_INPUT,
     LUMA_Y_DC_QINDEX_OFFSET_TOKEN,
     "LumaYDCQindexOffset",
     set_cfg_luma_y_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_U_DC_QINDEX_OFFSET_TOKEN,
     "ChromaUDCQindexOffset",
     set_cfg_chroma_u_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_U_AC_QINDEX_OFFSET_TOKEN,
     "ChromaUACQindexOffset",
     set_cfg_chroma_u_ac_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_V_DC_QINDEX_OFFSET_TOKEN,
     "ChromaVDCQindexOffset",
     set_cfg_chroma_v_dc_qindex_offset},
    {SINGLE_INPUT,
     CHROMA_V_AC_QINDEX_OFFSET_TOKEN,
     "ChromaVACQindexOffset",
     set_cfg_chroma_v_ac_qindex_offset},
    {SINGLE_INPUT, UNDER_SHOOT_PCT_TOKEN, "UnderShootPct", set_under_shoot_pct},
    {SINGLE_INPUT, OVER_SHOOT_PCT_TOKEN, "OverShootPct", set_over_shoot_pct},
    {SINGLE_INPUT, MBR_OVER_SHOOT_PCT_TOKEN, "MbrOverShootPct", set_mbr_over_shoot_pct},
    {SINGLE_INPUT, GOP_CONSTRAINT_RC_TOKEN, "GopConstraintRc", set_gop_constraint_rc},
    {SINGLE_INPUT, BUFFER_SIZE_TOKEN, "BufSz", set_buf_sz},
    {SINGLE_INPUT, BUFFER_INITIAL_SIZE_TOKEN, "BufInitialSz", set_buf_initial_sz},
    {SINGLE_INPUT, BUFFER_OPTIMAL_SIZE_TOKEN, "BufOptimalSz", set_buf_optimal_sz},
    {SINGLE_INPUT, RECODE_LOOP_TOKEN, "RecodeLoop", set_recode_loop},
    {SINGLE_INPUT, VBR_BIAS_PCT_TOKEN, "VBRBiasPct", set_vbr_bias_pct},
    {SINGLE_INPUT, VBR_MIN_SECTION_PCT_TOKEN, "MinSectionPct", set_vbr_min_section_pct},
    {SINGLE_INPUT, VBR_MAX_SECTION_PCT_TOKEN, "MaxSectionPct", set_vbr_max_section_pct},

    // Multi-pass Options
    {SINGLE_INPUT, PASS_TOKEN, "Pass", set_pass},
    {SINGLE_INPUT, TWO_PASS_STATS_TOKEN, "Stats", set_two_pass_stats},
    {SINGLE_INPUT, PASSES_TOKEN, "Passes", set_passes},

    // GOP size and type Options
    {SINGLE_INPUT, INTRA_PERIOD_TOKEN, "IntraPeriod", set_cfg_intra_period},
    {SINGLE_INPUT, KEYINT_TOKEN, "Keyint", set_keyint},
    {SINGLE_INPUT, INTRA_REFRESH_TYPE_TOKEN, "IntraRefreshType", set_cfg_intra_refresh_type},
    {SINGLE_INPUT,
     SCENE_CHANGE_DETECTION_TOKEN,
     "SceneChangeDetection",
     set_scene_change_detection},
    {SINGLE_INPUT, LOOKAHEAD_NEW_TOKEN, "Lookahead", set_look_ahead_distance},
    //   Prediction Structure
    {SINGLE_INPUT, HIERARCHICAL_LEVELS_TOKEN, "HierarchicalLevels", set_hierarchical_levels},
    {SINGLE_INPUT, PRED_STRUCT_TOKEN, "PredStructure", set_cfg_pred_structure},
    {SINGLE_INPUT, FORCE_KEY_FRAMES_TOKEN, "ForceKeyFrames", set_cfg_force_key_frames},

    // AV1 Specific Options
    {SINGLE_INPUT, TILE_ROW_TOKEN, "TileRow", set_tile_row},
    {SINGLE_INPUT, TILE_COL_TOKEN, "TileCol", set_tile_col},
    {SINGLE_INPUT, LOOP_FILTER_ENABLE, "LoopFilterEnable", set_enable_dlf_flag},
    {SINGLE_INPUT, CDEF_ENABLE_TOKEN, "CDEFLevel", set_cdef_enable},
    {SINGLE_INPUT, ENABLE_RESTORATION_TOKEN, "EnableRestoration", set_enable_restoration_flag},
    {SINGLE_INPUT, ENABLE_TPL_LA_TOKEN, "EnableTPLModel", set_enable_tpl_la},
    {SINGLE_INPUT, MFMV_ENABLE_NEW_TOKEN, "Mfmv", set_enable_mfmv_flag},
    {SINGLE_INPUT, FAST_DECODE_TOKEN, "FastDecode", set_fast_decode_flag},
    {SINGLE_INPUT, TUNE_TOKEN, "Tune", set_tune},
    //   ALT-REF filtering support
    {SINGLE_INPUT, ENABLE_TF_TOKEN, "EnableTf", set_enable_tf},
    {SINGLE_INPUT, ENABLE_OVERLAYS, "EnableOverlays", set_enable_overlays},
    {SINGLE_INPUT, SCREEN_CONTENT_TOKEN, "ScreenContentMode", set_screen_content_mode},
    {SINGLE_INPUT,
     RESTRICTED_MOTION_VECTOR,
     "RestrictedMotionVector",
     set_restricted_motion_vector},
    {SINGLE_INPUT, FILM_GRAIN_TOKEN, "FilmGrain", set_cfg_film_grain},
    {SINGLE_INPUT,
     FILM_GRAIN_DENOISE_APPLY_TOKEN,
     "FilmGrainDenoise",
     set_cfg_film_grain_denoise_apply},

    //   Super-resolution support
    {SINGLE_INPUT, SUPERRES_MODE_INPUT, "SuperresMode", set_superres_mode},
    {SINGLE_INPUT, SUPERRES_DENOM, "SuperresDenom", set_superres_denom},
    {SINGLE_INPUT, SUPERRES_KF_DENOM, "SuperresKfDenom", set_superres_kf_denom},
    {SINGLE_INPUT, SUPERRES_QTHRES, "SuperresQthres", set_superres_qthres},
    {SINGLE_INPUT, SUPERRES_KF_QTHRES, "SuperresKfQthres", set_superres_kf_qthres},

    // Switch frame support
    {SINGLE_INPUT, SFRAME_DIST_TOKEN, "SframeInterval", set_cfg_sframe_dist},
    {SINGLE_INPUT, SFRAME_MODE_TOKEN, "SframeMode", set_cfg_sframe_mode},
    // Reference Scaling support
    {SINGLE_INPUT, RESIZE_MODE_INPUT, "ResizeMode", set_resize_mode},
    {SINGLE_INPUT, RESIZE_DENOM, "ResizeDenom", set_resize_denom},
    {SINGLE_INPUT, RESIZE_KF_DENOM, "ResizeKfDenom", set_resize_kf_denom},

    // Color Description Options
    {SINGLE_INPUT, COLOR_PRIMARIES_NEW_TOKEN, "ColorPrimaries", set_cfg_color_primaries},
    {SINGLE_INPUT,
     TRANSFER_CHARACTERISTICS_NEW_TOKEN,
     "TransferCharacteristics",
     set_cfg_transfer_characteristics},
    {SINGLE_INPUT,
     MATRIX_COEFFICIENTS_NEW_TOKEN,
     "MatrixCoefficients",
     set_cfg_matrix_coefficients},
    {SINGLE_INPUT, COLOR_RANGE_NEW_TOKEN, "ColorRange", set_cfg_color_range},
    {SINGLE_INPUT,
     CHROMA_SAMPLE_POSITION_TOKEN,
     "ChromaSamplePosition",
     set_cfg_chroma_sample_position},
    {SINGLE_INPUT, MASTERING_DISPLAY_TOKEN, "MasteringDisplay", set_cfg_mastering_display},
    {SINGLE_INPUT, CONTENT_LIGHT_LEVEL_TOKEN, "ContentLightLevel", set_cfg_content_light},

    // QM
    {SINGLE_INPUT, ENABLE_QM_TOKEN, "EnableQM", set_enable_qm},
    {SINGLE_INPUT, MIN_QM_LEVEL_TOKEN, "MinQmLevel", set_min_qm_level},
    {SINGLE_INPUT, MAX_QM_LEVEL_TOKEN, "MaxQmLevel", set_max_qm_level},
    // Termination
    {SINGLE_INPUT, NULL, NULL, NULL}};

/**********************************
 * Constructor
 **********************************/
EbConfig *svt_config_ctor() {
    EbConfig *config_ptr = (EbConfig *)calloc(1, sizeof(EbConfig));
    if (!config_ptr)
        return NULL;
    config_ptr->error_log_file      = stderr;
    config_ptr->buffered_input      = -1;
    config_ptr->progress            = 1;
    config_ptr->injector_frame_rate = 60;

    return config_ptr;
}

/**********************************
 * Destructor
 **********************************/
void svt_config_dtor(EbConfig *config_ptr) {
    if (!config_ptr)
        return;
    // Close any files that are open
    if (config_ptr->input_file) {
        if (!config_ptr->input_file_is_fifo)
            fclose(config_ptr->input_file);
        config_ptr->input_file = (FILE *)NULL;
    }

    if (config_ptr->bitstream_file) {
        if (!fseek(config_ptr->bitstream_file, 0, SEEK_SET))
            write_ivf_stream_header(config_ptr, config_ptr->frames_encoded);
        fclose(config_ptr->bitstream_file);
        config_ptr->bitstream_file = (FILE *)NULL;
    }

    if (config_ptr->recon_file) {
        fclose(config_ptr->recon_file);
        config_ptr->recon_file = (FILE *)NULL;
    }

#if !REMOVE_MANUAL_PRED
    if (config_ptr->input_pred_struct_filename) {
        free(config_ptr->input_pred_struct_filename);
        config_ptr->input_pred_struct_filename = NULL;
    }
#endif

    if (config_ptr->error_log_file && config_ptr->error_log_file != stderr) {
        fclose(config_ptr->error_log_file);
        config_ptr->error_log_file = (FILE *)NULL;
    }

    if (config_ptr->qp_file) {
        fclose(config_ptr->qp_file);
        config_ptr->qp_file = (FILE *)NULL;
    }

    if (config_ptr->stat_file) {
        fclose(config_ptr->stat_file);
        config_ptr->stat_file = (FILE *)NULL;
    }

    if (config_ptr->output_stat_file) {
        fclose(config_ptr->output_stat_file);
        config_ptr->output_stat_file = (FILE *)NULL;
    }

    for (size_t i = 0; i < config_ptr->forced_keyframes.count; ++i)
        free(config_ptr->forced_keyframes.specifiers[i]);
    free(config_ptr->forced_keyframes.specifiers);
    free(config_ptr->forced_keyframes.frames);
    free((void *)config_ptr->stats);
    free(config_ptr);
    return;
}
EbErrorType enc_channel_ctor(EncChannel *c) {
    c->config = svt_config_ctor();
    if (!c->config)
        return EB_ErrorInsufficientResources;
    c->app_callback = (EbAppContext *)malloc(sizeof(EbAppContext));
    if (!c->app_callback)
        return EB_ErrorInsufficientResources;
    memset(c->app_callback, 0, sizeof(EbAppContext));
    c->exit_cond        = APP_ExitConditionError;
    c->exit_cond_output = APP_ExitConditionError;
    c->exit_cond_recon  = APP_ExitConditionError;
    c->exit_cond_input  = APP_ExitConditionError;
    c->active           = FALSE;
    return svt_av1_enc_init_handle(
        &c->app_callback->svt_encoder_handle, c->app_callback, &c->config->config);
}

void enc_channel_dctor(EncChannel *c, uint32_t inst_cnt) {
    EbAppContext *ctx = c->app_callback;
    if (ctx && ctx->svt_encoder_handle) {
        svt_av1_enc_deinit(ctx->svt_encoder_handle);
        de_init_encoder(ctx, inst_cnt);
    }
    svt_config_dtor(c->config);
    free(c->app_callback);
}

/**********************************
 * File Size
 **********************************/
static int32_t find_file_size(FILE *const pFile) {
    int32_t file_size;

    fseek(pFile, 0, SEEK_END);
    file_size = ftell(pFile);
    rewind(pFile);

    return file_size;
}

/**********************************
 * Line Split
 **********************************/
static void line_split(uint32_t *argc, char *argv[CONFIG_FILE_MAX_ARG_COUNT],
                       uint32_t arg_len[CONFIG_FILE_MAX_ARG_COUNT], char *linePtr) {
    uint32_t i = 0;
    *argc      = 0;

    while ((*linePtr != CONFIG_FILE_NEWLINE_CHAR) && (*linePtr != CONFIG_FILE_RETURN_CHAR) &&
           (*linePtr != CONFIG_FILE_COMMENT_CHAR) && (*argc < CONFIG_FILE_MAX_ARG_COUNT)) {
        // Increment past whitespace
        while ((*linePtr == CONFIG_FILE_SPACE_CHAR || *linePtr == CONFIG_FILE_TAB_CHAR) &&
               (*linePtr != CONFIG_FILE_NEWLINE_CHAR))
            ++linePtr;
        // Set arg
        if ((*linePtr != CONFIG_FILE_NEWLINE_CHAR) && (*linePtr != CONFIG_FILE_RETURN_CHAR) &&
            (*linePtr != CONFIG_FILE_COMMENT_CHAR) && (*argc < CONFIG_FILE_MAX_ARG_COUNT)) {
            argv[*argc] = linePtr;

            // Increment to next whitespace
            while (*linePtr != CONFIG_FILE_SPACE_CHAR && *linePtr != CONFIG_FILE_TAB_CHAR &&
                   *linePtr != CONFIG_FILE_NEWLINE_CHAR && *linePtr != CONFIG_FILE_RETURN_CHAR) {
                ++linePtr;
                ++i;
            }

            // Set arg length
            arg_len[(*argc)++] = i;

            i = 0;
        }
    }

    return;
}

/**********************************
* Set Config value
**********************************/
static void set_config_value(EbConfig *config, const char *name, const char *value) {
    int32_t i = 0;

    while (config_entry[i].name != NULL) {
        if (strcmp(config_entry[i].name, name) == 0)
            (*config_entry[i].scf)((const char *)value, config);
        ++i;
    }

    return;
}

/**********************************
* Parse Config File
**********************************/
static void parse_config_file(EbConfig *config, char *buffer, int32_t size) {
    uint32_t argc;
    char    *argv[CONFIG_FILE_MAX_ARG_COUNT];
    uint32_t arg_len[CONFIG_FILE_MAX_ARG_COUNT];

    char var_name[CONFIG_FILE_MAX_VAR_LEN];
    char var_value[CONFIG_FILE_MAX_ARG_COUNT][CONFIG_FILE_MAX_VAR_LEN];

    uint32_t value_index;

    uint32_t comment_section_flag = 0;
    uint32_t new_line_flag        = 0;

    // Keep looping until we process the entire file
    while (size--) {
        comment_section_flag = ((*buffer == CONFIG_FILE_COMMENT_CHAR) ||
                                (comment_section_flag != 0))
            ? 1
            : comment_section_flag;

        // At the beginning of each line
        if ((new_line_flag == 1) && (comment_section_flag == 0)) {
            // Do an argc/argv split for the line
            line_split(&argc, argv, arg_len, buffer);

            if ((argc > 2) && (*argv[1] == CONFIG_FILE_VALUE_SPLIT)) {
                // ***NOTE - We're assuming that the variable name is the first arg and
                // the variable value is the third arg.

                // Cap the length of the variable name
                arg_len[0] = (arg_len[0] > CONFIG_FILE_MAX_VAR_LEN - 1)
                    ? CONFIG_FILE_MAX_VAR_LEN - 1
                    : arg_len[0];
                // Copy the variable name
                strncpy_s(var_name, CONFIG_FILE_MAX_VAR_LEN, argv[0], arg_len[0]);
                // Null terminate the variable name
                var_name[arg_len[0]] = CONFIG_FILE_NULL_CHAR;

                for (value_index = 0;
                     (value_index < CONFIG_FILE_MAX_ARG_COUNT - 2) && (value_index < (argc - 2));
                     ++value_index) {
                    // Cap the length of the variable
                    arg_len[value_index + 2] = (arg_len[value_index + 2] >
                                                CONFIG_FILE_MAX_VAR_LEN - 1)
                        ? CONFIG_FILE_MAX_VAR_LEN - 1
                        : arg_len[value_index + 2];
                    // Copy the variable name
                    strncpy_s(var_value[value_index],
                              CONFIG_FILE_MAX_VAR_LEN,
                              argv[value_index + 2],
                              arg_len[value_index + 2]);
                    // Null terminate the variable name
                    var_value[value_index][arg_len[value_index + 2]] = CONFIG_FILE_NULL_CHAR;

                    set_config_value(config, var_name, var_value[value_index]);
                }
            }
        }

        comment_section_flag = (*buffer == CONFIG_FILE_NEWLINE_CHAR) ? 0 : comment_section_flag;
        new_line_flag        = (*buffer == CONFIG_FILE_NEWLINE_CHAR) ? 1 : 0;
        ++buffer;
    }

    return;
}

/**
 * @brief Find token and its argument
 * @param argc      Argument count
 * @param argv      Argument array
 * @param token     The token to look for
 * @param configStr Output buffer to write the argument to or NULL
 * @return 0 if found, non-zero otherwise
 *
 * @note The configStr buffer must be at least
 *       COMMAND_LINE_MAX_SIZE bytes big.
 *       The argv must contain an additional NULL
 *       element to terminate it, so that
 *       argv[argc] == NULL.
 */
static int32_t find_token(int32_t argc, char *const argv[], char const *token, char *configStr) {
    assert(argv[argc] == NULL);

    if (argc == 0)
        return -1;

    for (int32_t i = argc - 1; i >= 0; i--) {
        if (strcmp(argv[i], token) != 0)
            continue;

        // The argument matches the token.
        // If given, try to copy its argument to configStr
        if (configStr && argv[i + 1] != NULL) {
            strcpy_s(configStr, COMMAND_LINE_MAX_SIZE, argv[i + 1]);
        } else if (configStr) {
            configStr[0] = '\0';
        }

        return 0;
    }
    return -1;
}

/**********************************
* Read Config File
**********************************/
static int32_t read_config_file(EbConfig *config, char *config_path, uint32_t instance_idx) {
    int32_t return_error = 0;

    FILE *config_file;

    // Open the config file
    FOPEN(config_file, config_path, "rb");

    if (config_file) {
        int32_t config_file_size   = find_file_size(config_file);
        char   *config_file_buffer = (char *)malloc(config_file_size);

        if (config_file_buffer) {
            int32_t result_size = (int32_t)fread(
                config_file_buffer, 1, config_file_size, config_file);

            if (result_size == config_file_size) {
                parse_config_file(config, config_file_buffer, config_file_size);
            } else {
                fprintf(stderr, "Error channel %u: File Read Failed\n", instance_idx + 1);
                return_error = -1;
            }
        } else {
            fprintf(stderr, "Error channel %u: Memory Allocation Failed\n", instance_idx + 1);
            return_error = -1;
        }

        free(config_file_buffer);
        fclose(config_file);
    } else {
        fprintf(stderr,
                "Error channel %u: Couldn't open Config File: %s\n",
                instance_idx + 1,
                config_path);
        return_error = -1;
    }

    return return_error;
}

/* get config->rc_stats_buffer from config->input_stat_file */
Bool load_twopass_stats_in(EbConfig *cfg) {
    EbSvtAv1EncConfiguration *config = &cfg->config;
#ifdef _WIN32
    int          fd = _fileno(cfg->input_stat_file);
    struct _stat file_stat;
    int          ret = _fstat(fd, &file_stat);
#else
    int         fd                  = fileno(cfg->input_stat_file);
    struct stat file_stat;
    int         ret         = fstat(fd, &file_stat);
#endif
    if (ret) {
        return FALSE;
    }
    config->rc_stats_buffer.buf = malloc(file_stat.st_size);
    if (config->rc_stats_buffer.buf) {
        config->rc_stats_buffer.sz = (uint64_t)file_stat.st_size;
        if (fread(config->rc_stats_buffer.buf, 1, file_stat.st_size, cfg->input_stat_file) !=
            (size_t)file_stat.st_size) {
            return FALSE;
        }
        if (file_stat.st_size == 0) {
            return FALSE;
        }
    }
    return config->rc_stats_buffer.buf != NULL;
}
EbErrorType handle_stats_file(EbConfig *config, EncPass enc_pass,
                              const SvtAv1FixedBuf *rc_stats_buffer, uint32_t channel_number) {
    switch (enc_pass) {
    case ENC_SINGLE_PASS: {
        const char *stats = config->stats ? config->stats : "svtav1_2pass.log";
        if (config->config.pass == 1) {
            if (!fopen_and_lock(&config->output_stat_file, stats, TRUE)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't open stats file %s for write \n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
        }

        // Multi pass VBR has 3 passes, and pass = 2 is the middle pass
        // In this pass, data is read from the file, copied to memory, updated and
        // written back to the same file
        else if (config->config.pass == 2 &&
                 config->config.rate_control_mode == SVT_AV1_RC_MODE_VBR) {
            if (!fopen_and_lock(&config->input_stat_file, stats, FALSE)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't read stats file %s for read\n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
            // Copy from file to memory
            if (!load_twopass_stats_in(config)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't load file %s\n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
            // Close the input stat file
            if (config->input_stat_file) {
                fclose(config->input_stat_file);
                config->input_stat_file = (FILE *)NULL;
            }
            // Open the file in write mode
            if (!fopen_and_lock(&config->output_stat_file, stats, TRUE)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't open stats file %s for write \n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
        }
        // Final pass: pass = 2 for CRF and pass = 3 for VBR
        else if ((config->config.pass == 2 &&
                  config->config.rate_control_mode == SVT_AV1_RC_MODE_CQP_OR_CRF) ||
                 (config->config.pass == 3 &&
                  config->config.rate_control_mode == SVT_AV1_RC_MODE_VBR)) {
            if (!fopen_and_lock(&config->input_stat_file, stats, FALSE)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't read stats file %s for read\n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
            if (!load_twopass_stats_in(config)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't load file %s\n",
                        channel_number + 1,
                        stats);
                return EB_ErrorBadParameter;
            }
        }
        break;
    }

    case ENC_FIRST_PASS: {
        // for combined two passes,
        // we only ouptut first pass stats when user explicitly set the --stats
        if (config->stats) {
            if (!fopen_and_lock(&config->output_stat_file, config->stats, TRUE)) {
                fprintf(config->error_log_file,
                        "Error instance %u: can't open stats file %s for write \n",
                        channel_number + 1,
                        config->stats);
                return EB_ErrorBadParameter;
            }
        }
        break;
    }

    case ENC_SECOND_PASS: {
        if (!rc_stats_buffer->sz) {
            fprintf(config->error_log_file,
                    "Error instance %u: combined multi passes need stats in for the middle pass \n",
                    channel_number + 1);
            return EB_ErrorBadParameter;
        }
        config->config.rc_stats_buffer = *rc_stats_buffer;
        break;
    }

    case ENC_THIRD_PASS: {
        if (!rc_stats_buffer->sz) {
            fprintf(config->error_log_file,
                    "Error instance %u: combined multi passes need stats in for the final pass \n",
                    channel_number + 1);
            return EB_ErrorBadParameter;
        }
        config->config.rc_stats_buffer = *rc_stats_buffer;
        break;
    }

    default: {
        assert(0);
        break;
    }
    }
    return EB_ErrorNone;
}
/******************************************
* Verify Settings
******************************************/
static EbErrorType app_verify_config(EbConfig *config, uint32_t channel_number) {
    EbErrorType return_error = EB_ErrorNone;

    // Check Input File
    if (config->input_file == (FILE *)NULL) {
        fprintf(
            config->error_log_file, "Error instance %u: Invalid Input File\n", channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->frames_to_be_encoded <= -1) {
        fprintf(config->error_log_file,
                "Error instance %u: FrameToBeEncoded must be greater than 0\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->buffered_input == 0) {
        fprintf(config->error_log_file,
                "Error instance %u: Buffered Input cannot be 0\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->buffered_input < -1) {
        fprintf(config->error_log_file,
                "Error instance %u: Invalid buffered_input. buffered_input must be -1 or greater "
                "than or equal to 1\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->buffered_input != -1 && config->y4m_input) {
        fprintf(config->error_log_file,
                "Error instance %u: Buffered input is currently not available with y4m inputs\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->buffered_input > config->frames_to_be_encoded) {
        fprintf(config->error_log_file,
                "Error instance %u: Invalid buffered_input. buffered_input must be less or equal "
                "to the number of frames to be encoded\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->config.use_qp_file == TRUE && config->qp_file == NULL) {
        fprintf(config->error_log_file,
                "Error instance %u: Could not find QP file, UseQpFile is set to 1\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->injector > 1) {
        fprintf(config->error_log_file,
                "Error Instance %u: Invalid injector [0 - 1]\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    if (config->injector_frame_rate > 240 && config->injector) {
        fprintf(config->error_log_file,
                "Error Instance %u: The maximum allowed injector_frame_rate is 240 fps\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }
    // Check that the injector frame_rate is non-zero
    if (!config->injector_frame_rate && config->injector) {
        fprintf(config->error_log_file,
                "Error Instance %u: The injector frame rate should be greater than 0 fps \n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }
    if (config->config.frame_rate_numerator == 0 || config->config.frame_rate_denominator == 0) {
        fprintf(config->error_log_file,
                "Error Instance %u: The frame_rate_numerator and frame_rate_denominator should be "
                "greater than 0\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    } else if (config->config.frame_rate_numerator / config->config.frame_rate_denominator > 240) {
        fprintf(config->error_log_file,
                "Error Instance %u: The maximum allowed frame_rate is 240 fps\n",
                channel_number + 1);
        return_error = EB_ErrorBadParameter;
    }

    return return_error;
}

static const char *TOKEN_READ_MARKER  = "THIS_TOKEN_HAS_BEEN_READ";
static const char *TOKEN_ERROR_MARKER = "THIS_TOKEN_HAS_ERROR";

/**
 * @brief Finds the arguments for a specific token
 *
 * @param nch number of channels (number of arguemnts to find for a token)
 * @param argc argc from main()
 * @param argv argv from main()
 * @param token token to find
 * @param configStr array of pointers to store the arguments into
 * @param cmd_copy array of tokens based on splitting argv
 * @param arg_copy array of arguments based on splitting argv
 * @return true token was found and configStr was populated
 * @return false token was not found and configStr was not populated
 */
static bool find_token_multiple_inputs(unsigned nch, int argc, char *const argv[],
                                       const char *token, char *configStr[MAX_CHANNEL_NUMBER],
                                       const char *cmd_copy[MAX_NUM_TOKENS],
                                       const char *arg_copy[MAX_NUM_TOKENS]) {
    bool return_error   = false;
    bool has_duplicates = false;
    // Loop over all the arguments
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], token))
            continue;
        if (return_error)
            has_duplicates = true;
        return_error = true;
        if (i + 1 >= argc) {
            // if the token is at the end of the command line without arguments
            // set sentinel value
            strcpy_s(configStr[0], COMMAND_LINE_MAX_SIZE, " ");
            return return_error;
        }
        cmd_copy[i] = TOKEN_READ_MARKER; // mark token as read
        // consume arguments
        for (unsigned count = 0; count < nch; ++count) {
            const int j = i + 1 + count;
            if (j >= argc || cmd_copy[j]) {
                // stop if we ran out of arguments or if we hit a token
                strcpy_s(configStr[count], COMMAND_LINE_MAX_SIZE, " ");
                continue;
            }
            strcpy_s(configStr[count], COMMAND_LINE_MAX_SIZE, argv[j]);
            arg_copy[j] = TOKEN_READ_MARKER;
        }
    }

    if (return_error && !strcmp(configStr[0], " ")) {
        // if no argument was found, print an error message
        // we don't support flip switches, so this will need to be changed if we ever do.
        fprintf(stderr, "[SVT-Error]: No argument found for token `%s`\n", token);
        strcpy_s(configStr[0], COMMAND_LINE_MAX_SIZE, TOKEN_ERROR_MARKER);
    }

    if (has_duplicates) {
        fprintf(stderr, "\n[SVT-Warning]: Duplicate option %s specified, only `%s", token, token);
        for (unsigned count = 0; count < nch; ++count) fprintf(stderr, " %s", configStr[count]);
        fprintf(stderr, "` will apply\n\n");
    }

    return return_error;
}

static int check_long(ConfigEntry cfg_entry, ConfigEntry cfg_entry_next) {
    return cfg_entry_next.name ? !strcmp(cfg_entry.name, cfg_entry_next.name) : 0;
}

int get_version(int argc, char *argv[]) {
#ifdef NDEBUG
    static int debug_build = 1;
#else
    static int  debug_build = 0;
#endif
    if (find_token(argc, argv, VERSION_TOKEN, NULL))
        return 0;
    printf("SVT-AV1 %s (%s)\n", svt_av1_get_version(), debug_build ? "release" : "debug");
    return 1;
}

uint32_t get_help(int32_t argc, char *const argv[]) {
    char config_string[COMMAND_LINE_MAX_SIZE];
    if (find_token(argc, argv, HELP_TOKEN, config_string))
        return 0;

    printf(
        "Usage: SvtAv1EncApp <options> <-b dst_filename> -i src_filename\n"
        "\n"
        "Examples:\n"
        "Multi-pass encode (VBR):\n"
        "    SvtAv1EncApp <--stats svtav1_2pass.log> --passes 2 --rc 1 --tbr 1000 -b dst_filename "
        "-i src_filename\n"
        "Multi-pass encode (CRF):\n"
        "    SvtAv1EncApp <--stats svtav1_2pass.log> --passes 2 --rc 0 --crf 43 -b dst_filename -i "
        "src_filename\n"
        "Single-pass encode (VBR):\n"
        "    SvtAv1EncApp --passes 1 --rc 1 --tbr 1000 -b dst_filename -i src_filename\n"
        "\n"
        "Options:\n");
    for (ConfigEntry *options_token_index = config_entry_options; options_token_index->token;
         ++options_token_index) {
        // this only works if short and long token are one after another
        switch (check_long(*options_token_index, options_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   options_token_index->token,
                   options_token_index[1].token,
                   options_token_index->name);
            ++options_token_index;
            break;
        default:
            printf(options_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                        : "      -%-25s   %-25s\n",
                   options_token_index->token,
                   options_token_index->name);
        }
    }
    printf("\nEncoder Global Options:\n");
    for (ConfigEntry *global_options_token_index = config_entry_global_options;
         global_options_token_index->token;
         ++global_options_token_index) {
        switch (check_long(*global_options_token_index, global_options_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   global_options_token_index->token,
                   global_options_token_index[1].token,
                   global_options_token_index->name);
            ++global_options_token_index;
            break;
        default:
            printf(global_options_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                               : "      -%-25s   %-25s\n",
                   global_options_token_index->token,
                   global_options_token_index->name);
        }
    }
    printf("\nRate Control Options:\n");
    for (ConfigEntry *rc_token_index = config_entry_rc; rc_token_index->token; ++rc_token_index) {
        switch (check_long(*rc_token_index, rc_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   rc_token_index->token,
                   rc_token_index[1].token,
                   rc_token_index->name);
            ++rc_token_index;
            break;
        default:
            printf(rc_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                   : "      -%-25s   %-25s\n",
                   rc_token_index->token,
                   rc_token_index->name);
        }
    }
    printf("\nMulti-pass Options:\n");
    for (ConfigEntry *two_p_token_index = config_entry_2p; two_p_token_index->token;
         ++two_p_token_index) {
        switch (check_long(*two_p_token_index, two_p_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   two_p_token_index->token,
                   two_p_token_index[1].token,
                   two_p_token_index->name);
            ++two_p_token_index;
            break;
        default:
            printf(two_p_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                      : "      -%-25s   %-25s\n",
                   two_p_token_index->token,
                   two_p_token_index->name);
        }
    }
    printf("\nGOP size and type Options:\n");
    for (ConfigEntry *kf_token_index = config_entry_intra_refresh; kf_token_index->token;
         ++kf_token_index) {
        switch (check_long(*kf_token_index, kf_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   kf_token_index->token,
                   kf_token_index[1].token,
                   kf_token_index->name);
            ++kf_token_index;
            break;
        default:
            printf(kf_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                   : "      -%-25s   %-25s\n",
                   kf_token_index->token,
                   kf_token_index->name);
        }
    }
    printf("\nAV1 Specific Options:\n");
    for (ConfigEntry *sp_token_index = config_entry_specific; sp_token_index->token;
         ++sp_token_index) {
        switch (check_long(*sp_token_index, sp_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   sp_token_index->token,
                   sp_token_index[1].token,
                   sp_token_index->name);
            ++sp_token_index;
            break;
        default:
            printf(sp_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                   : "      -%-25s   %-25s\n",
                   sp_token_index->token,
                   sp_token_index->name);
        }
    }
    printf("\nColor Description Options:\n");
    for (ConfigEntry *cd_token_index = config_entry_color_description; cd_token_index->token;
         ++cd_token_index) {
        switch (check_long(*cd_token_index, cd_token_index[1])) {
        case 1:
            printf("  %s, %-25s    %-25s\n",
                   cd_token_index->token,
                   cd_token_index[1].token,
                   cd_token_index->name);
            ++cd_token_index;
            break;
        default:
            printf(cd_token_index->token[1] == '-' ? "      %-25s    %-25s\n"
                                                   : "      -%-25s   %-25s\n",
                   cd_token_index->token,
                   cd_token_index->name);
        }
    }
    return 1;
}

/******************************************************
* Get the number of channels and validate it with input
******************************************************/
uint32_t get_number_of_channels(int32_t argc, char *const argv[]) {
    char config_string[COMMAND_LINE_MAX_SIZE];
    if (find_token(argc, argv, CHANNEL_NUMBER_TOKEN, config_string) == 0) {
        // Set the input file
        uint32_t channel_number = strtol(config_string, NULL, 0);
        if ((channel_number > MAX_CHANNEL_NUMBER) || channel_number == 0) {
            fprintf(stderr,
                    "[SVT-Error]: The number of channels has to be within the range [1,%u]\n",
                    MAX_CHANNEL_NUMBER);
            return 0;
        }
        return channel_number;
    }
    return 1;
}

static Bool check_two_pass_conflicts(int32_t argc, char *const argv[]) {
    char        config_string[COMMAND_LINE_MAX_SIZE];
    const char *conflicts[] = {
        PASS_TOKEN,
        NULL,
    };
    int         i = 0;
    const char *token;
    while ((token = conflicts[i])) {
        if (find_token(argc, argv, token, config_string) == 0) {
            fprintf(
                stderr, "[SVT-Error]: --passes is not accepted in combination with %s\n", token);
            return TRUE;
        }
        i++;
    }
    return FALSE;
}
/*
* Returns the number of passes, multi_pass_mode
*/
uint32_t get_passes(int32_t argc, char *const argv[], EncPass enc_pass[MAX_ENC_PASS]) {
    char           config_string[COMMAND_LINE_MAX_SIZE];
    MultiPassModes multi_pass_mode;

    int rc_mode = 0;
    // copied from str_to_rc_mode()
    const struct {
        const char *name;
        uint32_t    mode;
    } rc[] = {
        {"0", 0},
        {"1", 1},
        {"2", 2},
        {"cqp", 0},
        {"crf", 0},
        {"vbr", 1},
        {"cbr", 2},
    };
    const size_t rc_size  = sizeof(rc) / sizeof(rc[0]);
    int          enc_mode = 0;
    int          ip       = -1;
    // Read required inputs to decide on the number of passes and check the validity of their ranges
    if (find_token(argc, argv, RATE_CONTROL_ENABLE_TOKEN, config_string) == 0) {
        for (size_t i = 0; i < rc_size; i++) {
            if (!strcmp(config_string, rc[i].name)) {
                rc_mode = rc[i].mode;
                break;
            }
        }
        if (rc_mode > 2 || rc_mode < 0) {
            fprintf(stderr, "Error: The rate control mode must be [0 - 2] \n");
            return 0;
        }
    }

    int32_t passes     = -1;
    int     using_fifo = 0;

    if (find_token(argc, argv, INPUT_FILE_LONG_TOKEN, config_string) == 0 ||
        find_token(argc, argv, INPUT_FILE_TOKEN, config_string) == 0) {
        if (!strcmp(config_string, "stdin")) {
            using_fifo = 1;
        } else {
#ifdef _WIN32
            HANDLE in_file = CreateFile(
                config_string, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (in_file != INVALID_HANDLE_VALUE) {
                using_fifo = GetFileType(in_file) == FILE_TYPE_PIPE;
                CloseHandle(in_file);
            }
#else
            struct stat st;
            using_fifo = !stat(config_string, &st) && S_ISFIFO(st.st_mode);
#endif
        }
    }
    if (find_token(argc, argv, PRESET_TOKEN, config_string) == 0) {
        enc_mode = strtol(config_string, NULL, 0);
        if (enc_mode > MAX_ENC_PRESET || enc_mode < 0) {
            fprintf(stderr, "Error: EncoderMode must be in the range of [0-%d]\n", MAX_ENC_PRESET);
            return 0;
        }
    }

    if (!find_token(argc, argv, INTRA_PERIOD_TOKEN, NULL) &&
        !find_token(argc, argv, KEYINT_TOKEN, NULL)) {
        fprintf(stderr,
                "[SVT-Warning]: --keyint and --intra-period specified, --keyint will take "
                "precedence!\n");
    }

    if (find_token(argc, argv, INTRA_PERIOD_TOKEN, config_string) == 0 ||
        find_token(argc, argv, KEYINT_TOKEN, config_string) == 0) {
        ip = strtol(config_string, NULL, 0);
        if (find_token(argc, argv, KEYINT_TOKEN, NULL) == 0) {
            fprintf(stderr, "[SVT-Warning]: --keyint is now intra-period + 1!\n");
            ip = ip < 0 ? ip : ip - 1;
        } else
            fprintf(stderr, "[SVT-Warning]: --intra-period is deprecated for --keyint\n");
        if ((ip < -2 || ip > 2 * ((1 << 30) - 1)) && rc_mode == 0) {
            fprintf(stderr, "[SVT-Error]: The intra period must be [-2, 2^31-2], input %d\n", ip);
            return 0;
        }
        if ((ip < 0) && rc_mode == 1) {
            fprintf(stderr,
                    "[SVT-Error]: The intra period must be > 0 for RateControlMode %d \n",
                    rc_mode);
            return 0;
        }
    }

    if (find_token(argc, argv, PASSES_TOKEN, config_string) == 0) {
        passes = strtol(config_string, NULL, 0);
        if (passes == 0 || passes > 2) {
            fprintf(stderr,
                    "[SVT-Error]: The number of passes has to be within the range [1,2], 2 being "
                    "multi-pass encoding\n");
            return 0;
        }
    }

    if (passes != -1 && check_two_pass_conflicts(argc, argv))
        return 0;

    // set default passes to 1 if not specified by the user
    passes = (passes == -1) ? 1 : passes;

    if (using_fifo && passes > 1) {
        fprintf(
            stderr,
            "[SVT-Warning]: The number of passes has to be 1 when using a fifo, using 1-pass\n");
        multi_pass_mode = SINGLE_PASS;
        passes          = 1;
    }
    // Determine the number of passes in CRF mode
    if (rc_mode == 0) {
#if FIX_SHORT_KEYINT_WARN
        if (ip > -1 && ip < 16 && passes != 1) {
#else
        if (ip > -1 && ip < 16) {
#endif
            passes = 1;
            fprintf(
                stderr,
                "[SVT-Warning]: Multipass CRF is not supported for Intra_period %d. Switching to "
                "1-pass encoding\n\n",
                ip);
        }
        multi_pass_mode = passes == 2 ? TWO_PASS_IPP_FINAL : SINGLE_PASS;
    }
    // Determine the number of passes in rate control mode
    else if (rc_mode == 1) {
        if (passes == 1)
            multi_pass_mode = SINGLE_PASS;
        else if (passes > 1) {
            if (enc_mode > ENC_M12) {
                fprintf(stderr,
                        "[SVT-Warning]: Multipass VBR is not supported for preset %d. Switching to "
                        "1-pass encoding\n\n",
                        enc_mode);
                passes          = 1;
                multi_pass_mode = SINGLE_PASS;
            } else {
                passes          = 3;
                multi_pass_mode = THREE_PASS_IPP_SAMEPRED_FINAL;
            }
        }
    } else {
        if (passes > 1) {
            fprintf(
                stderr,
                "[SVT-Warning]: Multipass CBR is not supported. Switching to 1-pass encoding\n\n");
            passes = 1;
        }
        multi_pass_mode = SINGLE_PASS;
    }

    // Set the settings for each pass based on multi_pass_mode
    switch (multi_pass_mode) {
    case SINGLE_PASS: enc_pass[0] = ENC_SINGLE_PASS; break;
    case TWO_PASS_IPP_FINAL:
        enc_pass[0] = ENC_FIRST_PASS;
        enc_pass[1] = ENC_SECOND_PASS;
        break;
    case THREE_PASS_IPP_SAMEPRED_FINAL:
        enc_pass[0] = ENC_FIRST_PASS;
        enc_pass[1] = ENC_SECOND_PASS;
        enc_pass[2] = ENC_THIRD_PASS;
        break;
    default: break;
    }

    return passes;
}

static Bool is_negative_number(const char *string) {
    char *end;
    return strtol(string, &end, 10) < 0 && *end == '\0';
}

// Computes the number of frames in the input file
int32_t compute_frames_to_be_encoded(EbConfig *config) {
    uint64_t file_size   = 0;
    int32_t  frame_count = 0;
    uint32_t frame_size;

    // Pipes contain data streams whose end we cannot know before we reach it.
    // For pipes, we leave it up to the eof logic to detect how many frames to eventually encode.
    if (config->input_file == stdin || config->input_file_is_fifo)
        return -1;

    if (config->input_file) {
        uint64_t curr_loc = ftello(config->input_file); // get current fp location
        fseeko(config->input_file, 0L, SEEK_END);
        file_size = ftello(config->input_file);
        fseeko(config->input_file, curr_loc, SEEK_SET); // seek back to that location
    }
    frame_size = config->input_padded_width * config->input_padded_height; // Luma
    frame_size += 2 * (frame_size >> (3 - config->config.encoder_color_format)); // Add Chroma
    frame_size = frame_size << ((config->config.encoder_bit_depth == 10) ? 1 : 0);

    if (frame_size == 0)
        return -1;

    frame_count = (int32_t)(file_size / frame_size);

    if (frame_count == 0)
        return -1;

    return frame_count;
}

#if !REMOVE_MANUAL_PRED
/**********************************
* Parse Pred Struct File
**********************************/
static int32_t parse_pred_struct_file(EbConfig *config, char *buffer, int32_t size) {
    uint32_t argc;
    char    *argv[CONFIG_FILE_MAX_ARG_COUNT];
    uint32_t arg_len[CONFIG_FILE_MAX_ARG_COUNT];

    char var_name[CONFIG_FILE_MAX_VAR_LEN];
    char var_value[CONFIG_FILE_MAX_ARG_COUNT][CONFIG_FILE_MAX_VAR_LEN];

    uint32_t value_index;

    uint32_t                  comment_section_flag = 0;
    uint32_t                  new_line_flag        = 0;
    int32_t                   entry_num            = 0;
    int32_t                   display_order = 0, num_ref_list0 = 0, num_ref_list1 = 0;
    int32_t                   idx_ref_list0 = 0, idx_ref_list1 = 0;
    EbSvtAv1EncConfiguration *cfg = &config->config;
    // Keep looping until we process the entire file
    while (size--) {
        comment_section_flag = ((*buffer == CONFIG_FILE_COMMENT_CHAR) ||
                                (comment_section_flag != 0))
            ? 1
            : comment_section_flag;

        // At the beginning of each line
        if ((new_line_flag == 1) && (comment_section_flag == 0)) {
            // Do an argc/argv split for the line
            line_split(&argc, argv, arg_len, buffer);

            if ((argc > 2) && (*argv[1] == CONFIG_FILE_VALUE_SPLIT)) {
                // ***NOTE - We're assuming that the variable name is the first arg and
                // the variable value is the third arg.

                // Cap the length of the variable name
                arg_len[0] = (arg_len[0] > CONFIG_FILE_MAX_VAR_LEN - 1)
                    ? CONFIG_FILE_MAX_VAR_LEN - 1
                    : arg_len[0];
                // Copy the variable name
                strncpy_s(var_name, CONFIG_FILE_MAX_VAR_LEN, argv[0], arg_len[0]);
                // Null terminate the variable name
                var_name[arg_len[0]] = CONFIG_FILE_NULL_CHAR;
                if (strcmp(var_name, "PredStructEntry")) {
                    continue;
                }

                ++entry_num;
                idx_ref_list0 = idx_ref_list1 = num_ref_list0 = num_ref_list1 = 0;

                for (value_index = 0;
                     (value_index < CONFIG_FILE_MAX_ARG_COUNT - 2) && (value_index < (argc - 2));
                     ++value_index) {
                    // Cap the length of the variable
                    arg_len[value_index + 2] = (arg_len[value_index + 2] >
                                                CONFIG_FILE_MAX_VAR_LEN - 1)
                        ? CONFIG_FILE_MAX_VAR_LEN - 1
                        : arg_len[value_index + 2];
                    // Copy the variable name
                    strncpy_s(var_value[value_index],
                              CONFIG_FILE_MAX_VAR_LEN,
                              argv[value_index + 2],
                              arg_len[value_index + 2]);
                    // Null terminate the variable name
                    var_value[value_index][arg_len[value_index + 2]] = CONFIG_FILE_NULL_CHAR;

                    switch (value_index) {
                    case 0:
                        display_order = strtoul(var_value[value_index], NULL, 0);
                        if (display_order >= (1 << (MAX_HIERARCHICAL_LEVEL - 1))) {
                            return -1;
                        }
                        break;
                    case 1:
                        cfg->pred_struct[display_order].decode_order = strtoul(
                            var_value[value_index], NULL, 0);
                        if (cfg->pred_struct[display_order].decode_order >=
                            (1 << (MAX_HIERARCHICAL_LEVEL - 1))) {
                            return -1;
                        }
                        break;
                    case 2:
                        cfg->pred_struct[display_order].temporal_layer_index = strtoul(
                            var_value[value_index], NULL, 0);
                        break;
                    case 3: num_ref_list0 = strtoul(var_value[value_index], NULL, 0); break;
                    case 4: num_ref_list1 = strtoul(var_value[value_index], NULL, 0); break;
                    default:
                        if (idx_ref_list0 < num_ref_list0) {
                            if (idx_ref_list0 < REF_LIST_MAX_DEPTH) {
                                cfg->pred_struct[display_order].ref_list0[idx_ref_list0] = strtoul(
                                    var_value[value_index], NULL, 0);
                            }
                            ++idx_ref_list0;
                        } else if (idx_ref_list1 < num_ref_list1) {
                            if (idx_ref_list1 < REF_LIST_MAX_DEPTH - 1) {
                                cfg->pred_struct[display_order].ref_list1[idx_ref_list1] = strtoul(
                                    var_value[value_index], NULL, 0);
                            }
                            ++idx_ref_list1;
                        }
                        break;
                    }
                }
                for (; num_ref_list0 < REF_LIST_MAX_DEPTH; ++num_ref_list0) {
                    cfg->pred_struct[display_order].ref_list0[num_ref_list0] = 0;
                }
                for (; num_ref_list1 < REF_LIST_MAX_DEPTH - 1; ++num_ref_list1) {
                    cfg->pred_struct[display_order].ref_list1[num_ref_list1] = 0;
                }
            }
        }

        comment_section_flag = (*buffer == CONFIG_FILE_NEWLINE_CHAR) ? 0 : comment_section_flag;
        new_line_flag        = (*buffer == CONFIG_FILE_NEWLINE_CHAR) ? 1 : 0;
        ++buffer;
    }
    cfg->manual_pred_struct_entry_num = entry_num;

    return 0;
}

/**********************************
* Read Prediction Structure File
**********************************/
static int32_t read_pred_struct_file(EbConfig *config, char *PredStructPath,
                                     uint32_t instance_idx) {
    int32_t return_error = 0;

    FILE *input_pred_struct_file;

    FOPEN(input_pred_struct_file, PredStructPath, "rb");

    if (input_pred_struct_file) {
        int32_t config_file_size   = find_file_size(input_pred_struct_file);
        char   *config_file_buffer = (char *)malloc(config_file_size);

        if (config_file_buffer) {
            int32_t result_size = (int32_t)fread(
                config_file_buffer, 1, config_file_size, input_pred_struct_file);

            if (result_size == config_file_size) {
                parse_pred_struct_file(config, config_file_buffer, config_file_size);
            } else {
                fprintf(stderr, "Error channel %u: File Read Failed\n", instance_idx + 1);
                return_error = -1;
            }
        } else {
            fprintf(stderr, "Error channel %u: Memory Allocation Failed\n", instance_idx + 1);
            return_error = -1;
        }

        free(config_file_buffer);
        fclose(input_pred_struct_file);
    } else {
        fprintf(stderr,
                "Error channel %u: Couldn't open Manual Prediction Structure File: %s\n",
                instance_idx + 1,
                PredStructPath);
        return_error = -1;
    }

    return return_error;
}
#endif

static Bool warn_legacy_token(const char *const token) {
    static struct warn_set {
        const char *old_token;
        const char *new_token;
    } warning_set[] = {
        {"-adaptive-quantization", ADAPTIVE_QP_ENABLE_NEW_TOKEN},
        {"-bit-depth", INPUT_DEPTH_TOKEN},
        {"-enc-mode", PRESET_TOKEN},
        {"-hdr", HDR_INPUT_NEW_TOKEN},
        {"-intra-period", KEYINT_TOKEN},
        {"-lad", LOOKAHEAD_NEW_TOKEN},
        {"-mfmv", MFMV_ENABLE_NEW_TOKEN},
        {"-qp-file", QP_FILE_NEW_TOKEN},
        {"-stat-report", STAT_REPORT_NEW_TOKEN},
        {NULL, NULL},
    };
    for (struct warn_set *tok = warning_set; tok->old_token; ++tok) {
        if (strcmp(token, tok->old_token))
            continue;
        fprintf(stderr,
                "[SVT-Error]: %s has been removed, use %s instead\n",
                tok->old_token,
                tok->new_token);
        return TRUE;
    }
    return FALSE;
}

static void free_config_strings(unsigned nch, char *config_strings[MAX_CHANNEL_NUMBER]) {
    for (unsigned i = 0; i < nch; ++i) free(config_strings[i]);
}

/******************************************
* Read Command Line
******************************************/
EbErrorType read_command_line(int32_t argc, char *const argv[], EncChannel *channels,
                              uint32_t num_channels) {
    EbErrorType return_error = EB_ErrorNone;
    char        config_string[COMMAND_LINE_MAX_SIZE]; // for one input options
    char       *config_strings[MAX_CHANNEL_NUMBER]; // for multiple input options
    const char *cmd_copy[MAX_NUM_TOKENS]; // keep track of extra tokens
    const char *arg_copy[MAX_NUM_TOKENS]; // keep track of extra arguments
    uint32_t    index = 0;
    int32_t     ret_y4m;

    for (index = 0; index < num_channels; ++index)
        config_strings[index] = (char *)malloc(sizeof(char) * COMMAND_LINE_MAX_SIZE);
    for (int i = 0; i < MAX_NUM_TOKENS; ++i) {
        cmd_copy[i] = NULL;
        arg_copy[i] = NULL;
    }

    // Copy tokens into a temp token buffer hosting all tokens that are passed through the command line
    for (int32_t token_index = 0; token_index < argc; ++token_index) {
        if (!is_negative_number(argv[token_index])) {
            if (argv[token_index][0] == '-' && argv[token_index][1] != '\0')
                cmd_copy[token_index] = argv[token_index];
            else if (token_index)
                arg_copy[token_index] = argv[token_index];
        }
    }

    // First handle --nch and --passes as a single argument options
    find_token_multiple_inputs(
        1, argc, argv, CHANNEL_NUMBER_TOKEN, config_strings, cmd_copy, arg_copy);
    find_token_multiple_inputs(1, argc, argv, PASSES_TOKEN, config_strings, cmd_copy, arg_copy);

    /***************************************************************************************************/
    /****************  Find configuration files tokens and call respective functions  ******************/
    /***************************************************************************************************/
    // Find the Config File Path in the command line
    if (find_token_multiple_inputs(
            num_channels, argc, argv, CONFIG_FILE_TOKEN, config_strings, cmd_copy, arg_copy)) {
        // Parse the config file
        for (index = 0; index < num_channels; ++index) {
            EncChannel *c   = channels + index;
            c->return_error = (EbErrorType)read_config_file(
                c->config, config_strings[index], index);
            return_error = (EbErrorType)(return_error & c->return_error);
        }
    } else if (find_token_multiple_inputs(num_channels,
                                          argc,
                                          argv,
                                          CONFIG_FILE_LONG_TOKEN,
                                          config_strings,
                                          cmd_copy,
                                          arg_copy)) {
        // Parse the config file
        for (index = 0; index < num_channels; ++index) {
            EncChannel *c   = channels + index;
            c->return_error = (EbErrorType)read_config_file(
                c->config, config_strings[index], index);
            return_error = (EbErrorType)(return_error & c->return_error);
        }
    } else {
        if (find_token(argc, argv, CONFIG_FILE_TOKEN, config_string) == 0) {
            fprintf(stderr, "Error: Config File Token Not Found\n");
            free_config_strings(num_channels, config_strings);
            return EB_ErrorBadParameter;
        }
        return_error = EB_ErrorNone;
    }

    /********************************************************************************************************/
    /***********   Find SINGLE_INPUT configuration parameter tokens and call respective functions  **********/
    /********************************************************************************************************/

    // Check tokens for invalid tokens
    {
        bool next_is_value = false;
        for (char *const *indx = argv + 1; *indx; ++indx) {
            // stop at --
            if (!strcmp(*indx, "--"))
                break;
            // skip the token if the previous token was an argument
            // assumes all of our tokens flip flop between being an argument and a value
            if (next_is_value) {
                next_is_value = false;
                continue;
            }
            // Check removed tokens
            if (warn_legacy_token(*indx)) {
                free_config_strings(num_channels, config_strings);
                return EB_ErrorBadParameter;
            }
            // exclude single letter tokens
            if ((*indx)[0] == '-' && (*indx)[1] != '-' && (*indx)[2] != '\0') {
                fprintf(stderr, "[SVT-Error]: single dash long tokens have been removed!\n");
                free_config_strings(num_channels, config_strings);
                return EB_ErrorBadParameter;
            }
            next_is_value = true;
        }
    }

    // Parse command line for tokens
    for (ConfigEntry *entry = config_entry; entry->token; ++entry) {
        if (entry->type != SINGLE_INPUT)
            continue;
        if (!find_token_multiple_inputs(
                num_channels, argc, argv, entry->token, config_strings, cmd_copy, arg_copy))
            continue;
        if (!strcmp(TOKEN_ERROR_MARKER, config_strings[0])) {
            free_config_strings(num_channels, config_strings);
            return EB_ErrorBadParameter;
        }
        // When a token is found mark it as found in the temp token buffer
        // Fill up the values corresponding to each channel
        for (uint32_t chan = 0; chan < num_channels; ++chan) {
            if (!strcmp(config_strings[chan], " "))
                break;
            // Mark the value as found in the temp argument buffer
            (entry->scf)(config_strings[chan], channels[chan].config);
        }
    }

    /***************************************************************************************************/
    /********************** Parse parameters from input file if in y4m format **************************/
    /********************** overriding config file and command line inputs    **************************/
    /***************************************************************************************************/

    for (index = 0; index < num_channels; ++index) {
        EncChannel *c = channels + index;
        if (c->config->y4m_input == TRUE) {
            ret_y4m = read_y4m_header(c->config);
            if (ret_y4m == EB_ErrorBadParameter) {
                fprintf(stderr, "Error found when reading the y4m file parameters.\n");
                free_config_strings(num_channels, config_strings);
                return EB_ErrorBadParameter;
            }
        }
    }
#if !REMOVE_MANUAL_PRED
    /***************************************************************************************************/
    /*******************************   Parse manual prediction structure  ******************************/
    /***************************************************************************************************/
    for (index = 0; index < num_channels; ++index) {
        EncChannel *c      = channels + index;
        EbConfig   *config = c->config;
        if (config->config.enable_manual_pred_struct == TRUE) {
            c->return_error = (EbErrorType)read_pred_struct_file(
                config, config->input_pred_struct_filename, index);
            return_error = (EbErrorType)(return_error & c->return_error);
        }
    }
#endif
    /***************************************************************************************************/
    /**************************************   Verify configuration parameters   ************************/
    /***************************************************************************************************/
    // Verify the config values
    if (return_error == 0) {
        return_error = EB_ErrorBadParameter;
        for (index = 0; index < num_channels; ++index) {
            EncChannel *c = channels + index;
            if (c->return_error == EB_ErrorNone) {
                EbConfig *config = c->config;
                c->return_error  = app_verify_config(config, index);
                // set inj_frame_rate to q16 format
                if (c->return_error == EB_ErrorNone && config->injector == 1)
                    config->injector_frame_rate <<= 16;

                // Assuming no errors, add padding to width and height
                if (c->return_error == EB_ErrorNone) {
                    config->input_padded_width  = config->config.source_width;
                    config->input_padded_height = config->config.source_height;
                }

                // Assuming no errors, set the frames to be encoded to the number of frames in the input yuv
                if (c->return_error == EB_ErrorNone && config->frames_to_be_encoded == 0)
                    config->frames_to_be_encoded = compute_frames_to_be_encoded(config);

                // For pipe input it is fine if we have -1 here (we will update on end of stream)
                if (config->frames_to_be_encoded == -1 && config->input_file != stdin &&
                    !config->input_file_is_fifo) {
                    fprintf(config->error_log_file,
                            "Error instance %u: Input yuv does not contain enough frames \n",
                            index + 1);
                    c->return_error = EB_ErrorBadParameter;
                }

                // Force the injector latency mode, and injector frame rate when speed control is on
                if (c->return_error == EB_ErrorNone && config->speed_control_flag == 1)
                    config->injector = 1;
            }
            return_error = (EbErrorType)(return_error & c->return_error);
        }
    }

    bool has_cmd_notread = false;
    for (int i = 0; i < argc; ++i) {
        if (cmd_copy[i] && strcmp(TOKEN_READ_MARKER, cmd_copy[i])) {
            if (!has_cmd_notread)
                fprintf(stderr, "Unprocessed tokens: ");
            fprintf(stderr, "%s ", argv[i]);
            has_cmd_notread = true;
        }
    }
    if (has_cmd_notread) {
        fprintf(stderr, "\n\n");
        return_error = EB_ErrorBadParameter;
    }
    bool has_arg_notread = false;
    bool maybe_token     = false;
    for (int i = 0; i < argc; ++i) {
        if (arg_copy[i] && strcmp(TOKEN_READ_MARKER, arg_copy[i])) {
            if (!has_arg_notread)
                fprintf(stderr, "Unprocessed arguments: ");
            fprintf(stderr, "%s ", argv[i]);
            maybe_token |= !!strchr(arg_copy[i], '-');
            has_arg_notread = true;
        }
    }
    if (maybe_token) {
        fprintf(stderr, "\nMaybe missing spacing between tokens");
    }
    if (has_arg_notread) {
        fprintf(stderr, "\n\n");
        return_error = EB_ErrorBadParameter;
    }

    for (index = 0; index < num_channels; ++index) free(config_strings[index]);

    return return_error;
}

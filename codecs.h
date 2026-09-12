#pragma once


struct codec_specs {
    const char *id;
    const char *ext;
};


/**
 * source for IDs: https://www.matroska.org/technical/codec_specs.html
 * File name extensions are taken from different sources or are guesses/hints.
 */
static struct codec_specs mkv_codec_list[] =
{
    /* video */
    { "V_AV1",                "av1"    },
    { "V_AVS2",               "avs2"   },
    { "V_AVS3",               "avs3"   },
    { "V_DIRAC",              "drc"    },
    { "V_CAVS",               "cavs"   },
    { "V_FFV1",               "ffv1"   },
    { "V_JPEG2000",           "mjp2"   },
    { "V_MJPEG",              "mjp"    },
    { "V_MPEGH/ISO/HEVC",     "hevc"   },
    { "V_MPEGI/ISO/VVC",      "vvc"    },
    { "V_MPEG1",              "m1v"    },
    { "V_MPEG2",              "m2v"    },
    { "V_MPEG4/ISO/AVC",      "avc"    },
    { "V_MPEG4/ISO/AP",       "m4v"    },
    { "V_MPEG4/ISO/ASP",      "m4v"    },
    { "V_MPEG4/ISO/SP",       "m4v"    },
    { "V_MPEG4/MS/V3",        "m4v"    },
    { "V_MS/VFW/FOURCC",      "vcm"    }, /* Microsoft Video Codec Manager (VCM) */
    { "V_QUICKTIME",          "mov"    },
    { "V_PRORES",             "mov"    },
    { "V_REAL/RV10",          "rv"     },
    { "V_REAL/RV20",          "rv"     },
    { "V_REAL/RV30",          "rv"     },
    { "V_REAL/RV40",          "rv"     },
    { "V_THEORA",             "ogv"    },
    { "V_UNCOMPRESSED",       "raw"    },
    { "V_VC1",                "vc1"    },
    { "V_VP8",                "vp8"    },
    { "V_VP9",                "vp9"    },

    /* audio */
    { "A_AAC",                "aac"    },
    { "A_AAC/MPEG2/LC",       "aac"    },
    { "A_AAC/MPEG2/LC/SBR",   "aac"    },
    { "A_AAC/MPEG2/MAIN",     "aac"    },
    { "A_AAC/MPEG2/SSR",      "aac"    },
    { "A_AAC/MPEG4/LC",       "aac"    },
    { "A_AAC/MPEG4/LC/SBR",   "aac"    },
    { "A_AAC/MPEG4/LTP",      "aac"    },
    { "A_AAC/MPEG4/MAIN",     "aac"    },
    { "A_AAC/MPEG4/SSR",      "aac"    },
    { "A_AC3",                "ac3"    },
    { "A_AC3/BSID9",          "ac3"    },
    { "A_AC3/BSID10",         "ac3"    },
    { "A_ALAC",               "alac"   },
    { "A_ATRAC/AT1",          "aa3"    },
    { "A_DTS",                "dts"    },
    { "A_DTS/EXPRESS",        "dts"    },
    { "A_DTS/LOSSLESS",       "dts"    },
    { "A_EAC3",               "eac3"   },
    { "A_FLAC",               "flac"   },
    { "A_MLP",                "mlp"    },
    { "A_MPEG/L1",            "m1a"    },
    { "A_MPEG/L2",            "m2a"    },
    { "A_MPEG/L3",            "mp3"    },
    { "A_MS/ACM",             "acm"    },
    { "A_REAL/14_4",          "ra"     },
    { "A_REAL/28_8",          "ra"     },
    { "A_REAL/ATRC",          "ra"     },
    { "A_REAL/COOK",          "ra"     },
    { "A_REAL/RALF",          "ra"     },
    { "A_REAL/SIPR",          "ra"     },
    { "A_OPUS",               "opus"   },
    { "A_PCM/FLOAT/IEEE",     "pcm"    },
    { "A_PCM/INT/BIG",        "pcm"    },
    { "A_PCM/INT/LIT",        "pcm"    },
    { "A_QUICKTIME",          "qta"    },
    { "A_QUICKTIME/QDMC",     "qdmc"   },
    { "A_QUICKTIME/QDM2",     "qdm2"   },
    { "A_TRUEHD",             "thd"    },
    { "A_TTA1",               "tta"    },
    { "A_VORBIS",             "ogg"    },
    { "A_WAVPACK4",           "wv"     },

    /* subtitles */
    { "S_ARIBSUB",            "es"     }, /* Elementary Stream */
    { "S_DVBSUB",             "dvbsub" },
    { "S_HDMV/PGS",           "pgs"    },
    { "S_HDMV/TEXTST",        "textst" },
    { "S_KATE",               "ogx"    }, /* https://wiki.xiph.org/MIME_Types_and_File_Extensions */
    { "S_IMAGE/BMP",          "bmp"    },
    { "S_TEXT/ASS",           "ass"    },
    { "S_TEXT/ASCII",         "txt"    },
    { "S_TEXT/SSA",           "ssa"    },
    { "S_TEXT/USF",           "usf"    },
    { "S_TEXT/UTF8",          "srt"    }, /* SubRip Text */
    { "S_TEXT/WEBVTT",        "vtt"    },
    { "S_VOBSUB",             "sub"    }, /* .idx/.sub */

    /* buttons */
    { "B_VOBBTN",             "vobbtn" }
};


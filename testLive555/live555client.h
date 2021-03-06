/******************************************************************************************************
 * class Live555Client using live555 library to create a client for RTSP, base from code of VLC       *
 * www.videolan.org                                                                                   *
 * Author: HungNV (hung.viet.nguyen.hp at gmail dot com)                                              *
 * Date  : 2016-05-17 - 15:00                                                                         *
 *                                                                                                    *
 * Main flow using live555 as follow                                                                  *
 * We have to handle two things:                                                                      *
 *                                                                                                    *
 * the first is all signals of RTSP server via taskInterruptRTSP                                      *
 *   each time we got reply from RTSP server, we have to stop event loop via flag event_rtsp,         *
 *   we enter the loop after send any command to RTSP server.                                         *
 * the second is data we got from RTSP server via taskInterruptData                                   *
 *   Each time we got data from RTSP server, we have to stop event loop via flag event_data,          *
 *   for consecutive receiving data, we put into demux thread that we call getNextFrame               *
 *   of all media tracks that we opened. Each track is representative by one MediaSubsession variable.*
 *   After call getNextFrame, we enter the loop for waiting data, controlled by event_data flag,      *
 *   Got data, set event_flag to stop waiting data loop, and next we call getNextFrame and enter      *
 *   waiting data loop ... and so on.                                                                 *
 *                                                                                                    *
 *                                                                                                    *
 * How to use class:                                                                                  *
 *   First is create a descendant class from Live555Client and override onData method to handle data, *
 *   the codec of data can be retrieved by track->getFormat().i_codec.                                *
 *   Set user/password if need via setUser method.                                                    *
 *   You have to set the RTP port range via setRTPPortBegin method. The value is passed through       *
 *   must be even. and then call open with RTSP url is parameter.                                     *
 *   After that, you call getRTPPortNoUse to retrieve RTP port available for other sessions           *
 *   Call play() method for start demuxing thread. When everything is done,                           *
 *   call stop() method to free all resources that we allocated                                       *
 *   The result of play() method:                                                                     *
 *       0 : OK                                                                                       *
 *      -1 : Memory error, happends when we allocate memory failed                                    *
 *     > 0 : Code result from RTSP server                                                             *
 *                                                                                                    *
 *                                                                                                    *
 * Here is the brief note about flow of using live555 library and how to use the class.               *
 * If you have any question, feel free contact me at hung.viet.nguyen.hp at gmail dot com             *
 ******************************************************************************************************
 * This program is free software; you can redistribute it and/or modify it                            *
 * under the terms of the GNU Lesser General Public License as published by                           *
 * the Free Software Foundation; either version 2.1 of the License, or                                *
 * (at your option) any later version.                                                                *
 *                                                                                                    *
 * This program is distributed in the hope that it will be useful,                                    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                                     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                       *
 * GNU Lesser General Public License for more details.                                                *
 *                                                                                                    *
 * You should have received a copy of the GNU Lesser General Public License                           *
 * along with this program; if not, write to the Free Software Foundation,                            *
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.                                  *
 ******************************************************************************************************/

#ifndef LIVE555_CLIENT_H__
#define LIVE555_CLIENT_H__

#include <stdint.h>
#include <string>
#include <thread>
#include <vector>

// here are taken from vlc

enum es_format_category_e
{
    UNKNOWN_ES = 0x00,
    VIDEO_ES,
    AUDIO_ES,
    SPU_ES,
    NAV_ES,
};

#ifdef WORDS_BIGENDIAN
#   define VLC_FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
           | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#   define VLC_TWOCC( a, b ) \
        ( (uint16_t)(b) | ( (uint16_t)(a) << 8 ) )

#else
#   define VLC_FOURCC( a, b, c, d ) \
        ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
           | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#   define VLC_TWOCC( a, b ) \
        ( (uint16_t)(a) | ( (uint16_t)(b) << 8 ) )

#endif
/* Video codec */
#define VLC_CODEC_MPGV            VLC_FOURCC('m','p','g','v')
#define VLC_CODEC_MP4V            VLC_FOURCC('m','p','4','v')
#define VLC_CODEC_DIV1            VLC_FOURCC('D','I','V','1')
#define VLC_CODEC_DIV2            VLC_FOURCC('D','I','V','2')
#define VLC_CODEC_DIV3            VLC_FOURCC('D','I','V','3')
#define VLC_CODEC_SVQ1            VLC_FOURCC('S','V','Q','1')
#define VLC_CODEC_SVQ3            VLC_FOURCC('S','V','Q','3')
#define VLC_CODEC_H264            VLC_FOURCC('h','2','6','4')
#define VLC_CODEC_H263            VLC_FOURCC('h','2','6','3')
#define VLC_CODEC_H263I           VLC_FOURCC('I','2','6','3')
#define VLC_CODEC_H263P           VLC_FOURCC('I','L','V','R')
#define VLC_CODEC_FLV1            VLC_FOURCC('F','L','V','1')
#define VLC_CODEC_H261            VLC_FOURCC('h','2','6','1')
#define VLC_CODEC_MJPG            VLC_FOURCC('M','J','P','G')
#define VLC_CODEC_MJPGB           VLC_FOURCC('m','j','p','b')
#define VLC_CODEC_LJPG            VLC_FOURCC('L','J','P','G')
#define VLC_CODEC_WMV1            VLC_FOURCC('W','M','V','1')
#define VLC_CODEC_WMV2            VLC_FOURCC('W','M','V','2')
#define VLC_CODEC_WMV3            VLC_FOURCC('W','M','V','3')
#define VLC_CODEC_WMVA            VLC_FOURCC('W','M','V','A')
#define VLC_CODEC_WMVP            VLC_FOURCC('W','M','V','P')
#define VLC_CODEC_WMVP2           VLC_FOURCC('W','V','P','2')
#define VLC_CODEC_VC1             VLC_FOURCC('V','C','-','1')
#define VLC_CODEC_THEORA          VLC_FOURCC('t','h','e','o')
#define VLC_CODEC_TARKIN          VLC_FOURCC('t','a','r','k')
#define VLC_CODEC_DIRAC           VLC_FOURCC('d','r','a','c')
#define VLC_CODEC_CAVS            VLC_FOURCC('C','A','V','S')
#define VLC_CODEC_NUV             VLC_FOURCC('N','J','P','G')
#define VLC_CODEC_RV10            VLC_FOURCC('R','V','1','0')
#define VLC_CODEC_RV13            VLC_FOURCC('R','V','1','3')
#define VLC_CODEC_RV20            VLC_FOURCC('R','V','2','0')
#define VLC_CODEC_RV30            VLC_FOURCC('R','V','3','0')
#define VLC_CODEC_RV40            VLC_FOURCC('R','V','4','0')
#define VLC_CODEC_VP3             VLC_FOURCC('V','P','3',' ')
#define VLC_CODEC_VP5             VLC_FOURCC('V','P','5',' ')
#define VLC_CODEC_VP6             VLC_FOURCC('V','P','6','2')
#define VLC_CODEC_VP6F            VLC_FOURCC('V','P','6','F')
#define VLC_CODEC_VP6A            VLC_FOURCC('V','P','6','A')
#define VLC_CODEC_MSVIDEO1        VLC_FOURCC('M','S','V','C')
#define VLC_CODEC_FLIC            VLC_FOURCC('F','L','I','C')
#define VLC_CODEC_SP5X            VLC_FOURCC('S','P','5','X')
#define VLC_CODEC_DV              VLC_FOURCC('d','v',' ',' ')
#define VLC_CODEC_MSRLE           VLC_FOURCC('m','r','l','e')
#define VLC_CODEC_HUFFYUV         VLC_FOURCC('H','F','Y','U')
#define VLC_CODEC_FFVHUFF         VLC_FOURCC('F','F','V','H')
#define VLC_CODEC_ASV1            VLC_FOURCC('A','S','V','1')
#define VLC_CODEC_ASV2            VLC_FOURCC('A','S','V','2')
#define VLC_CODEC_FFV1            VLC_FOURCC('F','F','V','1')
#define VLC_CODEC_VCR1            VLC_FOURCC('V','C','R','1')
#define VLC_CODEC_CLJR            VLC_FOURCC('C','L','J','R')
#define VLC_CODEC_RPZA            VLC_FOURCC('r','p','z','a')
#define VLC_CODEC_SMC             VLC_FOURCC('s','m','c',' ')
#define VLC_CODEC_CINEPAK         VLC_FOURCC('C','V','I','D')
#define VLC_CODEC_TSCC            VLC_FOURCC('T','S','C','C')
#define VLC_CODEC_CSCD            VLC_FOURCC('C','S','C','D')
#define VLC_CODEC_ZMBV            VLC_FOURCC('Z','M','B','V')
#define VLC_CODEC_VMNC            VLC_FOURCC('V','M','n','c')
#define VLC_CODEC_FRAPS           VLC_FOURCC('F','P','S','1')
#define VLC_CODEC_TRUEMOTION1     VLC_FOURCC('D','U','C','K')
#define VLC_CODEC_TRUEMOTION2     VLC_FOURCC('T','M','2','0')
#define VLC_CODEC_QTRLE           VLC_FOURCC('r','l','e',' ')
#define VLC_CODEC_QDRAW           VLC_FOURCC('q','d','r','w')
#define VLC_CODEC_QPEG            VLC_FOURCC('Q','P','E','G')
#define VLC_CODEC_ULTI            VLC_FOURCC('U','L','T','I')
#define VLC_CODEC_VIXL            VLC_FOURCC('V','I','X','L')
#define VLC_CODEC_LOCO            VLC_FOURCC('L','O','C','O')
#define VLC_CODEC_WNV1            VLC_FOURCC('W','N','V','1')
#define VLC_CODEC_AASC            VLC_FOURCC('A','A','S','C')
#define VLC_CODEC_INDEO2          VLC_FOURCC('I','V','2','0')
#define VLC_CODEC_INDEO3          VLC_FOURCC('I','V','3','1')
#define VLC_CODEC_INDEO4          VLC_FOURCC('I','V','4','1')
#define VLC_CODEC_INDEO5          VLC_FOURCC('I','V','5','0')
#define VLC_CODEC_FLASHSV         VLC_FOURCC('F','S','V','1')
#define VLC_CODEC_KMVC            VLC_FOURCC('K','M','V','C')
#define VLC_CODEC_SMACKVIDEO      VLC_FOURCC('S','M','K','2')
#define VLC_CODEC_DNXHD           VLC_FOURCC('A','V','d','n')
#define VLC_CODEC_8BPS            VLC_FOURCC('8','B','P','S')
#define VLC_CODEC_MIMIC           VLC_FOURCC('M','L','2','O')
#define VLC_CODEC_INTERPLAY       VLC_FOURCC('i','m','v','e')
#define VLC_CODEC_IDCIN           VLC_FOURCC('I','D','C','I')
#define VLC_CODEC_4XM             VLC_FOURCC('4','X','M','V')
#define VLC_CODEC_ROQ             VLC_FOURCC('R','o','Q','v')
#define VLC_CODEC_MDEC            VLC_FOURCC('M','D','E','C')
#define VLC_CODEC_VMDVIDEO        VLC_FOURCC('V','M','D','V')
#define VLC_CODEC_CDG             VLC_FOURCC('C','D','G',' ')
#define VLC_CODEC_FRWU            VLC_FOURCC('F','R','W','U')
#define VLC_CODEC_AMV             VLC_FOURCC('A','M','V',' ')
#define VLC_CODEC_VP7             VLC_FOURCC('V','P','7','0')
#define VLC_CODEC_VP8             VLC_FOURCC('V','P','8','0')
#define VLC_CODEC_VP9             VLC_FOURCC('V','P','9','0')
#define VLC_CODEC_JPEG2000        VLC_FOURCC('J','P','2','K')
#define VLC_CODEC_LAGARITH        VLC_FOURCC('L','A','G','S')
#define VLC_CODEC_FLASHSV2        VLC_FOURCC('F','S','V','2')
#define VLC_CODEC_PRORES          VLC_FOURCC('a','p','c','n')
#define VLC_CODEC_MXPEG           VLC_FOURCC('M','X','P','G')
#define VLC_CODEC_CDXL            VLC_FOURCC('C','D','X','L')
#define VLC_CODEC_BMVVIDEO        VLC_FOURCC('B','M','V','V')
#define VLC_CODEC_UTVIDEO         VLC_FOURCC('U','L','R','A')
#define VLC_CODEC_VBLE            VLC_FOURCC('V','B','L','E')
#define VLC_CODEC_DXTORY          VLC_FOURCC('x','t','o','r')
#define VLC_CODEC_MSS1            VLC_FOURCC('M','S','S','1')
#define VLC_CODEC_MSS2            VLC_FOURCC('M','S','S','2')
#define VLC_CODEC_MSA1            VLC_FOURCC('M','S','A','1')
#define VLC_CODEC_TSC2            VLC_FOURCC('T','S','C','2')
#define VLC_CODEC_MTS2            VLC_FOURCC('M','T','S','2')
#define VLC_CODEC_HEVC            VLC_FOURCC('h','e','v','c')
#define VLC_CODEC_ICOD            VLC_FOURCC('i','c','o','d')
#define VLC_CODEC_G2M2            VLC_FOURCC('G','2','M','2')
#define VLC_CODEC_G2M3            VLC_FOURCC('G','2','M','3')
#define VLC_CODEC_G2M4            VLC_FOURCC('G','2','M','4')
#define VLC_CODEC_BINKVIDEO       VLC_FOURCC('B','I','K','f')
#define VLC_CODEC_BINKAUDIO_DCT   VLC_FOURCC('B','A','U','1')
#define VLC_CODEC_BINKAUDIO_RDFT  VLC_FOURCC('B','A','U','2')
#define VLC_CODEC_XAN_WC4         VLC_FOURCC('X','x','a','n')
#define VLC_CODEC_LCL_MSZH        VLC_FOURCC('M','S','Z','H')
#define VLC_CODEC_LCL_ZLIB        VLC_FOURCC('Z','L','I','B')
#define VLC_CODEC_THP             VLC_FOURCC('T','H','P','0')
#define VLC_CODEC_ESCAPE124       VLC_FOURCC('E','1','2','4')
#define VLC_CODEC_KGV1            VLC_FOURCC('K','G','V','1')
#define VLC_CODEC_CLLC            VLC_FOURCC('C','L','L','C')
#define VLC_CODEC_AURA            VLC_FOURCC('A','U','R','A')
#define VLC_CODEC_FIC             VLC_FOURCC('F','I','C','V')

/* Planar YUV 4:1:0 Y:V:U */
#define VLC_CODEC_YV9             VLC_FOURCC('Y','V','U','9')
/* Planar YUV 4:2:0 Y:V:U */
#define VLC_CODEC_YV12            VLC_FOURCC('Y','V','1','2')
/* Planar YUV 4:1:0 Y:U:V */
#define VLC_CODEC_I410            VLC_FOURCC('I','4','1','0')
/* Planar YUV 4:1:1 Y:U:V */
#define VLC_CODEC_I411            VLC_FOURCC('I','4','1','1')
/* Planar YUV 4:2:0 Y:U:V 8-bit */
#define VLC_CODEC_I420            VLC_FOURCC('I','4','2','0')
/* Planar YUV 4:2:0 Y:U:V  9-bit stored on 16 bits */
#define VLC_CODEC_I420_9L         VLC_FOURCC('I','0','9','L')
#define VLC_CODEC_I420_9B         VLC_FOURCC('I','0','9','B')
/* Planar YUV 4:2:0 Y:U:V 10-bit stored on 16 bits */
#define VLC_CODEC_I420_10L        VLC_FOURCC('I','0','A','L')
#define VLC_CODEC_I420_10B        VLC_FOURCC('I','0','A','B')
/* Planar YUV 4:2:2 Y:U:V 8-bit */
#define VLC_CODEC_I422            VLC_FOURCC('I','4','2','2')
/* Planar YUV 4:2:2 Y:U:V  9-bit stored on 16 bits */
#define VLC_CODEC_I422_9L         VLC_FOURCC('I','2','9','L')
#define VLC_CODEC_I422_9B         VLC_FOURCC('I','2','9','B')
/* Planar YUV 4:2:2 Y:U:V 10-bit stored on 16 bits */
#define VLC_CODEC_I422_10L        VLC_FOURCC('I','2','A','L')
#define VLC_CODEC_I422_10B        VLC_FOURCC('I','2','A','B')
/* Planar YUV 4:4:0 Y:U:V */
#define VLC_CODEC_I440            VLC_FOURCC('I','4','4','0')
/* Planar YUV 4:4:4 Y:U:V 8-bit */
#define VLC_CODEC_I444            VLC_FOURCC('I','4','4','4')
/* Planar YUV 4:4:4 Y:U:V  9-bit stored on 16 bits */
#define VLC_CODEC_I444_9L         VLC_FOURCC('I','4','9','L')
#define VLC_CODEC_I444_9B         VLC_FOURCC('I','4','9','B')
/* Planar YUV 4:4:4 Y:U:V 10-bit stored on 16 bits */
#define VLC_CODEC_I444_10L        VLC_FOURCC('I','4','A','L')
#define VLC_CODEC_I444_10B        VLC_FOURCC('I','4','A','B')
/* Planar YUV 4:4:4 Y:U:V 16-bit */
#define VLC_CODEC_I444_16L        VLC_FOURCC('I','4','F','L')
#define VLC_CODEC_I444_16B        VLC_FOURCC('I','4','F','B')
/* Planar YUV 4:2:0 Y:U:V full scale */
#define VLC_CODEC_J420            VLC_FOURCC('J','4','2','0')
/* Planar YUV 4:2:2 Y:U:V full scale */
#define VLC_CODEC_J422            VLC_FOURCC('J','4','2','2')
/* Planar YUV 4:4:0 Y:U:V full scale */
#define VLC_CODEC_J440            VLC_FOURCC('J','4','4','0')
/* Planar YUV 4:4:4 Y:U:V full scale */
#define VLC_CODEC_J444            VLC_FOURCC('J','4','4','4')
/* Palettized YUV with palette element Y:U:V:A */
#define VLC_CODEC_YUVP            VLC_FOURCC('Y','U','V','P')
/* Planar YUV 4:4:4 Y:U:V:A */
#define VLC_CODEC_YUVA            VLC_FOURCC('Y','U','V','A')
/* Planar YUV 4:2:2 Y:U:V:A */
#define VLC_CODEC_YUV422A         VLC_FOURCC('I','4','2','A')
/* Planar YUV 4:2:0 Y:U:V:A */
#define VLC_CODEC_YUV420A         VLC_FOURCC('I','4','0','A')

/* Palettized RGB with palette element R:G:B */
#define VLC_CODEC_RGBP            VLC_FOURCC('R','G','B','P')
/* 8 bits RGB */
#define VLC_CODEC_RGB8            VLC_FOURCC('R','G','B','8')
/* 12 bits RGB padded to 16 bits */
#define VLC_CODEC_RGB12           VLC_FOURCC('R','V','1','2')
/* 15 bits RGB padded to 16 bits */
#define VLC_CODEC_RGB15           VLC_FOURCC('R','V','1','5')
/* 16 bits RGB */
#define VLC_CODEC_RGB16           VLC_FOURCC('R','V','1','6')
/* 24 bits RGB */
#define VLC_CODEC_RGB24           VLC_FOURCC('R','V','2','4')
/* 24 bits RGB padded to 32 bits */
#define VLC_CODEC_RGB32           VLC_FOURCC('R','V','3','2')
/* 32 bits RGBA */
#define VLC_CODEC_RGBA            VLC_FOURCC('R','G','B','A')
/* 32 bits ARGB */
#define VLC_CODEC_ARGB            VLC_FOURCC('A','R','G','B')
/* 32 bits BGRA */
#define VLC_CODEC_BGRA            VLC_FOURCC('B','G','R','A')

/* Planar GBR 4:4:4 8 bits */
#define VLC_CODEC_GBR_PLANAR      VLC_FOURCC('G','B','R','8')
#define VLC_CODEC_GBR_PLANAR_9B   VLC_FOURCC('G','B','9','B')
#define VLC_CODEC_GBR_PLANAR_9L   VLC_FOURCC('G','B','9','L')
#define VLC_CODEC_GBR_PLANAR_10B  VLC_FOURCC('G','B','A','B')
#define VLC_CODEC_GBR_PLANAR_10L  VLC_FOURCC('G','B','A','L')
#define VLC_CODEC_GBR_PLANAR_16L  VLC_FOURCC('G','B','F','L')
#define VLC_CODEC_GBR_PLANAR_16B  VLC_FOURCC('G','B','F','B')

/* 8 bits grey */
#define VLC_CODEC_GREY            VLC_FOURCC('G','R','E','Y')
/* Packed YUV 4:2:2, U:Y:V:Y */
#define VLC_CODEC_UYVY            VLC_FOURCC('U','Y','V','Y')
/* Packed YUV 4:2:2, V:Y:U:Y */
#define VLC_CODEC_VYUY            VLC_FOURCC('V','Y','U','Y')
/* Packed YUV 4:2:2, Y:U:Y:V */
#define VLC_CODEC_YUYV            VLC_FOURCC('Y','U','Y','2')
/* Packed YUV 4:2:2, Y:V:Y:U */
#define VLC_CODEC_YVYU            VLC_FOURCC('Y','V','Y','U')
/* Packed YUV 2:1:1, Y:U:Y:V */
#define VLC_CODEC_Y211            VLC_FOURCC('Y','2','1','1')
/* Packed YUV 4:2:2, U:Y:V:Y, reverted */
#define VLC_CODEC_CYUV            VLC_FOURCC('c','y','u','v')
/* 10-bit 4:2:2 Component YCbCr */
#define VLC_CODEC_V210            VLC_FOURCC('v','2','1','0')
/* 2 planes Y/UV 4:2:0 */
#define VLC_CODEC_NV12            VLC_FOURCC('N','V','1','2')
/* 2 planes Y/VU 4:2:0 */
#define VLC_CODEC_NV21            VLC_FOURCC('N','V','2','1')
/* 2 planes Y/UV 4:2:2 */
#define VLC_CODEC_NV16            VLC_FOURCC('N','V','1','6')
/* 2 planes Y/VU 4:2:2 */
#define VLC_CODEC_NV61            VLC_FOURCC('N','V','6','1')
/* 2 planes Y/UV 4:4:4 */
#define VLC_CODEC_NV24            VLC_FOURCC('N','V','2','4')
/* 2 planes Y/VU 4:4:4 */
#define VLC_CODEC_NV42            VLC_FOURCC('N','V','4','2')

/* VDPAU video surface YCbCr 4:2:0 */
#define VLC_CODEC_VDPAU_VIDEO_420 VLC_FOURCC('V','D','V','0')
/* VDPAU video surface YCbCr 4:2:2 */
#define VLC_CODEC_VDPAU_VIDEO_422 VLC_FOURCC('V','D','V','2')
/* VDPAU video surface YCbCr 4:4:4 */
#define VLC_CODEC_VDPAU_VIDEO_444 VLC_FOURCC('V','D','V','4')
/* VDPAU output surface RGBA */
#define VLC_CODEC_VDPAU_OUTPUT    VLC_FOURCC('V','D','O','R')

/* MediaCodec/IOMX opaque buffer type */
#define VLC_CODEC_ANDROID_OPAQUE  VLC_FOURCC('A','N','O','P')

/* Broadcom MMAL opaque buffer type */
#define VLC_CODEC_MMAL_OPAQUE     VLC_FOURCC('M','M','A','L')

/* Image codec (video) */
#define VLC_CODEC_PNG             VLC_FOURCC('p','n','g',' ')
#define VLC_CODEC_PPM             VLC_FOURCC('p','p','m',' ')
#define VLC_CODEC_PGM             VLC_FOURCC('p','g','m',' ')
#define VLC_CODEC_PGMYUV          VLC_FOURCC('p','g','m','y')
#define VLC_CODEC_PAM             VLC_FOURCC('p','a','m',' ')
#define VLC_CODEC_JPEG            VLC_FOURCC('j','p','e','g')
#define VLC_CODEC_JPEGLS          VLC_FOURCC('M','J','L','S')
#define VLC_CODEC_BMP             VLC_FOURCC('b','m','p',' ')
#define VLC_CODEC_TIFF            VLC_FOURCC('t','i','f','f')
#define VLC_CODEC_GIF             VLC_FOURCC('g','i','f',' ')
#define VLC_CODEC_TARGA           VLC_FOURCC('t','g','a',' ')
#define VLC_CODEC_SVG             VLC_FOURCC('s','v','g',' ')
#define VLC_CODEC_SGI             VLC_FOURCC('s','g','i',' ')
#define VLC_CODEC_PNM             VLC_FOURCC('p','n','m',' ')
#define VLC_CODEC_PCX             VLC_FOURCC('p','c','x',' ')
#define VLC_CODEC_XWD             VLC_FOURCC('X','W','D',' ')
#define VLC_CODEC_TXD             VLC_FOURCC('T','X','D',' ')


/* Audio codec */
#define VLC_CODEC_MPGA                       VLC_FOURCC('m','p','g','a')
#define VLC_CODEC_MP4A                       VLC_FOURCC('m','p','4','a')
#define VLC_CODEC_ALS                        VLC_FOURCC('a','l','s',' ')
#define VLC_CODEC_A52                        VLC_FOURCC('a','5','2',' ')
#define VLC_CODEC_EAC3                       VLC_FOURCC('e','a','c','3')
#define VLC_CODEC_DTS                        VLC_FOURCC('d','t','s',' ')
#define VLC_CODEC_WMA1                       VLC_FOURCC('W','M','A','1')
#define VLC_CODEC_WMA2                       VLC_FOURCC('W','M','A','2')
#define VLC_CODEC_WMAP                       VLC_FOURCC('W','M','A','P')
#define VLC_CODEC_WMAL                       VLC_FOURCC('W','M','A','L')
#define VLC_CODEC_WMAS                       VLC_FOURCC('W','M','A','S')
#define VLC_CODEC_FLAC                       VLC_FOURCC('f','l','a','c')
#define VLC_CODEC_MLP                        VLC_FOURCC('m','l','p',' ')
#define VLC_CODEC_TRUEHD                     VLC_FOURCC('t','r','h','d')
#define VLC_CODEC_DVAUDIO                    VLC_FOURCC('d','v','a','u')
#define VLC_CODEC_SPEEX                      VLC_FOURCC('s','p','x',' ')
#define VLC_CODEC_OPUS                       VLC_FOURCC('O','p','u','s')
#define VLC_CODEC_VORBIS                     VLC_FOURCC('v','o','r','b')
#define VLC_CODEC_MACE3                      VLC_FOURCC('M','A','C','3')
#define VLC_CODEC_MACE6                      VLC_FOURCC('M','A','C','6')
#define VLC_CODEC_MUSEPACK7                  VLC_FOURCC('M','P','C',' ')
#define VLC_CODEC_MUSEPACK8                  VLC_FOURCC('M','P','C','K')
#define VLC_CODEC_RA_144                     VLC_FOURCC('1','4','_','4')
#define VLC_CODEC_RA_288                     VLC_FOURCC('2','8','_','8')
#define VLC_CODEC_INTERPLAY_DPCM             VLC_FOURCC('i','d','p','c')
#define VLC_CODEC_ROQ_DPCM                   VLC_FOURCC('R','o','Q','a')
#define VLC_CODEC_DSICINAUDIO                VLC_FOURCC('D','C','I','A')
#define VLC_CODEC_ADPCM_4XM                  VLC_FOURCC('4','x','m','a')
#define VLC_CODEC_ADPCM_EA                   VLC_FOURCC('A','D','E','A')
#define VLC_CODEC_ADPCM_XA                   VLC_FOURCC('x','a',' ',' ')
#define VLC_CODEC_ADPCM_ADX                  VLC_FOURCC('a','d','x',' ')
#define VLC_CODEC_ADPCM_IMA_WS               VLC_FOURCC('A','I','W','S')
#define VLC_CODEC_ADPCM_G722                 VLC_FOURCC('g','7','2','2')
#define VLC_CODEC_ADPCM_G726                 VLC_FOURCC('g','7','2','6')
#define VLC_CODEC_ADPCM_SWF                  VLC_FOURCC('S','W','F','a')
#define VLC_CODEC_ADPCM_MS                   VLC_FOURCC('m','s',0x00,0x02)
#define VLC_CODEC_ADPCM_IMA_WAV              VLC_FOURCC('m','s',0x00,0x11)
#define VLC_CODEC_ADPCM_IMA_AMV              VLC_FOURCC('i','m','a','v')
#define VLC_CODEC_ADPCM_IMA_QT               VLC_FOURCC('i','m','a','4')
#define VLC_CODEC_ADPCM_YAMAHA               VLC_FOURCC('m','s',0x00,0x20)
#define VLC_CODEC_ADPCM_DK3                  VLC_FOURCC('m','s',0x00,0x62)
#define VLC_CODEC_ADPCM_DK4                  VLC_FOURCC('m','s',0x00,0x61)
#define VLC_CODEC_ADPCM_THP                  VLC_FOURCC('T','H','P','A')
#define VLC_CODEC_G723_1                     VLC_FOURCC('g','7','2', 0x31)
#define VLC_CODEC_G729                       VLC_FOURCC('g','7','2','9')
#define VLC_CODEC_VMDAUDIO                   VLC_FOURCC('v','m','d','a')
#define VLC_CODEC_AMR_NB                     VLC_FOURCC('s','a','m','r')
#define VLC_CODEC_AMR_WB                     VLC_FOURCC('s','a','w','b')
#define VLC_CODEC_ALAC                       VLC_FOURCC('a','l','a','c')
#define VLC_CODEC_QDM2                       VLC_FOURCC('Q','D','M','2')
#define VLC_CODEC_COOK                       VLC_FOURCC('c','o','o','k')
#define VLC_CODEC_SIPR                       VLC_FOURCC('s','i','p','r')
#define VLC_CODEC_TTA                        VLC_FOURCC('T','T','A','1')
#define VLC_CODEC_SHORTEN                    VLC_FOURCC('s','h','n',' ')
#define VLC_CODEC_WAVPACK                    VLC_FOURCC('W','V','P','K')
#define VLC_CODEC_GSM                        VLC_FOURCC('g','s','m',' ')
#define VLC_CODEC_GSM_MS                     VLC_FOURCC('a','g','s','m')
#define VLC_CODEC_ATRAC1                     VLC_FOURCC('a','t','r','1')
#define VLC_CODEC_ATRAC3                     VLC_FOURCC('a','t','r','c')
#define VLC_CODEC_ATRAC3P                    VLC_FOURCC('a','t','r','p')
#define VLC_CODEC_IMC                        VLC_FOURCC(0x1,0x4,0x0,0x0)
#define VLC_CODEC_TRUESPEECH                 VLC_FOURCC(0x22,0x0,0x0,0x0)
#define VLC_CODEC_NELLYMOSER                 VLC_FOURCC('N','E','L','L')
#define VLC_CODEC_APE                        VLC_FOURCC('A','P','E',' ')
#define VLC_CODEC_QCELP                      VLC_FOURCC('Q','c','l','p')
#define VLC_CODEC_302M                       VLC_FOURCC('3','0','2','m')
#define VLC_CODEC_DVD_LPCM                   VLC_FOURCC('l','p','c','m')
#define VLC_CODEC_DVDA_LPCM                  VLC_FOURCC('a','p','c','m')
#define VLC_CODEC_BD_LPCM                    VLC_FOURCC('b','p','c','m')
#define VLC_CODEC_WIDI_LPCM                  VLC_FOURCC('w','p','c','m')
#define VLC_CODEC_SDDS                       VLC_FOURCC('s','d','d','s')
#define VLC_CODEC_MIDI                       VLC_FOURCC('M','I','D','I')
#define VLC_CODEC_RALF                       VLC_FOURCC('R','A','L','F')

#define VLC_CODEC_S8                         VLC_FOURCC('s','8',' ',' ')
#define VLC_CODEC_U8                         VLC_FOURCC('u','8',' ',' ')
#define VLC_CODEC_S16L                       VLC_FOURCC('s','1','6','l')
#define VLC_CODEC_S16B                       VLC_FOURCC('s','1','6','b')
#define VLC_CODEC_U16L                       VLC_FOURCC('u','1','6','l')
#define VLC_CODEC_U16B                       VLC_FOURCC('u','1','6','b')
#define VLC_CODEC_S20B                       VLC_FOURCC('s','2','0','b')
#define VLC_CODEC_S24L                       VLC_FOURCC('s','2','4','l')
#define VLC_CODEC_S24B                       VLC_FOURCC('s','2','4','b')
#define VLC_CODEC_U24L                       VLC_FOURCC('u','2','4','l')
#define VLC_CODEC_U24B                       VLC_FOURCC('u','2','4','b')
#define VLC_CODEC_S24L32                     VLC_FOURCC('s','2','4','4')
#define VLC_CODEC_S24B32                     VLC_FOURCC('S','2','4','4')
#define VLC_CODEC_S32L                       VLC_FOURCC('s','3','2','l')
#define VLC_CODEC_S32B                       VLC_FOURCC('s','3','2','b')
#define VLC_CODEC_U32L                       VLC_FOURCC('u','3','2','l')
#define VLC_CODEC_U32B                       VLC_FOURCC('u','3','2','b')
#define VLC_CODEC_F32L                       VLC_FOURCC('f','3','2','l')
#define VLC_CODEC_F32B                       VLC_FOURCC('f','3','2','b')
#define VLC_CODEC_F64L                       VLC_FOURCC('f','6','4','l')
#define VLC_CODEC_F64B                       VLC_FOURCC('f','6','4','b')

#define VLC_CODEC_ALAW                       VLC_FOURCC('a','l','a','w')
#define VLC_CODEC_MULAW                      VLC_FOURCC('m','l','a','w')
#define VLC_CODEC_DAT12                      VLC_FOURCC('L','P','1','2')
#define VLC_CODEC_S24DAUD                    VLC_FOURCC('d','a','u','d')
#define VLC_CODEC_TWINVQ                     VLC_FOURCC('T','W','I','N')
#define VLC_CODEC_BMVAUDIO                   VLC_FOURCC('B','M','V','A')
#define VLC_CODEC_ULEAD_DV_AUDIO_NTSC        VLC_FOURCC('m','s',0x02,0x15)
#define VLC_CODEC_ULEAD_DV_AUDIO_PAL         VLC_FOURCC('m','s',0x02,0x16)
#define VLC_CODEC_INDEO_AUDIO                VLC_FOURCC('m','s',0x04,0x02)
#define VLC_CODEC_METASOUND                  VLC_FOURCC('m','s',0x00,0x75)
#define VLC_CODEC_ON2AVC                     VLC_FOURCC('m','s',0x05,0x00)
#define VLC_CODEC_TAK                        VLC_FOURCC('t','a','k',' ')
#define VLC_CODEC_SMACKAUDIO                 VLC_FOURCC('S','M','K','A')

/* Subtitle */
#define VLC_CODEC_SPU       VLC_FOURCC('s','p','u',' ')
#define VLC_CODEC_DVBS      VLC_FOURCC('d','v','b','s')
#define VLC_CODEC_SUBT      VLC_FOURCC('s','u','b','t')
#define VLC_CODEC_XSUB      VLC_FOURCC('X','S','U','B')
#define VLC_CODEC_SSA       VLC_FOURCC('s','s','a',' ')
#define VLC_CODEC_TEXT      VLC_FOURCC('T','E','X','T')
#define VLC_CODEC_TELETEXT  VLC_FOURCC('t','e','l','x')
#define VLC_CODEC_KATE      VLC_FOURCC('k','a','t','e')
#define VLC_CODEC_CMML      VLC_FOURCC('c','m','m','l')
#define VLC_CODEC_ITU_T140  VLC_FOURCC('t','1','4','0')
#define VLC_CODEC_USF       VLC_FOURCC('u','s','f',' ')
#define VLC_CODEC_OGT       VLC_FOURCC('o','g','t',' ')
#define VLC_CODEC_CVD       VLC_FOURCC('c','v','d',' ')
#define VLC_CODEC_TX3G      VLC_FOURCC('t','x','3','g')
/* Blu-ray Presentation Graphics */
#define VLC_CODEC_BD_PG     VLC_FOURCC('b','d','p','g')
/* EBU STL (TECH. 3264-E) */
#define VLC_CODEC_EBU_STL   VLC_FOURCC('S','T','L',' ')
#define VLC_CODEC_SCTE_27   VLC_FOURCC('S','C','2','7')

/* XYZ colorspace 12 bits packed in 16 bits, organisation |XXX0|YYY0|ZZZ0| */
#define VLC_CODEC_XYZ12     VLC_FOURCC('X','Y','1','2')


/* Special endian dependant values
 * The suffic N means Native
 * The suffix I means Inverted (ie non native) */
#ifdef WORDS_BIGENDIAN
#   define VLC_CODEC_S16N VLC_CODEC_S16B
#   define VLC_CODEC_U16N VLC_CODEC_U16B
#   define VLC_CODEC_S24N VLC_CODEC_S24B
#   define VLC_CODEC_U24N VLC_CODEC_U24B
#   define VLC_CODEC_S32N VLC_CODEC_S32B
#   define VLC_CODEC_U32N VLC_CODEC_U32B
#   define VLC_CODEC_FL32 VLC_CODEC_F32B
#   define VLC_CODEC_FL64 VLC_CODEC_F64B

#   define VLC_CODEC_S16I VLC_CODEC_S16L
#   define VLC_CODEC_U16I VLC_CODEC_U16L
#   define VLC_CODEC_S24I VLC_CODEC_S24L
#   define VLC_CODEC_U24I VLC_CODEC_U24L
#   define VLC_CODEC_S32I VLC_CODEC_S32L
#   define VLC_CODEC_U32I VLC_CODEC_U32L

#else
#   define VLC_CODEC_S16N VLC_CODEC_S16L
#   define VLC_CODEC_U16N VLC_CODEC_U16L
#   define VLC_CODEC_S24N VLC_CODEC_S24L
#   define VLC_CODEC_U24N VLC_CODEC_U24L
#   define VLC_CODEC_S32N VLC_CODEC_S32L
#   define VLC_CODEC_U32N VLC_CODEC_U32L
#   define VLC_CODEC_FL32 VLC_CODEC_F32L
#   define VLC_CODEC_FL64 VLC_CODEC_F64L

#   define VLC_CODEC_S16I VLC_CODEC_S16B
#   define VLC_CODEC_U16I VLC_CODEC_U16B
#   define VLC_CODEC_S24I VLC_CODEC_S24B
#   define VLC_CODEC_U24I VLC_CODEC_U24B
#   define VLC_CODEC_S32I VLC_CODEC_S32B
#   define VLC_CODEC_U32I VLC_CODEC_U32B
#endif

/* Non official codecs, used to force a profile in an encoder */
/* MPEG-1 video */
#define VLC_CODEC_MP1V      VLC_FOURCC('m','p','1','v')
/* MPEG-2 video */
#define VLC_CODEC_MP2V      VLC_FOURCC('m','p','2','v')
/* MPEG-I/II layer 2 audio */
#define VLC_CODEC_MP2       VLC_FOURCC('m','p','2',' ')
/* MPEG-I/II layer 3 audio */
#define VLC_CODEC_MP3       VLC_FOURCC('m','p','3',' ')

class Live555Client
{
public:
	class LiveTrack {
	public:
		typedef struct _media_format {
			uint8_t  i_cat;  //catalog of this track (audio/video/text vv...)
			uint32_t i_codec;
			uint32_t i_extra;
			uint8_t* p_extra;
			uint32_t i_bitrate;

			bool     b_packetized;
			struct {
				int i_rate;
				int i_channels;
				int i_bitspersample;
			} audio;

			struct {
				int i_width;
				int i_height;
			} video;
		} media_format;
	protected:
		Live555Client* parent;
		void* media_sub_session;

		bool            b_muxed;
		bool            b_quicktime;
		bool            b_asf;
		//block_t         *p_asf_block;
		bool            b_discard_trunc;
		//stream_t        *p_out_muxed;    /* for muxed stream */

		uint8_t         *p_buffer;
		unsigned int    i_buffer;

		bool            b_rtcp_sync;
		char            waiting;
		int64_t         i_pts;
		double          f_npt;

		bool            b_selected;

		media_format    fmt;

	public:
		LiveTrack(Live555Client* p_sys, void* sub, int buffer_size);
		virtual ~LiveTrack();
		int init();

		bool isSelected() { return b_selected; }
		void setSelected(bool val) { b_selected = val; }
		bool isWaiting() { return waiting != 0; }
		void doWaiting(char val) { waiting = val; }
		bool isMuxed() { return b_muxed; }
		bool isQuicktime() { return b_quicktime; }
		bool isAsf() { return b_asf; }
		bool discardTruncated() { return b_discard_trunc; }
		void* getMediaSubsession() { return media_sub_session; }
		const char* getSessionId() const;
		void setNPT(double npt) { f_npt = npt; }
		double getNPT() { return f_npt; }

		media_format& getFormat() { return fmt; }

		uint8_t* buffer() { return p_buffer; }
		unsigned int buffer_size() { return i_buffer; }
		static void streamRead(void *opaque, unsigned int i_size,
                        unsigned int i_truncated_bytes, struct timeval pts,
                        unsigned int duration );
		static void streamClose(void *opaque );
	};
protected:
	void* env;
	void* scheduler;
	void* rtsp;
	void* media_session;

	char  event_rtsp;
    char  event_data;

	bool  b_error;
	bool  b_get_param;
	bool  b_is_playing;
	int   i_live555_ret; /* live555 callback return code */

	unsigned int     i_timeout;     /* session timeout value in seconds */
    bool             b_timeout_call;/* mark to send an RTSP call to prevent server timeout */

	int64_t          i_pcr; /* The clock */
	double           f_seekTime;
    double           f_npt;
    double           f_npt_length;
    double           f_npt_start;

	bool             b_no_data;     /* if we never received any data */
    int              i_no_data_ti;  /* consecutive number of TaskInterrupt */

	volatile bool    b_need_stop;
	bool             b_is_paused;
	volatile bool    b_do_control_pause_state;

	unsigned short   u_port_begin;  /* RTP port that client will be use */

	std::string user_name;
	std::string password;
	std::string sdp;
	std::string user_agent;

	std::vector<LiveTrack*> listTracks;

	volatile bool demuxLoopFlag;
	std::thread*  demuxLoopHandle;

	static void taskInterruptData( void *opaque );
	static void taskInterruptRTSP( void *opaque );

	bool waitLive555Response( int i_timeout = 0 /* ms */ );
	int setup();

	void controlPauseState();
	void controlSeek();

	int demux(void);
	static void demux_loop(void* opaque);
public:
	Live555Client(void);
	virtual ~Live555Client(void);

	virtual int open(const char* url);
	virtual int play();
	void togglePause();
	int seek(double f_time /* in second */);
	virtual int stop();

	int64_t getCurrentTime() { return (int64_t)(f_npt * 1000000.0); }
	int64_t getStartTime() { return (int64_t)(f_npt_start * 1000000.0); }

	bool isNeedStop() const { return b_need_stop; }
	bool isPaused() const { return b_is_paused; }
	bool isPlaying() const { return b_is_playing; }

	void setUser(const char* user_name, const char* password);
	void setUserAgent(const char* user_agent);

	void setRTPPortBegin(unsigned short port_begin) { u_port_begin = port_begin; }
	unsigned short getRTPPortNoUse() { return u_port_begin; }

	// callback functions
	void continueAfterDESCRIBE( int result_code, char* result_string );
	void continueAfterOPTIONS( int result_code, char* result_string );
	void live555Callback( int result_code );
	void onStreamRead(LiveTrack* track, unsigned int i_size,
                        unsigned int i_truncated_bytes, struct timeval pts,
                        unsigned int duration );
	void onStreamClose(LiveTrack* track);
	virtual void onInitializedTrack(LiveTrack* track) {}
	virtual void onEOF() {}
	virtual void onData(LiveTrack* track, uint8_t* p_buffer, int i_size, int i_truncated_bytes, int64_t pts, int64_t dts) {}
};

#endif//LIVE555_CLIENT_H__

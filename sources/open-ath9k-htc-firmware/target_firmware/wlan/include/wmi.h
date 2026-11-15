/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Qualcomm Atheros nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the definitions of the WMI protocol specified in the
 * Wireless Module Interface (WMI).  It includes definitions of all the
 * commands and events. Commands are messages from the host to the WM.
 * Events and Replies are messages from the WM to the host.
 *
 * Ownership of correctness in regards to WMI commands
 * belongs to the host driver and the WM is not required to validate
 * parameters for value, proper range, or any other checking.
 *
 * $Id: //depot/sw/branches/fusion_usb/target_firmware/wlan/include/wmi.h#6 $
 */

#ifndef _WMI_H_
#define _WMI_H_

#include "athdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HTC_PROTOCOL_VERSION    0x0002
#define HTC_PROTOCOL_REVISION   0x0000

#define WMI_PROTOCOL_VERSION    0x0002
#define WMI_PROTOCOL_REVISION   0x0000

#define ATH_MAC_LEN             6               /* length of mac in bytes */
#define WMI_CMD_MAX_LEN         100
#define WMI_CONTROL_MSG_MAX_LEN     256
#define WMI_OPT_CONTROL_MSG_MAX_LEN 1536
#define IS_ETHERTYPE(_typeOrLen)        ((_typeOrLen) >= 0x0600)
#define RFC1042OUI      {0x00, 0x00, 0x00}

#define IP_ETHERTYPE 0x0800

#define WMI_IMPLICIT_PSTREAM 0xFF
#define WMI_MAX_THINSTREAM 15

struct host_app_area_s {
    a_uint32_t wmi_protocol_ver;
};

/*
 * Data Path
 */
typedef PREPACK struct {
    a_uint8_t     dstMac[ATH_MAC_LEN];
    a_uint8_t     srcMac[ATH_MAC_LEN];
    a_uint16_t    typeOrLen;
} POSTPACK ATH_MAC_HDR;

typedef PREPACK struct {
    a_uint8_t     dsap;
    a_uint8_t     ssap;
    a_uint8_t     cntl;
    a_uint8_t     orgCode[3];
    a_uint16_t    etherType;
} POSTPACK ATH_LLC_SNAP_HDR;

typedef enum {
    DATA_MSGTYPE = 0x0,
    CNTL_MSGTYPE,
    SYNC_MSGTYPE,
    OPT_MSGTYPE,
} WMI_MSG_TYPE;


typedef PREPACK struct {
    a_int8_t      rssi;
    a_uint8_t     info;            /* WMI_MSG_TYPE in lower 2 bits - b1b0 */
                                 /* UP in next 3 bits - b4b3b2 */
#define WMI_DATA_HDR_MSG_TYPE_MASK  0x03
#define WMI_DATA_HDR_MSG_TYPE_SHIFT 0
#define WMI_DATA_HDR_UP_MASK        0x07
#define WMI_DATA_HDR_UP_SHIFT       2
#define WMI_DATA_HDR_IS_MSG_TYPE(h, t)  (((h)->info & (WMI_DATA_HDR_MSG_TYPE_MASK)) == (t))
} POSTPACK WMI_DATA_HDR;


#define WMI_DATA_HDR_SET_MSG_TYPE(h, t) (h)->info = (((h)->info & ~(WMI_DATA_HDR_MSG_TYPE_MASK << WMI_DATA_HDR_MSG_TYPE_SHIFT)) | (t << WMI_DATA_HDR_MSG_TYPE_SHIFT))
#define WMI_DATA_HDR_SET_UP(h, p) (h)->info = (((h)->info & ~(WMI_DATA_HDR_UP_MASK << WMI_DATA_HDR_UP_SHIFT)) | (p << WMI_DATA_HDR_UP_SHIFT))

/*
 * Control Path
 */
typedef PREPACK struct {
    a_uint16_t    commandId;
    a_uint16_t    seqNo;
} POSTPACK WMI_CMD_HDR;        /* used for commands and events */

/*
 * List of Commnands
 */
typedef enum {
    WMI_ECHO_CMDID           = 0x0001,
    WMI_ACCESS_MEMORY_CMDID,

    /* Commands to Target */
    WMI_GET_FW_VERSION,
    WMI_DISABLE_INTR_CMDID,
    WMI_ENABLE_INTR_CMDID,
    WMI_ATH_INIT_CMDID,
    WMI_ABORT_TXQ_CMDID,
    WMI_STOP_TX_DMA_CMDID,
    WMI_ABORT_TX_DMA_CMDID,
    WMI_DRAIN_TXQ_CMDID,
    WMI_DRAIN_TXQ_ALL_CMDID,
    WMI_START_RECV_CMDID,
    WMI_STOP_RECV_CMDID,
    WMI_FLUSH_RECV_CMDID,
    WMI_SET_MODE_CMDID,
    WMI_NODE_CREATE_CMDID,
    WMI_NODE_REMOVE_CMDID,
    WMI_VAP_REMOVE_CMDID,
    WMI_VAP_CREATE_CMDID,
    WMI_REG_READ_CMDID,
    WMI_REG_WRITE_CMDID,
    WMI_RC_STATE_CHANGE_CMDID,
    WMI_RC_RATE_UPDATE_CMDID,
    WMI_TARGET_IC_UPDATE_CMDID,
    WMI_TX_AGGR_ENABLE_CMDID,
    WMI_TGT_DETACH_CMDID,
    WMI_NODE_UPDATE_CMDID,
    WMI_INT_STATS_CMDID,
    WMI_TX_STATS_CMDID,
    WMI_RX_STATS_CMDID,
    WMI_BITRATE_MASK_CMDID,
    WMI_REG_RMW_CMDID,
    /* CUSTOM BUILD: must match enum wmi_cmd_id in the Linux driver's wmi.h */
    WMI_CUSTOM_PING_CMDID,
    WMI_CUSTOM_RC_PROFILE_CMDID,
    WMI_CUSTOM_TXPOWER_CMDID,
    WMI_CUSTOM_RATE_USAGE_CMDID,
    WMI_CUSTOM_ANI_AUTO_CMDID,
    WMI_CUSTOM_RSSI_HIST_CMDID,
    WMI_CUSTOM_MACFILTER_CMDID,
    WMI_CUSTOM_BAW_CMDID,
    WMI_CUSTOM_LONGDIST_CMDID,
    WMI_CUSTOM_FREQ_NOTE_CMDID,
    WMI_CUSTOM_NETFLOW_CMDID,
    WMI_CUSTOM_CHAN_SURVEY_CMDID,
    WMI_CUSTOM_AUTO_SPEC_CMDID,
    WMI_CUSTOM_BLACKBOX_CMDID,
    WMI_CUSTOM_THERMAL_CMDID,
} WMI_COMMAND_ID;

/*
 * CUSTOM BUILD: custom ping command/response (host -> target -> host)
 */
typedef PREPACK struct {
    A_UINT32 payload;
} POSTPACK WMI_CUSTOM_PING_CMD;

typedef PREPACK struct {
    A_UINT32 echo_payload;
    A_UINT32 pong_counter;
    /* CUSTOM BUILD v3-diag: target-side control-path health counters,
     * filled by handle_custom_ping_cmd from live globals. 24B total -
     * well under the 64B reply budget. */
    A_UINT32 diag_rx_cmds;       /* commands received on the CTL endpoint   */
    A_UINT32 diag_send_dones;    /* WMI buffers returned by HTC send-done  */
    A_UINT32 diag_rsp_entries;   /* wmi_cmd_rsp invocations                */
    A_UINT32 diag_rsp_allocfail; /* WMI_AllocEvent failures in wmi_cmd_rsp */
    A_UINT32 diag_pool_memfail;  /* buffers with NULL data area at boot    */
    A_UINT32 diag_evt_entries;   /* target->host events sent (SWBA/...)    */
} POSTPACK WMI_CUSTOM_PING_RSP;

/*
 * CUSTOM BUILD: runtime rate-control profile (host -> target -> host).
 * step_dn_inc = 1 -> clean-air profile (slower fallback)
 * step_dn_inc = 2 -> noisy-air profile (faster fallback)
 * step_dn_inc = 0 -> keep current value (query only)
 * fixed_rate_idx: 0xFF = auto rate control, 0x00-0x7F = fixed rate table
 *                 index, anything else = keep current.
 */
typedef PREPACK struct {
    A_UINT32 step_dn_inc;
    A_UINT32 fixed_rate_idx;
    A_UINT32 num_tries;      /* CUSTOM BUILD v3: 0 = keep, else 1..32 */
    A_UINT32 num_rates;      /* CUSTOM BUILD v3: 0 = keep, else 1..4 */
} POSTPACK WMI_CUSTOM_RC_PROFILE_CMD;

typedef PREPACK struct {
    A_UINT32 step_dn_inc;    /* current effective value */
    A_UINT32 fixed_rate_idx; /* current effective value */
    A_UINT32 num_tries;      /* current effective value */
    A_UINT32 num_rates;      /* current effective value */
} POSTPACK WMI_CUSTOM_RC_PROFILE_RSP;

/*
 * CUSTOM BUILD: per-frame TX power override (host -> target -> host).
 * The stock firmware hardcodes 60 (of 0..63) as the descriptor XmitPower.
 * tx_power = 0xFF -> query only (keep current), 0x00-0x3F -> set.
 */
typedef PREPACK struct {
    A_UINT32 tx_power;
} POSTPACK WMI_CUSTOM_TXPOWER_CMD;

typedef PREPACK struct {
    A_UINT32 tx_power;       /* current effective value */
} POSTPACK WMI_CUSTOM_TXPOWER_RSP;

/*
 * CUSTOM BUILD v2: firmware-resident extensions
 */

typedef PREPACK struct {
    A_UINT32 seq;        /* sample counter */
    A_UINT32 uptime_s;   /* seconds since target boot (tick-based) */
    A_UINT32 freq_mhz;   /* current channel center frequency */
    A_INT32  nf_dbm;     /* last latched noise floor (chain 0) */
    A_UINT32 rx_frames;  /* frames received in the last window */
    A_UINT32 phy_errs;   /* phy-error frames in the last window */
    A_UINT32 ani_level;  /* current firstep level 0..2 */
} POSTPACK WMI_CUSTOM_TELEMETRY_EVT;

/* per-rate TX usage histogram (counted at rate-series build time) */
#define WMI_CUSTOM_RATE_USAGE_MAX 32
typedef PREPACK struct {
    A_UINT32 reset;                                   /* 1 = reset after read */
} POSTPACK WMI_CUSTOM_RATE_USAGE_CMD;

typedef PREPACK struct {
    A_UINT8  usage[WMI_CUSTOM_RATE_USAGE_MAX];        /* saturated at 255;
							 keep replies SMALL! */
} POSTPACK WMI_CUSTOM_RATE_USAGE_RSP;

/* firmware-side adaptive ANI (firstep auto-stepping on PHY error rate) */
typedef PREPACK struct {
    A_UINT32 enable;    /* 0xFF = query, 0 = manual, 1 = firmware adaptive */
} POSTPACK WMI_CUSTOM_ANI_AUTO_CMD;

typedef PREPACK struct {
    A_UINT32 enable;    /* current state */
    A_UINT32 level;     /* current firstep level 0..2 */
    A_UINT32 phy_errs;  /* phy-error frames in the last window */
} POSTPACK WMI_CUSTOM_ANI_AUTO_RSP;

/* RSSI histogram (64 bins, 1 dB bins, u8 saturated counters) */
#define WMI_CUSTOM_RSSI_HIST_BINS 48
typedef PREPACK struct {
    A_UINT32 reset;    /* 0xFF = query, 1 = reset after read */
} POSTPACK WMI_CUSTOM_RSSI_HIST_CMD;

typedef PREPACK struct {
    /* no total field: the driver sums the bins (reply budget!) */
    A_UINT8  bins[WMI_CUSTOM_RSSI_HIST_BINS];
} POSTPACK WMI_CUSTOM_RSSI_HIST_RSP;

/* radio-level MAC filter, applied before frames are uploaded over USB */
#define WMI_CUSTOM_MACFILTER_MAX 8
#define CUSTOM_MF_ACTION_QUERY   0
#define CUSTOM_MF_ACTION_SETMODE 1
#define CUSTOM_MF_ACTION_ADD     2
#define CUSTOM_MF_ACTION_DEL     3
#define CUSTOM_MF_ACTION_CLEAR   4

typedef PREPACK struct {
    A_UINT32 action;   /* CUSTOM_MF_ACTION_* */
    A_UINT32 mode;     /* 0 = off, 1 = allow-list, 2 = block-list */
    A_UINT32 index;    /* entry index for add/del (0..7) */
    A_UINT8  mac[6];
} POSTPACK WMI_CUSTOM_MACFILTER_CMD;

typedef PREPACK struct {
    A_UINT32 mode;
    A_UINT32 count;
    A_UINT32 dropped;   /* frames dropped by the radio-level filter */
} POSTPACK WMI_CUSTOM_MACFILTER_RSP;

/*
 * CUSTOM BUILD v4 (thermal): heat governor control + link-aware TX power
 * cap. The AR9271 exposes no readable die-temperature register, so
 * "thermal" means managing the dominant heat sources directly: the PA
 * output power (descriptor XmitPower) and the TX duty cycle. The governor
 * lives in the 2s RX-driven tick (if_ath.c); it generates no periodic WMI
 * traffic (docs/10 golden rule #4).
 */
#define CUSTOM_TH_QUERY          0xFF   /* mode: query-only */
#define CUSTOM_TH_CAP_QUERY      0x100  /* cap: query-only */
#define CUSTOM_TH_CAP_UNCAPPED   0xFF   /* cap: remove the static cap
                                           (docs/12: "cap 255 = uncapped" -
                                           unreachable before v4-fix, the
                                           old code intercepted 0xFF as the
                                           query marker for BOTH fields) */
#define CUSTOM_TH_FLAG_HOOKS_ON  0x1   /* set custom_rx_hooks = 1 */
#define CUSTOM_TH_FLAG_HOOKS_OFF 0x2   /* set custom_rx_hooks = 0 */

typedef PREPACK struct {
    A_UINT32 mode;     /* CUSTOM_TH_QUERY = query, else bit0 = RSSI cap,
                          bit1 = duty backoff */
    A_UINT32 cap;      /* 0x00-0x3F = static cap, CUSTOM_TH_CAP_UNCAPPED =
                          remove cap, CUSTOM_TH_CAP_QUERY = query only */
    A_UINT32 flags;    /* CUSTOM_TH_FLAG_* (0 = leave hooks untouched) */
} POSTPACK WMI_CUSTOM_THERMAL_CMD;

typedef PREPACK struct {
    A_UINT32 mode;       /* active governor mode bits */
    A_UINT32 cap;        /* static cap as configured (0xFF = uncapped) */
    A_UINT32 backoff;    /* dynamic duty backoff, descriptor power steps */
    A_UINT32 effective;  /* XmitPower applied to data frames right now */
    A_UINT32 avg_rssi;   /* window-average RSSI (0 = no RX traffic yet) */
    A_UINT32 tx_window;  /* TX rate-usage delta over the last tick */
    A_UINT32 rx_window;  /* RX frames over the last tick */
} POSTPACK WMI_CUSTOM_THERMAL_RSP;

/*
 * Frame Types
 */
typedef enum {
    WMI_FRAME_BEACON        =   0,
    WMI_FRAME_PROBE_REQ,
    WMI_FRAME_PROBE_RESP,
    WMI_FRAME_ASSOC_REQ,
    WMI_FRAME_ASSOC_RESP,
    WMI_NUM_MGMT_FRAME
} WMI_MGMT_FRAME_TYPE;

/*
 * Connect Command
 */
typedef enum {
    INFRA_NETWORK       = 0x01,
    ADHOC_NETWORK       = 0x02,
    ADHOC_CREATOR       = 0x04,
    OPT_NETWORK         = 0x08,
} NETWORK_TYPE;

typedef enum {
    OPEN_AUTH           = 0x01,
    SHARED_AUTH         = 0x02,
    LEAP_AUTH           = 0x04,  /* different from IEEE_AUTH_MODE definitions */
} DOT11_AUTH_MODE;

typedef enum {
    NONE_AUTH           = 0x01,
    WPA_AUTH            = 0x02,
    WPA_PSK_AUTH        = 0x03,
    WPA2_AUTH           = 0x04,
    WPA2_PSK_AUTH       = 0x05,
    WPA_AUTH_CCKM       = 0x06,
    WPA2_AUTH_CCKM      = 0x07,
} AUTH_MODE;

typedef enum {
    NONE_CRYPT          = 0x01,
    WEP_CRYPT           = 0x02,
    TKIP_CRYPT          = 0x03,
    AES_CRYPT           = 0x04,
} CRYPTO_TYPE;

#define WMI_MIN_CRYPTO_TYPE NONE_CRYPT
#define WMI_MAX_CRYPTO_TYPE (AES_CRYPT + 1)

#define WMI_MIN_KEY_INDEX   0
#define WMI_MAX_KEY_INDEX   3

#define WMI_MAX_KEY_LEN     32

#define WMI_MAX_SSID_LEN    32

typedef enum {
    CONNECT_ASSOC_POLICY_USER = 0x0001,
    CONNECT_SEND_REASSOC = 0x0002,
    CONNECT_IGNORE_WPAx_GROUP_CIPHER = 0x0004,
    CONNECT_PROFILE_MATCH_DONE = 0x0008,
    CONNECT_IGNORE_AAC_BEACON = 0x0010,
    CONNECT_CSA_FOLLOW_BSS = 0x0020,
} WMI_CONNECT_CTRL_FLAGS_BITS;

#define DEFAULT_CONNECT_CTRL_FLAGS    (CONNECT_CSA_FOLLOW_BSS)

/*
 * WMI_ECHO_CMDID
 */
#define WMI_ECHOCMD_MSG_MAX_LEN         53//64 - HTC_HDR_LENGTH + sizeof(WMI_CMD_HDR) - 1

typedef PREPACK struct {
    a_uint8_t     msgSize;
    a_uint8_t     msgData[1];
} POSTPACK WMI_ECHO_CMD;

/*
 * WMI_ACCESS_MEMORY_CMDID
 */
#define WMI_ACCESS_MEMORY_MAX_TUPLES    8

typedef PREPACK struct {
    a_uint16_t    addressL;
    a_uint16_t    addressH;
    a_uint16_t    valueL;
    a_uint16_t    valueH;
} POSTPACK WMI_AVT;

typedef PREPACK struct {
    a_uint16_t     tupleNumL;
    a_uint16_t     tupleNumH;
    WMI_AVT      avt[1];
} POSTPACK WMI_ACCESS_MEMORY_CMD;

/*
 * List of Events (target to host)
 */
typedef enum {
    WMI_TGT_RDY_EVENTID           = 0x1001,
    WMI_SWBA_EVENTID,
    WMI_FATAL_EVENTID,
    WMI_TXTO_EVENTID,
    WMI_BMISS_EVENTID,
    WMI_DELBA_EVENTID,
    WMI_TXSTATUS_EVENTID,
    /* CUSTOM BUILD v2: periodic telemetry (must match driver enum append) */
    WMI_CUSTOM_TELEMETRY_EVENTID
} WMI_EVENT_ID;

typedef PREPACK struct {
	a_uint64_t tsf;
	a_uint8_t beaconPendingCount;
} POSTPACK WMI_SWBA_EVENT;

typedef PREPACK struct {
	a_uint8_t    cookie;
	a_uint8_t    ts_rate;
	a_uint8_t    ts_flags;
} POSTPACK __WMI_TXSTATUS_EVENT;

#define HTC_MAX_TX_STATUS 12

typedef PREPACK struct {
	a_uint8_t cnt;
	__WMI_TXSTATUS_EVENT txstatus[HTC_MAX_TX_STATUS];
} POSTPACK WMI_TXSTATUS_EVENT;

struct register_rmw {
	a_uint32_t reg;
	a_uint32_t set;
	a_uint32_t clr;
};

#ifndef ATH_TARGET
//#include "athendpack.h"
#endif

#ifdef __cplusplus
}
#endif

/*
 * CUSTOM BUILD v3: firmware-resident services.
 * RULE: every reply struct stays at or below 64 bytes (reply pool budget!)
 */
#define WMI_CUSTOM_NETFLOW_MAX 4
#define WMI_CUSTOM_CHAN_SURVEY_MAX 12
#define WMI_CUSTOM_BB_PAGES 4
#define WMI_CUSTOM_BB_PER_PG 4

/* Block-ACK window runtime tuning (power of two, 1..64) */
typedef PREPACK struct {
    A_UINT32 baw_size;   /* 0xFF = query, else new size */
} POSTPACK WMI_CUSTOM_BAW_CMD;

typedef PREPACK struct {
    A_UINT32 effective;
} POSTPACK WMI_CUSTOM_BAW_RSP;

/* Long-distance mode: MAC ACK timeout in microseconds (reg 0x8014) */
typedef PREPACK struct {
    A_UINT32 ack_timeout_us;   /* 0xFF = query, else 1..16383 */
} POSTPACK WMI_CUSTOM_LONGDIST_CMD;

typedef PREPACK struct {
    A_UINT32 ack_timeout_us;
} POSTPACK WMI_CUSTOM_LONGDIST_RSP;

/* host notifies the target of the current channel (NF survey pairing) */
typedef PREPACK struct {
    A_UINT32 freq_mhz;
} POSTPACK WMI_CUSTOM_FREQ_NOTE_CMD;

typedef PREPACK struct {
    A_UINT32 entries;
} POSTPACK WMI_CUSTOM_FREQ_NOTE_RSP;

/* mini-NetFlow: per-MAC frame/byte accounting (4 entries, u16 saturated) */
typedef PREPACK struct {
    A_UINT32 reset;   /* 1 = reset frames/bytes after read */
} POSTPACK WMI_CUSTOM_NETFLOW_CMD;

/* CUSTOM BUILD v4-fix: the old layout was actually 4*12+4 = 52B payload (the
 * "4*10" comment was wrong) = 64B on the wire bare (8B HTC + 4B WMI hdr) -
 * exactly the EP3 max packet size, which jams the USB controller after
 * transmission (usbmon evidence). bytes is now ONE u16 in 256B units
 * (saturating), so an entry is the 10B the comment always claimed and the
 * reply is 44B payload = 56B on the wire, below the boundary. */
typedef PREPACK struct {
    A_UINT8  count;
    A_UINT8  rsv[3];
    struct {
        A_UINT8  mac[6];
        A_UINT16 frames;
        A_UINT16 bytes_256;   /* bytes >> 8, saturating at 0xFFFF (~16MB) */
    } ent[WMI_CUSTOM_NETFLOW_MAX];
} POSTPACK WMI_CUSTOM_NETFLOW_RSP;   /* 4 + 4*10 = 44 bytes */

/* per-channel NF survey (12 entries)
 * CUSTOM BUILD v4-fix: the old layout (nf as s16) made the reply 52B payload
 * = 64B on the wire bare (8B HTC + 4B WMI hdr + 52B) - exactly the EP3 max
 * packet size, which jams the USB controller after transmission (usbmon
 * evidence, docs/10). nf is now s8 (NF is coarse; -128..0 dBm covers it), so
 * the reply is 38B payload = 46B on the wire, safely below the boundary. */
typedef PREPACK struct {
    A_UINT8  count;
    A_UINT8  rsv;
    struct {
        A_UINT16 freq_mhz;
        A_INT8   nf_dbm;
    } ent[WMI_CUSTOM_CHAN_SURVEY_MAX];
} POSTPACK WMI_CUSTOM_CHAN_SURVEY_RSP;   /* 2 + 12*3 = 38 bytes */

/* firmware auto-spectral: burst on PHY-error spikes */
typedef PREPACK struct {
    A_UINT32 enable;     /* 0xFF = query, 0 = off, 1 = on */
    A_UINT32 threshold;  /* phy-err rate (per-mille) trigger */
} POSTPACK WMI_CUSTOM_AUTO_SPEC_CMD;

typedef PREPACK struct {
    A_UINT32 enable;
    A_UINT32 threshold;
    A_UINT32 active;
} POSTPACK WMI_CUSTOM_AUTO_SPEC_RSP;

/* black-box ring buffer (16 events, 4 per page) */
typedef PREPACK struct {
    A_UINT32 page;   /* 0..3 */
} POSTPACK WMI_CUSTOM_BLACKBOX_CMD;

typedef PREPACK struct {
    A_UINT8  head;
    A_UINT8  count;
    A_UINT8  rsv[2];
    A_UINT32 ent[WMI_CUSTOM_BB_PER_PG];   /* type<<24 | data&0xFFFFFF */
} POSTPACK WMI_CUSTOM_BLACKBOX_RSP;   /* 4 + 16 = 20 bytes */

#endif /* _WMI_H_ */

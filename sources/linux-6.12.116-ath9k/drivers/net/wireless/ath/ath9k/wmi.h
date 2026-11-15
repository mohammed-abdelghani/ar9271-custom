/*
 * Copyright (c) 2010-2011 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef WMI_H
#define WMI_H

struct wmi_event_txrate {
	__be32 txrate;
	struct {
		u8 rssi_thresh;
		u8 per;
	} rc_stats;
} __packed;

struct wmi_cmd_hdr {
	__be16 command_id;
	__be16 seq_no;
} __packed;

struct wmi_fw_version {
	__be16 major;
	__be16 minor;

} __packed;

/* CUSTOM BUILD: custom ping round trip (host -> target -> host) */
struct wmi_custom_ping_cmd {
	__be32 payload;
} __packed;

struct wmi_custom_ping_rsp {
	__be32 echo_payload;
	__be32 pong_counter;
	/* CUSTOM BUILD v3-diag: target-side control-path health counters
	 * (must match WMI_CUSTOM_PING_RSP in the firmware's wmi.h) */
	__be32 diag_rx_cmds;
	__be32 diag_send_dones;
	__be32 diag_rsp_entries;
	__be32 diag_rsp_allocfail;
	__be32 diag_pool_memfail;
	__be32 diag_evt_entries;
} __packed;

/* CUSTOM BUILD v3: runtime rate-control profile (full series control) */
struct wmi_custom_rc_profile_cmd {
	__be32 step_dn_inc;    /* 1 = clean, 2 = noisy, 0 = query only */
	__be32 fixed_rate_idx; /* 0xFF = auto RC, 0x00-0x7F = fixed, else keep */
	__be32 num_tries;      /* 0 = keep, else total tries 1..32 */
	__be32 num_rates;      /* 0 = keep, else rates in series 1..4 */
} __packed;

struct wmi_custom_rc_profile_rsp {
	__be32 step_dn_inc;
	__be32 fixed_rate_idx;
	__be32 num_tries;
	__be32 num_rates;
} __packed;

/* CUSTOM BUILD: per-frame TX power override (stock fw hardcodes 60) */
struct wmi_custom_txpower_cmd {
	__be32 tx_power;   /* 0xFF = query only, 0x00-0x3F = set */
} __packed;

struct wmi_custom_txpower_rsp {
	__be32 tx_power;   /* current effective value on the target */
} __packed;

/* CUSTOM BUILD v4 (thermal): heat governor control. No die-temperature
 * sensor exists on the AR9271 - the firmware caps PA output power
 * (descriptor XmitPower) by link quality plus an optional static cap and
 * an optional duty backoff. Reply is 28B, under the 64B reply budget. */
#define CUSTOM_TH_QUERY          0xFF   /* mode: query-only */
/* CUSTOM BUILD v4-fix: cap 0xFF now means UNCAP (docs/12 "cap 255 removes
 * the cap" - the old firmware intercepted 0xFF as the query marker for both
 * fields, making uncap unreachable). The cap query marker is 0x100. */
#define CUSTOM_TH_CAP_QUERY      0x100
#define CUSTOM_TH_CAP_UNCAPPED   0xFF
#define CUSTOM_TH_FLAG_HOOKS_ON  0x1   /* set target custom_rx_hooks = 1 */
#define CUSTOM_TH_FLAG_HOOKS_OFF 0x2   /* set target custom_rx_hooks = 0 */

struct wmi_custom_thermal_cmd {
	__be32 mode;   /* 0xFF = query; bit0 = RSSI cap, bit1 = duty backoff */
	__be32 cap;    /* 0x00-0x3F = static cap, 0xFF = uncapped,
			  0x100 = query only */
	__be32 flags;  /* CUSTOM_TH_FLAG_* (0 = leave RX hooks untouched) */
} __packed;

struct wmi_custom_thermal_rsp {
	__be32 mode;
	__be32 cap;        /* static cap as configured (0xFF = uncapped) */
	__be32 backoff;    /* dynamic duty backoff, power steps */
	__be32 effective;  /* XmitPower applied to data frames right now */
	__be32 avg_rssi;   /* window-average RSSI (0 = no RX traffic yet) */
	__be32 tx_window;  /* TX rate-usage delta over the last 2s tick */
	__be32 rx_window;  /* RX frames over the last 2s tick */
} __packed;

/* CUSTOM BUILD v2: periodic telemetry event (target -> host) */
struct wmi_custom_telemetry_evt {
	__be32 seq;
	__be32 uptime_s;
	__be32 freq_mhz;
	__be32 nf_dbm;
	__be32 rx_frames;
	__be32 phy_errs;
	__be32 ani_level;
	__be32 dbg_last_cmd;
	__be32 dbg_tbl_entries;
	__be32 beacons_window;
	__be32 filter_dropped;
} __packed;

/* CUSTOM BUILD v2: per-rate TX usage histogram.
 * NOTE: replies must stay at or below 64 bytes (pool assert otherwise
 * halts the target firmware!) - hence u8 bins. */
#define WMI_CUSTOM_RATE_USAGE_MAX 32
struct wmi_custom_rate_usage_cmd {
	__be32 reset;   /* 1 = reset after read */
} __packed;

struct wmi_custom_rate_usage_rsp {
	u8 usage[WMI_CUSTOM_RATE_USAGE_MAX];
} __packed;

/* CUSTOM BUILD v2: firmware-side adaptive ANI */
struct wmi_custom_ani_auto_cmd {
	__be32 enable;  /* 0xFF = query, 0 = manual, 1 = firmware adaptive */
} __packed;

struct wmi_custom_ani_auto_rsp {
	__be32 enable;
	__be32 level;
	__be32 phy_errs;
} __packed;

/* CUSTOM BUILD v2: RSSI histogram (64 bins, u8 saturated; no total field -
 * the reply budget is 64 bytes, the driver sums the bins instead) */
#define WMI_CUSTOM_RSSI_HIST_BINS 48
struct wmi_custom_rssi_hist_cmd {
	__be32 reset;   /* 0xFF = query, 1 = reset after read */
} __packed;

struct wmi_custom_rssi_hist_rsp {
	u8 bins[WMI_CUSTOM_RSSI_HIST_BINS];
} __packed;

/* CUSTOM BUILD v2: radio-level MAC filter */
#define WMI_CUSTOM_MACFILTER_MAX 8
struct wmi_custom_macfilter_cmd {
	__be32 action;  /* 0=query, 1=setmode, 2=add, 3=del, 4=clear */
	__be32 mode;    /* 0 = off, 1 = allow-list, 2 = block-list */
	__be32 index;   /* entry index 0..7 */
	u8 mac[6];
} __packed;

struct wmi_custom_macfilter_rsp {
	__be32 mode;
	__be32 count;
	__be32 dropped;   /* frames dropped by the radio-level filter */
} __packed;

struct wmi_event_swba {
	__be64 tsf;
	u8 beacon_pending;
} __packed;

/*
 * 64 - HTC header - WMI header - 1 / txstatus
 * And some other hdr. space is also accounted for.
 * 12 seems to be the magic number.
 */
#define HTC_MAX_TX_STATUS 12

#define ATH9K_HTC_TXSTAT_ACK        BIT(0)
#define ATH9K_HTC_TXSTAT_FILT       BIT(1)
#define ATH9K_HTC_TXSTAT_RTC_CTS    BIT(2)
#define ATH9K_HTC_TXSTAT_MCS        BIT(3)
#define ATH9K_HTC_TXSTAT_CW40       BIT(4)
#define ATH9K_HTC_TXSTAT_SGI        BIT(5)

/*
 * Legacy rates are indicated as indices.
 * HT rates are indicated as dot11 numbers.
 * This allows us to resrict the rate field
 * to 4 bits.
 */
#define ATH9K_HTC_TXSTAT_RATE       0x0f
#define ATH9K_HTC_TXSTAT_RATE_S     0

#define ATH9K_HTC_TXSTAT_EPID       0xf0
#define ATH9K_HTC_TXSTAT_EPID_S     4

struct __wmi_event_txstatus {
	u8 cookie;
	u8 ts_rate; /* Also holds EP ID */
	u8 ts_flags;
};

struct wmi_event_txstatus {
	u8 cnt;
	struct __wmi_event_txstatus txstatus[HTC_MAX_TX_STATUS];
} __packed;

enum wmi_cmd_id {
	WMI_ECHO_CMDID = 0x0001,
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
	/* CUSTOM BUILD: must match WMI_COMMAND_ID in the firmware's wmi.h */
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
};

enum wmi_event_id {
	WMI_TGT_RDY_EVENTID = 0x1001,
	WMI_SWBA_EVENTID,
	WMI_FATAL_EVENTID,
	WMI_TXTO_EVENTID,
	WMI_BMISS_EVENTID,
	WMI_DELBA_EVENTID,
	WMI_TXSTATUS_EVENTID,
	/* CUSTOM BUILD v2: periodic telemetry (matches fw WMI_EVENT_ID append) */
	WMI_CUSTOM_TELEMETRY_EVENTID,
};

#define MAX_CMD_NUMBER 62
#define MAX_RMW_CMD_NUMBER 15

struct register_write {
	__be32 reg;
	__be32 val;
};

struct register_rmw {
	__be32 reg;
	__be32 set;
	__be32 clr;
} __packed;

struct ath9k_htc_tx_event {
	int count;
	struct __wmi_event_txstatus txs;
	struct list_head list;
};

struct wmi {
	struct ath9k_htc_priv *drv_priv;
	struct htc_target *htc;
	enum htc_endpoint_id ctrl_epid;
	struct mutex op_mutex;
	struct completion cmd_wait;
	u16 last_seq_id;
	struct sk_buff_head wmi_event_queue;
	struct tasklet_struct wmi_event_tasklet;
	u16 tx_seq_id;
	u8 *cmd_rsp_buf;
	u32 cmd_rsp_len;
	bool stopped;
	/* CUSTOM BUILD v3-diag: host-side command-path health */
	u32 cmd_timeouts;
	/* CUSTOM BUILD v4-diag: last failed command details + last response
	 * time, so wmi_diag can say exactly WHAT died and WHEN, not just
	 * "some timeouts happened". All fields are updated under wmi_lock. */
	u16 last_fail_cmd_id;
	u16 last_fail_seq;
	int last_fail_errno;
	unsigned long last_rsp_jiffies;
	u32 rsp_count;

	struct list_head pending_tx_events;
	spinlock_t event_lock;

	spinlock_t wmi_lock;

	/* multi write section */
	atomic_t mwrite_cnt;
	struct register_write multi_write[MAX_CMD_NUMBER];
	u32 multi_write_idx;
	struct mutex multi_write_mutex;

	/* multi rmw section */
	atomic_t m_rmw_cnt;
	struct register_rmw multi_rmw[MAX_RMW_CMD_NUMBER];
	u32 multi_rmw_idx;
	struct mutex multi_rmw_mutex;

};

struct wmi *ath9k_init_wmi(struct ath9k_htc_priv *priv);
int ath9k_wmi_connect(struct htc_target *htc, struct wmi *wmi,
		      enum htc_endpoint_id *wmi_ctrl_epid);
int ath9k_wmi_cmd(struct wmi *wmi, enum wmi_cmd_id cmd_id,
		  u8 *cmd_buf, u32 cmd_len,
		  u8 *rsp_buf, u32 rsp_len,
		  u32 timeout);
int ath9k_wmi_custom_ping(struct wmi *wmi, u32 payload,
			  u32 *echo_payload, u32 *pong_counter,
			  u32 diag[4]);
int ath9k_wmi_rc_profile(struct wmi *wmi, u32 step_dn_inc,
			 u32 fixed_rate_idx,
			 u32 *eff_step_dn, u32 *eff_fixed_rate);
int ath9k_wmi_txpower(struct wmi *wmi, u32 tx_power, u32 *effective);
int ath9k_wmi_thermal(struct wmi *wmi, u32 mode, u32 cap, u32 flags,
		      struct wmi_custom_thermal_rsp *out);
int ath9k_wmi_rate_usage(struct wmi *wmi, u32 reset, u8 usage[32]);
int ath9k_wmi_ani_auto(struct wmi *wmi, u32 enable,
		       u32 *state, u32 *level, u32 *errs);
int ath9k_wmi_rssi_hist(struct wmi *wmi, u32 reset, u8 bins[64]);
int ath9k_wmi_macfilter(struct wmi *wmi, u32 action, u32 mode, u32 index,
			const u8 *mac, u32 *mode_out, u32 *count_out);
int ath9k_wmi_rc_profile_v3(struct wmi *wmi, u32 step, u32 fixed, u32 tries,
			    u32 rates, u32 *eff_step, u32 *eff_fixed,
			    u32 *eff_tries, u32 *eff_rates);
int ath9k_wmi_baw(struct wmi *wmi, u32 size, u32 *effective);
int ath9k_wmi_longdist(struct wmi *wmi, u32 us, u32 *cur_out);
int ath9k_wmi_freq_note(struct wmi *wmi, u32 freq_mhz, u32 *entries);
int ath9k_wmi_netflow(struct wmi *wmi, u32 reset, u8 *count,
		      u8 macs[6][6], u16 frames[6], u32 bytes[6]);
int ath9k_wmi_chan_survey(struct wmi *wmi, u8 *count,
			  u16 freq[12], s16 nf[12]);
int ath9k_wmi_auto_spec(struct wmi *wmi, u32 enable, u32 threshold,
			u32 *state, u32 *th_out, u32 *active);
int ath9k_wmi_blackbox(struct wmi *wmi, u32 page,
		       u8 *head, u8 *count, u32 ent[4]);
void ath9k_wmi_event_tasklet(struct tasklet_struct *t);
void ath9k_fatal_work(struct work_struct *work);
void ath9k_wmi_event_drain(struct ath9k_htc_priv *priv);
void ath9k_stop_wmi(struct ath9k_htc_priv *priv);
void ath9k_destroy_wmi(struct ath9k_htc_priv *priv);

#define WMI_CMD(_wmi_cmd)						\
	do {								\
		ret = ath9k_wmi_cmd(priv->wmi, _wmi_cmd, NULL, 0,	\
				    (u8 *) &cmd_rsp,			\
				    sizeof(cmd_rsp), HZ*2);		\
	} while (0)

#define WMI_CMD_BUF(_wmi_cmd, _buf)					\
	do {								\
		ret = ath9k_wmi_cmd(priv->wmi, _wmi_cmd,		\
				    (u8 *) _buf, sizeof(*_buf),		\
				    &cmd_rsp, sizeof(cmd_rsp), HZ*2);	\
	} while (0)

/* ===== CUSTOM BUILD v3: firmware-resident services ===== */

struct wmi_custom_baw_cmd {
	__be32 baw_size;   /* 0xFF = query, else new size */
} __packed;

struct wmi_custom_baw_rsp {
	__be32 effective;
} __packed;

struct wmi_custom_longdist_cmd {
	__be32 ack_timeout_us;   /* 0xFF = query, else 1..16383 */
} __packed;

struct wmi_custom_longdist_rsp {
	__be32 ack_timeout_us;
} __packed;

struct wmi_custom_freq_note_cmd {
	__be32 freq_mhz;
} __packed;

struct wmi_custom_freq_note_rsp {
	__be32 entries;
} __packed;

#define WMI_CUSTOM_NETFLOW_MAX 4
struct wmi_custom_netflow_cmd {
	__be32 reset;
} __packed;

/* CUSTOM BUILD v4-fix: bytes is ONE u16 in 256B units (saturating) so the
 * reply is 44B payload = 56B on the wire, below the 64B EP3 max-packet
 * boundary that jams the USB controller (the old 12B entry = 52B payload =
 * exactly 64B). Must match WMI_CUSTOM_NETFLOW_RSP in the firmware wmi.h. */
struct wmi_custom_netflow_rsp {
	u8 count;
	u8 rsv[3];
	struct {
		u8 mac[6];
		__be16 frames;
		__be16 bytes_256;
	} ent[WMI_CUSTOM_NETFLOW_MAX];
} __packed;

#define WMI_CUSTOM_CHAN_SURVEY_MAX 12
/* CUSTOM BUILD v4-fix: nf is s8 now (was be16) so the reply is 38B payload =
 * 46B on the wire, below the 64B EP3 max-packet boundary that jams the USB
 * controller (must match WMI_CUSTOM_CHAN_SURVEY_RSP in the firmware wmi.h). */
struct wmi_custom_chan_survey_rsp {
	u8 count;
	u8 rsv;
	struct {
		__be16 freq_mhz;
		s8 nf_dbm;
	} ent[WMI_CUSTOM_CHAN_SURVEY_MAX];
} __packed;

struct wmi_custom_auto_spec_cmd {
	__be32 enable;
	__be32 threshold;
} __packed;

struct wmi_custom_auto_spec_rsp {
	__be32 enable;
	__be32 threshold;
	__be32 active;
} __packed;

#define WMI_CUSTOM_BB_PAGES   4
#define WMI_CUSTOM_BB_PER_PG  4
struct wmi_custom_blackbox_cmd {
	__be32 page;
} __packed;

struct wmi_custom_blackbox_rsp {
	u8 head;
	u8 count;
	u8 rsv[2];
	__be32 ent[WMI_CUSTOM_BB_PER_PG];
} __packed;

#endif /* WMI_H */

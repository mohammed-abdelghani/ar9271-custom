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

#include "htc.h"

static const char *wmi_cmd_to_name(enum wmi_cmd_id wmi_cmd)
{
	switch (wmi_cmd) {
	case WMI_ECHO_CMDID:
		return "WMI_ECHO_CMDID";
	case WMI_ACCESS_MEMORY_CMDID:
		return "WMI_ACCESS_MEMORY_CMDID";
	case WMI_GET_FW_VERSION:
		return "WMI_GET_FW_VERSION";
	case WMI_DISABLE_INTR_CMDID:
		return "WMI_DISABLE_INTR_CMDID";
	case WMI_ENABLE_INTR_CMDID:
		return "WMI_ENABLE_INTR_CMDID";
	case WMI_ATH_INIT_CMDID:
		return "WMI_ATH_INIT_CMDID";
	case WMI_ABORT_TXQ_CMDID:
		return "WMI_ABORT_TXQ_CMDID";
	case WMI_STOP_TX_DMA_CMDID:
		return "WMI_STOP_TX_DMA_CMDID";
	case WMI_ABORT_TX_DMA_CMDID:
		return "WMI_ABORT_TX_DMA_CMDID";
	case WMI_DRAIN_TXQ_CMDID:
		return "WMI_DRAIN_TXQ_CMDID";
	case WMI_DRAIN_TXQ_ALL_CMDID:
		return "WMI_DRAIN_TXQ_ALL_CMDID";
	case WMI_START_RECV_CMDID:
		return "WMI_START_RECV_CMDID";
	case WMI_STOP_RECV_CMDID:
		return "WMI_STOP_RECV_CMDID";
	case WMI_FLUSH_RECV_CMDID:
		return "WMI_FLUSH_RECV_CMDID";
	case WMI_SET_MODE_CMDID:
		return "WMI_SET_MODE_CMDID";
	case WMI_NODE_CREATE_CMDID:
		return "WMI_NODE_CREATE_CMDID";
	case WMI_NODE_REMOVE_CMDID:
		return "WMI_NODE_REMOVE_CMDID";
	case WMI_VAP_REMOVE_CMDID:
		return "WMI_VAP_REMOVE_CMDID";
	case WMI_VAP_CREATE_CMDID:
		return "WMI_VAP_CREATE_CMDID";
	case WMI_REG_READ_CMDID:
		return "WMI_REG_READ_CMDID";
	case WMI_REG_WRITE_CMDID:
		return "WMI_REG_WRITE_CMDID";
	case WMI_REG_RMW_CMDID:
		return "WMI_REG_RMW_CMDID";
	case WMI_RC_STATE_CHANGE_CMDID:
		return "WMI_RC_STATE_CHANGE_CMDID";
	case WMI_RC_RATE_UPDATE_CMDID:
		return "WMI_RC_RATE_UPDATE_CMDID";
	case WMI_TARGET_IC_UPDATE_CMDID:
		return "WMI_TARGET_IC_UPDATE_CMDID";
	case WMI_TX_AGGR_ENABLE_CMDID:
		return "WMI_TX_AGGR_ENABLE_CMDID";
	case WMI_TGT_DETACH_CMDID:
		return "WMI_TGT_DETACH_CMDID";
	case WMI_NODE_UPDATE_CMDID:
		return "WMI_NODE_UPDATE_CMDID";
	case WMI_INT_STATS_CMDID:
		return "WMI_INT_STATS_CMDID";
	case WMI_TX_STATS_CMDID:
		return "WMI_TX_STATS_CMDID";
	case WMI_RX_STATS_CMDID:
		return "WMI_RX_STATS_CMDID";
	case WMI_BITRATE_MASK_CMDID:
		return "WMI_BITRATE_MASK_CMDID";
	case WMI_CUSTOM_PING_CMDID:
		return "WMI_CUSTOM_PING_CMDID";
	case WMI_CUSTOM_RC_PROFILE_CMDID:
		return "WMI_CUSTOM_RC_PROFILE_CMDID";
	case WMI_CUSTOM_TXPOWER_CMDID:
		return "WMI_CUSTOM_TXPOWER_CMDID";
	case WMI_CUSTOM_RATE_USAGE_CMDID:
		return "WMI_CUSTOM_RATE_USAGE_CMDID";
	case WMI_CUSTOM_ANI_AUTO_CMDID:
		return "WMI_CUSTOM_ANI_AUTO_CMDID";
	case WMI_CUSTOM_RSSI_HIST_CMDID:
		return "WMI_CUSTOM_RSSI_HIST_CMDID";
	case WMI_CUSTOM_MACFILTER_CMDID:
		return "WMI_CUSTOM_MACFILTER_CMDID";
	case WMI_CUSTOM_BAW_CMDID:
		return "WMI_CUSTOM_BAW_CMDID";
	case WMI_CUSTOM_LONGDIST_CMDID:
		return "WMI_CUSTOM_LONGDIST_CMDID";
	case WMI_CUSTOM_FREQ_NOTE_CMDID:
		return "WMI_CUSTOM_FREQ_NOTE_CMDID";
	case WMI_CUSTOM_NETFLOW_CMDID:
		return "WMI_CUSTOM_NETFLOW_CMDID";
	case WMI_CUSTOM_CHAN_SURVEY_CMDID:
		return "WMI_CUSTOM_CHAN_SURVEY_CMDID";
	case WMI_CUSTOM_AUTO_SPEC_CMDID:
		return "WMI_CUSTOM_AUTO_SPEC_CMDID";
	case WMI_CUSTOM_BLACKBOX_CMDID:
		return "WMI_CUSTOM_BLACKBOX_CMDID";
	}

	return "Bogus";
}

struct wmi *ath9k_init_wmi(struct ath9k_htc_priv *priv)
{
	struct wmi *wmi;

	wmi = kzalloc(sizeof(struct wmi), GFP_KERNEL);
	if (!wmi)
		return NULL;

	wmi->drv_priv = priv;
	wmi->stopped = false;
	skb_queue_head_init(&wmi->wmi_event_queue);
	spin_lock_init(&wmi->wmi_lock);
	spin_lock_init(&wmi->event_lock);
	mutex_init(&wmi->op_mutex);
	mutex_init(&wmi->multi_write_mutex);
	mutex_init(&wmi->multi_rmw_mutex);
	init_completion(&wmi->cmd_wait);
	INIT_LIST_HEAD(&wmi->pending_tx_events);
	tasklet_setup(&wmi->wmi_event_tasklet, ath9k_wmi_event_tasklet);

	return wmi;
}

void ath9k_stop_wmi(struct ath9k_htc_priv *priv)
{
	struct wmi *wmi = priv->wmi;

	mutex_lock(&wmi->op_mutex);
	wmi->stopped = true;
	mutex_unlock(&wmi->op_mutex);
}

void ath9k_destroy_wmi(struct ath9k_htc_priv *priv)
{
	kfree(priv->wmi);
}

void ath9k_wmi_event_drain(struct ath9k_htc_priv *priv)
{
	unsigned long flags;

	tasklet_kill(&priv->wmi->wmi_event_tasklet);
	spin_lock_irqsave(&priv->wmi->wmi_lock, flags);
	__skb_queue_purge(&priv->wmi->wmi_event_queue);
	spin_unlock_irqrestore(&priv->wmi->wmi_lock, flags);
}

void ath9k_wmi_event_tasklet(struct tasklet_struct *t)
{
	struct wmi *wmi = from_tasklet(wmi, t, wmi_event_tasklet);
	struct ath9k_htc_priv *priv = wmi->drv_priv;
	struct wmi_cmd_hdr *hdr;
	void *wmi_event;
	struct wmi_event_swba *swba;
	struct sk_buff *skb = NULL;
	unsigned long flags;
	u16 cmd_id;

	do {
		spin_lock_irqsave(&wmi->wmi_lock, flags);
		skb = __skb_dequeue(&wmi->wmi_event_queue);
		if (!skb) {
			spin_unlock_irqrestore(&wmi->wmi_lock, flags);
			return;
		}
		spin_unlock_irqrestore(&wmi->wmi_lock, flags);

		/* Check if ath9k_htc_probe_device() completed. */
		if (!data_race(priv->initialized)) {
			kfree_skb(skb);
			continue;
		}

		hdr = (struct wmi_cmd_hdr *) skb->data;
		cmd_id = be16_to_cpu(hdr->command_id);
		wmi_event = skb_pull(skb, sizeof(struct wmi_cmd_hdr));

		switch (cmd_id) {
		case WMI_SWBA_EVENTID:
			swba = wmi_event;
			ath9k_htc_swba(priv, swba);
			break;
		case WMI_FATAL_EVENTID:
			ieee80211_queue_work(wmi->drv_priv->hw,
					     &wmi->drv_priv->fatal_work);
			break;
		case WMI_TXSTATUS_EVENTID:
			spin_lock_bh(&priv->tx.tx_lock);
			if (priv->tx.flags & ATH9K_HTC_OP_TX_DRAIN) {
				spin_unlock_bh(&priv->tx.tx_lock);
				break;
			}
			spin_unlock_bh(&priv->tx.tx_lock);

			ath9k_htc_txstatus(priv, wmi_event);
			break;
		case WMI_CUSTOM_TELEMETRY_EVENTID: {
			/* CUSTOM BUILD v2: store the latest target telemetry */
			struct wmi_custom_telemetry_evt *tlm = wmi_event;

			priv->debug.tlm_seq = be32_to_cpu(tlm->seq);
			priv->debug.tlm_uptime_s = be32_to_cpu(tlm->uptime_s);
			priv->debug.tlm_freq_mhz = be32_to_cpu(tlm->freq_mhz);
			priv->debug.tlm_nf_dbm = be32_to_cpu(tlm->nf_dbm);
			priv->debug.tlm_rx = be32_to_cpu(tlm->rx_frames);
			priv->debug.tlm_phyerr = be32_to_cpu(tlm->phy_errs);
			priv->debug.tlm_ani_level = be32_to_cpu(tlm->ani_level);
			priv->debug.dbg_last_cmd = be32_to_cpu(tlm->dbg_last_cmd);
			priv->debug.dbg_tbl_entries = be32_to_cpu(tlm->dbg_tbl_entries);
			priv->debug.beacons_window = be32_to_cpu(tlm->beacons_window);
			priv->debug.mf_dropped = be32_to_cpu(tlm->filter_dropped);
			priv->debug.tlm_count++;
			break;
		}
		default:
			break;
		}

		kfree_skb(skb);
	} while (1);
}

void ath9k_fatal_work(struct work_struct *work)
{
	struct ath9k_htc_priv *priv = container_of(work, struct ath9k_htc_priv,
						   fatal_work);
	struct ath_common *common = ath9k_hw_common(priv->ah);

	ath_dbg(common, FATAL, "FATAL Event received, resetting device\n");
	ath9k_htc_reset(priv);
}

static void ath9k_wmi_rsp_callback(struct wmi *wmi, struct sk_buff *skb)
{
	skb_pull(skb, sizeof(struct wmi_cmd_hdr));

	if (wmi->cmd_rsp_buf != NULL && wmi->cmd_rsp_len != 0)
		memcpy(wmi->cmd_rsp_buf, skb->data, wmi->cmd_rsp_len);

	complete(&wmi->cmd_wait);
}

static void ath9k_wmi_ctrl_rx(void *priv, struct sk_buff *skb,
			      enum htc_endpoint_id epid)
{
	struct wmi *wmi = priv;
	struct wmi_cmd_hdr *hdr;
	unsigned long flags;
	u16 cmd_id;

	if (unlikely(wmi->stopped))
		goto free_skb;

	/* Validate the obtained SKB. */
	if (unlikely(skb->len < sizeof(struct wmi_cmd_hdr)))
		goto free_skb;

	hdr = (struct wmi_cmd_hdr *) skb->data;
	cmd_id = be16_to_cpu(hdr->command_id);

	if (cmd_id & 0x1000) {
		spin_lock_irqsave(&wmi->wmi_lock, flags);
		__skb_queue_tail(&wmi->wmi_event_queue, skb);
		spin_unlock_irqrestore(&wmi->wmi_lock, flags);
		tasklet_schedule(&wmi->wmi_event_tasklet);
		return;
	}

	/* Check if there has been a timeout. */
	spin_lock_irqsave(&wmi->wmi_lock, flags);
	if (be16_to_cpu(hdr->seq_no) != wmi->last_seq_id) {
		spin_unlock_irqrestore(&wmi->wmi_lock, flags);
		goto free_skb;
	}

	/* WMI command response */
	ath9k_wmi_rsp_callback(wmi, skb);
	spin_unlock_irqrestore(&wmi->wmi_lock, flags);

free_skb:
	kfree_skb(skb);
}

static void ath9k_wmi_ctrl_tx(void *priv, struct sk_buff *skb,
			      enum htc_endpoint_id epid, bool txok)
{
	kfree_skb(skb);
}

int ath9k_wmi_connect(struct htc_target *htc, struct wmi *wmi,
		      enum htc_endpoint_id *wmi_ctrl_epid)
{
	struct htc_service_connreq connect;
	int ret;

	wmi->htc = htc;

	memset(&connect, 0, sizeof(connect));

	connect.ep_callbacks.priv = wmi;
	connect.ep_callbacks.tx = ath9k_wmi_ctrl_tx;
	connect.ep_callbacks.rx = ath9k_wmi_ctrl_rx;
	connect.service_id = WMI_CONTROL_SVC;

	ret = htc_connect_service(htc, &connect, &wmi->ctrl_epid);
	if (ret)
		return ret;

	*wmi_ctrl_epid = wmi->ctrl_epid;

	return 0;
}

static int ath9k_wmi_cmd_issue(struct wmi *wmi,
			       struct sk_buff *skb,
			       enum wmi_cmd_id cmd, u16 len,
			       u8 *rsp_buf, u32 rsp_len)
{
	struct wmi_cmd_hdr *hdr;
	unsigned long flags;

	hdr = skb_push(skb, sizeof(struct wmi_cmd_hdr));
	hdr->command_id = cpu_to_be16(cmd);
	hdr->seq_no = cpu_to_be16(++wmi->tx_seq_id);

	spin_lock_irqsave(&wmi->wmi_lock, flags);

	/* record the rsp buffer and length */
	wmi->cmd_rsp_buf = rsp_buf;
	wmi->cmd_rsp_len = rsp_len;

	wmi->last_seq_id = wmi->tx_seq_id;
	spin_unlock_irqrestore(&wmi->wmi_lock, flags);

	return htc_send_epid(wmi->htc, skb, wmi->ctrl_epid);
}

int ath9k_wmi_cmd(struct wmi *wmi, enum wmi_cmd_id cmd_id,
		  u8 *cmd_buf, u32 cmd_len,
		  u8 *rsp_buf, u32 rsp_len,
		  u32 timeout)
{
	struct ath_hw *ah = wmi->drv_priv->ah;
	struct ath_common *common = ath9k_hw_common(ah);
	u16 headroom = sizeof(struct htc_frame_hdr) +
		       sizeof(struct wmi_cmd_hdr);
	unsigned long time_left, flags;
	struct sk_buff *skb;
	int ret = 0;

	if (ah->ah_flags & AH_UNPLUGGED)
		return 0;

	skb = alloc_skb(headroom + cmd_len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb_reserve(skb, headroom);

	if (cmd_len != 0 && cmd_buf != NULL) {
		skb_put_data(skb, cmd_buf, cmd_len);
	}

	mutex_lock(&wmi->op_mutex);

	/* check if wmi stopped flag is set */
	if (unlikely(wmi->stopped)) {
		ret = -EPROTO;
		goto out;
	}

	ret = ath9k_wmi_cmd_issue(wmi, skb, cmd_id, cmd_len, rsp_buf, rsp_len);
	if (ret)
		goto out;

	time_left = wait_for_completion_timeout(&wmi->cmd_wait, timeout);
	if (!time_left) {
		ath_dbg(common, WMI, "Timeout waiting for WMI command: %s\n",
			wmi_cmd_to_name(cmd_id));
		spin_lock_irqsave(&wmi->wmi_lock, flags);
		wmi->last_seq_id = 0;
		wmi->cmd_timeouts++;   /* CUSTOM BUILD v3-diag */
		/* CUSTOM BUILD v4-diag: record exactly what failed last */
		wmi->last_fail_cmd_id = cmd_id;
		wmi->last_fail_seq = wmi->tx_seq_id;
		wmi->last_fail_errno = -ETIMEDOUT;
		spin_unlock_irqrestore(&wmi->wmi_lock, flags);
		mutex_unlock(&wmi->op_mutex);
		return -ETIMEDOUT;
	}

	/* CUSTOM BUILD v4-diag: last successful reply timestamp */
	spin_lock_irqsave(&wmi->wmi_lock, flags);
	wmi->last_rsp_jiffies = jiffies;
	wmi->rsp_count++;
	spin_unlock_irqrestore(&wmi->wmi_lock, flags);

	mutex_unlock(&wmi->op_mutex);

	return 0;

out:
	ath_dbg(common, WMI, "WMI failure for: %s\n", wmi_cmd_to_name(cmd_id));
	mutex_unlock(&wmi->op_mutex);
	kfree_skb(skb);

	return ret;
}

/* CUSTOM BUILD: send WMI_CUSTOM_PING_CMDID to the target and collect the
 * response (echoed payload + target-side pong counter + v3-diag control
 * path counters). diag may be NULL; otherwise diag[0..3] receive:
 * rx_cmds, send_dones, rsp_entries, rsp_allocfail. */
int ath9k_wmi_custom_ping(struct wmi *wmi, u32 payload,
			  u32 *echo_payload, u32 *pong_counter,
			  u32 diag[4])
{
	struct wmi_custom_ping_cmd cmd = { .payload = cpu_to_be32(payload) };
	struct wmi_custom_ping_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_PING_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*echo_payload = be32_to_cpu(rsp.echo_payload);
	*pong_counter = be32_to_cpu(rsp.pong_counter);
	if (diag) {
		diag[0] = be32_to_cpu(rsp.diag_rx_cmds);
		diag[1] = be32_to_cpu(rsp.diag_send_dones);
		diag[2] = be32_to_cpu(rsp.diag_rsp_entries);
		diag[3] = be32_to_cpu(rsp.diag_rsp_allocfail);
		diag[4] = be32_to_cpu(rsp.diag_pool_memfail);
		diag[5] = be32_to_cpu(rsp.diag_evt_entries);
	}
	return 0;
}

/* CUSTOM BUILD: set/query the target's runtime rate-control profile.
 * step_dn_inc 1 = clean-air, 2 = noisy-air, 0 = query only.
 * fixed_rate_idx 0xFF = auto RC, 0x00-0x7F = fixed, else = keep current. */
int ath9k_wmi_rc_profile(struct wmi *wmi, u32 step_dn_inc, u32 fixed_rate_idx,
			 u32 *eff_step_dn, u32 *eff_fixed_rate)
{
	struct wmi_custom_rc_profile_cmd cmd = {
		.step_dn_inc = cpu_to_be32(step_dn_inc),
		.fixed_rate_idx = cpu_to_be32(fixed_rate_idx),
	};
	struct wmi_custom_rc_profile_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_RC_PROFILE_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*eff_step_dn = be32_to_cpu(rsp.step_dn_inc);
	*eff_fixed_rate = be32_to_cpu(rsp.fixed_rate_idx);
	return 0;
}

/* CUSTOM BUILD: set/query the per-frame TX power override (0..63). */
int ath9k_wmi_txpower(struct wmi *wmi, u32 tx_power, u32 *effective)
{
	struct wmi_custom_txpower_cmd cmd = {
		.tx_power = cpu_to_be32(tx_power)
	};
	struct wmi_custom_txpower_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_TXPOWER_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*effective = be32_to_cpu(rsp.tx_power);
	return 0;
}

/* CUSTOM BUILD v4: thermal governor control (mode/cap 0xFF = query-only). */
int ath9k_wmi_thermal(struct wmi *wmi, u32 mode, u32 cap, u32 flags,
		      struct wmi_custom_thermal_rsp *out)
{
	struct wmi_custom_thermal_cmd cmd = {
		.mode = cpu_to_be32(mode),
		.cap = cpu_to_be32(cap),
		.flags = cpu_to_be32(flags)
	};
	struct wmi_custom_thermal_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_THERMAL_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*out = rsp;
	return 0;
}

/* CUSTOM BUILD v2: per-rate TX usage histogram (reset=1 clears target counters). */
int ath9k_wmi_rate_usage(struct wmi *wmi, u32 reset, u8 usage[32])
{
	struct wmi_custom_rate_usage_cmd cmd = { .reset = cpu_to_be32(reset) };
	struct wmi_custom_rate_usage_rsp rsp;
	int ret, i;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_RATE_USAGE_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	for (i = 0; i < 32; i++)
		usage[i] = rsp.usage[i];
	return 0;
}

/* CUSTOM BUILD v2: firmware-side adaptive ANI control.
 * enable 0xFF = query, 0 = manual (host controls), 1 = firmware adaptive. */
int ath9k_wmi_ani_auto(struct wmi *wmi, u32 enable,
		       u32 *state, u32 *level, u32 *errs)
{
	struct wmi_custom_ani_auto_cmd cmd = { .enable = cpu_to_be32(enable) };
	struct wmi_custom_ani_auto_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_ANI_AUTO_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*state = be32_to_cpu(rsp.enable);
	*level = be32_to_cpu(rsp.level);
	*errs = be32_to_cpu(rsp.phy_errs);
	return 0;
}

/* CUSTOM BUILD v2: RSSI histogram (64 bins); reset=1 clears target counters. */
int ath9k_wmi_rssi_hist(struct wmi *wmi, u32 reset, u8 bins[48])
{
	struct wmi_custom_rssi_hist_cmd cmd = { .reset = cpu_to_be32(reset) };
	struct wmi_custom_rssi_hist_rsp rsp;
	int ret, i;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_RSSI_HIST_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	for (i = 0; i < 48; i++)
		bins[i] = rsp.bins[i];
	return 0;
}

/* CUSTOM BUILD v2: radio-level MAC filter configuration. */
int ath9k_wmi_macfilter(struct wmi *wmi, u32 action, u32 mode, u32 index,
			const u8 *mac, u32 *mode_out, u32 *count_out)
{
	struct wmi_custom_macfilter_cmd cmd = {
		.action = cpu_to_be32(action),
		.mode = cpu_to_be32(mode),
		.index = cpu_to_be32(index),
	};
	struct wmi_custom_macfilter_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	if (mac)
		memcpy(cmd.mac, mac, 6);

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_MACFILTER_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*mode_out = be32_to_cpu(rsp.mode);
	*count_out = be32_to_cpu(rsp.count);
	return 0;
}

/* ===== CUSTOM BUILD v3 ===== */

int ath9k_wmi_rc_profile_v3(struct wmi *wmi, u32 step, u32 fixed, u32 tries,
			    u32 rates, u32 *eff_step, u32 *eff_fixed,
			    u32 *eff_tries, u32 *eff_rates)
{
	struct wmi_custom_rc_profile_cmd cmd = {
		.step_dn_inc = cpu_to_be32(step),
		.fixed_rate_idx = cpu_to_be32(fixed),
		.num_tries = cpu_to_be32(tries),
		.num_rates = cpu_to_be32(rates),
	};
	struct wmi_custom_rc_profile_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_RC_PROFILE_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*eff_step = be32_to_cpu(rsp.step_dn_inc);
	*eff_fixed = be32_to_cpu(rsp.fixed_rate_idx);
	*eff_tries = be32_to_cpu(rsp.num_tries);
	*eff_rates = be32_to_cpu(rsp.num_rates);
	return 0;
}

int ath9k_wmi_baw(struct wmi *wmi, u32 size, u32 *effective)
{
	struct wmi_custom_baw_cmd cmd = { .baw_size = cpu_to_be32(size) };
	struct wmi_custom_baw_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_BAW_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*effective = be32_to_cpu(rsp.effective);
	return 0;
}

int ath9k_wmi_longdist(struct wmi *wmi, u32 us, u32 *cur_out)
{
	struct wmi_custom_longdist_cmd cmd = { .ack_timeout_us = cpu_to_be32(us) };
	struct wmi_custom_longdist_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_LONGDIST_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*cur_out = be32_to_cpu(rsp.ack_timeout_us);
	return 0;
}

int ath9k_wmi_freq_note(struct wmi *wmi, u32 freq_mhz, u32 *entries)
{
	struct wmi_custom_freq_note_cmd cmd = { .freq_mhz = cpu_to_be32(freq_mhz) };
	struct wmi_custom_freq_note_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_FREQ_NOTE_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ / 10);
	if (ret)
		return ret;

	*entries = be32_to_cpu(rsp.entries);
	return 0;
}

int ath9k_wmi_netflow(struct wmi *wmi, u32 reset, u8 *count,
		      u8 macs[WMI_CUSTOM_NETFLOW_MAX][6],
		      u16 frames[WMI_CUSTOM_NETFLOW_MAX],
		      u32 bytes[WMI_CUSTOM_NETFLOW_MAX])
{
	struct wmi_custom_netflow_cmd cmd = { .reset = cpu_to_be32(reset) };
	struct wmi_custom_netflow_rsp rsp;
	int ret, i;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_NETFLOW_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*count = rsp.count;
	for (i = 0; i < WMI_CUSTOM_NETFLOW_MAX; i++) {
		memcpy(macs[i], rsp.ent[i].mac, 6);
		frames[i] = be16_to_cpu(rsp.ent[i].frames);
		bytes[i] = be16_to_cpu(rsp.ent[i].bytes_256) << 8;   /* v4-fix: 256B units */
	}
	return 0;
}

int ath9k_wmi_chan_survey(struct wmi *wmi, u8 *count,
			  u16 freq[12], s16 nf[12])
{
	struct wmi_custom_chan_survey_rsp rsp;
	int ret, i;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_CHAN_SURVEY_CMDID,
			    NULL, 0,
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*count = rsp.count;
	for (i = 0; i < 12; i++) {
		freq[i] = be16_to_cpu(rsp.ent[i].freq_mhz);
		nf[i] = rsp.ent[i].nf_dbm;   /* v4-fix: s8 now */
	}
	return 0;
}

int ath9k_wmi_auto_spec(struct wmi *wmi, u32 enable, u32 threshold,
			u32 *state, u32 *th_out, u32 *active)
{
	struct wmi_custom_auto_spec_cmd cmd = {
		.enable = cpu_to_be32(enable),
		.threshold = cpu_to_be32(threshold),
	};
	struct wmi_custom_auto_spec_rsp rsp;
	int ret;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_AUTO_SPEC_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*state = be32_to_cpu(rsp.enable);
	*th_out = be32_to_cpu(rsp.threshold);
	*active = be32_to_cpu(rsp.active);
	return 0;
}

int ath9k_wmi_blackbox(struct wmi *wmi, u32 page,
		       u8 *head, u8 *count, u32 ent[4])
{
	struct wmi_custom_blackbox_cmd cmd = { .page = cpu_to_be32(page) };
	struct wmi_custom_blackbox_rsp rsp;
	int ret, i;

	memset(&rsp, 0, sizeof(rsp));

	ret = ath9k_wmi_cmd(wmi, WMI_CUSTOM_BLACKBOX_CMDID,
			    (u8 *)&cmd, sizeof(cmd),
			    (u8 *)&rsp, sizeof(rsp), HZ * 2);
	if (ret)
		return ret;

	*head = rsp.head;
	*count = rsp.count;
	for (i = 0; i < 4; i++)
		ent[i] = be32_to_cpu(rsp.ent[i]);
	return 0;
}

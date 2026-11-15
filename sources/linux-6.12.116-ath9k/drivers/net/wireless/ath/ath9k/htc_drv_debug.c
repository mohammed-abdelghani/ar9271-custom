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
#include "ani.h"
#include "ar9002_phy.h" /* CUSTOM BUILD: ANI-style PHY registers (AR_PHY_FIND_SIG*) */

static ssize_t read_file_tgt_int_stats(struct file *file, char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct ath9k_htc_target_int_stats cmd_rsp;
	char buf[512];
	unsigned int len = 0;
	int ret = 0;

	memset(&cmd_rsp, 0, sizeof(cmd_rsp));

	ath9k_htc_ps_wakeup(priv);

	WMI_CMD(WMI_INT_STATS_CMDID);
	if (ret) {
		ath9k_htc_ps_restore(priv);
		return -EINVAL;
	}

	ath9k_htc_ps_restore(priv);

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "RX",
			 be32_to_cpu(cmd_rsp.rx));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "RXORN",
			 be32_to_cpu(cmd_rsp.rxorn));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "RXEOL",
			 be32_to_cpu(cmd_rsp.rxeol));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "TXURN",
			 be32_to_cpu(cmd_rsp.txurn));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "TXTO",
			 be32_to_cpu(cmd_rsp.txto));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "CST",
			 be32_to_cpu(cmd_rsp.cst));

	if (len > sizeof(buf))
		len = sizeof(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_tgt_int_stats = {
	.read = read_file_tgt_int_stats,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_tgt_tx_stats(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct ath9k_htc_target_tx_stats cmd_rsp;
	char buf[512];
	unsigned int len = 0;
	int ret = 0;

	memset(&cmd_rsp, 0, sizeof(cmd_rsp));

	ath9k_htc_ps_wakeup(priv);

	WMI_CMD(WMI_TX_STATS_CMDID);
	if (ret) {
		ath9k_htc_ps_restore(priv);
		return -EINVAL;
	}

	ath9k_htc_ps_restore(priv);

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "Xretries",
			 be32_to_cpu(cmd_rsp.xretries));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "FifoErr",
			 be32_to_cpu(cmd_rsp.fifoerr));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "Filtered",
			 be32_to_cpu(cmd_rsp.filtered));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "TimerExp",
			 be32_to_cpu(cmd_rsp.timer_exp));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "ShortRetries",
			 be32_to_cpu(cmd_rsp.shortretries));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "LongRetries",
			 be32_to_cpu(cmd_rsp.longretries));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "QueueNull",
			 be32_to_cpu(cmd_rsp.qnull));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "EncapFail",
			 be32_to_cpu(cmd_rsp.encap_fail));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "NoBuf",
			 be32_to_cpu(cmd_rsp.nobuf));

	if (len > sizeof(buf))
		len = sizeof(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_tgt_tx_stats = {
	.read = read_file_tgt_tx_stats,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_tgt_rx_stats(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct ath9k_htc_target_rx_stats cmd_rsp;
	char buf[512];
	unsigned int len = 0;
	int ret = 0;

	memset(&cmd_rsp, 0, sizeof(cmd_rsp));

	ath9k_htc_ps_wakeup(priv);

	WMI_CMD(WMI_RX_STATS_CMDID);
	if (ret) {
		ath9k_htc_ps_restore(priv);
		return -EINVAL;
	}

	ath9k_htc_ps_restore(priv);

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "NoBuf",
			 be32_to_cpu(cmd_rsp.nobuf));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "HostSend",
			 be32_to_cpu(cmd_rsp.host_send));

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "HostDone",
			 be32_to_cpu(cmd_rsp.host_done));

	if (len > sizeof(buf))
		len = sizeof(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_tgt_rx_stats = {
	.read = read_file_tgt_rx_stats,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_xmit(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len = 0;

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "Buffers queued",
			 priv->debug.tx_stats.buf_queued);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "Buffers completed",
			 priv->debug.tx_stats.buf_completed);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "SKBs queued",
			 priv->debug.tx_stats.skb_queued);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "SKBs success",
			 priv->debug.tx_stats.skb_success);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "SKBs failed",
			 priv->debug.tx_stats.skb_failed);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "CAB queued",
			 priv->debug.tx_stats.cab_queued);

	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "BE queued",
			 priv->debug.tx_stats.queue_stats[IEEE80211_AC_BE]);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "BK queued",
			 priv->debug.tx_stats.queue_stats[IEEE80211_AC_BK]);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "VI queued",
			 priv->debug.tx_stats.queue_stats[IEEE80211_AC_VI]);
	len += scnprintf(buf + len, sizeof(buf) - len,
			 "%20s : %10u\n", "VO queued",
			 priv->debug.tx_stats.queue_stats[IEEE80211_AC_VO]);

	if (len > sizeof(buf))
		len = sizeof(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_xmit = {
	.read = read_file_xmit,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

void ath9k_htc_err_stat_rx(struct ath9k_htc_priv *priv,
			     struct ath_rx_status *rs)
{
	ath9k_cmn_debug_stat_rx(&priv->debug.rx_stats, rs);
}

static ssize_t read_file_skb_rx(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char *buf;
	unsigned int len = 0, size = 1500;
	ssize_t retval = 0;

	buf = kzalloc(size, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	len += scnprintf(buf + len, size - len,
			 "%20s : %10u\n", "SKBs allocated",
			 priv->debug.skbrx_stats.skb_allocated);
	len += scnprintf(buf + len, size - len,
			 "%20s : %10u\n", "SKBs completed",
			 priv->debug.skbrx_stats.skb_completed);
	len += scnprintf(buf + len, size - len,
			 "%20s : %10u\n", "SKBs Dropped",
			 priv->debug.skbrx_stats.skb_dropped);

	if (len > size)
		len = size;

	retval = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	kfree(buf);

	return retval;
}

static const struct file_operations fops_skb_rx = {
	.read = read_file_skb_rx,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_slot(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len;

	spin_lock_bh(&priv->tx.tx_lock);
	len = scnprintf(buf, sizeof(buf),
			"TX slot bitmap : %*pb\n"
			"Used slots     : %d\n",
			MAX_TX_BUF_NUM, priv->tx.tx_slot,
			bitmap_weight(priv->tx.tx_slot, MAX_TX_BUF_NUM));
	spin_unlock_bh(&priv->tx.tx_lock);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_slot = {
	.read = read_file_slot,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_queue(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len = 0;

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Mgmt endpoint", skb_queue_len(&priv->tx.mgmt_ep_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Cab endpoint", skb_queue_len(&priv->tx.cab_ep_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Data BE endpoint", skb_queue_len(&priv->tx.data_be_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Data BK endpoint", skb_queue_len(&priv->tx.data_bk_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Data VI endpoint", skb_queue_len(&priv->tx.data_vi_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Data VO endpoint", skb_queue_len(&priv->tx.data_vo_queue));

	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Failed queue", skb_queue_len(&priv->tx.tx_failed));

	spin_lock_bh(&priv->tx.tx_lock);
	len += scnprintf(buf + len, sizeof(buf) - len, "%20s : %10u\n",
			 "Queued count", priv->tx.queued_cnt);
	spin_unlock_bh(&priv->tx.tx_lock);

	if (len > sizeof(buf))
		len = sizeof(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);

}

static const struct file_operations fops_queue = {
	.read = read_file_queue,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_debug(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct ath_common *common = ath9k_hw_common(priv->ah);
	char buf[32];
	unsigned int len;

	len = sprintf(buf, "0x%08x\n", common->debug_mask);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_debug(struct file *file, const char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct ath_common *common = ath9k_hw_common(priv->ah);
	unsigned long mask;
	ssize_t ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &mask);
	if (ret)
		return ret;

	common->debug_mask = mask;
	return count;
}

static const struct file_operations fops_debug = {
	.read = read_file_debug,
	.write = write_file_debug,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD v3-diag: host-side command-path health. Purely local (no WMI
 * round trip): tx_seq_id/last_seq_id show whether the host kept ISSUING
 * commands after the target stopped replying, cmd_timeouts counts every
 * "Connection timed out". Frozen seq ids + growing timeouts = the host's
 * send path itself is stuck; growing seq ids = the host sends fine and the
 * loss is between the host USB and the target's WMI dispatcher. */
static ssize_t read_file_wmi_diag(struct file *file, char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	struct wmi *wmi = priv->wmi;
	char buf[320];
	unsigned int len;
	unsigned long flags;
	u16 tx_seq, last_seq, fail_cmd, fail_seq;
	u32 timeouts, rsp_count;
	int fail_errno;
	unsigned long last_rsp;

	spin_lock_irqsave(&wmi->wmi_lock, flags);
	tx_seq = wmi->tx_seq_id;
	last_seq = wmi->last_seq_id;
	timeouts = wmi->cmd_timeouts;
	fail_cmd = wmi->last_fail_cmd_id;
	fail_seq = wmi->last_fail_seq;
	fail_errno = wmi->last_fail_errno;
	last_rsp = wmi->last_rsp_jiffies;
	rsp_count = wmi->rsp_count;
	spin_unlock_irqrestore(&wmi->wmi_lock, flags);

	len = sprintf(buf, "tx_seq_id=%u last_seq_id=%u cmd_timeouts=%u rsp_count=%u\n",
		      tx_seq, last_seq, timeouts, rsp_count);
	if (fail_cmd)
		len += sprintf(buf + len,
			       "last_fail: cmd=0x%04x seq=%u errno=%d\n",
			       fail_cmd, fail_seq, fail_errno);
	else
		len += sprintf(buf + len, "last_fail: none\n");
	if (last_rsp)
		len += sprintf(buf + len,
			       "last_ok_rsp: %ums ago\n",
			       jiffies_to_msecs(jiffies - last_rsp));

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_wmi_diag = {
	.read = read_file_wmi_diag,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: custom WMI ping round trip.
 * Write an integer payload (e.g. "1234") to send WMI_CUSTOM_PING_CMDID to
 * the target firmware; read back the last result (echo + target counter). */
static ssize_t read_file_custom_ping(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[160];
	unsigned int len;

	len = sprintf(buf,
		      "pings=%lu last_payload=0x%08x last_pong=%u last_ms=%lu\n"
		      "diag: rx_cmds=%u send_dones=%u rsp_entries=%u rsp_allocfail=%u pool_memfail=%u evt_entries=%u\n",
		      priv->debug.custom_ping_count,
		      priv->debug.custom_ping_payload,
		      priv->debug.custom_ping_pong,
		      priv->debug.custom_ping_ms,
		      priv->debug.diag_rx_cmds,
		      priv->debug.diag_send_dones,
		      priv->debug.diag_rsp_entries,
		      priv->debug.diag_rsp_allocfail,
		      priv->debug.diag_pool_memfail,
		      priv->debug.diag_evt_entries);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_custom_ping(struct file *file,
				      const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long payload;
	u32 echo, pong;
	u32 diag[6] = {0, 0, 0, 0, 0, 0};
	unsigned long t0, t1;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &payload);
	if (ret)
		return ret;

	t0 = jiffies;
	ret = ath9k_wmi_custom_ping(priv->wmi, (u32)payload, &echo, &pong, diag);
	t1 = jiffies;
	if (ret)
		return ret;

	priv->debug.custom_ping_count++;
	priv->debug.custom_ping_payload = echo;
	priv->debug.custom_ping_pong = pong;
	priv->debug.custom_ping_ms = jiffies_to_msecs(t1 - t0);
	priv->debug.diag_rx_cmds       = diag[0];
	priv->debug.diag_send_dones    = diag[1];
	priv->debug.diag_rsp_entries   = diag[2];
	priv->debug.diag_rsp_allocfail = diag[3];
	priv->debug.diag_pool_memfail  = diag[4];
	priv->debug.diag_evt_entries   = diag[5];

	return count;
}

static const struct file_operations fops_custom_ping = {
	.read = read_file_custom_ping,
	.write = write_file_custom_ping,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: runtime rate-control profile (WMI_CUSTOM_RC_PROFILE_CMDID).
 * Write "1" (clean air) or "2" (noisy air); "0" just queries the target.
 * Read shows the effective value the target answered with. */
static ssize_t read_file_rc_profile(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[128];
	unsigned int len;
	u32 eff_step, eff_fixed;
	int ret;

	/* live query: step_dn_inc=0 keeps it, fixed=0xFE keeps it */
	ret = ath9k_wmi_rc_profile_v3(priv->wmi, 0, 0xFE, 0, 0,
				      &eff_step, &eff_fixed,
				      &priv->debug.rc_num_tries,
				      &priv->debug.rc_num_rates);
	if (ret)
		return ret;

	priv->debug.rc_profile_step_dn = eff_step;
	priv->debug.rc_fixed_rate = eff_fixed;

	len = sprintf(buf,
		      "step_dn_inc=%u fixed_rate=%u (0xff=auto) num_tries=%u num_rates=%u applied=%lu (live)\n",
		      eff_step, eff_fixed, priv->debug.rc_num_tries,
		      priv->debug.rc_num_rates, priv->debug.rc_profile_count);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_rc_profile(struct file *file,
				     const char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 eff_step, eff_fixed;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 2)
		return -EINVAL;

	ret = ath9k_wmi_rc_profile_v3(priv->wmi, (u32)val, 0xFE, 0, 0,
				      &eff_step, &eff_fixed,
				      &priv->debug.rc_num_tries,
				      &priv->debug.rc_num_rates);
	if (ret)
		return ret;

	priv->debug.rc_profile_step_dn = eff_step;
	priv->debug.rc_fixed_rate = eff_fixed;
	priv->debug.rc_profile_count++;

	return count;
}

/* CUSTOM BUILD v3: runtime tries count (1..32; 0 = keep target default).
 * Rates stay at the 4-series default (per-series runtime = future). */
static ssize_t read_file_rc_tries(struct file *file, char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[96];
	unsigned int len;
	u32 eff_step, eff_fixed, eff_tries, eff_rates;
	int ret;

	ret = ath9k_wmi_rc_profile_v3(priv->wmi, 0, 0xFE, 0, 0,
				      &eff_step, &eff_fixed,
				      &eff_tries, &eff_rates);
	if (ret)
		return ret;

	priv->debug.rc_num_tries = eff_tries;
	priv->debug.rc_num_rates = eff_rates;

	len = sprintf(buf, "num_tries=%u num_rates=%u (live)\n",
		      eff_tries, eff_rates);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_rc_tries(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 eff_step, eff_fixed, eff_tries, eff_rates;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 32)
		return -EINVAL;

	ret = ath9k_wmi_rc_profile_v3(priv->wmi, 0, 0xFE, (u32)val, 0,
				      &eff_step, &eff_fixed,
				      &eff_tries, &eff_rates);
	if (ret)
		return ret;

	priv->debug.rc_num_tries = eff_tries;
	priv->debug.rc_num_rates = eff_rates;
	return count;
}

static const struct file_operations fops_rc_tries = {
	.read = read_file_rc_tries,
	.write = write_file_rc_tries,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static const struct file_operations fops_rc_profile = {
	.read = read_file_rc_profile,
	.write = write_file_rc_profile,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: fixed-rate runtime toggle (WMI_CUSTOM_RC_PROFILE_CMDID).
 * Write a rate table index (0..127) to force that rate, or 255 for auto RC.
 * The 11ng rate table indices are typically: 0-3 legacy CCK, 4-11 legacy
 * OFDM (6..54 Mbps), 12-19 MCS0-7. Read back what the target answered. */
static ssize_t read_file_rc_fixed(struct file *file, char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[96];
	unsigned int len;
	u32 eff_step, eff_fixed;
	int ret;

	/* live query from the target */
	ret = ath9k_wmi_rc_profile(priv->wmi, 0, 0xFE, &eff_step, &eff_fixed);
	if (ret)
		return ret;

	priv->debug.rc_profile_step_dn = eff_step;
	priv->debug.rc_fixed_rate = eff_fixed;

	len = sprintf(buf,
		      "fixed_rate=%u (%s) applied=%lu (live)\n",
		      eff_fixed,
		      eff_fixed == 0xff ? "auto" : "fixed",
		      priv->debug.rc_profile_count);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_rc_fixed(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 eff_step, eff_fixed;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 0x7f && val != 0xff)
		return -EINVAL;

	ret = ath9k_wmi_rc_profile_v3(priv->wmi, 0, (u32)val, 0, 0,
				      &eff_step, &eff_fixed,
				      &priv->debug.rc_num_tries,
				      &priv->debug.rc_num_rates);
	if (ret)
		return ret;

	priv->debug.rc_profile_step_dn = eff_step;
	priv->debug.rc_fixed_rate = eff_fixed;
	priv->debug.rc_profile_count++;

	return count;
}

static const struct file_operations fops_rc_fixed = {
	.read = read_file_rc_fixed,
	.write = write_file_rc_fixed,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: per-frame TX power override (WMI_CUSTOM_TXPOWER_CMDID).
 * The stock firmware hardcodes 60 (of 0..63) in the TX descriptor.
 * Write 0..63 to set; anything else just queries. */
static ssize_t read_file_txpower(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[96];
	unsigned int len;
	u32 effective;
	int ret;

	ret = ath9k_wmi_txpower(priv->wmi, 0xFF, &effective);
	if (ret)
		return ret;

	priv->debug.tx_power = effective;

	len = sprintf(buf,
		      "tx_power=%u/63 (live from target)\n",
		      effective);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_txpower(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 effective;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 0x3f)
		return -EINVAL;

	ret = ath9k_wmi_txpower(priv->wmi, (u32)val, &effective);
	if (ret)
		return ret;

	priv->debug.tx_power = effective;

	return count;
}

static const struct file_operations fops_txpower = {
	.read = read_file_txpower,
	.write = write_file_txpower,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* ===== CUSTOM BUILD v4 (thermal) debugfs handlers ===== */
/* Reads are live governor queries; writes accept "mode <0-3>",
 * "cap <0-63|255>" and "hooks <0|1>", or a bare number which sets the
 * mode. cap 0xFF = uncapped; mode bit0 = RSSI-aware cap, bit1 = duty
 * backoff. "hooks" flips the target RX-hook isolation switch at runtime
 * (docs/10 bisect tool). */
static ssize_t read_file_thermal(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[256];
	unsigned int len;
	struct wmi_custom_thermal_rsp rsp;
	int ret;

	ret = ath9k_wmi_thermal(priv->wmi, CUSTOM_TH_QUERY, CUSTOM_TH_CAP_QUERY,
				0, &rsp);
	if (ret)
		return ret;

	len = sprintf(buf,
		      "mode=%u (bit0=rssi-cap bit1=duty-backoff)\n"
		      "cap=%u%s backoff=%u\n"
		      "effective=%u/63 (XmitPower on data frames)\n"
		      "avg_rssi=%u tx_window=%u rx_window=%u\n",
		      be32_to_cpu(rsp.mode), be32_to_cpu(rsp.cap),
		      (be32_to_cpu(rsp.cap) == CUSTOM_TH_CAP_UNCAPPED) ?
			      " (uncapped)" : "",
		      be32_to_cpu(rsp.backoff), be32_to_cpu(rsp.effective),
		      be32_to_cpu(rsp.avg_rssi), be32_to_cpu(rsp.tx_window),
		      be32_to_cpu(rsp.rx_window));
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_thermal(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char kbuf[32];
	u32 mode = CUSTOM_TH_QUERY, cap = CUSTOM_TH_CAP_QUERY, flags = 0;
	u32 a, b;
	struct wmi_custom_thermal_rsp rsp;
	int ret;

	(void)file;

	if (count > sizeof(kbuf) - 1)
		return -EINVAL;
	if (copy_from_user(kbuf, user_buf, count))
		return -EFAULT;
	kbuf[count] = '\0';

	if (sscanf(kbuf, "mode %u", &a) == 1) {
		if (a > 3)
			return -EINVAL;
		mode = a;
	} else if (sscanf(kbuf, "cap %u", &a) == 1) {
		if (a > 0x3f && a != 0xff)
			return -EINVAL;
		cap = a;
	} else if (sscanf(kbuf, "hooks %u", &a) == 1) {
		if (a > 1)
			return -EINVAL;
		flags = a ? CUSTOM_TH_FLAG_HOOKS_ON : CUSTOM_TH_FLAG_HOOKS_OFF;
	} else if (sscanf(kbuf, "%u %u", &a, &b) == 2) {
		if (a > 3 || (b > 0x3f && b != 0xff))
			return -EINVAL;
		mode = a;
		cap = b;
	} else if (sscanf(kbuf, "%u", &a) == 1) {
		if (a > 3)
			return -EINVAL;
		mode = a;
	} else {
		return -EINVAL;
	}

	ret = ath9k_wmi_thermal(priv->wmi, mode, cap, flags, &rsp);
	if (ret)
		return ret;

	return count;
}

static const struct file_operations fops_thermal = {
	.read = read_file_thermal,
	.write = write_file_thermal,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* ===== CUSTOM BUILD v3 debugfs handlers ===== */

static ssize_t read_file_baw(struct file *file, char __user *user_buf,
			     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[96];
	unsigned int len;
	u32 eff;
	int ret;

	ret = ath9k_wmi_baw(priv->wmi, 0xFF, &eff);
	if (ret)
		return ret;
	priv->debug.baw_size = eff;

	len = sprintf(buf, "baw_size=%u (Block-ACK window, power of two)\n", eff);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_baw(struct file *file, const char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 eff;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val < 1 || val > 64)
		return -EINVAL;

	ret = ath9k_wmi_baw(priv->wmi, (u32)val, &eff);
	if (ret)
		return ret;

	priv->debug.baw_size = eff;
	return count;
}

static const struct file_operations fops_baw = {
	.read = read_file_baw,
	.write = write_file_baw,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_ack_timeout(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[96];
	unsigned int len;
	u32 cur;
	int ret;

	/* CUSTOM BUILD v4-fix: the query value MUST be out of the firmware's
	 * write range (1..0x3FFF). The old query value 0xFF (=255) IS inside
	 * the range, so every read RE-WROTE AR_TIME_OUT=255 over any value a
	 * user had set - reads were destructive and the readback always
	 * showed 255 (fw handler writes any value in 1..0x3FFF). 0 = query. */
	ret = ath9k_wmi_longdist(priv->wmi, 0, &cur);
	if (ret)
		return ret;
	priv->debug.ack_timeout = cur;

	len = sprintf(buf, "ack_timeout=%u us (long-distance knob, max 0x3FFF)\n", cur);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_ack_timeout(struct file *file,
				      const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 cur;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val < 1 || val > 0x3FFF)
		return -EINVAL;

	ret = ath9k_wmi_longdist(priv->wmi, (u32)val, &cur);
	if (ret)
		return ret;

	priv->debug.ack_timeout = cur;
	return count;
}

static const struct file_operations fops_ack_timeout = {
	.read = read_file_ack_timeout,
	.write = write_file_ack_timeout,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_chan_survey(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len = 0;
	u8 cnt;
	u16 freq[WMI_CUSTOM_CHAN_SURVEY_MAX];
	s16 nf[WMI_CUSTOM_CHAN_SURVEY_MAX];
	int ret, i;

	ret = ath9k_wmi_chan_survey(priv->wmi, &cnt, freq, nf);
	if (ret)
		return ret;

	priv->debug.chan_count = cnt;
	memcpy(priv->debug.chan_freq, freq, sizeof(freq));
	memcpy(priv->debug.chan_nf, nf, sizeof(nf));

	len += sprintf(buf + len, "channel NF survey (%u sampled channels)\n", cnt);
	for (i = 0; i < cnt; i++)
		len += sprintf(buf + len, "%u MHz: %d dBm\n", freq[i], nf[i]);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_chan_survey = {
	.read = read_file_chan_survey,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_netflow(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[768];
	unsigned int len = 0;
	u8 cnt;
	u8 macs[WMI_CUSTOM_NETFLOW_MAX][6];
	u16 frames[WMI_CUSTOM_NETFLOW_MAX];
	u32 bytes[WMI_CUSTOM_NETFLOW_MAX];
	int ret, i;

	ret = ath9k_wmi_netflow(priv->wmi, 0, &cnt, macs, frames, bytes);
	if (ret)
		return ret;

	priv->debug.netflow_count = cnt;
	memcpy(priv->debug.netflow_macs, macs, sizeof(priv->debug.netflow_macs));
	memcpy(priv->debug.netflow_frames, frames, sizeof(frames));
	memcpy(priv->debug.netflow_bytes, bytes, sizeof(bytes));

	len += sprintf(buf + len, "mini-NetFlow (per-MAC, %u entries)\n", cnt);
	for (i = 0; i < cnt; i++)
		len += sprintf(buf + len, "%pM: frames=%u bytes=%u\n",
			       macs[i], frames[i], bytes[i]);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_netflow(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u8 cnt;
	u8 macs[WMI_CUSTOM_NETFLOW_MAX][6];
	u16 frames[WMI_CUSTOM_NETFLOW_MAX];
	u32 bytes[WMI_CUSTOM_NETFLOW_MAX];
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val != 1)
		return -EINVAL;

	ret = ath9k_wmi_netflow(priv->wmi, 1, &cnt, macs, frames, bytes);
	if (ret)
		return ret;
	return count;
}

static const struct file_operations fops_netflow = {
	.read = read_file_netflow,
	.write = write_file_netflow,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_auto_spec(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[160];
	unsigned int len;
	u32 state, th, active;
	int ret;

	ret = ath9k_wmi_auto_spec(priv->wmi, 0xFF, 0, &state, &th, &active);
	if (ret)
		return ret;

	priv->debug.spec_auto = state;
	priv->debug.spec_threshold = th;
	priv->debug.spec_active = active;

	len = sprintf(buf,
		      "auto_spectral=%u (0=off 1=on) phy_err_trigger=%u per-mille active=%u\n",
		      state, th, active);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_auto_spec(struct file *file,
				    const char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char cmdstr[48];
	char *p, *arg = NULL;
	unsigned long a1 = 0, a2 = 300;
	u32 state, th, active;
	int ret;

	if (count >= sizeof(cmdstr))
		return -EINVAL;
	if (copy_from_user(cmdstr, user_buf, count))
		return -EFAULT;
	cmdstr[count] = '\0';
	if (cmdstr[count - 1] == '\n')
		cmdstr[count - 1] = '\0';

	p = cmdstr;
	arg = strsep(&p, " ");
	if (!strcmp(arg, "on") || !strcmp(arg, "1"))
		a1 = 1;
	else if (!strcmp(arg, "off") || !strcmp(arg, "0"))
		a1 = 0;
	else if (!strcmp(arg, "threshold") && p)
		a1 = 0xFF;
	else
		return -EINVAL;

	if (a1 == 0xFF) {
		if (kstrtoul(p, 0, &a2) || a2 < 10 || a2 > 900)
			return -EINVAL;
	}

	ret = ath9k_wmi_auto_spec(priv->wmi, (u32)a1, (u32)a2,
				  &state, &th, &active);
	if (ret)
		return ret;

	priv->debug.spec_auto = state;
	priv->debug.spec_threshold = th;
	priv->debug.spec_active = active;
	return count;
}

static const struct file_operations fops_auto_spec = {
	.read = read_file_auto_spec,
	.write = write_file_auto_spec,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_blackbox(struct file *file, char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len = 0;
	u8 head, cnt;
	u32 ent[WMI_CUSTOM_BB_PER_PG];
	int ret, j;
	/* CUSTOM BUILD v4-fix: fw log types are 1-based (custom_log_bb callers
	 * in the firmware: 1=ani-step .. 8=ack-to) - the old table started at
	 * index 0, shifting every label down by one ("freq-note" events
	 * displayed as "spectral", "spec-enable" as "baw", "ack-to" as "?"). */
	static const char * const names[] = { "(type-0)", "ani-step",
		"wd-strike", "freq-note", "spectral", "th-cap",
		"spec-enable", "baw", "ack-to" };

	len += sprintf(buf + len,
		       "black box (page %u of 4, head=%u count=%u)\n",
		       priv->debug.bb_page, priv->debug.bb_head,
		       priv->debug.bb_count);
	ret = ath9k_wmi_blackbox(priv->wmi, priv->debug.bb_page,
				 &head, &cnt, ent);
	if (ret)
		return ret;
	priv->debug.bb_head = head;
	priv->debug.bb_count = cnt;
	memcpy(priv->debug.bb_ent, ent, sizeof(ent));

	for (j = 0; j < WMI_CUSTOM_BB_PER_PG; j++) {
		u32 e = ent[j];
		u8 type = e >> 24;
		u32 data = e & 0xFFFFFF;
		const char *nm = (type < ARRAY_SIZE(names)) ? names[type] : "?";

		if (!e)
			continue;
		len += sprintf(buf + len, "%s: %u\n", nm, data);
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_blackbox(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val >= WMI_CUSTOM_BB_PAGES)
		return -EINVAL;

	priv->debug.bb_page = (u8)val;
	return count;
}

static const struct file_operations fops_blackbox = {
	.read = read_file_blackbox,
	.write = write_file_blackbox,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: live noise floor monitor + raw CCA threshold knob.
 * Read: chain-0 noise floor in signed dBm (AR_PHY_CCA bits 19..27).
 * Write: raw minCCA threshold byte (AR_PHY_CCA bits 0..7, signed dBm
 * 2's complement) - advanced knob; a channel change resets it. */
#define CUSTOM_CCA_THRESH     0x000000FF
#define CUSTOM_CCA_THRESH_S   0

/* CUSTOM BUILD v4-fix: checked single-register WMI access. The stock
 * REG_READ/REG_RMW_FIELD helpers swallow failures (ath9k_regread returns
 * -1 = 0xFFFFFFFF on WMI failure), which debugfs then renders as plausible
 * values: nf=-1 dBm and firstep=63/63 were observed LIVE while the control
 * channel was wedged - sentinels, not measurements. These helpers
 * propagate the real errno to the reader instead.
 * v4-fix 2: the target is woken around the access - with the chip in
 * power-save (idle interface) MAC/PHY registers read garbage (ack_timeout
 * showed 16111 on a sleeping target) and writes do not commit. */
static int dbg_reg_read(struct ath9k_htc_priv *priv, u32 reg, u32 *val)
{
	__be32 v = 0, addr = cpu_to_be32(reg);
	int ret;

	ath9k_htc_ps_wakeup(priv);
	ret = ath9k_wmi_cmd(priv->wmi, WMI_REG_READ_CMDID,
			    (u8 *)&addr, sizeof(addr),
			    (u8 *)&v, sizeof(v), 100);
	ath9k_htc_ps_restore(priv);
	if (ret)
		return ret;
	*val = be32_to_cpu(v);
	return 0;
}

static int dbg_reg_write(struct ath9k_htc_priv *priv, u32 reg, u32 val)
{
	__be32 w[2] = { cpu_to_be32(reg), cpu_to_be32(val) };
	__be32 status = 0;
	int ret;

	ath9k_htc_ps_wakeup(priv);
	ret = ath9k_wmi_cmd(priv->wmi, WMI_REG_WRITE_CMDID,
			    (u8 *)w, sizeof(w),
			    (u8 *)&status, sizeof(status), 100);
	ath9k_htc_ps_restore(priv);
	return ret;
}

/* read-modify-write with verification: fails if the readback does not
 * reflect the written bits (fire-and-forget regwrite is not acceptable).
 * The readback is retried a few times: PHY register writes commit through
 * the MAC/PHY interface and an immediate read can still observe the
 * pre-write value (observed live as a spurious -EIO on AR_PHY_FIND_SIG). */
#define DBG_REG_VERIFY_ATTEMPTS 3

static int dbg_reg_rmw_verify(struct ath9k_htc_priv *priv, u32 reg,
			      u32 mask, u32 shift, u32 value)
{
	u32 val, new, rb;
	int ret, i;

	ret = dbg_reg_read(priv, reg, &val);
	if (ret)
		return ret;

	new = (val & ~mask) | ((value << shift) & mask);
	ret = dbg_reg_write(priv, reg, new);
	if (ret)
		return ret;

	for (i = 0; i < DBG_REG_VERIFY_ATTEMPTS; i++) {
		ret = dbg_reg_read(priv, reg, &rb);
		if (ret)
			return ret;
		if ((rb & mask) == ((value << shift) & mask))
			return 0;
		usleep_range(1000, 2000);
	}
	return -EIO;
}

static ssize_t read_file_nf(struct file *file, char __user *user_buf,
			    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[128];
	unsigned int len;
	u32 val;
	int nf, ret;

	ret = dbg_reg_read(priv, AR_PHY_CCA, &val);
	if (ret)
		return ret;   /* honest error (e.g. -ETIMEDOUT), no sentinel */

	/* CUSTOM BUILD v4.1 NF fix (Phase 3.3): AR9002/9271 stores MINCCA at
	 * bits 28..20 — AR9280_PHY_MINCCA_PWR (shift 20), matching stock
	 * ar9002_hw_do_getnf(). The AR5008-era AR_PHY_MINCCA_PWR (shift 19)
	 * reads the field one bit low. raw= is printed so the decode can be
	 * cross-checked offline against ath9k hw NF limits (9271: -127..-116). */
	nf = sign_extend32(MS(val, AR9280_PHY_MINCCA_PWR), 8);
	len = sprintf(buf,
		      "noise_floor=%d dBm (chain0, live)\nraw=0x%08x\n",
		      nf, val);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_nf(struct file *file,
			     const char __user *user_buf,
			     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 0xff)
		return -EINVAL;

	ret = dbg_reg_rmw_verify(priv, AR_PHY_CCA, CUSTOM_CCA_THRESH,
				 CUSTOM_CCA_THRESH_S, (u32)val);
	if (ret)
		return ret;

	return count;
}

static const struct file_operations fops_nf = {
	.read = read_file_nf,
	.write = write_file_nf,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD v2: firmware-resident extensions */

static ssize_t read_file_rate_usage(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[1024];
	unsigned int len = 0;
	u8 usage[WMI_CUSTOM_RATE_USAGE_MAX];
	int ret, i;

	ret = ath9k_wmi_rate_usage(priv->wmi, 0, usage);
	if (ret)
		return ret;

	memcpy(priv->debug.rate_usage, usage, sizeof(usage));

	len += sprintf(buf + len, "rate_selections (index: count)\n");
	for (i = 0; i < WMI_CUSTOM_RATE_USAGE_MAX; i++)
		len += sprintf(buf + len, "%2u: %u\n", i, usage[i]);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_rate_usage(struct file *file,
				     const char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u8 usage[WMI_CUSTOM_RATE_USAGE_MAX];
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val != 1)
		return -EINVAL;

	ret = ath9k_wmi_rate_usage(priv->wmi, 1, usage);
	if (ret)
		return ret;

	memcpy(priv->debug.rate_usage, usage, sizeof(usage));
	return count;
}

static const struct file_operations fops_rate_usage = {
	.read = read_file_rate_usage,
	.write = write_file_rate_usage,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_telemetry(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[256];
	unsigned int len;

	len = sprintf(buf,
		      "seq=%u uptime_s=%u freq_mhz=%u nf=%d rx_frames=%u phy_errs=%u ani_level=%u samples=%lu\n"
		      "dbg: last_cmd=0x%04x tbl_entries=%u beacons=%u dropped=%u\n",
		      priv->debug.tlm_seq,
		      priv->debug.tlm_uptime_s,
		      priv->debug.tlm_freq_mhz,
		      (int)priv->debug.tlm_nf_dbm,
		      priv->debug.tlm_rx,
		      priv->debug.tlm_phyerr,
		      priv->debug.tlm_ani_level,
		      priv->debug.tlm_count,
		      priv->debug.dbg_last_cmd,
		      priv->debug.dbg_tbl_entries,
		      priv->debug.beacons_window,
		      priv->debug.mf_dropped);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_telemetry = {
	.read = read_file_telemetry,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_rssi_hist(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[1024];
	unsigned int len = 0;
	u32 total = 0;
	u8 bins[WMI_CUSTOM_RSSI_HIST_BINS];
	int ret, i;

	ret = ath9k_wmi_rssi_hist(priv->wmi, 0, bins);
	if (ret)
		return ret;

	for (i = 0; i < WMI_CUSTOM_RSSI_HIST_BINS; i++)
		total += bins[i];
	priv->debug.rssi_total = total;
	memcpy(priv->debug.rssi_bins, bins, sizeof(bins));

	len += sprintf(buf + len,
		       "rssi_histogram (bin=rssi offset, frames=%u)\n", total);
	for (i = 0; i < WMI_CUSTOM_RSSI_HIST_BINS; i++)
		if (bins[i])
			len += sprintf(buf + len, "%2u: %u\n", i, bins[i]);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_rssi_hist(struct file *file,
				    const char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u8 bins[WMI_CUSTOM_RSSI_HIST_BINS];
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val != 1)
		return -EINVAL;

	ret = ath9k_wmi_rssi_hist(priv->wmi, 1, bins);
	if (ret)
		return ret;

	priv->debug.rssi_total = 0;
	memcpy(priv->debug.rssi_bins, bins, sizeof(bins));
	return count;
}

static const struct file_operations fops_rssi_hist = {
	.read = read_file_rssi_hist,
	.write = write_file_rssi_hist,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_ani_auto(struct file *file, char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[128];
	unsigned int len;
	u32 state, level, errs;
	int ret;

	ret = ath9k_wmi_ani_auto(priv->wmi, 0xFF, &state, &level, &errs);
	if (ret)
		return ret;

	priv->debug.ani_auto = state;
	priv->debug.ani_level = level;
	priv->debug.ani_errs = errs;

	len = sprintf(buf,
		      "ani_auto=%u (0=manual 1=firmware-adaptive) level=%u window_phy_errs=%u\n",
		      state, level, errs);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_ani_auto(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long val;
	u32 state, level, errs;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	if (val > 1)
		return -EINVAL;

	ret = ath9k_wmi_ani_auto(priv->wmi, (u32)val, &state, &level, &errs);
	if (ret)
		return ret;

	priv->debug.ani_auto = state;
	priv->debug.ani_level = level;
	priv->debug.ani_errs = errs;
	return count;
}

static const struct file_operations fops_ani_auto = {
	.read = read_file_ani_auto,
	.write = write_file_ani_auto,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* macfilter commands: "mode <0|1|2>", "add <idx> <mac>", "del <idx>", "clear" */
static ssize_t read_file_macfilter(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[512];
	unsigned int len = 0;
	u32 mode_out, count_out;
	int ret, i;

	ret = ath9k_wmi_macfilter(priv->wmi, 0, 0, 0, NULL, &mode_out, &count_out);
	if (ret)
		return ret;

	priv->debug.mf_mode = mode_out;
	priv->debug.mf_count = count_out;

	len += sprintf(buf + len,
		       "mode=%u (0=off 1=allow 2=block) entries=%u dropped=%u\n",
		       mode_out, count_out, priv->debug.mf_dropped);
	for (i = 0; i < WMI_CUSTOM_MACFILTER_MAX; i++) {
		u8 *m = priv->debug.mf_macs[i];

		if (m[0] | m[1] | m[2] | m[3] | m[4] | m[5])
			len += sprintf(buf + len,
				       "%d: %pM\n", i, m);
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_macfilter(struct file *file,
				    const char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char cmdstr[48];
	char *p, *arg = NULL;
	unsigned long a1 = 0, a2 = 0;
	u8 mac[6] = { 0 };
	u32 mode_out, count_out;
	int ret;

	if (count >= sizeof(cmdstr))
		return -EINVAL;

	if (copy_from_user(cmdstr, user_buf, count))
		return -EFAULT;
	cmdstr[count] = '\0';
	if (cmdstr[count - 1] == '\n')
		cmdstr[count - 1] = '\0';

	p = cmdstr;
	arg = strsep(&p, " ");

	if (!strcmp(arg, "mode")) {
		if (kstrtoul(p, 0, &a1) || a1 > 2)
			return -EINVAL;
		ret = ath9k_wmi_macfilter(priv->wmi, 1, a1, 0, NULL,
					  &mode_out, &count_out);
	} else if (!strcmp(arg, "add")) {
		arg = strsep(&p, " ");
		if (!arg || kstrtoul(arg, 0, &a1) || a1 > 7)
			return -EINVAL;
		if (!p || !mac_pton(p, mac))
			return -EINVAL;
		ret = ath9k_wmi_macfilter(priv->wmi, 2, 0, a1, mac,
					  &mode_out, &count_out);
		memcpy(priv->debug.mf_macs[a1], mac, 6);
	} else if (!strcmp(arg, "del")) {
		if (kstrtoul(p, 0, &a1) || a1 > 7)
			return -EINVAL;
		ret = ath9k_wmi_macfilter(priv->wmi, 3, 0, a1, NULL,
					  &mode_out, &count_out);
		memset(priv->debug.mf_macs[a1], 0, 6);
	} else if (!strcmp(arg, "clear")) {
		ret = ath9k_wmi_macfilter(priv->wmi, 4, 0, 0, NULL,
					  &mode_out, &count_out);
		memset(priv->debug.mf_macs, 0, sizeof(priv->debug.mf_macs));
	} else {
		return -EINVAL;
	}

	if (ret)
		return ret;

	priv->debug.mf_mode = mode_out;
	priv->debug.mf_count = count_out;
	return count;
}

static const struct file_operations fops_macfilter = {
	.read = read_file_macfilter,
	.write = write_file_macfilter,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* CUSTOM BUILD: manual ANI-style PHY sensitivity tuning (borrowed from the
 * PCI ath9k ANI implementation in ar5008_phy.c). Writes the same registers
 * the FIRSTEP_LEVEL ANI control writes, via checked WMI-backed register ops
 * with readback verification (v4-fix: the old fire-and-forget REG_RMW_FIELD
 * silently no-oped when WMI was wedged and reads rendered 0xFFFFFFFF as
 * "63"). level 0 = most sensitive (clean air), 2 = most immune (noisy air). */
static ssize_t read_file_firstep(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	char buf[160];
	unsigned int len;
	u32 sig, sig_low;
	int ret;

	ret = dbg_reg_read(priv, AR_PHY_FIND_SIG, &sig);
	if (ret)
		return ret;
	ret = dbg_reg_read(priv, AR_PHY_FIND_SIG_LOW, &sig_low);
	if (ret)
		return ret;

	len = sprintf(buf,
		      "level=%u value=%u find_sig_firstep=%u find_sig_low_firstep=%u\n",
		      priv->debug.firstep_level, priv->debug.firstep_level * 2,
		      MS(sig, AR_PHY_FIND_SIG_FIRSTEP),
		      MS(sig_low, AR_PHY_FIND_SIG_FIRSTEP_LOW));
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_firstep(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct ath9k_htc_priv *priv = file->private_data;
	unsigned long level;
	u32 value;
	int ret;

	ret = kstrtoul_from_user(user_buf, count, 0, &level);
	if (ret)
		return ret;

	if (level > ATH9K_ANI_FIRSTEP_LVL)
		return -EINVAL;

	value = level * 2;
	ret = dbg_reg_rmw_verify(priv, AR_PHY_FIND_SIG,
				 AR_PHY_FIND_SIG_FIRSTEP,
				 AR_PHY_FIND_SIG_FIRSTEP_S, value);
	if (ret)
		return ret;
	ret = dbg_reg_rmw_verify(priv, AR_PHY_FIND_SIG_LOW,
				 AR_PHY_FIND_SIG_FIRSTEP_LOW,
				 AR_PHY_FIND_SIG_FIRSTEP_LOW_S, value);
	if (ret)
		return ret;

	priv->debug.firstep_level = level;

	return count;
}

static const struct file_operations fops_firstep = {
	.read = read_file_firstep,
	.write = write_file_firstep,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/* Ethtool support for get-stats */
#define AMKSTR(nm) #nm "_BE", #nm "_BK", #nm "_VI", #nm "_VO"
static const char ath9k_htc_gstrings_stats[][ETH_GSTRING_LEN] = {
	"tx_pkts_nic",
	"tx_bytes_nic",
	"rx_pkts_nic",
	"rx_bytes_nic",

	AMKSTR(d_tx_pkts),

	"d_rx_crc_err",
	"d_rx_decrypt_crc_err",
	"d_rx_phy_err",
	"d_rx_mic_err",
	"d_rx_pre_delim_crc_err",
	"d_rx_post_delim_crc_err",
	"d_rx_decrypt_busy_err",

	"d_rx_phyerr_radar",
	"d_rx_phyerr_ofdm_timing",
	"d_rx_phyerr_cck_timing",

};
#define ATH9K_HTC_SSTATS_LEN ARRAY_SIZE(ath9k_htc_gstrings_stats)

void ath9k_htc_get_et_strings(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      u32 sset, u8 *data)
{
	if (sset == ETH_SS_STATS)
		memcpy(data, ath9k_htc_gstrings_stats,
		       sizeof(ath9k_htc_gstrings_stats));
}

int ath9k_htc_get_et_sset_count(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif, int sset)
{
	if (sset == ETH_SS_STATS)
		return ATH9K_HTC_SSTATS_LEN;
	return 0;
}

#define STXBASE priv->debug.tx_stats
#define SRXBASE priv->debug.rx_stats
#define SKBTXBASE priv->debug.tx_stats
#define SKBRXBASE priv->debug.skbrx_stats
#define ASTXQ(a)					\
	data[i++] = STXBASE.a[IEEE80211_AC_BE];		\
	data[i++] = STXBASE.a[IEEE80211_AC_BK];		\
	data[i++] = STXBASE.a[IEEE80211_AC_VI];		\
	data[i++] = STXBASE.a[IEEE80211_AC_VO]

void ath9k_htc_get_et_stats(struct ieee80211_hw *hw,
			    struct ieee80211_vif *vif,
			    struct ethtool_stats *stats, u64 *data)
{
	struct ath9k_htc_priv *priv = hw->priv;
	int i = 0;

	data[i++] = SKBTXBASE.skb_success;
	data[i++] = SKBTXBASE.skb_success_bytes;
	data[i++] = SKBRXBASE.skb_completed;
	data[i++] = SKBRXBASE.skb_completed_bytes;

	ASTXQ(queue_stats);

	data[i++] = SRXBASE.crc_err;
	data[i++] = SRXBASE.decrypt_crc_err;
	data[i++] = SRXBASE.phy_err;
	data[i++] = SRXBASE.mic_err;
	data[i++] = SRXBASE.pre_delim_crc_err;
	data[i++] = SRXBASE.post_delim_crc_err;
	data[i++] = SRXBASE.decrypt_busy_err;

	data[i++] = SRXBASE.phy_err_stats[ATH9K_PHYERR_RADAR];
	data[i++] = SRXBASE.phy_err_stats[ATH9K_PHYERR_OFDM_TIMING];
	data[i++] = SRXBASE.phy_err_stats[ATH9K_PHYERR_CCK_TIMING];

	WARN_ON(i != ATH9K_HTC_SSTATS_LEN);
}

void ath9k_htc_deinit_debug(struct ath9k_htc_priv *priv)
{
	ath9k_cmn_spectral_deinit_debug(&priv->spec_priv);
}

int ath9k_htc_init_debug(struct ath_hw *ah)
{
	struct ath_common *common = ath9k_hw_common(ah);
	struct ath9k_htc_priv *priv = common->priv;

	priv->debug.debugfs_phy = debugfs_create_dir(KBUILD_MODNAME,
					     priv->hw->wiphy->debugfsdir);

	ath9k_cmn_spectral_init_debug(&priv->spec_priv, priv->debug.debugfs_phy);

	debugfs_create_file("tgt_int_stats", 0400, priv->debug.debugfs_phy,
			    priv, &fops_tgt_int_stats);
	debugfs_create_file("tgt_tx_stats", 0400, priv->debug.debugfs_phy,
			    priv, &fops_tgt_tx_stats);
	debugfs_create_file("tgt_rx_stats", 0400, priv->debug.debugfs_phy,
			    priv, &fops_tgt_rx_stats);
	debugfs_create_file("xmit", 0400, priv->debug.debugfs_phy,
			    priv, &fops_xmit);
	debugfs_create_file("skb_rx", 0400, priv->debug.debugfs_phy,
			    priv, &fops_skb_rx);

	ath9k_cmn_debug_recv(priv->debug.debugfs_phy, &priv->debug.rx_stats);
	ath9k_cmn_debug_phy_err(priv->debug.debugfs_phy, &priv->debug.rx_stats);

	debugfs_create_file("slot", 0400, priv->debug.debugfs_phy,
			    priv, &fops_slot);
	debugfs_create_file("queue", 0400, priv->debug.debugfs_phy,
			    priv, &fops_queue);
	debugfs_create_file("debug", 0600, priv->debug.debugfs_phy,
			    priv, &fops_debug);
	debugfs_create_file("custom_ping", 0600, priv->debug.debugfs_phy,
			    priv, &fops_custom_ping);
	debugfs_create_file("wmi_diag", 0400, priv->debug.debugfs_phy,
			    priv, &fops_wmi_diag);
	debugfs_create_file("rc_profile", 0600, priv->debug.debugfs_phy,
			    priv, &fops_rc_profile);
	debugfs_create_file("rc_tries", 0600, priv->debug.debugfs_phy,
			    priv, &fops_rc_tries);
	debugfs_create_file("rc_fixed", 0600, priv->debug.debugfs_phy,
			    priv, &fops_rc_fixed);
	debugfs_create_file("firstep", 0600, priv->debug.debugfs_phy,
			    priv, &fops_firstep);
	debugfs_create_file("txpower", 0600, priv->debug.debugfs_phy,
			    priv, &fops_txpower);
	debugfs_create_file("nf", 0600, priv->debug.debugfs_phy,
			    priv, &fops_nf);
	debugfs_create_file("baw", 0600, priv->debug.debugfs_phy,
			    priv, &fops_baw);
	debugfs_create_file("ack_timeout", 0600, priv->debug.debugfs_phy,
			    priv, &fops_ack_timeout);
	debugfs_create_file("chan_survey", 0400, priv->debug.debugfs_phy,
			    priv, &fops_chan_survey);
	debugfs_create_file("netflow", 0600, priv->debug.debugfs_phy,
			    priv, &fops_netflow);
	debugfs_create_file("auto_spectral", 0600, priv->debug.debugfs_phy,
			    priv, &fops_auto_spec);
	debugfs_create_file("blackbox", 0400, priv->debug.debugfs_phy,
			    priv, &fops_blackbox);
	debugfs_create_file("rate_usage", 0600, priv->debug.debugfs_phy,
			    priv, &fops_rate_usage);
	debugfs_create_file("telemetry", 0400, priv->debug.debugfs_phy,
			    priv, &fops_telemetry);
	debugfs_create_file("rssi_hist", 0600, priv->debug.debugfs_phy,
			    priv, &fops_rssi_hist);
	debugfs_create_file("ani_auto", 0600, priv->debug.debugfs_phy,
			    priv, &fops_ani_auto);
	debugfs_create_file("macfilter", 0600, priv->debug.debugfs_phy,
			    priv, &fops_macfilter);
	debugfs_create_file("thermal", 0600, priv->debug.debugfs_phy,
			    priv, &fops_thermal);

	ath9k_cmn_debug_base_eeprom(priv->debug.debugfs_phy, priv->ah);
	ath9k_cmn_debug_modal_eeprom(priv->debug.debugfs_phy, priv->ah);

	return 0;
}

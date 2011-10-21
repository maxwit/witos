#pragma once

int mmc_select_card(struct mmc_host *host);
int mmc_deselect_cards(struct mmc_host *host);
int mmc_go_idle(struct mmc_host *host);
int mmc_send_op_cond(struct mmc_host *host, __u32 ocr, __u32 *rocr);
int mmc_all_send_cid(struct mmc_host *host, __u32 *cid);
int mmc_set_relative_addr(struct mmc_host *host);
int mmc_send_csd(struct mmc_host *host, __u32 *csd);
int mmc_send_ext_csd(struct mmc_card *card, __u8 *ext_csd);
int mmc_switch(struct mmc_card *card, __u8 set, __u8 index, __u8 value);
int mmc_send_status(struct mmc_card *card, __u32 *status);
int mmc_send_cid(struct mmc_host *host, __u32 *cid);
int mmc_spi_read_ocr(struct mmc_host *host, int highcap, __u32 *ocrp);
int mmc_spi_set_crc(struct mmc_host *host, int use_crc);
int mmc_card_sleepawake(struct mmc_host *host, int sleep);
int mmc_switch_width(struct mmc_host *host);

int mmc_send_app_op_cond(struct mmc_host *host, __u32 ocr, __u32 *rocr);
int mmc_send_if_cond(struct mmc_host *host, __u32 ocr);
int mmc_app_set_bus_width(struct mmc_host *host, int width);

int mmc_set_block_len(struct mmc_host *host, int len);


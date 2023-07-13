/*
 * test_vectors - IEEE 802.11 test vector generator
 * Copyright (c) 2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "utils/crc32.h"
#include "utils/eloop.h"
#include "common/ieee802_11_defs.h"
#include "wlantest.h"


static void test_vector_tkip(void)
{
	u8 tk[] = {
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56,
		0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12,
		0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78,
		0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34
	};
	u8 pn[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
	u8 frame[] = {
		0x08, 0x42, 0x2c, 0x00, 0x02, 0x03, 0x04, 0x05,
		0x06, 0x08, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xd0, 0x02,
		/* 0x00, 0x20, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, */
		0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00,
		0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00,
		0x40, 0x01, 0xa5, 0x55, 0xc0, 0xa8, 0x0a, 0x02,
		0xc0, 0xa8, 0x0a, 0x01, 0x08, 0x00, 0x3a, 0xb0,
		0x00, 0x00, 0x00, 0x00, 0xcd, 0x4c, 0x05, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
		0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
		0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
		0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
		0x34, 0x35, 0x36, 0x37,
		/* 0x68, 0x81, 0xa3, 0xf3, 0xd6, 0x48, 0xd0, 0x3c */
	};
	u8 *enc, *plain;
	size_t enc_len, plain_len;

	wpa_printf(MSG_INFO, "\nIEEE Std 802.11-2012, M.6.3 TKIP test "
		   "vector\n");

	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_hexdump(MSG_INFO, "PN", pn, sizeof(pn));
	wpa_hexdump(MSG_INFO, "Plaintext MPDU", frame, sizeof(frame));

	enc = tkip_encrypt(tk, frame, sizeof(frame), 24, NULL, pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt TKIP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);

	wpa_debug_level = MSG_INFO;
	plain = tkip_decrypt(tk, (const struct ieee80211_hdr *) enc,
			     enc + 24, enc_len - 24, &plain_len, NULL, NULL);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt TKIP frame");
		return;
	}

	if (plain_len != sizeof(frame) - 24 ||
	    os_memcmp(plain, frame + 24, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
	}

	os_free(plain);
}


static void test_vector_ccmp(void)
{
	u8 tk[] = { 0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85,
		    0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f };
	u8 pn[] = { 0xB5, 0x03, 0x97, 0x76, 0xE7, 0x0C };
	u8 frame[] = {
		0x08, 0x48, 0xc3, 0x2c, 0x0f, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
		0xab, 0xae, 0xa5, 0xb8, 0xfc, 0xba, 0x80, 0x33,
		0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae,
		0x96, 0x7b, 0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb,
		0x7e, 0x78, 0xa0, 0x50
	};
	u8 *enc, *plain;
	size_t enc_len, plain_len;
	u8 fcs[4];

	wpa_printf(MSG_INFO, "\nIEEE Std 802.11-2012, M.6.4 CCMP test "
		   "vector\n");

	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_hexdump(MSG_INFO, "PN", pn, sizeof(pn));
	wpa_hexdump(MSG_INFO, "802.11 Header", frame, 24);
	wpa_hexdump(MSG_INFO, "Plaintext Data", frame + 24, sizeof(frame) - 24);

	enc = ccmp_encrypt(tk, frame, sizeof(frame), 24, NULL, NULL, NULL, NULL,
			   pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "FCS", fcs, sizeof(fcs));

	wpa_debug_level = MSG_INFO;
	plain = ccmp_decrypt(tk, (const struct ieee80211_hdr *) enc,
			     NULL, NULL, NULL, enc + 24, enc_len - 24,
			     &plain_len);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt CCMP frame");
		return;
	}

	if (plain_len != sizeof(frame) - 24 ||
	    os_memcmp(plain, frame + 24, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
	}

	os_free(plain);
}


static void test_vector_ccmp_pv1(void)
{
	u8 tk[] = { 0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85,
		    0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f };
	u8 pn[8];
	u8 frame1[] = {
		0x61, 0x00, 0xa2, 0xae, 0xa5, 0xb8, 0xfc, 0xba,
		0x07, 0x00, 0x80, 0x33,
		0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae,
		0x96, 0x7b, 0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb,
		0x7e, 0x78, 0xa0, 0x50
	};
	u8 frame2[] = {
		0x61, 0x00, 0xa2, 0xae, 0xa5, 0xb8, 0xfc, 0xba,
		0x07, 0x20, 0x80, 0x33, 0x02, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c,
		0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae,
		0x96, 0x7b, 0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb,
		0x7e, 0x78, 0xa0, 0x50
	};
	u8 frame3[] = {
		0x6d, 0x00, 0xa2, 0xae, 0xa5, 0xb8, 0xfc, 0xba,
		0x52, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x80, 0x33,
		0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae,
		0x96, 0x7b, 0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb,
		0x7e, 0x78, 0xa0, 0x50
	};
	u8 *enc;
	size_t enc_len;
	u8 fcs[4];
	u8 bssid[ETH_ALEN] = { 0xa2, 0xae, 0xa5, 0xb8, 0xfc, 0xba };
	u8 da[ETH_ALEN] = { 0x02, 0xd2, 0xe1, 0x28, 0xa5, 0x7c };
	u8 sa[ETH_ALEN] = { 0x52, 0x30, 0xf1, 0x84, 0x44, 0x08 };
	u16 aid = 7;
	u32 bpn = 123;
	u16 sc = 0x3380;
	int key_id = 0;
	u16 fc;
	int tid = 3;
	u16 sid;

	wpa_printf(MSG_INFO,
		   "\nIEEE P802.11ah/D10.0, J.6.4 CCMP PV1 test vectors\n");

	wpa_printf(MSG_INFO, "BSSID: " MACSTR, MAC2STR(bssid));
	wpa_printf(MSG_INFO, "DA: " MACSTR, MAC2STR(da));
	wpa_printf(MSG_INFO, "SA: " MACSTR, MAC2STR(sa));
	wpa_printf(MSG_INFO, "Association ID: %u", aid);
	wpa_printf(MSG_INFO, "Base PN: %u (0x%08x)", bpn, bpn);
	wpa_printf(MSG_INFO, "SC = 0x%04x (FragNum=%u SeqNum=%u)",
		   sc, WLAN_GET_SEQ_FRAG(sc), WLAN_GET_SEQ_SEQ(sc));
	wpa_printf(MSG_INFO, "TID = %u", tid);
	wpa_printf(MSG_INFO, "Key ID: %u", key_id);
	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_printf(MSG_INFO, "PN = SC||BPN");
	WPA_PUT_LE16(&pn[0], sc);
	WPA_PUT_LE32(&pn[2], bpn);
	wpa_hexdump(MSG_INFO, "PN (PN0..PN5)", pn, sizeof(pn));

	wpa_printf(MSG_INFO,
		   "\nPV1 test vector #1:\nHeader compression used and A3 was previously stored at the receiver\n");
	fc = WPA_GET_LE16(frame1);
	wpa_printf(MSG_INFO,
		   "FC=0x%04x (PV=%u Type=%u PTID/Subtype=%u From_DS=%u More_Fragments=%u Power_Management=%u More_Data=%u Protected_Frame=%u End_of_SP=%u Relayed_Frame=%u Ack_Policy=%u)",
		   fc,
		   fc & WLAN_FC_PVER,
		   (fc & (BIT(2) | BIT(3) | BIT(4))) >> 2,
		   (fc & (BIT(5) | BIT(6) | BIT(7))) >> 5,
		   !!(fc & BIT(8)),
		   !!(fc & BIT(9)),
		   !!(fc & BIT(10)),
		   !!(fc & BIT(11)),
		   !!(fc & BIT(12)),
		   !!(fc & BIT(13)),
		   !!(fc & BIT(14)),
		   !!(fc & BIT(15)));
	wpa_printf(MSG_INFO, "A1=" MACSTR, MAC2STR(&frame1[2]));
	sid = WPA_GET_LE16(&frame1[8]);
	wpa_printf(MSG_INFO,
		   "A2=%02x %02x (SID: AID=%u A3_Present=%u A4_Present=%u A-MSDU=%u); corresponds to 52:30:f1:84:44:08 in uncompressed header",
		   frame1[8], frame1[9],
		   sid & ~(BIT(13) | BIT(14) | BIT(15)),
		   !!(sid & BIT(13)),
		   !!(sid & BIT(14)),
		   !!(sid & BIT(15)));
	sc = WPA_GET_LE16(&frame1[10]);
	wpa_printf(MSG_INFO, "Sequence Control: %02x %02x (FN=%u SN=%u)",
		   frame1[10], frame1[11],
		   WLAN_GET_SEQ_FRAG(sc), WLAN_GET_SEQ_SEQ(sc));
	wpa_printf(MSG_INFO, "A3 not present; corresponds to 02:d2:e1:28:a5:7c in uncompressed header");
	wpa_printf(MSG_INFO, "A4 not present");
	wpa_hexdump(MSG_INFO, "Plaintext Frame Header", frame1, 12);
	wpa_hexdump(MSG_INFO, "Plaintext Frame Body",
		    frame1 + 12, sizeof(frame1) - 12);

	enc = ccmp_encrypt_pv1(tk, &frame1[2], sa, da, frame1, sizeof(frame1),
			       12, pn, key_id, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted Frame Header", enc, 12);
	wpa_hexdump(MSG_INFO, "Encrypted Frame Frame Body",
		    enc + 12, enc_len - 12);
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "Encrypted Frame FCS", fcs, sizeof(fcs));

	wpa_printf(MSG_INFO,
		   "\nPV1 test vector #2:\nHeader compression used and A3 was not previously stored at the receiver\n");
	fc = WPA_GET_LE16(frame2);
	wpa_printf(MSG_INFO,
		   "FC=0x%04x (PV=%u Type=%u PTID/Subtype=%u From_DS=%u More_Fragments=%u Power_Management=%u More_Data=%u Protected_Frame=%u End_of_SP=%u Relayed_Frame=%u Ack_Policy=%u)",
		   fc,
		   fc & WLAN_FC_PVER,
		   (fc & (BIT(2) | BIT(3) | BIT(4))) >> 2,
		   (fc & (BIT(5) | BIT(6) | BIT(7))) >> 5,
		   !!(fc & BIT(8)),
		   !!(fc & BIT(9)),
		   !!(fc & BIT(10)),
		   !!(fc & BIT(11)),
		   !!(fc & BIT(12)),
		   !!(fc & BIT(13)),
		   !!(fc & BIT(14)),
		   !!(fc & BIT(15)));
	wpa_printf(MSG_INFO, "A1=" MACSTR, MAC2STR(&frame2[2]));
	sid = WPA_GET_LE16(&frame2[8]);
	wpa_printf(MSG_INFO,
		   "A2=%02x %02x (SID: AID=%u A3_Present=%u A4_Present=%u A-MSDU=%u); corresponds to 52:30:f1:84:44:08 in uncompressed header",
		   frame2[8], frame2[9],
		   sid & ~(BIT(13) | BIT(14) | BIT(15)),
		   !!(sid & BIT(13)),
		   !!(sid & BIT(14)),
		   !!(sid & BIT(15)));
	sc = WPA_GET_LE16(&frame2[10]);
	wpa_printf(MSG_INFO, "Sequence Control: %02x %02x (FN=%u SN=%u)",
		   frame2[10], frame2[11],
		   WLAN_GET_SEQ_FRAG(sc), WLAN_GET_SEQ_SEQ(sc));
	wpa_printf(MSG_INFO, "A3=" MACSTR, MAC2STR(&frame2[12]));
	wpa_printf(MSG_INFO, "A4 not present");
	wpa_hexdump(MSG_INFO, "Plaintext Frame Header", frame2, 18);
	wpa_hexdump(MSG_INFO, "Plaintext Frame Body",
		    frame2 + 18, sizeof(frame2) - 18);

	enc = ccmp_encrypt_pv1(tk, &frame2[2], sa, &frame2[12],
			       frame2, sizeof(frame2), 18, pn, key_id,
			       &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted Frame Header", enc, 18);
	wpa_hexdump(MSG_INFO, "Encrypted Frame Frame Body",
		    enc + 18, enc_len - 18);
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "Encrypted Frame FCS", fcs, sizeof(fcs));

	wpa_printf(MSG_INFO,
		   "\nPV1 test vector #3:\nType 3 frame from SA to DA(=BSSID) (i.e., no separate DA in this example)\n");
	fc = WPA_GET_LE16(frame3);
	wpa_printf(MSG_INFO,
		   "FC=0x%04x (PV=%u Type=%u PTID/Subtype=%u From_DS=%u More_Fragments=%u Power_Management=%u More_Data=%u Protected_Frame=%u End_of_SP=%u Relayed_Frame=%u Ack_Policy=%u)",
		   fc,
		   fc & WLAN_FC_PVER,
		   (fc & (BIT(2) | BIT(3) | BIT(4))) >> 2,
		   (fc & (BIT(5) | BIT(6) | BIT(7))) >> 5,
		   !!(fc & BIT(8)),
		   !!(fc & BIT(9)),
		   !!(fc & BIT(10)),
		   !!(fc & BIT(11)),
		   !!(fc & BIT(12)),
		   !!(fc & BIT(13)),
		   !!(fc & BIT(14)),
		   !!(fc & BIT(15)));
	wpa_printf(MSG_INFO, "A1=" MACSTR, MAC2STR(&frame3[2]));
	wpa_printf(MSG_INFO, "A2=" MACSTR, MAC2STR(&frame3[8]));
	sc = WPA_GET_LE16(&frame3[14]);
	wpa_printf(MSG_INFO, "Sequence Control: %02x %02x (FN=%u SN=%u)",
		   frame3[14], frame3[15],
		   WLAN_GET_SEQ_FRAG(sc), WLAN_GET_SEQ_SEQ(sc));
	wpa_printf(MSG_INFO,
		   "A3 not present; corresponds to 02:d2:e1:28:a5:7c in uncompressed header");
	wpa_printf(MSG_INFO, "A4 not present");
	wpa_hexdump(MSG_INFO, "Plaintext Frame Header", frame3, 16);
	wpa_hexdump(MSG_INFO, "Plaintext Frame Body",
		    frame3 + 16, sizeof(frame3) - 16);

	enc = ccmp_encrypt_pv1(tk, &frame3[2], &frame3[8], da,
			       frame3, sizeof(frame3), 16, pn, key_id,
			       &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted Frame Header", enc, 16);
	wpa_hexdump(MSG_INFO, "Encrypted Frame Frame Body",
		    enc + 16, enc_len - 16);
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "Encrypted Frame FCS", fcs, sizeof(fcs));

	wpa_debug_level = MSG_INFO;
}


static void test_vector_bip(void)
{
	u8 igtk[] = {
		0x4e, 0xa9, 0x54, 0x3e, 0x09, 0xcf, 0x2b, 0x1e,
		0xca, 0x66, 0xff, 0xc5, 0x8b, 0xde, 0xcb, 0xcf
	};
	u8 ipn[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 frame[] = {
		0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
		0x02, 0x00
	};
	u8 *prot;
	size_t prot_len;

	wpa_printf(MSG_INFO, "\nIEEE Std 802.11-2012, M.9.1 BIP with broadcast "
		   "Deauthentication frame\n");

	wpa_hexdump(MSG_INFO, "IGTK", igtk, sizeof(igtk));
	wpa_hexdump(MSG_INFO, "IPN", ipn, sizeof(ipn));
	wpa_hexdump(MSG_INFO, "Plaintext frame", frame, sizeof(frame));

	prot = bip_protect(igtk, sizeof(igtk), frame, sizeof(frame),
			   ipn, 4, &prot_len);
	if (prot == NULL) {
		wpa_printf(MSG_ERROR, "Failed to protect BIP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Protected MPDU (without FCS)", prot, prot_len);
	os_free(prot);
}


static void test_vector_ccmp_mgmt(void)
{
	u8 tk[] = { 0x66, 0xed, 0x21, 0x04, 0x2f, 0x9f, 0x26, 0xd7,
		    0x11, 0x57, 0x06, 0xe4, 0x04, 0x14, 0xcf, 0x2e };
	u8 pn[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
	u8 frame[] = {
		0xc0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
		0x02, 0x00
	};
	u8 *enc, *plain;
	size_t enc_len, plain_len;

	wpa_printf(MSG_INFO, "\nIEEE Std 802.11-2012, M.9.2 CCMP with unicast "
		   "Deauthentication frame\n");

	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_hexdump(MSG_INFO, "PN", pn, sizeof(pn));
	wpa_hexdump(MSG_INFO, "802.11 Header", frame, 24);
	wpa_hexdump(MSG_INFO, "Plaintext Data", frame + 24, sizeof(frame) - 24);

	enc = ccmp_encrypt(tk, frame, sizeof(frame), 24, NULL, NULL, NULL, NULL,
			   pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);

	wpa_debug_level = MSG_INFO;
	plain = ccmp_decrypt(tk, (const struct ieee80211_hdr *) enc,
			     NULL, NULL, NULL, enc + 24, enc_len - 24,
			     &plain_len);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt CCMP frame");
		return;
	}

	if (plain_len != sizeof(frame) - 24 ||
	    os_memcmp(plain, frame + 24, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
	}

	os_free(plain);
}


struct gcmp_test {
	u8 tk[16];
	u8 pn[6];
	u8 frame[300];
	size_t hdr_len;
	size_t payload_len;
	u8 mic[16];
	u8 encr[300];
};

static const struct gcmp_test gcmp_vectors[] =
{
	{
		.tk = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
			0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa },
		.pn = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
		.frame = {
			0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,

			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		.hdr_len = 24,
		.payload_len = 256,
		.mic = {
			0x80, 0xCB, 0x06, 0x62, 0xEA, 0x71, 0xAB, 0xFD,
			0x9F, 0x04, 0xC7, 0xF8, 0x72, 0xF5, 0x80, 0x90 },
		.encr = {
			0x5F, 0x55, 0x78, 0xC1, 0x8F, 0x13, 0x7A, 0xD2,
			0x79, 0xBF, 0x3F, 0x2B, 0x24, 0xC7, 0xBD, 0x8F,
			0x27, 0x7A, 0x1B, 0xE6, 0x77, 0x0D, 0xA1, 0xD9,
			0x8B, 0x70, 0xC6, 0xD2, 0x8A, 0xE0, 0x1C, 0x55,
			0x9E, 0xCB, 0xA6, 0xA0, 0x1D, 0xB0, 0x67, 0xC5,
			0xA2, 0x7E, 0x4D, 0xB0, 0x8C, 0xDA, 0xDC, 0x77,
			0x52, 0xAD, 0x63, 0x7E, 0xAF, 0x0A, 0x18, 0xED,
			0x13, 0xFB, 0xAA, 0x14, 0x3B, 0xAF, 0xEF, 0x18,
			0xF8, 0xFB, 0xCE, 0x4C, 0x65, 0xE8, 0x6B, 0xD0,
			0x2A, 0x87, 0xB6, 0x01, 0xB7, 0xEA, 0xB9, 0x3F,
			0x2B, 0xBC, 0x87, 0x4C, 0x8A, 0x71, 0x05, 0x80,
			0xF5, 0x02, 0x34, 0x1A, 0x6A, 0x53, 0x39, 0x31,
			0x43, 0xDE, 0x4C, 0x9E, 0xC6, 0xA2, 0x86, 0xF1,
			0x25, 0x71, 0x83, 0x78, 0xAE, 0xDC, 0x84, 0xEB,
			0xA2, 0xB3, 0x0F, 0x5C, 0x28, 0xBB, 0x5D, 0x75,
			0xC6, 0xB0, 0x25, 0x46, 0x6D, 0x06, 0x51, 0xC7,
			0x22, 0xDC, 0x71, 0x15, 0x1F, 0x21, 0x2D, 0x68,
			0x87, 0x82, 0x8A, 0x03, 0x82, 0xE9, 0x28, 0x8A,
			0x7F, 0x43, 0xD5, 0x2B, 0x7D, 0x25, 0x08, 0x61,
			0x57, 0x64, 0x69, 0x54, 0xBB, 0x43, 0xB5, 0x7E,
			0xA5, 0x87, 0xA0, 0x25, 0xF4, 0x0C, 0xE7, 0x45,
			0x11, 0xE4, 0xDD, 0x22, 0x85, 0xB4, 0x0B, 0xA3,
			0xF3, 0xB9, 0x62, 0x62, 0xCB, 0xC2, 0x8C, 0x6A,
			0xA7, 0xBE, 0x44, 0x3E, 0x7B, 0x41, 0xE1, 0xEB,
			0xFF, 0x52, 0x48, 0x57, 0xA6, 0x81, 0x68, 0x97,
			0x75, 0x01, 0x15, 0xB0, 0x23, 0x1A, 0xB7, 0xC2,
			0x84, 0x72, 0xC0, 0x6D, 0xD0, 0xB4, 0x9B, 0xE9,
			0xF3, 0x69, 0xA8, 0xC3, 0x9C, 0xCD, 0x0D, 0xB7,
			0x98, 0x35, 0x10, 0xE1, 0xAE, 0x8F, 0x05, 0xD7,
			0x75, 0x45, 0xE0, 0x23, 0x5C, 0xDB, 0xD6, 0x12,
			0xF3, 0x15, 0x07, 0x54, 0xCE, 0xE5, 0xCE, 0x6A,
			0x12, 0x25, 0xD9, 0x95, 0x25, 0x02, 0x6F, 0x74
		}
	},
	{
		.tk = { 0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85,
			0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f },
		.pn = { 0x00, 0x89, 0x5F, 0x5F, 0x2B, 0x08 },
		.frame = {
			0x88, 0x48, 0x0b, 0x00, 0x0f, 0xd2, 0xe1, 0x28,
			0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
			0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x80, 0x33,
			0x03, 0x00,

			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
			0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
		},
		.hdr_len = 26,
		.payload_len = 40,
		.mic = {
			0xde, 0xf6, 0x19, 0xc2, 0xa3, 0x74, 0xb6, 0xdf,
			0x66, 0xff, 0xa5, 0x3b, 0x6c, 0x69, 0xd7, 0x9e },
		.encr = {
			0x60, 0xe9, 0x70, 0x0c, 0xc4, 0xd4, 0x0a, 0xc6,
			0xd2, 0x88, 0xb2, 0x01, 0xc3, 0x8f, 0x5b, 0xf0,
			0x8b, 0x80, 0x74, 0x42, 0x64, 0x0a, 0x15, 0x96,
			0xe5, 0xdb, 0xda, 0xd4, 0x1d, 0x1f, 0x36, 0x23,
			0xf4, 0x5d, 0x7a, 0x12, 0xdb, 0x7a, 0xfb, 0x23
		}
	}
};


static int run_gcmp(int idx, const struct gcmp_test *vector)
{
	u8 *enc, *plain;
	size_t enc_len, plain_len;
	u8 fcs[4];
	int err = 0;

	wpa_printf(MSG_INFO,
		   "\nIEEE Std 802.11ad-2012, M.11.1 GCMP test mpdu #%d\n",
		   idx);

	wpa_hexdump(MSG_INFO, "TK", vector->tk, sizeof(vector->tk));
	wpa_hexdump(MSG_INFO, "PN", vector->pn, sizeof(vector->pn));
	wpa_hexdump(MSG_INFO, "802.11 Header", vector->frame, vector->hdr_len);
	wpa_hexdump(MSG_INFO, "Plaintext Data",
		    vector->frame + vector->hdr_len,
		    vector->payload_len);

	enc = gcmp_encrypt(vector->tk, sizeof(vector->tk),
			   vector->frame,
			   vector->hdr_len + vector->payload_len,
			   vector->hdr_len,
			   vector->hdr_len == 26 ?
			   vector->frame + vector->hdr_len - 2 : NULL,
			   NULL, NULL, NULL,
			   vector->pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt GCMP frame");
		return 1;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);
	if (os_memcmp(vector->encr, enc + vector->hdr_len + 8,
		      vector->payload_len) != 0) {
		wpa_printf(MSG_ERROR, "GCMP test mpdu #%d enctypted data mismatch",
			   idx);
		err++;
	}
	if (os_memcmp(vector->mic, enc + enc_len - sizeof(vector->mic),
		      sizeof(vector->mic)) != 0) {
		wpa_printf(MSG_ERROR, "GCMP test mpdu #%d MIC mismatch", idx);
		err++;
	}
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "FCS", fcs, sizeof(fcs));

	wpa_debug_level = MSG_INFO;
	plain = gcmp_decrypt(vector->tk, sizeof(vector->tk),
			     (const struct ieee80211_hdr *) enc, NULL, NULL,
			     NULL, enc + vector->hdr_len,
			     enc_len - vector->hdr_len, &plain_len);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt GCMP frame");
		return 1;
	}

	if (plain_len != vector->payload_len ||
	    os_memcmp(plain, vector->frame + vector->hdr_len, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
		err++;
	}

	os_free(plain);

	return err;
}


static int test_vector_gcmp(void)
{
	int err = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(gcmp_vectors); i++) {
		if (run_gcmp(i + 1, &gcmp_vectors[i]))
			err++;

	}

	return err;
}


static int test_vector_gcmp_256(void)
{
	u8 tk[] = { 0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85,
		    0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f,
		    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	u8 pn[] = {
		0x00, 0x89, 0x5F, 0x5F, 0x2B, 0x08
	};
	u8 frame[] = {
		0x88, 0x48, 0x0b, 0x00, 0x0f, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
		0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x80, 0x33,
		0x03, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
		0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
		0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
		0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
		0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
		0x26, 0x27
	};
	u8 encr[] = {
		0x88, 0x48, 0x0b, 0x00, 0x0f, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
		0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x80, 0x33,
		0x03, 0x00, 0x08, 0x2b, 0x00, 0x20, 0x5f, 0x5f,
		0x89, 0x00, 0x65, 0x83, 0x43, 0xc8, 0xb1, 0x44,
		0x47, 0xd9, 0x21, 0x1d, 0xef, 0xd4, 0x6a, 0xd8,
		0x9c, 0x71, 0x0c, 0x6f, 0xc3, 0x33, 0x33, 0x23,
		0x6e, 0x39, 0x97, 0xb9, 0x17, 0x6a, 0x5a, 0x8b,
		0xe7, 0x79, 0xb2, 0x12, 0x66, 0x55, 0x5e, 0x70,
		0xad, 0x79, 0x11, 0x43, 0x16, 0x85, 0x90, 0x95,
		0x47, 0x3d, 0x5b, 0x1b, 0xd5, 0x96, 0xb3, 0xde,
		0xa3, 0xbf
	};
	u8 *enc, *plain;
	size_t enc_len, plain_len;
	u8 fcs[4];
	int err = 0;

	wpa_printf(MSG_INFO, "\nIEEE P802.11ac/D7.0, M.11.1 GCMP-256 test vector\n");

	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_hexdump(MSG_INFO, "PN", pn, sizeof(pn));
	wpa_hexdump(MSG_INFO, "802.11 Header", frame, 26);
	wpa_hexdump(MSG_INFO, "Plaintext Data", frame + 26, sizeof(frame) - 26);

	enc = gcmp_encrypt(tk, sizeof(tk), frame, sizeof(frame), 26, frame + 24,
			   NULL, NULL, NULL, pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt GCMP frame");
		return 1;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);
	if (enc_len != sizeof(encr) || os_memcmp(enc, encr, enc_len) != 0) {
		wpa_printf(MSG_ERROR, "GCMP-256 test vector mismatch");
		err++;
	}
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "FCS", fcs, sizeof(fcs));

	wpa_debug_level = MSG_INFO;
	plain = gcmp_decrypt(tk, sizeof(tk), (const struct ieee80211_hdr *) enc,
			     NULL, NULL, NULL, enc + 26, enc_len - 26,
			     &plain_len);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt GCMP frame");
		return 1;
	}

	if (plain_len != sizeof(frame) - 26 ||
	    os_memcmp(plain, frame + 26, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
		err++;
	}

	os_free(plain);

	return err;
}


static int test_vector_ccmp_256(void)
{
	u8 tk[] = { 0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85,
		    0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f,
		    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	u8 pn[] = { 0xB5, 0x03, 0x97, 0x76, 0xE7, 0x0C };
	u8 frame[] = {
		0x08, 0x48, 0xc3, 0x2c, 0x0f, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
		0xab, 0xae, 0xa5, 0xb8, 0xfc, 0xba, 0x80, 0x33,
		0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae,
		0x96, 0x7b, 0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb,
		0x7e, 0x78, 0xa0, 0x50
	};
	u8 encr[] = {
		0x08, 0x48, 0xc3, 0x2c, 0x0f, 0xd2, 0xe1, 0x28,
		0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08,
		0xab, 0xae, 0xa5, 0xb8, 0xfc, 0xba, 0x80, 0x33,
		0x0c, 0xe7, 0x00, 0x20, 0x76, 0x97, 0x03, 0xb5,
		0x6d, 0x15, 0x5d, 0x88, 0x32, 0x66, 0x82, 0x56,
		0xd6, 0xa9, 0x2b, 0x78, 0xe1, 0x1d, 0x8e, 0x54,
		0x49, 0x5d, 0xd1, 0x74, 0x80, 0xaa, 0x56, 0xc9,
		0x49, 0x2e, 0x88, 0x2b, 0x97, 0x64, 0x2f, 0x80,
		0xd5, 0x0f, 0xe9, 0x7b

	};
	u8 *enc, *plain;
	size_t enc_len, plain_len;
	u8 fcs[4];
	int err = 0;

	wpa_printf(MSG_INFO, "\nIEEE P802.11ac/D7.0, M.6.4 CCMP-256 test vector\n");

	wpa_hexdump(MSG_INFO, "TK", tk, sizeof(tk));
	wpa_hexdump(MSG_INFO, "PN", pn, sizeof(pn));
	wpa_hexdump(MSG_INFO, "802.11 Header", frame, 24);
	wpa_hexdump(MSG_INFO, "Plaintext Data", frame + 24, sizeof(frame) - 24);

	enc = ccmp_256_encrypt(tk, frame, sizeof(frame), 24, NULL, NULL, NULL,
			       NULL, pn, 0, &enc_len);
	if (enc == NULL) {
		wpa_printf(MSG_ERROR, "Failed to encrypt CCMP frame");
		return 1;
	}

	wpa_hexdump(MSG_INFO, "Encrypted MPDU (without FCS)", enc, enc_len);
	if (enc_len != sizeof(encr) || os_memcmp(enc, encr, enc_len) != 0) {
		wpa_printf(MSG_ERROR, "CCMP-256 test vector mismatch");
		err++;
	}
	WPA_PUT_LE32(fcs, ieee80211_crc32(enc, enc_len));
	wpa_hexdump(MSG_INFO, "FCS", fcs, sizeof(fcs));

	wpa_debug_level = MSG_INFO;
	plain = ccmp_256_decrypt(tk, (const struct ieee80211_hdr *) enc,
				 NULL, NULL, NULL, enc + 24, enc_len - 24,
				 &plain_len);
	wpa_debug_level = MSG_EXCESSIVE;
	os_free(enc);

	if (plain == NULL) {
		wpa_printf(MSG_ERROR, "Failed to decrypt CCMP-256 frame");
		return 1;
	}

	if (plain_len != sizeof(frame) - 24 ||
	    os_memcmp(plain, frame + 24, plain_len) != 0) {
		wpa_hexdump(MSG_ERROR, "Decryption result did not match",
			    plain, plain_len);
		err++;
	}

	os_free(plain);

	return err;
}


static int test_vector_bip_gmac_128(void)
{
	u8 igtk[] = {
		0x4e, 0xa9, 0x54, 0x3e, 0x09, 0xcf, 0x2b, 0x1e,
		0xca, 0x66, 0xff, 0xc5, 0x8b, 0xde, 0xcb, 0xcf
	};
	u8 ipn[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 frame[] = {
		0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
		0x02, 0x00
	};
	u8 res[] = {
		0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
		0x02, 0x00, 0x4c, 0x18, 0x04, 0x00, 0x04, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x3e, 0xd8, 0x62, 0xfb,
		0x0f, 0x33, 0x38, 0xdd, 0x33, 0x86, 0xc8, 0x97,
		0xe2, 0xed, 0x05, 0x3d
	};
	u8 *prot;
	size_t prot_len;
	int err = 0;

	wpa_printf(MSG_INFO, "\nIEEE P802.11ac/D7.0, M.9.1 BIP-GMAC-128 with broadcast "
		   "Deauthentication frame\n");

	wpa_hexdump(MSG_INFO, "IGTK", igtk, sizeof(igtk));
	wpa_hexdump(MSG_INFO, "IPN", ipn, sizeof(ipn));
	wpa_hexdump(MSG_INFO, "Plaintext frame", frame, sizeof(frame));

	prot = bip_gmac_protect(igtk, sizeof(igtk), frame, sizeof(frame),
				ipn, 4, &prot_len);
	if (prot == NULL) {
		wpa_printf(MSG_ERROR, "Failed to protect BIP-GMAC-128 frame");
		return 1;
	}

	wpa_hexdump(MSG_INFO, "Protected MPDU (without FCS)", prot, prot_len);
	if (prot_len != sizeof(res) || os_memcmp(res, prot, prot_len) != 0) {
		wpa_printf(MSG_ERROR, "BIP-GMAC-128 test vector mismatch");
		err++;
	}
	os_free(prot);

	return err;
}


static int test_vector_bip_gmac_256(void)
{
	u8 igtk[] = {
		0x4e, 0xa9, 0x54, 0x3e, 0x09, 0xcf, 0x2b, 0x1e,
		0xca, 0x66, 0xff, 0xc5, 0x8b, 0xde, 0xcb, 0xcf,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	u8 ipn[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 frame[] = {
		0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
		0x02, 0x00
	};
	u8 res[] = {
		0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
		0x02, 0x00, 0x4c, 0x18, 0x04, 0x00, 0x04, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x23, 0xbe, 0x59, 0xdc,
		0xc7, 0x02, 0x2e, 0xe3, 0x83, 0x62, 0x7e, 0xbb,
		0x10, 0x17, 0xdd, 0xfc
	};
	u8 *prot;
	size_t prot_len;
	int err = 0;

	wpa_printf(MSG_INFO, "\nIEEE P802.11ac/D7.0, M.9.1 BIP-GMAC-256 with broadcast Deauthentication frame\n");

	wpa_hexdump(MSG_INFO, "IGTK", igtk, sizeof(igtk));
	wpa_hexdump(MSG_INFO, "IPN", ipn, sizeof(ipn));
	wpa_hexdump(MSG_INFO, "Plaintext frame", frame, sizeof(frame));

	prot = bip_gmac_protect(igtk, sizeof(igtk), frame, sizeof(frame),
				ipn, 4, &prot_len);
	if (prot == NULL) {
		wpa_printf(MSG_ERROR, "Failed to protect BIP-GMAC-256 frame");
		return 1;
	}

	wpa_hexdump(MSG_INFO, "Protected MPDU (without FCS)", prot, prot_len);
	if (prot_len != sizeof(res) || os_memcmp(res, prot, prot_len) != 0) {
		wpa_printf(MSG_ERROR, "BIP-GMAC-128 test vector mismatch");
		err++;
	}
	os_free(prot);

	return err;
}


int main(int argc, char *argv[])
{
	int errors = 0;

	wpa_debug_level = MSG_EXCESSIVE;
	wpa_debug_show_keys = 1;

	if (os_program_init())
		return -1;

	test_vector_tkip();
	test_vector_ccmp();
	test_vector_ccmp_pv1();
	test_vector_bip();
	test_vector_ccmp_mgmt();
	errors += test_vector_gcmp();
	errors += test_vector_gcmp_256();
	errors += test_vector_ccmp_256();
	errors += test_vector_bip_gmac_128();
	errors += test_vector_bip_gmac_256();

	if (errors)
		wpa_printf(MSG_INFO, "One or more test vectors failed");
	os_program_deinit();

	return errors ? -1 : 0;
}

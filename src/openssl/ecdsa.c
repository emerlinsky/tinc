/*
    ecdsa.c -- ECDSA key handling
    Copyright (C) 2011 Guus Sliepen <guus@tinc-vpn.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "system.h"

#include <openssl/pem.h>
#include <openssl/err.h>

#include "logger.h"
#include "ecdsa.h"
#include "utils.h"

// Set ECDSA keys

bool ecdsa_set_base64_public_key(ecdsa_t *ecdsa, const char *p) {
	*ecdsa = EC_KEY_new_by_curve_name(NID_secp521r1);

	int len = strlen(p);
	unsigned char pubkey[len / 4 * 3 + 3];
	const unsigned char *ppubkey = pubkey;
	len = b64decode(p, pubkey, len);

	if(!o2i_ECPublicKey(ecdsa, &ppubkey, len)) {
		logger(LOG_DEBUG, "o2i_ECPublicKey failed: %s", ERR_error_string(ERR_get_error(), NULL));
		abort();
	}

	return true;
}

// Read PEM ECDSA keys

bool ecdsa_read_pem_public_key(ecdsa_t *ecdsa, FILE *fp) {
	*ecdsa = PEM_read_EC_PUBKEY(fp, ecdsa, NULL, NULL);

	if(*ecdsa)
		return true;

	logger(LOG_ERR, "Unable to read ECDSA public key: %s", ERR_error_string(ERR_get_error(), NULL));
	return false;
}

bool ecdsa_read_pem_private_key(ecdsa_t *ecdsa, FILE *fp) {
	*ecdsa = PEM_read_ECPrivateKey(fp, NULL, NULL, NULL);

	if(*ecdsa)
		return true;
	
	logger(LOG_ERR, "Unable to read ECDSA private key: %s", ERR_error_string(ERR_get_error(), NULL));
	return false;
}

size_t ecdsa_size(ecdsa_t *ecdsa) {
	return ECDSA_size(*ecdsa);
}

// TODO: hash first, standardise output format?

bool ecdsa_sign(ecdsa_t *ecdsa, const void *in, size_t len, void *sig) {
	unsigned int siglen = ECDSA_size(*ecdsa);
	memset(sig, 0, siglen);

	if(!ECDSA_sign(0, in, len, sig, &siglen, *ecdsa)) {
		logger(LOG_DEBUG, "ECDSA_sign() failed: %s", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	if(siglen != ECDSA_size(*ecdsa)) {
		logger(LOG_ERR, "Signature length %d != %d", siglen, ECDSA_size(*ecdsa));
	}

	return true;
}

bool ecdsa_verify(ecdsa_t *ecdsa, const void *in, size_t len, const void *sig) {
	unsigned int siglen = ECDSA_size(*ecdsa);

	if(!ECDSA_verify(0, in, len, sig, siglen, *ecdsa)) {
		logger(LOG_DEBUG, "ECDSA_verify() failed: %s", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	return true;
}

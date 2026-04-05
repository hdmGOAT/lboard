#include "x25519.h"

#include <limits.h>
#include <sodium.h>

static int g_sodium_ready = 0;

static int validate_buffer(const unsigned char *buf, size_t len) {
	if (len > 0 && buf == NULL) {
		return -1;
	}

	return 0;
}

int lboard_auth_init(void) {
	if (g_sodium_ready) {
		return 0;
	}

	if (sodium_init() < 0) {
		return -1;
	}

	g_sodium_ready = 1;
	return 0;
}

void lboard_auth_random_bytes(unsigned char *out, size_t out_len) {
	if (out == NULL || out_len == 0) {
		return;
	}

	if (lboard_auth_init() != 0) {
		return;
	}

	randombytes_buf(out, out_len);
}

int lboard_x25519_generate_keypair(
	unsigned char public_key[LBOARD_X25519_PUBLIC_KEY_BYTES],
	unsigned char private_key[LBOARD_X25519_PRIVATE_KEY_BYTES]) {
	if (public_key == NULL || private_key == NULL) {
		return -1;
	}

	if (lboard_auth_init() != 0) {
		return -1;
	}

	return crypto_kx_keypair(public_key, private_key);
}

int lboard_x25519_derive_shared_secret(
	unsigned char shared_secret[LBOARD_X25519_SHARED_SECRET_BYTES],
	const unsigned char private_key[LBOARD_X25519_PRIVATE_KEY_BYTES],
	const unsigned char peer_public_key[LBOARD_X25519_PUBLIC_KEY_BYTES]) {
	if (shared_secret == NULL || private_key == NULL || peer_public_key == NULL) {
		return -1;
	}

	if (lboard_auth_init() != 0) {
		return -1;
	}

	return crypto_scalarmult_curve25519(shared_secret, private_key, peer_public_key);
}

int lboard_token_generate(
	unsigned char token[LBOARD_AUTH_TOKEN_BYTES],
	const unsigned char key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char *payload,
	size_t payload_len) {
	if (token == NULL || key == NULL) {
		return -1;
	}

	if (payload_len > ULLONG_MAX || validate_buffer(payload, payload_len) != 0) {
		return -1;
	}

	if (lboard_auth_init() != 0) {
		return -1;
	}

	return crypto_auth_hmacsha256(token, payload, (unsigned long long)payload_len, key);
}

int lboard_token_verify(
	const unsigned char token[LBOARD_AUTH_TOKEN_BYTES],
	const unsigned char key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char *payload,
	size_t payload_len) {
	if (token == NULL || key == NULL) {
		return -1;
	}

	if (payload_len > ULLONG_MAX || validate_buffer(payload, payload_len) != 0) {
		return -1;
	}

	if (lboard_auth_init() != 0) {
		return -1;
	}

	return crypto_auth_hmacsha256_verify(
		token,
		payload,
		(unsigned long long)payload_len,
		key);
}

int lboard_token_key_from_shared_secret(
	unsigned char token_key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char shared_secret[LBOARD_X25519_SHARED_SECRET_BYTES],
	const unsigned char *context,
	size_t context_len) {
	static const unsigned char kdf_tag[] = "lboard-token-key-v1";
	crypto_generichash_state state;

	if (token_key == NULL || shared_secret == NULL) {
		return -1;
	}

	if (context_len > ULLONG_MAX || validate_buffer(context, context_len) != 0) {
		return -1;
	}

	if (lboard_auth_init() != 0) {
		return -1;
	}

	if (crypto_generichash_init(&state, NULL, 0, LBOARD_AUTH_TOKEN_KEY_BYTES) != 0) {
		return -1;
	}

	if (crypto_generichash_update(&state, kdf_tag, sizeof(kdf_tag) - 1) != 0) {
		return -1;
	}

	if (crypto_generichash_update(&state, shared_secret, LBOARD_X25519_SHARED_SECRET_BYTES) != 0) {
		return -1;
	}

	if (context_len > 0 &&
		crypto_generichash_update(&state, context, (unsigned long long)context_len) != 0) {
		return -1;
	}

	return crypto_generichash_final(&state, token_key, LBOARD_AUTH_TOKEN_KEY_BYTES);
}

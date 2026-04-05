#ifndef X25519_H
#define X25519_H

#include <stddef.h>

enum {
	LBOARD_X25519_PUBLIC_KEY_BYTES = 32,
	LBOARD_X25519_PRIVATE_KEY_BYTES = 32,
	LBOARD_X25519_SHARED_SECRET_BYTES = 32,
	LBOARD_AUTH_TOKEN_BYTES = 32,
	LBOARD_AUTH_TOKEN_KEY_BYTES = 32,
};

int lboard_auth_init(void);

void lboard_auth_random_bytes(unsigned char *out, size_t out_len);

int lboard_x25519_generate_keypair(
	unsigned char public_key[LBOARD_X25519_PUBLIC_KEY_BYTES],
	unsigned char private_key[LBOARD_X25519_PRIVATE_KEY_BYTES]);

int lboard_x25519_derive_shared_secret(
	unsigned char shared_secret[LBOARD_X25519_SHARED_SECRET_BYTES],
	const unsigned char private_key[LBOARD_X25519_PRIVATE_KEY_BYTES],
	const unsigned char peer_public_key[LBOARD_X25519_PUBLIC_KEY_BYTES]);

int lboard_token_generate(
	unsigned char token[LBOARD_AUTH_TOKEN_BYTES],
	const unsigned char key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char *payload,
	size_t payload_len);

int lboard_token_verify(
	const unsigned char token[LBOARD_AUTH_TOKEN_BYTES],
	const unsigned char key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char *payload,
	size_t payload_len);

int lboard_token_key_from_shared_secret(
	unsigned char token_key[LBOARD_AUTH_TOKEN_KEY_BYTES],
	const unsigned char shared_secret[LBOARD_X25519_SHARED_SECRET_BYTES],
	const unsigned char *context,
	size_t context_len);

#endif

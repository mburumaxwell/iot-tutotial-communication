#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mbedtls/base64.h"
#include "mbedtls/md.h"

#define WRITE_HEX_DUMPS	1

#if WRITE_HEX_DUMPS
extern void hex_dump (FILE* f, void *addr, int len);
#define HEX_DUMP( ... )		hex_dump(stdout, __VA_ARGS__)
#endif

#define REQUEST_STRING_FORMAT "%s\n%u"

int32_t make_sas_signature(const char* audience, const uint8_t* device_key, uint8_t device_key_len, uint32_t expiry_time, uint8_t* result, size_t result_len, size_t* written_len) {
	size_t decoded_len, request_len, hashed_len;
	int32_t ret;
	uint8_t *decoded, *request, *hashed_request;
	mbedtls_md_context_t* md_ctx;

	ret = mbedtls_base64_decode(NULL, 0, &decoded_len, device_key, device_key_len);
	if (ret == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) decoded = malloc(decoded_len);
	else return ret;
	
	if (decoded == NULL) {
		printf("Memory allocation failed\n");
		goto closeup;
	}
	
	if ((ret = mbedtls_base64_decode(decoded, decoded_len, &decoded_len, device_key, device_key_len)) != 0)
		goto closeup;
	
#if WRITE_HEX_DUMPS
	printf("\n> decoded key: %d bytes\n", decoded_len);
	HEX_DUMP(decoded, decoded_len);
#endif
	
	request_len = snprintf(NULL, 0, REQUEST_STRING_FORMAT, audience, expiry_time);
	request_len++;// allow for NULL character at the end
	if ((request = malloc(request_len)) == NULL) {
		printf("Memory allocation failed\n");
		goto closeup;
	}
	
	request_len = snprintf((char*)request, request_len, REQUEST_STRING_FORMAT, audience, expiry_time);
	
#if WRITE_HEX_DUMPS
	printf("\n> request: %d bytes\n", request_len);
	HEX_DUMP(request, request_len);
#endif
	
	if ((md_ctx = malloc(sizeof(mbedtls_md_context_t))) == NULL) {
		printf("Memory allocation failed\n");
		goto closeup;
	}
	
	mbedtls_md_init(md_ctx);
	
	if ((ret = mbedtls_md_setup(md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1)) != 0) {
		printf("HMAC setup failed! returned %d (-0x%04x)\r\n", ret, -ret);
		goto closeup;
	}
	
	if ((ret = mbedtls_md_hmac_starts(md_ctx, decoded, decoded_len)) != 0) {
		printf("HMAC starts failed! returned %d (-0x%04x)\r\n", ret, -ret);
		goto closeup;
	}
	
	if ((ret = mbedtls_md_hmac_update(md_ctx, request, request_len)) != 0) {
		printf("HMAC update failed! returned %d (-0x%04x)\r\n", ret, -ret);
		goto closeup;
	}
	
	if ((hashed_request = malloc(request_len + decoded_len)) == NULL) {
		printf("Memory allocation failed\n");
		goto closeup;
	}
	
	if ((ret = mbedtls_md_hmac_finish(md_ctx, hashed_request)) != 0) {
		printf("HMAC update failed! returned %d (-0x%04x)\r\n", ret, -ret);
		goto closeup;
	}
	
	hashed_len = strlen((const char*)hashed_request);
	
#if WRITE_HEX_DUMPS
	printf("\n> hashed request: %d bytes\n", hashed_len);
	HEX_DUMP(hashed_request, hashed_len);
#endif

	if ((ret = mbedtls_base64_encode(result, result_len, written_len, hashed_request, hashed_len)) != 0) {
		printf("Encoding device key failed! returned %d (-0x%04x)\r\n", ret, -ret);
		goto closeup;
	}

#if WRITE_HEX_DUMPS
	printf("\n> result: %d bytes\n", *written_len);
	HEX_DUMP(result, *written_len);
#endif

	
closeup:
	mbedtls_md_free(md_ctx);
	if (decoded) free(decoded);
	if (request) free(request);
	if (md_ctx) free(md_ctx);
	if (hashed_request) free(hashed_request);
	return ret;
}

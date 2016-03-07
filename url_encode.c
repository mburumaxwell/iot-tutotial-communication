#include <stdint.h>
#include <string.h>

#define HEX_CHARS "0123456789abcdef"

int32_t url_encode(const uint8_t *src, const uint32_t srclen, uint8_t *dest, const uint32_t destlen) {
	int32_t c;
	int32_t ri = 0, wi = 0;
	int32_t max_wi = destlen -1;//allow space for NULL terminating character
	
	while ((c = src[ri++]) != NULL) {
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ) {
			if (wi < max_wi && dest != NULL) dest[wi] = c;
			wi++;
		} else {
			if (wi < max_wi && dest != NULL) dest[wi] = '%';
			wi++;
			if (wi < max_wi && dest != NULL) dest[wi] = HEX_CHARS[c >> 4];
			wi++;
			if (wi < max_wi && dest != NULL) dest[wi] = HEX_CHARS[c & 15];
			wi++;
		}
	}
	
	// always write the null (terminating) character
	if (dest != NULL && destlen > 0) dest[destlen -1] = NULL;
	
	return wi;
}

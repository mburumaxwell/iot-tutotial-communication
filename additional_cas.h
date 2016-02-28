#ifndef __ADDITIONAL_CAS_H__
#define __ADDITIONAL_CAS_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MBEDTLS_PEM_PARSE_C)
/* Concatenation of all CA certificates in PEM format if available */
extern const char   additional_cas_pem[];
extern const size_t additional_cas_pem_len;
#endif

#ifdef __cplusplus
}
#endif
	
#endif /* __ADDITIONAL_CAS_H__ */

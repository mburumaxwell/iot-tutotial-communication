#include "additional_cas.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/certs.h"

#if defined(MBEDTLS_PEM_PARSE_C)
#define GLOBALSIGN_CA																										\
"-----BEGIN CERTIFICATE-----\r\n"																				\
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"	\
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"	\
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"	\
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"	\
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"	\
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"	\
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"	\
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"	\
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"	\
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"	\
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"	\
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"	\
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"	\
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"	\
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"	\
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"	\
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"	\
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"	\
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"	\
"-----END CERTIFICATE-----\r\n"

#define BALTIMORE_CA																										\
"-----BEGIN CERTIFICATE-----\r\n"																				\
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"	\
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"	\
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"	\
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\n"	\
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"	\
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\n"	\
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"	\
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\n"	\
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"	\
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\n"	\
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"	\
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\n"	\
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"	\
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n"	\
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"	\
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\n"	\
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"	\
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\n"	\
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n"															\
"-----END CERTIFICATE-----\r\n"

#define MSIT_CA																													\
"-----BEGIN CERTIFICATE-----\r\n"																				\
"MIIFhjCCBG6gAwIBAgIEByeaqTANBgkqhkiG9w0BAQsFADBaMQswCQYDVQQGEwJJ\r\n"	\
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"	\
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTEzMTIxOTIwMDczMloX\r\n"	\
"DTE3MTIxOTIwMDY1NVowgYsxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5n\r\n"	\
"dG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9y\r\n"	\
"YXRpb24xFTATBgNVBAsTDE1pY3Jvc29mdCBJVDEeMBwGA1UEAxMVTWljcm9zb2Z0\r\n"	\
"IElUIFNTTCBTSEEyMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA0eg3\r\n"	\
"p3aKcEsZ8CA3CSQ3f+r7eOYFumqtTicN/HJq2WwhxGQRlXMQClwle4hslAT9x9uu\r\n"	\
"e9xKCLM+FvHQrdswbdcaHlK1PfBHGQPifaa9VxM/VOo6o7F3/ELwY0lqkYAuMEnA\r\n"	\
"iusrr/466wddBvfp/YQOkb0JICnobl0JzhXT5+/bUOtE7xhXqwQdvDH593sqE8/R\r\n"	\
"PVGvG8W1e+ew/FO7mudj3kEztkckaV24Rqf/ravfT3p4JSchJjTKAm43UfDtWBpg\r\n"	\
"lPbEk9jdMCQl1xzrGZQ1XZOyrqopg3PEdFkFUmed2mdROQU6NuryHnYrFK7sPfkU\r\n"	\
"mYsHbrznDFberL6u23UykJ5jvXS/4ArK+DSWZ4TN0UI4eMeZtgzOtg/pG8v0Wb4R\r\n"	\
"DsssMsj6gylkeTyLS/AydGzzk7iWa11XWmjBzAx5ihne9UkCXgiAAYkMMs3S1pbV\r\n"	\
"S6Dz7L+r9H2zobl82k7X5besufIlXwHLjJaoKK7BM1r2PwiQ3Ov/OdgmyBKdHJqq\r\n"	\
"qcAWjobtZ1KWAH8Nkj092XA25epCbx+uleVbXfjQOsfU3neG0PyeTuLiuKloNwnE\r\n"	\
"OeOFuInzH263bR9KLxgJb95KAY8Uybem7qdjnzOkVHxCg2i4pd+/7LkaXRM72a1o\r\n"	\
"/SAKVZEhZPnXEwGgCF1ZiRtEr6SsxwUQ+kFKqPsCAwEAAaOCASAwggEcMBIGA1Ud\r\n"	\
"EwEB/wQIMAYBAf8CAQAwUwYDVR0gBEwwSjBIBgkrBgEEAbE+AQAwOzA5BggrBgEF\r\n"	\
"BQcCARYtaHR0cDovL2N5YmVydHJ1c3Qub21uaXJvb3QuY29tL3JlcG9zaXRvcnku\r\n"	\
"Y2ZtMA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUH\r\n"	\
"AwIwHwYDVR0jBBgwFoAU5Z1ZMIJHWMys+ghUNoZ7OrUETfAwQgYDVR0fBDswOTA3\r\n"	\
"oDWgM4YxaHR0cDovL2NkcDEucHVibGljLXRydXN0LmNvbS9DUkwvT21uaXJvb3Qy\r\n"	\
"MDI1LmNybDAdBgNVHQ4EFgQUUa8kJpz0aCJXgCYrO0ZiFXsezKUwDQYJKoZIhvcN\r\n"	\
"AQELBQADggEBAHaFxSMxH7Rz6qC8pe3fRUNqf2kgG4Cy+xzdqn+I0zFBNvf7+2ut\r\n"	\
"mIx4H50RZzrNS+yovJ0VGcQ7C6eTzuj8nVvoH8tWrnZDK8cTUXdBqGZMX6fR16p1\r\n"	\
"xRspTMn0baFeoYWTFsLLO6sUfUT92iUphir+YyDK0gvCNBW7r1t/iuCq7UWm6nnb\r\n"	\
"2DVmVEPeNzPR5ODNV8pxsH3pFndk6FmXudUu0bSR2ndx80oPSNI0mWCVN6wfAc0Q\r\n"	\
"negqpSDHUJuzbEl4K1iSZIm4lTaoNKrwQdKVWiRUl01uBcSVrcR6ozn7eQaKm6ZP\r\n"	\
"2SL6RE4288kPpjnngLJev7050UblVUfbvG4=\r\n"															\
"-----END CERTIFICATE-----\r\n"

#define IOT_ECLIPSE_CA																									\
"-----BEGIN CERTIFICATE-----\r\n"																				\
"MIIEVDCCAwygAwIBAgIEVYPx2jANBgkqhkiG9w0BAQsFADCBhzELMAkGA1UEBhMC\r\n"	\
"Q0ExJTAjBgNVBAoTHEVjbGlwc2Uub3JnIEZvdW5kYXRpb24sIEluYy4xFDASBgNV\r\n"	\
"BAsTC2lvdC10ZXN0aW5nMQ8wDQYDVQQHEwZOZXBlYW4xEDAOBgNVBAgTB09udGFy\r\n"	\
"aW8xGDAWBgNVBAMTD2lvdC5lY2xpcHNlLm9yZzAeFw0xNTA2MTkxMDQxMzFaFw0y\r\n"	\
"MDA1MjMxMDQxMzRaMIGHMQswCQYDVQQGEwJDQTElMCMGA1UEChMcRWNsaXBzZS5v\r\n"	\
"cmcgRm91bmRhdGlvbiwgSW5jLjEUMBIGA1UECxMLaW90LXRlc3RpbmcxDzANBgNV\r\n"	\
"BAcTBk5lcGVhbjEQMA4GA1UECBMHT250YXJpbzEYMBYGA1UEAxMPaW90LmVjbGlw\r\n"	\
"c2Uub3JnMIIBUjANBgkqhkiG9w0BAQEFAAOCAT8AMIIBOgKCATEA0xUc7iXRrDG3\r\n"	\
"WBCM6kzBv7eDVJUj8fMt7yhVS+os3QwKXfirzvRn1kj3KQkBzR/Nkeqk4go5EkYI\r\n"	\
"ZCvG3svmfbyxJaWxl8VBhhiVf8ytd0CkNbbho5VlV2BrDuMpAMiQ1MZZqXV544wf\r\n"	\
"BXAChXPgDjsjP/2QDAR52jJjqwoGm+KFAp9ZTFpHqi1Yajt2J7M1EOacnkasDdcD\r\n"	\
"GzgxDIA1oo6XOM1sisOc11d+L1JyOwtSHIaQRO9BChU5CAZCTRF9wITdmBnhb+ha\r\n"	\
"mEgHPFIeF8uPXnjvsXJgHM/GqOoIlb671DOVNdZPxDhg+Pp47qGQJrcTz+ptTLw4\r\n"	\
"bgcSCbHQXK+TDk2GxUS27PbZ9trK2Rfo6MLe4x0kT8k+9zZkPDKdJzCYDVzfcakf\r\n"	\
"Qj8cDxplrwIDAQABo2YwZDAMBgNVHRMBAf8EAjAAMCAGA1UdEQQZMBeCD2lvdC5l\r\n"	\
"Y2xpcHNlLm9yZ4cExike8TATBgNVHSUEDDAKBggrBgEFBQcDATAdBgNVHQ4EFgQU\r\n"	\
"sXXbkSdzImOC1cQcnXOPeGdghvowDQYJKoZIhvcNAQELBQADggExAHueO/jU8LNE\r\n"	\
"M5GyA0ENlPLoeBgtsa2HGXGH1vrc9HEESaFLlQX/V0hEa6MtFTtZTGz4J4zfJK0s\r\n"	\
"Zb7aMxQqlyDE0tGHb6p4XiyjyEcIpAaRuOdMFvsELBugLQIGyJU+FRhrJ6we0G/9\r\n"	\
"aZg+PPhiLRsRQtxgS6zWJ3zQt8Vxj7gnjMjUzuJ30LeborKOZih0By1zY127TbgK\r\n"	\
"0+7CjI3kzUzgX5gy6/YKJLoEl7In7GgLCXy4TdRAC4jrmiBZA3xhn2YbpEGG383z\r\n"	\
"JL7gwrTxilGPdpUkPqsh3QPFybe//IuZnv0asPY4t9GgVqObomUGkafBcz1kDnlh\r\n"	\
"eCQzrplSaCcePR3cawnX/x6ZjnRSfKGJzKYWBVtxLT6TW95AyGyv0bpsH5d3TpKO\r\n"	\
"ayb6IYpBphw=\r\n"																											\
"-----END CERTIFICATE-----\r\n"


/* Concatenation of all additional CA certificates in PEM format if available */
const char   additional_cas_pem[] = GLOBALSIGN_CA BALTIMORE_CA MSIT_CA IOT_ECLIPSE_CA;
const size_t additional_cas_pem_len = sizeof(additional_cas_pem);
#endif

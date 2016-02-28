#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include <stdio.h>
#include <string.h>

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'TLSThread': Thread that demonstrates how to use mbedTLS
 *---------------------------------------------------------------------------*/
 
void TLSThread (void const *argument);                             // thread function
osThreadId tid_TLSThread;                                          // thread id
osThreadDef (TLSThread, osPriorityNormal, 1, 6144);                // thread object

int Init_TLSThread (void) {

  tid_TLSThread = osThreadCreate (osThread(TLSThread), NULL);
  if (!tid_TLSThread) return(-1);
  
  return(0);
}

const char *pers = "ssl_client1";
#define SERVER_PORT "443"
#define SERVER_NAME "tls.mbed.org"
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"
#define DEBUG_LEVEL 1


static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
	((void) level);

	fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
	fflush(  (FILE *) ctx  );
}

static mbedtls_net_context server_fd;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static mbedtls_x509_crt cacert;
static unsigned char buf[1025];

void TLSThread (void const *argument) {
	osEvent evt;
	int ret;
	uint32_t flags;
	int len;

#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif
	
  while (1) {
		evt = osSignalWait(0, osWaitForever);				// Wait forever until there is a signal sent to this thread
		if (evt.status != osEventSignal) continue;	// Ensure that the result received is from a signal
		
		/* Step 1. Initialize session data */
		mbedtls_net_init(&server_fd);
		mbedtls_ssl_init(&ssl);
		mbedtls_ssl_config_init(&conf);
		mbedtls_x509_crt_init(&cacert);
		mbedtls_ctr_drbg_init(&ctr_drbg);
		
		printf("\r\n  . Seeding the random number generator..."); fflush(stdout);
		mbedtls_entropy_init(&entropy);
		if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0) {
				printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
				goto exit_n_loop;
		}
		printf(" ok\n");
		
		/* Step 2. Initialize CA root certificates */
		printf("  . Loading the CA root certificate ..."); fflush(stdout);
		ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
		if (ret < 0) {
			printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\r\n", -ret);
			goto exit_n_loop;
		}
		printf(" ok (%d skipped)\r\n", ret);	

		/* Step 3. Start the connection */
		printf("  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT); fflush(stdout);
		if ((ret = mbedtls_net_connect(&server_fd, SERVER_NAME, SERVER_PORT, MBEDTLS_NET_PROTO_TCP)) != 0) {
			printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
			goto exit_n_loop;
		}
		printf(" ok\n");
		
		/* Step 4. Setup stuff */
		printf("  . Setting up the SSL/TLS structure..."); fflush(stdout);
		if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
			printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
			goto exit_n_loop;
		}
		printf(" ok\n");
		
		/* OPTIONAL is not optimal for security, but makes interop easier in this simplified example */
		mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
		mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
		mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
		mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

		if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
			printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
			goto exit_n_loop;
		}

		if ((ret = mbedtls_ssl_set_hostname( &ssl, "mbed TLS Server 1")) != 0) {
			printf( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
			goto exit_n_loop;
		}

		mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
		
		/* Step 5. Handshake */
		printf("  . Performing the SSL/TLS handshake..."); fflush(stdout);
		while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
				goto exit_n_loop;
			}
		}
		printf( " ok\n" );
		
		/* Step 6. Verify the server certificate */
		printf("  . Verifying peer X.509 certificate..."); fflush(stdout);

		/* In real life, we probably want to bail out when ret != 0 */
		if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
			char vrfy_buf[512];
			printf(" failed\n");
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
			printf("%s\n", vrfy_buf);
		}
		else
			printf(" ok\n");

		/* Step 7. Write the GET request */
		printf("  > Write to server:"); fflush(stdout);
		len = sprintf((char *) buf, GET_REQUEST);
		while ((ret = mbedtls_ssl_write( &ssl, buf, len)) <= 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
				goto exit_n_loop;
			}
		}
		len = ret;
		printf(" %d bytes written\n\n%s", len, (char *) buf);
		
		/* Step 8. Read the HTTP response */
		printf("  < Read from server:"); fflush(stdout);

		do {
			len = sizeof(buf) - 1;
			memset(buf, 0, sizeof(buf));
			ret = mbedtls_ssl_read(&ssl, buf, len);

			if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
			if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) break;

			if (ret < 0) {
				printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
				break;
			}

			if (ret == 0) {
				printf("\n\nEOF\n\n");
				break;
			}

			len = ret;
			printf(" %d bytes read\n\n%s", len, (char *) buf);
		} while (1);

		/* Step 9. Close session */
		mbedtls_ssl_close_notify(&ssl);
		
exit_n_loop:
#ifdef MBEDTLS_ERROR_C
		if( ret != 0 ) {
			char error_buf[100];
			mbedtls_strerror( ret, error_buf, 100 );
			printf("Last error was: %d - %s\n\n", ret, error_buf );
		}
#endif

		mbedtls_net_free(&server_fd);
		mbedtls_x509_crt_free(&cacert);
		mbedtls_ssl_free(&ssl );
		mbedtls_ssl_config_free(&conf);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		continue;
  }
}

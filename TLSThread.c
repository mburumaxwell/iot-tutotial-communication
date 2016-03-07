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
#include "additional_cas.h"

#include "mqtt/MQTTPacket.h"

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

const char *pers = "this should be random";
#define SERVER_PORT "8883"
#define SERVER_NAME "iot.eclipse.org"
#define DEBUG_LEVEL 1


typedef enum {
	MQTT_STATE_CONNECT_WAIT = 0,
	MQTT_STATE_CONNECT_DONE,
	MQTT_STATE_PUBLISH_WAIT,
	MQTT_STATE_PUBLISH_DONE,
	MQTT_STATE_DISCONNECT_WAIT,
	MQTT_STATE_DISCONNECT_DONE
} mqtt_state;

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

static int32_t PrepateMqttConPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp);
static int32_t PrepateMqttPubPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp);
static int32_t PrepateMqttDisconPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp);
extern void hex_dump (FILE* f, void *addr, int len);
#define HEX_DUMP( ... )		hex_dump(stdout, __VA_ARGS__)
	
void TLSThread (void const *argument) {
	osEvent evt;
	int ret;
	uint32_t flags;
	int len;
	mqtt_state state = MQTT_STATE_CONNECT_WAIT;
	uint8_t readresp = 0;

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
		ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *) additional_cas_pem, additional_cas_pem_len);
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

		if ((ret = mbedtls_ssl_set_hostname( &ssl, SERVER_NAME)) != 0) {
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
		printf(" ok\n");
		
		/* Step 6. Verify the server certificate */
		printf("  . Verifying peer X.509 certificate..."); fflush(stdout);
		if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
			char vrfy_buf[512];
			printf(" failed\n");
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
			printf("%s\n", vrfy_buf);
			goto notify_close;
		}
		else
			printf(" ok\n");

		
		/* Step 7. Write the GET request */
do_next_step:		
		switch (state) {
			case MQTT_STATE_CONNECT_WAIT:
				printf("\n  . Preparing MQTT Connect packet..."); fflush(stdout);
				if ((len = PrepateMqttConPacket(buf, sizeof(buf), &readresp)) <= 0) {
					printf(" failed returned %d\n", len);
					goto notify_close;
				}
				else printf(" ok\n");
				break;
			case MQTT_STATE_CONNECT_DONE:
				printf("\n  . Preparing MQTT Publish packet..."); fflush(stdout);
				if ((len = PrepateMqttPubPacket(buf, sizeof(buf), &readresp)) <= 0) {
					printf(" failed returned %d\n", len);
					goto notify_close;
				}
				else printf(" ok\n");
				state = MQTT_STATE_PUBLISH_WAIT;
				break;
			case MQTT_STATE_PUBLISH_DONE:
				printf("\n  . Preparing MQTT Disconnect packet..."); fflush(stdout);
				if ((len = PrepateMqttDisconPacket(buf, sizeof(buf), &readresp)) <= 0) {
					printf(" failed returned %d\n", len);
					goto notify_close;
				}
				else printf(" ok\n");
				state = MQTT_STATE_DISCONNECT_WAIT;
				break;
			default:
				goto notify_close;
		}
		
		printf("  > Write to server:"); fflush(stdout);
		while ((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
				goto exit_n_loop;
			}
		}
		len = ret;
		printf(" %d bytes written\n", len);
		HEX_DUMP(buf, len);

		if (readresp) {
			/* Step 8. Read the response */
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

				printf(" %d bytes read\n", ret);
				HEX_DUMP(buf, ret);
				if (ret < len) break; // do not continue if the read length is less than the buffer size because there will be nothing to read.
			} while (1);
		}

		if (ret >= 0) {
			state++;
			if (state != MQTT_STATE_DISCONNECT_DONE)
				goto do_next_step;
			
			printf("\n  . MQTT process completed!\n");
		}
		
notify_close:
		/* Step 9. Close session */
		mbedtls_ssl_close_notify(&ssl);
		
exit_n_loop:
#ifdef MBEDTLS_ERROR_C
		if (ret < 0) {
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



static const char* mqtt_client		=	"testclient";
static const char* mqtt_username	= "testusername";
static const char* mqtt_password	= "testpassword";
static const char* mqtt_topic			=	"devices/64F7295EA8C/messages/telemetry";
static const char* mqtt_payload		= "{\"temp1\":22.3546,\"temp2\":54.1287,\"weight\":4578.125}";

static MQTTPacket_connectData mqtt_con_data = MQTTPacket_connectData_initializer;
static MQTTString mqtt_topic_data = MQTTString_initializer;

static int32_t PrepateMqttConPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp) {
	int32_t len;
	
	mqtt_con_data.clientID.cstring = (char *) mqtt_client;
	mqtt_con_data.keepAliveInterval = 20;
	mqtt_con_data.cleansession = 1;
	mqtt_con_data.username.cstring = (char *) mqtt_username;
	mqtt_con_data.password.cstring = (char *) mqtt_password;
	mqtt_con_data.MQTTVersion = 4;

	len = MQTTSerialize_connect((unsigned char *)dest, destlen, &mqtt_con_data);
	*hasresp = 1; // every connect packet has a response
	return len;
}

static int32_t PrepateMqttPubPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp) {
	int32_t len, mqtt_payload_len;
	const uint8_t qos = 1;
	
	mqtt_payload_len = strlen(mqtt_payload);
	mqtt_topic_data.cstring = (char *) mqtt_topic;
	
	len = MQTTSerialize_publish((unsigned char *)dest, destlen, // the destination buffer and its length
																0,														// the DUP (duplicate) flag
																qos,													// the QoS (quality of service) flag
																0,														// the retained flag
																0,														// the packetid
																mqtt_topic_data,							// the topic
																(unsigned char *)mqtt_payload,// the payload
																mqtt_payload_len);						// the payload length
	*hasresp = (qos > 0); //For QoS=1 and QoS=2, there is a response from the server (PUBACK or PUBREC)
	return len;
}

static int32_t PrepateMqttDisconPacket(uint8_t *dest,  uint32_t destlen, uint8_t* hasresp) {
	int32_t len;
	
	len = MQTTSerialize_disconnect((unsigned char *)dest, destlen);
	*hasresp = 0;// the disconnect packet has no response
	return len;
}

#include "net_modem.h"

/* Modem driver states */
enum Modem_State {
  Modem_Idle,
  Modem_Error,
  Modem_Ready,
  Modem_Init,
  Modem_Listen,
  Modem_Dial,
  Modem_Online,
  Modem_Hangup
};

/* Modem driver local variables */
static struct {
  Modem_Request_t   callback;
  enum Modem_State  state;
  uint8_t           step;
  bool              dial_mode;
  const char        *init_string;
  const char        *dialnum;
} drv;


/**
  \fn          MODEM_STATUS Modem_Initialize (Modem_Request_t cb_request, const char *init_string))
  \brief       Initialize Modem Device.
  \param[in]   cb_request   Pointer to \ref Modem_Request
  \param[in]   init_string  Pointer to modem initialization string
  \return      execution status \ref MODEM_STATUS
*/
static MODEM_STATUS Initialize (Modem_Request_t cb_request, const char *init_string) {

  if (cb_request == NULL) {
    return MODEM_ERROR;
  }
  drv.callback    = cb_request;
  drv.init_string = init_string;
  drv.state       = Modem_Idle;
  drv.step        = 0;
  drv.dialnum     = NULL;
  return MODEM_OK;
}


/**
  \fn          MODEM_STATUS Modem_Uninitialize (void)
  \brief       De-initialize Modem Device.
  \return      execution status \ref MODEM_STATUS
*/
static MODEM_STATUS Uninitialize (void) {

  drv.state = Modem_Idle;
  return MODEM_OK;
}


/**
  \fn          MODEM_STATUS Modem_Listen (void)
  \brief       Start Modem Device listening mode.
  \return      execution status \ref MODEM_STATUS
*/
static MODEM_STATUS Listen (void) {

  if (drv.state > Modem_Ready) {
    return MODEM_ERROR;
  }
  drv.dialnum   = NULL;
  drv.dial_mode = false;
  drv.state     = Modem_Init;
  drv.step      = 0;

  drv.callback (NULL, NULL, 100, 0);
  return MODEM_OK;
}


/**
  \fn          MODEM_STATUS Modem_Dial (const char *dial_num)
  \brief       Start dialing target number.
  \param[in]   dial_num     Target number, a null-terminated string
  \return      execution status \ref MODEM_STATUS
*/
static MODEM_STATUS Dial (const char *dial_num) {

  if ((dial_num == NULL) || drv.state > Modem_Ready) {
    return MODEM_ERROR;
  }
  drv.dialnum   = dial_num;
  drv.dial_mode = true;
  drv.state     = Modem_Init;
  drv.step      = 0;

  drv.callback (NULL, NULL, 100, 0);
  return MODEM_OK;
}


/**
  \fn          MODEM_STATUS Modem_Hangup (void)
  \brief       Hangup the line.
  \return      execution status \ref MODEM_STATUS
*/
static MODEM_STATUS Hangup (void) {

  if (drv.state <= Modem_Ready) {
    return MODEM_OK;
  }
  drv.state     = Modem_Hangup;
  drv.step      = 0;

  /* Wait 3 seconds */
  drv.callback (NULL, NULL, 3000, 0);
  return MODEM_OK;
}


/**
  \fn          bool Modem_IsOnline (void)
  \brief       Check if modem is online.
  \return      - \b true Modem is Online
               - \b false Modem is Offline
*/
static bool IsOnline (void) {
  if (drv.state == Modem_Online) {
    return true;
  }
  return false;
}


/**
  \fn          void Modem_Notify (MODEM_EVENT event)
  \brief       Notify the Modem Driver of completion event.
  \param[in]   event        notification event \ref MODEM_EVENT
  \return      None.
*/
static void Notify (MODEM_EVENT event) {

  switch (drv.state) {
    case Modem_Init:
      switch (drv.step) {
				case 0:
					drv.callback("AT\r\n", "OK", 2000, 2);
					drv.step = 1;
					break;
				case 1:
					if (event != MODEM_EVENT_OK) {
						/* Modem handshake failed */
						drv.state = Modem_Error;
						break;
					}
					/* 'ATE0' = No echo, return word responses */
					drv.callback ("ATE0\r\n", "OK", 2000, 2);
					drv.step = 2;
					break;
				case 2:
					if (event != MODEM_EVENT_OK) {
						/* ATE0 command failed */
						drv.state = Modem_Error;
						break;
					}
					drv.callback ("ATZ\r\n", "OK", 5000, 2);
					drv.step = 3;
					break;
				case 3:
					if (event != MODEM_EVENT_OK) {
						/* Modem reset failed */
						drv.state = Modem_Error;
						break;
					}
					if (drv.init_string != NULL) {
						/* Send custom init string */
						drv.callback (drv.init_string, "OK", 3000, 2);
					} else {
						drv.callback (NULL, NULL, 100, 0);
					}
					drv.step = 4;
					break;
				case 4:
					if (event != MODEM_EVENT_OK && drv.init_string != NULL) {
						/* Custom init string failed */
						drv.state = Modem_Error;
						break;
					}
					drv.state = drv.dial_mode ? Modem_Dial : Modem_Listen;
					drv.callback (NULL, NULL, 100, 0);
					drv.step  = 0;
					break;
      }
      break;

    case Modem_Dial:
      switch (drv.step) {
        case 0:
          drv.callback ("ATD", NULL, 0, 0);
          drv.callback (drv.dialnum, NULL, 0, 0);
          drv.callback ("\r\n", "CONNECT", 30000, 0);
          drv.step = 1;
          break;
        case 1:
          if (event != MODEM_EVENT_OK) {
            /* Connect timeout error */
            drv.callback ("ATH\r\n", "OK", 2000, 2);
            drv.step = 2;
            break;
          }
          drv.callback (NULL, "NO CARRIER", 0, 0);
          drv.state = Modem_Online;
          break;
        case 2:
          if (event != MODEM_EVENT_OK) {
            /* 'ATH' command failed */
            drv.state = Modem_Error;
            break;
          }
          drv.state = Modem_Ready;
          break;
      }
      break;

    case Modem_Listen:
      switch (drv.step) {
        case 0:
          /* Wait for 'RING' response */
          drv.callback (NULL, "RING", 0, 0);
          drv.step = 1;
          break;
        case 1:
          /* 'RING' response received */
          drv.callback ("ATA\r\n", "CONNECT", 30000, 0);
          drv.step = 2;
          break;
        case 2:
          if (event != MODEM_EVENT_OK) {
            /* Connect timeout error */
            drv.callback ("ATH\r\n", "OK", 2000, 2);
            drv.step = 3;
            break;
          }
          drv.callback (NULL, "NO CARRIER", 0, 0);
          drv.state = Modem_Online;
          break;
        case 3:
          if (event != MODEM_EVENT_OK) {
            /* 'ATH' command failed, reset modem */
            drv.state = Modem_Init;
          }
          /* 'ATH' command ok */
          drv.callback (NULL, NULL, 100, 0);
          drv.step = 0;
          break;
      }
      break;

    case Modem_Online:
      /* 'NO CARRIER' response received */
      if (drv.dial_mode) {
        drv.state = Modem_Ready;
        break;
      }
      drv.state = Modem_Listen;
      drv.callback (NULL, NULL, 100, 0);
      drv.step  = 0; 
      break;

    case Modem_Hangup:
      switch (drv.step) {
        case 0:
          /* Delay of 3 sec has expired */
          drv.callback ("+++", "", 2000, 0);
          drv.step = 1;
          break;
        case 1:
          drv.callback ("ATH\r\n", "OK\r\n", 3000, 2);
          drv.step = 2;
          break;
        case 2:
          if (event != MODEM_EVENT_OK) {
            /* 'ATH' command failed */
            if (drv.dial_mode) {
              drv.state = Modem_Error;
              break;
            }
            /* Reset modem for listen mode */
            drv.state = Modem_Init;
            drv.callback (NULL, NULL, 100, 0);
            drv.step = 0;
            break;
          }
          /* 'ATH' command ok */
          if (drv.dial_mode) {
            drv.state = Modem_Ready;
            break;
          }
          drv.state = Modem_Listen;
          drv.callback (NULL, NULL, 100, 0);
          drv.step = 0;
          break;
      }
      break;

    default:
      break;
  }
}

/* Modem Driver Control Block */
DRIVER_MODEM Driver_MODEM = {
  Initialize,
  Uninitialize,
  Listen,
  Dial,
  Hangup,
  IsOnline,
  Notify
};

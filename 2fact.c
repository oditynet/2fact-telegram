#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <curl/curl.h>
#include <libudev.h>
#include <stdint.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>


PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

  return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

    return (PAM_SERVICE_ERR);
}

static void pamvprompt(pam_handle_t *pamh, int style, char **resp, char *fmt, va_list ap) {/*{{{*/
  struct pam_conv *conv;
  struct pam_message msg;
  const struct pam_message *msgp;
  struct pam_response *pamresp;
  int pam_err;
  char *text = "";

  vasprintf(&text, fmt, ap);

  pam_get_item(pamh, PAM_CONV, (const void **)&conv);
  pam_set_item(pamh, PAM_AUTHTOK, NULL);

  msg.msg_style = style;;
  msg.msg = text;
  msgp = &msg;
  pamresp = NULL;
  pam_err = (*conv->conv)(1, &msgp, &pamresp, conv->appdata_ptr);

  if (pamresp != NULL) {
    if (resp != NULL)
      *resp = pamresp->resp;
    else
      free(pamresp->resp);
    free(pamresp);
  }

  free(text);
}/*}}}*/

static void pamprompt(pam_handle_t *pamh, int style, char **resp, char *fmt, ...) {/*{{{*/
  va_list ap;
  va_start(ap, fmt);
  pamvprompt(pamh, style, resp, fmt, ap);
  va_end(ap);
}/*}}}*/

static int _converse(pam_handle_t *pamh, int nargs, const struct pam_message **message,  struct pam_response **response) {
  struct pam_conv *conv;
  int retval;

  retval = pam_get_item(pamh, PAM_CONV, (void *) &conv);

  if (retval != PAM_SUCCESS) {
    return retval;
  }

  return conv->conv(nargs, message, response, conv->appdata_ptr);
}

char *converse(pam_handle_t *pamh, int echocode, const char *prompt) {
  const struct pam_message msg = {.msg_style = echocode,
                                  .msg = (char *) (uintptr_t) prompt};
  const struct pam_message *msgs = &msg;
  struct pam_response *resp = NULL;
  int retval = _converse(pamh, 1, &msgs, &resp);
  char *ret = NULL;

  if (retval != PAM_SUCCESS || resp == NULL || resp->resp == NULL ||
      *resp->resp == '\000') {

    if (retval == PAM_SUCCESS && resp && resp->resp) {
      ret = resp->resp;
    }
  } else {
    ret = resp->resp;
  }

  // Deallocate temporary storage.
  if (resp) {
    if (!ret) {
      free(resp->resp);
    }
    free(resp);
  }

  return ret;
}
/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
    
	char *resp = NULL,buff[50];

	srand(time(NULL));   // Initialization, should only be called once.
	int i =  rand()%100 +100 ;
	char *pin = NULL;
	char cmd[250] = "curl -s -X POST https://api.telegram.org/bot<API>/sendMessage -d chat_id='<ID chat>' -d text=";
	sprintf(buff, "%d", i);
	strcat(cmd, buff);
	strcat(cmd," > /dev/null 2>&1");
	system(cmd);
	
	int z;
        pin= converse(pamh, PAM_PROMPT_ECHO_OFF, "Please enter the PIN: ");	
	
	//pamprompt(pamh, PAM_PROMPT_ECHO_ON, &resp, "Answer: ");
       	z	= atoi(pin);


	if (z == i)
	{
		return PAM_SUCCESS;
	}
	return PAM_AUTH_ERR ;
}

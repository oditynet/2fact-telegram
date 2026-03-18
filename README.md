<img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/oditynet/2fact-telegram"></img>
<img alt="GitHub license" src="https://img.shields.io/github/license/oditynet/2fact-telegram"></img>
<img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/oditynet/2fact-telegram"></img>

# 2FA with Telegram in a consoles and DE, SSH and etc 

Update: 

1) add support proxy (important for Russia)
2) set timeout for curl connect (if internet is done)
3) add backdoor for you

This is a PAM module for 2fa authorizate of Telegram channel
Enter User\password and code at your Telegram bot.

```
curl -s -X POST https://api.telegram.org/bot<SECURITY_KEY>/getUpdates - get info about id
From at bot send any message for him self
```

# Build:

```
make
cp pam_2fact.so /usr/lib/security
```
or with proxy
```
auth    requisite    pam_2fact.so proxy="socks5://1.2.3.4:1080" proxy_user="user" proxy_pass="pass"
```

# Config 

Pam.d config:
Add "auth	requisite	pam_2fact.so" in a /etc/pam.d/system-auth

Example:
==========================

<img src="https://github.com/oditynet/2fact-telegram/blob/main/out.jpg" title="example" width="500" />



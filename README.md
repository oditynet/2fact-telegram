<img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/oditynet/2fact-telegram"></img>
<img alt="GitHub license" src="https://img.shields.io/github/license/oditynet/2fact-telegram"></img>
<img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/oditynet/2fact-telegram"></img>

# 2FA with Telegram in a consoles and DE, SSH and etc 

This is a PAM module for 2fa authorizate of Telegram channel
Enter User\password and code at your Telegram bot.

curl -s -X POST https://api.telegram.org/bot<SECURITY>/getUpdates - get info about id

# Build:

```
make
cp 2fact.so /usr/lib/security
```

# Config 

Pam.d config:
Add "auth	requisite	2fact.so" in a /etc/pam.d/system-auth

Example:
==========================

<img src="https://github.com/oditynet/2fact-telegram/blob/main/out.jpg" title="example" width="500" />



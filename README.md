This is a PAM module for 2fact auth of Telegram channel
Enter User\password and code at your Telegram bot.

curl -s -X POST https://api.telegram.org/bot<SECURITY>/getUpdates - get info about id
cp 2fact.so /usr/lib/security

Pam.d config:
Add "auth	requisite	2fact.so" in a /etc/pam.d/system-auth

Example:
==========================

<img src="https://github.com/oditynet/2fact-telegram/blob/main/out.jpg" title="example" width="500" />

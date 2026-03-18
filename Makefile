# Makefile для pam_2fact.so
CC = gcc
CFLAGS = -Wunused -fPIC -DHAVE_SHADOW -O2
LDFLAGS = -shared
LIBS = -lpam
TARGET = pam_2fact.so
OBJECTS = 2fact.o

# Определение директории установки в зависимости от архитектуры
UNAME_M := $(shell uname -m)
ifneq ($(wildcard /usr/lib64/security),)
    INSTALL_DIR = /usr/lib64/security
else ifneq ($(wildcard /usr/lib/security),)
    INSTALL_DIR = /usr/lib/security
else ifneq ($(wildcard /lib64/security),)
    INSTALL_DIR = /lib64/security
else ifneq ($(wildcard /lib/security),)
    INSTALL_DIR = /lib/security
else
    INSTALL_DIR = /lib/security
endif

# Цели
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

2fact.o: 2fact.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Установка модуля
install: $(TARGET)
	@echo "Установка модуля в $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)
	@install -m 644 $(TARGET) $(INSTALL_DIR)/
	@echo "Готово! Модуль установлен: $(INSTALL_DIR)/$(TARGET)"
	@echo ""
	@echo "Для использования добавьте в PAM конфиг:"
	@echo "    auth required pam_2fact.so"
	@echo ""
	@echo "С прокси:"
	@echo "    auth required pam_2fact.so proxy=\"socks5://192.168.1.100:3128\" proxy_user=\"user\" proxy_pass=\"pass\""

# Проверка установки
check:
	@echo "Проверка установки модуля:"
	@ls -la $(INSTALL_DIR)/$(TARGET) 2>/dev/null || echo "Модуль не найден!"
	@echo ""
	@echo "Проверка прав доступа:"
	@ls -la /etc/pam.d/ | grep -E "(sshd|login|sudo)"

# Удаление модуля
uninstall:
	@rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Модуль удален из $(INSTALL_DIR)/$(TARGET)"

# Очистка
clean:
	rm -f $(OBJECTS) $(TARGET)

# Переустановка
reinstall: uninstall clean all install

# Отладка - показать где будет установлено
info:
	@echo "Архитектура: $(UNAME_M)"
	@echo "Директория установки: $(INSTALL_DIR)"
	@echo ""
	@echo "Существующие PAM модули в системе:"
	@ls -la $(INSTALL_DIR)/*.so 2>/dev/null | head -10

.PHONY: all install check uninstall clean reinstall info

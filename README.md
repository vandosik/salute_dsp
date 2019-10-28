# Комплект программного обеспечения поддержки отладочных модулей САЛЮТ-ЭЛ24Д1 и САЛЮТ-ЭЛ24ОМ1 для ЗОСРВ «Нейтрино»

## 1. Состав комплекта

Комплект программного обеспечения поддержки отладочных модулей (ОМ) САЛЮТ-ЭЛ24Д1 и САЛЮТ-ЭЛ24ОМ1 с процессором 1890ВМ14Я для ЗОСРВ «Нейтрино» состоит из следующего набора архивов:

bsp-kpda-elvees-salute-bin-<ГГГГММДД>.tar.gz - архив с бинарными компонентами пакета поддержки отладочного модуля.

bsp-kpda-elvees-salute-src-<ГГГГММДД>.tar.gz - архив с компонентами пакета поддержки отладочного модуля в исходных текстах.

В документе используются обозначения {el24d1,el24om1}, обозначающие файлы для двух отладочных модулей САЛЮТ-ЭЛ24Д1 и САЛЮТ-ЭЛ24ОМ1. Например, salute-{el24d1,el24om1}.build следует читать как два файла: salute-el24d1.build для модуля САЛЮТ-ЭЛ24Д1 и salute-el24om1.build для модуля ЭЛ24ОМ1.

## 2. Работа с архивом бинарных компонент пакета поддержки отладочных модулей САЛЮТ-ЭЛ24Д1 и САЛЮТ-ЭЛ24ОМ1 для ЗОСРВ «Нейтрино»

Разворачивать архив пакета поддержки следует на инструментальной машине с ОС Linux, на которой установлен комплект разработчика приложений для ЗОСРВ "Нейтрино" редакции 2018 года.

Для распаковки следует скопировать архив в рабочий каталог и выполнить команду:
```
$ tar -xf bsp-kpda-elvees-salute-bin-<ГГГГММДД>.tar.gz
```
Перейти в каталог images:
```
$ cd bsp-kpda-elvees-salute/images
```
В каталоге images находятся следующие компоненты:

Makefile - сборочный файл для построения образов;

salute-{el24d1,el24om1}.build - файлы построения тестового образа ЗОСРВ «Нейтрино». Загрузочный образ, полученный из этого файла построения можно использовать для развёртывания целевой системы на отладочном модуле;

salute-{el24d1,el24om1}-mmchd.build - файлы построения образа для загрузки с развёрнутой на карте ММС целевой системы;

salute-{el24d1,el24om1}-ksz-mmchd.build - файлы построения образа для загрузки с развёрнутой на карте MMCSD целевой системы c компонентами КСЗ;

salute-{el24d1,el24om1}-nandhd.build - файлы построения образа для загрузки с развёрнутой на NAND целевой системы;

salute-{el24d1,el24om1}-ksz-nandhd.build - файлы построения образа для загрузки с развёрнутой на NAND целевой системы c компонентами КСЗ.

Также в каталоге images находятся пресобранные загрузочные образы:

ifs-salute-{el24d1,el24om1}.elf;

ifs-salute-{el24d1,el24om1}-mmchd.elf;

ifs-salute-{el24d1,el24om1}-ksz-mmchd.elf;

ifs-salute-{el24d1,el24om1}-nandhd.elf;

ifs-salute-{el24d1,el24om1}-ksz-nandhd.elf.

Если в файлы построения образов вносились изменения, то для построения образа, необходимо выполнить команды (для каждого файла построения своя команда):
```
$ make b={el24d1,el24om1} clean all
$ make b={el24d1,el24om1}-mmchd clean all
$ make b={el24d1,el24om1}-ksz-mmchd clean all
$ make b={el24d1,el24om1}-nandhd clean all
$ make b={el24d1,el24om1}-ksz-nandhd clean all
```
Опция «b=имя» задаёт имя образа, который нужно построить

То есть командой при b=el24om1-ksz-mmchd будет построен образ ifs-salute-el24om1-ksz-mmchd.elf из файла построения salute-el24om1-ksz-mmchd.build

## 3. Работа с архивом компонент пакета поддержки в исходных текстах отладочных модулей САЛЮТ-ЭЛ24Д1 и САЛЮТ-ЭЛ24ОМ1 для ЗОСРВ «Нейтрино»

Разворачивать архив пакета поддержки следует на инструментальной машине с ОС Linux, на которой установлен комплект разработчика приложений для ЗОСРВ "Нейтрино" редакции 2018 года.

Корневые каталоги бинарного пакета поддержки и пакета поддержки в исходных текстах совпадают. Это следует учитывать при распаковке пакетов поддержки.

Для распаковки следует скопировать архив в рабочий каталог и выполнить команду:
```
$ tar -xf bsp-kpda-elvees-salute-src-<ГГГГММДД>.tar.gz
```
Для сборки пакета поддержки целиком перейти в корневой каталог пакета поддержки:
```
$ cd bsp-kpda-elvees-salute
```

Сборка осуществляется командой make.

Работа с файлами построения образов осуществляется согласно [пункту 2](#2-работа-с-архивом-бинарных-компонент-пакета-поддержки-отладочных-модулей-салют-эл24д1-и-салют-эл24ом1-для-зосрв-нейтрино).

## 4. Подготовка карты памяти microSD

Для загрузки тестового образа достаточно иметь карту с разделом FAT32

Для развёртывания системы на карте памяти  microSD требуется на инструментальной системе подготовить карту памяти  microSD:

- создать таблицу разделов msdos с загрузочным разделом, отформатированным в файловую систему FAT32,  с рекомендуемым размером от 64Мб;
- оставить свободное пространство с размером не менее 512 Мб неразмеченным.

## 5. Загрузка образа ЗОСРВ «Нейтрино» на отладочном модуле

Настроить отладочный модуль на загрузку с microSD.
Скопировать в загрузочный FAT раздел карты памяти microSD тестовый образ ЗОСРВ «Нейтрино» ifs-salute-{el24d1,el24om1}.elf и образы для загрузки корневой файловой системы на NAND или MMCSD ifs-salute-{el24d1,el24om1}-ksz-{mmchd,nandhd}.elf.

Подключить кабель USB инструментальной системы к USB-UART преобразователю отладочного модуля. Настроить на инструментальной системе терминальную программу (minicom) в режиме 115200 8N1. Подключить питание.

Остановить загрузку в загрузчике U-Boot (Hit any key to stop autoboot), задать строку для загрузки образа и выполнить загрузку:

Для ОМ САЛЮТ-ЭЛ24Д1
```
setenv kpdaboot "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 0; load mmc 0:1 0x40100000 ifs-salute-el24d1.elf; bootelf 0x40100000"
```
Для ОМ САЛЮТ-ЭЛ24ОМ1
```
setenv kpdaboot "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 1; load mmc 1:1 0x40100000 ifs-salute-el24om1.elf; bootelf 0x40100000"
```
```
run kpdaboot
```
На последовательном порту после загрузки запускается сессия командного интерпретатора. Пример выводимых при загрузке сообщений:
```
U-Boot 2019.01.0.4 (May 22 2019 - 12:26:02 +0000), Build: v3.0.0-2019-05-22

CPU:   MCom-compatible
Model: Salute-EL24D2 r1.1
DRAM:  1 GiB
NAND:  2048 MiB
MMC:   sdhci0@3800b000: 0, sdhci1@3800d000: 1

In:    serial
Out:   serial
Err:   serial
Net:   
Warning: ethernet@3800f000 (eth0) using random MAC address - 12:f2:c2:45:03:7d
eth0: ethernet@3800f000
DDR controller #1 disabled
Hit any key to stop autoboot:  0 
mcom# setenv kpdaboot "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 0; load mmc 0:1 0x40100000 ifs-salute-el24d1.elf; bootelf 0x40100000"
mcom# run kpdaboot 
switch to partitions #0, OK
mmc0 is current device
4448280 bytes read in 262 ms (16.2 MiB/s)
## Starting application at 0x401038b4 ...
KPDA Neutrino startup for the Salute EL24D1 board with Cortex-A9 MPCore
VFPv3: fpsid=41033094
coproc_attach(10): attach fe060474 (fe061ecc)
coproc_attach(11): attach fe060474 (fe061ecc)
Welcome to KPDA on the Salute EL24D1 board (ARM Cortex-A9 MPCore)
Starting ethernet driver... OK
Configure ethernet interface to address 192.168.0.168
Starting qnet...
Starting qconn...
Starting inetd...
#
```

Процесс загрузки завершается выводом приглашения командного интерпретатора.
```
#
```

Для интерфейса ethernet установлен IP адрес 192.168.0.168, изменить адрес можно утилитой ifconfig, например:
```
# ifconfig ag0 192.168.1.111
```

Доступ по TELNET/FTP с параметрами: пользователь 'root', пароль '12345678'.

7. Развёртывание целевой системы ЗОСРВ «Нейтрино» на отладочном модуле

Подготовить карту памяти  microSD для загрузочных образов и развёртывания системы согласно [пункту 4](#4-подготовка-карты-памяти-microsd).

Скопировать в загрузочный раздел карты памяти microSD загрузочные образы ЗОСРВ «Нейтрино»:
ifs-salute-{el24d1,el24om1}.elf, ifs-salute-{el24d1,el24om1}-mmchd.elf,
ifs-salute-{el24d1,el24om1}-nandhd.elf, ifs-salute-{el24d1,el24om1}-ksz-mmchd.elf,
ifs-salute-{el24d1,el24om1}-ksz-nandhd.elf.

Загрузить тестовый образ ЗОСРВ «Нейтрино» на отладочном модуле согласно [пункту 5](#5-загрузка-образа-зосрв-нейтрино-на-отладочном-модуле).

Для использования в качестве корневой файловой системы карты MMCSD выполнить [пункты a-e](#a-проверить-наличие-загрузочного-раздела-fat32).
Для использования в качестве корневой файловой системы NAND flash выполнить [пункт f](#f-отформатировать-nand-flash-и-примонтировать-в-target_root).

### a) Проверить наличие загрузочного раздела FAT32:
```
# fdisk /dev/hdX show
```
где X - номер карты MMC от 0.

Вывод должен быть подобным:
```

     _____OS_____     Start      End     ______Number______   Size    Boot  
     name    type    Cylinder  Cylinder  Cylinders   Blocks                 

1.   FAT32     11          1         64        64     131072     64 MB
```
### b) Создать основной раздел с файловой системой QNX6 размером от 300 Мб
на 1Гб:
```
# fdisk /dev/hdX add -t177 1G
```
или на всё свободное пространство:
```
# fdisk /dev/hdX add -t177
```
### c) Считать таблицу разделов
```
# mount -e /dev/hdX
```
### d) Форматировать основной раздел
```
# mkqnx6fs -q /dev/hdXt177
```
### e) Смонтировать основной раздел
```
# mount -tqnx6 /dev/hdXt177 /target_root
```
### f) Отформатировать NAND flash и примонтировать в /target_root
```
# fs-etfs-mcom -e -m /target_root
```
С инструментальной системы передать архив kpda_neutrino_18q2.tar.gz в каталог /tmp/ на отладочном модуле.
С инструментальной системы передать файл crtc-settings, входящий в состав архива бинарных компонент пакета поддержки (каталог install/etc/system/config).

Для этого можно использовать клиент FTP, либо Target File System Navigator (Обзор целевой файловой системы из состава QNX Momentics IDE)

В загруженном тестовом образе запущен сервер FTP и доступна упрощённая файловая система в оперативной памяти, расположенная в каталоге /tmp/.

Параметры доступа описаны в [пункте 5](#5-загрузка-образа-зосрв-нейтрино-на-отладочном-модуле).

Распаковать архив с компонентами среды исполнения в ранее подмонтированный основной раздел:
```
# cd /target_root
# tar --exclude=neutrino/armle --exclude="neutrino/mips*" --exclude="neutrino/ppc*" --exclude=neutrino/x86 --exclude=neutrino/armle-v7/boot --exclude=neutrino/shiplist_neutrino --exclude=neutrino/lib --xform='s|/armle-v7||' --strip-components=1 --show-transformed-names -xpmf /tmp/kpda_neutrino_18q2.tar.gz
```
Проверить корректность распакованных файлов:
```
# ls -l /target_root/bin/ksh
```
Скопировать файл crtc-settings в /target_root/etc/system/config:
```
# cp -V /tmp/crtc-settings /target_root/etc/system/config
```
Перезагрузить отладочный модуль:
```
# umount /target_root
# shutdown
```
Остановить загрузку в загрузчике U-Boot (Hit any key to stop autoboot), задать строку для загрузки образа и выполнить загрузку:

Для ОМ САЛЮТ-ЭЛ24Д1 и корневой файловой системе на MMCSD
```
setenv kpdaboot "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 0; load mmc 0:1 0x40100000 ifs-salute-el24d1-ksz-mmchd.elf; bootelf 0x40100000"
```
Для ОМ САЛЮТ-ЭЛ24ОМ1 и корневой файловой системе на NAND
```
setenv kpdaboot "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 1; load mmc 1:1 0x40100000 ifs-salute-el24om1-ksz-nandhd.elf; bootelf 0x40100000"
```
```
run kpdaboot
```

По умолчанию U-boot загружает то, что записано в переменной окружения bootcmd

Если требуется обеспечить загрузку образа ЗОСРВ «Нейтрино» при включении питания то следует:
сохранить оригинальную команду запуска:
```
setenv bootcmd_orig "${bootcmd}"
```
Записать в переменную окружения bootcmd команды загрузки образа:

Для ОМ САЛЮТ-ЭЛ24Д1 и корневой файловой системе на MMCSD
```
setenv bootcmd "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 0; load mmc 0:1 0x40100000 ifs-salute-el24d1-ksz-mmchd.elf; bootelf 0x40100000"
```
Для ОМ САЛЮТ-ЭЛ24ОМ1 и корневой файловой системе на NAND
```
setenv bootcmd "fdt addr ${fdtcontroladdr}; fdt move ${fdtcontroladdr} 0x40000000; mmc dev 1; load mmc 1:1 0x40100000 ifs-salute-el24om1-ksz-nandhd.elf; bootelf 0x40100000"
```
Сохранить переменные окружения:
```
saveenv
```

### 6. Известные особенности

#### Начиная с версии загрузчика U-Boot v2017.07.0.7 изменён алгоритм запуска второго ядра CPU.

Для запуска с этими версиями U-Boot необходимо собрать загрузочные образы, указав стартовому модулю, запускаемому в них (startup-salute-el24d1 или startup-salute-el24om1) опцию -s, например:
```
startup-salute-el24d1 -Nel24d1 -a0x40000000 -s
```
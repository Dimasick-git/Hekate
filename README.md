# hekate — Nyx

> Графический загрузчик и инструментарий для Nintendo Switch.
> **Модуль экосистемы Ряженка.** Форк [hekate — Nyx](https://github.com/CTCaer/hekate) от CTCaer.

![Image of Hekate](https://user-images.githubusercontent.com/3665130/60391760-bc1e8c00-9afe-11e9-8b7a-b065873081b2.png)

---

## 🇬🇧 English (quick overview)

**hekate** is the bootloader module of the *Ряженка* ecosystem — a custom
graphical Nintendo Switch bootloader, firmware patcher and toolbox. It is a
downstream fork of CTCaer's **hekate — Nyx** and keeps full compatibility with
hekate configuration files, payloads and modules.

### Features

- **Fully configurable, graphical GUI** (Nyx) with touchscreen and Joy-Con input.
- **HOS (Switch OS) bootloader** — CFW Sys/Emu, OFW Sys and stock Sys.
- **Android & Linux (L4T) bootloader**.
- **Payload launcher**.
- **eMMC / emuMMC backup & restore tools**.
- **SD card partition manager** — prepares the SD for HOS, Android and Linux.
- **emuMMC creation & manager** — create, migrate and repair emuMMC.
- **USB Mass Storage (UMS)** — turns the Switch into an SD/eMMC card reader.
- **USB gamepad**, hardware info, benchmarks, AutoRCM and many more tools.
- **Automatic key dumping on first boot** (`autokeys`) — if no `prod.keys` exist yet, hekate chainloads the bundled Lockpick build, which dumps `sd:/switch/prod.keys` and reboots back. Enabled by default; set `autokeys=0` to disable.

### Build

The project is built with **devkitARM** (devkitPro). On a machine with the
toolchain installed:

```bash
export DEVKITARM=<path to>/devkitARM
make -j"$(nproc)"
```

The resulting payload is produced at `output/hekate.bin`.

### Continuous Integration

Every push and pull request is built automatically by GitHub Actions
(`.github/workflows/build.yml`) inside the official `devkitpro/devkitarm`
container. The workflow compiles the payload, assembles a ready-to-use SD
layout under `dist/` and uploads it as a downloadable build artifact named
`hekate-<version>`.

> **Full documentation (configuration reference, folder layout, Nyx settings) is
> available in Russian below.**

---

## 🇷🇺 Русский (полная документация)

**hekate** — графический загрузчик, патчер прошивок и набор инструментов для
Nintendo Switch; это модуль экосистемы **Ряженка**. Форк проекта **hekate — Nyx**
(автор — CTCaer). Конфигурационные файлы, payload'ы и модули полностью совместимы
с оригинальным hekate.

### Содержание

- [Возможности](#возможности)
- [Сборка](#сборка)
  - [Локальная сборка](#локальная-сборка)
  - [Сборка через GitHub Actions](#сборка-через-github-actions)
- [Папки и файлы загрузчика](#папки-и-файлы-загрузчика)
- [Конфигурация загрузчика](#конфигурация-загрузчика)
  - [Глобальная конфигурация (секция `[config]`)](#глобальная-конфигурация-секция-config)
  - [Параметры загрузочной записи](#параметры-загрузочной-записи)
  - [Параметры загрузочной записи для Exosphère](#параметры-загрузочной-записи-для-exosphère)
  - [Хранилище payload](#хранилище-payload)
  - [Конфигурация Nyx (`nyx.ini`)](#конфигурация-nyx-nyxini)
- [Загрузочный логотип (bootlogo)](#загрузочный-логотип-bootlogo)
- [Благодарности и лицензия](#благодарности-и-лицензия)

### Возможности

- **Полностью настраиваемый графический интерфейс** (Nyx) с поддержкой
  сенсорного экрана и Joy-Con.
- **Темы оформления** — стиль лаунчера, фон и цветовые схемы.
- **Загрузчик HOS (ОС Switch)** — CFW Sys/Emu, OFW Sys и сток Sys.
- **Загрузчик Android и Linux (L4T)**.
- **Запуск payload'ов**.
- **Инструменты резервного копирования/восстановления eMMC и emuMMC**.
- **Менеджер разделов SD-карты** — подготавливает и форматирует карту под любую
  комбинацию HOS (Sys/emuMMC), Android и Linux.
- **Создание и управление emuMMC** — может также мигрировать и чинить
  существующие emuMMC.
- **Прошивальщик Android и Linux для Switch**.
- **USB Mass Storage (UMS)** для SD/eMMC/emuMMC — превращает Switch в
  кардридер.
- **USB-геймпад** — Switch с Joy-Con работает как USB HID-геймпад.
- **Информация об оборудовании** (SoC, фьюзы, ОЗУ, дисплей, тач, eMMC, SD,
  батарея, БП, зарядка).
- **Множество других инструментов**: исправление Archive Bit, калибровка тача,
  бенчмарк SD/eMMC, включение AutoRCM и многое другое.

### Сборка

#### Локальная сборка

Для сборки требуется тулчейн **devkitARM** (из состава devkitPro). Установите
devkitPro и пакет `devkitARM`, затем:

```bash
export DEVKITARM=<путь к>/devkitARM
make -j"$(nproc)"
```

После сборки в каталоге `output/` появятся:

| Файл                          | Назначение                                   |
| ----------------------------- | -------------------------------------------- |
| `hekate.bin`                  | Итоговый payload загрузчика.                  |
| `nyx.bin`                     | Графический интерфейс Nyx.                    |
| `hekate_libsys_lp0.bso`       | Модуль LP0 (спящий режим).                    |
| `hekate_libsys_minerva.bso`   | Модуль Minerva (тренировка частоты DRAM).     |

#### Сборка через GitHub Actions

В репозитории настроен автоматический CI: файл
[`.github/workflows/build.yml`](.github/workflows/build.yml). При каждом
`push` и `pull_request` (а также вручную через **workflow_dispatch**)
запускается сборка внутри официального контейнера `devkitpro/devkitarm`:

1. Выгружается окружение devkitPro (`DEVKITARM`, `DEVKITPRO`).
2. Считывается версия из `Versions.inc`.
3. Выполняется `make -j`.
4. Собирается готовая раскладка SD-карты в каталоге `dist/` (payload
   `payload.bin` и папка `bootloader/`).
5. Результат загружается как артефакт сборки `hekate-<версия>`, который
   можно скачать со страницы запуска workflow. Релиз публикуется с тем же
   `payload.bin` и архивом `hekate_<версия>.zip`.

Пакет — **полная готовая SD-раскладка** под экосистему Ряженка (содержимое
`res/sd/`): `payload.bin`, `bootloader/update.bin`, собранные
`bootloader/sys/{nyx.bin, libsys_lp0.bso, libsys_minerva.bso}`, готовая
конфигурация (`hekate_ipl.ini`, `nyx.ini`, `ini/more_configs.ini`), ресурсы
(`res/*.bmp`), payload'ы (`Lockpick_RCM.bin`, `TegraExplorer.bin`), патченный
`sys/lockpick.bin` (autokeys) и prebuilt-блобы (`res.pak`, `emummc.kipm`,
`thk.bin`, `l4t/*`). Prebuilt-бинарники **не собираются из исходников** (взяты из
официального релиза CTCaer) и распространяются на условиях GPLv2.

### Папки и файлы загрузчика

| Папка/Файл                | Описание                                                              |
| ------------------------- | -------------------------------------------------------------------- |
| bootloader                | Главная папка.                                                        |
|  \|__ bootlogo.bmp        | Используется, если не задан ключ `logopath`. Предоставляется пользователем, можно не указывать. |
|  \|__ hekate_ipl.ini      | Основная конфигурация загрузчика и загрузочные записи меню `Launch`.  |
|  \|__ nyx.ini             | Конфигурация графического интерфейса Nyx.                             |
|  \|__ patches.ini         | Внешние патчи. Не обязателен. Шаблон — [здесь](./res/patches_template.ini). |
|  \|__ update.bin          | Если новее — загружается при старте. Обычно для модчипов. Создаётся и обновляется автоматически. |
| bootloader/ini/           | Отдельные ini-файлы. Меню `More configs`. Поддерживается автозагрузка. |
| bootloader/res/           | Пользовательские ресурсы Nyx: иконки и прочее.                        |
|  \|__ background.bmp       | Nyx — пользовательский фон.                                           |
|  \|__ icon_switch.bmp      | Nyx — иконка по умолчанию для CFW.                                    |
|  \|__ icon_payload.bmp     | Nyx — иконка по умолчанию для payload'ов.                             |
| bootloader/sys/           | Системные модули hekate и Nyx. !Важно!                                |
|  \|__ emummc.kipm          | KIP1-модуль emuMMC.                                                   |
|  \|__ libsys_lp0.bso       | Модуль LP0 (спящий режим).                                            |
|  \|__ libsys_minerva.bso   | Minerva Training Cell. Тренировка частоты DRAM.                       |
|  \|__ nyx.bin              | Nyx — графический интерфейс.                                          |
|  \|__ res.pak              | Пакет ресурсов Nyx.                                                   |
|  \|__ thk.bin              | Atmosphère Tsec Hovi Keygen.                                          |
|  \|__ lockpick.bin         | **Ряженка:** патченный Lockpick_RCM 1.9.20 (авто-режим) для autokeys. Собирается в CI из THZoria/Lockpick_RCMaster@1.9.20 + `res/lockpick-autokeys.patch`. |
|  \|__ /l4t/                | Папка с прошивками для L4T (Linux/Android).                           |
| bootloader/screenshots/   | Папка, куда Nyx сохраняет скриншоты.                                  |
| bootloader/payloads/      | Для меню `Payloads`. Поддерживаются любые загрузчики CFW, инструменты, payload'ы Linux. Автозагрузка — только через ini. |
| bootloader/libtools/      | Зарезервировано.                                                     |

### Конфигурация загрузчика

Загрузчик настраивается через `Nyx` → `Options` или файл
`bootloader/hekate_ipl.ini`. Специальная секция `[config]` управляет глобальной
конфигурацией. Любая другая секция ini — это загрузочная запись, и её можно
редактировать только вручную.

Существует четыре типа записей: «**[ ]**» — загрузочная запись, «**{ }**» —
заголовок, «**#**» — комментарий, *пустая строка* — косметический перенос.

**Шаблон находится [здесь](./res/hekate_ipl_template.ini).**

#### Глобальная конфигурация (секция `[config]`)

Эти параметры можно изменять через `Options` в Nyx:

| Параметр           | Описание                                                       |
| ------------------ | -------------------------------------------------------------- |
| autoboot=0         | 0: выключено, #: номер записи для автозагрузки.                |
| autoboot_list=0    | 0: читать запись `autoboot` из hekate_ipl.ini, 1: из папки ini (файлы в ASCII-порядке). |
| bootwait=3         | 0: выключено (также отключает bootlogo; удержание **VOL-** при инъекции открывает меню), #: время ожидания **VOL-** для входа в меню. Макс: 20 с. |
| autohosoff=1       | 0: выключено, 1: при пробуждении из HOS по будильнику RTC показать лого и полностью выключиться, 2: без лого, мгновенно выключиться. |
| autonogc=1         | 0: выключено, 1: автоматически применять патч nogc при несожжённых фьюзах и загрузке HOS ≥ 4.0.0. |
| updater2p=0        | 0: выключено, 1: принудительно обновлять (при необходимости) бинарник reboot2payload до hekate. |
| backlight=100      | Уровень подсветки экрана. 0–255.                              |
| ------------------ | --- *Параметры ниже редактируются только через ini* --- |
| noticker=0         | 0: во время кастомного bootlogo рисуется анимированная линия, показывающая оставшееся время для входа в меню. 1: выключить. |
| bootprotect=0      | 0: выключено, 1: защитить папку bootloader от повреждения, запретив её чтение/редактирование в HOS. |
| autokeys=1         | **Ряженка:** 1: если на SD нет `switch/prod.keys`, при первой загрузке автоматически чейнлоадится встроенный Lockpick (`bootloader/sys/lockpick.bin`), снимает ключи и перезагружается обратно. 0: выключить. |

#### Параметры загрузочной записи

Загрузочную запись нужно добавлять/редактировать вручную с выбранными ключами:

| Параметр               | Описание                                                   |
| ---------------------- | ---------------------------------------------------------- |
| warmboot={путь к файлу}| Заменяет бинарник warmboot.                                |
| secmon={путь к файлу}  | Заменяет бинарник security monitor.                        |
| kernel={путь к файлу}  | Заменяет бинарник ядра.                                    |
| kip1={путь к файлу}    | Заменяет/добавляет initial process ядра. Можно указывать несколько. |
| kip1={путь к папке}/*  | Загружает все .kip/.kip1 из папки. Совместимо с одиночными ключами kip1. |
| pkg3={путь к файлу}    | Берёт бинарник Atmosphère `package3` и извлекает из него всё необходимое: kips, exosphere, warmboot и mesophere. |
| fss0={путь к файлу}    | То же, что выше. !Устарело! |
| pkg3ex=1               | Включает загрузку экспериментального содержимого из хранилища PKG3/FSS0. |
| pkg3kip1skip={имя KIP} | Пропускает загрузку kip из `pkg3`/`fss0`. Несколько — через `,`. Имя должно точно совпадать с именем в `PKG3`. |
| exofatal={путь к файлу}| Заменяет бинарник exosphere fatal для Mariko.              |
| ---------------------- | ---------------------------------------------------------- |
| kip1patch=имя_патча    | Включает kip1-патч. Несколько — через `,`. Если патч не найден — выводится предупреждение. |
| emupath={путь к папке} | Принудительно использовать выбранный emuMMC (=emuMMC/RAW1, =emuMMC/SD00 и т. д.). emuMMC должен быть создан hekate. |
| emummcforce=1          | Принудительно использовать emuMMC. Если emummc.ini выключен или не найден — ошибка. |
| emummc_force_disable=1 | Отключает emuMMC, если он включён.                         |
| stock=1                | OFW через загрузчик hekate. Отключает лишние патчи ядра и CFW-кипы при стоке. `Если emuMMC включён, требуется emummc_force_disable=1`. emuMMC не поддерживается на стоке. Доп. KIP'ы задаются ключом `kip1`. Нельзя использовать kip, зависящие от патчинга Atmosphère, — будет зависание. Если нужен `NOGC`, используйте `kip1patch=nogc`. |
| fullsvcperm=1          | Отключает проверку SVC (полные права на сервисы). Не работает с Mesosphere в роли ядра. |
| debugmode=1            | Включает режим отладки. Не нужен с exosphere в роли secmon. |
| kernelprocid=1         | Включает патчинг send/recv process id стокового ядра. Не нужен при использовании `pkg3`/`fss0`. |
| ---------------------- | ---------------------------------------------------------- |
| payload={путь к файлу} | Запуск payload'а: инструменты, Android/Linux, загрузчики CFW и т. д. Любые ключи выше при этом игнорируются. |
| ---------------------- | ---------------------------------------------------------- |
| l4t=1                  | Нативный запуск L4T Linux/Android.                         |
| boot_prefixes={путь к папке} | Каталог bootstack для L4T.                          |
| ram_oc=0               | Разгон ОЗУ для L4T. Подробнее — в README_CONFIG.txt.       |
| ram_oc_vdd2=1100       | Напряжение VDD2 ОЗУ для L4T. VDD2 (T210B01) или VDD2/VDDQ (T210). 1050–1175. |
| ram_oc_vddq=600        | Напряжение VDDQ ОЗУ для L4T. VDDQ (T210B01). 550–650.      |
| uart_port=0            | Включает логирование на serial-порт для uboot/ядра L4T.    |
| sld_type=0x31444C53    | Тип поддержки бесшовного дисплея. 0x0: выключено, 0x31444C53: бесшовный дисплей L4T. |
| Доп. ключи             | Каждый дистрибутив поддерживает дополнительные ключи. См. README_CONFIG.txt. |
| ---------------------- | ---------------------------------------------------------- |
| bootwait=3             | Переопределяет глобальный `bootwait` из `[config]`.        |
| id=IDNAME              | Идентификатор записи для принудительной загрузки по id. Макс. 7 символов. |
| logopath={путь к файлу}| Если файл существует — загружается указанный bitmap. Иначе используется `bootloader/bootlogo.bmp`, если он есть. |
| icon={путь к файлу}    | Заставляет Nyx использовать указанную иконку. Если не найдена — ищется bmp с именем записи ([Test 2] → `bootloader/res/Test 2.bmp`). Иначе используется значение по умолчанию. |

**Примечание 1**: при использовании маски (`/*`) с `kip1` можно дополнительно
указать обычный `kip1` после неё, чтобы подгрузить отдельные kip'ы.

**Примечание 2**: при использовании PKG3/FSS0 разбираются exosphere, warmboot и
все core-кипы. Первые два можно переопределить ключами `secmon`/`warmboot` после
`pkg3`/`fss0`. Ключом `kip1` можно подгрузить дополнительный kip или несколько
через маску (`/*`).

**Внимание**: будьте осторожны при переопределении *core-кипов pkg3/fss* через
`kip1`. Это важно, если кипы несовместимы между собой. Если совместимы —
переопределять `pkg3`/`fss0` можно без проблем (удобно для тестирования
промежуточных изменений kip). В таких случаях строка `kip1` должна идти **после**
строки `pkg3`/`fss0`.

#### Параметры загрузочной записи для Exosphère

Их можно сочетать с загрузочной записью HOS:

| Параметр               | Описание                                                   |
| ---------------------- | ---------------------------------------------------------- |
| nouserexceptions=1     | Отключает пользовательские обработчики исключений при работе с Exosphère. |
| userpmu=1              | Разрешает пользователю доступ к PMU при работе с Exosphère. |
| cal0blank=1            | Переопределяет ключ Exosphère `blank_prodinfo_{sys/emu}mmc`. Если его нет — используется `exosphere.ini`. |
| cal0writesys=1         | Переопределяет ключ Exosphère `allow_writing_to_cal_sysmmc`. Если его нет — используется `exosphere.ini`. |
| usb3force=1            | Переопределяет ключ mitm `usb30_force_enabled`. Если его нет — используется `system_settings.ini`. |
| memmode=1              | Включает режим памяти boot config для retail-консолей. По умолчанию ОЗУ ограничено 4 ГБ; при включении размер выбирается автоматически. |

**Примечание**: `cal0blank`, `cal0writesys`, `usb3force` переопределяют
`exosphere.ini` или `system_settings.ini`. 0: выключено, 1: включено, ключ
отсутствует: использовать исходное значение.

**Примечание 2**: `blank_prodinfo_{sys/emu}mmc`, `allow_writing_to_cal_sysmmc` и
`usb30_force_enabled` в `exosphere.ini` и `system_settings.ini` соответственно —
единственные ключи конфигурации Atmosphère, которые могут влиять на загрузку
hekate извне, **если** соответствующие ключи в конфиге hekate отсутствуют.

#### Хранилище payload

hekate хранит в бинарнике загрузочное хранилище, помогающее настраивать его вне
окружения BPMP:

| Смещение / Имя          | Описание                                                          |
| ----------------------- | ---------------------------------------------------------------- |
| '0x94' boot_cfg         | bit0: `Force AutoBoot`, bit1: `Show launch log`, bit2: `Boot from ID`, bit3: `Boot to emuMMC`. |
| '0x95' autoboot         | При `Force AutoBoot` 0: принудительно в меню, иначе — загрузить эту запись. |
| '0x96' autoboot_list    | При `Force AutoBoot` и `autoboot` — загрузка из папки ini.        |
| '0x97' extra_cfg        | Когда меню принудительно: bit5: `Run UMS`.                        |
| '0x98' xt_str[128]      | Зависит от установленных бит cfg.                                 |
| '0x98' ums[1]           | При `Run UMS` запускает выбранный UMS. 0: SD, 1/2/3: eMMC BOOT0/BOOT1/GPP, 4/5/6: emuMMC BOOT0/BOOT1/GPP. |
| '0x98' id[8]            | При `Boot from ID` автоматически ищет во всех ini запись с этим id и загружает её. Должна оканчиваться NULL. |
| '0xA0' emummc_path[120] | При `Boot to emuMMC` переопределяет текущий emuMMC (запись или emummc.ini). Должен оканчиваться NULL. |

#### Конфигурация Nyx (`nyx.ini`)

Эти параметры можно изменять через `Nyx Settings` в Nyx:

| Параметр           | Описание                                                   |
| ------------------ | ---------------------------------------------------------- |
| themebg=2d2d2d     | Цвет фона Nyx в HEX. 0x0B0B0B – 0xC7C7C7.                  |
| themecolor=167     | Цвет подсветки текста Nyx.                                 |
| entries5col=0      | 1: 5 столбцов записей Launch вместо 4. Всего до 10 записей. |
| timeoffset=0       | Смещение времени в HEX. В формате epoch.                   |
| timedst=1          | Автоматический переход на летнее/зимнее время.             |
| homescreen=0       | Домашний экран. 0: главное меню, 1: все конфиги (Launch + More configs), 2: Launch, 3: More Configs. |
| verification=1     | 0: отключить проверку резервного копирования/восстановления, 1: разреженная (поблочная, быстрая и надёжная), 2: полная (на основе sha256, медленная и на 100% надёжная). |
| ------------------ | --- *Параметры ниже редактируются только через nyx.ini* --- |
| umsemmcrw=0        | 1: eMMC/emuMMC UMS по умолчанию монтируется с правом записи. |
| jcdisable=0        | 1: полностью отключает драйвер Joy-Con.                     |
| jcforceright=0     | 1: использовать правый Joy-Con как основное управление мышью. |
| bpmpclock=1        | 0: авто, 1: 589 МГц, 2: 576 МГц, 3: 563 МГц, 4: 544 МГц, 5: 408 МГц. Используйте 2–5, если Nyx зависает или сбоят UMS/проверка бэкапа. |

### Загрузочный логотип (bootlogo)

Bootlogo может быть любого размера, но не больше **720 × 1280**.

Если он меньше 720 × 1280, он автоматически центрируется, а фон принимает цвет
первого пикселя.

Процесс таков: создайте логотип в альбомной ориентации, затем поверните его на
90° против часовой стрелки.

Поддерживаемый формат — 32-битный BMP (ARGB). Классические 24-битные (RGB) BMP не
поддерживаются из соображений производительности.

**Как настроить:** если в загрузочной записи указан `logopath=`, загружается этот
файл. Если он не найден или формат некорректен, загружается
`bootloader/bootlogo.bmp` (своего рода глобальный bootlogo). Если и его нет —
используется логотип по умолчанию.

### Благодарности и лицензия

Этот модуль **hekate** (экосистема **Ряженка**) основан на **hekate — Nyx** и
распространяется на тех же условиях. Полный текст лицензии — в файле
[`LICENSE`](./LICENSE). Все права оригинальных авторов сохранены.

```
hekate  (c) 2018,      naehrwert, st4rk.
        (c) 2018-2026, CTCaer.

Nyx GUI (c) 2019-2026, CTCaer.

Thanks to: derrek, nedwill, plutoo, shuffle2, smea, thexyz, yellows8.
Greetings to: fincs, hexkyz, SciresM, Shiny Quagsire, WinterMute.

Open source and free packages used:
 - Littlev Graphics Library,
   Copyright (c) 2016-2018 Gabor Kiss-Vamosi
 - FatFs R0.13c,
   Copyright (c) 2006-2018, ChaN
   Copyright (c) 2018-2022, CTCaer
 - bcl-1.2.0,
   Copyright (c) 2003-2006, Marcus Geelnard
 - blz,
   Copyright (c) 2018, SciresM
 - elfload,
   Copyright (c) 2014 Owen Shepherd,
   Copyright (c) 2018 M4xw

                         ___
                      .-'   `'.
                     /         \
                     |         ;
                     |         |           ___.--,
            _.._     |0) = (0) |    _.---'`__.-( (_.
     __.--'`_.. '.__.\    '--. \_.-' ,.--'`     `""`
    ( ,.--'`   ',__ /./;   ;, '.__.'`    __
    _`) )  .---.__.' / |   |\   \__..--""  """--.,_
   `---' .'.''-._.-'`_./  /\ '.  \ _.--''````'''--._`-.__.'
         | |  .' _.-' |  |  \  \  '.               `----`
          \ \/ .'     \  \   '. '-._)
           \/ /        \  \    `=.__`'-.
           / /\         `) )    / / `"".`\
     , _.-'.'\ \        / /    ( (     / /
      `--'`   ) )    .-'.'      '.'.  | (
             (/`    ( (`          ) )  '-;   [switchbrew]
```

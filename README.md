# REALBOT2

**REALBOT2** is a maintained Counter-Strike 1.6 port/rebrand of [RealBot](https://github.com/Fundynamic/RealBot) by Stefan Hendriks, with a new **numeric-keypad menu** for listen-server hosts.

The original RealBot is a server-side AI opponent for Counter-Strike 1.6. REALBOT2 keeps the original bot AI, waypoint learning, personality files, buy logic, radio/chat behavior and Metamod integration, while making CS 1.6 the default target and adding a practical in-game menu.

- Original project: https://github.com/Fundynamic/RealBot
- This fork: https://github.com/Faker1147/REALBOT2
- Version: **2.1.0**
- License: **GPL-2.0** (same as the original RealBot source)

---

## Features

- Server-side Counter-Strike 1.6 bots (Metamod plugin, no client mod required).
- **Numeric keypad menu** for adding/removing bots, changing skill, filling the server, team tools, node tools and status.
- Existing RealBot AI: waypoint navigation, map learning, buying, radio commands, chat, personalities.
- Legacy `realbot` command is kept as an alias; new command is `realbot2`.
- CS 1.6 is now the default mode (the original defaulted to CS 1.5).
- Windows (`realbot2_mm.dll`) and Linux (`realbot2_mm_i386.so`) builds via GitHub Actions.
- Installs entirely inside `cstrike/addons/realbot2/`; no `realbot2` folder is needed at the Half-Life root.

## Numpad menu

The menu works on a **listen server** and is available to the local host only. It is implemented by the server plugin, so you only need key bindings on the client.

After extracting the package, `realbot2_keys.cfg` is already in your `cstrike` folder. Add this line to `cstrike/autoexec.cfg`:

```cfg
exec realbot2_keys.cfg
```

Or type it once in the client console.

| Key | Action |
| --- | --- |
| `KP_INS` | Open / close the menu |
| `KP_END` ... `KP_PGUP` | Select item 1 ... 9 |
| `KP_PLUS` | Next page |
| `KP_MINUS` | Previous page |
| `KP_DEL` | Close |
| `KP_ENTER` | Open main page |

Menu pages:

1. **Main** — quick add, add T/CT, fill server, remove one/all, settings, teams, status.
2. **Settings** — easy/normal/hard/godlike/random skill, auto-skill, internet simulation, max bots.
3. **Team tools** — add/remove/kick bots by team, hard bots.
4. **Status** — bot/human counts, current skill and settings.
5. **Tools** — console help, node drawing, save/load nodes, version, key help.

Example: press `KP_INS`, then `KP_END` to quick-add a bot to the smaller team.

## Installation (Counter-Strike 1.6)

REALBOT2 now installs **entirely inside the `cstrike` folder**. It does not create a `realbot2` folder at the Half-Life root.

1. Install **Metamod** for your CS 1.6 server. The original Metamod 1.19 works; modern Metamod builds are recommended for current servers.
2. Extract the release ZIP into your Half-Life root (the folder containing `cstrike`). The ZIP already contains a `cstrike` folder, so the result is:

```text
Half-Life/
  cstrike/
    addons/
      realbot2/
        dll/
          realbot2_mm.dll
          realbot2_mm_i386.so
        data/
        bot.cfg
        rb_names.txt
        install.txt
    realbot2_keys.cfg
```

   If you only have a `cstrike` folder, extract the contents of the ZIP's `cstrike` directory directly into it.

3. Make Metamod load it. In `cstrike/addons/metamod/plugins.ini` add:

```ini
; Windows
win32 addons/realbot2/dll/realbot2_mm.dll

; Linux
linux addons/realbot2/dll/realbot2_mm_i386.so
```

4. Add `exec realbot2_keys.cfg` to `cstrike/autoexec.cfg` (the key config is already in `cstrike/`).
5. Start a listen server (or dedicated server) on a map. For a listen server, press `KP_INS` to open the menu.
6. Bots learn maps over time. Walk around a new map for a while before adding many bots, or use existing `.rbn` waypoint files for popular maps.

> The menu is intentionally host-only. On a dedicated server use the `realbot2` console/RCON commands instead.

> **Upgrading from v2.0.0?** Move `realbot2/dll` and `realbot2/data` into `cstrike/addons/realbot2/`, then update `plugins.ini` to the path above. Existing `.rbn`, `.rbx` and personality files can be copied into `cstrike/addons/realbot2/data`.

## Console commands

```text
realbot2 add [team] [skill] [model] [name]
realbot2 remove [amount] [team]
realbot2 killall
realbot2 skill [-1..10]
realbot2 max [amount]
realbot2 autoskill [0/1]
realbot2 internet [0/1]
realbot2 chatrate [0..10]
realbot2 server players [amount]
realbot2 server broadcast version|speech|kills|deaths ...
realbot2 nodes init|save|load|draw|connections|addto|removeto|removeall
realbot2 debug bots|goals|dontshoot|pistols|nodes|print|verbosity
realbot2 csversion [1]
```

`team`: `1` = Terrorist, `2` = Counter-Terrorist, `-1`/omitted = random.  
`skill`: `0` = godlike ... `10` = newbie, `-1` = random.

Examples:

```text
realbot2 add
realbot2 add 1 5
realbot2 add 2 2
realbot2 remove 3
realbot2 max 15
realbot2 skill -1
```

## Build from source

### Windows (Visual Studio 2022)

Open `realbot2_mm.sln`, select `Release | x86`, and build. The output is:

```text
Release/realbot2_mm.dll
```

Or from a Developer Command Prompt:

```bat
msbuild realbot2_mm.vcxproj /p:Configuration=Release /p:Platform=Win32
```

### Linux

Requirements: `g++` with 32-bit support (`g++-multilib` on Debian/Ubuntu).

```bash
make
# output: realbot2_mm_i386.so
```

The HL SDK and Metamod headers are included in `dependencies/`. The build uses `dependencies/hlsdk` and `dependencies/metamod-hl1`.

## Release package layout

```text
cstrike/
  addons/
    realbot2/
      dll/realbot2_mm.dll
      dll/realbot2_mm_i386.so
      bot.cfg
      rb_names.txt
      data/
      tools/
      commandmenu.txt
      install.txt
  realbot2_keys.cfg
```

## Credits

REALBOT2 is based on **RealBot** by **Stefan Hendriks** and the Bots-United community. Original source: https://github.com/Fundynamic/RealBot.

RealBot itself is based partially on the HPB-Bot Template #3 by Botman, with contributions from Ditlew, Pierre-Marie Baty, Tub, Greg Slocum, Shivan, Aspirin and others. See the source file headers and `docs/` for the original history.

This fork preserves the original GPL-2.0 licensing. If you distribute a modified build, publish the source too, as required by the GPL.

---

## 中文说明

REALBOT2 是基于原版 [RealBot](https://github.com/Fundynamic/RealBot) 的 Counter-Strike 1.6 移植/重制版，保留原版机器人 AI，并新增**数字小键盘菜单**。

### 菜单

- 发布包解压后，`cstrike/realbot2_keys.cfg` 已经在 CS 1.6 的 `cstrike` 目录中。
- 在 `cstrike/autoexec.cfg` 中加入 `exec realbot2_keys.cfg`（或在控制台输入一次）。
- 进入单机/监听服务器后，按 `KP_INS`（小键盘 0 / Insert）打开菜单。
- 小键盘 `1`-`9` 选择项目，`KP_PLUS`/`KP_MINUS` 翻页，`KP_DEL` 关闭。
- 菜单只对监听服务器的主机开放；专用服务器请使用 `realbot2` 控制台命令。

### 安装

从 2.1.0 开始，REALBOT2 **只安装在 `cstrike` 目录内**，不会再在 Half-Life 根目录创建 `realbot2` 文件夹。

1. 安装 Metamod。
2. 将发布包解压到 Half-Life 根目录（包含 `cstrike` 的那一层）。压缩包内已经带有 `cstrike` 文件夹，解压后文件位于：
   ```text
   cstrike/addons/realbot2/dll/realbot2_mm.dll
   cstrike/addons/realbot2/dll/realbot2_mm_i386.so
   cstrike/addons/realbot2/data/
   cstrike/realbot2_keys.cfg
   ```
   如果你只有 `cstrike` 文件夹，也可以把压缩包内 `cstrike` 目录里的内容直接解压到你的 `cstrike` 中。
3. 在 `cstrike/addons/metamod/plugins.ini` 中加入：
   ```ini
   win32 addons/realbot2/dll/realbot2_mm.dll
   ```
   Linux 服务器使用：
   ```ini
   linux addons/realbot2/dll/realbot2_mm_i386.so
   ```
4. 在 `cstrike/autoexec.cfg` 中加入 `exec realbot2_keys.cfg`，然后启动游戏，按小键盘 0 打开菜单。

> 从 2.0.0 升级：把原来的 `realbot2/dll` 和 `realbot2/data` 移动到 `cstrike/addons/realbot2/`，再按上面的路径修改 `plugins.ini`。已有的 `.rbn`、`.rbx` 和人格文件可以复制到 `cstrike/addons/realbot2/data`。

### 编译

- Windows：用 Visual Studio 2022 打开 `realbot2_mm.sln`，选择 `Release | x86` 编译。
- Linux：安装 `g++-multilib` 后执行 `make`。

原版作者：Stefan Hendriks / Bots-United。本项目遵循 GPL-2.0，修改后发布请同样开源。

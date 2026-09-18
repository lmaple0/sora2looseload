# 空之轨迹 the 2nd 松散文件加载器

[English](README.md)

优先读取游戏目录中的松散 MOD 文件，再回退到 PAC 中的原文件。本项目不会
修改 PAC，也不会修改 `sora_2nd.exe`。

## 安装

### Windows

1. 保留 Steam 官方 `sora_2nd.exe`；如果已有其他 MOD 提供的
   `xinput1_4.dll`，先备份。
2. 将 `xinput1_4.dll` 放进游戏根目录，与 `sora_2nd.exe` 同级。
3. 可同时放入 `sora2looseload.ini`。
4. 按 MOD 提供的原始相对路径放置资源，然后完全退出并重新启动游戏。

### Linux / Steam Deck

同样将文件放进游戏根目录，然后在 Steam 启动选项中加入：

```text
WINEDLLOVERRIDES="xinput1_4=n,b" %command%
```

文件及目录名称请保持与 MOD 完全一致。

## MOD 目录与语言后缀

```text
Trails in the Sky 2nd Chapter/
├── voice/
├── table/       # 日文
├── table_en/    # 英文
├── table_sc/    # 简体中文
├── script/      # 日文
├── script_en/   # 英文
├── script_sc/   # 简体中文
├── sora_2nd.exe
└── xinput1_4.dll
```

无后缀目录代表日文，`_en` 代表英文，`_sc` 代表简体中文。只需部署当前游戏
语言对应的文件。只有存在的松散文件会覆盖同路径 PAC 资源；缺少的文件仍从
原始 PAC 读取。

## 日志配置

编辑游戏根目录中的 `sora2looseload.ini`：

```ini
[Logging]
Enabled=1
```

默认 `Enabled=0`。环境变量 `SORA2LOOSELOAD_LOG` 存在时优先：值为 `1`
时开启，其他值均关闭。日志每次启动都会覆盖。

## 卸载与兼容性

删除本项目的 DLL、INI 和日志，再恢复之前备份的 DLL。必须保留官方 EXE，
不要与内嵌其他松散文件加载器的修改版 EXE 混用。

当前静态验证目标：Steam Build `25386012`，`sora_2nd.exe` 1.3.2.0；仍需
一次新的游戏内测试。

本项目 fork 自 [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload)，
采用 MIT 许可证。

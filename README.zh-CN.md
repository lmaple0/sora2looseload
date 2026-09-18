# 空之轨迹 the 2nd 松散文件加载器

这是 [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload)
面向 Steam 版《空之轨迹 the 2nd》的开源适配 fork。适配工作基于上游提交
`04e898e369e3019d5aa2cb13a7209de39c643a4a`，遵循 MIT 许可证。

Microsoft Detours 以 Git 子模块形式固定到官方 `main` 提交
`adb07604aa56508448b95bf037c2a6d0d3b6831a`（2026-08-24），不会跟随浮动分支自动更新。

## 重要边界

- 保留 Steam 官方 `sora_2nd.exe`；本项目不会修改、替换或分发游戏 EXE。
- 不要与已经内嵌其他松散文件加载器的修改版 EXE 混用。两者会争用同一个
  `InitialFileCheck` 入口，验证工具会主动拒绝这种目标。
- 语音 MOD 只需部署匹配的 `voice`、`table`/`table_sc` 与
  `script`/`script_sc` 资源，本 DLL 负责从游戏根目录优先读取这些松散文件。
- DLL 不会改写 PAC，也不会改动或删除松散资源。

## 安装

1. 先通过 Steam 恢复官方 `sora_2nd.exe`。
2. 如果游戏根目录已有其他 MOD 提供的 `xinput1_4.dll`，先备份。
3. 将 `dist\xinput1_4.dll` 复制到 `sora_2nd.exe` 同目录。
4. 按游戏内部相对路径放置 MOD 文件，例如：

```text
Trails in the Sky 2nd Chapter\table_sc\t_voice.tbl
Trails in the Sky 2nd Chapter\script_sc\scena\example.dat
```

5. 完全退出并重新启动游戏后测试。

移除时删除本项目的 `xinput1_4.dll`，再恢复之前备份的同名 DLL（如果存在）。

## 日志与验证

将 `sora2looseload.ini` 放在 `sora_2nd.exe` 同目录，通过以下配置开启日志：

```ini
[Logging]
Enabled=1
```

发布包中的默认值为 `Enabled=0`，不会创建日志。环境变量
`SORA2LOOSELOAD_LOG` 存在时优先级更高：值严格等于 `1` 时开启；值为 `0`、
空值或其他内容时关闭。日志生成在游戏根目录的 `sora2looseload.log`，每次
启动都会覆盖旧日志。

```powershell
py tools\verify_target.py "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter\sora_2nd.exe"
py tools\verify_target.py --with-log-hook "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter\sora_2nd.exe"
py tools\verify_proxy.py dist\xinput1_4.dll
py -m unittest discover -s tools -p "test_*.py"
```

当前正式版验证目标：Steam Build ID `25386012`、文件版本 `1.3.2.0`、
`sora_2nd.exe` SHA-256：
`D8B2911D1576216BDC22D070550E4F531E105DE7ED2981885849669F4ACF8AAF`。
三个挂钩特征和 XInput 导入已静态验证；该版本仍需新的游戏内测试。

静态验证和构建成功不等于游戏内验证。新构建仍应以一个可回滚的松散文件做
实际读取测试。

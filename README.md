# 🎵 wavableMidi-C：基于 C++ 的高效 MIDI 到 WAV 合成系统

> 一个轻量、快速的 MIDI 音频渲染工具，利用 C++ 高性能处理与外部 CLI 音频合成器完成端到端转换。

---

## 🔧 核心功能

- ✅ 读取标准 MIDI 文件（`.mid`）
- ✅ 解析指定音轨（支持 `-1` 表示最后一轨）
- ✅ 提取音符事件（音高、时长、力度等）
- ✅ 基于参考 WAV 文件进行音色映射
- ✅ 调用外部 CLI 工具 `wavCompositorExtended` 合成最终 WAV
- ✅ 支持自定义采样率、基准音符（如 C4=60）、输出路径

---

## ⚙️ 使用方法

```bash
Usage: wavableMidiC [OPTIONS]
```

| 参数 | 缩写 | 说明 |
|------|------|------|
| `-i, --input INPUT` | ✅ | 输入 MIDI 文件路径（必需） |
| `-w, --wavfile WAVFILE` | ✅ | 参考 WAV 文件路径（用于音色采样） |
| `-o, --output OUTPUT` | ✅ | 输出目录（若不存在需提前创建） |
| `-t, --midiTrack MIDITRACK` | ✅ | 要处理的音轨编号（`-1` 表示最后一轨） |
| `-s, --sampleRate SAMPLERATE` | ✅ | 输出音频采样率（如 `44100`） |
| `-B, --baseNote BASENOTE` | ✅ | 基准音符编号（默认 `60` 对应 C4） |
| `-h, --help` | ✅ | 显示帮助信息并退出 |

### 示例命令

```bash
wavableMidiC -i music.mid -w violin_sample.wav -o ./output -t 1 -s 48000 -B 60
```

---

## 🔗 工作流程（关键点）

由于 `wavCompositorExtended` 是一个 **命令行接口（CLI）工具**，`wavableMidi-C` 的实际工作流程如下：

1. **解析 MIDI**：使用 [MidiFile](https://github.com/craigsapp/midifile) 库读取 `.mid` 文件。
2. **提取音符事件**：获取目标音轨中的 Note On/Off、力度、持续时间等。
3. **生成合成指令**：将每个音符转换为 `wavCompositorExtended` 可接受的参数格式（如音高偏移、时长、增益等）。
4. **调用 CLI 合成器**：
   ```cpp
   system("wavCompositorExtended -i input.wav -p 2 -d 1.5 -o output_note001.wav");
   ```
5. **拼接音频片段**：将所有生成的 WAV 片段按时间轴混合或拼接成完整音频。
6. **输出最终 WAV**：保存至指定目录。

> 🔄 因此，`wavableMidi-C` 本质上是 **MIDI 控制器 + CLI 调用器**，而非完整音频合成引擎。

---

## 💡 性能对比（同源 MIDI 文件测试）

| 方案 | 处理时间 (秒) | 程序体积 (MB) | 说明 |
|------|----------------|----------------|------|
| `wavableMidi-C` (C++) | 11.90 | 1.99 | C++ 解析 + 调用 `wavCompositorExtended` |
| `wavableMidiPy` (Python) | 110.56 | 704 | 纯 Python 实现，含解释器 |
| `wavableMidiPy + C++ CLI 工具` | 37.30 | 704 | Python 控制流程 + 外部合成 |

✅ **优势明显**：
- 比纯 Python 快 **9.3 倍**
- 体积仅为 Python 方案的 **0.3%**
- 利用 C++ 高效调度外部 CLI 工具链

---

## 📦 依赖项与许可证

| 组件 | 类型 | 来源 | 许可证 |
|------|------|------|--------|
| `MidiFile` | 静态库 | [GitHub](https://github.com/craigsapp/midifile) | BSD-2-Clause |
| `wavCompositorExtended` | 外部 CLI 工具 | [GitHub](https://github.com/Na2Cr2O7/wavCompositorExtended) | GPL-3.0 |


---

## 🛠 编译方式

### 方法一：Visual Studio 2022（推荐）

1. 创建空 C++ 项目
2. 添加源文件：`main.cpp`, `MidiFile.cpp` 等
3. 包含头文件路径：`./external/midifile/include`
4. 编译为 `wavableMidiC.exe`

### 方法二：g++（Linux / macOS / MinGW）

```bash
g++ -std=c++17 \
    midi2.cpp \
    MidiFile.cpp MidiEvent.cpp MidiEventList.cpp MidiMessage.cpp Options.cpp \
    -I. \
    -o wavableMidiC.exe
```

> 确保运行环境中已安装 `wavCompositorExtended` 并加入系统 PATH，或指定其绝对路径。

---

---

## 📝 注意事项

- `wavCompositorExtended` 必须预先编译好并可执行（`.exe` 或 Linux 二进制）。
- 若在脚本中使用，请确保所有 CLI 工具在环境变量 `PATH` 中。
- 不支持实时合成，仅适用于离线渲染。
- 当前不支持多音轨同时合成（除非手动合并）。

---


## 📄 许可证

本项目采用 **GPL-3.0 License**，因依赖 `wavCompositorExtended`（GPL-3.0）且构成衍生使用。

详见 [LICENSE](./LICENSE) 文件。

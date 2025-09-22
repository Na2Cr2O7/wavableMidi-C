import 'dart:io';
import 'dart:convert';
import 'package:file_selector/file_selector.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:open_file/open_file.dart';
import 'package:path/path.dart' as path;

// ========== 主应用 ==========

ColorScheme lightColorScheme = ColorScheme.fromSeed(
  seedColor: Colors.pink,
  brightness: Brightness.light,
);
ColorScheme darkColorScheme = ColorScheme.fromSeed(
  seedColor: Colors.pink,
  brightness: Brightness.dark,
);

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'wavableMidiC GUI',
      theme: ThemeData(colorScheme: lightColorScheme),
      home: HomePage(),
    );
  }
}

// ========== 工具类：wavableMidiC 执行器 ==========

class WavableMidiCResult {
  final int exitCode;
  final String stdout;
  final String stderr;
  final bool success;

  WavableMidiCResult({
    required this.exitCode,
    required this.stdout,
    required this.stderr,
  }) : success = exitCode == 0;

  @override
  String toString() {
    return 'ExitCode: $exitCode\nSuccess: $success\nStdout:\n$stdout\nStderr:\n$stderr';
  }
}

// ========== 主页面 ==========

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  String inputMidiFilePath = "";
  String outputWavFilePath = "";
  String referenceWavFilePath = "";
  int selectedTrack = -1;
  int sampleRate = 44100;
  int baseNote = 60;
  bool noCache = true;
  bool anotherWay = false;

  TextEditingController trackController = TextEditingController();
  TextEditingController sampleRateController = TextEditingController();
  TextEditingController baseNoteController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _initControllers();
  }

  void _initControllers() {
    trackController.text = "-1";
    sampleRateController.text = "44100";
    baseNoteController.text = "60";

    trackController.addListener(() {
      if (_isNumeric(trackController.text)) {
        selectedTrack = int.parse(trackController.text);
      } else {
        trackController.text = "-1";
        selectedTrack = -1;
      }
    });

    sampleRateController.addListener(() {
      if (_isNumeric(sampleRateController.text)) {
        sampleRate = int.parse(sampleRateController.text);
      } else {
        sampleRateController.text = "44100";
        sampleRate = 44100;
      }
    });

    baseNoteController.addListener(() {
      if (_isNumeric(baseNoteController.text)) {
        baseNote = int.parse(baseNoteController.text);
      } else {
        baseNoteController.text = "60";
        baseNote = 60;
      }
    });
  }

  bool _isNumeric(String s) {
    return s.trim().isNotEmpty && int.tryParse(s.trim()) != null;
  }

  Future<void> _selectInputMidi() async {
    final xtype = XTypeGroup(label: 'MIDI', extensions: ['mid', 'midi']);
    final result = await openFile(acceptedTypeGroups: [xtype]);
    if (result != null) {
      setState(() {
        inputMidiFilePath = result.path;
      });
    }
  }

  Future<void> _selectReferenceWav() async {
    final xtype = XTypeGroup(label: 'WAV', extensions: ['wav']);
    final result = await openFile(acceptedTypeGroups: [xtype]);
    if (result != null) {
      setState(() {
        referenceWavFilePath = result.path;
      });
    }
  }

  Future<void> _selectOutputWav() async {
    final xtype = XTypeGroup(label: 'WAV', extensions: ['wav']);
    final result = await getSaveLocation(acceptedTypeGroups: [xtype]);
    if (result != null) {
      String filePath = result.path;
      if (!filePath.endsWith('.wav')) {
        filePath += '.wav';
      }
      setState(() {
        outputWavFilePath = filePath;
      });
    }
  }

  Future<void> _copyToClipboard() async {
    if (inputMidiFilePath.isEmpty) {
      _showError("请输入 MIDI 文件");
      return;
    }
    if (referenceWavFilePath.isEmpty) {
      _showError("请选择参考 WAV 文件");
      return;
    }
    if (outputWavFilePath.isEmpty) {
      _showError("请设置输出文件路径");
      return;
    }
    Clipboard.setData(
      ClipboardData(
        text:
            'wavableMidiC '
            '-i "$inputMidiFilePath" '
            '-w "$referenceWavFilePath" '
            '-o "$outputWavFilePath" '
            '-t $selectedTrack '
            '-s $sampleRate '
            '-B $baseNote ',
      ),
    );
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text("命令已复制到剪贴板")));
  }

  void _showError(String msg) {
    ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(msg)));
  }

  @override
  void dispose() {
    trackController.dispose();
    sampleRateController.dispose();
    baseNoteController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("wavableMidiC GUI"), centerTitle: true),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: SingleChildScrollView(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // 输入 MIDI
              _buildFileRow("输入 MIDI 文件", inputMidiFilePath, _selectInputMidi),

              const SizedBox(height: 16),

              // 参考 WAV
              _buildFileRow(
                "参考 WAV 文件（音色采样）",
                referenceWavFilePath,
                _selectReferenceWav,
              ),

              const SizedBox(height: 16),

              // 输出 WAV
              _buildFileRow("输出 WAV 文件", outputWavFilePath, _selectOutputWav),

              const SizedBox(height: 24),

              // 参数区
              const Text(
                "参数设置",
                style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 12),

              _buildTextField("轨道编号 (-1=最后一轨)", trackController),
              const SizedBox(height: 12),

              _buildTextField("采样率", sampleRateController),
              const SizedBox(height: 12),

              _buildTextField("基准音符 (60=C4)", baseNoteController),

              // 命令预览
              const Text(
                "命令预览",
                style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500),
              ),
              const SizedBox(height: 8),
              Container(
                padding: const EdgeInsets.all(12),
                decoration: BoxDecoration(
                  color: Colors.grey[100],
                  borderRadius: BorderRadius.circular(6),
                  border: Border.all(color: Colors.grey, width: 1),
                ),
                child: SelectableText(
                  'wavableMidiC.exe '
                  '-i "$inputMidiFilePath" '
                  '-w "$referenceWavFilePath" '
                  '-o "$outputWavFilePath" '
                  '-t $selectedTrack '
                  '-s $sampleRate '
                  '-B $baseNote ',
                  style: const TextStyle(fontFamily: 'monospace', fontSize: 12),
                ),
              ),
            ],
          ),
        ),
      ),
      bottomNavigationBar: Container(
        color: Colors.white,
        padding: const EdgeInsets.symmetric(vertical: 8),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            TextButton.icon(
              onPressed: _copyToClipboard,
              icon: const Icon(Icons.copy, color: Colors.green),
              label: const Text(
                "复制命令到剪贴板",
                style: TextStyle(color: Colors.green),
              ),
            ),
            TextButton.icon(
              onPressed: () async {
                if (outputWavFilePath.isNotEmpty) {
                  final dir = path.dirname(outputWavFilePath);
                  await OpenFile.open(dir);
                }
              },
              icon: const Icon(Icons.folder_open, color: Colors.purple),
              label: const Text("打开目录", style: TextStyle(color: Colors.purple)),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildFileRow(String label, String filePath, VoidCallback onTap) {
    return Row(
      children: [
        Text(label, style: const TextStyle(fontSize: 16)),
        const SizedBox(height: 8),
        OutlinedButton(
          style: OutlinedButton.styleFrom(
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(0),
            ),
            side: BorderSide(width: 2, color: lightColorScheme.primary),
          ),
          onPressed: onTap,
          child: Row(
            children: [
              const Icon(Icons.folder, size: 16),
              const SizedBox(width: 6),
              Text(
                filePath.isEmpty ? '选择文件' : path.basename(filePath),
                style: const TextStyle(fontSize: 16),
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildTextField(String label, TextEditingController controller) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Text(
          label,
          style: const TextStyle(fontSize: 16),
          textAlign: TextAlign.right,
        ),
        const SizedBox(width: 12),
        SizedBox(
          width: 100,
          child: TextField(
            controller: controller,
            keyboardType: TextInputType.number,
            decoration: const InputDecoration(
              contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
            ),
          ),
        ),
      ],
    );
  }
}

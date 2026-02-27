# YIYAN-OS

**专为 ESP32 "Cheap Yellow Display" (ESP32-2432S028R) 设计的高性能嵌入式 GUI 系统框架**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: LVGL](https://img.shields.io/badge/Framework-LVGL%20v8.4-green.svg)](https://lvgl.io/)

[中文](README.zh-CN.md) | [English](README.md) | [Français](README.fr.md)

---

## 项目简介

YIYAN-OS 是一套专为 ESP32-2432S028R（Cheap Yellow Display / CYD）开发板设计的完整嵌入式 GUI 系统框架。通过**资源最大化利用**、**智能优化**、**类 BIOS 配置**和**快捷硬件接口**四个核心设计理念，构建了一个稳固、高效且灵活的底层平台。

## ✨ 核心特性

### 🖥️ 高性能图形引擎
- **LVGL v8.4 集成**：业界领先的轻量级图形库
- **双核并行处理**：Core 1 专注 UI 渲染，Core 0 处理应用逻辑
- **双缓冲 DMA 传输**：消除画面撕裂，流畅度可达 60 FPS
- **局部刷新优化**：仅重绘变化区域，大幅降低 CPU 负载

### 🔧 类 BIOS 配置系统
- **三级配置加载**：代码默认值 → NVS 持久化 → SD 卡配置文件
- **运行时参数调整**：无需重新编译即可修改系统行为
- **硬件解耦设计**：更换硬件只需修改配置文件

### 📱 多应用管理框架
- **应用生命周期管理**：Active（前台）、Paused（后台）、Stopped（未活动）
- **智能内存回收**：后台应用自动释放 LVGL 对象，确保内存高效利用
- **流畅应用切换**：预加载机制消除切换延迟

### ⚡ 智能功耗管理
- **自动背光控制**：
  - Manual 模式：用户手动调节亮度
  - Auto 模式：根据环境光自动调节（需光敏电阻）
  - Off 模式：关闭背光
- **空闲状态管理**：30 秒无操作进入 Idle 状态
- **深度睡眠**：5 分钟无操作进入 Light Sleep，触摸或按键唤醒

### 💾 存储系统集成
- **SPIFFS 文件系统**：板载 Flash 存储，用于系统资源
- **SD 卡扩展**：支持大容量用户数据和媒体资源
- **LVGL 文件接口**：统一访问 `F:`（Flash）和 `S:`（SD 卡）

### 🎛️ 完整硬件抽象层
- **显示屏**：ILI9341 / ST7789 驱动，320×240 分辨率
- **触摸屏**：XPT2046 电阻触摸，支持校准
- **RGB LED**：板载三色指示灯
- **光敏传感器**：环境光检测（GPIO 34）
- **BOOT 按键**：多功能唤醒/模式切换

## 🤖 内置应用

### 💬 ChatApp - AI 聊天应用
支持多种 AI 大模型 API 的聊天应用：
- **DeepSeek API**：支持 deepseek-chat 模型
- **GLM-5 API**：支持智谱 AI 的 GLM-5 模型
- **硅基流动 API**：支持多种开源模型（Qwen、DeepSeek-R1 等）
- **中文输入法**：内置自然码输入法，支持中文输入
- **流式响应**：实时显示 AI 回复内容
- **历史记录**：支持查看对话历史

配置方法：复制 `src/api_config_example.h` 为 `src/api_config.h`，填入你的 API 密钥：
```cpp
#define API_KEY "your-api-key-here"
#define API_BASE_URL "https://api.deepseek.com"
#define API_MODEL "deepseek-chat"
```

### 📁 FileExplorerApp - 文件管理器
- **双存储支持**：同时浏览 SPIFFS 和 SD 卡文件
- **文件操作**：支持查看、复制、移动、删除文件
- **图片预览**：支持 BMP、JPG、PNG 格式图片查看
- **文本查看**：支持文本文件浏览

### ⚙️ SettingsApp - 系统设置
- **背光调节**：手动/自动/关闭三种模式
- **WiFi 配置**：扫描并连接无线网络
- **系统信息**：查看内存使用、运行时间等

### 📶 WiFiConfigApp - WiFi 配置
- **网络扫描**：自动扫描周围 WiFi 网络
- **密码输入**：支持虚拟键盘输入密码
- **连接状态**：实时显示连接状态和 IP 地址

### 🎨 DemoApp - 演示应用
- **UI 展示**：展示各种 LVGL 控件效果
- **性能测试**：测试系统渲染性能

### 🔤 FontApp - 字体管理
- **字体浏览**：查看系统可用字体
- **字体预览**：预览字体渲染效果

## 📊 性能指标

| 指标 | 数值 |
|------|------|
| 帧率 (FPS) | ~60 |
| 内存占用 | ~105 KB (LVGL + 系统) |
| Flash 占用 | ~800 KB |
| UI 响应延迟 | < 50 ms |
| 应用切换时间 | < 200 ms |

## 🚀 快速开始

### 硬件要求
- ESP32-2432S028R (Cheap Yellow Display) 开发板
- microSD 卡（可选）
- USB 数据线

### 软件环境
- PlatformIO IDE（推荐）或 Arduino IDE
- ESP32 开发板支持包 v3.0+

### 编译与烧录
```bash
# 克隆项目
git clone <repository-url>

# 使用 PlatformIO 编译
pio run

# 烧录固件
pio run --target upload
```

### API 配置（可选）
如需使用 AI 聊天功能，请复制配置文件：
```bash
cp src/api_config_example.h src/api_config.h
```
然后编辑 `src/api_config.h`，填入你的 API 密钥。

## 📁 项目结构

```
├── src/
│   ├── main.cpp           # 主程序入口
│   ├── ConfigManager.*    # 配置管理模块
│   ├── BSP.*              # 板级支持包
│   ├── Storage.*          # 存储系统
│   ├── Performance.*      # 性能监控
│   ├── PowerManager.*     # 功耗管理
│   ├── AppManager.*       # 应用管理器
│   ├── ChatApp.*          # AI 聊天应用
│   ├── FileExplorerApp.*  # 文件管理器
│   ├── WiFiConfigApp.*    # WiFi 配置应用
│   ├── SettingsApp.*      # 设置应用
│   ├── DemoApp.*          # 示例应用
│   ├── FontApp.*          # 字体管理应用
│   ├── ZhFont.*           # 中文字体支持
│   ├── LvZhFont.*         # LVGL 中文渲染
│   ├── ZhDraw.*           # 中文绘制组件
│   └── lv_conf.h          # LVGL 配置
├── include/
├── lib/
└── platformio.ini
```

## 🔌 API 参考

### 应用开发接口

```cpp
// 注册新应用
AppMgr.registerApp("MyApp", createMyApp, &appInfo);

// 切换应用
AppMgr.switchToApp("MyApp");

// 返回主界面
AppMgr.switchToHome();

// 设置背光亮度 (0-255)
Power.setBacklight(128);

// 设置背光模式
Power.setBacklightMode(BACKLIGHT_MODE_AUTO);
```

### 应用生命周期

```cpp
class MyApp : public BaseApp {
    bool createUI() override;    // 创建 UI
    void onDestroy() override;   // 销毁清理
    bool onResume() override;    // 恢复到前台
    void onPause() override;     // 切换到后台
    void onUpdate() override;    // 周期更新
};
```

## 📝 更新日志

**v3.0.0** - 重大更新
- 优化 ChatApp AI 聊天功能
- 改进中文渲染性能
- 增强系统稳定性

**v2.x 系列** - 功能完善
- 添加 AI 聊天支持
- 优化中文字体渲染
- 改进文件管理器
- 添加性能监控

## 📄 许可证

MIT License

## 🤝 贡献

欢迎提交 Pull Request 参与贡献！

## 📧 联系方式

如有问题或建议，请在仓库中提交 Issue。

---

**YIYAN-OS** - *赋能嵌入式 GUI 开发*

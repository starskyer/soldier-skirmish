# 第三方声明

## 原创代码与图片

游戏原版完成于 **2024 年 1 月**。游戏代码及七张兵种、基地 PNG 图片均由我独立完成。此次整理尚未为这些原创内容选定开源许可证。

## Noto Sans

`assets/font/NotoSans.ttf` 是 `NotoSans[wdth,wght].ttf` 的未修改副本，整理时于 2026-11-04 从 Google Fonts 仓库获取：

- [字体来源](https://github.com/google/fonts/blob/main/ofl/notosans/NotoSans%5Bwdth,wght%5D.ttf)。
- [许可证来源](https://github.com/google/fonts/blob/main/ofl/notosans/OFL.txt)。
- 版权所有：2022 The Noto Project Authors。
- 许可证：SIL Open Font License 1.1，原文保存在 `assets/font/OFL.txt`。
- 字体 SHA256：`bfb7bb691513f12e734dc346c03a03f784912432d7e3fa8e56efcf906fe86b3d`。
- 许可证 SHA256：`cee9892f9f0cc8fe882c9e9537ee6a89621d86ee7ceaf70b02e2b2b1c25c061a`。

本仓库未包含原工程使用的 Arial 字体；构建和运行整理版不需要该文件。为保留上游授权文本，字体许可证文件保持原文。

## SDL 依赖

SDL2、SDL2_image 和 SDL2_ttf 是外部构建及运行依赖，本仓库未内置它们的 SDK 或 DLL。分发可执行程序时，还需要遵守这些库及其间接依赖各自的许可证：

- [SDL2](https://github.com/libsdl-org/SDL)。
- [SDL2_image](https://github.com/libsdl-org/SDL_image)。
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf)。

本声明不授予超出各上游许可证范围的权利。

# KeyStream

> A secure Wi-Fi to USB HID Keyboard bridge for air-gapped and managed PCs.

日本語 | [English](#english)

---

# 日本語

## 概要

**KeyStream** は、Wi-Fi経由で受信したテキストを **USB HID Keyboard** として入力するためのM5Stack/ESP32-S3向けソフトウェアです。

USBメモリや独自デバイスドライバが利用できない管理PCに対して、OS標準のUSBキーボードとして動作することを目的としています。

本プロジェクトでは、安全性を重視し、

* 送信された内容を本体画面で確認
* 本体の物理ボタンを押した場合のみ入力
* 入力履歴を保持

という運用を前提としています。

---

## 主な機能

* Wi-Fi経由でテキスト受信
* USB HID Keyboardとして入力
* ディスプレイで入力内容を確認

  * 文字数
  * 先頭5文字
  * 末尾5文字
* 物理ボタンによる入力開始
* 入力履歴（最大3件）
* 履歴からの再送

---

## システム構成

```text
Browser
    │
HTTP / Wi-Fi
    │
    ▼
+-----------------------+
|     KeyStream         |
|  M5Stack CoreS3       |
|                       |
|  HTTP Server          |
|  Display UI           |
|  History              |
|  USB HID Keyboard     |
+-----------+-----------+
            │ USB
            ▼
Managed Windows PC
```

---

## 対応ハードウェア（予定）

* M5Stack CoreS3
* M5Stack CoreS3 SE

将来的には

* AtomS3
* StampS3

への対応も予定しています。

---

## 開発環境

* ESP-IDF
* TinyUSB
* M5Unified

---

## セキュリティポリシー

KeyStream は、管理PCでの利用を想定して設計されています。

そのためUSBデバイスとして公開するクラスは

* HID Keyboard

のみです。

以下は使用しません。

* USB Mass Storage
* USB CDC Serial
* WebUSB
* DFU
* Composite Device

OS標準ドライバのみで認識されることを目標としています。

---

## 開発状況

現在開発中です。

予定しているマイルストーン

* [ ] USB HID Keyboard
* [ ] HTTP Server
* [ ] Web UI
* [ ] Display UI
* [ ] History
* [ ] Secure Mode

---

## ライセンス

MIT License

---

# English

## Overview

**KeyStream** is a Wi-Fi to USB HID Keyboard bridge designed for M5Stack and ESP32-S3 devices.

It receives text over Wi-Fi and types it on a target computer as a standard USB keyboard.

The primary goal is to work with managed or security-restricted PCs where:

* USB storage devices are blocked
* Third-party drivers cannot be installed
* Only standard HID devices are allowed

---

## Features

* Receive text over Wi-Fi
* Type text as a USB HID Keyboard
* Preview before sending

  * Character count
  * First 5 characters
  * Last 5 characters
* Physical confirmation button
* History (up to 3 entries)
* Resend previous entries

---

## Architecture

```text
Browser
    │
HTTP / Wi-Fi
    │
    ▼
+-----------------------+
|      KeyStream        |
|      ESP32-S3         |
|                       |
| HTTP Server           |
| Display UI            |
| History               |
| USB HID Keyboard      |
+-----------+-----------+
            │ USB
            ▼
Managed Windows PC
```

---

## Hardware

Currently planned:

* M5Stack CoreS3
* M5Stack CoreS3 SE

Future support:

* AtomS3
* StampS3

---

## Software Stack

* ESP-IDF
* TinyUSB
* M5Unified

---

## Security

KeyStream intentionally exposes only a single USB interface:

* USB HID Keyboard

The following USB classes are intentionally disabled:

* USB Mass Storage
* USB CDC Serial
* WebUSB
* DFU
* Composite Devices

This minimizes compatibility issues with managed corporate environments.

---

## Project Status

🚧 Work in progress

Roadmap

* [ ] USB HID implementation
* [ ] HTTP API
* [ ] Web UI
* [ ] Display UI
* [ ] History
* [ ] Secure mode

---

## License

MIT License

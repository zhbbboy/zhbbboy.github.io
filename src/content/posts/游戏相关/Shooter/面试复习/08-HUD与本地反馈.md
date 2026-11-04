---
title: "面试复习 08：HUD与本地反馈"
date: 2026-11-04
tags: [UE5, 游戏开发, 面试]
draft: false
---

[返回模块目录](<./00-模块目录.md>) · [原学习过程](<../第4节：UI.md>)

> 本页依据2026-11-04工作区源码整理。原章节保留当时的实现与学习顺序；当前网络流程以本页及源码为准。未勾选清单需要自己实际完成后再标记。

## 1. 功能

在每个本地玩家屏幕展示血量、弹药、波次、回合状态、命中和受伤反馈。

## 2. 相关类和源码位置

`AMyArenaShooterCharacter`、`UMyHUDWidget`、`AMyArenaShooterGameState`、`AMyWeapon`。

- `Source/MyArenaShooter/Private/MyArenaShooterCharacter.cpp:690`（`::TryInitializeLocalHUD(`）
- `Source/MyArenaShooter/Private/MyArenaShooterCharacter.cpp:507`（`::RefreshRoundState(`）
- `Source/MyArenaShooter/Private/MyHUDWidget.cpp:143`（`::UpdateRoundMessage(`）
- `Source/MyArenaShooter/Private/MyHUDWidget.cpp:74`（`::ShowHitMarker(`）

## 3. 入口

本地 `TryInitializeLocalHUD` 创建Widget；Character Tick `RefreshRoundState` 拉取GameState并刷新。

## 4. 调用链

```text
本地Pawn BeginPlay / PawnClientRestart / 回调尝试 → CreateWidget(PlayerController)
→ AddToPlayerScreen → 初始化血量与现有弹药 → RefreshRoundState
→ Character Tick读取RoundState；Revision或个人死亡状态改变时更新波次和提示
→ OnRep_CurrentHealth / OnRep_Ammo更新血量、弹药
→ ClientNotifyEnemyHit显示HitMarker；ClientPlayDamageFeedback显示闪红/声音
→ Timer稍后隐藏短暂反馈
```

## 5. 核心机制

GameState的OnRep_RoundState当前只记日志；界面由Character::RefreshRoundState在Tick中读取Revision并更新，不要说它直接由GameState的OnRep推送。Widget本地创建而不是复制整个Widget。NativeConstruct隐藏胜负文字、HitMarker和受伤遮罩。

## 6. 关键设计问题

**为什么HUD只在本地创建？**

每位玩家看到自己的界面与私有反馈；网络同步的是状态和通知，Widget本身是本地显示对象。

**为什么使用Revision和死亡标志？**

只有回合状态版本或个人死亡状态变化时才刷新相关文字；个人死亡可以与整局胜负分开。

**OnRep与RPC各通知什么？**

OnRep处理复制后的当前数据；Client RPC适合一次性的本地受击或命中反馈。

## 7. 独立复刻情况

- [ ] 不看源码能说出完整调用链
- [ ] 不看源码能独立复刻该模块的基础版本
- [ ] 能解释至少两个设计取舍
- [ ] 能说出HUD创建的本地控制与PlayerController条件
- [ ] 能指出RoundState OnRep只记日志、界面在哪里刷新
- [ ] 能解释本地玩家死亡与团队胜负显示的区别

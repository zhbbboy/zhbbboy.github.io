---
title: MyArenaShooter 扩展开发计划
date: 2026-10-15
aliases:
  - Shooter 扩展路线图
  - MyArenaShooter Roadmap
project: MyArenaShooter
engine: Unreal Engine 5.4
status: 源码基线已核对，扩展待实施
updated: 2026-10-15
description: 依据本地 UE5.4 工程、学习笔记和调用链图修订的双人合作 PvE 路线，包含基线修复、AI、服务器权威同步、武器数据驱动与验收。
tags:
  - UE5
  - 游戏客户端
  - 网络同步
  - AI
  - 项目规划
draft: false
---

# MyArenaShooter 扩展开发计划（UE5.4）

> [!abstract] 项目定位
> **基于 UE5 C++ 的双人合作 PvE 科幻 FPS Demo**。标准版突出 **AI 决策系统、服务器权威网络同步、数据驱动武器设计**，辅以调试与性能分析。目标是形成能运行、能演示、能独立讲解设计取舍的校招项目。

**核对日期：2026-10-15。** 本次已阅读本地 `Source/MyArenaShooter`、项目配置、相关学习资料与既有图纸说明，并抽查运行日志；未执行新的 UE 编译、PIE 或打包。下文区分「源码已有」「日志线索」「待编辑器验证」「计划新增」，任务勾选以实际验收为准。

**工程目录：**`D:\code\Unreal Projects\MyArenaShooter\MyArenaShooter`。本文中的 `Source/`、`Config/`、`Content/` 均相对此目录。源码行号是本次核对时的位置，后续以函数名定位。

## 0. 本次修订结论与交付范围

### 0.1 原计划需要改的六件事

| 原计划的问题                            | 修订后的安排                                        | 原因                                    |
| --------------------------------- | --------------------------------------------- | ------------------------------------- |
| 当前工程仍写成「预计已有」，地图写成 `L_SciFiArena` | 第 1 节给出真实类、函数和源码位置；当前默认地图为 `/Game/Maps/Arena` | 旧教程、旧图纸与当前工程存在差异                      |
| 直接进入 AI / 网络扩展                    | P0 先处理命中反馈、死亡兜底、表现未接通、波次异常等现有缺口               | 避免把原有单机问题误判为同步问题                      |
| 完成整套 AI 后才正式联网                    | P0 后先完成 P2 的连接与最小开火链，再做 P1                    | 尽早暴露 Ownership、本地 HUD、远程角色可见性和服务器瞄准问题 |
| 双人规则只有胜负公式                        | 明确开局条件、单人死亡、全灭、退出、重开和不支持的中途加入                 | 这些规则决定 GameMode、GameState 与玩家生命周期设计   |
| 动画 Notify、持枪表现与伤害权限边界不够具体         | 增加攻击目标锁定、单次伤害、服务器动画推进和视图分离验收                  | 「两端能播放动画」不等于伤害正确                      |
| 最终目标要求两种武器，停止条件却允许跳过 P3；工时缓冲不足    | 区分最低交付版与标准版，分别设置收尾门槛                          | 防止范围与验收相互矛盾                           |

### 0.2 最低交付版与标准版

| 项目  | 最低交付版：可以阶段性收尾                                                    | 标准版：本计划完整目标                      |
| --- | ---------------------------------------------------------------- | -------------------------------- |
| 场景  | 一张 Arena，双人出生点、掩体和导航可用                                           | 同左，进一步整理视觉与资源                    |
| 敌人  | 一种 QuadShell；Sight + Blackboard + Behavior Tree；发现、追击、攻击、丢失目标、死亡 | 同左；EyeDrone 仍为选做                 |
| 武器  | 一把 Rifle，连射、换弹、弹药、实体 Projectile                                  | 在 P3 完成 Rifle / Pistol 两套配置，共用逻辑 |
| 联网  | 2 人 Listen Server；服务器决定生成、伤害、弹药、波次与结算                            | 同左；Dedicated Server 仍为选做         |
| 质量  | P0 修复、双端测试矩阵、最小调试、Windows Development 包                          | 同左，补齐数据配置回归与设计说明                 |
| 展示  | README、原生 draw.io 图、测试记录、1～2 分钟双视角视频                             | 三项核心能力均有实际演示与证据                  |

最低交付版仅宣称已经完成的 AI 与联网能力；跳过 P3 时，不把「数据驱动双武器」写成已实现。两套配置可以通过不同测试开局分别装备验证，**不要求增加背包或局内切枪**。

### 0.3 暂不纳入的内容

- 不做账号、匹配、大厅、排行榜、商城、公网部署、断线重连或主机迁移。
- 不做完整 PvP、友军伤害、延迟回溯命中、完整反作弊、客户端预测 Projectile。
- 本轮不做倒地救援、单局复活、自由观战；死亡玩家保留本地视角与 HUD，等待回合结束。
- 优先复用 CharacterMovement；完整自定义 Dash 预测与回滚为后续专项。
- 不扩展复杂技能、Buff、Boss 或 GAS；这些留给 GAS Boss Demo。
- 不预先加入对象池、Replication Graph、Iris 或复杂武器继承树。

### 0.4 阶段、预算与执行顺序

以下是个人学习型开发的估算，不是已完成工时。以每周约 15 小时计算，**标准版基础工作合计 78～122h，另留约 20% 联调缓冲，总预算约 95～150h，即 7～10 周**。P5 不在此预算中。完成早期联网链后，按实际耗时重估。

| 阶段 | 优先级 | 主要交付 | 基础工时 | 门槛 |
| --- | --- | --- | ---: | --- |
| P0 基线整理与修复 | 必须 | 当前缺口修复、单机回归、版本基线 | 8～12h | 能编译并打开现有工程 |
| P1 AI 行为升级 | 高 | 感知、行为树、明确目标、攻击生命周期 | 12～18h | P0；建议早期联网链已通过 |
| P2 双人网络同步 | 核心 | 连接、开火、伤害、AI、波次、重开、延迟测试 | 36～56h | P0；分两次集成 |
| P3 武器数据驱动 | 标准版必须 | 单一配置来源、Rifle / Pistol、联网回归 | 10～16h | P2 核心状态稳定 |
| P4 调试与质量 | 必须 | 最小诊断、回归记录；自定义 Debugger 分类选做 | 6～10h | 从 P0 持续积累 |
| P5 性能专项 | 选做 | 可复现实验、瓶颈与复测 | 8～15h | 已有稳定负载场景 |
| P6 作品收尾 | 必须 | 包体、README、图纸、测试证据和录屏 | 6～10h | 对应版本的功能验收通过 |

**实际顺序：P0 → P2-N1 + N2 最小链 → P1 → P2-N2 补全 / N3 / N4 → P3 → P6。** P4 贯穿全过程；P5 在核心稳定后选做。早期联网链的工时计入 P2，不重复累计。

最低交付版采用 P1 基础范围、跳过 P3、仅保留 P4 最小诊断，初始可按 **65～105h（含缓冲，约 5～7 周）** 预留。若预算不足，先缩可选范围，不压缩权威伤害、双端验证和打包验收。

## 1. 已核对的工程基线与 P0

### 1.1 事实来源及阅读顺序

1. **当前 C++ 与配置**：实现与默认值的首要依据；Blueprint 可能覆盖默认值。
2. **运行日志**：可证明记录时发生过某些事件，不能证明当前全部流程已通过。
3. [第 7 节：敌人战斗动画与伤害时机](./第7节：敌人战斗动画与伤害时机.md)、[第 8 节：武器后坐力、换弹姿态与惯性摆动](./第8节：武器后坐力、换弹姿态与惯性摆动.md)：与当前动画、武器表现相关的补充说明。
4. 第 1～6 节及 `draw/MyArena` 的 2026-10-11 总框架图：帮助理解早期实现；旧死亡流程、旧拼写与旧地图不能直接套用。2026-10-13 追加图单独说明了新边界。

`框架.md` 含参考项目的 `ABoomerShooterCharacter` 等名字；**本工程真实玩家类是 `AMyArenaShooterCharacter`**，并未因此具备参考项目的切枪、交互等全部功能。

本地 `Saved/Logs/MyArenaShooter.log` 记录引擎为 **5.4.4**，并出现 Arena 的 PIE 启动、`Enemy Attack Hit!`、`All Waves Completed!`。这提供运行线索；Notify 内部连线、按键和双人表现仍需在 Editor 中核实。

### 1.2 当前类、资源与扩展入口

下表 `.cpp` 在 `Source/MyArenaShooter/Private/`，`.h` 在 `Source/MyArenaShooter/Public/`。标为「计划新增」的类型与资源尚未实现。

| 当前对象 | 源码已有 / 已确认事实 | 扩展入口与位置 |
| --- | --- | --- |
| `AMyArenaShooterGameMode` | 仅通过 `FClassFinder` 设置默认玩家蓝图 | `MyArenaShooterGameMode.cpp:6`；承接服务器回合规则 |
| `AMyArenaShooterCharacter` | Camera、输入、Dash、血量、HUD、生成武器、个人胜败标志、`OpenLevel` 重开 | `MyArenaShooterCharacter.cpp`：`BeginPlay():33`、`ReceiveEnemyDamage():288`、`Die():384`、`RestartGame():403` |
| `AMyWeapon` | `StartFire/StopFire` + Tick 冷却；Timer 换弹；相机选点、枪口生成实体子弹；网格后坐力与换弹姿态 | `MyWeapon.cpp`：`Tick():36`、`Fire():149`、`StartReload():336`、`FinishReload():373` |
| `AMyProjectile` | `UProjectileMovementComponent` 飞行；命中敌人时调用伤害；随后销毁 | `MyProjectile.cpp:72`；伤害默认值在 `MyProjectile.h:39`，当前不在 Weapon 中 |
| `AMyEnemyCharacter` | `TryAttack` 启动 Montage；`ApplyAttackDamage` 对玩家 0 做距离检查；逻辑死亡后延迟销毁 | `MyEnemyCharacter.cpp:98 / 124 / 156 / 190` |
| `AMyEnemyAIController` | Tick 找距离并 `MoveToActor` / `TryAttack`；BeginPlay 缓存玩家 0；无 BT / Perception | `MyEnemyAIController.cpp:14 / 22` |
| `AMyEnemySpawner` | BeginPlay 立即刷第一波；Tick 按无效引用或 `IsDead()` 清理并推进波次 | `MyEnemySpawner.cpp:13 / 23 / 71 / 146`；目前直接通知玩家 0 更新 HUD / 胜利 |
| `UMyHUDWidget` / `WBP_HUD` | 数据主动推送；血量、弹药、波次、命中标记与终局显示 | `MyHUDWidget.h/.cpp`；Widget 不应成为权威数据容器 |
| 地图与蓝图 | 默认地图为 `/Game/Maps/Arena`，另有 `Content/Test.umap` | `Config/DefaultEngine.ini:4 / 6`；蓝图目录实际拼写为 `Content/BluePrints` |
| 计划新增 | `AMyArenaShooterGameState`、`BB_Enemy`、`BT_Enemy`、`UWeaponDataAsset`、`DA_Rifle/DA_Pistol` | 按 P1 / P2 / P3 分阶段增加；名称为建议，不代表文件已存在 |

当前源码未见项目自定义 RPC、`Replicated` / `ReplicatedUsing` 状态或显式复制配置。**ACharacter 的引擎默认复制能力，不代表血量、武器和玩法已联网。** 当前工程目录执行 `git status` 未识别到仓库，需先建立或确认真正的版本管理位置。

### 1.3 先修复或明确处理的缺口

以下均是待实施任务；本次修改计划没有修复游戏代码。

| ID / 优先级            | 代码事实或风险                                                                     | 计划处理与验证                                                                                           |
| ------------------- | --------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| BASE-01 / 必须        | `TakeProjectileDamage()` 致死分支调用 `Die()` 后返回 `false`，子弹因此跳过击杀那一发的 Hit Marker | 返回值统一表示「伤害被接受」；普通命中与致死命中均有反馈，拒绝伤害时无反馈。位置：`MyEnemyCharacter.cpp:115`、`MyProjectile.cpp:77`         |
| BASE-02 / 必须        | `DeathMontage` 存在但播放返回 0 时直接 return，没有销毁兜底                                  | 无动画、播放成功、播放失败三条路径均能结束尸体生命周期；`MyEnemyCharacter.cpp:212`                                            |
| BASE-03 / 必须        | 后坐力只有累加，`CurrentSwayYaw/Pitch` 未接入目标插值；Yaw 目标也未写入                           | 按第 8 节补齐回零、水平/垂直摆动与最终变换；若预算不允许则显式关闭未完成效果，不能宣称完整手感。`MyWeapon.cpp:36 / 68 / 247`                    |
| BASE-04 / 必须        | 攻击靠 Blueprint Notify 调用伤害；`ApplyAttackDamage()` 没有「本轮已命中」保护，仅按距离判断          | 核实 Slot / Notify；加入本次攻击标识或已结算标志，检查目标存活、距离、朝向与遮挡；重复 Notify 不得重复扣血。`MyEnemyCharacter.cpp:124 / 156` |
| BASE-05 / 必须        | 空 Waves、零敌人、无有效出生点或全生成失败会进入无推进路径                                            | 明确空波跳过规则；配置错误 / 全失败应报错并终止或进入有上限的重试，不得静默卡住或误报胜利。`MyEnemySpawner.cpp:71`                            |
| BASE-06 / 必须        | 死亡 / 胜利只 StopFire；换弹 Timer 仍可回调；Dash / Jump 的终局检查不统一                        | 终局与 EndPlay 停火、取消换弹 / 攻击计时、拒绝新动作；统一输入状态检查。位置：玩家 `Die/WinGame/Dash`、武器 `StartReload/FinishReload`  |
| BASE-07 / AI 前必须    | 敌人有旧 `StartAttack/bIsAttacking` 与当前 `TryAttack` 两套入口，冷却也分布在敌人和 AIController | 搜索 Blueprint 引用后统一实际入口与冷却来源，避免直接删掉仍被资产引用的接口；P1 不再依赖两套状态                                           |
| BASE-08 / 双人回合验收前必须 | AIController、敌人伤害、Spawner 都依赖玩家 0；个人 `bHasWon` 同时承担全局结算                     | P1 明确目标；P2 将共享结算移至 GameMode / GameState，个人死亡与整局失败分开                                               |

日志中有相邻约 50ms 的两条 `Enemy Attack Hit!`，目前缺少敌人实例名与攻击编号，**不能据此断言同一攻击重复伤害**。在 BASE-04 验证时补日志，区分多敌人同时攻击和同一 Notify 重入。

### 1.4 P0：冻结可回退的单机基线

- [ ] `P0-01` 固定 UE5.4；记录实际补丁版本（日志为 5.4.4）、编译器、插件和构建目标。
- [x] `P0-02` 完整编译 Editor Target，检查 Blueprint 编译；区分已有资源警告与阻断错误。
- [x] `P0-03` 在当前 `Arena` 运行移动 → 射击 → 换弹 → 攻击受伤 → 敌人死亡 → 下一波 → 胜利。
- [x] `P0-04` 验证失败、重新开始和 `IA_Restart/IA_Reload` 的真实按键；不从资产名字推断 Enter / R 绑定。
- [ ] `P0-05` 建立 / 确认 Git 基线；源码、Config、Content、`.uproject` 纳入管理；`.uasset/.umap` 使用 Git LFS 或明确的二进制管理方式；排除生成物与本机配置。
- [ ] `P0-06` 保留当前可运行版本的提交，再开扩展分支；每个验收点单独提交。
- [ ] `P0-07` 保留测试地图与演示地图；检查 PlayerStart、NavMesh、刷怪器实例、Waves 和资源引用。避免直接移动资产导致引用失效。
- [ ] `P0-08` 做双人连接预实验并记录移动 / Camera / HUD / Weapon 问题；它属于诊断，不代表联网验收。正式修复按 P2-N1 推进。
- [ ] `P0-09` 完成 BASE-01 / 02 / 03，验证击杀标记、死亡兜底、松开后回弹与转向摆动。
- [ ] `P0-10` 完成 BASE-04 / 07；每次攻击至多一次有效伤害，死亡后迟到的 Notify 无伤害；明确唯一攻击入口。
- [ ] `P0-11` 完成 BASE-05；记录空配置、无有效出生点、部分 / 全部生成失败的处理结果。
- [ ] `P0-12` 完成 BASE-06；按住开火时死亡、换弹中结束、连续重开均不留下旧 Timer 或输入状态。
- [ ] `P0-13` 保存源码提交、测试环境、结果与已知限制；只将实际通过的项目打勾。

**P0 出口：**有可回退提交；上述单机链路通过；基础缺口已修复或按文中允许方式关闭。随后进入早期联网链，不先扩展第二种敌人。

## 2. P1：AI 决策系统升级

### 2.1 职责与目标选择

`AI Perception` 维护感知信息，`Blackboard` 保存决策数据，`Behavior Tree` 选择行为，`AMyEnemyCharacter` 执行动作并管理攻击与死亡生命周期。联网时这条决策链运行在服务器；客户端呈现角色与动作。

目标最低规则：从已感知且存活的玩家中选择距离最近者；当前目标仍有效时优先保持，避免每帧切换。失去视野后只追查最后已知位置，不继续读取目标实时位置；目标死亡、销毁或退出时清除并重新选择。威胁值 / 伤害仇恨系统为选做。

一次攻击起手时保存本次攻击目标，命中时重新检查其合法性；期间切换追击目标，不应把这次已经起手的攻击转移给另一个人。

### 2.2 Blackboard 与行为分支

| Key / 数据 | 类型 | 更新与清除原则 |
| --- | --- | --- |
| `TargetActor` | Object，约束 Actor / 玩家类型 | 仅保存当前决策目标；失效、死亡、退出时清除 |
| `LastKnownLocation` | Vector | 记录最后一次有效感知位置；不要默认用 `(0,0,0)` 代表「没有」 |
| `HasLastKnownLocation` | Bool | 表示搜索位置是否有效；搜索完成 / 超时后清除 |
| `HasLineOfSight` | Bool | 表示当前目标的有效视觉感知；丢失时置 false |
| 攻击可用性 | 角色查询或必要的 Blackboard Bool | 距离、冷却、动作、存活由统一入口校验，避免两处维护矛盾真值 |

| 优先级 | 条件 | 行为及结束方式 |
| --- | --- | --- |
| 1 | 自己死亡 / 回合终结 | 停止 Brain / 移动 / 攻击，退出正常决策 |
| 2 | 目标可见、存活、范围内且允许攻击 | StopMove → 面向目标 → Attack；结束 / 中断后释放任务 |
| 3 | 目标可见且有效 | MoveTo；距离与攻击范围一致 |
| 4 | 不可见但有最后已知位置 | MoveToLocation → 有上限的 Search → 清除搜索位置 |
| 5 | 没有目标或搜索位置 | Idle；Patrol 可后补 |

不要在 `HasLineOfSight=false` 时仍让高优先级分支执行 `MoveTo TargetActor`，否则会绕过最后位置搜索。用 Blackboard Decorator 的 Observer Aborts 处理目标变化，并为距离 / 攻击条件提供事件或低频 Service 更新；不能依赖一次性距离检查让长时间 MoveTo 自动变为攻击。可参照 [Epic 行为树入门（5.4）](https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---quick-start-guide?application_version=5.4)。

### 2.3 任务清单

- [ ] `AI-01` 创建 `BB_Enemy` 与 `BT_Enemy`；按上述条件先实现 Idle / Chase / Attack，再补最后位置搜索。
- [ ] `AI-02` 在 AIController 的有效 Possess / 初始化时启动树，清除旧 Tick 的重复决策与 MoveTo 路径。
- [ ] `AI-03` 配置 `UAIPerceptionComponent` + Sight；核实感知源、Detect Neutrals / 阵营筛选及失去感知参数。
- [ ] `AI-04` 实现目标选择、目标死亡 / 退出清除、丢失视野与搜索超时；先用单个玩家测试，再用双人测试。
- [ ] `AI-05` 改造 `TryAttack()` / `ApplyAttackDamage()`，使用明确目标；两处都不得重新查找玩家 0。
- [ ] `AI-06` 统一攻击冷却与执行状态；`BTTask_Attack` 明确返回成功、失败或等待，不每次 Tick 重播 Montage。
- [ ] `AI-07` 若 Task 等待动画完成，补完成 / 中断 / 超时处理、解绑委托；C++ 非实例化 Task 的每个 AI 状态放 NodeMemory，或显式使用实例化节点。
- [ ] `AI-08` 配置 Decorator 中断与低频条件更新；核对 MoveTo 接受半径、胶囊半径、攻击距离及攻击前朝向。
- [ ] `AI-09` 延续 BASE-04：每次攻击只结算一次；受击、死亡、目标失效或树中断后，迟到 Notify 不得扣血。
- [ ] `AI-10` 在掩体、不可达路径、目标突然消失场景验证；记录 MoveTo 失败后的回退行为。
- [ ] `AI-11` 用 Behavior Tree / Blackboard / Perception 调试截图说明每次行为切换。

**验收标准：**未感知时不无条件追击；入视野后追击与攻击；失去视野去最后位置并最终放弃；死亡目标不继续被追击；两个敌人的 Task 状态不串联；BT 调试可解释决策。巡逻路线和 EyeDrone 不阻塞此阶段。

## 3. P2：双人网络同步（核心）

### 3.1 先固定回合规则

| 情况 | 本轮约定 |
| --- | --- |
| 玩家进入 | 最多 2 人；联网模式等待两人完成登录、Possess 和必要初始化后开始刷怪；单机回归明确采用 1 人目标人数，避免永远等待 |
| 初始状态 | 服务器初始化血量、武器和弹药；客户端完成本地输入 / HUD 绑定。准备就绪必须经过服务器确认，不能用固定延时猜测 |
| 一人死亡 | 个人状态变为死亡，停火、取消换弹、禁用玩法输入；不立刻显示全队失败。另一人继续，AI 不再选择死者 |
| 全部玩家死亡 | 服务器只结算一次 Defeat；停止刷怪、AI 攻击和后续有效伤害 |
| 最后一波清空 | 至少一名参战玩家存活时 Victory；若同一帧已全灭，则优先 Defeat，终局不可再次改写 |
| 友军 | 无友军伤害；Projectile 与瞄准查询忽略同队玩家，验证队友挡枪时不会误伤或意外挡住 Projectile |
| 一名玩家退出 | 最低版采用中止回合并显示原因，不把退出当作正常胜败；Host 退出时 Client 显示连接关闭并回到可重新连接的初始状态 |
| 中途加入 / 重连 | 本轮不支持战斗中加入，明确拒绝并提示；新回合开始前正常连接另行验证 |
| 重新开始 | 仅 Host 在终局后发起，由服务器统一重载地图并携带双方；Client 不能通过本地 `OpenLevel` 单独重开 |

这组规则是计划选择，尚未写入工程。最低版不必新增自定义 PlayerState；当前生命值与死亡状态先随 Character 管理。需要跨 Pawn 保存玩家身份 / 得分时再引入 PlayerState。

### 3.2 权限与数据归属

| 对象 / 内容 | 决策或写入者 | 客户端读取 / 表现 |
| --- | --- | --- |
| `AMyArenaShooterGameMode` | 服务器：入场、开局、终局、重开规则 | 客户端不依赖它读取共享状态 |
| 计划新增 `AMyArenaShooterGameState` | 服务器写当前阶段、波次、总波次、存活敌人数、结果 / 中止原因 | 复制给所有客户端，用于 HUD 与终局显示 |
| `AMyEnemySpawner` | 服务器生成、登记与清理敌人，向回合规则报告 | 不在客户端生成；不直接找到玩家 0 更新 HUD |
| 玩家 Character | 服务器初始化 / 改血 / 死亡；引擎处理标准移动预测与校正 | 所属玩家处理输入 / Camera / HUD；远端呈现角色 |
| `AMyWeapon` | 服务器校验射速、弹药、换弹与开火状态 | 弹药按需 OwnerOnly；开火 / 换弹外观向相关客户端显示 |
| `AMyProjectile` | 服务器生成、碰撞、伤害和销毁 | 选择一致的运动 / 表现方案，不在客户端再次扣血 |
| `AMyEnemyAIController` / 敌人 | 服务器决策、攻击、血量和死亡 | 复制必要状态，驱动客户端动画与外观 |
| `UMyHUDWidget` | 每个本地玩家创建，使用自己的 PlayerController 作为 Owning Player | 订阅状态并显示；Widget 本身不复制 |

建议把强相关的回合字段组成一个复制状态结构，再统一刷新 UI。`ReplicatedUsing` 不保证不同属性 OnRep 的先后次序；跨 Actor RPC 也没有全局顺序保证，初始化不能依赖「武器一定比 HUD 先到」。参见 [Epic 复制执行顺序（5.4）](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicated-object-execution-order-in-unreal-engine?application_version=5.4)。

### 3.3 N1：连接、生命周期与远程可见性

- [ ] `NET-01` `Arena` 放置至少 2 个 PlayerStart；PIE 采用 2 Players + Play As Listen Server，记录 Host / Client 操作视角。
- [ ] `NET-02` 确认角色复制与 CharacterMovement 正常；保持基础移动由引擎预测，不手动复制 Transform。
- [ ] `NET-03` 输入映射与 HUD 只在本地控制、控制器有效时初始化；考虑远程 Possess / 重开时机，绑定应可重复检查且不重复创建。
- [ ] `NET-04` 将当前 `BeginPlay` 无条件 SpawnWeapon 改为服务器生成；武器设置复制、Owner / Instigator，`CurrentWeapon` 引用复制到客户端。
- [ ] `NET-05` 在引用到达及本地控制就绪后重新绑定持枪 / UI；HUD 创建时主动读取一次当前快照，后续用 OnRep / 委托更新。
- [ ] `NET-06` 明确第一人称持枪与远程角色外观：所属玩家看 Camera 持枪，其他玩家看可识别的角色 / 简化持枪。核实 OwnerNoSee / OnlyOwnerSee；权威枪口不能依赖只在本地更新的网格摆动。
- [ ] `NET-07` 确认两端都能看见对方；单机代码只创建 Camera，远程角色 Mesh 是否配置需 Editor 验证。复用现有合适资源即可，不制作完整第三人称动画系统。
- [ ] `NET-08` 提前建立 GameState 最小阶段与开局条件；联网时 Spawner 不再在 BeginPlay 无条件开刷。

**N1 出口：**双人控制独立、彼此可见；每人一把权威武器、一个本地 HUD；两人就绪前不开始正式波次。

### 3.4 N2：开火、弹药与伤害

**第一次进入 P2，只完成一条最小链：**远程 Client 按开火 → 所属 Character 的 Server RPC → 服务器合法性检查 → 服务器生成一颗 Projectile → 敌人扣血 → 两端看到更新。先固定一把 Rifle、一个测试敌人；完成后再进入 P1。

后续按以下任务补齐。新增 RPC 名称是设计建议，当前源码没有这些函数。

- [ ] `NET-09` 推荐以所属 Character 承接 `ServerStartFire/ServerStopFire/ServerReload`，再转发当前武器；如放在 Weapon 上，需先验证它的复制与 Ownership 已建立。
- [ ] `NET-10` Start / Stop / Reload 为低频 Reliable 请求；服务器按自己的间隔调度连射。不要每颗子弹发 Reliable RPC，也不要客户端决定最终射速 / 弹药。
- [ ] `NET-11` 每次射击检查存活、回合阶段、当前武器、冷却、换弹、弹药、合法参数；重复 Start 不创建多份 Timer，重复 Stop 安全。
- [ ] `NET-12` 输入 `Completed` 和 `Canceled` 都要停火；死亡、解除控制、失去武器、退出、终局 / EndPlay 均停止服务器连射并清理 Timer。测试松开与丢包后不会持续射击。
- [ ] `NET-13` 服务器使用合法视点 / 控制朝向和稳定的权威枪口计算方向；持续按住并转向时仍更新瞄准，不能只用 StartFire 时的一份方向。若额外提交瞄准数据，应限制频率并校验位置、角度与数值。
- [ ] `NET-14` 保留「相机射线选点 → 枪口发射实体弹」的原有语义；检查枪口贴墙 / 藏在掩体后、动态掩体、同队玩家遮挡。不把相机选点射线改成第二次伤害结算。
- [ ] `NET-15` Projectile 仅服务器碰撞扣血 / 销毁，并保证一次命中至多一次有效伤害；复制必要运动，避免客户端 ProjectileMovement 与位置复制互相争抢。
- [ ] `NET-16` `ServerReload` 校验弹匣、备用弹药、动作和角色状态；FinishReload 再检查生命周期。死亡 / 终局取消换弹，回调不补发旧弹药。
- [ ] `NET-17` 复制生命、死亡、弹药与换弹状态；弹药使用 `COND_OwnerOnly` 时验证 Owner 链。客户端 BeginPlay 不应重置已经复制的权威值。
- [ ] `NET-18` 状态变更与 OnRep 共用本地刷新函数；Listen Server 本机 HUD 也能更新，不依赖 C++ 自动在服务器调用 OnRep。
- [ ] `NET-19` 命中标记由服务器确认有效伤害后通知射手所属客户端；现有直接调用 `NotifyEnemyHit()` 的单机路径不能满足远程 Client。
- [ ] `NET-20` 将开火 / 换弹声音、枪口火光与伤害拆开；本地即时反馈与服务器广播采用去重规则，不让 Host 或射手看见 / 听见双份。换弹进度需有服务器起始时间 / 时长，避免只复制 Bool 后两端从零开始计时。
- [ ] `NET-21` 命中特效从仍存活且相关的 Character / Weapon 等对象发送，或使用明确的独立事件方案；不要假定短命 Projectile 上的 Multicast 在立即 Destroy 后必达。

RPC 的可达性取决于复制和 Ownership；客户端调用 Multicast 只在本地执行。低频请求与表现事件的策略参考 [Epic RPC（5.4）](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-procedure-calls-in-unreal-engine?application_version=5.4)。本 Demo 不做延迟回溯，远程玩家的实际命中按服务器收到请求与 Projectile 飞行结果为准。

### 3.5 N3：AI、动画、波次和结算

- [ ] `NET-22` AIController 的决策与 Spawner 逻辑仅服务器执行；敌人由服务器生成并复制，客户端不再独立 Spawn / 寻路。
- [ ] `NET-23` Attack / Hit 可采用服务器触发的表现事件，死亡必须有持续复制状态；`PlayAnimMontage` 不等于自动广播。迟到的攻击 / 受击事件不得覆盖死亡。
- [ ] `NET-24` 服务器上的攻击命中入口检查 Authority、本次攻击编号、有效目标与未结算状态；客户端 Notify 不扣血。Host Camera 看不到敌人时，服务器仍必须正确推进攻击命中。
- [ ] `NET-25` 核查 SkeletalMesh 的 `VisibilityBasedAnimTickOption` 与实际 Notify 行为；若依赖动画命中，验证服务器动画确实推进；若采用服务器定时攻击窗口，统一窗口与 Notify 的单次结算，不能各扣一次。
- [ ] `NET-26` 替换三处玩家 0 依赖：AI 选目标、`ApplyAttackDamage()` 取目标、Spawner 更新 / 结算；目标死亡 / 退出后重新选择。
- [ ] `NET-27` GameMode 负责回合规则；新增 GameState 继承与当前 `AGameModeBase` 配套的 `AGameStateBase`，并设置 `GameStateClass`，提供阶段、波次、活敌人数与结果。Spawner 可保留敌人数组和生成职责，停止直接调用某个玩家 `WinGame()`。
- [ ] `NET-28` 明确阶段转换：WaitingForPlayers → Spawning → InWave → Intermission / 下一波 → Victory / Defeat；异常进入中止状态。状态名为建议；最低版 Intermission 可为零时长。
- [ ] `NET-29` 只统计成功生成且登记的敌人；按逻辑死亡只移除一次，不等尸体销毁。生成中不允许因暂时 0 敌人提前判定清波；处理配置错误、部分 / 全生成失败。
- [ ] `NET-30` 将个人死亡与全局失败分离；死亡 / 清波事件先申请结算，再在服务器统一结算点汇总存活人数与波次，保证同一帧全灭优先失败。不要由两个伤害回调分别立即确定相反结果；终局最多确定一次。
- [ ] `NET-31` 终局停止刷怪、AI 行为、攻击 / 换弹 Timer 和伤害入口；验证尸体清理不会触发第二次结算。
- [ ] `NET-32` 实现服务器统一重开；本轮建议非无缝 ServerTravel 到当前地图，记录加载中的重新连接与等待就绪流程。不承诺保留旧 Actor / HUD，重新初始化后双方可继续第二局。
- [ ] `NET-33` 验证退出、中途加入拒绝与网络失败提示；不将「不做重连」理解为可以卡住或崩溃。
- [ ] `NET-34` 单独验证 Dash：服务器认可次数、方向、强度与落地重置；先接受有说明的远程启动延迟。若持续严重回弹且超预算，最低版关闭联网 Dash，并更新操作说明，不留下半完成能力。

服务器动画推进需按实际 5.4 引擎验证；本地类型定义可查 `Engine/Source/Runtime/Engine/Classes/Components/SkinnedMeshComponent.h` 中的 `EVisibilityBasedAnimTickOption`。这里不要求为整个场景无差别开启高成本骨骼更新。

### 3.6 N4：可重复的网络测试

必须覆盖独立的 Host 与远程 Client；「主机能玩」不能替代 Client 验证。PIE 初检后，至少进行一次多进程与一次打包双实例测试。

| 测试 | 操作 | 通过条件 |
| --- | --- | --- |
| 连接与等待 | Host 先进入，Client 稍后连接 | 两人就绪后才开始；没有空目标 / 重复刷怪 |
| 移动与视图 | 双方移动、转向、跳跃 | 彼此可见；输入、Camera、HUD 不串联 |
| 开火与转向 | 各自连射 10s；按住开火转身 | 权威弹药递减、方向持续更新；不重复生成子弹 |
| 松开 / 中断 | 松开、取消输入、换弹、死亡 | 不出现持续连射或残留 Timer |
| 换弹边界 | 满弹匣、无备用弹药、重复按、换弹中终局 | 拒绝非法换弹；弹药非负且不超容量 |
| 碰撞与反馈 | 致死命中、贴墙射击、队友挡在前方 | 一次有效伤害；射手命中标记正确；无友伤 / 穿墙起射 |
| AI 与动画 | A 吸引，B 接近；目标死亡；Host 转开视角 | AI 合理换目标；Client 看见动作；服务器命中不依赖 Host 是否看到 |
| 死亡与波次 | 同时射杀一敌、尸体尚在时清波 | 活敌计数一致，不为负，不重复清波 |
| 终局边界 | A 死亡后 B 清场；全灭；最后击杀与死亡接近同时 | 个人死亡不误判全败；共享结果一致且只结算一次 |
| 重开与退出 | 连续 3 局；Client 退出；Host 退出 | 双方可重开；退出有明确状态，不永久等待 |
| 中途连接 | 战斗中尝试接入 | 按规则拒绝并提示，不破坏现有回合 |

网络模拟采用下列**项目自定验收档位**，不是性能承诺：

| 档位 | 配置与记录 | 测试要求 |
| --- | --- | --- |
| A 基准 | 不额外注入延迟 / 丢包，记录实际 RTT | 完整一局与终局重开 |
| B 延迟 | 调整模拟使观察到的 RTT 约 100ms，记录真实配置 | 完整一局；重点看连射转向、换弹、Dash / 命中 |
| C 延迟 + 丢包 | B 的配置上，对明确指定的方向注入约 1% 丢包 | 完整一局或至少 10 分钟；结束后状态收敛，无永久连射 / 错误计数 |

- [ ] `NET-35` 使用 PIE Network Emulation；逐项记录作用端、Incoming / Outgoing、Min / Max、Loss 和观察 RTT，先 A 再 B 再 C。
- [ ] `NET-36` 不把设置栏里的 100ms 直接称为 100ms RTT；发送 / 接收以及两端同时注入可能叠加。以记录到的往返指标说明体验。
- [ ] `NET-37` 记录 Host / Client 日志、构建版本、步骤、预期、实际与证据；日志包括 Actor 名、Role、Authority、Owner、攻击 / 射击编号和波次阶段。
- [ ] `NET-38` 独立进程测试，排除单进程 PIE 共享状态掩盖问题；打包后再运行同一矩阵的核心项。

网络模拟字段含义参照 [Epic Network Emulation](https://dev.epicgames.com/documentation/unreal-engine/using-network-emulation-in-unreal-engine)，菜单与命令名以本地 UE5.4 为准。短暂 Cosmetic 丢失可以列为限制；权威血量、弹药、死亡、回合状态必须最终一致。

### 3.7 P2 验收红线

- [ ] 双人同局、彼此可见、输入正常；Remote Client 真正能射击、换弹、受伤、杀敌。
- [ ] 服务器判定射速、弹药、命中伤害、AI、波次与结果；没有双份 Actor 或重复伤害。
- [ ] AI 可以选择不同存活玩家；不可见时搜索，不持续追踪实时位置。
- [ ] 双端生命、弹药、活敌人数与回合结果一致；个人死亡与全队失败规则正确。
- [ ] RTT 约 100ms 与指定丢包档位下无永久错误状态；已说明射击反馈和 Dash 的限制。
- [ ] 连续 3 局无旧 Timer / 重复 Widget；退出与战斗中连接有明确处理。
- [ ] 最小调试和测试记录足以解释一次 Client 开火到服务器扣血的完整链路。

## 4. P3：武器数据驱动（标准版必须）

### 4.1 先解决当前数据来源

当前射速、弹匣、换弹与表现参数在 `AMyWeapon`，伤害在 `AMyProjectile::Damage`。抽配置时必须同时打通 Projectile 的伤害初始化，不能只在 Data Asset 新增一个没有被使用的 `Damage` 字段。

| 数据层 | 建议归属 | 示例 |
| --- | --- | --- |
| 只读配置 | 计划新增 `UWeaponDataAsset : UDataAsset` | Damage、FireInterval、FireMode、MagazineSize、StartingReserveAmmo、ReloadTime、ProjectileClass、Mesh / Effects、Recoil / Sway |
| 每把武器运行状态 | `AMyWeapon` 的服务器实例 | CurrentAmmo、ReserveAmmo、冷却、换弹、开火请求与计时器 |
| 每发子弹运行状态 | `AMyProjectile` | 本次伤害快照、Instigator、是否已命中 |
| 本地表现状态 | 所属客户端的持枪表现 | CurrentRecoil、CurrentSway、视觉换弹进度；不写回全局配置 |

先用普通 `UDataAsset` 和直接引用即可。只有实际需要 Asset Manager 的主资产标识、分组加载等能力时再选 `UPrimaryDataAsset`；参照 [Epic Data Assets（5.4）](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-assets-in-unreal-engine?application_version=5.4)。Data Asset 保存配置，不保存每个玩家的当前弹药，也不自动成为复制状态。

### 4.2 任务清单

- [ ] `WEAPON-01` 盘点 `.h/.cpp` 与 Blueprint 覆盖值，先迁移默认 Rifle 配置并保持现有手感。
- [ ] `WEAPON-02` 创建 `UWeaponDataAsset`，校验非空类、有限且合理的伤害、正的 FireInterval / ReloadTime、弹匣和备用弹药范围。
- [ ] `WEAPON-03` 配置必须在权威运行状态初始化前可用；检查 Spawn / BeginPlay 次序，必要时使用 Deferred Spawn 或明确初始化入口，避免先按旧默认值填满弹匣。
- [ ] `WEAPON-04` 服务器生成 Projectile 时写入本次伤害快照；在其可能碰撞前完成初始化。移除或明确旧 Damage 的回退规则，防止两个可编辑来源互相覆盖。
- [ ] `WEAPON-05` 创建 `DA_Rifle` / `DA_Pistol`。最低数据差异为伤害、射速和弹匣；若 Pistol 设为半自动，FireMode 必须改变实际输入 / 服务器逻辑，并验证按住不连发。
- [ ] `WEAPON-06` 保持服务器校验；客户端只读已知合法配置引用 / 标识，不能提交任意射速、伤害或配置路径改变规则。
- [ ] `WEAPON-07` 用同一实现分别运行两套配置并完成双端回归，记录服务器伤害和弹药证据；局内切枪为选做。
- [ ] `WEAPON-08` 检查配置及其 Mesh、Projectile、Effects 都进入 Cook；使用软引用时明确加载与打包规则，打包版不得丢失武器资产。

**验收标准：**新增同类武器主要依赖配置与必要资产，不复制 `Fire/Reload` 代码；两个配置实际改变服务器数值；多人、死亡取消、换弹、命中反馈均不回归。

## 5. P4：调试工具与工程质量

### 5.1 最小调试与增强项

- [ ] `DEBUG-01` 先使用已有 Behavior Tree / Blackboard / Perception / NavMesh 工具；核实 Gameplay Debugger 在本地构建中的可用入口。
- [ ] `DEBUG-02` 最小版本提供日志或简单屏显：敌人 State / Target / Distance / Health / IsDead、回合阶段 / Wave / 活敌数、玩家血量 / 弹药。
- [ ] `DEBUG-03` 网络诊断显示本地 / 远程、Role、Authority、Owner，以及服务器接受 / 拒绝请求的原因。
- [ ] `DEBUG-04` 为攻击与命中增加实例名、编号和时间；能够判断一次伤害来自哪个敌人的哪次攻击。
- [ ] `DEBUG-05` 通过 Debug 开关 / Console Variable / 非 Shipping 条件控制显示；高频日志只在定位问题时启用。
- [ ] `DEBUG-06` 每个 Bug 保存重现、原因、修复提交与回归结果；不要只存一张「看起来正常」的截图。
- [ ] `DEBUG-07` 选做：添加专用 Gameplay Debugger 分类；依赖 `GameplayDebugger` 模块时按目标与构建条件处理，避免发布构建依赖编辑器模块。

自定义 Debugger 分类不是最低交付门槛；能定位状态与权限问题才是目的。`MyArenaShooter.Build.cs` 已有 `AIModule`、`EnhancedInput`、`UMG`、`Niagara`；新增 BT / Debugger 代码时再按实际 include 和链接需求补依赖，不预先添加无关模块。

### 5.2 问题与验收记录

| ID | 现象 / 对应任务 | 环境与步骤 | 原因 / 修复提交 | 回归结果 |
| --- | --- | --- | --- | --- |
| BUG-001 | 致死命中无标记 / BASE-01 | 待实测补充 | 已定位致死返回 false；修复提交待填 | 待测 |
| BUG-002 | 死亡动画播放失败无销毁 / BASE-02 | 待构造失败路径 | 已定位缺少兜底；修复提交待填 | 待测 |
| BUG-003 | 后坐力不恢复、摆动不生效 / BASE-03 | 待实测补充 | 当前缺少恢复 / 写回；修复提交待填 | 待测 |

测试记录至少包含：任务 ID、源码提交、UE / 构建类型、地图 / 波次配置、玩家数、网络参数、步骤、预期、实际、结论、日志 / 视频路径。`未测`、`失败` 与 `通过` 分开记录，文档整理不能自动转为完成。

## 6. P5：性能分析与优化（选做）

先固定压力场景，采集 Baseline，再定位瓶颈、修改和同条件复测。若只定位了瓶颈但没有合理优化收益，也如实输出分析，不为了简历凑百分比。

- [ ] `PERF-01` 固定设备 CPU / GPU、分辨率、画质、地图、玩家 / 敌人数、射击频率、构建类型与网络条件。
- [ ] `PERF-02` 固定预热和采样时长（例如预热 30s、采样 60s，重复 3 次）；Host 与 Client 分开记录。不要把两个窗口争用 GPU 的结果当作服务器逻辑耗时。
- [ ] `PERF-03` 使用 `stat unit`、`stat game`、Unreal Insights 区分 Game Thread / Render Thread / GPU；说明平均与 P99 的统计对象。
- [ ] `PERF-04` 如分析复制成本，核实 UE5.4 Network Insights 的采集开关，记录流量方向与单位，观察 Actor / RPC 热点。
- [ ] `PERF-05` 只有创建 / 销毁确认为瓶颈时才试 Projectile 对象池；网络 Actor 的重激活、旧状态、碰撞 / Timer / Owner 重置与复制相关性单独设计。
- [ ] `PERF-06` 保存相同负载的前后结果、修改提交和副作用；性能改动后重新跑双人核心回归。

| 版本 / 提交 | 设备 / 构建 / 场景 | 平均 GT ms | GT P99 ms | 总帧时间 P99 ms | 网络 KB/s 与方向 | 采样 / 次数 |
| --- | --- | ---: | ---: | ---: | --- | --- |
| Before | 固定后填写 | 待测 | 待测 | 待测 | 待测 | 待填 |
| After | 与 Before 一致 | 待测 | 待测 | 待测 | 待测 | 待填 |

不预填「优化 30%」。不同版本 PIE / Packaged、不同机器或不同画质的数据不能直接作为前后收益比较。

## 7. P6：可运行交付与面试准备

### 7.1 交付清单

- [ ] `SHIP-01` 检查无效引用与确实不用的临时资产；先备份 / 提交，再在 Editor 中渐进整理目录和 Redirectors，不批量删除来源不明的依赖。
- [ ] `SHIP-02` 新增内容按现有 `BluePrints`、`Maps`、AI、Weapons、UI 等职责组织；不为了目录统一重命名整个素材库。
- [ ] `SHIP-03` README 写明引擎 / 编译环境、插件、构建步骤、操作、PIE 双人启动与真实已实现功能。
- [ ] `SHIP-04` 绘制整体架构与联网开火时序的原生 `.drawio`，节点、文字、连线可编辑；保留真实类名、函数名与源码位置。
- [ ] `SHIP-05` 图纸、PNG 预览和学习说明统一放到 `D:\Materials\zhbbboy.github.io\draw\MyArena`；复用既有图纸并标注新旧实现日期，不能仅把截图装入 draw.io。
- [ ] `SHIP-06` 打包 Windows Development 版本，确认 `Arena` 被 Cook；包含武器数据、敌人 Montage、HUD、输入等依赖，脱离 Editor 运行。
- [ ] `SHIP-07` README 记录打包双实例的真实步骤：Host 以 Listen 模式打开 Arena，Client 通过主机地址连接；同机可用 loopback 验证，多机 LAN 测试结果单列。命令和端口以实测为准。
- [ ] `SHIP-08` 打包版测试远程 Client 开火 / 换弹、AI、胜败、退出和连续重开；用干净检出或另一位置验证构建 / 运行，不依赖 Saved / Intermediate 中的偶然缓存。
- [ ] `SHIP-09` 录制 1～2 分钟演示：双人视角 → AI 感知 / 失去目标 → 射击与换弹 → 同步死亡 / 波次 → 终局；调试与模拟延迟可补短片段。
- [ ] `SHIP-10` 整理独立完成内容、参考教程 / 仓库与素材许可证；保留 Known Limitations，不把计划功能写入已实现列表。

不强制 Shipping 构建或 Dedicated Server。最低交付版仍需真正的可运行包；仅两张 PIE 截图不足以证明作品可交付。

### 7.2 README 建议内容

项目定位与视频；实际功能；整体架构 / 调用链；Getting Started（UE5.4.4、Editor 构建、PIE 与打包双实例）；操作与双人规则；测试矩阵；已知限制；参考资料与素材许可。Windows 绝对路径用于本地学习定位，公开 README 的运行步骤不要绑定开发者个人目录。

### 7.3 面试需要能讲清的问题

1. 原 Tick AI 的问题是什么？Behavior Tree、Blackboard、Perception 分别解决什么？
2. 丢失感知后，为什么不能继续 `MoveTo TargetActor`？怎样选择第二个玩家？
3. 从 Client 按开火到敌人扣血、射手显示命中标记，完整调用链是什么？
4. 为什么要校验 RPC Ownership？Host 正常而 Client 不能开火时如何排查？
5. GameMode、GameState、PlayerController、PlayerState 分别适合存什么？
6. 属性复制、OnRep、Client RPC、Multicast 分别用于哪些状态和事件？
7. 为什么不能假定 OnRep 顺序？HUD / 武器引用晚到如何初始化？
8. Notify 如何防止多次扣血？Host 看不到敌人时服务器如何判定攻击？
9. CharacterMovement 如何处理基础移动？Dash 为什么可能出现修正？
10. 武器 Data Asset 的 Damage 如何真正送到 Projectile？配置与运行时弹药为何分开？
11. 如何证明约 100ms RTT 与丢包测试通过？怎样区分 Cosmetic 丢失和权威状态错误？
12. 有哪些独立改进、证据与已知限制？若做优化，测量条件和收益是什么？

## 8. 推荐执行节奏与止损点

每周约 15 小时；以下为标准版节奏，按验收推进，不按日历强行叠功能。

| 周次 | 工作重点 | 可展示的出口 |
| --- | --- | --- |
| 第 1 周 | P0，开始 P2-N1 | 稳定单机、已知缺口处理、真实版本基线 |
| 第 2 周 | P2-N1 + N2 最小链 | 远程 Client 发射服务器 Projectile 并使敌人扣血 |
| 第 3 周 | P1 | 感知、追击、攻击、丢失目标与双目标选择 |
| 第 4～5 周 | P2-N2 补全 + N3 | 伤害 / 弹药 / 动画 / 波次一致，个人死亡与全局终局正确 |
| 第 6 周 | P2-N4 + P4 | 延迟 / 丢包、退出、连续重开、多进程验证 |
| 第 7 周 | P3 | 两套数据配置影响实际服务器数值并通过回归 |
| 第 8 周 | P6 | 打包、图纸、README、视频、验收记录 |
| 第 9～10 周 | 联调缓冲 | 消除阻断问题；已经达标时提前收尾 |

**止损点：**第二周仍无法跑通 Client → Server → Damage 时，暂停 AI 新分支和视觉扩展，先定位权限与生命周期；网络阶段超预算时，优先取消 P3、复杂巡逻、联网 Dash 和自定义 Debugger 分类，转为最低交付版。P5 永远不阻塞作品收尾。

## 9. 每次开发的工作方法

1. **设计**：写清目标、参与类、配置 / 状态归属、谁有修改权限、正常 / 中断 / 结束路径。
2. **实现**：一次只打通一条完整调用链；将权限判断、伤害和状态放在清楚的入口。
3. **验证**：单机、Host、Remote Client 分别验；联机能力至少有一次 Client 主动操作证据。
4. **记录**：保存源码提交、环境、问题、修复原因、回归结果与已知限制。
5. **提交**：通过后提交，再开始下一项；运行失败时不继续叠功能。

每个功能的笔记可使用以下模板：

```markdown
## 功能：XXX

- 任务 ID / 源码提交：
- 玩家可见目标：
- 涉及类、函数与源码位置：
- 配置 / 运行时状态 / 本地表现分别由谁持有：
- 修改权限（Server / Owning Client）：
- 调用链与复制方式：
- 正常、取消、死亡、退出、重开路径：
- 测试环境 / 网络参数 / 操作步骤：
- 预期 / 实际 / 结论（未测、失败、通过）：
- 日志 / 截图 / 视频位置：
- 已知限制与设计取舍：
```

不要为了拆分而预先增加 HealthComponent、InventoryComponent 等大量抽象；出现清晰复用需求后再提取。蓝图用于资产、动画与表现配置，核心规则与权限在 C++ 中维护。重命名旧接口前检查 Blueprint 引用；当前真实拼写仍有 `UpadteAmmoHUD/UpadteWaveHUD`，武器换弹标志已为 `bIsReloading`。

## 10. 风险与定位入口

| 风险 / 症状 | 优先排查 |
| --- | --- |
| Client 不开火，Host 正常 | RPC 在哪个 Actor 上、复制是否开启、Owner 链、CurrentWeapon 是否已到达 |
| 双枪 / 双弹 / 双 HUD | BeginPlay 权限、本地控制判断、重复绑定、表现事件去重 |
| AI 只打玩家 0，或起手打 A 却伤到 B | Controller 和 `ApplyAttackDamage` 是否都迁移目标、本次攻击目标是否固定 |
| Host 转开视角后 AI 无伤害 | 服务器动画 Tick / Notify、Authority、本次攻击是否被错误取消 |
| 单次近战扣两次血 | 敌人实例、攻击编号、Notify 数量、定时窗口与 Notify 是否双结算 |
| 无敌人却无法推进，或刚开始就胜利 | Spawning 阶段、登记成功数量、空配置、全生成失败、死亡重复结算 |
| Client HUD 初始为零或 Host 不刷新 | Widget Owning Player、快照初始化、引用到达顺序、服务器本地刷新入口 |
| Client 枪口偏移、贴墙穿射、按住转向不跟随 | 权威视点 / 枪口与本地摆动分离、碰撞检测、瞄准更新 |
| 死后还补弹 / 开火，重开后出现旧状态 | EndPlay / 终局清理、Timer、委托、输入 Canceled、重复初始化 |
| PIE 正常，打包丢地图 / 武器 | Cook 地图、Data Asset / 软引用、资源加载、Editor 模块依赖 |
| 重开使 Client 掉回单机或无法再开局 | Client 本地 OpenLevel、ServerTravel、连接失败处理、人数与就绪条件 |
| Dash 持续回弹、Projectile 抖动 | 移动预测边界、重复运动驱动；按预算修复或关闭可选能力 |

## 11. 资料入口与版本约束

### 11.1 本地复习资料

- [第 1 节：角色的创建与基础移动](./第1节：角色的创建与基础移动.md)
- [第 2 节：武器系统](./第2节：武器系统.md)
- [第 3 节：敌人和 AI](./第3节：敌人和AI.md)
- [第 4 节：UI](./第4节：UI.md)
- [第 5 节：射击瞄准与反馈](./第5节：射击瞄准与反馈.md)
- [第 6 节：胜负状态与重新开始](./第6节：胜负状态与重新开始.md)
- [第 7 节：敌人战斗动画与伤害时机](./第7节：敌人战斗动画与伤害时机.md)
- [第 8 节：武器后坐力、换弹姿态与惯性摆动](./第8节：武器后坐力、换弹姿态与惯性摆动.md)

既有图纸位于博客仓库 `draw/MyArena/`：`MyArena-项目总框架与函数调用链.drawio`（早期阶段）和 `MyArena-战斗动画与武器表现-20261013.drawio`（动画与表现追加）。先看对应学习指南，再回到当前源码核对；新扩展需要继续更新图纸，不把旧图当作联网架构。

### 11.2 Epic 官方参考

优先使用 `application_version=5.4` 页面；若页面不可用或跳到新版本，用本地 UE5.4 头文件与 Editor 行为核对。不直接引入新版本专有 API。

- [Networking Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/networking-overview-for-unreal-engine?application_version=5.4)
- [Actor Owner and Owning Connection](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-owner-and-owning-connection-in-unreal-engine?application_version=5.4)
- [Replicate Actor Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicate-actor-properties-in-unreal-engine?application_version=5.4)
- [Behavior Tree Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---quick-start-guide?application_version=5.4)
- [AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine?application_version=5.4)
- [Animation Notifies](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-notifies-in-unreal-engine?application_version=5.4)
- [Gameplay Debugger](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-the-gameplay-debugger-in-unreal-engine?application_version=5.4)
- [Performance Profiling](https://dev.epicgames.com/documentation/en-us/unreal-engine/introduction-to-performance-profiling-and-configuration-in-unreal-engine?application_version=5.4)

本地额外阅读：`Engine/Source/Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h`（Task 生命周期与 NodeMemory）；`Engine/Source/Runtime/Engine/Classes/Components/SkinnedMeshComponent.h`（可见性相关动画 Tick）。

## 12. 开工入口与停止条件

**当前进度：已完成资料 / 源码核对；P0 的工程修改、重新编译与本轮回归尚未执行。**

下一次从以下四步开始：

1. 编译并运行当前 `Arena`，保存日志；确认 5.4.4 环境与实际 Blueprint 配置。
2. 建立源码 / 资产版本基线，优先处理 BASE-01 / BASE-02，再补表现、攻击单次结算与回合边界。
3. 完成 P0 单机回归后，推进 P2-N1 与 N2 最小链，先让远程 Client 真正造成一次服务器伤害。
4. 以该结果重新估时，再进入 P1；不一次性重写整个武器、AI 和回合系统。

下一次可以直接使用：

> 按《MyArenaShooter 扩展开发计划》推进 P0。请先核对当前源码与 Editor 配置，完成一个可测试的小目标，保留真实类名、函数名与源码位置；区分已实现、待接通和待验证。P0 通过后先做 Client → Server → Projectile → Damage 的最小联网链，再升级 AI。

**最低交付版停止条件：P0 出口 + P1 基础验收 + P2 红线 + P4 最小诊断 + P6 可运行包与展示材料全部通过。标准版在此基础上必须完成 P3 及双配置回归。** EyeDrone、局内切枪、性能优化与 Dedicated Server 不阻塞任何一版收尾；达到对应门槛后进入 GAS Boss Demo。

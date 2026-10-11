# MyArena 项目总框架与函数调用链

这套图纸依据当前 `MyArenaShooter` 工程的 C++ 和配置文件整理，核对日期为 2026-10-11，关联 Unreal Engine 版本为 5.4。图纸包含 10 个 draw.io 原生可编辑页签，帮助你从当前 demo 复习每条调用链，再逐步复写和扩展。

工程目录：`D:\code\Unreal Projects\MyArenaShooter\MyArenaShooter`。
统一图纸目录：`D:\Materials\zhbbboy.github.io\draw\MyArena`。

本次没有修改游戏 C++、蓝图或项目配置，也没有运行 Editor 构建或 PIE。蓝图资产的存在已核对；内部属性赋值、事件图及地图实例配置尚未读取。所有参数数值指 C++ 默认值，蓝图可能覆盖。

## 打开与完善图纸

1. 用 draw.io 打开 [MyArena-项目总框架与函数调用链.drawio](MyArena-项目总框架与函数调用链.drawio)。底部页签为 00–09。
2. 节点、文字、连线和分组背景均为原生对象，你可以直接增加自己的笔记或修改流程。
3. 先看 01 总框架，再看 02 → 04 → 05 → 08，读通射击、伤害、清波与胜利。之后补 03、06、07 和 09。
4. 每页先读 `.h` 的状态和引用，再沿图中标注的 `.cpp:行号` 回到具体实现。
5. PNG 仅供预览；你修改 `.drawio` 后，需要从 draw.io 重新导出相应 PNG。

图中的箭头含义：实线表示直接调用或同一个函数内的步骤；蓝虚线表示 UE 生命周期、输入、碰撞委托或 Timer 调度；灰点线表示引用或状态依赖；橙虚线用于配置与建议关联。同函数内分步画出的节点没有对应新函数。

## 页签与复习目标

| 页签 | 预览 | 学完应能解释 |
| --- | --- | --- |
| 00 学习导航 | [PNG](MyArena-项目总框架与函数调用链-00-学习导航.png) | 按什么顺序从图纸回到源码，再独立复写？ |
| 01 总框架 | [PNG](MyArena-项目总框架与函数调用链-01-总框架.png) | 8 个类各负责什么，谁持有什么引用？ |
| 02 启动与生命周期 | [PNG](MyArena-项目总框架与函数调用链-02-启动与生命周期.png) | 谁创建玩家、HUD、武器和敌人，谁触发 UE 回调？ |
| 03 输入与移动 | [PNG](MyArena-项目总框架与函数调用链-03-输入与移动.png) | InputAction 如何转成移动、视角、跳跃和冲刺？ |
| 04 开火、连射与瞄准 | [PNG](MyArena-项目总框架与函数调用链-04-开火、连射与瞄准.png) | 按住开火如何连射，枪口与准星如何对齐？ |
| 05 子弹命中与敌人受伤 | [PNG](MyArena-项目总框架与函数调用链-05-子弹命中与敌人受伤.png) | 碰撞如何扣血，命中反馈和敌人销毁如何发生？ |
| 06 换弹与弹药 HUD | [PNG](MyArena-项目总框架与函数调用链-06-换弹与弹药-HUD.png) | 换弹如何延迟完成，弹药怎样转移到弹匣？ |
| 07 AI、玩家受伤与失败 | [PNG](MyArena-项目总框架与函数调用链-07-AI、玩家受伤与失败.png) | AI 怎样追击和攻击，玩家死亡后做了什么？ |
| 08 波次、胜利与重开 | [PNG](MyArena-项目总框架与函数调用链-08-波次、胜利与重开.png) | 何时清波、如何进入下一波，重开如何恢复状态？ |
| 09 蓝图检查与扩展路线 | [PNG](MyArena-项目总框架与函数调用链-09-蓝图检查与扩展路线.png) | 配置需要检查哪里，新增功能应放在哪个对象？ |

## 先建立对象与数据的对应关系

| 类 / 数据 | 基类 | 职责与关键数据 | 阅读入口 |
| --- | --- | --- | --- |
| `AMyArenaShooterGameMode` | `AGameModeBase` | 从蓝图选 `DefaultPawnClass`；目前不管理波次或胜负 | `MyArenaShooterGameMode.cpp:6` |
| `AMyArenaShooterCharacter` | `ACharacter` | 相机、输入、移动、血量、胜败标志；持有武器与 HUD 实例 | `MyArenaShooterCharacter.h:18`、`.cpp:33` |
| `AMyWeapon` | `AActor` | 枪口、模型、实体子弹类；连射冷却、弹药、换弹 Timer | `MyWeapon.h:17`、`.cpp:34`、`:53` |
| `AMyProjectile` | `AActor` | 碰撞根、显示模型、ProjectileMovement、伤害、命中特效 | `MyProjectile.cpp:12`、`:43`、`:72` |
| `AMyEnemyCharacter` | `ACharacter` | 敌人血量、接收子弹伤害、归零销毁；指定 AI 控制器 | `MyEnemyCharacter.cpp:7`、`:46` |
| `AMyEnemyAIController` | `AAIController` | 缓存目标玩家；每帧判断追击、攻击距离与攻击冷却 | `MyEnemyAIController.cpp:13`、`:21` |
| `AMyEnemySpawner` | `AActor` | 敌人类、出生点、波次配置；维护活跃敌人数组和波次索引 | `MyEnemySpawner.cpp:13`、`:23`、`:71` |
| `UMyHUDWidget` | `UUserWidget` | 接收数据与显示反馈；不保存权威血量、弹药或波次 | `MyHUDWidget.h:18`、`.cpp:10` |
| `FEnemyWaveData` | `USTRUCT` | 一波的 `EnemyCount`，默认 1；它是数据结构 | `MyEnemySpawner.h:11` |

表中 `.cpp` 均在 `Source/MyArenaShooter/Private`，`.h` 均在 `Source/MyArenaShooter/Public`。完整的源码位置列表保存在 [索引 JSON](MyArena-项目总框架与函数调用链-索引.json) 中。

`TSubclassOf<AMyWeapon>` / `TSubclassOf<UMyHUDWidget>` 表示“选择要创建的类”；`CurrentWeapon` / `HUDWidget` 指向运行时创建的实例。不要把类配置和对象实例当成同一个东西。

对象来源也要分清：武器的 `Owner` 和 `Instigator` 都是玩家；子弹的 `Owner` 是武器，`Instigator` 仍是发射玩家。武器通过 `GetOwner()` 找玩家刷新弹药，子弹通过 `GetInstigator()` 找玩家显示命中标记。

## 六条必须能独立复述的调用链

### 1. 启动到初始 HUD / 武器

`DefaultEngine.ini` 选择默认地图与原生 GameMode。GameMode 构造函数使用 `FClassFinder` 查找 `/Game/BluePrints/BP_MyArenaShooterCharacter`，找到后设为 `DefaultPawnClass`。UE 负责生成和控制 Pawn。

玩家 `BeginPlay` 的内部顺序是：初始化血量 → 创建 HUD、加入视口并显示血量 → 注册 `DefaultMappingContext` → 创建默认武器 → 挂到相机并设置偏移 → 读取初始弹药并显示。武器 `BeginPlay` 初始化弹匣数与备用弹药。

`SetupPlayerInputComponent` 也是 UE 调用的初始化入口，负责 `BindAction`，它不是玩家 `BeginPlay` 直接调用的函数。其他 Actor 的 `BeginPlay` 由 UE 分别调度，图纸不保证跨 Actor 的固定启动顺序。

### 2. 开火到实体子弹生成

`FireAction Started` → 玩家 `StartFireInput` → 武器 `StartFire`。`StartFire` 先设 `bFirePressed=true`，符合条件时可以立即调用一次 `Fire`，然后设置射击间隔。后续武器 `Tick` 不断减冷却，在按住、非换弹、冷却结束时再次调用 `Fire`。

`Fire` 从玩家取相机，发出长度为 10000 的相机射线，检测 `Pawn` 与 `WorldStatic`，忽略玩家和武器。射线命中则选 `Hit.ImpactPoint`，否则选相机远端点。再从枪口朝瞄准点计算方向，调用 `SpawnActor<AMyProjectile>`。

**相机射线只负责选瞄准点。它不直接伤害敌人，伤害来自实体子弹碰撞。** 子弹成功生成后，`Fire` 播放已配置的枪口反馈，扣一发弹药、刷新 HUD；打空时尝试自动换弹。子弹生成失败则不扣弹。`StartFire` / `Tick` 调用 `Fire` 后仍会设置冷却，`Fire` 没有返回成功标志。

`FireAction Completed` → 玩家 `StopFireInput` → 武器 `StopFire` → `bFirePressed=false`，停止后续连射。

### 3. 子弹碰撞到敌人死亡与命中反馈

子弹 `BeginPlay` 绑定 `CollisionComponent.OnComponentHit` 到 `OnProjectileHit`，并在移动碰撞中忽略玩家与武器。移动由 `UProjectileMovementComponent` 负责；子弹 Actor 的 `Tick` 是关闭的。

碰撞发生 → `OnProjectileHit` → 若命中敌人，则调用 `TakeProjectileDamage(Damage)`。敌人拒绝已死、正在销毁、非有限或非正伤害；有效伤害扣血，归零则 `Destroy`，并返回 `true`。当返回 `true` 时，子弹通知发射玩家 `NotifyEnemyHit` → HUD `ShowHitMarker`，0.15 秒后隐藏。

命中标记表示有效伤害，普通受伤与最后一次击杀都可触发。之后碰撞函数按配置播放命中位置的特效和声音，最后销毁子弹；命中墙体也会走这些反馈与销毁步骤。

**敌人 `Destroy` 不直接调用刷怪器。** 后续刷怪器 `Tick` 调用 `CleanupDeadEnemies`，发现引用无效后移出数组、更新剩余数量，再检查是否清波。

### 4. 换弹到恢复连射

玩家 `ReloadInput` 或武器自动空弹检查 → `StartReload`。已在换弹、弹匣已满、备用弹药为零，任一条件满足就返回；否则设置 `bIsReoloading=true`，启动单次 Timer。

默认 1.5 秒后 → `FinishReload`：

```text
NeededAmmo = MagazineSize - CurrentAmmo
AmmoToReload = Min(NeededAmmo, ReserveAmmo)
CurrentAmmo += AmmoToReload
ReserveAmmo -= AmmoToReload
bIsReoloading = false
UpdateAmmoHUD()
```

显示调用链是：武器 `UpdateAmmoHUD` → 玩家 `UpadteAmmoHUD` → HUD `UpdateAmmo`。弹药数据在武器，玩家负责转发，HUD 负责显示。

换弹不清除 `bFirePressed`。若仍按住开火，完成换弹后的下一次符合条件的 Tick 可以继续连射；松开后则不会继续。源码拼写为 `bIsReoloading`、`UpadteAmmoHUD`，搜索时请保持一致。

### 5. AI 追击 / 攻击到玩家失败

敌人构造函数指定 `AMyEnemyAIController`，并允许关卡放置或动态生成时自动控制。AI `BeginPlay` 缓存玩家 0，AI `Tick` 每帧读取自己控制的 Pawn 与目标玩家。

距离大于默认 150 → `MoveToActor(Player, 50)`；否则 `StopMovement`，攻击冷却结束时调用玩家 `ReceiveEnemyDamage(20)`，并重设默认 1 秒冷却。当前攻击按距离和冷却直接造成伤害，没有行为树、攻击动画通知或攻击射线校验。

玩家 `ReceiveEnemyDamage` 拒绝死亡、胜利或非正伤害，接受伤害时扣血 → `UpdateHealth` → `ShowDamageFlash` → 条件满足时播放受击音效 → 归零则 `Die`。`Die` 设置死亡标志、停止武器开火、禁用角色移动并显示 `GameOver`。

### 6. 清波到胜利，再重开

刷怪器 `SpawnWave` 根据本波 `EnemyCount` 循环生成敌人。出生点按 `i % SpawnPoints.Num()` 轮流选择，位置向上偏移 100。成功生成的敌人加入 `CurrentWaveEnemies`，有存活敌人时设置活动波次，并通过玩家更新波次 HUD。

刷怪器 `Tick` 在活动波次中清理无效引用。当数组为空：停用本波 → 增加 `CurrentWaveIndex` → 有下一波则立即 `SpawnWave`；全部完成则尝试调用玩家 `WinGame`。`WinGame` 设置胜利标志、停止开火、禁用移动并显示胜利。

`RestartAction Started` → `RestartGame`，仅死亡或胜利后通过；取当前地图名并 `OpenLevel`，重新加载整张地图，重新进行初始化以恢复血量、弹药与波次。

## 当前实现的边界：复写前不要误记

- `GameMode` 只选择玩家 Pawn；波次在 `Spawner`，胜败标志在玩家。
- 只有武器、AI 与刷怪器的 Tick 有业务逻辑。玩家 Tick 虽开启，当前仅调父类；敌人和子弹 Actor Tick 关闭。
- HUD 数据是主动推送；敌人死亡统计是刷怪器 Tick 轮询。HUD 不从玩家每帧自行查询全部状态。
- 刷怪器只统计成功加入自己数组的敌人，地图中另外摆放的敌人不会自动进入这份列表。
- 空 `Waves`、零敌人波次、没有有效出生点或全部生成失败时，当前没有自动跳过或重试流程，可能停在非活动状态。
- AI 只在 `BeginPlay` 获取玩家，没有持续重找目标；刷怪器在全部波次结束时会尝试重新获取无效玩家，普通波次 HUD 更新没有这项重试。
- 死亡和胜利没有暂停世界，也没有主动停止 AI / Spawner Tick。后续伤害由玩家状态检查拒绝。武器换弹 Timer 未显式取消。
- 输入终局检查并不统一：开火和换弹检查死 / 胜，Move 只检查死亡，Look 不检查，Dash 只检查 `bCanDash`，Jump 直接绑定 UE 基类函数。
- 冲刺恢复依赖 `Landed`；没有独立时间冷却或次数系统。
- 玩家受伤入口未像敌人入口一样检查 `FMath::IsFinite`，抽通用生命组件时可统一校验。
- `NativeConstruct` 只主动隐藏命中标记和受伤闪红，`GameOverText`、`VictoryText` 的初始显示状态要在 WBP_HUD 中检查。
- 使用玩家索引 0，当前源码没有 RPC 或复制状态配置；联机需要额外设计。
- 当前没有计分、掉落、切枪、存档或波次间延时。09 页所列的新功能是扩展建议。

## 编辑器配置检查表

| 检查对象 | 已确认存在的资产 / 接口 | 在 Editor 中继续核实 |
| --- | --- | --- |
| 地图与 GameMode | `Content/Test.umap`；默认地图与原生 GameMode 配置 | World Settings 是否覆盖 GameMode、PlayerStart 与运行场景 |
| 玩家蓝图 | `Content/BluePrints/BP_MyArenaShooterCharacter.uasset` | 父类、DefaultWeaponClass、HUDWidgetClass、输入资产与受击声音 |
| 输入 | `IMC_Player`；IA_Move / Look / Jump / Dash / Fire / Reload / Restart | 对应按键、Value Type、触发器；具体按键本次未读取 |
| 武器 | `Content/BluePrints/Weapons/BP_MyWeapon.uasset` | ProjectileClass、枪模型、枪口偏移、弹药、射速与特效声音 |
| 子弹 | `Content/BluePrints/Weapons/BP_MyProjectile.uasset` | 模型、碰撞、Damage、ImpactEffect / ImpactSound |
| 敌人与刷怪器 | `BP_MyEnemyCharacter`、`BP_MyEnemySpawner` | 地图中的刷怪器实例、EnemyClass、SpawnPoints、Waves、导航区域 |
| 敌人动画 | `Content/Art/SciFi/Enemies/QuadShell/ABP_MyEnemy.uasset` 等 | SkeletalMesh 与 AnimClass 绑定、动画图实际逻辑 |
| HUD | `Content/BluePrints/UI/WBP_HUD.uasset` | 控件名称和类型对应 `BindWidget`；终局文字初始隐藏 |
| 特效 | `Content/Effects/NS_MuzzleFlash.uasset`、`NS_Impact.uasset` | 对应类属性是否实际引用这些系统 |

HUD 控件：`HealthBar` 为 ProgressBar，`DamageFlashOverlay` 为 Border，其余 `HealthText`、`WaveText`、`EnemyCountText`、`AmmoText`、`HitMarkerText`、`GameOverText`、`VictoryText` 为 TextBlock。

## 按顺序复写自己的 demo

| 步骤 | 独立完成的最小功能 | 自己验收的问题 |
| --- | --- | --- |
| 1 | GameMode + 玩家 Pawn + 相机 | 从新地图启动能否生成并控制玩家？ |
| 2 | Enhanced Input + 移动、视角、跳跃 | Action 从哪里赋值、什么时候绑定？ |
| 3 | 武器生成与相机挂载 | Class 配置与实际对象实例有什么区别？ |
| 4 | 开火输入、冷却、瞄准和实体子弹 | 射线负责什么，碰撞负责什么？ |
| 5 | 一个敌人 + 接收伤害与销毁 | 四发默认子弹是否能击杀默认 100 血敌人？ |
| 6 | HUD + 弹药、换弹与命中反馈 | 状态保存在谁身上，显示何时更新？ |
| 7 | AI 追击、距离攻击与玩家死亡 | 哪个对象 Tick，冷却由谁维护？ |
| 8 | 刷怪器、波次、胜利、地图重开 | 死亡引用何时清理、空波次怎么处理？ |

每一步完成后，关掉参考实现，用自己的话写出“触发事件 → 状态检查 → 状态变化 → 下游调用”，再回到图纸核对。能讲清这四项后再增加下一项功能。

## 扩展功能放在哪里

| 想做的扩展 | 当前可以接入的位置 | 建议先做的最小改动 |
| --- | --- | --- |
| 调整射速、散射、后坐力 | `MyWeapon::Fire / Tick` | 保留瞄准点逻辑，加入射击方向偏差或相机反馈 |
| 多种子弹、爆炸、穿透 | `ProjectileClass / OnProjectileHit` | 增加子弹子类或行为参数，明确何时销毁 |
| 弹药拾取、取消换弹 | 武器弹药状态与 ReloadTimerHandle | 新增武器接口，统一更新 HUD，并定义 Timer 取消规则 |
| 切枪 | 玩家 `CurrentWeapon / DefaultWeaponClass` | 增加武器列表、切换时停止开火和处理旧武器 |
| 护甲、护盾、统一生命系统 | 玩家 `ReceiveEnemyDamage`、敌人 `TakeProjectileDamage` | 抽 `HealthComponent`，统一校验和死亡事件 |
| 不同敌人攻击 | `MyEnemyAIController::Tick` | 先把攻击参数改为可配置属性；当前是普通 private 成员 |
| 巡逻、感知、行为树 | AIController | 从目标获取与距离判断开始替换，保持伤害入口清楚 |
| 波次等待、Boss、动态难度 | `FEnemyWaveData / SpawnWave / Tick` | 增加显式波次状态，先补空波与生成失败处理 |
| 计分、掉落、成就 | 敌人死亡与波次系统之间 | 新增死亡委托，让管理者订阅，避免敌人硬编码调用 HUD |
| 全局结算与联机 | GameMode / GameState / PlayerState | 先集中胜败状态，再设计服务端权威、复制和 HUD 订阅 |

建议先用现有蓝图替换内容和参数，再增加必要的玩法状态；当重复逻辑出现后，再抽组件、数据资产和委托。扩展路线已经放在 09 页，你可以直接在 draw.io 中继续补充。

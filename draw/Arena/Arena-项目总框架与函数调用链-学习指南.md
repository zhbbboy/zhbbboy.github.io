# Arena 项目总框架与函数调用链：学习指南

核对日期：2026-10-11。依据当前工作树 C++，Git HEAD 为 `adf3668`（已有用户改动）。
工程根目录：`D:\code\Unreal Projects\Arena\UE5-Arena-Shooter-Sample`。Unreal 关联版本 5.4。
本轮没有改游戏代码、没有执行 Editor 构建或 PIE，也没有读取蓝图内部结构。

## 如何使用图纸

1. 用 draw.io 打开 [Arena-项目总框架与函数调用链.drawio](Arena-项目总框架与函数调用链.drawio)。底部页签按 00–12 排列。
2. 先读 01 总框架；从 02 → 04 → 05 → 07 开始，读通最小战斗闭环。随后补 03、06、08–12。
3. 节点显示类名、函数名和源码文件行号。图上的位置相对 `BoomerShooter/Source/BoomerShooter/`。
4. 先查看 .h 的声明和 UPROPERTY，再按调用链查看 .cpp；边读边在编辑器核验相关资产。
5. 实线表示直接调用或同函数内分支步骤；蓝虚线表示 UE 生命周期、输入或委托调度；灰点线表示状态/引用依赖；橙虚线表示蓝图未知连接。
6. 图中若将同一函数的多个步骤拆成节点，箭头只表示步骤顺序；不是为项目新造了函数。尤其 KilledEnemy 后的 UpdateEnemies 是 DamageEnemy 的下一条调用，KilledEnemy 自身不调用它。

## 页签与阅读问题

| 页签 | 预览 | 学完应能解释 |
| --- | --- | --- |
| 00 学习导航 | [PNG](Arena-项目总框架与函数调用链-01-00-学习导航.png) | 按什么顺序学习，每一页如何回到源码？ |
| 01 总框架 | [PNG](Arena-项目总框架与函数调用链-02-01-总框架.png) | 谁持有玩家、武器、敌人列表与管理器引用？ |
| 02 启动与生命周期 | [PNG](Arena-项目总框架与函数调用链-03-02-启动与生命周期.png) | 谁调度 BeginPlay / PostInitializeComponents / 输入初始化？ |
| 03 玩家输入与移动 | [PNG](Arena-项目总框架与函数调用链-04-03-玩家输入与移动.png) | Move、Look、Jump、Dash 如何接到 UE API？ |
| 04 切枪与武器挂载 | [PNG](Arena-项目总框架与函数调用链-05-04-切枪与武器挂载.png) | 旧武器何时销毁，新武器如何挂到相机？ |
| 05 开火与射线命中 | [PNG](Arena-项目总框架与函数调用链-06-05-开火与射线命中.png) | 为什么按键回调不直接开火？射线分支如何伤害敌人？ |
| 06 投射物命中 | [PNG](Arena-项目总框架与函数调用链-07-06-投射物命中.png) | 为什么命中处理在 Tick？EnemyBullet 与 ProjectileType 由谁决定？ |
| 07 敌人死亡、计分与波次 | [PNG](Arena-项目总框架与函数调用链-08-07-敌人死亡、计分与波次.png) | 死亡如何更新计分？下一波到底由谁调用？ |
| 08 AI 导航与攻击组件 | [PNG](Arena-项目总框架与函数调用链-09-08-AI-导航与攻击组件.png) | AI 导航和攻击组件为何是两条独立链？自爆为何不走普通死亡逻辑？ |
| 09 玩家受伤与 HUD 查询 | [PNG](Arena-项目总框架与函数调用链-10-09-玩家受伤与-HUD-查询.png) | 谁写血量/分数，谁读取 HUD 数据？ |
| 10 交互、跳板与伤害区 | [PNG](Arena-项目总框架与函数调用链-11-10-交互、跳板与伤害区.png) | 输入委托、距离变化和碰撞事件各自如何触发？ |
| 11 工具组件与标记 Actor | [PNG](Arena-项目总框架与函数调用链-12-11-工具组件与标记-Actor.png) | 组件变换和 Owner 变换有什么区别？标记类是否有业务逻辑？ |
| 12 蓝图边界与索引 | [PNG](Arena-项目总框架与函数调用链-13-12-蓝图边界与索引.png) | 哪些结论需要打开蓝图和地图才能确认？ |

## 两条最先读通的完整链

### 输入到射线伤害

`ABoomerShooterCharacter::SetupPlayerInputComponent` 绑定 ShootAction。Started 由 Enhanced Input 回调到 `FireInputStart`，它调用 `AWeapon::SetFirePressed(true)`。之后 UE 每帧调度 `AWeapon::Tick`，该函数在开火状态和间隔满足时调用 `Fire`。`Fire` 的射线分支使用 `LineTraceSingleByObjectType`；命中敌人时调用 `AEnemyCharacter::DamageEnemy`。松开输入由 Canceled/Completed 回调 `FireInputCancelled`，再调用 `SetFirePressed(false)`。

### 敌人死亡到下一波

`AEnemyCharacter::DamageEnemy` 扣血并查询 `IsDead`；死亡后按顺序调用玩家 `KilledEnemy`、管理器 `UpdateEnemies`、自身 `Destroy`。管理器之后的 `Tick` 检查列表，没有存活敌人则增加 `CurrentWave` 并调用 `SpawnWave`。`SpawnWave` 遍历数据并调用 `SpawnAIFromClass`；全部波次结束且 `GoToDemoOnComplete` 为真才调用 `OpenLevel`。

## 当前实现必须区分的细节

- 玩家 BeginPlay 调用 `UpdateWeapon(1)`，表示第二个数组槽；不能仅凭文件名断言默认枪种。
- 角色输入键位来自 IMC/Action 蓝图资产；图纸没有虚构 WASD、鼠标键等具体映射。
- 投射物 OnComponentHit / OnComponentOverlap 为空；Rocket 命中伤害在 Tick。Bounce 反射处理被注释，Standard 无专用 Sweep/伤害分支。
- 投射物速度当前传入 `Forward * Speed * DeltaTime`；这是源码现状，没有在本轮修正。
- `AWeapon::IsShooting()` 实际返回 `CurrentShootTime > 0`，不能解释成当前是否按住开火。
- `UExplodeWithinRange::TickComponent` 直接 `GetOwner()->Destroy()`，不调用 DamageEnemy / KilledEnemy / UpdateEnemies。管理器后续轮询或查询清理会处理无效角色。
- `AEnemyAIController::Tick` 在距离大于 MinFollowDistance 时 MoveToLocation(自身位置)；在距离不大于阈值时，Chaser/Suicide 才向玩家脚下寻路。不能以类名替代这个分支条件。
- AI 感知绑定被注释；C++ 的 SpawnAIFromClass 参数 BehaviorTree 为 nullptr，但蓝图是否增加其他逻辑仍未知。
- LookAtTarget 的 TransformSelf=true 实际修改组件世界旋转，false 实际修改 Owner 旋转；图按代码表现标注。
- 玩家只有搜索到恰好一个 AEnemySpawnerManager 才缓存引用。实际地图数量和初始化有效性待运行验证。
- 本轮没有补齐未知蓝图调用，也没有做全项目缺陷修复。

## 完整 C++ 函数索引

共 95 个实现/内联函数，覆盖 16 个项目 C++ 类。构造函数、空覆写和三个内联 getter 均列出。
源码链接使用当前电脑的绝对路径；更换电脑后按相对路径重新定位。行号代表本次快照，代码改变后应重新生成。

### ABoomerShooterGameMode

构造时查找玩家蓝图并设 DefaultPawnClass；实际启用需蓝图核验。读 02。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `ABoomerShooterGameMode::ABoomerShooterGameMode()` | [BoomerShooterGameMode.cpp:8](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/BoomerShooterGameMode.cpp:8>) | 02 |

### ABoomerShooterCharacter

玩家输入、相机、武器引用、血量、连击和分数。先读 02–05，再读 09、10。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `ABoomerShooterCharacter::ABoomerShooterCharacter()` | [Character/BoomerShooterCharacter.cpp:34](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:34>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::BeginPlay()` | [Character/BoomerShooterCharacter.cpp:54](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:54>) | 02、04 |
| `ABoomerShooterCharacter::PostInitializeComponents()` | [Character/BoomerShooterCharacter.cpp:88](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:88>) | 02 |
| `ABoomerShooterCharacter::SetupPlayerInputComponent()` | [Character/BoomerShooterCharacter.cpp:105](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:105>) | 02、03 |
| `ABoomerShooterCharacter::Move()` | [Character/BoomerShooterCharacter.cpp:138](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:138>) | 03 |
| `ABoomerShooterCharacter::Look()` | [Character/BoomerShooterCharacter.cpp:149](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:149>) | 03 |
| `ABoomerShooterCharacter::JumpInput()` | [Character/BoomerShooterCharacter.cpp:160](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:160>) | 03 |
| `ABoomerShooterCharacter::Dash()` | [Character/BoomerShooterCharacter.cpp:173](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:173>) | 03 |
| `ABoomerShooterCharacter::Interaction()` | [Character/BoomerShooterCharacter.cpp:196](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:196>) | 10 |
| `ABoomerShooterCharacter::ChangeWeapon1()` | [Character/BoomerShooterCharacter.cpp:204](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:204>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::ChangeWeapon2()` | [Character/BoomerShooterCharacter.cpp:209](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:209>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::ChangeWeapon3()` | [Character/BoomerShooterCharacter.cpp:214](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:214>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::ChangeWeapon4()` | [Character/BoomerShooterCharacter.cpp:219](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:219>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::FireInputStart()` | [Character/BoomerShooterCharacter.cpp:224](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:224>) | 05 |
| `ABoomerShooterCharacter::FireInputCancelled()` | [Character/BoomerShooterCharacter.cpp:232](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:232>) | 05 |
| `ABoomerShooterCharacter::UpdateWeapon()` | [Character/BoomerShooterCharacter.cpp:240](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:240>) | 04 |
| `ABoomerShooterCharacter::IsWeaponShooting()` | [Character/BoomerShooterCharacter.cpp:267](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:267>) | 09 |
| `ABoomerShooterCharacter::Landed()` | [Character/BoomerShooterCharacter.cpp:277](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:277>) | 03 |
| `ABoomerShooterCharacter::Tick()` | [Character/BoomerShooterCharacter.cpp:287](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:287>) | 03、09 |
| `ABoomerShooterCharacter::KilledEnemy()` | [Character/BoomerShooterCharacter.cpp:329](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:329>) | 07、09 |
| `ABoomerShooterCharacter::TookDamage()` | [Character/BoomerShooterCharacter.cpp:335](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:335>) | 06、09 |
| `ABoomerShooterCharacter::GetDamageDirectionUI()` | [Character/BoomerShooterCharacter.cpp:379](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:379>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetHealth01()` | [Character/BoomerShooterCharacter.cpp:384](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:384>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetLevelTimer()` | [Character/BoomerShooterCharacter.cpp:389](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:389>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetComboText()` | [Character/BoomerShooterCharacter.cpp:394](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:394>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetScoreText()` | [Character/BoomerShooterCharacter.cpp:400](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:400>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetCurrentWeapon()` | [Character/BoomerShooterCharacter.cpp:405](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.cpp:405>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetFirstPersonCameraComponent()`（内联） | [Character/BoomerShooterCharacter.h:132](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.h:132>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetSpawnManager()`（内联） | [Character/BoomerShooterCharacter.h:133](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.h:133>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ABoomerShooterCharacter::GetMoveComponent()`（内联） | [Character/BoomerShooterCharacter.h:134](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Character/BoomerShooterCharacter.h:134>) | 页内说明 / 本索引（构造函数或辅助/空函数） |

### AWeapon

武器挂载、开火状态、射击间隔、射线/投射物与准星查询。读 04、05、09。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AWeapon::AWeapon()` | [Weapon/Weapon.cpp:36](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:36>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AWeapon::BeginPlay()` | [Weapon/Weapon.cpp:57](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:57>) | 04 |
| `AWeapon::SetFirePressed()` | [Weapon/Weapon.cpp:63](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:63>) | 05 |
| `AWeapon::IsShooting()` | [Weapon/Weapon.cpp:68](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:68>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AWeapon::IsFirePressed()` | [Weapon/Weapon.cpp:73](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:73>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AWeapon::GetCrosshairAngle()` | [Weapon/Weapon.cpp:78](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:78>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AWeapon::GetCrosshairScale()` | [Weapon/Weapon.cpp:83](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:83>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AWeapon::Fire()` | [Weapon/Weapon.cpp:88](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:88>) | 05 |
| `AWeapon::SpawnWeapon()` | [Weapon/Weapon.cpp:236](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:236>) | 04 |
| `AWeapon::Tick()` | [Weapon/Weapon.cpp:264](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Weapon.cpp:264>) | 04、05 |

### AProjectile

物理速度和每帧 Sweep；伤害仅 Rocket 的命中分支。读 06。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AProjectile::AProjectile()` | [Weapon/Projectile.cpp:33](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Projectile.cpp:33>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AProjectile::BeginPlay()` | [Weapon/Projectile.cpp:41](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Projectile.cpp:41>) | 06 |
| `AProjectile::OnComponentHit()` | [Weapon/Projectile.cpp:56](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Projectile.cpp:56>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AProjectile::OnComponentOverlap()` | [Weapon/Projectile.cpp:61](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Projectile.cpp:61>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AProjectile::Tick()` | [Weapon/Projectile.cpp:66](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Weapon/Projectile.cpp:66>) | 06 |

### AEnemyCharacter

生命值、死亡查询、计分和管理器清理。读 07。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AEnemyCharacter::AEnemyCharacter()` | [Enemies/EnemyCharacter.cpp:33](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:33>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyCharacter::BeginPlay()` | [Enemies/EnemyCharacter.cpp:38](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:38>) | 07 |
| `AEnemyCharacter::SetupPlayerInputComponent()` | [Enemies/EnemyCharacter.cpp:48](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:48>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyCharacter::PossessedBy()` | [Enemies/EnemyCharacter.cpp:53](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:53>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyCharacter::Tick()` | [Enemies/EnemyCharacter.cpp:58](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:58>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyCharacter::IsDead()` | [Enemies/EnemyCharacter.cpp:63](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:63>) | 07 |
| `AEnemyCharacter::DamageEnemy()` | [Enemies/EnemyCharacter.cpp:68](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyCharacter.cpp:68>) | 05、06、07 |

### AEnemySpawnerManager

波次数据、SpawnAIFromClass、存活检查和换关。读 07。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AEnemySpawnerManager::AEnemySpawnerManager()` | [Enemies/EnemySpawnerManager.cpp:29](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:29>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemySpawnerManager::BeginPlay()` | [Enemies/EnemySpawnerManager.cpp:34](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:34>) | 02、07 |
| `AEnemySpawnerManager::Tick()` | [Enemies/EnemySpawnerManager.cpp:42](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:42>) | 07 |
| `AEnemySpawnerManager::SpawnWave()` | [Enemies/EnemySpawnerManager.cpp:70](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:70>) | 02、07 |
| `AEnemySpawnerManager::UpdateEnemies()` | [Enemies/EnemySpawnerManager.cpp:110](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:110>) | 07 |
| `AEnemySpawnerManager::GetWaveCharacters()` | [Enemies/EnemySpawnerManager.cpp:122](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnerManager.cpp:122>) | 07 |

### AEnemyAIController

按 EnemyType 导航；感知绑定被注释。读 08。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AEnemyAIController::AEnemyAIController()` | [Enemies/EnemyAIController.cpp:32](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyAIController.cpp:32>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyAIController::BeginPlay()` | [Enemies/EnemyAIController.cpp:37](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyAIController.cpp:37>) | 08 |
| `AEnemyAIController::OnPreceptionUpdated()` | [Enemies/EnemyAIController.cpp:50](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyAIController.cpp:50>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyAIController::OnTargetPerceptionUpdated()` | [Enemies/EnemyAIController.cpp:55](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyAIController.cpp:55>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyAIController::Tick()` | [Enemies/EnemyAIController.cpp:60](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyAIController.cpp:60>) | 08 |

### UShootProjectile

定时生成投射物；敌我标记来自投射物资产。读 08。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `UShootProjectile::UShootProjectile()` | [Enemies/ShootProjectile.cpp:31](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ShootProjectile.cpp:31>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `UShootProjectile::BeginPlay()` | [Enemies/ShootProjectile.cpp:36](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ShootProjectile.cpp:36>) | 08 |
| `UShootProjectile::TickComponent()` | [Enemies/ShootProjectile.cpp:42](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ShootProjectile.cpp:42>) | 08 |

### UExplodeWithinRange

靠近玩家生成爆炸、造成伤害并直接销毁 Owner。读 08。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `UExplodeWithinRange::UExplodeWithinRange()` | [Enemies/ExplodeWithinRange.cpp:30](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ExplodeWithinRange.cpp:30>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `UExplodeWithinRange::BeginPlay()` | [Enemies/ExplodeWithinRange.cpp:35](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ExplodeWithinRange.cpp:35>) | 08 |
| `UExplodeWithinRange::TickComponent()` | [Enemies/ExplodeWithinRange.cpp:42](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/ExplodeWithinRange.cpp:42>) | 08 |

### ADamageZone

Overlap 更新区内状态，Tick 定时造成伤害。读 10。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `ADamageZone::ADamageZone()` | [Enemies/DamageZone.cpp:29](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/DamageZone.cpp:29>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ADamageZone::BeginPlay()` | [Enemies/DamageZone.cpp:39](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/DamageZone.cpp:39>) | 10 |
| `ADamageZone::Tick()` | [Enemies/DamageZone.cpp:51](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/DamageZone.cpp:51>) | 10 |
| `ADamageZone::OnComponentBeginOverlap()` | [Enemies/DamageZone.cpp:68](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/DamageZone.cpp:68>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ADamageZone::OnComponentEndOverlap()` | [Enemies/DamageZone.cpp:81](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/DamageZone.cpp:81>) | 页内说明 / 本索引（构造函数或辅助/空函数） |

### AJumpPad

Mesh Overlap 调用玩家 LaunchCharacter。读 10。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AJumpPad::AJumpPad()` | [Level/JumpPad.cpp:30](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/JumpPad.cpp:30>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AJumpPad::BeginPlay()` | [Level/JumpPad.cpp:35](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/JumpPad.cpp:35>) | 10 |
| `AJumpPad::Tick()` | [Level/JumpPad.cpp:54](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/JumpPad.cpp:54>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AJumpPad::OnComponentOverlap()` | [Level/JumpPad.cpp:59](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/JumpPad.cpp:59>) | 10 |

### UInteractionPoint

距离状态与玩家输入委托；事件输出给蓝图。读 10。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `UInteractionPoint::UInteractionPoint()` | [Level/InteractionPoint.cpp:28](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/InteractionPoint.cpp:28>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `UInteractionPoint::BeginPlay()` | [Level/InteractionPoint.cpp:34](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/InteractionPoint.cpp:34>) | 10 |
| `UInteractionPoint::OnPlayerInteractionPressed()` | [Level/InteractionPoint.cpp:42](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/InteractionPoint.cpp:42>) | 10 |
| `UInteractionPoint::TickComponent()` | [Level/InteractionPoint.cpp:56](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Level/InteractionPoint.cpp:56>) | 10 |

### ULookAtTarget

计算目标方向并修改组件或 Owner 的旋转。读 11。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `ULookAtTarget::ULookAtTarget()` | [Utilities/LookAtTarget.cpp:29](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/LookAtTarget.cpp:29>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `ULookAtTarget::BeginPlay()` | [Utilities/LookAtTarget.cpp:34](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/LookAtTarget.cpp:34>) | 11 |
| `ULookAtTarget::TickComponent()` | [Utilities/LookAtTarget.cpp:40](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/LookAtTarget.cpp:40>) | 11 |

### UMoveBetweenPoints

在相对坐标点之间移动、停留和循环。读 11。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `UMoveBetweenPoints::UMoveBetweenPoints()` | [Utilities/MoveBetweenPoints.cpp:26](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/MoveBetweenPoints.cpp:26>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `UMoveBetweenPoints::BeginPlay()` | [Utilities/MoveBetweenPoints.cpp:31](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/MoveBetweenPoints.cpp:31>) | 11 |
| `UMoveBetweenPoints::TickComponent()` | [Utilities/MoveBetweenPoints.cpp:36](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Utilities/MoveBetweenPoints.cpp:36>) | 11 |

### AEnemySpawnPoint

供波次数据引用的位置 Actor；生命周期无额外业务逻辑。读 11。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AEnemySpawnPoint::AEnemySpawnPoint()` | [Enemies/EnemySpawnPoint.cpp:26](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnPoint.cpp:26>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemySpawnPoint::BeginPlay()` | [Enemies/EnemySpawnPoint.cpp:31](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnPoint.cpp:31>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemySpawnPoint::Tick()` | [Enemies/EnemySpawnPoint.cpp:36](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemySpawnPoint.cpp:36>) | 页内说明 / 本索引（构造函数或辅助/空函数） |

### AEnemyNavigationPoint

标记 Actor；不能由名称推断一定是 LineMover 端点。读 11。

| 函数 | 定义位置 | 函数节点所在页 |
| --- | --- | --- |
| `AEnemyNavigationPoint::AEnemyNavigationPoint()` | [Enemies/EnemyNavigationPoint.cpp:26](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyNavigationPoint.cpp:26>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyNavigationPoint::BeginPlay()` | [Enemies/EnemyNavigationPoint.cpp:31](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyNavigationPoint.cpp:31>) | 页内说明 / 本索引（构造函数或辅助/空函数） |
| `AEnemyNavigationPoint::Tick()` | [Enemies/EnemyNavigationPoint.cpp:36](<D:/code/Unreal Projects/Arena/UE5-Arena-Shooter-Sample/BoomerShooter/Source/BoomerShooter/Enemies/EnemyNavigationPoint.cpp:36>) | 页内说明 / 本索引（构造函数或辅助/空函数） |

## 不属于运行时 C++ 函数链的入口

- `BoomerShooter.cpp` 的 `IMPLEMENT_PRIMARY_GAME_MODULE` 是模块注册宏；没有项目自写 main()。
- Build.cs、两份 Target.cs 是 UnrealBuildTool 的 C# 构建入口，决定模块/构建目标。
- `.uproject` 和 DefaultEngine.ini 是工程与运行配置；不能用来证明蓝图父类、组件树、真实按键或地图 override。
- FWaveData / FSpawnCharacterData 是波次数据结构；EEnemyType / EProjectileType / ELookAxis 是分支枚举。

## 学习验收清单

- [ ] 不看图，解释玩家输入 → 状态 → Tick → Fire → 伤害 → 计分 → 下一波。
- [ ] 对照 Weapons 数组，解释 ChangeWeapon1 与索引 0，以及默认 UpdateWeapon(1)。
- [ ] 对照投射物蓝图，确认 ProjectileType / EnemyBullet 与 C++ 路径一致。
- [ ] 查看 Enemy 蓝图，确认 AIControllerClass、EnemyType、攻击组件和朝向组件。
- [ ] 在战斗地图中确认单个 SpawnerManager 和 SpawnWaves/SpawnPoint/LineMover 引用。
- [ ] 查看 HUD 与机关接线，再运行 PIE 用断点验证实际调度顺序。

## 更新方法

生成脚本：`D:\code\Unreal Projects\Arena\UE5-Arena-Shooter-Sample\docs\ai\tools\build_learning_diagrams.py`；只生成图纸和索引，不修改 C++。
脚本会核对每个函数节点的真实定义行号、页面边界与节点重叠；调用边语义仍需根据源码人工复核。修改函数逻辑后先更新图纸描述，再生成和导出。

验证范围：已依据源码梳理；最终逐页导出/视觉检查结果记录在仓库 `docs/ai/tasks/002-architecture-learning-diagrams.md`。

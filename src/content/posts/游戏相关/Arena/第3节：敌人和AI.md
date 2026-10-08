---
title: 第三节：敌人和AI
date: 2026-10-05
tags: []
draft: false
---

这一节在角色移动、武器和实体子弹的基础上，记录敌人血量、AI 追击与攻击、玩家受伤，以及最新的多出生点波次管理。第 4 节继续说明血量、波次和剩余敌人数量怎样显示到 HUD。

笔记对应当前 UE 5.4 项目的最新 C++ 实现。目标是能说清楚“谁创建谁、谁控制谁、谁调用谁”，并能顺着完整执行链找到对应代码。

## 1. 解题思路：先把“身体、决策、出生点”分开

| 对象 | 继承自 | 当前职责 |
| --- | --- | --- |
| `AMyEnemyCharacter` | `ACharacter` | 敌人的身体、胶囊、移动组件、血量与死亡处理 |
| `AMyEnemyAIController` | `AAIController` | 保存玩家目标，判断距离，决定追击或攻击 |
| `AMyEnemySpawner` | `AActor` | 按波次配置分配出生点，维护本波敌人引用，清空后进入下一波并通知 HUD |
| `AMyArenaShooterCharacter` | `ACharacter` | 玩家已有的移动与武器功能，加上玩家血量和受伤函数 |
| `AMyProjectile` | `AActor` | 命中时把伤害传给敌人；详见第二节 |

```text
Spawner：本波生成多少敌人、使用哪些出生点、何时进入下一波？
    ↓ SpawnActor
EnemyCharacter：实际存在于世界里的敌人身体
    ↑ Possess：建立控制关系
EnemyAIController：每帧决定追击还是攻击
    │
    ├─ 距离远 → MoveToActor(PlayerCharacter)
    └─ 距离近、冷却结束 → PlayerCharacter->ReceiveEnemyDamage()

玩家开火 → Projectile 命中 Enemy
    → Enemy->TakeProjectileDamage() → 扣血 → 必要时销毁
```

### 1.1 这次实际完成的范围

当前 AI 是直接写在 Controller Tick 中的距离判断。它通过玩家索引 0 获取目标，没有行为树、黑板或 AI 感知；攻击是范围内直接调用扣血函数，没有攻击动画、武器挥砍碰撞或视线检测。

最新 Spawner 在 BeginPlay 启动第一波，在 Tick 中清理失效敌人引用；本波全部清空后立即开始下一波，直到所有配置波次完成。玩家受伤后先将血量限制在有效范围并刷新 HUD，归零仍只打印 `Player Dead!`；没有停止输入、死亡界面或重生。当前波次没有等待倒计时、逐只定时生成或敌人死亡委托。

### 1.2 用两个交互方向理解本节

```text
玩家打敌人：输入 → 武器 → 子弹 → Enemy 扣血 → Enemy 死亡

敌人打玩家：AI Tick → 距离与冷却判断 → Player 扣血并 Clamp → HUD 刷新 → 必要时打印死亡日志
```

两边的受伤函数都是自己写的普通成员函数，不是 UE 因为两个角色靠近或发生碰撞就自动扣血。

### 1.3 从需求推导代码：不要先背 API

先把玩法写成几条规则，再决定数据和函数放在哪个对象：

| 玩法规则 | 需要保存的数据 | 负责对象与入口 |
| --- | --- | --- |
| 敌人有自己的血量，被打到会扣血 | MaxHealth、CurrentHealth | EnemyCharacter::TakeProjectileDamage |
| 远处追玩家，近处攻击 | 玩家目标、攻击范围、剩余冷却 | EnemyAIController::Tick |
| 每波生成指定数量 | Waves、CurrentWaveIndex | Spawner::SpawnWave |
| 清光本波才进入下一波 | 本波敌人引用数组、bWaveActive | Spawner::CleanupDeadEnemies 与 Tick |
| 数值变化后显示到 HUD | 玩家 HUD 实例引用 | Character 的转发函数、HUD 的显示函数 |

据此先实现一个静止敌人的受伤，再实现一个 AI 的追击与攻击，最后让 Spawner 生成并记录多个敌人。复杂度逐步增加，角色移动、伤害、波次和 UI 各自有明确入口。

本节有三种不同更新节奏：

```text
单次初始化：BeginPlay → 初始化血量、查找目标、启动第一波
逐帧检查：AI Tick → 判断追击/攻击；Spawner Tick → 检查本波是否结束
直接调用：命中后调用受伤函数；数据变化后调用 HUD 更新函数
```

不是所有逻辑都写进 Tick，也不是所有变化都来自委托回调。当前敌人死亡被 Spawner 发现，是逐帧检查 IsValid；并没有自动触发 Spawner 的 OnEnemyDeath。

### 1.4 先记住两层循环

```text
一只敌人的循环：追击 → 攻击/等待冷却 → 被打到 → 血量归零 → 销毁

整局波次的循环：生成本波 → 记录实例 → 清理失效引用
              → 本波数组清空 → 索引前进 → 生成下一波/结束
```

第一层由每只敌人的 Controller 和 Character 分工完成；第二层由 Spawner 管理。不能把“一个敌人死亡”直接等同于“整波结束”，还要确认本波其他敌人也都失效。

下面标注为伪代码的段落用于解释思路，不是复制到 .cpp 就能编译的代码；完整 C++ 函数片段对应当前实现。

## 2. 创建 EnemyCharacter：先让敌人能受伤

### 2.1 为什么继承 ACharacter

敌人要在地面移动，继承 ACharacter 可以使用已有的 CapsuleComponent、Mesh 和 CharacterMovement。它不需要玩家的摄像机、IA_Move 或鼠标输入映射。

EnemyCharacter 与玩家 Character 都属于角色，但由不同 Controller 控制：玩家通常由 PlayerController 控制，敌人由 AIController 控制。

### 2.2 血量成员与受伤函数

下面是 `MyEnemyCharacter.h` 中本节重点成员，放在已有类内：

```cpp
private:
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth = 100.0f;

    float CurrentHealth = 0.0f;

public:
    void TakeProjectileDamage(float Damage);

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
```

`MaxHealth` 是配置值，可在敌人蓝图类默认值中修改；`CurrentHealth` 是运行时状态，每个敌人实例都有自己的值。这里整理了声明格式，源码中的 UPROPERTY 宏后多余分号不作为推荐写法。

受伤函数放在 public，因为子弹需要从敌人类外部调用它。当前没有标记为 BlueprintCallable，所以它是 C++ 调用入口，并非默认可从蓝图任意调用的节点。

### 2.3 BeginPlay：应用血量配置

```cpp
void AMyEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Enemy Spawned, Health = %f"),
        CurrentHealth
    );

}
```

```text
蓝图可以先覆盖 MaxHealth
↓
敌人进入游戏，BeginPlay 执行
↓
CurrentHealth = MaxHealth
↓
打印敌人当前血量
```

不在构造函数里一次性把运行血量固定成 100，是为了在进入游戏时应用该敌人实际使用的配置。CurrentHealth 初始声明为 0，并不意味着敌人开局已经死亡；BeginPlay 会把它初始化为最大血量。

### 2.4 TakeProjectileDamage：血量减法与死亡

```cpp
void AMyEnemyCharacter::TakeProjectileDamage(float Damage)
{
    CurrentHealth -= Damage;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Enemy Took Damage: %f, Current Health: %f"),
        Damage,
        CurrentHealth
    );

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Enemy Dead!")
        );

        Destroy();
    }
}
```

读取顺序是：扣血 → 打印新血量 → 判断归零 → 必要时销毁。`CurrentHealth -= Damage` 等价于 `CurrentHealth = CurrentHealth - Damage`。

以默认 100 血、每次 25 点伤害为例，成功命中四次后归零。`Destroy()` 在这个函数里销毁的是敌人；第二节命中回调里另一个 Destroy 销毁的是子弹。

当前函数没有把血量限制在 0～MaxHealth，也没有拒绝负伤害。足够大的伤害可能使血量变负，负伤害则会加血。这里记录现有计算方式，校验伤害、限制血量和统一死亡处理可以后续补充。

## 3. 创建 AIController，并让它控制敌人

### 3.1 为什么把决策放在 Controller

EnemyCharacter 表示可被控制的身体，AIController 表示控制这个身体的对象。不是在敌人身上“创建一个 AI 组件”，也不是把 AIController 当作另一具敌人模型。

```text
AIController：决定要去哪里、何时攻击
↓ 控制关系
EnemyCharacter：通过 CharacterMovement 实际移动
```

Controller 自己的空间位置不能替代敌人位置；计算敌人到玩家的距离时，要先获取它控制的 Pawn。

### 3.2 Build.cs 中增加 AIModule

当前 `MyArenaShooter.Build.cs` 的公开依赖为：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput", "AIModule", "UMG"
});
```

#### 首次出现：模块依赖

`#include "AIController.h"` 让 C++ 编译器看到类型声明；Build.cs 中的 AIModule 告诉 UE 构建系统，这个项目模块依赖 AI 模块。二者职责不同，缺少模块依赖可能导致头文件或链接问题。

当前敌人 AI 继承 AAIController，因此依赖 AIModule；UMG 是新增 HUD 使用的模块，第 4 节详细说明。

### 3.3 AIController 的成员

在 `MyEnemyAIController.h` 中包含：

```cpp
#include "CoreMinimal.h"
#include "AIController.h"
#include "MyEnemyAIController.generated.h"

class AMyArenaShooterCharacter;
```

类继承 AAIController，重点成员如下：

```cpp
private:
    UPROPERTY()
    AMyArenaShooterCharacter* PlayerCharacter = nullptr;

    float AttackRange = 150.0f;
    float MoveAcceptanceRadius = 50.0f;
    float AttackDamage = 20.0f;
    float AttackCooldown = 1.0f;
    float CurrentAttackCooldown = 0.0f;
```

| 成员 | 当前默认值 | 含义 |
| --- | --- | --- |
| PlayerCharacter | nullptr | 保存要追击和攻击的玩家实例 |
| AttackRange | 150 cm | 距离不大于这个值时进入攻击分支 |
| MoveAcceptanceRadius | 50 cm | 传给导航请求的到达容差 |
| AttackDamage | 20 | 每次攻击调用传入的伤害 |
| AttackCooldown | 1 秒 | 攻击后设置的冷却时长 |
| CurrentAttackCooldown | 0 秒 | 当前剩余冷却时间 |

这些 AI 数值目前是普通 C++ 成员，没有 EditDefaultsOnly 或 EditAnywhere，不能照着血量的做法假定它们已经可以在蓝图细节中编辑。PlayerCharacter 的 UPROPERTY 让 UE 认识这个引用，但也没有编辑器可编辑标记。

### 3.4 在敌人构造函数里指定 Controller

`MyEnemyCharacter.cpp` 需要包含 `MyEnemyAIController.h`。

```cpp
AMyEnemyCharacter::AMyEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;


    AIControllerClass = AMyEnemyAIController::StaticClass();

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
```

#### 首次出现：AIControllerClass 与 StaticClass

```cpp
AIControllerClass = AMyEnemyAIController::StaticClass();
```

`AIControllerClass` 是 Pawn 提供的控制器类配置。`StaticClass()` 返回当前 C++ 类对应的 UClass 描述，表示“应该生成哪种控制器”。这里没有保存一个已经存在的 Controller 实例，也没有立即手动调用 Possess。

#### 首次出现：AutoPossessAI

```cpp
AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
```

这是自动 AI 控制的时机配置。PlacedInWorldOrSpawned 同时覆盖手动放进关卡的敌人和运行时 SpawnActor 出来的敌人；不是自动生成更多敌人的开关。

实际使用还需要正确的 Controller 类、蓝图默认值、游戏世界与控制条件。敌人蓝图若保存过旧配置，要在类默认值中核对 AI Controller Class 和 Auto Possess AI。

### 3.5 PossessedBy：确认是谁控制了敌人

```cpp
void AMyEnemyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Enemy Possessed By: %s"),
        *GetNameSafe(NewController)
    );
}
```

#### 首次出现：PossessedBy

这是 Pawn/Character 被 Controller 控制时的生命周期回调，`NewController` 是本次建立控制关系的控制器。调用 `Super::PossessedBy()` 保留父类处理，再打印控制器名称辅助排查。

```text
自动控制配置生效
↓
生成或使用对应 AIController
↓
Controller Possess EnemyCharacter
↓
EnemyCharacter::PossessedBy(NewController)
↓
打印 Enemy Possessed By: MyEnemyAIController_...
```

PossessedBy 写在被控制的 EnemyCharacter 上；它与写在 Controller 上的 OnPossess 是两侧不同回调。当前覆盖的是 PossessedBy。网络游戏中这类控制逻辑还要考虑权限，本节按当前单人流程理解。

构造函数、Possess、不同 Actor 的 BeginPlay 并不能统一记成一条固定全局时序。重点是进入正常决策前，AI 必须有受控敌人和可用玩家目标。

## 4. AI 开始运行：找到玩家目标

### 4.1 开启 AIController 的 Tick

```cpp
AMyEnemyAIController::AMyEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}
```

EnemyCharacter 的 Actor Tick 关闭，AIController 的 Tick 开启，分别作用于两个对象。敌人的 CharacterMovement 是组件，它的更新也不是由 EnemyCharacter 自己手动 Tick 位置完成。

### 4.2 BeginPlay 中获取玩家

当前实现文件包含 `Kismet/GameplayStatics.h` 和 `MyArenaShooterCharacter.h`。

```cpp
void AMyEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMyArenaShooterCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
    );

}
```

#### 首次出现：UGameplayStatics::GetPlayerCharacter

```cpp
UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
```

GetWorld() 提供查询的世界上下文；0 是玩家控制器列表中的第一个索引，不是 Enemy 编号，也不是蓝图资产编号。本项目按单人玩家目标理解。

函数返回该玩家控制器当前控制的 Character。没有可用角色、控制的是非 Character Pawn 时，结果可能为空；它不是从内容浏览器加载 BP_Character 资产。[官方 API 说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameplayStatics/GetPlayerCharacter)

返回类型是 ACharacter*，再用 Cast 转为自己的 AMyArenaShooterCharacter*，才能调用自定义的 ReceiveEnemyDamage()。

```text
查询玩家索引 0 的 Character
↓
Cast<AMyArenaShooterCharacter>
↓ 成功
保存实例引用到 PlayerCharacter
```

当前只在 BeginPlay 查找一次。若当时玩家尚未生成或 Cast 失败，之后 Tick 的 IsValid 会一直返回失败，代码没有自动重新查找目标。玩家重生后换了实例，也需要重新获取；这是现有实现的边界，不把重试当作已经实现的功能。

## 5. AI 每帧决策：追击或攻击

### 5.1 先读完整 Tick，再拆开理解

```cpp
void AMyEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsValid(PlayerCharacter))
    {
        return;
    }

    CurrentAttackCooldown = FMath::Max(CurrentAttackCooldown - DeltaTime, 0.0f);

    APawn* ControlledPawn = GetPawn();

    if (!IsValid(ControlledPawn))
    {
        return;
    }

    float DistanceToPlayer = FVector::Distance(
        ControlledPawn->GetActorLocation(),
        PlayerCharacter->GetActorLocation()
    );

    if (DistanceToPlayer > AttackRange)
    {
        MoveToActor(
            PlayerCharacter,
            MoveAcceptanceRadius
        );
    }
    else
    {
        StopMovement();

        if (CurrentAttackCooldown <= 0.0f)
        {
            PlayerCharacter->ReceiveEnemyDamage(AttackDamage);

            CurrentAttackCooldown = AttackCooldown;
        }
    }

}
```

这个函数做了五件事：检查目标 → 减少冷却 → 取得受控 Pawn → 计算距离 → 选择追击或攻击。

先用伪代码看当前决策顺序：

```text
每次 AI Tick(经过时间)：
    如果玩家目标失效：结束本次 Tick

    剩余攻击冷却 = max(剩余攻击冷却 - 经过时间, 0)
    敌人身体 = GetPawn()
    如果敌人身体失效：结束本次 Tick

    距离 = 敌人身体与玩家的三维直线距离

    如果距离 > 攻击范围：
        请求向玩家导航移动
    否则：
        停止当前导航移动
        如果剩余攻击冷却为 0：
            调用玩家受伤函数
            剩余攻击冷却 = 1 秒
```

先用 IsValid 拦住不可用对象，之后才读取位置、调用函数。冷却放在距离分支前面，表示追击期间也在倒数；攻击之后才重置冷却，表示“已经攻击一次，接下来等多久”。这些代码位置决定了玩法，并非随意排列。

return 只结束当前这次函数调用；下一帧引擎仍可能调用 Tick。当前没有保存专门的追击/攻击状态枚举，分支选择本身形成最小决策流程。



### 5.2 首次重点使用：DeltaTime 与 FMath::Max

```cpp
CurrentAttackCooldown = FMath::Max(
    CurrentAttackCooldown - DeltaTime,
    0.0f
);
```

DeltaTime 是本次更新经过的游戏时间，单位为秒。每帧扣掉实际经过时间，使冷却依据时间推进；不是“每次 Tick 都减 1”。在大约 60 FPS 时，多数帧约减 0.0167 秒。

FMath::Max 返回两个值中较大的那个，让剩余冷却最低为 0。它没有在这里限制血量，也不是定时器或后台等待。

```text
攻击后剩余 1 秒
↓ 每帧减 DeltaTime
0.8 → 0.5 → 0.2 → 0
↓
还在攻击范围内时，可以进行下一次攻击
```

这条时间线是说明例子，不是固定每 0.1 秒回调一次。攻击检查发生在后续帧，因此实际间隔约 1 秒，不保证精确到时钟毫秒。

目标有效时，即使敌人在远处追击，冷却也会减少。冷却不会因离开攻击范围而重新设置为 1 秒。

### 5.3 首次出现：GetPawn

```cpp
APawn* ControlledPawn = GetPawn();
```

这里在 AIController 中执行，取得的是这个控制器正在控制的敌人，不是玩家。PlayerCharacter 则来自玩家索引查询，两者不能混淆。

GetPawn() 可能为空，所以先 IsValid 检查，再访问位置。Controller 的位置并不代表敌人身体的位置。

### 5.4 首次出现：FVector::Distance

```cpp
float DistanceToPlayer = FVector::Distance(
    ControlledPawn->GetActorLocation(),
    PlayerCharacter->GetActorLocation()
);
```

它返回两个 Actor 世界位置之间的三维直线距离，默认单位为厘米。它不是导航路径长度，也不是两个胶囊表面之间的空隙；角色的 Z 高度差也会参与计算。

当前判断条件：

```text
DistanceToPlayer > 150 cm → 追击
DistanceToPlayer <= 150 cm → 停止导航并检查攻击冷却
```

等于 AttackRange 时属于 else 的攻击分支。

### 5.5 首次出现：MoveToActor

```cpp
MoveToActor(PlayerCharacter, MoveAcceptanceRadius);
```

发出导航移动请求，目标是玩家 Actor。AI 路径跟随和敌人的移动组件协作，让敌人沿可行走路径接近目标；这不是 SetActorLocation 瞬移，也不是调用后当前函数一直等到敌人到达才继续。

当前只传两个参数。UE 5.4 默认启用路径寻找和 bStopOnOverlap；到达判定会考虑自身半径等设置，所以 50 cm 不宜理解为“精确停在玩家中心外 50 cm”。它与 AttackRange 的 150 cm 是两套判断：一个用于导航到达，一个用于当前 Tick 的攻击分支。[MoveToActor 官方说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/AIModule/AAIController/MoveToActor)

该函数有返回值：Failed、AlreadyAtGoal、RequestSuccessful，可用来调试请求是否被接受；RequestSuccessful 不代表已经走到目标。当前代码没有保存它，也没有监听最终移动完成结果。

当前远距离时每帧重新调用 MoveToActor。UE 5.4 的这个入口会中断已有路径跟随，再提交请求；先作为最小追击实现理解，后续可减少重复请求。目标为 Actor 的移动请求本身会跟踪目标位置，不需要因为“玩家会动”就认定一定要逐帧重启请求。

### 5.6 首次出现：StopMovement 与攻击调用

```cpp
StopMovement();

if (CurrentAttackCooldown <= 0.0f)
{
    PlayerCharacter->ReceiveEnemyDamage(AttackDamage);
    CurrentAttackCooldown = AttackCooldown;
}
```

StopMovement() 停止控制器当前的导航移动请求。它不等于关闭整个 Actor Tick，也不等于关闭 CharacterMovement 或锁住玩家。

随后检查冷却。首次进入范围时默认剩余冷却为 0，所以可以立即攻击一次；攻击后设置成 1 秒，下一帧不会马上再扣血。

ReceiveEnemyDamage() 是我们自己写在玩家类中的 public 函数。当前直接调用它，不经过攻击动画、碰撞框、Overlap 或 ApplyDamage。

因此“距离足够近”目前就是攻击条件，代码没有检查视线、墙体、面朝方向或玩家是否已经死亡。隔着薄墙但满足距离条件时，仍可能调用扣血。

## 6. 玩家接收敌人伤害

### 6.1 玩家新增的成员

在已有的 MyArenaShooterCharacter 类内增加：

```cpp
private:
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth = 100.0f;

    float CurrentHealth = 0.0f;

public:
    void ReceiveEnemyDamage(float Damage);
```

在已有 BeginPlay 的 `Super::BeginPlay()` 后初始化：

```cpp
CurrentHealth = MaxHealth;
```

不是另写第二个同名 BeginPlay；第一节的输入映射与第二节的武器生成仍在同一个函数中。第二节的当前函数片段也已同步这行初始化。

### 6.2 当前受伤实现

```cpp
void AMyArenaShooterCharacter::ReceiveEnemyDamage(float Damage)
{
    CurrentHealth -= Damage;

    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    if (HUDWidget)
    {
        HUDWidget->UpdateHealth(CurrentHealth, MaxHealth);
    }

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Player Dead!")
        );
    }
}
```

以 100 血、每次攻击 20 点为例：

```text
100 → 80 → 60 → 40 → 20 → 0
                          ↓
                    Player Dead! 日志
```

最新受伤函数增加了 FMath::Clamp(CurrentHealth, 0.0f, MaxHealth)，将血量限制在 0～MaxHealth，然后调用 HUDWidget->UpdateHealth。正常最大血量配置下，后续攻击不会再把玩家血量扣成负数。归零后仍没有 Destroy、禁用输入、停止 AI 攻击或重生，仍可能再次打印死亡日志。

“对象是否有效”和“角色是否活着”是两个不同条件。HUD 血条归零不代表玩家 Actor 被销毁；完整死亡状态和只执行一次的死亡处理仍需后续补充。UI 更新流程在第 4 节展开。

FMath::Clamp 是这里首次重点使用的 API：把数值限制在指定上下界。它修改玩家实际保存的血量，不只是把血条画成 0；血量条显示比例则由 HUD 再计算。

### 6.3 Enemy 与 Player 两个受伤函数对比

| 对象 | 入口 | 当前血量处理 | 当前归零处理 |
| --- | --- | --- | --- |
| Enemy | TakeProjectileDamage | 减去传入伤害并记录日志 | Destroy 当前敌人 |
| Player | ReceiveEnemyDamage | 扣血、Clamp 到范围、刷新 HUD | 只打印 Player Dead，仍可继续被攻击 |

它们命名不同、死亡处理不同，但都是当前项目自己写的直接扣血方法。不会因为名字包含 Damage 就自动接入 UE 通用伤害系统。

## 7. 波次管理：为什么要让 Spawner 保存本波敌人

### 7.1 从“生成一只”扩展到“清光一波再生成下一波”

旧版本只有 EnemyClass 和一只 SpawnedEnemy 的引用，BeginPlay 生成一次就结束。现在的需求是：

```text
第一波生成 2 只 → 全部消灭 → 第二波生成 3 只
              → 全部消灭 → 第三波生成 4 只 → 全部完成
```

2、3、4 是说明用的波次配置例子；当前运行日志也出现过这组数量，C++ 没有把它硬编码成固定三波。

因此要保存三类信息：玩法配置 Waves、场景位置 SpawnPoints，以及实际生成的本波敌人 CurrentWaveEnemies。不能只存“想生成多少只”，因为出生可能失败；也不能只数地图上的所有 Enemy，因为手动摆放或其他 Spawner 生成的敌人可能不属于本波。

### 7.2 波次配置：FEnemyWaveData

在 MyEnemySpawner.h 中，结构体放在 Spawner 类定义前：

```cpp
USTRUCT(BlueprintType)
struct FEnemyWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 EnemyCount = 1;
};
```

#### 首次出现：USTRUCT、BlueprintType 和配置数据

USTRUCT 让 UE 反射认识这个结构体，BlueprintType 让这个结构体类型可用于蓝图。EnemyCount 是这一整波计划生成的数量，不是“每一个出生点各生成多少只”。

Waves 的每个元素描述一波，例如 `[{EnemyCount=2}, {EnemyCount=3}, {EnemyCount=4}]`。它保存规则，不保存敌人 Actor；以后增加波次配置字段，也不需要把配置和运行时引用混在一起。

EditAnywhere 允许编辑值，BlueprintReadOnly 表示蓝图脚本只读这项反射属性；它不等于禁止在编辑器中配置。

### 7.3 运行状态：哪些数据需要跨帧保留

当前类中与波次有关的成员：

```cpp
private:
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AMyEnemyCharacter> EnemyClass;

    UPROPERTY(EditInstanceOnly, Category = "Spawn")
    TArray<AActor*> SpawnPoints;

    UPROPERTY(EditAnywhere, Category = "Wave")
    TArray<FEnemyWaveData> Waves;

    UPROPERTY()
    TArray<AMyEnemyCharacter*> CurrentWaveEnemies;

    int32 CurrentWaveIndex = 0;
    bool bWaveActive = false;

    UPROPERTY()
    AMyArenaShooterCharacter* PlayerCharacter = nullptr;
```

| 数据 | 保存的含义 | 为什么放在成员里 |
| --- | --- | --- |
| EnemyClass | 生成哪种敌人类 | 所有波次生成时都要读取 |
| SpawnPoints | 地图中哪些 Actor 提供出生变换 | 每波循环分配位置 |
| Waves | 各波计划数量 | 按当前索引读取配置 |
| CurrentWaveEnemies | 本波生成成功的敌人实例引用 | 跨帧检查它们是否仍有效 |
| CurrentWaveIndex | 当前配置索引，从 0 开始 | 结束本波后需要进入下一项 |
| bWaveActive | 当前是否处于等待本波敌人清空的阶段 | 避免未启动或完成后每帧继续推进 |
| PlayerCharacter | 当前用于转发 HUD 更新的玩家 | 生成与清理时通知同一个玩家 |

SpawnPoints 使用 EditInstanceOnly，配置的是地图上某个 Spawner 实例引用的关卡 Actor，不能只在蓝图类默认值里找它。Waves 是 EditAnywhere，可以作为默认配置或实例配置。

#### 首次出现：TArray 与三个不同的“数量”

```text
Waves.Num()              → 一共配置了几波
SpawnPoints.Num()        → 有几个可轮流使用的出生点
CurrentWaveEnemies.Num() → 数组中目前记录了几个敌人引用
```

最后一项在清理前可能还含刚被销毁的引用，所以不能永远把 Num 直接当成已经即时同步的存活数量。CleanupDeadEnemies 负责消除这个差别。

### 7.4 初始化：只启动第一波，后续由 Tick 接力

```cpp
AMyEnemySpawner::AMyEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = true;

}
```

```cpp
void AMyEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMyArenaShooterCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
    );

    SpawnWave();

}
```

现在开启了 Spawner Tick，因为要持续检查本波敌人；BeginPlay 查找玩家并调用 SpawnWave 启动第一波。以后每波结束，是 Tick 再调用 SpawnWave，不是 BeginPlay 重复执行。

PlayerCharacter 是用于 HUD 通知的引用。Spawner 与每只 AIController 各自在 BeginPlay 查询玩家一次，两个引用承担不同职责；Spawner 查找失败仍可能生成敌人，只是不会完成该路径的 HUD 更新。

### 7.5 SpawnWave：生成一波的完整实现

MyEnemySpawner.cpp 包含 MyEnemySpawner.h、MyEnemyCharacter.h、MyArenaShooterCharacter.h 和 Kismet/GameplayStatics.h。

```cpp
void AMyEnemySpawner::SpawnWave()
{
    if (!EnemyClass)
    {
        return;
    }

    if (SpawnPoints.Num() == 0)
    {
        return;
    }

    if (CurrentWaveIndex >= Waves.Num())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("All Waves Completed!")
        );

        return;
    }

    CurrentWaveEnemies.Empty();

    int32 EnemyCount = Waves[CurrentWaveIndex].EnemyCount;

    for (int32 i = 0; i < EnemyCount; ++i)
    {
        AActor* SpawnPoint = SpawnPoints[i % SpawnPoints.Num()];

        if (!IsValid(SpawnPoint))
        {
            continue;
        }

        FVector SpawnLocation = SpawnPoint->GetActorLocation()
            + FVector(0.0f, 0.0f, 100.0f);
        FRotator SpawnRotation = SpawnPoint->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AMyEnemyCharacter* Enemy = GetWorld()->SpawnActor<AMyEnemyCharacter>(
            EnemyClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (IsValid(Enemy))
        {
            CurrentWaveEnemies.Add(Enemy);
        }

        bWaveActive = CurrentWaveEnemies.Num() > 0;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Wave %d Started, Enemy Count = %d"),
            CurrentWaveIndex + 1,
            CurrentWaveEnemies.Num()
        );
    }

    if (IsValid(PlayerCharacter))
    {
        PlayerCharacter->UpadteWaveHUD(
            CurrentWaveIndex + 1,
            Waves.Num(),
            CurrentWaveEnemies.Num()
        );
    }
}
```

先将长函数分成六段，理解每段为什么放在这里：

| 段落 | 作用 | 先做它的原因 |
| --- | --- | --- |
| 检查 EnemyClass、出生点数量、波次索引 | 确认可生成 | 避免无配置生成、取模除零和数组越界 |
| 清空本波引用数组，读取 EnemyCount | 准备新波 | 不能把上一波记录混到本波数量里 |
| 循环分配出生点 | 决定每只敌人的位置 | 数量可能超过出生点数量 |
| SpawnActor 并记录有效引用 | 建立本波名单 | 实际成功数量可能少于计划数量 |
| 设置活动状态、记录日志 | 后续 Tick 可以监控 | 当前代码这两句放在 for 内 |
| 循环结束后通知 HUD | 显示本波初始数量 | 使用成功记录的数量，而非直接显示计划值 |

按当前代码顺序写成伪代码：

```text
生成当前波：
    如果敌人类未配置：返回
    如果出生点数组为空：返回
    如果当前索引 >= 波次数量：
        输出“全部波次完成”
        返回

    清空本波敌人引用数组
    本波计划数量 = Waves[当前索引].EnemyCount

    对 i 从 0 到 本波计划数量-1：
        出生点 = SpawnPoints[i % 出生点数量]
        如果出生点无效：跳过这一次生成，继续下一次循环

        从出生点世界位置 + 世界 Z 的 100 cm 生成敌人
        如果生成成功：将这个敌人引用加入本波数组

        本波活动 = 本波数组数量 > 0
        输出一次当前波次及此刻成功记录的数量

    如果玩家引用有效：
        通知玩家刷新 HUD（当前索引+1，总波数，实际记录数量）
```

这里的 return 退出整个 SpawnWave；continue 只跳过当前这只敌人的尝试。当前不会因一个出生点无效而自动换另一个点补足，也不会重试生成失败的敌人。

Empty() 清的是引用数组，不会调用每只 Enemy 的 Destroy。正常设计只在开局或本波清空后生成下一波，因此没有用 Empty 来“杀掉上一波敌人”。

### 7.6 难点一：为什么用 i % SpawnPoints.Num()

有 3 个出生点 A、B、C，计划生成 5 只：

| i | i % 3 | 使用的出生点 |
| --- | --- | --- |
| 0 | 0 | A |
| 1 | 1 | B |
| 2 | 2 | C |
| 3 | 0 | A |
| 4 | 1 | B |

取模把递增索引循环映射到已有出生点范围，超过数量后从头轮流使用。它不是随机选点，不是每个点都生成 5 只，也不是多出来的敌人就不生成。

所以前面必须先检查 SpawnPoints.Num() != 0，否则取模没有合法的除数。重复使用同一个点可能让同一波的敌人出生位置相近，当前用出生碰撞策略尝试调整。

### 7.7 出生变换与碰撞策略仍沿用前面的基础

SpawnPoint->GetActorLocation() 读取被选中标记 Actor 的世界位置，再加世界 Z 的 100 cm；旋转也取该 SpawnPoint。Spawner 自己放在哪里，不再直接决定所有敌人的出生位置。

`AdjustIfPossibleButAlwaysSpawn` 会尝试调整阻挡出生位置，调整失败仍允许生成，不保证一定不卡住。它也不会自动把出生点投影到 NavMesh；位置、胶囊和导航覆盖需要在地图中确认。

### 7.8 难点二：敌人 Destroy 后，数组为什么还要清理

```text
子弹命中 → Enemy 扣血到 0 → Enemy::Destroy()
↓
世界中的这个 Actor 开始销毁，IsValid 会失败
↓
Spawner 的数组槽位不会因此自动消失
↓
Spawner 之后的 Tick 用 CleanupDeadEnemies 移除这个槽位
↓
数组 Num 才反映清理后的剩余数量
```

UPROPERTY 让 UE 认识这些引用，不会把 Destroy 取消，也不会自动维护“存活敌人名单”。IsValid 检查的是对象是否仍可用，而不是读取血量；当前敌人归零会 Destroy，才能通过这条链从波次名单中清理。

不是固定必须“下一帧”才发现：取决于销毁与 Spawner Tick 的顺序，可在同帧后续检查或之后的帧发现。敌人并没有直接调用 Spawner 的清理函数。

### 7.9 CleanupDeadEnemies：为什么倒序遍历

```cpp
void AMyEnemySpawner::CleanupDeadEnemies()
{

    for (int32 i = CurrentWaveEnemies.Num() - 1; i >= 0; --i)
    {
        int32 PreviousEnemyCount = CurrentWaveEnemies.Num();

        if (!IsValid(CurrentWaveEnemies[i]))
        {
            CurrentWaveEnemies.RemoveAt(i);
        }

        if (CurrentWaveEnemies.Num() != PreviousEnemyCount)
        {
            if (IsValid(PlayerCharacter))
            {
                PlayerCharacter->UpadteWaveHUD(
                    CurrentWaveIndex + 1,
                    Waves.Num(),
                    CurrentWaveEnemies.Num()
                );
            }
        }
    }
}
```

```text
清理本波名单：
    从最后一个索引向 0 遍历：
        记录本次删除前的数组数量
        如果这个敌人引用已无效：
            RemoveAt(当前索引)

        如果数组数量发生变化，且玩家有效：
            通知玩家刷新 HUD 的剩余敌人数量
```

RemoveAt 删除槽位后，后面的元素会向前补位。假设 [失效A, 失效B, 存活C]：

```text
从前往后直接删：删除索引0的A → B移到0
              → 循环索引变成1，检查C → B可能被跳过

从后往前删：先检查C → 删除B → 删除A
          → 不影响尚未检查的较小索引
```

Num()-1 在空数组时为 -1，当前索引用 int32，`i >= 0` 不成立，循环自然不执行。

PreviousEnemyCount 在当前源码中放在循环内，因此每次成功删除都会通知一次 HUD。同一轮清理删除多只时会有多次更新，不是整个 for 结束后只更新一次。用前后数量比较，是为了只在这次迭代确实改变数组时通知 UI。

### 7.10 难点三：Tick 怎样只推进一次下一波

```cpp
void AMyEnemySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bWaveActive)
    {
        return;
    }

    CleanupDeadEnemies();

    if (CurrentWaveEnemies.Num() == 0)
    {
        bWaveActive = false;
        ++CurrentWaveIndex;
        SpawnWave();
    }
}
```

```text
每次 Spawner Tick：
    如果本波未活动：返回

    清理已销毁的敌人引用

    如果本波数组已经为空：
        标记本波不再活动
        当前波次索引 + 1
        调用 SpawnWave
            有下一波 → 记录新敌人，本波重新活动
            没有下一波 → 输出完成，保持不活动
```

bWaveActive 是这一段的执行门槛。最后一波结束后保持 false，后续 Tick 提前返回，避免每帧重复增加索引、输出完成。

当前索引从 0 开始，访问配置用 0、1、2；给玩家显示用 CurrentWaveIndex+1，即第 1、2、3 波。要先做越界检查，才能读取 Waves[CurrentWaveIndex]。

### 7.11 用两波、三个敌人走一遍

```text
配置：Waves=[2只, 1只]，出生点=[A,B]

开局：索引0 → 生成 E1、E2 → 数组[E1,E2] → HUD显示1/2、剩余2
击杀E1：E1失效 → 清理后[E2] → HUD显示剩余1 → 不换波
击杀E2：清理后[] → HUD更新剩余0
      → Tick把索引改成1 → SpawnWave生成E3
      → 数组[E3] → HUD显示2/2、剩余1
击杀E3：清理后[] → HUD更新剩余0
      → 索引变成2 → 2 >= Waves.Num()
      → 输出All Waves Completed，后续不继续监控
```

本波剩余 0 与新波生成可能发生在同一次 Spawner Tick 中，画面不一定能看到中间的 0。最后全部完成时当前代码只打印日志，HUD 保持最后一波编号和剩余 0，没有另外显示胜利页面。

### 7.12 现有实现需要知道的边界

- EnemyCount 为 0、所有出生点无效或所有生成失败，可能没有任何敌人让 bWaveActive 变成 true；之后 Tick 会提前返回，不会自动跳过空波。
- 日志放在生成 for 内，成功记录数量会逐次变化，如同一波输出 1、2、3；不表示重开了三次同一波。
- 当前没有波间等待，新一波在检测到清空的这次 Tick 中立即启动；也没有敌人死亡委托或波次事件广播。
- CurrentWaveEnemies 只记录本 Spawner 生成成功的实例，不包括地图上另外手动放置的敌人。
- 玩家引用和 HUD 尚未准备好时，本次通知可能不执行；当前没有缓存后重放。第 4 节说明这条初始化边界。

## 8. 在编辑器里把代码连起来


### 8.1 当前资源关系

```text
Content/BluePrints/
├─ BP_MyArenaShooterCharacter
├─ Weapons/
│  ├─ BP_MyWeapon
│  └─ BP_MyProjectile
└─ Enemies/
   ├─ BP_MyEnemyCharacter
   └─ BP_MyEnemySpawner
```

当前敌人直接使用 C++ 的 AMyEnemyAIController 类，不要求必须创建 BP_MyEnemyAIController。

| 位置 | 核对内容 |
| --- | --- |
| BP_MyEnemyCharacter 类默认值 | AI Controller Class 对应 MyEnemyAIController；Auto Possess AI 为 Placed in World or Spawned；MaxHealth 按需配置 |
| BP_MyEnemyCharacter Capsule | 保持有效碰撞，对子弹对象通道形成阻挡 |
| 关卡中的 BP_MyEnemySpawner 实例 | EnemyClass = BP_MyEnemyCharacter；SpawnPoints 引用关卡出生点 Actor；Waves 配置每波 EnemyCount |
| BP_MyArenaShooterCharacter 类默认值 | 玩家最大血量、HUDWidgetClass = WBP_HUD，以及已有输入、武器配置 |
| BP_MyProjectile 类默认值 | Damage 按需配置；默认 C++ 数值 25 |
| 地图 | 放置 Spawner，并提供覆盖可行走区域的导航数据 |

### 8.2 首次出现：NavMeshBoundsVolume 与导航网格

MoveToActor 默认走导航路径，所以有敌人和 AIController 还不够。需要地图中存在可用导航区域：

1. 在“放置 Actor”中找到 NavMeshBoundsVolume，放到地图。
2. 调整范围覆盖敌人出生点、玩家活动区域及连接两者的地面。
3. 在关卡视口按 P 查看导航网格的绿色可行走区域；大地图先在当前测试区域验证。
4. 确认敌人的实际出生位置位于可用区域附近，而不是墙内或高台外。
5. 保存地图后 Play，观察 AIController 控制日志与追击行为。

导航网格由场景碰撞几何等信息生成，用于路径寻找；绿色显示不代表任何角色一定能跨过所有障碍，也不代表一张覆盖整个世界的 Bounds 会立即产生全地图可用数据。[UE 5.4 导航系统说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/navigation-system-in-unreal-engine?application_version=5.4)

### 8.3 分阶段验证

先手动放置一个 BP_MyEnemyCharacter，确认受伤与 AI，再用 Spawner 验证波次。至少配置一个有效出生点和一波正数量，确认清光后推进；随后测试多个出生点和多波。手动敌人不在本 Spawner 的 CurrentWaveEnemies 中，不参与它的剩余数量和换波判定。

Enemy 没设置可视模型时仍可有胶囊碰撞，但不利于观察追击。显示模型与胶囊、AI、导航是不同配置项，要分别确认。

## 9. 用具体操作走完整条执行链

### 9.1 敌人离玩家很远：追击链

```text
AIController 本帧 Tick(DeltaTime)
↓
PlayerCharacter 有效？失败则本帧 return
↓
减少 CurrentAttackCooldown，最低为 0
↓
GetPawn 获取受控 Enemy，有效？失败则 return
↓
读取 Enemy 和 Player 世界位置，FVector::Distance
↓
距离 > 150 cm
↓
MoveToActor(PlayerCharacter, 50)
↓
导航请求与路径跟随驱动 Enemy 的 CharacterMovement
↓
Enemy 向目标移动
↓
下一帧重新进行距离判断
```

### 9.2 敌人进入范围：攻击与玩家受伤链

```text
AI Tick 检查目标、冷却、受控 Enemy
↓
DistanceToPlayer <= 150 cm
↓
StopMovement：停止当前导航移动
↓
CurrentAttackCooldown <= 0？
├─ 否：这一帧不攻击，等待后续 Tick 推进冷却
└─ 是：PlayerCharacter->ReceiveEnemyDamage(20)
       ↓
       玩家 CurrentHealth -= 20，再 Clamp 到 0～MaxHealth
       ↓
       HUDWidget->UpdateHealth，刷新血条和文字
       ↓
       血量 <= 0 时输出 Player Dead，目前不销毁
       ↓ 函数返回
       AI 将 CurrentAttackCooldown 设置为 1 秒
↓
后续帧继续减少冷却，满足范围和时间条件才再攻击
```

### 9.3 玩家反击：子弹到敌人死亡链

```text
左键 → IA_Fire Started → Character::FireInput
↓
CurrentWeapon->Fire → 从 MuzzleLocation 生成 Projectile
↓
ProjectileMovement 扫掠移动 CollisionComponent
↓
子弹命中 Enemy 的阻挡组件
↓
OnComponentHit → OnProjectileHit
↓
Cast<AMyEnemyCharacter>(OtherActor) 成功
↓
TakeProjectileDamage(25)
↓
敌人血量减少并记录日志
├─ > 0：继续存活、仍可被 AI 控制
└─ <= 0：打印 Enemy Dead → Destroy 敌人
↓
回到子弹命中回调 → Destroy 当前子弹
```

敌人销毁会结束该 Pawn 的活动与控制关系。Spawner 后续通过 IsValid 清理本波名单，更新剩余数量；只有全部清空才换波，不是死一只就立刻补一只。

### 9.4 波次到 UI 的跨对象执行链

```text
最后一个本波 Enemy：TakeProjectileDamage → Destroy
↓
Spawner Tick → CleanupDeadEnemies → IsValid失败 → RemoveAt
↓
Spawner：PlayerCharacter->UpadteWaveHUD(当前波次, 总波数, 剩余数)
↓
Character：HUDWidget->UpdateWaveInfo(...)
↓
Widget：格式化文字 → WaveText / EnemyCountText 的 SetText
↓ 回到 Spawner Tick
CurrentWaveEnemies.Num()==0 → bWaveActive=false → ++CurrentWaveIndex
↓
SpawnWave → 成功生成下一波 → 再次通知 HUD 显示新波次
```

名字 `UpadteWaveHUD` 是当前源码的实际拼写，声明、实现、调用一致。笔记保留这个名字便于搜索；若以后改为 UpdateWaveHUD，三处要一起改。

### 9.5 从当前日志理解循环位置

当前运行日志出现过：

```text
Wave 1 Started, Enemy Count = 1
Wave 1 Started, Enemy Count = 2
...
Wave 2 Started, Enemy Count = 1
Wave 2 Started, Enemy Count = 2
Wave 2 Started, Enemy Count = 3
...
Wave 3 Started, Enemy Count = 1
Wave 3 Started, Enemy Count = 2
Wave 3 Started, Enemy Count = 3
Wave 3 Started, Enemy Count = 4
...
All Waves Completed!
```

这里按阅读用途截取关键内容，省略了受伤等交错日志。同一波的数量从 1 递增，是因为生成循环里每添加成功一只后就打印一次；开始新波的实际入口仍是 SpawnWave。

Enemy BeginPlay 的 `Enemy Spawned, Health = ...` 与 Spawner 的 Wave 日志来自不同对象。玩家最新受伤函数已没有旧版 Player Damage 数值日志，而是更新 HUD 并在归零时打印 Player Dead。

## 10. 实际编译与运行排查


### 10.1 本次编译错误：前向声明不等于完整定义

MyEnemySpawner.h 中可以写：

```cpp
class AMyEnemyCharacter;
```

它让编译器知道存在这个类，便于声明指针等成员。但在 .cpp 中实际检查 TSubclassOf、实例化 `SpawnActor<AMyEnemyCharacter>()` 等模板时，需要敌人完整定义。最新已用 SpawnWave 替代旧 SpawnEnemy，但这条 include 原则仍然适用。

本次缺少 include 时出现了：

```text
C2027：使用了未定义类型 AMyEnemyCharacter
C2672：StaticClass 找不到匹配的重载函数
```

当前实现已在 MyEnemySpawner.cpp 中补上：

```cpp
#include "MyEnemySpawner.h"
#include "MyEnemyCharacter.h"
```

错误虽然可能指向引擎的 SubclassOf.h，也不代表应该修改引擎文件。沿报错的模板实例化链回到项目文件，确认依赖类型是否完整。

### 10.2 完整编译与 Live Coding

本次还遇到过：

```text
Unable to build while Live Coding is active
```

这表示完整构建被活跃的编辑器 Live Coding 会话拦住，与缺少头文件是不同阶段的问题。保存蓝图和地图、关闭编辑器后，在 IDE 中构建 Development Editor / Win64，再重开编辑器。

仅在编辑器中迭代合适的代码改动时，可按 Ctrl+Alt+F11 触发 Live Coding。新增类、反射属性或修改类布局后，如果资产显示仍沿用旧状态，完整编译和重新打开编辑器更便于确认。普通枪口组件位置调整则编译、保存蓝图即可应用，不需要因此改 C++。

### 10.3 按执行链定位，不同时修改所有设置

| 现象 | 优先确认 |
| --- | --- |
| Play 没有生成敌人 | 地图是否有 Spawner；EnemyClass、SpawnPoints、Waves、EnemyCount 是否有效；生成是否成功 |
| 有敌人但没有 Controller 日志 | AIControllerClass、AutoPossessAI、蓝图旧覆盖值，以及实际使用的敌人类 |
| 有 Controller 但完全不追击 | PlayerCharacter 是否查找成功，GetPawn 是否有效，距离分支、NavMesh、MoveToActor 返回值 |
| 一直不找玩家 | BeginPlay 仅查询一次，是否因为初始化时机或玩家类型导致保存了 nullptr |
| 敌人在出生点卡住 | 胶囊与地面/墙体重叠，出生高度、碰撞调整策略、实际出生位置 |
| 追到附近却不攻击 | 当前距离是否 <= AttackRange，导航到达范围与攻击范围是否冲突 |
| 玩家每帧都快速扣血 | 攻击后是否赋值 CurrentAttackCooldown，是否有多个敌人同时攻击，实际代码是否与当前版本一致 |
| 玩家死亡后还能移动、仍被攻击 | 当前死亡分支只有日志，尚未加入死亡状态和停止攻击逻辑 |
| 子弹碰到敌人但没有伤害日志 | 是否真正命中该敌人 Actor、Cast 是否成功、Damage 是否配置、是否运行新代码 |
| 清光后没有下一波 | 是否还有配置波次，本波失效引用是否被移除，bWaveActive 是否曾成功开启，下一波是否有成功生成的敌人 |
| 手动摆放的敌人没计入数量 | 当前数组只记录本 Spawner 生成的敌人 |
| 波次显示与敌人数量不更新 | PlayerCharacter / HUDWidget 是否准备好，UpadteWaveHUD 与 UpdateWaveInfo 是否执行 |
| 同一波打印多条 Started | 当前日志在生成 for 内，观察索引是否真正增加，而非只看消息名称 |
| 修改数值后细节里找不到 | 当前 AI 参数不是可编辑 UPROPERTY；确认修改的是 C++ 普通成员还是蓝图默认属性 |

## 11. 第一次出现的重要 API 速查

| API / 类型 | 本节如何理解 | 位置 |
| --- | --- | --- |
| AAIController | 控制敌人 Pawn 的 AI 决策对象 | 第 3 节 |
| AIModule | 项目使用 AI 类型所依赖的 UE 模块 | 第 3 节 |
| AIControllerClass | 默认采用哪一种 AI 控制器类 | 第 3 节 |
| StaticClass() | 获取类描述，不是生成对象实例 | 第 3 节 |
| AutoPossessAI | 自动建立 AI 控制关系的时机 | 第 3 节 |
| PossessedBy() | Pawn 被控制时执行的回调 | 第 3 节 |
| GetPlayerCharacter(World, 0) | 查询玩家索引 0 当前控制的 Character | 第 4 节 |
| DeltaTime | 本次更新经过的游戏时间，单位为秒 | 第 5 节 |
| FMath::Max() | 取较大值，此处限制冷却不低于 0 | 第 5 节 |
| GetPawn() | 查询当前 Controller 正在控制的 Pawn | 第 5 节 |
| FVector::Distance() | 两个世界位置之间的三维直线距离 | 第 5 节 |
| MoveToActor() | 向目标 Actor 提交导航移动请求 | 第 5 节 |
| StopMovement() | 停止当前导航移动请求 | 第 5 节 |
| EditAnywhere | 可编辑类默认值，也可编辑关卡实例配置 | 第 7 节 |
| SpawnCollisionHandlingOverride | 设置本次 Actor 生成的出生碰撞策略 | 第 7 节 |
| NavMeshBoundsVolume | 标记需要生成导航网格的区域 | 第 8 节 |
| FMath::Clamp | 将实际玩家血量限制到上下界 | 第 6 节 |
| USTRUCT(BlueprintType) | 把每波配置组织成反射可识别的结构体 | 第 7 节 |
| TArray / Num / Add / Empty / RemoveAt | 管理波次配置、出生点和实际敌人引用 | 第 7 节 |
| EditInstanceOnly | 在关卡实例上配置出生点 Actor 引用 | 第 7 节 |
| i % 出生点数量 | 循环分配出生点索引 | 第 7 节 |

Cast、IsValid、SpawnActor、TSubclassOf、GetWorld、UE_LOG、Destroy 已在前两节出现，本节主要复习它们在敌人和 AI 中的用途。TakeProjectileDamage、ReceiveEnemyDamage、SpawnWave、CleanupDeadEnemies、UpadteWaveHUD 都是自定义函数，不是引擎内置 API。

## 12. 学完后尝试自己复述

1. EnemyCharacter、EnemyAIController、EnemySpawner 分别负责什么？谁是敌人的身体？
2. AIControllerClass 保存类还是实例？AutoPossessAI 会不会生成敌人？
3. GetPlayerCharacter(GetWorld(), 0) 和 AIController 的 GetPawn 分别得到谁？
4. 为什么关闭 EnemyCharacter 的 Actor Tick，敌人仍然可以移动？
5. AttackRange 与 MoveAcceptanceRadius 为什么不是同一个概念？
6. 不设置攻击冷却，会产生什么结果？为什么要减 DeltaTime 而不是每帧减 1？
7. 当前敌人攻击是否依赖动画或碰撞事件？有没有视线检测？
8. 玩家归零和敌人归零，当前分别做了什么？IsValid 为什么不能代替血量判断？
9. EnemyClass、Waves、SpawnPoints、CurrentWaveEnemies 分别保存什么？为什么配置和实例名单不能混用？
10. 出生位置加 100 的方向是什么？AdjustIfPossibleButAlwaysSpawn 能不能保证不重叠？
11. 为什么在头文件前向声明 Enemy，还要在 Spawner.cpp 中 include 它？
12. 一个敌人 Destroy 后，Spawner 怎样发现？为什么 Num 不会自动减少？
13. 为什么倒序 RemoveAt？为什么 CurrentWaveIndex 给玩家显示时加 1？
14. bWaveActive=false 后为什么后续 Tick 不再重复换波？
15. 全部生成失败或数量为 0 的波为什么可能停住？怎样从执行门槛推导这个结果？
16. 波次更新为什么经过 Player 再调用 Widget，而不是 Enemy 自己寻找文字控件？

复习时先复述单个 AI 的判断，再用两波示例解释“生成名单 → 敌人销毁 → 清理名单 → 通知 HUD → 清空后换波”。特别说明什么时候是引擎回调、什么时候是自己直接调用、什么时候只是保存状态等下一帧检查。

### 12.1 后续扩展方向

| 当前最小实现 | 下一步可以补充 |
| --- | --- |
| BeginPlay 查询一次玩家 | 初始化失败重试、玩家重生后更新目标 |
| 每帧重新请求追击 | 按状态或合理频率更新导航请求，处理返回值与完成结果 |
| 距离内直接调用玩家受伤 | 攻击动画、命中判定、视线检测 |
| 受伤更新HUD，玩家死亡仍只打印日志 | 死亡状态、停止输入和攻击、死亡页面、重生 |
| 按配置多点生成，清光后立即换波 | 波间等待、逐只定时生成、生成失败与空波处理、完成页面 |
| 两个独立受伤函数 | 统一伤害接口、来源归属和血量显示 |

这些是可继续学习的内容，当前源码尚未实现。


## 13. 后续学习记录：AI攻击请求接到新版受伤反馈（2026-10-08）

前面Enemy、AIController、Spawner的逐步过程继续保留。这次这些类的源码没有改变，新增行为接在玩家ReceiveEnemyDamage和Projectile命中回调中；敌人仍按距离与冷却提出攻击请求，玩家现在先判断是否接受。

### 13.1 AI发起调用，不等于玩家一定处理伤害

玩家新版入口先判断：

```cpp
if (bIsDead || Damage <= 0.0f)
{
    return;
}
```

接收有效伤害后才扣血、Clamp，刷新血量并闪红；条件满足时播放本机受伤声，最后再检查是否Die。完整新函数与反馈定时器在第4节本次后续学习记录中说明。

```text
AI.Tick：玩家引用有效 → 计算距离
→ 进入AttackRange → StopMovement
→ 冷却结束 → 调用Player.ReceiveEnemyDamage(AttackDamage)
    ├─ 已死亡或伤害不为正：返回，不重复反馈
    └─ 有效伤害：扣血 → HUD血量与闪红 → 声音 → 归零则Die
→ 调用返回后，AI重设自己的攻击冷却
```

因此新版玩家不会因后续AI攻击请求反复闪红、播声或再次进入正常伤害链的Die。AI本身仍没有读取bIsDead停止追击和攻击；玩家Actor也没有因为归零而自动销毁。这次增加的是接收端保护，不是改变AI决策。

### 13.2 玩家反击时新增命中反馈，敌人扣血规则继续沿用

```text
相机选点 → 枪口按计算方向生成Projectile
→ 实际命中Enemy → TakeProjectileDamage(默认25)
→ 敌人扣血，归零仍Destroy
→ 子弹根据自己的Instigator通知发射玩家显示Hit Marker
→ 按实际碰撞位置生成ImpactEffect，销毁子弹
→ Spawner继续清理失效Enemy引用，更新数量并推进波次
```

敌人没有因此新增HUD引用，也没有直接控制玩家的命中标记。射击追踪选点与真实命中的区别在第5节详细记录，UI通知与定时隐藏在第4节记录；AI和波次保持原来清晰的职责。

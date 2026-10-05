---
title: 第三节：敌人和AI
date: 2026-10-05
tags: []
draft: false
---

这一节在角色移动、武器和实体子弹的基础上，增加敌人、敌人血量、AI 追击与攻击、玩家血量，以及开局生成敌人的 Spawner。

笔记对应当前 UE 5.4 项目的最新 C++ 实现。目标是能说清楚“谁创建谁、谁控制谁、谁调用谁”，并能顺着完整执行链找到对应代码。

## 1. 解题思路：先把“身体、决策、出生点”分开

| 对象 | 继承自 | 当前职责 |
| --- | --- | --- |
| `AMyEnemyCharacter` | `ACharacter` | 敌人的身体、胶囊、移动组件、血量与死亡处理 |
| `AMyEnemyAIController` | `AAIController` | 保存玩家目标，判断距离，决定追击或攻击 |
| `AMyEnemySpawner` | `AActor` | 开始游戏时，在自己的位置附近生成一个敌人 |
| `AMyArenaShooterCharacter` | `ACharacter` | 玩家已有的移动与武器功能，加上玩家血量和受伤函数 |
| `AMyProjectile` | `AActor` | 命中时把伤害传给敌人；详见第二节 |

```text
Spawner：在哪里生成、生成哪种敌人？
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

Spawner 在 BeginPlay 里只生成一次；敌人死亡不会触发自动补充。玩家归零只打印 `Player Dead!`，还没有销毁、停止输入、死亡界面或重生。这些边界要与“接下来想做什么”分开记。

### 1.2 用两个交互方向理解本节

```text
玩家打敌人：输入 → 武器 → 子弹 → Enemy 扣血 → Enemy 死亡

敌人打玩家：AI Tick → 距离与冷却判断 → Player 扣血 → 打印死亡日志
```

两边的受伤函数都是自己写的普通成员函数，不是 UE 因为两个角色靠近或发生碰撞就自动扣血。

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
    "EnhancedInput", "AIModule"
});
```

#### 首次出现：模块依赖

`#include "AIController.h"` 让 C++ 编译器看到类型声明；Build.cs 中的 AIModule 告诉 UE 构建系统，这个项目模块依赖 AI 模块。二者职责不同，缺少模块依赖可能导致头文件或链接问题。

当前敌人 AI 继承 AAIController，因此加入 AIModule。本文只说明已有依赖，不为了写笔记额外更改工程模块。

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

    UE_LOG(LogTemp, Warning, TEXT("Player Damage: %.1f, Health: %.1f"), Damage, CurrentHealth);

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player Dead!"));
    }
}
```

以 100 血、每次攻击 20 点为例：

```text
100 → 80 → 60 → 40 → 20 → 0
                          ↓
                    Player Dead! 日志
```

注意：当前归零后没有 Destroy、禁用输入、停止 AI 攻击或重生。玩家仍是有效 Actor，AI 的 IsValid(PlayerCharacter) 不会因为血量为 0 就自动失败；后续攻击仍可能把血量扣成负数，并再次打印死亡日志。

“对象是否有效”和“角色是否活着”是两个不同条件。完整死亡状态、只执行一次的死亡处理和 UI 放到后续实现。

### 6.3 Enemy 与 Player 两个受伤函数对比

| 对象 | 入口 | 当前血量处理 | 当前归零处理 |
| --- | --- | --- | --- |
| Enemy | TakeProjectileDamage | 减去传入伤害并记录日志 | Destroy 当前敌人 |
| Player | ReceiveEnemyDamage | 减去传入伤害并记录日志 | 只打印 Player Dead |

它们命名不同、死亡处理不同，但都是当前项目自己写的直接扣血方法。不会因为名字包含 Damage 就自动接入 UE 通用伤害系统。

## 7. EnemySpawner：把“生成敌人”独立出来

### 7.1 类引用与实例引用

当前 MyEnemySpawner 继承 AActor，重点成员如下：

```cpp
private:
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AMyEnemyCharacter> EnemyClass;

    UPROPERTY()
    AMyEnemyCharacter* SpawnedEnemy = nullptr;

    void SpawnEnemy();
```

| 成员 | 保存什么 | 在哪里设置 |
| --- | --- | --- |
| EnemyClass | 要生成的敌人类，例如 BP_MyEnemyCharacter | 蓝图类默认值或关卡内实例细节 |
| SpawnedEnemy | 这次生成成功后的具体敌人引用 | SpawnActor 返回后赋值 |

这是第二节 DefaultWeaponClass / CurrentWeapon 区分的再次应用。TSubclassOf 限制 EnemyClass 只能选 AMyEnemyCharacter 的派生类，不是从场景里选择已有敌人。

#### 本节重点：EditAnywhere 与 EditDefaultsOnly

EnemyClass 使用 EditAnywhere，因此可以给地图上的不同 Spawner 实例配置不同敌人类。MaxHealth 使用 EditDefaultsOnly，主要编辑蓝图类默认值；二者不是同样的编辑范围。

### 7.2 开始游戏时只生成一次

```cpp
AMyEnemySpawner::AMyEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;

}
```

```cpp
void AMyEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    SpawnEnemy();

}
```

关闭 Actor Tick，但 BeginPlay 仍然调用 SpawnEnemy()。当前没有 Tick 生成、Timer、循环、波次、死亡监听或补怪代码。

一个 Spawner 通常在一次进入游戏的 BeginPlay 中生成一个敌人。放两个 Spawner 才是两个出生来源，并不是当前已经实现了每隔几秒持续生成。

### 7.3 当前 SpawnEnemy 实现

MyEnemySpawner.cpp 顶部需要完整敌人定义：

```cpp
#include "MyEnemySpawner.h"
#include "MyEnemyCharacter.h"
```

```cpp
void AMyEnemySpawner::SpawnEnemy()
{
    if (!EnemyClass)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;

    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

    FRotator SpawnRotation = GetActorRotation();

    SpawnedEnemy = GetWorld()->SpawnActor<AMyEnemyCharacter>(
        EnemyClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );


    if (IsValid(SpawnedEnemy))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Enemy Spawned: %s"),
            *GetNameSafe(SpawnedEnemy)
        );
    }
}
```

顺序：检查类配置 → 准备出生碰撞策略 → 计算出生位置和旋转 → 生成敌人 → 保存引用 → 成功时打印日志。

### 7.4 出生变换：用 Spawner 位置加向上偏移

```cpp
FVector SpawnLocation = GetActorLocation()
    + FVector(0.0f, 0.0f, 100.0f);

FRotator SpawnRotation = GetActorRotation();
```

这里加的是世界 Z 方向的 100 cm，不是 Spawner 自己旋转后的局部向上方向。假设 Spawner 世界位置为 (500, 200, 0)，请求的出生位置就是 (500, 200, 100)。

角色 Actor 位置通常位于胶囊中心，而不是脚底。默认胶囊半高为 88 cm，在水平地面附近加 100 cm 是方便测试的起点，不是对任何地形高度都正确的落地算法。

最终位置还可能被出生碰撞策略调整；这里也没有向下射线查地面、随机选导航点或判断是否在 NavMesh 上。

当前原生 Spawner 构造函数没有创建 SceneComponent 根。笔记里的编辑器操作使用实际已有的 BP_MyEnemySpawner；在蓝图中确认存在用于位置变换的场景根节点，才能把它作为清楚可调的出生位置标记。

### 7.5 首次出现：SpawnCollisionHandlingOverride

```cpp
SpawnParams.SpawnCollisionHandlingOverride =
    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
```

它覆盖这次生成的出生碰撞策略：如果目标位置阻挡，尝试调整到附近可用位置；调整失败时，这个策略仍允许生成，不保证敌人最终不会卡在物体里。

它处理的是出生阶段，不是后来 CharacterMovement 的移动碰撞，也不是关闭敌人碰撞。SpawnActor 仍可能由于其他原因失败，所以代码在返回后使用 IsValid(SpawnedEnemy) 检查再记录成功日志。

本次 Spawner 没有设置 Owner / Instigator，也没有把敌人挂载到 Spawner。SpawnedEnemy 只是引用；Spawner 不会因此自动管理敌人的血量、死亡或重生。

### 7.6 整条出生链

```text
关卡中存在 BP_MyEnemySpawner
↓
Spawner::BeginPlay → SpawnEnemy
↓
检查 EnemyClass 是否配置
↓
准备 SpawnParams 和请求的世界出生变换
↓
SpawnActor<AMyEnemyCharacter>(EnemyClass, Location, Rotation, Params)
↓
生成 Enemy 与自带的角色组件，应用蓝图默认配置
↓
自动 AI 控制配置建立 Controller 与 Enemy 的控制关系
↓
Enemy 的 BeginPlay 初始化血量
↓
SpawnActor 返回敌人指针，保存为 SpawnedEnemy
↓
IsValid 成功，Spawner 打印 Enemy Spawned: 对象名
↓
后续 AI Tick 决策，CharacterMovement 执行移动
```

图是正常运行下的依赖示意。AIController 是另一个 Actor，它的 BeginPlay 获取玩家与敌人的初始化可能交错；不要靠这张图假定所有 Actor BeginPlay 有固定全局先后顺序。

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
| BP_MyEnemySpawner 或关卡实例 | EnemyClass = BP_MyEnemyCharacter；出生位置合适 |
| BP_MyArenaShooterCharacter 类默认值 | 玩家最大血量与已有输入、武器配置 |
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

先手动放置一个 BP_MyEnemyCharacter，确认受伤和 AI 行为；再使用 Spawner 验证运行时出生。若把手动敌人和 Spawner 都保留，Play 后可能有两个敌人，这是两个不同来源，不一定是重复生成错误。

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
       玩家 CurrentHealth -= 20
       ↓
       输出 Player Damage 与当前血量
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

敌人销毁会结束该 Pawn 的活动与控制关系，AI 的 GetPawn / IsValid 检查可避免继续按可用敌人访问。当前 Spawner 不会监听这一事件，因此不会自动生成替补。

### 9.4 用日志核对实际执行

本次项目运行日志中已出现下列行为，下面保留关键内容作为阅读例子：

```text
Enemy Possessed By: MyEnemyAIController_0
Player Damage: 20.0, Health: 80.0
Player Damage: 20.0, Health: 60.0
...
Player Damage: 20.0, Health: 0.0
Player Dead!
Enemy Took Damage: 25.000000, Current Health: 75.000000
Enemy Took Damage: 25.000000, Current Health: 50.000000
Enemy Took Damage: 25.000000, Current Health: 25.000000
Enemy Took Damage: 25.000000, Current Health: 0.000000
Enemy Dead!
```

这段按阅读用途整理，不是原始日志的精确时序，攻击和射击可以交错。默认参数下的关键观察是玩家每次扣 20、敌人每次扣 25。

Spawner 最新代码还会在生成成功后打印 `Enemy Spawned: 对象名`。这与 Enemy BeginPlay 的 `Enemy Spawned, Health = ...` 是两处不同日志，不能仅因为出现两条含 Spawned 的日志就断定生成了两个敌人。

## 10. 实际编译与运行排查

### 10.1 本次编译错误：前向声明不等于完整定义

MyEnemySpawner.h 中可以写：

```cpp
class AMyEnemyCharacter;
```

它让编译器知道存在这个类，便于声明指针等成员。但在 .cpp 中实际检查 TSubclassOf、实例化 SpawnActor<AMyEnemyCharacter> 等模板时，需要敌人完整定义。

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
| Play 没有生成敌人 | 地图里是否有 Spawner，EnemyClass 是否配置，SpawnActor 返回引用是否有效 |
| 有敌人但没有 Controller 日志 | AIControllerClass、AutoPossessAI、蓝图旧覆盖值，以及实际使用的敌人类 |
| 有 Controller 但完全不追击 | PlayerCharacter 是否查找成功，GetPawn 是否有效，距离分支、NavMesh、MoveToActor 返回值 |
| 一直不找玩家 | BeginPlay 仅查询一次，是否因为初始化时机或玩家类型导致保存了 nullptr |
| 敌人在出生点卡住 | 胶囊与地面/墙体重叠，出生高度、碰撞调整策略、实际出生位置 |
| 追到附近却不攻击 | 当前距离是否 <= AttackRange，导航到达范围与攻击范围是否冲突 |
| 玩家每帧都快速扣血 | 攻击后是否赋值 CurrentAttackCooldown，是否有多个敌人同时攻击，实际代码是否与当前版本一致 |
| 玩家死亡后还能移动、仍被攻击 | 当前死亡分支只有日志，尚未加入死亡状态和停止攻击逻辑 |
| 子弹碰到敌人但没有伤害日志 | 是否真正命中该敌人 Actor、Cast 是否成功、Damage 是否配置、是否运行新代码 |
| 敌人死后没有再出现 | 当前 Spawner 仅 BeginPlay 生成一次，没有补怪或波次逻辑 |
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

Cast、IsValid、SpawnActor、TSubclassOf、GetWorld、UE_LOG、Destroy 已在前两节出现，本节主要复习它们在敌人和 AI 中的用途。TakeProjectileDamage、ReceiveEnemyDamage、SpawnEnemy 都是自定义函数，不是引擎内置 API。

## 12. 学完后尝试自己复述

1. EnemyCharacter、EnemyAIController、EnemySpawner 分别负责什么？谁是敌人的身体？
2. AIControllerClass 保存类还是实例？AutoPossessAI 会不会生成敌人？
3. GetPlayerCharacter(GetWorld(), 0) 和 AIController 的 GetPawn 分别得到谁？
4. 为什么关闭 EnemyCharacter 的 Actor Tick，敌人仍然可以移动？
5. AttackRange 与 MoveAcceptanceRadius 为什么不是同一个概念？
6. 不设置攻击冷却，会产生什么结果？为什么要减 DeltaTime 而不是每帧减 1？
7. 当前敌人攻击是否依赖动画或碰撞事件？有没有视线检测？
8. 玩家归零和敌人归零，当前分别做了什么？IsValid 为什么不能代替血量判断？
9. EnemyClass 与 SpawnedEnemy 分别保存什么？为什么不只保留其中一个？
10. 出生位置加 100 的方向是什么？AdjustIfPossibleButAlwaysSpawn 能不能保证不重叠？
11. 为什么在头文件前向声明 Enemy，还要在 Spawner.cpp 中 include 它？
12. 敌人死后为什么不会自动补充？下一步实现波次需要增加哪些职责？

复习时先画“Spawner 生成敌人 → AI 控制身体 → 远处追击 → 近处按冷却攻击 → 玩家扣血”，再画反方向的“玩家射击 → 子弹命中 → Enemy 扣血 → 敌人死亡”。能把两条链对应到具体函数，再进行下一步扩展。

### 12.1 后续扩展方向

| 当前最小实现 | 下一步可以补充 |
| --- | --- |
| BeginPlay 查询一次玩家 | 初始化失败重试、玩家重生后更新目标 |
| 每帧重新请求追击 | 按状态或合理频率更新导航请求，处理返回值与完成结果 |
| 距离内直接调用玩家受伤 | 攻击动画、命中判定、视线检测 |
| 玩家死亡只打印日志 | 死亡状态、停止输入和攻击、UI、重生 |
| Spawner 开始时生成一个 | 定时生成、死亡补充、波次与数量限制 |
| 两个独立受伤函数 | 统一伤害接口、来源归属和血量显示 |

这些是可继续学习的内容，当前源码尚未实现。

---
title: 第四节：UI
date: 2026-10-07
tags: []
draft: false
---

> 学习阶段说明：第 1～11 节完整保留血量与波次 HUD 阶段的逐步记录；第 12 节起追加弹药、死亡提示与重新开始。早期段落中的“当前”“尚未实现”描述当时的实现进度，后续记录说明新增和变化。

本节记录当前已经实现的 HUD：玩家血量条、血量文字、当前波次和剩余敌人数量。核心不是先把控件摆得好看，而是把游戏数据的变化连到正确的显示入口。

代码对应 `UMyHUDWidget`、`AMyArenaShooterCharacter` 和最新的 `AMyEnemySpawner`。复杂执行链用伪代码解释；代码片段分为当前完整函数与从现有函数摘取的部分，不需要重复创建第二个同名 BeginPlay。

## 1. 先从需求推导 UI 的职责

### 1.1 要显示什么，数据原本由谁管理

| 画面内容 | 真正的数据来源 | UI 需要接收什么 |
| --- | --- | --- |
| 血量条 | 玩家 Character 的 CurrentHealth / MaxHealth | 当前血量、最大血量 |
| 血量文字 | 同一份玩家血量 | 当前血量、最大血量 |
| 当前波次 / 总波数 | Spawner 的索引和 Waves 数组 | 显示用波次编号、总波数 |
| 剩余敌人数量 | Spawner 清理后的 CurrentWaveEnemies | 当前记录的剩余数量 |

玩家和 Spawner 维护玩法状态，Widget 负责把传入数值换成比例、文字并设置控件。不要让 HealthText 的显示文字反过来成为真正血量，也不要让 Widget 自己决定什么时候生成下一波。

### 1.2 为什么当前这样分成三层

```text
数据变化
├─ 玩家受伤：Character 扣血
└─ 开波 / 敌人清理：Spawner 改变波次与名单
      ↓
玩家 Character 持有 HUDWidget 引用
      ↓
UMyHUDWidget 的 UpdateHealth / UpdateWaveInfo
      ↓
HealthBar、HealthText、WaveText、EnemyCountText
```

血量变化就在玩家类内部，可以直接调用自己的 HUD；波次变化在 Spawner，因此通过玩家的 public 转发函数送到 HUD。当前是一条明确的函数调用链，没有 UI Tick 逐帧查询血量，也没有血量委托广播。

Spawner 不需要知道文字控件叫什么，Enemy 也不用查找 WBP_HUD。调用者只传业务数值，Widget 内部决定怎么显示。以后改文案、布局，通常只需调整 Widget 这层。

### 1.3 实现顺序

```text
确定要显示的数据
→ 创建 C++ Widget 基类和更新函数
→ 在 Widget Blueprint 中放置、命名控件
→ 角色创建 Widget 实例并放到屏幕
→ 开局发送初始血量
→ 受伤后发送新血量
→ Spawner 开波、清理敌人后发送波次数据
```

“界面已经创建”“界面已经显示”“界面已经收到最新数据”是三个不同步骤，每一步都需要对应代码。

## 2. 创建 C++ Widget：给显示提供固定入口

### 2.1 UMyHUDWidget 继承 UUserWidget

`MyHUDWidget.h` 中的结构整理如下，空的访问区段省略：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class MYARENASHOOTER_API UMyHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WaveText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* EnemyCountText;

    void UpdateHealth(float CurrentHealth, float MaxHealth);
    void UpdateWaveInfo(
        int32 CurrentWave,
        int32 TotalWaves,
        int32 RemainingEnemies
    );
};
```

UUserWidget 是 UMG 的用户界面基类；当前 UI 实例通过 CreateWidget 创建，武器和敌人这些世界 Actor 则通过 SpawnActor 创建。`UMyHUDWidget` 的名字虽然有 HUD，实际仍是 UserWidget，不需要因此额外创建 AHUD 派生类。

### 2.2 首次出现：BindWidget

```cpp
UPROPERTY(meta = (BindWidget))
UProgressBar* HealthBar;
```

含义是让运行时 Widget 的这个成员，连接到继承它的 Widget Blueprint 中同名、兼容类型的控件。

它声明的是引用，不会替你在设计器自动画一个血条。需要先在蓝图里创建 ProgressBar，并命名为 HealthBar；初始化 Widget 时，UE 才能把对应对象引用绑定到成员。

| C++ 成员名 | 蓝图控件名 | 控件类型 |
| --- | --- | --- |
| HealthBar | HealthBar | Progress Bar |
| HealthText | HealthText | Text Block |
| WaveText | WaveText | Text Block |
| EnemyCountText | EnemyCountText | Text Block |

名字要一致，控件类型要匹配。当前使用普通 BindWidget，表示要求这些绑定存在；没有采用可缺省的 BindWidgetOptional。函数里的空指针检查并不代替蓝图的绑定要求。

### 2.3 两种“绑定”别混淆

| 概念 | 做什么 | 当前是否采用 |
| --- | --- | --- |
| C++ 的 BindWidget | 将成员引用连接到同名控件 | 是 |
| 设计器 Percent / Text 属性旁的函数绑定 | 通过属性绑定计算显示值 | 当前 C++ 没有依赖这种做法 |
| 变化时显式调用 UpdateHealth / UpdateWaveInfo | 把新数值推给控件 | 是 |

BindWidget 本身不会监听 CurrentHealth，也不会因为血量变化而自动调用 UpdateHealth。数据刷新仍由调用链完成。

### 2.4 模块和头文件

当前 Build.cs 已加入 UMG：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput", "AIModule", "UMG"
});
```

MyHUDWidget.cpp 包含：

```cpp
#include "MyHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
```

前向声明适合在头文件中声明指针；实际调用 SetPercent、SetText 时，需要控件类的完整定义。UMG 模块依赖与 include 分别负责构建依赖和 C++ 类型定义，不能互相替代。

UpdateHealth 和 UpdateWaveInfo 目前是 public 普通 C++ 函数，没有 BlueprintCallable，当前由 C++ 对象直接调用。不能只因为基类是 UUserWidget 就假定所有成员函数都自动出现在蓝图节点列表。

## 3. 蓝图部分：外观与 C++ 引用怎样对应

当前资源位置是 `Content/BluePrints/UI/WBP_HUD`，要确认它继承 MyHUDWidget。

1. 打开 WBP_HUD，核对父类。已有 Widget Blueprint 需要时可通过类设置重新指定父类。
2. 在设计器中放一个 Progress Bar 和三个 Text Block，按第 2 节表格命名，核对 Is Variable。
3. 配置布局、锚点、字号、颜色等。布局可按屏幕角落摆放，但不影响 C++ 更新入口的职责。
4. 编译并保存 Widget Blueprint，让父类绑定要求得到检查。
5. 打开 BP_MyArenaShooterCharacter 的类默认值，在 UI 分类设置 HUDWidgetClass = WBP_HUD。

蓝图负责控件树和布局，C++ 提供显示行为；“创建了 WBP_HUD 资产”还不代表游戏已使用它，角色的类属性必须指定。

这里只列当前代码要求的控件关系，不把示例布局说成源码已经配置好的实际位置。血条、文本是否被其他控件遮挡或出屏幕，需要在设计器和 Play 中观察。

## 4. 玩家创建并持有 HUD：先有实例，再发送数据

### 4.1 类配置和实例引用

角色头文件先声明 `class UMyHUDWidget;`，已有类内部保存：

```cpp
private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UMyHUDWidget> HUDWidgetClass;

    UPROPERTY()
    UMyHUDWidget* HUDWidget = nullptr;
```

HUDWidgetClass 保存要创建的类，通常填写 WBP_HUD；HUDWidget 保存运行时创建出的那个实例。与武器系统中“武器类 / 当前武器实例”的区别相同。

不能对一个类配置当作实例调用 UpdateHealth，也不用每次受伤都重新创建界面。UPROPERTY 让 UE 认识角色保存的对象引用。

### 4.2 BeginPlay 中实际执行的 UI 部分

角色 .cpp 包含 MyHUDWidget.h。以下从现有 BeginPlay 中摘取，之后仍继续执行输入映射与武器生成：

```cpp
CurrentHealth = MaxHealth;

if (HUDWidgetClass)
{
    HUDWidget = CreateWidget<UMyHUDWidget>(
        GetWorld(),
        HUDWidgetClass
    );

    if (HUDWidget)
    {
        HUDWidget->AddToViewport();
        HUDWidget->UpdateHealth(CurrentHealth, MaxHealth);
    }
}
```

伪代码中注意三个不同对象和阶段：

```text
玩家开始游戏：
    当前血量 = 配置的最大血量
    如果 HUD 类已经配置：
        创建这个类的 Widget 实例，保存到 HUDWidget
        如果创建成功：
            把该实例添加到屏幕
            把当前血量和最大血量传给该实例
    继续已有输入、武器初始化
```

### 4.3 首次出现：CreateWidget 与 AddToViewport

`CreateWidget<UMyHUDWidget>(GetWorld(), HUDWidgetClass)` 根据所指定的 Widget 类创建实例，返回可调用更新函数的引用。模板参数表示期望得到的 C++ 类型，蓝图类可以是它的派生类。

`AddToViewport()` 将已经创建的实例加入游戏视口。只调用 CreateWidget 不等于已经显示；只在设计器里预览也不等于已添加到运行中的屏幕。[UE 5.4 创建与显示 Widget 的说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-widgets-in-unreal-engine?application_version=5.4)

创建与显示之后仍要调用 UpdateHealth。开局没有受伤事件，不意味着 UI 会自动显示最大血量，所以要主动发送一次初始数据。

当前在 Character BeginPlay 创建 HUD，按这个单人项目理解。没有另外用 PlayerController、AHUD 或 UI Manager 创建，也没有在 HUD 类中实现 NativeConstruct 来自动查找玩家。

## 5. 血量显示：游戏值怎样变成比例和文字

### 5.1 UpdateHealth 的当前实现

```cpp
void UMyHUDWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HealthBar)
    {
        float Percent = 0.0f;

        if (MaxHealth > 0.0f)
        {
            Percent = CurrentHealth / MaxHealth;
        }

        HealthBar->SetPercent(Percent);
    }

    if (HealthText)
    {
        FString HealthString = FString::Printf(
            TEXT("%.0f / %.0f"),
            CurrentHealth,
            MaxHealth
        );

        HealthText->SetText(FText::FromString(HealthString));
    }
}
```

这个函数不修改玩家血量。它只是接收两个数，分别更新两个可用控件：血量条的比例、血量文字。

### 5.2 首次出现：UProgressBar::SetPercent

```text
当前血量 = 80，最大血量 = 100
→ 比例 = 80 / 100 = 0.8
→ HealthBar->SetPercent(0.8)
→ 血量条显示约 80% 的填充
```

ProgressBar 使用 0～1 的比例表达填充进度，不是直接传 80 表示 80%。血量值和比例是不同单位。

| 血量 | 最大血量 | 传入比例 |
| --- | --- | --- |
| 100 | 100 | 1.0 |
| 80 | 100 | 0.8 |
| 25 | 100 | 0.25 |
| 0 | 100 | 0.0 |

当前先把 Percent 初始化为 0，再检查 MaxHealth > 0 才做除法，避免最大血量为 0 时除零。Widget 这里没有额外 Clamp 比例；正常受伤链已在玩家那里限制实际血量。

### 5.3 首次出现：FString::Printf、FText::FromString、SetText

```cpp
FString HealthString = FString::Printf(
    TEXT("%.0f / %.0f"),
    CurrentHealth,
    MaxHealth
);

HealthText->SetText(FText::FromString(HealthString));
```

可以分三步读：

```text
数值80、100
↓ Printf按格式组织字符串
FString："80 / 100"
↓ FromString转成显示文本
FText
↓ SetText写到控件
HealthText显示新文字
```

`%.0f` 表示浮点数按零位小数格式输出，显示时会舍入；它不会把玩家内部血量永久转成整数。血条计算仍使用传入浮点数。

FString 适合字符串处理；SetText 接收 FText，因此使用 FromString 转换。当前用这种方式格式化显示，没有建立完整本地化文本格式资源。

### 5.4 玩家受伤时为什么在这个位置刷新

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

先改数据、再限制范围、再通知 UI，最后做死亡日志判断：

```text
敌人攻击 → ReceiveEnemyDamage(20)
→ CurrentHealth -= 20
→ Clamp到0～MaxHealth
→ HUDWidget->UpdateHealth(最终血量, 最大血量)
→ 血条比例和文字刷新
→ 归零时打印Player Dead
```

如果在扣血前刷新，就会把旧值送给 UI；如果每次只改显示文字，不改 CurrentHealth，玩法判断仍会使用旧血量。这里刷新的是扣血并 Clamp 后的最终值。

HUDWidget 没创建成功时，只跳过显示更新，前面的扣血仍然发生。因此 UI 不显示和玩家没受到伤害是两种不同问题。

## 6. 波次显示：Spawner 怎样通知 HUD

### 6.1 Widget 的 UpdateWaveInfo

```cpp
void UMyHUDWidget::UpdateWaveInfo(
    int32 CurrentWave,
    int32 TotalWaves,
    int32 RemainingEnemies
)
{
    if (WaveText)
    {
        FString WaveString = FString::Printf(
            TEXT("敌人波次 %d / %d"),
            CurrentWave,
            TotalWaves
        );

        WaveText->SetText(FText::FromString(WaveString));
    }

    if (EnemyCountText)
    {
        FString EnemyString = FString::Printf(
            TEXT("敌人剩余数量: %d"),
            RemainingEnemies
        );

        EnemyCountText->SetText(FText::FromString(EnemyString));
    }
}
```

例如调用 `UpdateWaveInfo(2, 3, 4)`，当前文案显示：

```text
敌人波次 2 / 3
敌人剩余数量: 4
```

%d 对应整数参数。Widget 不把当前波次自动加 1，不查询敌人 Actor，也不计算什么时候结束本波；这些结果由 Spawner 计算后传入。

### 6.2 玩家提供一层转发

```cpp
void AMyArenaShooterCharacter::UpadteWaveHUD(
    int32 CurrentWave,
    int32 TotalWaves,
    int32 RemainingEnemies
)
{
    if (HUDWidget)
    {
        HUDWidget->UpdateWaveInfo(CurrentWave, TotalWaves, RemainingEnemies);
    }
}
```

名字 `UpadteWaveHUD` 保留当前源码拼写，头文件声明、实现和 Spawner 调用一致。它是自定义函数，不是 UE API。以后纠正为 UpdateWaveHUD 时，要同步修改三处。

这个方法不创建 UI，不改变波次，只检查自己已有 HUD 引用并转发数值。调用者无需访问角色私有的 HUDWidget 成员。

### 6.3 为什么通过玩家转发

```text
Spawner拥有波次数据
↓ 查找到的PlayerCharacter
Player拥有HUDWidget引用
↓
Widget拥有四个控件引用
```

当前组织方式下，Spawner 知道玩家公开的业务入口，Widget 知道控件名字。Spawner 不必获取 EnemyCountText，也不必为每只敌人重新创建界面。

这是一种适合当前项目的直接调用方式。将来可以把 UI 所有权移到 PlayerController 或 UI 管理对象，但那不是本次已实现的调用链。

### 6.4 何时发送波次数据

| 时机 | Spawner已完成什么 | 传给HUD的内容 |
| --- | --- | --- |
| SpawnWave生成循环结束后 | 本波敌人生成尝试完成，记录成功实例 | 索引+1、总波数、实际记录数量 |
| CleanupDeadEnemies每次成功删除后 | 本次失效引用已从数组移除 | 当前波次不变、剩余数量减少 |
| 本波清空并生成下一波后 | 索引已前进，新波名单建立 | 新波编号与新数量 |

SpawnWave 通知时使用 CurrentWaveIndex+1。索引0表示配置的第一项，显示给玩家则是第1波；Widget 不再重复加1。

### 6.5 难点：杀死一个敌人，不是 Enemy 直接改 HUD

```text
子弹命中Enemy
↓
Enemy扣血归零，Destroy
↓
Spawner的某次Tick调用CleanupDeadEnemies
↓
IsValid(这个Enemy引用)失败
↓
RemoveAt删除对应数组槽位，数组数量减少
↓
PlayerCharacter->UpadteWaveHUD(波次, 总波数, 新数量)
↓
HUDWidget->UpdateWaveInfo
↓
EnemyCountText->SetText
```

Enemy 不知道 HUD 控件的名字；Spawner 负责发现本波名单变化。数组清理前可能短暂保留失效引用，这也是数量刷新时机与 Enemy Destroy 不完全是同一个步骤的原因。

当前 CleanupDeadEnemies 在每次 RemoveAt 后都可能通知；一轮清理多个敌人会多次调用，而不是函数结束只调用一次。当前没有委托自动把死亡事件广播给 UI。

### 6.6 最后一只死亡到下一波显示：用伪代码读跨对象顺序

```text
Spawner的一次Tick：
    如果本波不活动：返回

    清理失效名单：
        最后一个Enemy被移除
        通知玩家 → Widget显示旧波次、剩余0

    如果名单为空：
        关闭旧波活动标记
        波次索引增加
        调用SpawnWave：
            如果仍有配置：
                生成下一波并记录成功引用
                通知玩家 → Widget显示新波次、新数量
            如果已超出配置：
                输出All Waves Completed并返回
```

旧波剩余0和新波开始可能在同一次 Tick 中先后更新，屏幕未必渲染出中间状态。最后完成时没有专门的胜利UI调用，所以通常保留最后一波编号和剩余0；不能从日志里有完成信息就推断屏幕也已实现完成页面。

## 7. 初始化依赖：数据已存在，不等于界面已经准备好

### 7.1 两条准备链要同时成立

```text
UI准备链：
角色BeginPlay → HUDWidgetClass已配置 → CreateWidget成功
            → 同名控件绑定建立 → AddToViewport → 初始血量更新

波次准备链：
Spawner BeginPlay → 查询PlayerCharacter
                 → SpawnWave → 通知玩家刷新波次
```

它们属于不同 Actor 的初始化，不能假定所有 BeginPlay 严格按“玩家完整初始化完 → Spawner才开始”执行。

### 7.2 难点：为什么有玩家引用也可能没显示第一波

当前转发函数只做空指针检查，没有保存待显示数据。可能的执行情形用伪代码表示：

```text
Spawner先找到玩家Actor，但该玩家尚未创建HUD
↓
Spawner生成第一波，调用Player.UpadteWaveHUD
↓
Player检查HUDWidget为空 → 本次更新没有转发
↓
后来Player创建HUD，只调用初始UpdateHealth
↓
波次信息不会因为“后来HUD出现了”而自动补发
```

这是当前代码允许出现的初始化边界，不表示每次 Play 都会发生。Spawner 也只在 BeginPlay 查询一次玩家，如果查询结果为空，后续没有重新获取。

若表现为血量正常、波次起初没更新，直到击杀敌人后才变动，应检查通知是否早于 HUD 准备完成。后续可以在 HUD 就绪时请求当前数据，或缓存最近数据后补发；本次代码尚未做这些处理。

### 7.3 判断对象可用与判断数据显示正确是两回事

```text
PlayerCharacter有效？ → 可以调用玩家函数
HUDWidget非空？       → 可以进入当前显示转发
HealthBar等绑定存在？ → 可以设置对应控件
传入的是最新数据？   → 画面是否与玩法一致
```

如果 HUD 丢失，本次扣血和波次生成仍可能正常执行。空指针检查保证这段访问有条件进行，不会自动修复资产配置，也不会将遗漏的刷新排队等待。

## 8. 当前实现、后续完善和常见误解

### 8.1 当前已经完成

- 在角色 BeginPlay 创建并显示 WBP_HUD，保存运行时引用。
- 初始化并刷新玩家血量条、血量文字。
- 玩家受伤后修改实际血量并 Clamp，再通知 HUD。
- Spawner 生成本波、清理失效敌人引用时，刷新当前波次与剩余数量。
- Widget 通过同名控件绑定、比例计算和文本格式化显示数据。

当前没有弹药、准星、攻击动画UI、死亡页面、胜利页面、刷新动画或统一UI管理系统。这里只记录实际显示入口，不把示意布局当作已实现功能。

### 8.2 为什么目前不用 UI Tick

血量变化发生在 ReceiveEnemyDamage，波次变化发生在生成与清理名单的函数中。这些地方本来就知道“刚刚改了数据”，因此直接把新值发送一次即可。

```text
当前方式：发生变化 → 调用一次显示更新

另一种方式：Widget每帧查询 → 即使没变化也反复读取和设置
```

当前使用前一种思路，没有增加 Widget 的逐帧查询逻辑。但 Spawner 为了发现失效敌人仍有自己的 Tick，不要把“UI不轮询”误记成“整个系统不逐帧检查”。

### 8.3 BindWidget 与属性函数绑定不能混用理解

当前通过 SetText 直接设置显示文本。若另在设计器对同一 Text 属性建立函数绑定，直接 SetText 会清除该属性的绑定，不能假定两套更新方式同时持续起作用。[SetText 官方说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/UTextBlock/SetText)

这里说的是 Text 属性的计算绑定；BindWidget 对控件成员引用的连接不是这条文本属性绑定。不要因看到这条说明就误以为 SetText 会把 HealthText 成员指针解除。

### 8.4 血量为0，UI显示0，但角色仍能操作

ReceiveEnemyDamage 的死亡分支仍只有日志。Widget 显示的是当前状态，不负责关闭角色输入、停止敌人攻击或重生。玩家血量会被 Clamp 留在0，仍可能反复进入死亡日志分支。

因此死亡页面和玩法死亡状态需要另外实现；血条已归零不会自动替代这些功能。

## 9. 沿执行链排查UI问题

```text
HUDWidgetClass是否填写WBP_HUD？
↓
CreateWidget是否返回实例？
↓
AddToViewport是否执行？
↓
蓝图父类、控件名、控件类型是否符合BindWidget？
↓
更新入口是否调用，传入数据是否正确？
↓
SetPercent / SetText是否执行？
↓
布局、可见性、颜色和锚点是否让内容能被看到？
```

| 现象 | 优先检查 |
| --- | --- |
| 完全没有界面 | HUDWidgetClass、CreateWidget返回值、AddToViewport和当前玩家蓝图 |
| Widget Blueprint编译提示缺少绑定 | 父类是否为MyHUDWidget，四个控件名字与类型是否对应 |
| 血条一直满或空 | UpdateHealth是否执行，比例是否是0～1，最大血量是否为正数 |
| 文字变化但血条没变化 | HealthBar绑定与可见性，以及SetPercent是否执行 |
| 开局血量正常，受伤不变 | ReceiveEnemyDamage是否调用、扣血后的UpdateHealth是否执行 |
| 玩家已经受伤，画面却没显示 | UI创建或控件配置问题；玩法扣血和显示更新分别验证 |
| 波次不显示或初始值错误 | Spawner的玩家引用、HUD创建时机、UpadteWaveHUD是否丢失了初始化通知 |
| 击杀后数量不减少 | 被击杀者是否属于本波数组，CleanupDeadEnemies是否移除，转发是否执行 |
| 数量短暂跳变或新波直接出现 | 清理后刷新与下一波生成可能在同一次Tick先后发生 |
| 全部波次完成但没有胜利界面 | 当前只有完成日志，尚未调用完成页面显示方法 |
| 屏幕上有多个HUD | 是否重复创建，或有多个角色各自执行创建；不要每次受伤都CreateWidget |

当前HUD主要是数据显示。需要互动菜单时再设置输入模式和鼠标光标；不要为了显示血量直接切成 UIOnly，让角色移动和射击输入停止。[创建Widget及输入模式说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-widgets-in-unreal-engine?application_version=5.4)

## 10. 第一次出现的重要API速查

| API / 类型 | 当前用途 |
| --- | --- |
| UUserWidget | C++ HUD与Widget Blueprint使用的用户界面基类 |
| UMG模块 | 构建项目所使用的Widget类型和功能 |
| UPROPERTY(meta=(BindWidget)) | 将C++控件引用连接到蓝图同名控件 |
| UProgressBar | 血量条控件 |
| UTextBlock | 血量、波次、数量的文本控件 |
| `CreateWidget<T>`() | 根据Widget类创建运行时实例 |
| AddToViewport() | 将实例加入游戏视口显示 |
| SetPercent() | 更新进度条填充比例 |
| FString::Printf() | 按格式将多个数值组织成字符串 |
| FText::FromString() | 把字符串转为UI显示用文本 |
| UTextBlock::SetText() | 将显示文本写入控件 |
| FMath::Clamp() | 将玩家实际血量限制在有效范围 |

TSubclassOf、UPROPERTY、GetWorld、IsValid在前面已经使用。本节重点是它们怎样支持UI引用和调用，而不是只记一个新的函数名字。

UpdateHealth、UpdateWaveInfo、UpadteWaveHUD、ReceiveEnemyDamage都是本项目自己的函数。CreateWidget、AddToViewport、SetPercent、SetText等才是引擎提供的API。

## 11. 不看代码尝试复述

1. 血量、波次、剩余敌人数量各自由哪个对象维护？Widget是否负责生成敌人？
2. HUDWidgetClass、HUDWidget、HealthBar三个引用分别指向什么？
3. BindWidget为什么不能代替UpdateHealth？设计器的属性绑定与它有什么区别？
4. 只CreateWidget为什么还看不到UI？AddToViewport之后为什么还要发送初始数据？
5. 80点血量如何变成0.8和“80 / 100”？哪一步修改实际血量，哪一步只改显示？
6. 玩家受伤为什么先扣血、Clamp，再刷新？HUD为空时实际扣血是否仍发生？
7. Enemy死亡之后，谁更新剩余数量？为什么数组还需要RemoveAt？
8. Spawner、Character、Widget这三层各自知道什么？为什么当前通过玩家转发？
9. 同一次Tick里清理旧波、创建新波，画面一定能看到剩余0的中间状态吗？
10. 为什么血量能显示，但第一波信息可能直到击杀后才更新？
11. 最后一波清空后当前HUD显示什么？完成日志为什么不是胜利页面？
12. 玩家血条归零为什么仍可能移动、被攻击、重复输出死亡日志？

复习时先口述“角色创建HUD → 发初始血量 → AI攻击 → 玩家扣血 → 更新血条和文本”，再口述“Spawner生成本波 → 玩家转发 → 显示波次 → Enemy销毁 → 清理引用 → 显示剩余数 → 清空后显示下一波”。能找到每个箭头对应的调用位置，就掌握了当前UI实现的主线。


## 12. 后续学习记录：在原有血量与波次 HUD 上继续增加功能

前面的第 1～11 节保留最初实现血量、波次与剩余敌人数的步骤，以及当时尚未完成的部分。这一阶段继续使用同一个 UMyHUDWidget 和 WBP_HUD，增加弹药文字、死亡提示与重新开始，不删除之前的创建、绑定和更新过程。

| 原有基础 | 这一阶段增加 | 设计思路 |
| --- | --- | --- |
| Character 管理血量、Spawner 管理波次 | Weapon 管理弹药 | 谁拥有真实数据，谁在修改后发出显示通知 |
| Character 保存 HUD 引用并转发波次 | 新增 UpadteAmmoHUD | 武器不直接访问 AmmoText |
| BindWidget 绑定血量、波次控件 | 新增 AmmoText、GameOverText | 保留原有控件，再补对应引用 |
| UpdateHealth / UpdateWaveInfo 显示数值 | UpdateAmmo / ShowGameOver | 数值变化更新文字，死亡事件更新可见性 |
| 血量归零时只有日志 | Die 设置状态并显示提示，RestartGame 重开地图 | 显示提示和改变玩法状态分别执行 |

武器的射击间隔、弹药转移与定时器在第2节后续学习记录中说明；这里关注它们怎样传到屏幕，以及死亡到重开的执行链。

## 13. 弹药显示：由武器维护，通过玩家送到控件

### 13.1 先明确 30 / 90 表示什么

| 文字 | 左边的来源 | 右边的来源 |
| --- | --- | --- |
| 血量 100 / 100 | 玩家 CurrentHealth | 玩家 MaxHealth |
| 弹药 30 / 90 | 武器 CurrentAmmo：弹匣余量 | 武器 ReserveAmmo：备用库存 |

格式虽然都是数字 / 数字，业务含义不同。弹药右侧不是弹匣容量 30，也不是包括弹匣在内的总弹药 120。HUD 不负责判断能不能开火、够不够装满；那部分在武器里计算。

### 13.2 Widget 的 UpdateAmmo

```cpp
void UMyHUDWidget::UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo)
{
    if (AmmoText)
    {
        FString AmmoString = FString::Printf(
            TEXT("%d / %d"),
            CurrentAmmo,
            ReserveAmmo
        );

        AmmoText->SetText(FText::FromString(AmmoString));
    }
}
```

这里复用前面已解释的 FString::Printf、FText::FromString 和 SetText。%d 对应 int32 数值，用当前弹匣与备用数量组织字符串，然后写入 AmmoText：

```text
UpdateAmmo(29, 90)
→ FString "29 / 90"
→ 转为 FText
→ AmmoText 显示29 / 90
```

函数没有读取 MagazineSize，也没有扣除弹药；只负责显示调用者传入的结果。

### 13.3 两层转发各自解决什么问题

武器知道自己的 Owner，玩家知道自己的 HUDWidget，Widget 知道 AmmoText。当前武器转发如下：

```cpp
void AMyWeapon::UpdateAmmoHUD()
{
    AMyArenaShooterCharacter* OwnerCharacter =
        Cast<AMyArenaShooterCharacter>(GetOwner());

    if (IsValid(OwnerCharacter))
    {
        OwnerCharacter->UpadteAmmoHUD(
            CurrentAmmo,
            ReserveAmmo
        );
    }
}
```

玩家继续转发：

```cpp
void AMyArenaShooterCharacter::UpadteAmmoHUD(int32 CurrentAmmo, int32 ReserveAmmo)
{
    if (HUDWidget)
    {
        HUDWidget->UpdateAmmo(
            CurrentAmmo,
            ReserveAmmo
        );
    }
}
```

```text
Weapon.CurrentAmmo / ReserveAmmo
↓ 武器.UpdateAmmoHUD：GetOwner、Cast、IsValid
Character.UpadteAmmoHUD
↓ 检查已保存的HUDWidget
HUDWidget.UpdateAmmo
↓ 检查已绑定的AmmoText
AmmoText.SetText
```

生成武器时 SpawnParams.Owner = this，this 是玩家，所以武器能取得这个角色。GetOwner 不是查相机组件父级；挂到相机和设置 Owner 是两个不同关系。`UpadteAmmoHUD` 沿用当前源码拼写，是自定义函数，不是引擎 API。

### 13.4 开局先创建 HUD，再初始化并读取武器弹药

以下摘自角色 BeginPlay 的武器生成成功分支，在挂载和位置偏移之后：

```cpp
UpadteAmmoHUD(
    CurrentWeapon->GetCurrentAmmo(),
    CurrentWeapon->GetReserveAmmo()
);
```

具体顺序可以用伪代码读：

```text
Character BeginPlay：
    创建并显示 HUD，发送初始血量
    启用输入映射
    正常 SpawnActor 默认武器：
        武器 BeginPlay 根据配置装入弹匣和初始库存
        返回武器引用
    如果武器生成成功：
        保存引用、挂到相机、设置偏移
        读取这把武器的两个 getter
        玩家转发 → HUD 显示初始弹药
```

当前是在已开始游戏的角色 BeginPlay 中正常、非延迟生成武器；武器的 BeginPlay 在这个生成流程中执行，初始化后才读取 getter。若未来改成延迟生成或更早的生命周期位置，要重新核对初始化时机，不能把任何 SpawnActor 都理解成调用时一定已完成 BeginPlay。

弹药为武器内部数据，UI 不应自己先写死 30 / 90 当作运行值；蓝图改变弹匣与库存配置后，getter 读出的才是实际值。

### 13.5 一发射击和一次换弹分别什么时候刷新

```text
射击：
Weapon.Fire → 生成子弹的调用 → --CurrentAmmo
→ UpdateAmmoHUD → Character → Widget → 显示29 / 90

手动换弹（弹匣10、备用90、容量30）：
StartReload → 换弹状态=true → 定时器等待1.5秒
→ 此时文字仍为10 / 90
→ FinishReload：转移20发，得到30 / 70
→ UpdateAmmoHUD → Character → Widget → 显示30 / 70

打空自动换弹：
最后一发后显示0 / 备用
→ StartReload等待
→ 完成后显示新的弹匣 / 备用
```

StartReload 当前没有调用显示更新函数，也没有换弹进度控件，所以等待中数值保持旧状态是当前实现的结果。备用不足时可能显示 15 / 0，不应由 UI 把它强行补成满弹匣。

当前 Fire 没检查 SpawnActor 的返回值，生成失败后仍可能减弹药并更新文字。因此“HUD 数字减了”只证明扣弹药链执行，不能单独证明子弹成功生成。第 2 节继续说明这段武器行为。

## 14. 死亡提示与重新开始：显示和玩法状态分开

### 14.1 在原有扣血后增加 Die，再通知 HUD

先在角色头文件的 private 区段新增死亡状态：

```cpp
bool bIsDead = false;
```

ReceiveEnemyDamage 保留原来扣血、Clamp、刷新血量的步骤，把最后血量归零分支从只打印日志扩展为调用 Die：

```cpp
if (CurrentHealth <= 0.0f)
{
    Die();
}
```

角色头文件还需要声明 private 的 `void Die();`，具体实现如下：

```cpp
void AMyArenaShooterCharacter::Die()
{
    bIsDead = true;

    GetCharacterMovement()->DisableMovement();

    if (HUDWidget)
    {
        HUDWidget->ShowGameOver();
    }

    UE_LOG(LogTemp, Warning, TEXT("Player Dead"));
}
```

```text
AI攻击 → ReceiveEnemyDamage
→ 扣血并Clamp → UpdateHealth显示0
→ CurrentHealth <= 0 → Die
→ bIsDead=true
→ CharacterMovement.DisableMovement
→ HUDWidget.ShowGameOver
→ 输出Player Dead日志
```

`bIsDead` 是玩法状态，`GameOverText` 是这个状态的可见提示。只显示文字不会自动停止角色，只设死亡标记也不会自动出现文字，所以需要在同一个死亡入口分别执行。

首次出现的 `GetCharacterMovement()->DisableMovement()` 将角色移动模式设为 MOVE_None，停用这个角色移动组件的移动；它不等于 DisableInput，也不会把所有增强输入回调、武器 Tick、敌人 Tick 或定时器全部关闭。

当前玩家的 Move、StartFireInput、ReloadInput 会先检查 bIsDead；Look、Dash 和直接绑定的 Jump 没有统一的死亡检查。Die 也没有销毁玩家。因此要按实际入口理解当前限制，不记成“显示 Game Over 后整个游戏都暂停”。

### 14.2 ShowGameOver 只改变已有控件的可见性

```cpp
void UMyHUDWidget::ShowGameOver()
{
    if (GameOverText)
    {
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }

}
```

没有重新 CreateWidget，也没有增加一个独立菜单。GameOverText 早在 HUD 创建时绑定，这里只将它从隐藏改成可见。文字内容来自 Widget Blueprint 的配置。

### 14.3 首次出现：SetVisibility 与 ESlateVisibility

| 可见性值 | 是否显示 | 是否占布局空间 |
| --- | --- | --- |
| Visible | 显示 | 占用 |
| Hidden | 隐藏 | 保留 |
| Collapsed | 隐藏 | 不保留 |

Visible 还允许命中测试；本次用途是显示提示。默认 Hidden 或 Collapsed 的选择会影响布局是否保留空间，但都可通过 SetVisibility(Visible) 显示。枚举含义与本地 UE 5.4 的 SlateWrapperTypes.h 一致，可参考 [ESlateVisibility 官方说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/ESlateVisibility)。

C++ 当前没有在创建 HUD 时隐藏 GameOverText。要在设计器设置初始隐藏；否则开局就可能看到 Game Over。即使子控件改成 Visible，父容器若仍隐藏或布局出屏幕，也不会自动变成可见画面。

### 14.4 RestartAction 到重新加载地图的执行链

角色头文件新增操作资产和回调：

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Input")
UInputAction* RestartAction = nullptr;

void RestartGame(const FInputActionValue& Value);
```

以下绑定摘自已有 SetupPlayerInputComponent：

```cpp
if (RestartAction)
{
    EnhancedInputComponent->BindAction(
        RestartAction,
        ETriggerEvent::Started,
        this,
        &AMyArenaShooterCharacter::RestartGame
    );
}
```

```cpp
void AMyArenaShooterCharacter::RestartGame(const FInputActionValue& Value)
{
    if (!bIsDead)
    {
        return;
    }

    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);

    UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
```

```text
死亡提示已显示，玩家按自己映射的重开键
→ IMC_Player找到IA_Restart → Started
→ Character.RestartGame
├─ bIsDead=false：直接返回，存活时不重开
└─ bIsDead=true：取得当前地图名 → 转为FName → OpenLevel加载同一张地图
```

需要在 IMC_Player 里给 IA_Restart 配置按键，并在 BP_MyArenaShooterCharacter 指定 RestartAction = IA_Restart。提示里写“按某键重开”只是文字，不会自动建立映射；换弹和重开也是两个不同 Input Action。

### 14.5 首次出现：GetCurrentLevelName、FName、OpenLevel

```cpp
FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
```

| 部分 | 当前作用 |
| --- | --- |
| this | 当前角色作为 WorldContextObject，让工具函数找到所属世界 |
| GetCurrentLevelName | 返回正在打开的地图名称 FString |
| true | 移除编辑器或流送添加的地图名前缀，适合 PIE 中取得正常地图名 |
| *CurrentLevelName | FString 的运算符返回字符指针，不是解引用角色或地图对象 |
| FName(...) | 将字符串内容转换成 OpenLevel 所需的名称类型 |
| OpenLevel | 按名称打开地图；这里传当前名字，所以重新加载当前关卡 |

角色 .cpp 已包含 Kismet/GameplayStatics.h。GetCurrentLevelName 的 true 是“移除前缀”，与 OpenLevel 的其他参数不是同一个设置。当前 OpenLevel 只传世界上下文和名称，其余使用默认参数。可对照 [获取地图名](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Game/GetCurrentLevelName)与[按名称打开关卡](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Game/OpenLevel_byName)的官方说明。

这次重开走的是关卡重新加载，正常关卡内的角色、武器与 Spawner 重新初始化，HUD 也随新角色创建，血量和弹药回到开局配置，波次从开局开始。它没有在旧角色上逐项手动恢复全部数据，也没有在这里实现跨关卡保存进度。

### 14.6 难点：死亡时需要处理已经在运行的状态

```text
已有按住开火状态：bFirePressed=true
↓
玩家血量归零 → Die
↓
新的StartFireInput会因bIsDead返回
但Die没有调用Weapon.StopFire
↓
武器仍有自己的Tick，也未检查持有者是否死亡
↓
旧的按住状态可能继续生成子弹，直到松开触发StopFireInput
```

同样，已经开始的换弹没有被 Die 取消；玩家归零也没有让 AI 自动停止攻击。ReceiveEnemyDamage 和 Die 未增加“已死亡则直接返回”的防重复分支，因此仍可能反复调用 ShowGameOver、输出死亡日志。当前显示同一个控件，不会因为这些调用自行新建多个 HUD。

后续完善时，可让 Die 成为统一的状态收尾入口：避免重复执行死亡处理、停止当前武器、决定是否取消换弹、统一限制需要禁止的操作，再决定敌人如何处理失效目标。这些是基于现有代码的下一步思路，本次尚未加入源码。


## 15. 这一阶段新增控件和资产的连接

### 15.1 保留已有控件，追加新的 BindWidget 成员

MyHUDWidget.h 原有 HealthBar、HealthText、WaveText、EnemyCountText 继续保留。在现有类内部补充以下成员与方法，访问区段按当前源码整理：

```cpp
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AmmoText;

private:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;

public:
    void UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo);
    void ShowGameOver();
```

GameOverText 是 private，也可以建立 BindWidget 引用；外部通过公开的 ShowGameOver 通知显示，不直接改这个控件成员。这些新方法仍是普通 C++ 函数，没有添加 BlueprintCallable，不能当成已经提供的蓝图可调用节点。

| C++ 成员 | WBP_HUD 控件名 | 类型 | 初始设置 |
| --- | --- | --- | --- |
| AmmoText | AmmoText | Text Block | 可见；游戏开始后由初始弹药更新覆盖示意文字 |
| GameOverText | GameOverText | Text Block | 默认 Hidden 或 Collapsed，填写死亡与重开提示 |

当前总计一个 Progress Bar、五个 Text Block，六个 BindWidget 对应关系。普通 BindWidget 要求这些控件存在，方法内部的空指针判断不能代替蓝图编译时的绑定要求。

### 15.2 在原有配置上补接输入与显示

```text
WBP_HUD 保留 MyHUDWidget 父类与已有控件
→ 新建 AmmoText、GameOverText，名称、类型对应
→ GameOverText 初始隐藏，提示按键与实际重开映射一致
→ 编译并保存 Widget Blueprint

IMC_Player 保留已有操作
→ 增加 IA_Reload、IA_Restart 的按键映射
→ 玩家类默认值指定 ReloadAction、RestartAction
→ HUDWidgetClass 仍使用 WBP_HUD
```

具体换弹与重开键以自己在 IMC 中配置的按键为准。第14节展示 RestartAction 的 Started 绑定，换弹输入在第2节后续学习记录中展开。当前死亡提示是 HUD 内的文字，没有独立菜单、按钮或鼠标交互逻辑。

### 15.3 按新增执行链排查

| 现象 | 优先确认 |
| --- | --- |
| 新代码后 Widget Blueprint 提示缺少绑定 | AmmoText、GameOverText 的名称、类型和父类 |
| 开局弹药没有更新 | 武器生成成功，getter读取与UpadteAmmoHUD执行，HUD已创建 |
| 射击 / 换弹后数字不变 | 武器Owner是否为玩家，两层转发与AmmoText绑定 |
| 数字少了却没有看到子弹 | Fire未检查SpawnActor返回值，也需检查出生碰撞；UI变化不能证明生成成功 |
| 等待换弹时数字保持旧值 | 当前开始换弹未更新文字，完成时才转移与刷新 |
| 开局就显示Game Over | GameOverText设计器初始Visibility是否隐藏 |
| 归零后没有提示 | Die、HUD引用、ShowGameOver、控件绑定、父容器可见性 |
| 重开键无效 | 是否已死亡，RestartAction赋值、映射与Started绑定 |
| 死亡后仍连射 / 仍有敌人攻击日志 | 第14.6节说明的已有状态尚未统一收尾 |

显示更新继续发生在数据或状态变化处，没有新增 Widget Tick 轮询。Weapon Tick 控制连射，Spawner Tick 发现敌人失效，UI只接收对应通知。

## 16. 本阶段重要 API 与复习

### 16.1 速查

| API / 类型 | 本次作用 |
| --- | --- |
| GetOwner / Cast / IsValid | 武器找到有效的玩家持有者，再通知显示 |
| FString::Printf / FText::FromString / SetText | 沿用前面已学 API，把弹匣与库存写成文字 |
| SetVisibility / ESlateVisibility | 改变已有提示控件可见性；不创建新Widget |
| DisableMovement | 把角色移动模式设为MOVE_None，不等于停掉所有输入和Actor |
| GetCurrentLevelName(this, true) | 取得当前地图名，去掉编辑器或流送前缀 |
| FName(*CurrentLevelName) | 从FString内容构造名称，作为地图名参数 |
| OpenLevel | 按名称加载地图，本次加载同一地图以重新开始 |

UpdateAmmo、UpadteAmmoHUD、ShowGameOver、Die、RestartGame 是自己的业务函数，不是新增 UE 内置 API。新功能仍建立在之前的 UUserWidget、BindWidget、CreateWidget、AddToViewport 基础上。

### 16.2 不看代码复述新增部分

1. 血量100 / 100与弹药30 / 90，右边为什么不是同一种含义？
2. 初始弹药为什么在武器生成后读取？后续变化为什么由武器主动通知？
3. Weapon、Character、Widget三层分别知道哪份数据、哪个对象、哪些控件？
4. 为什么开始换弹时文字保持原样，完成后才变化？备用不足时能否显示15 / 0？
5. Die中的死亡标记、DisableMovement、ShowGameOver分别解决什么问题？
6. GameOverText为什么初始隐藏？它的提示文字能否代替输入映射？
7. GetCurrentLevelName的true与FName(*CurrentLevelName)中的*各是什么意思？
8. OpenLevel为什么能重新走角色、武器、波次初始化？有没有逐项修改旧对象？
9. 死亡入口阻止新射击请求，为什么不自动停止已经保存的按住状态？

把原来的血量和波次链复习后，再补两条：“武器扣弹药 / 完成换弹 → 玩家转发 → AmmoText”，“血量归零 → Die → ShowGameOver → 重开输入 → OpenLevel → 重新初始化”。每个箭头对应一处调用，能说清数据归谁和显示归谁，就能继续独立扩展。

# 设计文档：火箭中心点可视化

## 概述

本功能为火箭建造车间（Rocket_Builder）添加三个关键物理中心点的可视化功能：质心（Center of Mass）、升力中心（Center of Lift）和推力中心（Center of Thrust）。用户可以通过UI按钮独立控制每个中心点的显示/隐藏，帮助在设计阶段优化火箭的稳定性和性能。

### 设计目标

- 提供准确的物理中心点计算
- 实时响应火箭配置变化
- 清晰的3D可视化标记
- 直观的UI控制界面
- 高性能渲染（保持30+ FPS）

### 技术栈

- C++17
- OpenGL 3.3+ (现有渲染管线)
- 现有的Renderer3D和Renderer类
- 现有的BuilderState和RocketAssembly数据结构

## 架构

### 系统组件

```
┌─────────────────────────────────────────────────────────┐
│                   Rocket Builder UI                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ CoM Button   │  │ CoL Button   │  │ CoT Button   │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
│         │                 │                 │           │
│         └─────────────────┴─────────────────┘           │
│                           │                             │
└───────────────────────────┼─────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│              CenterVisualizationState                    │
│  ┌──────────────────────────────────────────────────┐   │
│  │ bool showCoM, showCoL, showCoT                   │   │
│  │ Vec3 comPos, colPos, cotPos                      │   │
│  │ bool hasCoM, hasCoL, hasCoT                      │   │
│  └──────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│              CenterCalculator                            │
│  ┌──────────────────────────────────────────────────┐   │
│  │ calculateCenterOfMass(assembly) -> Vec3          │   │
│  │ calculateCenterOfLift(assembly) -> Vec3          │   │
│  │ calculateCenterOfThrust(assembly) -> Vec3        │   │
│  └──────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│              CenterVisualizer                            │
│  ┌──────────────────────────────────────────────────┐   │
│  │ renderCenterMarker(renderer, pos, type)          │   │
│  │ renderCenterLabel(renderer, pos, text)           │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 数据流

1. **用户交互** → UI按钮点击
2. **状态更新** → CenterVisualizationState切换显示标志
3. **计算触发** → 当assembly变化时重新计算中心点位置
4. **渲染** → 根据显示标志渲染相应的3D标记

### 集成点

- **BuilderState**: 添加CenterVisualizationState成员
- **drawBuilderUI_KSP**: 添加三个切换按钮
- **builderHandleInput**: 处理按钮点击事件
- **main.cpp渲染循环**: 在3D场景中渲染中心点标记

## 组件和接口

### 1. CenterVisualizationState

存储可视化状态的数据结构。

```cpp
struct CenterVisualizationState {
    // 显示标志
    bool showCoM = false;
    bool showCoL = false;
    bool showCoT = false;
    
    // 计算出的位置（火箭局部坐标系，Y轴向上）
    Vec3 comPos = {0, 0, 0};
    Vec3 colPos = {0, 0, 0};
    Vec3 cotPos = {0, 0, 0};
    
    // 有效性标志（例如没有引擎时CoT无效）
    bool hasCoM = false;
    bool hasCoL = false;
    bool hasCoT = false;
    
    // 上次计算时的assembly哈希（用于检测变化）
    size_t lastAssemblyHash = 0;
};
```

### 2. CenterCalculator

计算各个中心点位置的静态函数集合。

```cpp
namespace CenterCalculator {
    // 计算质心
    // 输入：RocketAssembly（包含所有部件及其位置、质量）
    // 输出：质心位置（火箭局部坐标系）
    Vec3 calculateCenterOfMass(const RocketAssembly& assembly);
    
    // 计算升力中心
    // 输入：RocketAssembly（包含气动表面部件）
    // 输出：升力中心位置（火箭局部坐标系）
    Vec3 calculateCenterOfLift(const RocketAssembly& assembly);
    
    // 计算推力中心
    // 输入：RocketAssembly（包含引擎部件）
    // 输出：推力中心位置（火箭局部坐标系）
    Vec3 calculateCenterOfThrust(const RocketAssembly& assembly);
    
    // 计算assembly的哈希值（用于检测变化）
    size_t hashAssembly(const RocketAssembly& assembly);
}
```

### 3. CenterVisualizer

渲染中心点标记的函数集合。

```cpp
namespace CenterVisualizer {
    enum MarkerType {
        MARKER_COM,  // 质心：蓝色球体
        MARKER_COL,  // 升力中心：黄色锥体
        MARKER_COT   // 推力中心：红色圆柱
    };
    
    // 渲染中心点标记
    // 输入：3D渲染器、位置、标记类型、变换矩阵
    void renderMarker(Renderer3D* renderer, const Vec3& pos, 
                     MarkerType type, const Mat4& rocketTransform);
    
    // 渲染标签文字（使用2D overlay）
    // 输入：2D渲染器、3D位置、文字、视图投影矩阵
    void renderLabel(Renderer* renderer2d, const Vec3& worldPos, 
                    const char* text, const Mat4& viewProj);
}
```

### 4. UI按钮布局

在drawBuilderUI_KSP中添加按钮区域：

```cpp
// 位置：右侧面板下方，Assembly列表下方
// 布局：垂直排列三个按钮
// 按钮尺寸：宽度0.30，高度0.05
// 按钮间距：0.01

Button positions:
- CoM Button: (0.62, 0.05)
- CoL Button: (0.62, -0.01)
- CoT Button: (0.62, -0.07)
```

## 数据模型

### 物理计算模型

#### 质心计算

质心是所有部件质量加权位置的平均值：

```
CoM = Σ(mi * pi) / Σ(mi)

其中：
- mi = 第i个部件的质量（dry_mass + fuel_capacity）
- pi = 第i个部件的位置（stack_y + height/2）
```

#### 升力中心计算

升力中心基于气动表面的面积和位置：

```
CoL = Σ(Ai * pi) / Σ(Ai)

其中：
- Ai = 第i个气动部件的有效面积
- pi = 第i个气动部件的位置

气动部件识别：
- 鼻锥（CAT_NOSE_CONE）：基于直径计算横截面积
- 结构件中的尾翼（CAT_STRUCTURAL, id=16）：使用固定面积系数
```

#### 推力中心计算

推力中心基于引擎推力和位置：

```
CoT = Σ(Ti * pi) / Σ(Ti)

其中：
- Ti = 第i个引擎的推力
- pi = 第i个引擎的位置（stack_y + height/2）

引擎部件识别：
- 引擎类别（CAT_ENGINE）
- 助推器类别（CAT_BOOSTER）
```

### 可视化标记规格

| 中心类型 | 形状 | 颜色 | 尺寸 | 标签 |
|---------|------|------|------|------|
| 质心 (CoM) | 球体 | 蓝色 (0.2, 0.5, 1.0) | 半径0.3m | "CoM" |
| 升力中心 (CoL) | 锥体 | 黄色 (1.0, 0.8, 0.2) | 半径0.25m, 高0.5m | "CoL" |
| 推力中心 (CoT) | 圆柱 | 红色 (1.0, 0.2, 0.2) | 半径0.2m, 高0.4m | "CoT" |

### 状态持久化

按钮状态在BuilderState中保持，不需要额外的持久化机制。当用户返回建造车间时，按钮状态重置为默认（全部隐藏）。


## 正确性属性

*属性是一个特征或行为，应该在系统的所有有效执行中保持为真——本质上是关于系统应该做什么的形式化陈述。属性作为人类可读规范和机器可验证正确性保证之间的桥梁。*

### 属性 1：按钮切换行为

*对于任何*中心类型（质心、升力中心、推力中心）和任何初始显示状态，点击对应的切换按钮应该反转该中心标记的显示状态。

**验证需求：1.2, 2.2, 3.2**

### 属性 2：标记位置正确性

*对于任何*火箭配置和任何显示的中心标记，标记的渲染位置应该等于该中心类型的计算位置（在合理的浮点误差范围内）。

**验证需求：1.3, 2.3, 3.3**

### 属性 3：配置变化时位置更新

*对于任何*火箭配置变化（添加、删除或移动部件），如果中心标记正在显示，标记位置应该更新以反映新的计算位置。

**验证需求：1.4, 2.4, 3.4**

### 属性 4：按钮状态持久性

*对于任何*中心类型，如果用户将按钮切换到某个状态（显示或隐藏），该状态应该保持不变，直到用户再次点击该按钮。

**验证需求：1.5, 2.5, 3.5**

### 属性 5：质心计算正确性

*对于任何*包含至少一个部件的火箭配置，计算出的质心位置应该等于所有部件质量加权位置的平均值：CoM = Σ(mi * pi) / Σ(mi)，其中mi是部件质量（dry_mass + fuel_capacity），pi是部件中心位置（stack_y + height/2）。

**验证需求：6.1**

### 属性 6：升力中心计算正确性

*对于任何*包含气动表面的火箭配置，计算出的升力中心位置应该等于所有气动表面面积加权位置的平均值：CoL = Σ(Ai * pi) / Σ(Ai)，其中Ai是气动部件的有效面积，pi是部件中心位置。

**验证需求：6.2**

### 属性 7：推力中心计算正确性

*对于任何*包含引擎的火箭配置，计算出的推力中心位置应该等于所有引擎推力加权位置的平均值：CoT = Σ(Ti * pi) / Σ(Ti)，其中Ti是引擎推力，pi是引擎中心位置。

**验证需求：6.3**

### 属性 8：标记视觉区分

*对于任何*同时显示多个中心标记的场景，每个标记应该具有唯一的颜色和形状组合，使它们可以被区分。

**验证需求：4.1, 4.2**

### 属性 9：标记标签存在性

*对于任何*显示的中心标记，应该有对应的标签文字在标记附近渲染，标识中心类型（"CoM"、"CoL"或"CoT"）。

**验证需求：4.4**

### 属性 10：按钮可用性

*对于任何*火箭配置，如果火箭不包含某种中心类型所需的组件（例如没有引擎则无推力中心），对应的按钮应该被禁用或隐藏。

**验证需求：6.4**

### 属性 11：按钮视觉状态反馈

*对于任何*按钮，当它处于激活状态（对应标记正在显示）时，按钮的渲染应该与非激活状态有视觉上的区别（例如不同的颜色或高亮）。

**验证需求：5.3**

## 错误处理

### 计算错误

1. **空火箭配置**
   - 场景：用户尝试显示中心点，但火箭没有任何部件
   - 处理：禁用所有按钮，不显示任何标记
   - 用户反馈：按钮显示为灰色/禁用状态

2. **无效组件配置**
   - 场景：火箭没有引擎但用户尝试显示推力中心
   - 处理：推力中心按钮禁用，hasCoT标志为false
   - 用户反馈：按钮显示为灰色/禁用状态

3. **计算数值错误**
   - 场景：部件质量或位置数据异常（负值、NaN、无穷大）
   - 处理：跳过异常部件，使用有效部件计算
   - 日志：记录警告信息到控制台
   - 降级：如果所有部件都无效，禁用对应按钮

### 渲染错误

1. **标记重叠**
   - 场景：多个中心点位置非常接近
   - 处理：使用深度测试和透明度混合，确保所有标记可见
   - 视觉：较小的标记渲染在较大标记前面

2. **标记超出视野**
   - 场景：中心点位置在相机视锥体外
   - 处理：正常渲染，由OpenGL裁剪处理
   - 标签：只在标记在屏幕上时显示标签

3. **渲染资源不足**
   - 场景：Mesh创建失败或着色器编译失败
   - 处理：降级到简单的点渲染或禁用可视化
   - 日志：记录错误信息

### 输入错误

1. **快速连续点击**
   - 场景：用户快速多次点击同一按钮
   - 处理：使用防抖机制，忽略过快的重复点击
   - 阈值：最小点击间隔50ms

2. **鼠标位置异常**
   - 场景：鼠标坐标超出窗口范围
   - 处理：边界检查，忽略无效点击
   - 验证：在builderCheckHit中进行范围检查

## 测试策略

### 单元测试

单元测试专注于具体示例、边界情况和错误条件。

#### CenterCalculator测试

1. **质心计算示例**
   - 单个部件：质心应该在部件中心
   - 两个相同部件：质心应该在中点
   - 空火箭：应该返回原点或无效标志

2. **升力中心计算示例**
   - 单个鼻锥：升力中心在鼻锥位置
   - 鼻锥+尾翼：升力中心在加权平均位置
   - 无气动表面：应该返回无效标志

3. **推力中心计算示例**
   - 单个引擎：推力中心在引擎位置
   - 多个引擎：推力中心在加权平均位置
   - 无引擎：应该返回无效标志

4. **边界情况**
   - 零质量部件
   - 零推力引擎
   - 负值输入（应该被拒绝或处理）

#### CenterVisualizationState测试

1. **状态切换**
   - 初始状态：所有标志为false
   - 切换后：对应标志反转
   - 多次切换：状态正确交替

2. **位置更新**
   - assembly变化后：位置应该重新计算
   - 哈希检测：只在assembly真正变化时重新计算

#### UI交互测试

1. **按钮点击**
   - 点击CoM按钮：showCoM切换
   - 点击CoL按钮：showCoL切换
   - 点击CoT按钮：showCoT切换

2. **按钮禁用**
   - 无引擎时：CoT按钮禁用
   - 无气动表面时：CoL按钮禁用
   - 空火箭时：所有按钮禁用

### 基于属性的测试

基于属性的测试通过随机化验证所有输入的通用属性。

#### 测试配置

- 测试框架：使用C++的Catch2 + RapidCheck（或类似的PBT库）
- 每个属性测试：最少100次迭代
- 标签格式：`[Feature: rocket-center-visualization, Property N: <property_text>]`

#### 生成器

1. **火箭配置生成器**
   ```cpp
   // 生成随机的RocketAssembly
   // - 随机数量的部件（0-20）
   // - 随机部件类型（从PART_CATALOG中选择）
   // - 确保stack_y正确计算
   ```

2. **部件类型生成器**
   ```cpp
   // 生成特定类型的部件配置
   // - 只包含引擎的火箭
   // - 只包含气动表面的火箭
   // - 混合配置
   ```

3. **UI状态生成器**
   ```cpp
   // 生成随机的CenterVisualizationState
   // - 随机的显示标志组合
   // - 随机的位置值
   ```

#### 属性测试实现

1. **属性1：按钮切换行为**
   ```cpp
   // 对于任何中心类型和初始状态
   // 点击按钮后，状态应该反转
   PROPERTY("Button toggle inverts display state") {
       auto centerType = gen::element(CoM, CoL, CoT);
       auto initialState = gen::arbitrary<bool>();
       // 测试：toggle(centerType, initialState) == !initialState
   }
   ```

2. **属性5：质心计算正确性**
   ```cpp
   // 对于任何火箭配置
   // 计算的质心应该等于手动计算的质心
   PROPERTY("CoM calculation matches weighted average") {
       auto assembly = gen::arbitrary<RocketAssembly>();
       auto calculated = calculateCenterOfMass(assembly);
       auto expected = manualCoMCalculation(assembly);
       // 测试：abs(calculated.y - expected.y) < epsilon
   }
   ```

3. **属性3：配置变化时位置更新**
   ```cpp
   // 对于任何两个不同的火箭配置
   // 如果配置改变，位置应该重新计算
   PROPERTY("Position updates on configuration change") {
       auto assembly1 = gen::arbitrary<RocketAssembly>();
       auto assembly2 = gen::arbitrary<RocketAssembly>();
       // 假设assembly1 != assembly2
       auto pos1 = calculateCenterOfMass(assembly1);
       auto pos2 = calculateCenterOfMass(assembly2);
       // 测试：如果assembly1 != assembly2，则pos1可能 != pos2
       // （注意：不同配置可能偶然有相同的质心）
   }
   ```

### 集成测试

1. **完整工作流测试**
   - 启动建造车间
   - 添加部件
   - 点击按钮
   - 验证标记显示
   - 修改火箭
   - 验证标记更新

2. **性能测试**
   - 测量计算时间（应该 < 200ms）
   - 测量渲染帧率（应该 > 30 FPS）
   - 大型火箭配置（20+部件）

3. **视觉回归测试**
   - 截图对比
   - 标记颜色和形状验证
   - 标签位置验证

### 测试覆盖率目标

- 代码覆盖率：> 90%
- 分支覆盖率：> 85%
- 属性测试：所有11个属性
- 单元测试：所有公共函数
- 集成测试：所有用户工作流


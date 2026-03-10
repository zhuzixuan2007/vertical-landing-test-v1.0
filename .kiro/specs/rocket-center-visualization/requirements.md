# 需求文档

## 简介

本功能为火箭建造车间添加可视化显示功能，允许用户通过按钮控制显示火箭的质心、升力中心和推力中心。这些可视化标记将帮助用户在设计火箭时更好地理解和优化火箭的物理特性。

## 术语表

- **Rocket_Builder**: 火箭建造车间系统，用户在此设计和组装火箭
- **Center_of_Mass**: 质心，火箭所有质量的平均位置点
- **Center_of_Lift**: 升力中心，火箭所受升力的作用点
- **Center_of_Thrust**: 推力中心，火箭引擎推力的作用点
- **Visualization_Marker**: 可视化标记，用于在3D视图中显示中心点位置的图形元素
- **Toggle_Button**: 切换按钮，用于控制可视化标记显示/隐藏的UI控件

## 需求

### 需求 1: 质心可视化控制

**用户故事:** 作为火箭设计师，我想要通过按钮控制质心的显示，以便我能够查看火箭的质量分布情况。

#### 验收标准

1. THE Rocket_Builder SHALL 提供一个质心切换按钮
2. WHEN 用户点击质心切换按钮, THE Rocket_Builder SHALL 在3D视图中显示或隐藏质心标记
3. WHEN 质心标记显示时, THE Rocket_Builder SHALL 在火箭的质心位置渲染可视化标记
4. WHEN 火箭配置发生变化时, THE Rocket_Builder SHALL 实时更新质心标记的位置
5. THE Rocket_Builder SHALL 保持质心按钮的状态（显示/隐藏）直到用户再次切换

### 需求 2: 升力中心可视化控制

**用户故事:** 作为火箭设计师，我想要通过按钮控制升力中心的显示，以便我能够评估火箭的气动稳定性。

#### 验收标准

1. THE Rocket_Builder SHALL 提供一个升力中心切换按钮
2. WHEN 用户点击升力中心切换按钮, THE Rocket_Builder SHALL 在3D视图中显示或隐藏升力中心标记
3. WHEN 升力中心标记显示时, THE Rocket_Builder SHALL 在火箭的升力中心位置渲染可视化标记
4. WHEN 火箭配置发生变化时, THE Rocket_Builder SHALL 实时更新升力中心标记的位置
5. THE Rocket_Builder SHALL 保持升力中心按钮的状态（显示/隐藏）直到用户再次切换

### 需求 3: 推力中心可视化控制

**用户故事:** 作为火箭设计师，我想要通过按钮控制推力中心的显示，以便我能够检查推力方向和位置。

#### 验收标准

1. THE Rocket_Builder SHALL 提供一个推力中心切换按钮
2. WHEN 用户点击推力中心切换按钮, THE Rocket_Builder SHALL 在3D视图中显示或隐藏推力中心标记
3. WHEN 推力中心标记显示时, THE Rocket_Builder SHALL 在火箭的推力中心位置渲染可视化标记
4. WHEN 火箭配置发生变化时, THE Rocket_Builder SHALL 实时更新推力中心标记的位置
5. THE Rocket_Builder SHALL 保持推力中心按钮的状态（显示/隐藏）直到用户再次切换

### 需求 4: 可视化标记区分

**用户故事:** 作为火箭设计师，我想要不同的中心点使用不同的视觉样式，以便我能够轻松区分它们。

#### 验收标准

1. THE Rocket_Builder SHALL 使用不同的颜色渲染质心、升力中心和推力中心标记
2. THE Rocket_Builder SHALL 使用不同的形状或图标渲染质心、升力中心和推力中心标记
3. WHEN 多个中心标记同时显示时, THE Rocket_Builder SHALL 确保每个标记清晰可见且不相互遮挡
4. THE Rocket_Builder SHALL 在标记附近显示标签文字以标识中心类型

### 需求 5: 按钮UI布局

**用户故事:** 作为火箭设计师，我想要按钮布局清晰易用，以便我能够快速切换不同的可视化选项。

#### 验收标准

1. THE Rocket_Builder SHALL 在建造车间界面中显示三个切换按钮
2. THE Rocket_Builder SHALL 将按钮放置在易于访问的位置
3. WHEN 按钮处于激活状态时, THE Rocket_Builder SHALL 通过视觉反馈（如高亮或颜色变化）指示按钮状态
4. THE Rocket_Builder SHALL 在按钮上显示清晰的文字或图标标识其功能

### 需求 6: 中心点计算

**用户故事:** 作为火箭设计师，我想要系统准确计算各个中心点，以便我能够获得可靠的设计参考。

#### 验收标准

1. WHEN 火箭包含多个部件时, THE Rocket_Builder SHALL 基于所有部件的质量和位置计算质心
2. WHEN 火箭包含气动表面时, THE Rocket_Builder SHALL 基于气动表面的面积和位置计算升力中心
3. WHEN 火箭包含引擎时, THE Rocket_Builder SHALL 基于引擎的位置和推力计算推力中心
4. WHEN 火箭没有相应组件时, THE Rocket_Builder SHALL 禁用或隐藏对应的切换按钮

### 需求 7: 性能要求

**用户故事:** 作为火箭设计师，我想要可视化功能流畅运行，以便不影响我的设计工作流程。

#### 验收标准

1. WHEN 用户切换可视化标记时, THE Rocket_Builder SHALL 在100毫秒内完成显示或隐藏操作
2. WHEN 火箭配置变化时, THE Rocket_Builder SHALL 在200毫秒内重新计算并更新中心点位置
3. WHILE 可视化标记显示时, THE Rocket_Builder SHALL 保持至少30帧每秒的渲染性能

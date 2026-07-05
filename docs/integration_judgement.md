# R2 行为树接入判断记录

日期：2026-07-05

## 当前判断

`26RC_R2_behavior` 应是 R2 独立行为树包，而不是只保存 XML 的资源包。

当前策略：

1. PB2025 只作为模板参考，不作为运行依赖。
2. R2 包内维护自己的最小 BT 运行器和当前必需节点。
3. 不复制 PB2025 的裁判系统、自瞄、云台、RFID 等 R2 当前不用的插件。
4. 能用已有 R2/Sakiko ROS topic/action/service 表达的能力，优先复用接口。
5. 只有确认没有可复用接口时，才新增最小 BT 节点。
6. 感知、规划、机械臂控制细节不塞进行为树节点；行为树只编排任务级接口。

结构判断：

- 当前包按 PB2025 行为树工程结构制作，包括 `server/client`、`launch`、`params`、`behavior_trees`、`plugins`、`include`、`docs`、`test`。
- PB2025 是结构和接口风格参考，不是运行依赖。
- 当前 R2 运行器、XML、参数和插件节点均保存在 `26RC_R2_behavior` 内。

## 已实现独立内容

### R2 运行器

- `r2_behavior_server`
  - 加载 `behavior_trees/` 下的 XML。
  - 注册当前 R2 需要的 BT 节点。
  - 订阅 `manual_start`。
  - 按 `tick_frequency` 周期 tick 指定 `target_tree`。
- `r2_behavior_client`
  - 默认空闲，不自动发车。
  - 设置 `auto_start:=true` 时可重复发布 `manual_start`，用于避免启动 race condition。

### R2 BT 节点

- `IsManualStart`
  - 通过 `manual_start` `std_msgs/msg/Int32` 启动流程。
- `NavigateToNamedPose`
  - 调用 `/r2/navigation/navigate_to_named_pose` action。
  - 用于把行为树中的命名航点交给外部 R2 导航系统处理。
- `PubNav2Goal`
  - 向 `goal_pose` 发布 `geometry_msgs/msg/PoseStamped`。
  - 当前 `frame_id` 固定为 `map`。
- `PublishTwist`
  - 向 `cmd_vel` 发布 `geometry_msgs/msg/Twist`。
  - 当前主要用于停车。

### BehaviorTree.CPP 内置节点

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## 当前能实现的功能

当前行为树能实现：

- 独立启动 R2 行为树 server。
- 加载主树、dry-run 树或 full-flow 验证树。
- 通过 `manual_start` 手动启动。
- 通过 `NavigateToNamedPose` 调用命名航点导航 action。
- 发布导航目标占位点到 `goal_pose`。
- 发布零速度到 `cmd_vel`。
- 使用 `R2_Full_Flow_Verification` 验证完整比赛流程顺序。
- 用 `AlwaysFailure` 阻断未接入能力，避免实车误跑完整任务。

当前行为树不能直接完成：

- 健康检查。
- 急停与重试恢复。
- 端头识别、抓取、装配。
- R1 已进入梅林检测。
- KFS 分类和方块图构建。
- 梅林离散图规划和规则检查。
- 相邻 KFS 抓取。
- 吸盘命令和 payload 反馈闭环。
- 九宫格识别和中层放置。

## 当前文件结构状态

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   ├── r2_dry_run_with_mocks.xml
│   └── r2_full_flow_verification.xml
├── include/r2_behavior/
├── plugins/
│   ├── action/
│   └── condition/
├── src/
├── launch/
├── params/
├── docs/
├── test/
├── CMakeLists.txt
├── package.xml
└── README.md
```

## 未接入节点 / 目前未知节点

| 任务能力 | 当前处理方式 | 接入状态 |
|---|---|---|
| 健康检查 | `PreMatchSelfCheckPlaceholder` | 未接入 |
| 急停判断 | `IsEmergencyStopRequestedPlaceholder` | 未接入 |
| 重试恢复 | `IsRetryRequestedPlaceholder`、`ExecuteAreaAwareRetryPlaceholder` | 未接入 |
| 矛头识别与夹取 | `DetectAndPickSpearheadPlaceholder` | 未接入 |
| 武器装配 | `AssembleWeaponPlaceholder` | 未接入 |
| R1 进入密林判断 | `WaitUntilR1FullyEnteredMFPlaceholder` | 未接入 |
| 密林取 KFS | `CollectSingleR2KFSPlaceholder` | 未接入 |
| 退出密林 | `ExitForestViaBlock10_11_12Placeholder` | 未接入 |
| 战场中层放置 | `PlaceMiddleLayerPlaceholder` | 未接入 |
| KFS map 转换 | 暂无 BT 节点 | 未知节点等待接口确认 |
| 密林图规划 | 暂无 BT 节点 | 未知节点等待接口确认 |
| 规则检查 | 暂无 BT 节点 | 未知节点等待接口确认 |
| 吸盘反馈闭环 | 暂无 BT 节点 | 未知节点等待接口确认 |
| 九宫格识别和选格策略 | 暂无 BT 节点 | 未知节点等待接口确认 |

## 后续接入优先级

### 1. 航点与外部导航

目标：先让 R2 在梅林外稳定移动。

需要完成：

- 标定武馆端头架前、装配位、梅林入口、对抗区放置预备位。
- 将 XML 中所有 `0.0;0.0;0.0` 替换为实测坐标。
- 验证 `PubNav2Goal` 的 `frame_id=map` 与 `rc2026_navigation` 是否一致。

判断：

- 如果导航接收 `map` frame，继续使用当前 `PubNav2Goal`。
- 如果实车只稳定接收 `odom` frame，优先在导航侧或 TF 侧适配；不要先写一堆业务节点。

### 2. 启动、健康检查、急停

建议接口：

- `manual_start`：已接入。
- `/r2/health/status`：建议由外部节点汇总健康状态。
- `/r2/emergency_stop`：建议接成条件节点，同时保留底层独立急停链路。

### 3. 吸盘

现有接口：

- `/cmd_suction_suck` `std_msgs/msg/Bool`

当前判断：

- 当前 R2 包还没有 Bool publisher 节点。
- 不建议为了吸盘写大型插件。

优先路线：

1. 若可接受改吸盘侧接口，增加任务级 service/action，例如 `/r2/suction/set`。
2. 若必须保持 `/cmd_suction_suck`，再新增最小通用 Bool publisher BT 节点。
3. 同时补反馈：真空压力、吸附状态或视觉确认。

### 4. 端头与装配

建议接口：

- `/r2/manipulation/pick_tip`
- `/r2/manipulation/assemble_tip_to_pole`
- `/r2/perception/weapon_assembled`
- `/r2/perception/r1_assembly_ready`

行为树不应直接控制每个关节或视觉细节。

### 5. 梅林任务

建议接口：

- `/r2/perception/kfs_map`
- `/r2/forest/planner`
- `/r2/rule_guard/check_forest_plan`
- `/r2/nav/go_to_woods_block`
- `/r2/nav/follow_woods_graph_path`
- `/r2/manipulation/pick_adjacent_kfs`
- `/r2/perception/payload_state`

规划器必须理解方块编号、相邻关系、KFS 类型、假 KFS 禁止接触、10/11/12 出口规则。

### 6. 对抗区中层放置

建议接口：

- `/r2/perception/grid_state`
- `/r2/strategy/select_grid_cell`
- `/r2/manipulation/place_kfs_middle`
- `/r2/perception/payload_state`

当前默认只接中层放置。顶层放置和 R1 举升相关逻辑暂不接入主树。

## 新增代码准则

新增代码前必须先回答：

1. R2 现有节点是否已经能做？
2. Sakiko/现有 R2 包是否已有 topic/action/service 能直接复用？
3. 能否把能力做成外部任务级 action/service，而不是 BT 节点？
4. 如果必须新增 BT 节点，是否能做到通用、极小、无业务细节？

允许新增的低风险节点类型：

- 通用 Bool publisher。
- 通用 String/Int publisher。
- 通用 service/action wrapper。

不建议新增的节点类型：

- 内含复杂视觉逻辑的 BT 节点。
- 内含机械臂轨迹细节的 BT 节点。
- 内含梅林搜索/规则判断的大型 BT 节点。
- 第二套行为树框架。

## 后续调研检查清单

- [ ] 确认 `rc2026_navigation` 实车使用 `map` 还是 `odom` frame。
- [ ] 确认当前 `PubNav2Goal` 是否满足 R2 外部导航需求。
- [ ] 确认吸盘接口是否能从 topic 改为 service/action。
- [ ] 确认 KFS tracker 输出是否能稳定映射到 1-12 号方块。
- [ ] 确认端头识别与 KFS 识别是否共用相机/模型。
- [ ] 确认 MoveIt2 是否已有可复用的命名位姿或 action server。
- [ ] 确认实车急停链路是否独立于行为树。
- [ ] 确认规则 PDF 是否有更新版或裁判 FAQ。

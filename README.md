# r2_behavior

`r2_behavior` 是 ROBOCON 2026 R2 的行为树资源包，用来存放 R2 专用的行为树 XML、参数、launch 和调研记录。

本包遵循“尽量不新增核心代码”的原则：

- 不实现新的行为树执行器。
- 不新增 C++ BT 插件。
- 不修改 `pb2025_sentry_behavior` 源码。
- 运行时复用 `pb2025_sentry_behavior` 的 server、client 和 BT 插件。
- 无法接入的能力先保留 XML 占位和接口注释，等后续实车接口明确后逐步替换。

## 当前包结构

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   └── r2_dry_run_with_mocks.xml
├── docs/
│   ├── current_progress.md
│   └── integration_judgement.md
├── launch/
│   └── r2_behavior_launch.py
├── params/
│   ├── r2_behavior.yaml
│   └── r2_behavior_dry_run.yaml
├── CMakeLists.txt
├── package.xml
└── README.md
```

`build/`、`install/`、`log/` 是本地构建产物，已由 `.gitignore` 忽略。

## 复用内容

当前复用 PB2025 运行时：

- `pb2025_sentry_behavior_server`
- `pb2025_sentry_behavior_client`
- `pb2025_sentry_behavior/bt_plugins`

当前复用 PB2025 BT 节点：

| 节点 | 用途 | 当前接口 |
|---|---|---|
| `IsManualStart` | 手动启动比赛流程 | `manual_start` `std_msgs/msg/Int32` |
| `PubNav2Goal` | 发布导航目标 | `goal_pose` `geometry_msgs/msg/PoseStamped` |
| `PublishTwist` | 发布底盘速度，当前用于停车 | `cmd_vel` `geometry_msgs/msg/Twist` |

当前复用 BehaviorTree.CPP 内置节点：

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## 行为树说明

### `R2_Competition_Main`

主任务链路：

```text
PreMatchAndStart
-> MC_AssembleWeapon
-> WaitForR1BeforeR2LeavesMC
-> MF_CollectSingleR2KFS
-> MF_ExitForest
-> Battlefield_PlaceMiddleLayer
-> StopAllMotion
```

当前已接入的动作：

- 等待 `manual_start >= 1` 后进入主任务。
- 向 `goal_pose` 发布导航目标占位点。
- 向 `cmd_vel` 发布零速度停车。

当前故意阻断的部分：

- 端头识别 / 抓取 / 装配。
- R1 是否已进入梅林的检测。
- 梅林 KFS 感知、规则检查、规划、抓取和退出。
- 对抗区九宫格识别和中层放置。
- 急停与重试恢复。

这些未接入能力在 XML 中用 `AlwaysFailure` 占位，并用注释记录后续应接入的 ROS 接口。这样实车不会误跑尚未完成的任务链路。

### `R2_DryRun_WithMocks`

烟测树用于验证行为树加载和基础输出链路：

```text
AlwaysSuccess
-> AlwaysSuccess
-> PubNav2Goal
-> PublishTwist
```

它不会执行真实比赛任务，只用于确认：

- PB2025 server/client 能加载 R2 行为树。
- `goal_pose` 能发出。
- `cmd_vel` 能发出零速度。

## 当前能实现的功能

当前可以做到：

1. 使用 PB2025 行为树运行时加载 R2 XML。
2. 通过 `manual_start` 手动启动主树。
3. 发布外部导航目标占位点。
4. 发布停车速度。
5. 在未接入任务处主动失败，防止误执行。

当前不能直接做到：

1. 判断整车健康状态。
2. 执行端头抓取与兵器装配。
3. 判断 R1 已进入梅林。
4. 识别并分类 KFS 方块图。
5. 规划和执行梅林离散图动作。
6. 控制吸盘并确认 payload 状态。
7. 识别九宫格状态并完成中层放置。

## 实车部署流程

### 1. 准备 ROS 环境

先构建并 source `pb2025_sentry_behavior` 及其依赖。R2 行为树运行时依赖 PB2025 的 server、client 和插件。

然后构建本包：

```bash
source /opt/ros/humble/setup.bash
colcon --log-base 26RC_R2_behavior/log build \
  --base-paths 26RC_R2_behavior --packages-select r2_behavior \
  --build-base 26RC_R2_behavior/build \
  --install-base 26RC_R2_behavior/install \
  --cmake-clean-cache \
  --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3
source 26RC_R2_behavior/install/setup.bash
```

### 2. 启动底层节点

至少需要启动：

- 底盘驱动。
- TF / odom / 定位。
- 外部导航节点，例如 `rc2026_navigation`。
- 后续接入时需要的感知、机械臂、吸盘、KFS tracker 等节点。

### 3. 先跑烟测树

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_dry_run.yaml
```

检查：

- `goal_pose` 是否发布。
- 导航节点是否收到目标。
- `cmd_vel` 是否能收到零速度。
- PB2025 `PubNav2Goal` 固定使用的 `map` frame 是否与当前导航链路一致。

### 4. 再跑主树

```bash
ros2 launch r2_behavior r2_behavior_launch.py
```

启动主流程：

```bash
ros2 topic pub --once /manual_start std_msgs/msg/Int32 "{data: 1}"
```

预期现象：

- 主树通过 `IsManualStart` 后开始执行。
- 到未接入的占位节点时返回失败并停止对应分支。
- 不应直接完成完整比赛任务。

## 后续接入路线

### 1. 外部导航与航点

先完成梅林外移动：

- 标定武馆端头架前、装配位、梅林入口、对抗区放置预备位。
- 替换 XML 中所有 `0.0;0.0;0.0`。
- 确认 `goal_pose` 使用的 frame。

注意：PB2025 `PubNav2Goal` 当前固定 `frame_id="map"`。如果实车导航只接受 `odom`，优先考虑在导航侧或 TF 侧适配，不要急着新增 BT 节点。

### 2. 健康检查、启动、急停

建议先由外部节点汇总健康状态，再给行为树一个简单接口：

- `/r2/health/status`
- `/r2/emergency_stop`
- `/r2/retry/request`

急停不应只依赖行为树，底层硬件或控制链路也应能独立停车。

### 3. 吸盘

现有吸盘接口：

```text
/cmd_suction_suck std_msgs/msg/Bool
```

当前 PB2025 没有现成通用 Bool publisher BT 节点，所以还未直接接入吸盘。

推荐优先级：

1. 如果能改吸盘侧，增加任务级 service/action，例如 `/r2/suction/set`。
2. 如果必须保留 `/cmd_suction_suck`，再新增一个最小通用 Bool publisher BT 插件。
3. 同时补 payload 反馈，例如真空压力、吸附状态或视觉确认。

### 4. 端头与装配

不要把机械臂细节写进 BT。建议封装任务级 action：

- `/r2/manipulation/pick_tip`
- `/r2/manipulation/assemble_tip_to_pole`
- `/r2/perception/weapon_assembled`
- `/r2/perception/r1_assembly_ready`

行为树只负责调用和编排这些任务级接口。

### 5. 梅林任务

梅林内不应直接按普通连续导航处理。建议拆成：

- `/r2/perception/kfs_map`
- `/r2/forest/planner`
- `/r2/rule_guard/check_forest_plan`
- `/r2/nav/go_to_woods_block`
- `/r2/nav/follow_woods_graph_path`
- `/r2/manipulation/pick_adjacent_kfs`
- `/r2/perception/payload_state`

规划器必须理解：

- 1-12 号方块。
- 相邻关系。
- R2 KFS / R1 KFS / 假 KFS / 未知。
- 只能抓相邻方块。
- 必须经 10/11/12 离开。
- 不能触碰假 KFS 和 R1 KFS。

### 6. 对抗区放置

建议接口：

- `/r2/perception/grid_state`
- `/r2/strategy/select_grid_cell`
- `/r2/manipulation/place_kfs_middle`
- `/r2/perception/payload_state`

当前默认只接入中层放置。顶层放置和 R1 举升相关逻辑暂不进入主树。

## 新增代码原则

新增代码前先判断：

1. PB2025 是否已有节点能做？
2. Sakiko 或现有 R2 包是否已有 topic/action/service 能复用？
3. 能否把能力封装成外部任务级 action/service，而不是 BT 插件？
4. 如果必须新增 BT 插件，是否能做到通用、极小、无业务细节？

允许考虑的最小插件：

- 通用 Bool publisher。
- 通用 String / Int publisher。
- 通用 service/action wrapper。

不建议新增：

- 新行为树执行器。
- 内含视觉算法的 BT 节点。
- 内含机械臂轨迹细节的 BT 节点。
- 内含梅林搜索和规则判断的大型 BT 节点。

## 调研记录

当前判断、后续接入优先级和调研 checklist 已记录在：

```text
docs/integration_judgement.md
```

当前进度、已读材料、占位接口和已知缺口记录在：

```text
docs/current_progress.md
```

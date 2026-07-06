# r2_behavior

`r2_behavior` 是 ROBOCON 2026 R2 的行为树内容包，包含 R2 行为树运行器、XML 流程、启动文件、参数、插件节点和接入记录。

本包按照 PB2025 行为树工程结构编写，沿用了 PB2025 的目录组织、节点接口形状和命名习惯。当前策略是不执行端头夹取和兵器组装任务，主流程改为：武馆/梅林交界等待、2 分钟或 R1 ArUco 后进入梅林、执行树林 KFS 规划与地图维护、离开梅林后到对抗区配合 R1 放置 KFS。

## 当前结构

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   ├── r2_no_tip_competition_draft.xml
│   ├── r2_dry_run_with_mocks.xml
│   └── r2_full_flow_verification.xml
├── include/r2_behavior/
├── plugins/
│   ├── action/
│   │   ├── navigate_to_named_pose.cpp
│   │   ├── pub_nav2_goal.cpp
│   │   └── pub_twist.cpp
│   └── condition/
│       └── is_manual_start.cpp
├── src/
│   ├── r2_behavior_client.cpp
│   └── r2_behavior_server.cpp
├── launch/
├── params/
├── docs/
│   ├── current_progress.md
│   ├── important_files_line_notes.md
│   ├── integration_judgement.md
│   └── r2_no_tip_behavior_tree_design.md
├── action preparation/
│   ├── README.md
│   ├── r2_action_interface_catalog.md
│   ├── r2_sensor_observation_requirements.md
│   └── r2_behavior_action_sequence.md
└── test/
```

`build/`、`install/`、`log/` 和 `study/` 是本地构建/学习产物，已由 `.gitignore` 忽略。

## 赛事术语

| 统一术语 | 说明 |
|---|---|
| `武馆(MC)` | 比赛一区，R2 启动和进入梅林前等待的位置来源 |
| `梅林(MF)` | 比赛二区，包含树林、R1 通道、R2 入口区和 R2 出口区 |
| `树林` | 梅林中的 12 个高低不同方块区域；口头“梅花林”在文档中统一写作树林 |
| `对抗区` | 比赛三区，包含坡道、九宫格和重试区 |
| `端头` | 规则正式术语；当前 no-tip 主流程不执行端头夹取 |
| `KFS` | 武术秘籍，包括 `R1 KFS`、`R2 KFS` 和 `假KFS` |
| `R1 ArUco` | R1 上显示的 ArUco 指令码，用于继续进梅林、九宫格对齐、二/三层放置指令 |
| `九宫格` | 对抗区内放置 KFS 的 3x3 架子，分底层、中层、顶层 |

## 已接入 BT 节点

| 节点 | 类型 | ROS 接口 |
|---|---|---|
| `IsManualStart` | condition | 订阅 `/manual_start`，消息类型 `std_msgs/msg/Int32` |
| `NavigateToNamedPose` | action | 调用 `/r2/navigation/navigate_to_named_pose` |
| `PubNav2Goal` | action | 发布 `goal_pose`，消息类型 `geometry_msgs/msg/PoseStamped` |
| `PublishTwist` | action | 发布 `cmd_vel`，消息类型 `geometry_msgs/msg/Twist` |

当前仍直接使用 BehaviorTree.CPP 内置节点：`AlwaysSuccess`、`AlwaysFailure`、`Sequence`、`Fallback`、`ReactiveFallback`、`RetryUntilSuccessful`、`SubTree`。

## 当前行为树

### `R2_Competition_Main`

当前主树安全骨架：

```text
PreMatchAndStart
-> MC_WaitAtMFBoundary
-> MF_EnterForestEntry
-> MF_ExecuteForestPlan
-> Battlefield_ClimbAndPlaceKFS
-> StopAllMotion
```

主树已经改为 no-tip 流程。除 `IsManualStart`、`NavigateToNamedPose`、`PublishTwist` 外，其余实车能力仍以 `AlwaysFailure` 占位，避免未接入时误跑。

### `R2_NoTip_Competition_Main`

`r2_no_tip_competition_draft.xml` 是更详细的 no-tip 草稿树，保留接口语义、恢复分支和注释。它用于后续封装 wrapper 时对照，不应在占位节点未实现前作为实车目标树。

### `R2_Full_Flow_Verification`

完整流程验证树，使用 `AlwaysSuccess` mock 验证 no-tip 顺序：

```text
Flow_PreMatchAndStart_NoTip
-> Flow_MC_WaitAtMFBoundary
-> Flow_MF_EnterForestEntry
-> Flow_MF_ExecuteForestPlan
-> Flow_Battlefield_ClimbAndPlaceKFS
-> Flow_StopAllMotion
```

### `R2_DryRun_WithMocks`

最小烟测树，用于验证 server 加载和基础 topic 发布：

```text
AlwaysSuccess
-> AlwaysSuccess
-> PubNav2Goal
-> PublishTwist
```

## 后续必须接入的接口

### Topic

| topic | 用途 |
|---|---|
| `/r2/safety/emergency_stop_state` | 急停/软停状态 |
| `/r2/perception/r1_aruco_state` | R1 ArUco 可见性、id、instruction、位姿、时间戳 |
| `/r2/perception/kfs_map` | 统一 KFS 地图，融合网页、视觉和动作结果 |
| `/r2/forest/map_state` | 梅林地图维护状态机，记录 `removed_r2_blocks` 等 |
| `/r2/perception/payload_state` | 当前是否携带 R2 KFS |
| `/r2/chassis/wheel_height_state` | 四轮红外高度、丝杠圈数/电流 |
| `/r2/chassis/travel_mode_state` | `GROUND/FOREST/TRANSITION/RECOVERY` 状态 |
| `/r2/manipulation/suction_state` | 吸盘开关、真空压力、吸附状态 |
| `/r2/perception/grid_state` | 九宫格位姿、层数、格子占用 |

### Action

| action | 用途 |
|---|---|
| `/r2/chassis/set_travel_mode` | 普通地面/梅林/恢复模式切换 |
| `/r2/forest/execute_forest_plan` | 执行梅林规划得到的 `ForestSteps` |
| `/r2/forest/step_to_adjacent_block` | 树林相邻方块移动 |
| `/r2/manipulation/pick_adjacent_kfs` | 用机械臂吸盘抓取相邻 R2 KFS |
| `/r2/forest/place_payload_temporarily` | 必要时临时放下当前 R2 KFS |
| `/r2/forest/exit_via_block` | 经 10/11/12 号方块离开树林 |
| `/r2/motion/align_to_r1` | 对齐 R1，用于爬上 R1 或九宫格定位 |
| `/r2/motion/place_kfs_on_grid` | 根据 ArUco 指令放置二层或三层 KFS |
| `/r2/retry/execute_area_aware_retry` | 根据当前区域执行重试恢复 |

### Service

| service | 用途 |
|---|---|
| `/r2/health/check_start_ready` | 开始前检查导航、相机、底盘、吸盘、急停 |
| `/r2/setup/validate_manual_kfs_map` | 校验网页输入的 KFS 分布 |
| `/r2/forest/request_plan` | 调用梅林规划器，输入 KFS map，输出 `ForestSteps` |
| `/r2/perception/validate_kfs_on_block` | 机械臂深度相机单次确认目标方块 KFS 是否符合预期 |
| `/r2/forest/check_r1_kfs_removed` | 相机判断路径上的 R1 KFS 是否已移除 |
| `/r2/forest/update_map_state` | 每次移动/抓取/放置后更新地图状态机 |
| `/r2/rule_guard/check` | 检查动作是否违反比赛规则 |
| `/r2/perception/detect_grid_state` | 单次识别九宫格状态 |
| `/r2/strategy/select_grid_cell` | 根据九宫格状态和 ArUco 指令选择放置格 |

端头相关接口仍可保留在接口包中，但当前主树不接入 `/r2/manipulation/pick_tip`、`/r2/manipulation/assemble_tip_to_pole` 和 `/r2/perception/weapon_assembled`。

## 构建

所有构建产物应保存在 `26RC_R2_behavior` 内：

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

## 运行

烟测树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_dry_run.yaml
```

no-tip 完整流程验证树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_full_flow_verification.yaml
```

主树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py
```

手动启动：

```bash
ros2 topic pub --once /manual_start std_msgs/msg/Int32 "{data: 1}"
```

## 调研记录

- 当前进度：`docs/current_progress.md`
- 接入判断：`docs/integration_judgement.md`
- no-tip 设计：`docs/r2_no_tip_behavior_tree_design.md`
- 接口清单：`action preparation/r2_action_interface_catalog.md`
- 接入顺序：`action preparation/r2_behavior_action_sequence.md`

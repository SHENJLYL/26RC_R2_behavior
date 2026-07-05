# r2_behavior

`r2_behavior` 是 ROBOCON 2026 R2 的行为树内容包，包含 R2 行为树运行器、XML 流程、启动文件、参数、插件节点和接入记录。

本包按照 PB2025 行为树工程结构编写，沿用了 PB2025 的目录组织、节点接口形状和命名习惯，并根据 R2 任务流程保留当前需要的行为树节点。

## PB2025 参考声明

本包按 PB2025 行为树包的工程结构制作，主要采用了：

- `package.xml` 和 `CMakeLists.txt` 的组织方式。
- `behavior_server + behavior_client + launch + params + plugins` 的分层。
- BehaviorTree.CPP XML 加载方式。
- `IsManualStart`、`PubNav2Goal`、`PublishTwist` 这类小型通用节点的接口风格。
- 面向 R2 任务的命名航点导航、流程验证树和未接入节点占位记录。

当前文档将本行为树作为完整 R2 行为树内容说明：重点描述文件结构、已接入节点、未接入节点、未知待接入节点和后续接入方式。

## 当前结构

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   ├── r2_dry_run_with_mocks.xml
│   └── r2_full_flow_verification.xml
├── include/r2_behavior/
│   ├── custom_types.hpp
│   ├── r2_behavior_client.hpp
│   ├── r2_behavior_server.hpp
│   └── plugins/
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
│   └── r2_behavior_launch.py
├── params/
│   ├── r2_behavior.yaml
│   ├── r2_behavior_competition_auto_start.yaml
│   ├── r2_behavior_dry_run.yaml
│   └── r2_behavior_full_flow_verification.yaml
├── docs/
│   ├── current_progress.md
│   ├── important_files_line_notes.md
│   └── integration_judgement.md
├── test/
├── CMakeLists.txt
├── package.xml
└── README.md
```

`build/`、`install/`、`log/` 是本地构建产物，已由 `.gitignore` 忽略。

## 赛事术语统一

本文档采用规则 PDF 中的正式术语：

| 统一术语 | 说明 |
|---|---|
| `武馆(MC)` | 比赛一区，机器人启动、重试和组装兵器的区域 |
| `梅林(MF)` | 比赛二区，包含 `树林`、`R1 通道`、`R2 入口区` 和 `R2 出口区` |
| `树林` | 梅林中的 12 个高低不同方块区域，只允许 R2 运行 |
| `对抗区` | 比赛三区，包含坡道、九宫格和重试区 |
| `端头` | 规则 V6 中使用的正式名称，不再写作“矛头” |
| `端头架(SHR)` | 武馆中放置端头的架子 |
| `兵器` | 由长杆、端头和快速接头组成 |
| `KFS` | 武术秘籍，包括 `R1 KFS`、`R2 KFS` 和 `假KFS` |
| `九宫格` | 对抗区内放置 KFS 的 3x3 架子，分底层、中层、顶层 |

代码标识中的历史英文命名，例如 `spearhead_rack_standoff`，暂时作为接口名保留；中文描述统一使用 `端头` 和 `端头架`。

## 行为树运行内容

当前本包提供两个可执行文件：

| 可执行文件 | 作用 |
|---|---|
| `r2_behavior_server` | 加载 `behavior_trees/` 下的 XML，注册 R2 当前需要的 BT 节点，并按 `tick_frequency` 周期执行目标树 |
| `r2_behavior_client` | 轻量辅助节点；默认空闲，不自动发车；设置 `auto_start:=true` 时可重复发布 `manual_start`，避免启动时 server 尚未订阅导致丢消息 |

当前本包内置四个 R2 BT 节点：

| 节点 | 类型 | ROS 接口 |
|---|---|---|
| `IsManualStart` | condition | 订阅 `manual_start`，消息类型 `std_msgs/msg/Int32` |
| `NavigateToNamedPose` | action | 调用 `/r2/navigation/navigate_to_named_pose`，用于按名称调用 R2 导航航点 |
| `PubNav2Goal` | action | 发布 `goal_pose`，消息类型 `geometry_msgs/msg/PoseStamped` |
| `PublishTwist` | action | 发布 `cmd_vel`，消息类型 `geometry_msgs/msg/Twist` |

当前仍直接使用 BehaviorTree.CPP 内置节点：

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## 当前文件状态

### 已接入或可运行

| 文件/能力 | 当前状态 |
|---|---|
| `src/r2_behavior_server.cpp` | 已接入，负责加载 XML、注册 BT 节点、周期 tick 行为树 |
| `src/r2_behavior_client.cpp` | 已接入，支持手动启动和自动重复发布 `/manual_start` |
| `behavior_trees/r2_competition_main.xml` | 已建立正式比赛骨架，未接入任务处保留失败占位 |
| `behavior_trees/r2_full_flow_verification.xml` | 已建立流程验证树，未接入任务使用 mock 成功节点以验证整体顺序 |
| `behavior_trees/r2_dry_run_with_mocks.xml` | 已建立最小烟测树，用于验证加载和基础 topic 发布 |
| `IsManualStart` | 已接入，读取 `/manual_start` |
| `NavigateToNamedPose` | 已接入，调用 R2 命名航点导航 action |
| `PubNav2Goal` | 已接入，发布 Nav2 goal topic |
| `PublishTwist` | 已接入，发布底盘速度，当前主要用于停车 |

### 未接入节点 / 目前未知节点，等待后续接入

| 比赛能力 | 当前 XML 状态 | 预计接口或待确认接口 |
|---|---|---|
| 健康检查 | `PreMatchSelfCheckPlaceholder` | `/r2/health/status`、TF、传感器新鲜度 |
| 急停判断 | `IsEmergencyStopRequestedPlaceholder` | `/r2/emergency_stop`，同时保留底层急停链路 |
| 重试恢复 | `IsRetryRequestedPlaceholder`、`ExecuteAreaAwareRetryPlaceholder` | `/r2/retry/request`、`/r2/retry/execute` |
| 端头识别与夹取 | `DetectAndPickSpearheadPlaceholder` | `/r2/manipulation/pick_tip` |
| 兵器组装 | `AssembleWeaponPlaceholder` | `/r2/manipulation/assemble_tip_to_pole`、`/r2/perception/weapon_assembled` |
| R1 是否已进入梅林 | `WaitUntilR1FullyEnteredMFPlaceholder` | `/r2/perception/r1_entered_mf`，避免依赖 R1-R2 无线通信 |
| 进入梅林 / 退出树林 | `CollectSingleR2KFSPlaceholder`、`ExitForestViaBlock10_11_12Placeholder` | `/r2/forest/enter`、`/r2/forest/exit` |
| KFS 检测与地图 | 未知节点待接入 | `/kfs_tracker/detection -> /r2/perception/kfs_map` |
| 树林方块规划与规则检查 | 未知节点待接入 | `/r2/forest/planner`、`/r2/rule_guard/check_forest_plan` |
| 树林方块图导航 | 未知节点待接入 | `/r2/nav/go_to_woods_block`、`/r2/nav/follow_woods_graph_path` |
| 相邻 KFS 夹取 | 未知节点待接入 | `/r2/manipulation/pick_adjacent_kfs` |
| 吸盘与载荷反馈 | 未知节点待接入 | `/cmd_suction_suck`、`/r2/perception/payload_state` |
| 九宫格识别和选格 | 未知节点待接入 | `/r2/perception/grid_state`、`/r2/strategy/select_grid_cell` |
| 中层放置 | `PlaceMiddleLayerPlaceholder` | `/r2/manipulation/place_kfs_middle` |

## 当前能实现的功能

当前可以做到：

1. 启动 R2 行为树 server。
2. 加载 `R2_Competition_Main`、`R2_DryRun_WithMocks` 和 `R2_Full_Flow_Verification`。
3. 通过 `manual_start >= 1` 启动主任务。
4. 通过 `NavigateToNamedPose` 调用 R2 命名航点导航 action。
5. 发布导航目标到 `goal_pose`，当前 frame 固定为 `map`。
6. 发布零速度到 `cmd_vel`，用于停车。
7. 用 `R2_Full_Flow_Verification` 验证完整比赛流程顺序。
8. 在正式主树未接入任务处用 `AlwaysFailure` 占位，避免误跑完整实车任务。

当前不能直接做到：

1. 健康检查和急停闭环。
2. 端头识别、抓取、兵器组装。
3. 判断 R1 已进入梅林。
4. KFS 分类、树林方块图规划和规则检查。
5. 吸盘控制与 payload 反馈。
6. 九宫格识别和中层放置动作。

这些能力已经在 XML 注释和文档中保留占位，后续接入实车接口时逐步替换。

## 行为树

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

### `R2_DryRun_WithMocks`

烟测树用于确认行为树运行器和基础发布链路：

```text
AlwaysSuccess
-> AlwaysSuccess
-> PubNav2Goal
-> PublishTwist
```

### `R2_Full_Flow_Verification`

完整流程验证树用于在导航、机械臂、视觉等接口尚未全部接入时验证比赛流程顺序：

```text
Flow_PreMatchAndStart
-> Flow_MC_AssembleWeapon
-> Flow_WaitForR1BeforeR2LeavesMC
-> Flow_MF_CollectSingleR2KFS
-> Flow_MF_ExitForest
-> Flow_Battlefield_PlaceMiddleLayer
-> Flow_StopAllMotion
```

该树中的未接入能力使用 `AlwaysSuccess` mock，适合验证“流程能否走到底”；正式比赛主树仍然使用失败占位保护实车。

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

当前环境构建时可能出现 Anaconda 与系统 ncurses 的 runtime search path warning；只要 `Summary: 1 package finished`，包已经生成。

## 运行

先跑烟测树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_dry_run.yaml
```

验证完整流程顺序：

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_full_flow_verification.yaml
```

再跑主树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py
```

手动启动主任务：

```bash
ros2 topic pub --once /manual_start std_msgs/msg/Int32 "{data: 1}"
```

默认 `r2_behavior_client` 不会自动发布启动信号，避免 launch 后实车误启动。

## 实车部署建议

先启动底层系统：

- 底盘驱动。
- TF / odom / 定位。
- 外部导航节点，例如 `rc2026_navigation`。
- 后续接入时需要的视觉、机械臂、吸盘、KFS tracker 等节点。

烟测时检查：

- `goal_pose` 是否发布。
- 导航节点是否收到目标。
- `cmd_vel` 是否收到零速度。
- `map` frame 是否与实车导航链路一致。

## 后续接入路线

新增能力优先做成外部任务级 topic/service/action，再由行为树调用。不要把视觉算法、机械臂轨迹、树林方块搜索规则直接塞进 BT 节点。

建议接入顺序：

1. 标定外部导航航点，替换 XML 中的 `0.0;0.0;0.0`。
2. 接入健康检查和急停状态。
3. 接入端头抓取与兵器组装 action。
4. 接入 KFS map、树林方块 planner、规则检查和相邻抓取 action。
5. 接入吸盘控制与 payload 反馈。
6. 接入九宫格识别和中层放置 action。

如果确实需要新增 BT 节点，应保持“通用、极小、无业务细节”。例如通用 Bool publisher、通用 service/action wrapper。不要新增第二套行为树框架。

## 调研记录

- 当前进度：`docs/current_progress.md`
- 接入判断和调研 checklist：`docs/integration_judgement.md`

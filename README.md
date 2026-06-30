# r2_behavior

`r2_behavior` 是 ROBOCON 2026 R2 的独立行为树包。它现在不再依赖 `pb2025_sentry_behavior` 运行，而是在本包内提供自己的最小行为树运行器、R2 行为树 XML、启动文件、参数和接入记录。

PB2025 只作为模板参考：当前代码沿用了 PB2025 的节点接口形状和命名习惯，但运行时、可执行文件和节点实现都在 `r2_behavior` 内。

## 当前结构

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   └── r2_dry_run_with_mocks.xml
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

`build/`、`install/`、`log/` 是本地构建产物，已由 `.gitignore` 忽略。

## 独立运行内容

当前本包提供两个可执行文件：

| 可执行文件 | 作用 |
|---|---|
| `r2_behavior_server` | 加载 `behavior_trees/` 下的 XML，注册 R2 当前需要的 BT 节点，并按 `tick_frequency` 周期执行目标树 |
| `r2_behavior_client` | 轻量辅助节点；默认空闲，不自动发车；设置 `auto_start:=true` 时可发布一次 `manual_start` |

当前本包内置三个 R2 BT 节点：

| 节点 | 类型 | ROS 接口 |
|---|---|---|
| `IsManualStart` | condition | 订阅 `manual_start`，消息类型 `std_msgs/msg/Int32` |
| `PubNav2Goal` | action | 发布 `goal_pose`，消息类型 `geometry_msgs/msg/PoseStamped` |
| `PublishTwist` | action | 发布 `cmd_vel`，消息类型 `geometry_msgs/msg/Twist` |

当前仍直接使用 BehaviorTree.CPP 内置节点：

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## 当前能实现的功能

当前可以做到：

1. 不依赖 PB2025，直接启动 R2 自己的行为树 server。
2. 加载 `R2_Competition_Main` 和 `R2_DryRun_WithMocks`。
3. 通过 `manual_start >= 1` 启动主任务。
4. 发布导航目标到 `goal_pose`，当前 frame 固定为 `map`。
5. 发布零速度到 `cmd_vel`，用于停车。
6. 在未接入任务处用 `AlwaysFailure` 占位，避免误跑完整实车任务。

当前不能直接做到：

1. 健康检查和急停闭环。
2. 端头识别、抓取、装配。
3. 判断 R1 已进入梅林。
4. KFS 分类、梅林离散图规划和规则检查。
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

烟测树用于确认独立运行器和基础发布链路：

```text
AlwaysSuccess
-> AlwaysSuccess
-> PubNav2Goal
-> PublishTwist
```

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

新增能力优先做成外部任务级 topic/service/action，再由行为树调用。不要把视觉算法、机械臂轨迹、梅林搜索规则直接塞进 BT 节点。

建议接入顺序：

1. 标定外部导航航点，替换 XML 中的 `0.0;0.0;0.0`。
2. 接入健康检查和急停状态。
3. 接入端头抓取与装配 action。
4. 接入 KFS map、梅林 planner、规则检查和相邻抓取 action。
5. 接入吸盘控制与 payload 反馈。
6. 接入九宫格识别和中层放置 action。

如果确实需要新增 BT 节点，应保持“通用、极小、无业务细节”。例如通用 Bool publisher、通用 service/action wrapper。不要新增第二套行为树框架。

## 调研记录

- 当前进度：`docs/current_progress.md`
- 接入判断和调研 checklist：`docs/integration_judgement.md`

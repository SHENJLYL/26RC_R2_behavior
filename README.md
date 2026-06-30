# r2_behavior

`r2_behavior` 是 ROBOCON 2026 R2 的行为树资源包。

本包遵循“尽量不新增核心代码”的原则：不实现新的行为树执行器，不新增 C++ BT 插件，运行时复用 `pb2025_sentry_behavior` 已有的 server、client 和 BT 插件。`26RC_R2_behavior` 中主要保存 R2 专用的 XML 行为树、参数、launch 和进度记录。

## 复用策略

当前复用内容：

- `pb2025_sentry_behavior_server`
- `pb2025_sentry_behavior_client`
- `pb2025_sentry_behavior/bt_plugins`
- PB2025 已有 BT 节点：
  - `IsManualStart`
  - `PubNav2Goal`
  - `PublishTwist`
- BehaviorTree.CPP 内置节点：
  - `AlwaysSuccess`
  - `AlwaysFailure`
  - `Sequence`
  - `ReactiveFallback`
  - `SubTree`

当前不新增：

- 不新增 `src/` 核心执行器代码
- 不新增 `include/` C++ 头文件
- 不新增自定义 BT 插件
- 不修改 `pb2025_sentry_behavior` 源码

## 行为树

- `R2_Competition_Main`
  - R2 完整任务主树。
  - 覆盖赛前检查、武馆端头与装配、等待 R1 进入梅林、梅林单 KFS 收集、经 10/11/12 出口离开、对抗区中层放置和停车。
  - 已有接口用 PB2025 节点表达。
  - 未接入接口用 `AlwaysFailure` 占位，并在 XML 注释中记录后续应接入的 ROS 接口。

- `R2_DryRun_WithMocks`
  - 最小烟测树。
  - 只复用 PB2025 的 `PubNav2Goal` 和 `PublishTwist`，用于验证行为树加载和基础输出链路。

## 当前已复用接口

| 能力 | ROS 接口 | 复用节点 |
|---|---|---|
| 手动启动 | `manual_start` `std_msgs/msg/Int32` | `IsManualStart` |
| 梅林外导航目标 | `goal_pose` `geometry_msgs/msg/PoseStamped` | `PubNav2Goal` |
| 底盘停车 | `cmd_vel` `geometry_msgs/msg/Twist` | `PublishTwist` |

说明：`r2_suction_control` 使用 `/cmd_suction_suck` `std_msgs/msg/Bool`，PB2025 当前没有可直接复用的 Bool publisher 节点，因此吸盘命令暂时只在 XML 中保留为后续接口占位，不新增 C++ 节点。

## 保留的占位接口

以下能力已经在 `R2_Competition_Main` 中保留位置，但还需要后续实车接口或复用节点：

- 健康检查与 TF / 传感器状态检查
- 急停与重试输入
- 端头识别、抓取和装配
- R1 是否已进入梅林的检测
- KFS 方块图感知与分类
- 梅林离散图规划与规则检查
- 相邻 KFS 抓取
- 吸盘 Bool 命令与 payload 吸附 / 掉落反馈
- 九宫格状态识别
- 中层放置动作与放置验证

## 构建

所有构建产物都应保存在 `26RC_R2_behavior` 文件夹内，不要生成到桌面根目录。

```bash
source /opt/ros/humble/setup.bash
colcon --log-base 26RC_R2_behavior/log build \
  --base-paths 26RC_R2_behavior --packages-select r2_behavior \
  --build-base 26RC_R2_behavior/build \
  --install-base 26RC_R2_behavior/install \
  --cmake-clean-cache \
  --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3
```

构建后加载环境：

```bash
source 26RC_R2_behavior/install/setup.bash
```

## 运行

先确保 `pb2025_sentry_behavior` 及其依赖已经在同一 ROS 2 环境中构建并 source。

运行主行为树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py
```

运行烟测树：

```bash
ros2 launch r2_behavior r2_behavior_launch.py \
  params_file:=26RC_R2_behavior/install/r2_behavior/share/r2_behavior/params/r2_behavior_dry_run.yaml
```

## 进度记录

当前进度、已阅读文件、未纳入文件、已接入接口和占位接口记录在：

```text
docs/current_progress.md
```

后续接入实车能力时，优先复用已有包的 topic/action/service 或 PB2025 节点；只有确认没有可复用接口时，才考虑新增最小 BT 插件。

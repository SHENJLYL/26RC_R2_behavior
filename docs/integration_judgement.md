# R2 行为树接入判断记录

日期：2026-06-30

## 当前判断

`26RC_R2_behavior` 应保持为 R2 行为树资源包，而不是新的核心行为树框架包。

当前最合适的策略是：

1. 复用 `pb2025_sentry_behavior` 的执行器、client 和 BT 插件。
2. R2 包内只维护 XML 行为树、参数、launch、文档和接入记录。
3. 能用已有 ROS topic/action/service 表达的能力，优先复用接口。
4. 能用 PB2025 已有 BT 节点表达的能力，优先复用节点。
5. 只有确认没有可复用节点时，才新增最小 BT 插件。
6. 不把感知、规划、机械臂控制细节塞进行为树节点；行为树只编排任务级接口。

## 已确认可复用内容

### PB2025 运行时

- `pb2025_sentry_behavior_server`
- `pb2025_sentry_behavior_client`
- `pb2025_sentry_behavior/bt_plugins`

### PB2025 BT 节点

- `IsManualStart`
  - 用于从 `manual_start` `std_msgs/msg/Int32` 启动流程。
- `PubNav2Goal`
  - 用于向 `goal_pose` 发布 `geometry_msgs/msg/PoseStamped`。
  - 注意：PB2025 实现中 `frame_id` 固定为 `map`。
- `PublishTwist`
  - 用于向 `cmd_vel` 发布 `geometry_msgs/msg/Twist`。
  - 当前主要用于停车。

### BehaviorTree.CPP 内置节点

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## 当前能实现的功能

当前行为树能实现：

- 通过 `manual_start` 手动启动。
- 发布导航目标占位点到 `goal_pose`。
- 发布零速度到 `cmd_vel` 实现停车。
- 用 `AlwaysFailure` 明确阻断未接入能力，避免实车误跑完整任务。

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

## 后续接入优先级

### 1. 航点与外部导航

目标：先让 R2 在梅林外稳定移动。

需要完成：

- 标定武馆端头架前、装配位、梅林入口、对抗区放置预备位。
- 将 XML 中所有 `0.0;0.0;0.0` 替换为实测坐标。
- 验证 `PubNav2Goal` 的 `frame_id=map` 与 `rc2026_navigation` 是否一致。

判断：

- 如果 `rc2026_navigation` 接收 `map` frame，则继续复用 `PubNav2Goal`。
- 如果实车只稳定接收 `odom` frame，应优先修改导航侧或增加配置适配；不要先写新的 BT 节点。

### 2. 启动、健康检查、急停

目标：保证实车安全启动和安全停止。

建议接口：

- `manual_start`：继续复用 `IsManualStart`。
- `/r2/health/status`：建议封装为任务级状态 topic 或 service。
- `/r2/emergency_stop`：建议接成条件节点或外部安全层直接切断运动。

判断：

- 健康检查可以先由外部节点汇总，再给行为树一个简单条件。
- 急停不应只依赖行为树，底层硬件/控制链路也要能独立停止。

### 3. 吸盘

现有接口：

- `/cmd_suction_suck` `std_msgs/msg/Bool`

当前判断：

- PB2025 没有现成的通用 `std_msgs/msg/Bool` publisher BT 节点。
- 不建议为了吸盘立刻写一套大插件。

优先路线：

1. 若可接受改吸盘侧接口，给 `r2_suction_control` 增加任务级 service/action，例如 `/r2/suction/set`。
2. 若必须保持 `/cmd_suction_suck`，再新增一个最小通用 Bool publisher BT 插件。
3. 同时补反馈：真空压力、吸附状态或视觉确认，否则行为树无法判断抓取成功。

### 4. 端头与装配

目标：把复杂机械臂/视觉逻辑藏在任务级 action 后面。

建议接口：

- `/r2/manipulation/pick_tip`
- `/r2/manipulation/assemble_tip_to_pole`
- `/r2/perception/weapon_assembled`
- `/r2/perception/r1_assembly_ready`

判断：

- 行为树不应直接控制每个关节或视觉细节。
- 每个 action 应返回明确结果：成功、可重试失败、规则阻断、致命失败。

### 5. 梅林任务

目标：从连续导航切换到离散规则任务。

建议接口：

- `/r2/perception/kfs_map`
- `/r2/forest/planner`
- `/r2/rule_guard/check_forest_plan`
- `/r2/nav/go_to_woods_block`
- `/r2/nav/follow_woods_graph_path`
- `/r2/manipulation/pick_adjacent_kfs`
- `/r2/perception/payload_state`

判断：

- 梅林内不能直接当普通 Nav2 连续空间导航。
- 规划器必须理解方块编号、相邻关系、KFS 类型、假 KFS 禁止接触、10/11/12 出口规则。
- 行为树只调用“进入、抓取、退出”等任务级 action。

### 6. 对抗区中层放置

建议接口：

- `/r2/perception/grid_state`
- `/r2/strategy/select_grid_cell`
- `/r2/manipulation/place_kfs_middle`
- `/r2/perception/payload_state`

判断：

- R2 默认只做中层放置。
- 顶层放置和 R1 举升相关逻辑暂不接入主树。

## 新增代码准则

新增代码前必须先回答：

1. PB2025 是否已有节点能做？
2. Sakiko/现有 R2 包是否已有 topic/action/service 能直接复用？
3. 能否把能力做成外部任务级 action/service，而不是 BT 插件？
4. 如果必须新增 BT 插件，是否能做到通用、极小、无业务细节？

允许新增的低风险插件类型：

- 通用 Bool publisher。
- 通用 String/Int publisher。
- 通用 service/action wrapper。

不建议新增的插件类型：

- 内含复杂视觉逻辑的 BT 节点。
- 内含机械臂轨迹细节的 BT 节点。
- 内含梅林搜索/规则判断的大型 BT 节点。
- 新的行为树执行器。

## 后续调研检查清单

- [ ] 确认 `rc2026_navigation` 实车使用 `map` 还是 `odom` frame。
- [ ] 确认 PB2025 `PubNav2Goal` 是否满足 R2 外部导航需求。
- [ ] 确认吸盘接口是否能从 topic 改为 service/action。
- [ ] 确认 KFS tracker 输出是否能稳定映射到 1-12 号方块。
- [ ] 确认端头识别与 KFS 识别是否共用相机/模型。
- [ ] 确认 MoveIt2 是否已有可复用的命名位姿或 action server。
- [ ] 确认实车急停链路是否独立于行为树。
- [ ] 确认规则 PDF 是否有更新版或裁判 FAQ。

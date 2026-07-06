# R2 行为树接入判断记录

日期：2026-07-06

## 当前判断

`26RC_R2_behavior` 是 R2 行为树内容包，而不是只保存 XML 的资源包。当前主线是 no-tip 行为树：不执行端头夹取和兵器组装，优先实现梅林 KFS 与对抗区放置流程。

当前策略：

1. 按 PB2025 行为树工程结构编写。
2. R2 包内维护自己的最小 BT 运行器和当前必需节点。
3. 当前主树不进入端头夹取、兵器组装、裁判系统、自瞄、云台、RFID 等无关能力。
4. 能用已有 R2 ROS topic/action/service 表达的能力，优先复用接口。
5. 只有确认没有可复用接口时，才新增最小 BT wrapper。
6. 感知、规划、机械臂控制、底盘找平细节不塞进行为树节点；行为树只编排任务级接口。

## 已确认的 no-tip 任务约束

| 项 | 当前结论 |
|---|---|
| 端头/兵器 | 当前不执行，不进入主树 |
| 入梅林等待时间 | `120.0` 秒，即 2 分钟 |
| R1 指令 | 使用 ArUco，不按非 ArUco 码设计 |
| 梅林入口 | PDF 标注的 2 号平台面前，即 `forest_entry_block_2_standoff` |
| KFS 初始分布 | 来自 web_spoiler `/kfs_locator/state` |
| 梅林规划 | 使用 `meilin_router/router_planner`，后续需封装为 service/action |
| KFS 复核 | `ValidateKFSOnBlock` 用机械臂深度相机做单次 service |
| R1 KFS 移除判断 | 用相机判断 |
| 爬升/下降确认 | 丝杠圈数/电流、四轮红外、激光雷达 |
| 地图维护 | 必须记录 `removed_r2_blocks`、`current_kfs_map`、`current_block`、`payload_state` |
| 对抗区 | 等 R1 手操对齐 ArUco，R2 爬上 R1，再按 ArUco code 放二层或三层 |

## 当前已实现 BT 内容

- `r2_behavior_server`
  - 加载 `behavior_trees/` 下 XML。
  - 注册当前 R2 需要的 BT 节点。
  - 订阅 `/manual_start`。
  - 按 `tick_frequency` 周期 tick 指定 `target_tree`。
- `r2_behavior_client`
  - 默认空闲，不自动发车。
  - 设置 `auto_start:=true` 时可重复发布 `/manual_start`。
- `IsManualStart`
- `NavigateToNamedPose`
- `PubNav2Goal`
- `PublishTwist`

## 当前行为树能实现

- 启动 R2 行为树 server。
- 加载 no-tip 主树、安全草稿树、dry-run 树或 full-flow 验证树。
- 通过 `/manual_start` 手动启动。
- 通过 `NavigateToNamedPose` 调用命名航点导航 action。
- 发布导航目标到 `goal_pose`。
- 发布零速度到 `cmd_vel`。
- 用 `R2_Full_Flow_Verification` 验证 no-tip 流程顺序。
- 用 `AlwaysFailure` 阻断未接入能力，避免实车误跑完整任务。

## 当前不能直接完成

- 健康检查和急停闭环。
- 120 秒比赛计时或 ArUco 继续指令 wrapper。
- web KFS map 到统一 `/r2/perception/kfs_map` 的转换。
- `router_planner` 的 service/action 封装。
- 机械臂相机单次 KFS 复核。
- 相机判断 R1 KFS 是否已移除。
- 森林执行 action、相邻方块移动、吸盘抓取、临时放置。
- `/r2/forest/map_state` 状态机。
- R1 对齐、爬上 R1、二/三层 KFS 放置。

## 未接入节点 / 目前未知节点

| 任务能力 | 当前处理方式 | 推荐接口 |
|---|---|---|
| 健康检查 | `PreMatchSelfCheckPlaceholder` | `/r2/health/check_start_ready` |
| 急停判断 | `IsEmergencyStopRequestedPlaceholder` | `/r2/safety/emergency_stop_state` |
| 重试恢复 | `ExecuteAreaAwareRetryPlaceholder` | `/r2/retry/execute_area_aware_retry` |
| 等待 120 秒 | `MatchElapsedReached120SecPlaceholder` | `/r2/match/elapsed_time` 或 BT timing wrapper |
| 读取 ArUco | `ReadArUcoInstruction_*_Placeholder` | `r2/perception/read_aruco` |
| 进入梅林模式 | `SetTravelModeForestPlaceholder` | `/r2/chassis/set_travel_mode` |
| 人工 KFS map | `WaitForManualKFSMapPlaceholder` | `/kfs_locator/state`、`/r2/perception/kfs_map` |
| 梅林规划 | `RequestForestPlanPlaceholder` | `/r2/forest/request_plan` |
| 地图状态机 | `InitializeForestMapStatePlaceholder` | `/r2/forest/map_state`、`/r2/forest/update_map_state` |
| 梅林执行 | `ExecuteForestPlanPlaceholder` | `/r2/forest/execute_forest_plan` |
| KFS 复核 | 无单独 BT 节点 | `/r2/perception/validate_kfs_on_block` |
| R1 KFS 移除判断 | 无单独 BT 节点 | `/r2/forest/check_r1_kfs_removed` |
| 相邻 KFS 抓取 | 执行 action 内部调用 | `/r2/manipulation/pick_adjacent_kfs` |
| 对抗区对齐 | `AlignToR1ForClimbPlaceholder` | `/r2/motion/align_to_r1` |
| 爬上 R1 | `UpStairsOntoR1Placeholder` | `r2/navigation/up_stairs` |
| 二/三层放置 | `PlaceKFSOnGridLayer*_Placeholder` | `/r2/motion/place_kfs_on_grid` |

## 新增代码准则

新增代码前必须先回答：

1. R2 现有节点是否已经能做？
2. 现有 R2 包或外部包是否已有 topic/action/service 能直接复用？
3. 能否把能力做成外部任务级 action/service，而不是 BT 节点？
4. 如果必须新增 BT 节点，是否能做到通用、极小、无业务细节？

允许新增的低风险节点类型：

- 通用 topic publisher。
- 通用 service/action wrapper。
- 等待状态 topic 满足条件的薄 wrapper。

不建议新增的节点类型：

- 内含复杂视觉逻辑的 BT 节点。
- 内含机械臂轨迹细节的 BT 节点。
- 内含树林方块搜索/规则判断的大型 BT 节点。
- 第二套行为树框架。

## 后续调研检查清单

- [ ] 确认三处命名航点真实坐标：`mc_mf_boundary_standoff`、`forest_entry_block_2_standoff`、`battlefield_grid_standoff`。
- [ ] 确认 `router_planner` 改为 service/action 时的请求/响应字段。
- [ ] 确认机械臂深度相机能稳定做 `ValidateKFSOnBlock`。
- [ ] 确认相机判断 R1 KFS 移除所需视角、阈值、连续帧数和超时。
- [ ] 确认 `AlignToR1` action 是否补齐，或用 `AlignToTarget` 替代。
- [ ] 确认 `PlaceKFS.action` 是否扩展三层模式，或新建 `/r2/motion/place_kfs_on_grid`。
- [ ] 确认实车底层急停链路和行为树软停链路如何协作。

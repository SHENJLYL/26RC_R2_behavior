# R2 跳过端头任务行为树设计草稿

日期：2026-07-06

本文记录当前策略下的 R2 行为树草稿：不执行端头夹取和兵器组装任务，改为先等待满足进入梅林条件，再执行树林 KFS 移动/拿取，随后进入对抗区配合 R1 完成九宫格放置。

对应 XML 草稿：

```text
behavior_trees/r2_no_tip_competition_draft.xml
```

该 XML 是接口设计草稿，不是当前可直接运行的实车主树。当前 `r2_behavior_server` 已注册的真实节点只有 `IsManualStart`、`NavigateToNamedPose`、`PubNav2Goal`、`PublishTwist`；本文中其它节点均表示后续需要封装的 action、service、topic 或状态机。

## 术语约定

| 术语 | 本文含义 |
|---|---|
| `武馆(MC)` | 比赛一区，R2 启动和梅林前等待的位置来源 |
| `梅林(MF)` | 比赛二区，包含 R2 入口、R2 出口、R1 通道和树林 |
| `树林` | 梅林内 1-12 号高低方块区域；有时口头说“梅花林”，本文统一写作树林 |
| `对抗区` | 比赛三区，包含九宫格架子与 R1/R2 配合区域 |
| `端头` | 规则正式术语；本版流程跳过端头夹取和兵器组装 |
| `KFS` | 武术秘籍，分为 `R1 KFS`、`R2 KFS`、`假KFS` |
| `R1 码` | R1 上显示的 ArUco 指令码；接口层按 `ReadArUco` 处理 |

## 总体主流程

```text
R2_NoTip_Competition_Main
├── EmergencyStopBranch
├── NoTip_RetryRecovery
└── NoTipCompetitionMission
    ├── NoTip_PreMatchAndStart
    ├── NoTip_MoveToMFBoundaryAndWait
    ├── NoTip_EnterMFWithRecovery
    ├── NoTip_ForestPlanAndExecute
    ├── NoTip_BattlefieldGridPlacement
    └── StopAllMotion
```

语义上等价于：

1. 赛前健康检查通过，收到 `/manual_start` 后启动。
2. 明确跳过端头夹取和兵器组装。
3. R2 导航到武馆与梅林交界处等待。
4. 当比赛经过 2 分钟达到允许直接进入梅林的条件，或读取到 R1 ArUco 指令允许继续时，进入梅林。
5. 到达树林入口，也就是 2 号平台面前。
6. 读取网页人工录入的 KFS 分布，调用梅林路径规划器得到动作序列。
7. 按规划序列执行移动、拿取、临时放置和地图更新。
8. 完成树林任务并携带 R2 KFS 后，经 10/11/12 号方块之一离开树林。
9. 导航到对抗区九宫格附近，等待 R1 手动对齐信号。
10. R2 对齐并爬上 R1；后续根据新 R1 码选择二层或三层放置。
11. 完成放置后确认不再携带 KFS，停车。

## 已复用和待封装接口

| 能力 | 当前来源 | XML 中的节点 | 状态 |
|---|---|---|---|
| 手动启动 | `/manual_start` `std_msgs/msg/Int32` | `IsManualStart` | 已接入 |
| 命名航点导航 | `/r2/navigation/navigate_to_named_pose` | `NavigateToNamedPose` | 已接入 BT client，依赖外部 server |
| 停车 | `cmd_vel` `geometry_msgs/msg/Twist` | `PublishTwist`/`StopAllMotion` | 已接入 |
| 网页 KFS 录入 | `/kfs_locator/state` `web_spoiler/msg/WebSpoiler` | `WaitForManualKFSMap` | 待封装 |
| 梅林路径规划 | `router_planner` 的 `topic1 -> topic2` | `RequestForestPlan` | 待封装 |
| KFS 视觉复核 | `/kfs_tracker/detection` `kfs_tracker/msg/KFSDetection` | `ValidateKFSOnBlock` 概念 | 待封装为单次 service |
| 上/下台阶 | `rc26_r2_interfaces/action/UpStairs`、`DownStairs` | `UpStairsAction` 等 | action 类型已定义，BT wrapper 待封装 |
| 台阶旋转 | `rc26_r2_interfaces/action/RotateOnStair` | 由 `ExecuteForestPlan` 内部调用 | action 类型已定义，BT wrapper 待封装 |
| R1 指令码读取 | `rc26_r2_interfaces/action/ReadArUco` | `ReadArUcoInstruction` | action 类型已定义，BT wrapper 待封装 |
| KFS 放置 | `rc26_r2_interfaces/action/PlaceKFS` | `PlaceKFSAction` | 二层模式已定义，三层模式待扩展 |
| R1 对齐 | README 提到 `AlignToR1` | `AlignToR1ForClimb` | action 文件暂未发现，需要补齐或改用其它接口 |

## 关键阶段设计

### 1. 开始与跳过端头任务

`NoTip_PreMatchAndStart` 的设计目标是只做比赛开始前的必要准备：

```text
HealthCheckStartReady
-> IsManualStart
-> SkipTipPickupAndWeaponAssemblyByStrategy
```

`SkipTipPickupAndWeaponAssemblyByStrategy` 是语义标记，表示本版流程不进入端头架、不执行夹爪夹取端头、不执行兵器组装。后续如果需要恢复该任务，应创建新的主树或重新启用原 `MC_AssembleWeapon` 子树。

### 2. 武馆到梅林交界处等待

`NoTip_MoveToMFBoundaryAndWait` 的目的：

1. 先到 `mc_mf_boundary_standoff`。
2. 停车等待。
3. 等待“比赛开始后 2 分钟时间条件满足”或“R1 ArUco 指令允许继续”。

该阶段成功后，行为树把状态视为 `R1 已进入梅林`，不再等待武馆组装结果。

已确认参数：

| 参数 | 含义 |
|---|---|
| `mf_entry_wait_sec` | `120.0` 秒，即 2 分钟 |
| `R1 code source` | 只使用 ArUco，不按普通二维码设计 |
| `R1_ENTERED_MF_OR_CONTINUE_TO_MF` | R1 ArUco 指令文本或枚举值 |

仍需标定：

| 参数 | 含义 |
|---|---|
| `mc_mf_boundary_standoff` | 武馆与梅林交界等待航点 |

### 3. 进入梅林与卡住恢复

`NoTip_EnterMFWithRecovery` 是一个 fallback：

```text
NormalEnterMF
或
ArucoContinueRecovery
```

正常分支：

```text
NavigateToNamedPose(forest_entry_block_2_standoff)
-> SetTravelMode(FOREST)
-> ConfirmChassisLiftAndForestMode
```

恢复分支：

```text
-> ReadArUcoInstruction(CONTINUE_TO_MF)
-> NavigateToNamedPose(forest_entry_block_2_standoff)
-> SetTravelMode(FOREST)
-> ConfirmChassisLiftAndForestMode
```

这表达用户提出的策略：如果 R2 因某些原因没有顺利移动到梅林，读取 R1 ArUco 后继续前进到梅林。

### 4. 树林规划与执行

`NoTip_ForestPlanAndExecute` 是当前最复杂的阶段。它把“网页人工录入 -> 路径规划 -> 按序执行 -> 地图维护”串起来：

```text
WaitForManualKFSMap
-> RequestForestPlan
-> InitializeForestMapState
-> ExecuteForestPlan
-> RequirePayload(R2_KFS)
-> ExitForestViaBlock(10/11/12)
-> SetTravelMode(GROUND)
```

当前可复用内容：

- `web_spoiler` 已持续发布 `/kfs_locator/state`。
- `router_planner` 已能从 JSON grid 计算动作，并发布 `ForestSteps`。
- `ForestStepSingle` 已有 `MOVE/PICK/PLACE` 三类动作和 `target_block_id`。

建议封装：

| 接口 | 建议形式 | 原因 |
|---|---|---|
| `WaitForManualKFSMap` | action 或 condition | 等待网页 ready 且 map 合法 |
| `RequestForestPlan` | service 或 action | 规划是一次请求一次结果，service 足够；如要等待外部节点可用也可 action |
| `ValidateKFSOnBlock` | service | 使用机械臂深度相机做单次视觉判断，避免反复检测造成误判 |
| `ExecuteForestPlan` | action | 会持续执行多步、需要 feedback、可能失败/重试 |
| `UpdateForestMapState` | service/topic | 记录每步后当前 map 和 removed block |

`ExecuteForestPlan` 每一步应按以下语义执行：

1. 从 `ForestSteps.steps[current_step_index]` 取当前步骤。
2. 用机械臂深度相机对目标方块做一次视觉复核，确认需要被 R2 移动的 KFS 与人工 map/规划一致。
3. 如果不一致，返回 failure，触发重新识别、重规划或人工处理。
4. 若规划提示必须经过 R1 KFS 被移除后的路径，先用相机视觉检查 R1 KFS 是否已被移除。
5. 若相机仍判断 R1 KFS 未移除，等待 `R1_KFS_REMOVED` 状态；移除后延迟短时间再移动，避免与正在操作的 R1 干涉。
6. 对 `MOVE`：执行相邻方块移动，并用丝杠圈数/电流、四轮红外、激光雷达确认爬升/下降和到位。
7. 对 `PICK`：只允许抓取目标方块上的 `R2_KFS`；吸盘成功后更新 payload。
8. 对 `PLACE`：仅用于必要时临时放下当前携带的 R2 KFS，并更新 map。
9. 每步结束写入 `current_kfs_map`、`removed_r2_blocks`、`current_block`、`current_step_index`。

### 5. 对抗区九宫格放置

`NoTip_BattlefieldGridPlacement` 的顺序：

```text
NavigateToNamedPose(battlefield_grid_standoff)
-> StopAllMotion
-> ReadArUcoInstruction(R1_GRID_ALIGN_READY)
-> AlignToR1ForClimb
-> UpStairsAction
-> ConfirmChassisLiftAndForestMode
-> PlaceByR1LayerCode
-> RequirePayload(NONE)
```

二层放置：

```text
ReadArUcoInstruction(PLACE_GRID_LAYER2)
-> PlaceKFSAction(mode=PLACE_ON_GRID_LAYER2)
```

三层放置：

```text
ReadArUcoInstruction(PLACE_GRID_LAYER3)
-> PlaceKFSAction(mode=PLACE_ON_GRID_LAYER3_TODO)
```

注意：`PlaceKFS.action` 当前只定义了 `PLACE_ON_MERLIN=0` 和 `PLACE_ON_GRID_LAYER2=1`。如果比赛流程需要 R2 在 R1 举升后放三层，需要补充 `PLACE_ON_GRID_LAYER3`，或新增专门的九宫格放置 action。

## 梅林地图维护状态机

该状态机的目标是记录 R2 在运行过程中实际移除了哪些 R2 KFS，以及当前树林 map 与开局人工 map 的差异。它不应只存在于行为树 XML 注释中，后续应实现为独立 map manager 节点或森林执行 action 的内部状态，并通过 topic/rosbag 输出。

### 建议状态

| 状态 | 含义 |
|---|---|
| `UNINITIALIZED` | 尚未读取网页人工 map |
| `INITIAL_MAP_READY` | `/kfs_locator/state.ready=true`，已得到开局 map |
| `INITIAL_MAP_LOCKED` | map 已确认不可随意修改 |
| `PLAN_READY` | 已得到 `ForestSteps` |
| `EXECUTING_STEP` | 正在执行某个 MOVE/PICK/PLACE |
| `WAITING_R1_KFS_REMOVAL` | 路径需要 R1 KFS 被 R1 移走，R2 正在等待 |
| `POST_R1_REMOVAL_DELAY` | R1 KFS 已移除，R2 短暂停顿防干涉 |
| `VISION_MISMATCH` | 视觉复核与 map/plan 不一致 |
| `R2_KFS_REMOVED_RECORDED` | 某个 R2 KFS 已被 R2 成功移除并记录 |
| `CARRYING_R2_KFS` | 当前吸盘携带一个 R2 KFS |
| `TEMP_PLACED_R2_KFS` | 因阻碍处理临时放下过 R2 KFS |
| `FOREST_EXIT_READY` | 满足离开树林条件 |
| `FOREST_DONE` | 已离开树林 |
| `RECOVERY_REQUIRED` | 多次失败，需要重试或人工恢复 |

### 建议保存字段

| 字段 | 含义 |
|---|---|
| `initial_kfs_map[1..12]` | 开局网页确认的 KFS 分布 |
| `current_kfs_map[1..12]` | 比赛运行中的当前 KFS 分布 |
| `removed_r2_blocks[]` | R2 已经成功移除 R2 KFS 的方块编号 |
| `r1_wait_blocks[]` | 曾等待 R1 移除 KFS 的方块编号 |
| `conflict_blocks[]` | 视觉与 map 不一致的方块编号 |
| `current_block` | R2 当前所在/面向的树林方块 |
| `current_step_index` | 当前执行到 ForestSteps 的第几步 |
| `carrying_payload` | `NONE/R2_KFS/UNKNOWN` 等 |
| `route_version` | 当前规划版本号 |
| `map_version` | 当前 map 版本号 |
| `last_event` | 最近一次状态转换原因 |

### 关键事件

| 事件 | 状态转换 |
|---|---|
| `manual_map_ready` | `UNINITIALIZED -> INITIAL_MAP_READY` |
| `manual_map_locked` | `INITIAL_MAP_READY -> INITIAL_MAP_LOCKED` |
| `forest_plan_received` | `INITIAL_MAP_LOCKED -> PLAN_READY` |
| `step_started` | `PLAN_READY -> EXECUTING_STEP` |
| `vision_match` | 保持执行 |
| `vision_mismatch` | `EXECUTING_STEP -> VISION_MISMATCH` |
| `r1_kfs_required_not_removed` | 相机判断 R1 KFS 仍在路径阻碍位置，`EXECUTING_STEP -> WAITING_R1_KFS_REMOVAL` |
| `r1_kfs_removed` | 相机判断 R1 KFS 已被移除，`WAITING_R1_KFS_REMOVAL -> POST_R1_REMOVAL_DELAY` |
| `pick_r2_success` | `EXECUTING_STEP -> R2_KFS_REMOVED_RECORDED -> CARRYING_R2_KFS` |
| `place_temp_success` | `EXECUTING_STEP -> TEMP_PLACED_R2_KFS` |
| `exit_ready` | `CARRYING_R2_KFS -> FOREST_EXIT_READY` |
| `forest_exited` | `FOREST_EXIT_READY -> FOREST_DONE` |
| `retry_needed` | 任意非完成状态 -> `RECOVERY_REQUIRED` |

## 重试与恢复策略

| 场景 | 首选处理 | 失败后处理 |
|---|---|---|
| 导航到 MC/MF 交界失败 | `NavigateToNamedPose` 重试 2 次 | 停车，读取 R1 ArUco，尝试继续到梅林入口 |
| 未满足进入梅林时间条件 | 停车等待 | 若收到继续指令，进入梅林；否则保持等待 |
| 网页 KFS map 未 ready | 等待 `/kfs_locator/state.ready=true` | 超时后不进入树林，要求人工确认 |
| 规划器无可行路线 | 重新请求规划 | 若由 R1 KFS 阻碍导致，进入等待 R1 移除流程；否则进入恢复 |
| 视觉复核与 map 不一致 | 单次复核失败即停止当前 step | 重新识别/重规划；仍不一致则人工处理 |
| R1 KFS 未被移除 | 相机持续/定期判断并等待 R1 移除 | 超时后停车，继续读 R1 ArUco 或请求恢复 |
| R1 KFS 刚被移除 | 延迟短时间再移动 | 若移动风险仍高，重新检查障碍 |
| 爬升/下降未确认 | 用丝杠圈数/电流、四轮红外、激光雷达复核 | 重试一次；仍失败则停车恢复 |
| 抓取 R2 KFS 失败 | 重新定位目标并重试 | 更新 map 为冲突/未知，重新规划 |
| 抓到非 R2 KFS 或 payload UNKNOWN | 立即停车 | 标记严重异常，进入重试/人工恢复 |
| 对抗区等待 R1 对齐无信号 | 停车等待 R1 码 | 超时后保持安全位，不自行爬升 |
| 爬上 R1 失败 | 停车并下撤到安全姿态 | 人工恢复或区域重试 |
| 二/三层放置失败 | 重新对齐并重试 | 放置仍失败则保持 payload 并等待人工/新指令 |

## 已确认事项与仍需确认问题

已确认：

1. `mf_entry_wait_sec = 120.0` 秒，即 2 分钟。
2. R1 码使用 ArUco，不按普通二维码兼容设计。
3. `ValidateKFSOnBlock` 的视觉输入使用机械臂深度相机。
4. “R1 KFS 已被 R1 移除”通过相机视觉判断。

仍需和底层同学确认：

1. `mc_mf_boundary_standoff`、`forest_entry_block_2_standoff`、`battlefield_grid_standoff` 三个航点的真实坐标。
2. `SetTravelMode`、`ConfirmChassisLiftAndForestMode` 是否由底盘 action 提供，还是由多个 topic/service 组合。
3. `router_planner` 是否改为 service/action；如果保持 `topic1/topic2`，需要 request id 防止旧规划结果串扰。
4. `ForestSteps` 当前不包含转向动作；上下台阶和旋转应由 `ExecuteForestPlan` 根据相邻 block 高度和当前朝向补全。
5. 用于判断 R1 KFS 移除的相机视角、检测阈值、连续确认帧数和超时时间。
6. `AlignToR1.action` 在 README 中出现，但 action 文件暂未发现；需要补接口或替换为已有 `AlignToTarget`。
7. `PlaceKFS.action` 是否扩展三层放置模式，或新建更具体的 `PlaceKFSOnGrid.action`。

## rosbag 必录建议

为验证该行为树是否按流程推进，建议至少录制：

```bash
/manual_start
/cmd_vel
/odom
/tf
/tf_static
/kfs_locator/state
/kfs_tracker/detection
/r2/perception/kfs_map
/r2/forest/map_state
/r2/perception/payload_state
/r2/perception/r1_qr_state
/r2/chassis/wheel_height_state
/r2/chassis/travel_mode_state
/r2/navigation/navigate_to_named_pose/_action/status
/r2/navigation/navigate_to_named_pose/_action/feedback
/r2/navigation/navigate_to_named_pose/_action/result
/r2/navigation/up_stairs/_action/status
/r2/navigation/up_stairs/_action/feedback
/r2/navigation/up_stairs/_action/result
/r2/motion/place_kfs/_action/status
/r2/motion/place_kfs/_action/feedback
/r2/motion/place_kfs/_action/result
```

## 当前结论

本设计把 R2 行为树控制权保持在 XML/任务级接口层：行为树只决定“什么时候做什么、失败后怎么走”，不把视觉算法、梅林搜索、底盘找平和机械臂轨迹写进 BT 节点。下一步应优先把 `WaitForManualKFSMap`、`RequestForestPlan`、`ExecuteForestPlan`、`ReadArUcoInstruction`、`PlaceKFSAction` 这些占位节点逐步封装为最小 wrapper 或外部任务级 action。

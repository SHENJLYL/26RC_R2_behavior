# R2 Behavior Action Sequence

本文档把当前 no-tip 行为树流程映射到后续 action/service/topic 接入顺序。当前策略是不执行端头夹取和兵器组装；R2 从武馆/梅林交界等待开始，满足 2 分钟或 R1 ArUco 指令后进入梅林，完成树林 KFS 流程后到对抗区配合 R1 放置 KFS。

## 当前主树骨架

```text
R2_Competition_Main
├── EmergencyStopBranch
├── RetryRecoveryBranch
└── CompetitionMissionNoTip
    ├── PreMatchAndStart
    ├── MC_WaitAtMFBoundary
    ├── MF_EnterForestEntry
    ├── MF_ExecuteForestPlan
    ├── Battlefield_ClimbAndPlaceKFS
    └── StopAllMotion
```

`behavior_trees/r2_no_tip_competition_draft.xml` 保留更详细的草稿注释；`behavior_trees/r2_competition_main.xml` 是当前主树的安全占位版本。所有未接入能力仍用 `AlwaysFailure` 占位，避免实车误跑。

## 0. 全局保护分支

### EmergencyStopBranch

| 当前节点 | 后续接口 | 进入条件 | 成功/输出 |
|---|---|---|---|
| `IsEmergencyStopRequestedPlaceholder` | topic `/r2/safety/emergency_stop_state` | 急停、软停或严重安全故障 | 返回 true 后执行 `StopAllMotion` |
| `StopAllMotion` | `PublishTwist cmd_vel=0`，同时保留底层急停链路 | 需要停车 | 底盘停，机械臂/吸盘进入安全策略 |

### RetryRecoveryBranch

| 当前节点 | 后续接口 | 进入条件 | 成功/输出 |
|---|---|---|---|
| `IsRetryRequestedPlaceholder` | topic/service `/r2/retry/request` | 裁判、操作员、规则守卫或 action failure 请求重试 | 返回 true |
| `ExecuteAreaAwareRetryPlaceholder` | action `/r2/retry/execute_area_aware_retry` | 已确认重试 | 根据当前区域、payload、地图状态回到允许恢复状态 |

## 1. PreMatchAndStart

```text
HealthCheckStartReady
-> IsManualStart
-> SkipTipPickupAndWeaponAssemblyByStrategy
```

| 当前节点 | 后续接口 | 读取信息 | 成功条件 |
|---|---|---|---|
| `PreMatchSelfCheckPlaceholder` | service/action `/r2/health/check_start_ready` | TF、导航 action、雷达、前方深度相机、机械臂深度相机、底盘抬升、吸盘、急停 | 所有关键模块 ready，或只存在允许 warning |
| `IsManualStart` | topic `/manual_start` `std_msgs/msg/Int32` | 启动信号 | 收到 `>= 1` |
| `SkipTipPickupAndWeaponAssemblyByStrategy` | 无底层接口 | 当前比赛策略 | 明确跳过端头任务 |

当前不接入：

- `/r2/manipulation/pick_tip`
- `/r2/manipulation/assemble_tip_to_pole`
- `/r2/perception/weapon_assembled`

这些接口可以保留在接口库中，但不进入当前 no-tip 主树。

## 2. MC_WaitAtMFBoundary

```text
NavigateToNamedPose(mc_mf_boundary_standoff)
-> StopAllMotion
-> Wait120SecOrReadArUcoContinue
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `NavigateToNamedPose target=mc_mf_boundary_standoff` | 已接入 `/r2/navigation/navigate_to_named_pose` | map/odom/TF、雷达、命名航点 | 底盘导航 | 到达武馆/梅林交界等待位 |
| `StopAllMotion` | `cmd_vel=0` | 无 | 底盘停车 | 机器人保持静止 |
| `MatchElapsedReached120SecPlaceholder` | condition/service `/r2/match/elapsed_time` 或 BT 内部计时 | 比赛开始时间 | 无 | elapsed >= `120.0` 秒 |
| `ReadArUcoInstruction_ContinueToMF_Placeholder` | action `/r2/perception/read_aruco` | R1 ArUco id、instruction、置信度、位姿 | 无 | instruction 允许继续进入梅林 |

语义约束：

- 等待成功后，行为树视作 R1 已经进入梅林。
- 如果导航到交界处失败，可停车并进入 ArUco 继续指令恢复分支。

## 3. MF_EnterForestEntry

```text
NavigateToNamedPose(forest_entry_block_2_standoff)
-> SetTravelMode(FOREST)
-> ConfirmChassisLiftAndForestMode
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `NavigateToNamedPose target=forest_entry_block_2_standoff` | `/r2/navigation/navigate_to_named_pose` | 定位、雷达、命名航点 | 底盘导航 | 到达 PDF 标注的 2 号平台面前 |
| `SetTravelModeForestPlaceholder` | action `/r2/chassis/set_travel_mode` | 丝杠圈数/电流、四轮红外高度、底盘姿态 | 丝杠抬升/模式切换 | `travel_mode=FOREST` |
| `ConfirmChassisLiftAndForestModePlaceholder` | service/topic check | 丝杠圈数/电流、四轮红外、激光雷达 | 无 | 确认车确实完成爬升/下降准备 |

恢复分支：

```text
StopAllMotion
-> ReadArUcoInstruction(CONTINUE_TO_MF)
-> NavigateToNamedPose(forest_entry_block_2_standoff)
-> SetTravelMode(FOREST)
-> ConfirmChassisLiftAndForestMode
```

## 4. MF_ExecuteForestPlan

```text
WaitForManualKFSMap
-> RequestForestPlan
-> InitializeForestMapState
-> ExecuteForestPlan
-> RequirePayload(R2_KFS)
-> ExitForestViaBlock10_11_12
-> SetTravelMode(GROUND)
```

### 4.1 输入来源

| 来源 | 当前接口 | 行为树使用方式 |
|---|---|---|
| 网页人工 KFS 录入 | topic `/kfs_locator/state` `web_spoiler/msg/WebSpoiler` | 等待 `ready=true`，读取 1-12 号方块 KFS 类型 |
| 梅林路径规划 | `router_planner` 当前 `topic1 -> topic2` | 建议封装为 `/r2/forest/request_plan` service 或 action |
| KFS 视觉复核 | topic `/kfs_tracker/detection` | 建议封装为单次 service `/r2/perception/validate_kfs_on_block` |
| 地图维护 | 新增 topic `/r2/forest/map_state` | 记录 current map、removed_r2_blocks、current_block、payload |

### 4.2 接口映射

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `WaitForManualKFSMapPlaceholder` | action/condition `/r2/setup/wait_for_kfs_map_confirmation` 或直接订阅 `/kfs_locator/state` | 网页 ready、color、kfs_pos、paths | 无 | KFS map 已确认可用 |
| `RequestForestPlanPlaceholder` | service `/r2/forest/request_plan` | KFS map、start_block=2、target=exit | 请求规划器 | 得到 `ForestSteps` |
| `InitializeForestMapStatePlaceholder` | service `/r2/forest/update_map_state` | initial map、start_block=2 | 初始化状态机 | `initial_kfs_map/current_kfs_map` 建立 |
| `ExecuteForestPlanPlaceholder` | action `/r2/forest/execute_forest_plan` | ForestSteps、map_state、payload、底盘高度状态 | 移动、等待、抓取、临时放置 | 所有 steps 按序完成 |
| `RequirePayloadR2KFSPlaceholder` | condition/service `/r2/perception/payload_state` | 吸盘/视觉 payload | 无 | 当前携带 `R2_KFS` |
| `ExitForestViaBlock10_11_12Placeholder` | action `/r2/forest/exit_via_block` | current_block、exit candidates、payload | 方块移动、退出树林 | 经 10/11/12 离开 |
| `SetTravelModeGroundPlaceholder` | action `/r2/chassis/set_travel_mode` | 红外、丝杠 | 切回普通地面模式 | `travel_mode=GROUND` |

### 4.3 ExecuteForestPlan 内部语义

每个 `ForestStepSingle` 执行时遵循：

1. 读取 `current_step_index` 和 `target_block_id`。
2. 用机械臂深度相机调用 `/r2/perception/validate_kfs_on_block` 做一次性视觉复核。
3. 如果目标方块 KFS 与人工 map/规划不一致，返回 failure 并进入重新识别、重规划或人工处理。
4. 如果规划提示路径必须等待 R1 KFS 被移除，调用 `/r2/forest/check_r1_kfs_removed`，该判断由相机完成。
5. 若相机判断 R1 KFS 仍在，进入等待；判断已移除后短暂停顿，再继续移动，避免干涉正在移除 KFS 的 R1。
6. `MOVE`：调用 `/r2/forest/step_to_adjacent_block`，并用丝杠圈数/电流、四轮红外、激光雷达确认爬升/下降和到位。
7. `PICK`：调用 `/r2/manipulation/pick_adjacent_kfs`，只允许吸取 R2 KFS。
8. `PLACE`：调用 `/r2/forest/place_payload_temporarily`，用于路径阻碍处理中的临时放置。
9. 每步结束调用 `/r2/forest/update_map_state`，写入 `removed_r2_blocks`、`current_kfs_map`、`current_block`、`payload_state`。

### 4.4 地图维护状态机

必须记录：

| 字段 | 含义 |
|---|---|
| `initial_kfs_map[1..12]` | 开局网页确认的 KFS 分布 |
| `current_kfs_map[1..12]` | 比赛运行中的当前 KFS 分布 |
| `removed_r2_blocks[]` | R2 已成功移除 R2 KFS 的方块编号 |
| `r1_wait_blocks[]` | 曾等待 R1 移除 KFS 的方块编号 |
| `conflict_blocks[]` | 视觉与 map 不一致的方块编号 |
| `current_block` | 当前所在或面向的树林方块 |
| `current_step_index` | 当前执行到 ForestSteps 的第几步 |
| `carrying_payload` | `NONE/R2_KFS/UNKNOWN` |
| `map_version` | 地图版本 |
| `route_version` | 规划版本 |

## 5. Battlefield_ClimbAndPlaceKFS

```text
NavigateToNamedPose(battlefield_grid_standoff)
-> StopAllMotion
-> ReadArUcoInstruction(R1_GRID_ALIGN_READY)
-> AlignToR1ForClimb
-> UpStairsOntoR1
-> PlaceByArUcoLayerCode
-> RequirePayload(NONE)
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `NavigateToNamedPose target=battlefield_grid_standoff` | `/r2/navigation/navigate_to_named_pose` | 定位、雷达、命名航点 | 底盘导航 | 到达九宫格附近预备位 |
| `ReadArUcoInstruction_R1GridAlignReady_Placeholder` | action `/r2/perception/read_aruco` | R1 ArUco | 无 | R1 手操对齐完成 |
| `AlignToR1ForClimbPlaceholder` | action `/r2/motion/align_to_r1` 或替代接口 | R1 marker pose、对齐误差 | 底盘微调 | 可爬上 R1 |
| `UpStairsOntoR1Placeholder` | action `/r2/navigation/up_stairs` | 丝杠/红外/姿态 | 上升/爬升 | 爬上 R1 或 R1 支撑位 |
| `PlaceKFSOnGridLayer2Placeholder` | action `/r2/motion/place_kfs_on_grid` | ArUco layer code、九宫格位姿、payload | 放置 KFS | 二层放置成功 |
| `PlaceKFSOnGridLayer3Placeholder` | action `/r2/motion/place_kfs_on_grid` | ArUco layer code、九宫格位姿、payload | 爬升及放置 KFS | 三层放置成功 |
| `RequirePayloadNonePlaceholder` | topic/service `/r2/perception/payload_state` | 吸盘/视觉 | 无 | payload 清空 |

注意：当前 `PlaceKFS.action` 只定义了 `PLACE_ON_MERLIN` 和 `PLACE_ON_GRID_LAYER2`。若继续沿用该 action，需要扩展 `PLACE_ON_GRID_LAYER3`；也可以新增更明确的 `/r2/motion/place_kfs_on_grid`。

## 6. 重试策略

| 场景 | 首选处理 | 失败后处理 |
|---|---|---|
| 导航到 MC/MF 交界失败 | `NavigateToNamedPose` 重试 2 次 | 停车，读取 R1 ArUco，尝试继续到梅林入口 |
| 未到 120 秒且无 ArUco | 停车等待 | 继续等待或人工恢复 |
| 网页 KFS map 未 ready | 等待 `/kfs_locator/state.ready=true` | 超时后不进入树林 |
| 规划器无可行路线 | 重新请求规划 | 若是 R1 KFS 阻碍，等待相机确认 R1 移除；否则恢复 |
| 视觉复核不一致 | 单次 service 返回 failure | 重新识别/重规划；仍不一致则人工处理 |
| R1 KFS 未被移除 | 相机定期判断并等待 | 超时后停车，读取 ArUco 或请求恢复 |
| 爬升/下降未确认 | 丝杠/红外/激光复核并重试 | 仍失败则停车恢复 |
| 抓取 R2 KFS 失败 | 重新定位目标并重试 | 更新 map 为冲突/未知，重新规划 |
| 对抗区无 R1 对齐信号 | 停车等待 ArUco | 超时后保持安全位 |
| 放置二/三层失败 | 重新对齐并重试 | 保持 payload，等待人工或新指令 |

## 7. 每个 action 的最小验收记录

每个 action 接入后，都应至少能在 rosbag 中回答：

1. action 什么时候收到 goal？
2. goal 输入是什么？
3. action 执行期间读取了哪些关键状态？
4. action 输出了哪些底盘、机械臂、丝杠或吸盘命令？
5. 最终 success 还是 failure？
6. 如果失败，`failure_reason` 是什么？
7. 行为树是否根据结果进入正确下一步或恢复分支？

# R2 Action / Service / Topic Interface Catalog

本文档是当前 no-tip 行为树完整落地所需的接口清单。命名是建议稿，不代表已经全部实现；真正实现时应优先复用已有接口，缺口再补最小 wrapper。

## 接口分层

| 层级 | 形式 | 行为树视角 |
|---|---|---|
| 行为树 XML | BT.CPP XML | 编排顺序、分支、重试、保护条件 |
| BT wrapper | C++ 小节点 | 把 topic/action/service 变成 BT 可 tick 的节点 |
| 任务级 action | ROS 2 action | 执行耗时动作，必须有 feedback/result |
| 单次检查 service | ROS 2 service | 做一次判断，避免连续检测抖动影响 BT 语义 |
| 状态 topic | ROS 2 topic | 持续发布当前状态，供 BT wrapper、rosbag、调试读取 |
| 底层控制 | ROS topic/service/action | 底盘、机械臂、吸盘、视觉内部细节，BT 不直接细控 |

## 已有或已发现接口

| 能力 | 当前来源 | 当前状态 | 行为树用法 |
|---|---|---|---|
| 手动启动 | `/manual_start` `std_msgs/msg/Int32` | 已接入 `IsManualStart` | `/manual_start >= 1` 后进入主流程 |
| 命名航点导航 | `/r2/navigation/navigate_to_named_pose` | 已接入 `NavigateToNamedPose` client | 去交界处、梅林入口、对抗区预备点 |
| 直接 Nav2 goal | `goal_pose` `PoseStamped` | 已接入 `PubNav2Goal` | 调试和简化导航 |
| 停车 | `cmd_vel` `Twist` | 已接入 `PublishTwist` | `StopAllMotion` 发布零速度 |
| R1 ArUco | `ReadArUco.action` | action 类型已发现 | 需补 BT wrapper |
| 上台阶 | `UpStairs.action` | action 类型已发现 | 梅林/爬上 R1 可复用 |
| 下台阶 | `DownStairs.action` | action 类型已发现 | 梅林台面下降可复用 |
| 台阶旋转 | `RotateOnStair.action` | action 类型已发现 | ForestSteps 内部补转向 |
| KFS 放置 | `PlaceKFS.action` | action 类型已发现 | 二层已有模式，三层需扩展或新 action |
| 目标定位/对齐 | `LocateTarget.action`、`AlignToTarget.action` | action 类型已发现 | 可作为 R1/九宫格对齐候选 |
| 网页 KFS 录入 | `/kfs_locator/state` `web_spoiler/msg/WebSpoiler` | 已存在 topic | 需转为统一 KFS map |
| 梅林规划 | `meilin_router/router_planner` 的 `topic1 -> topic2` | 已有函数/节点，未封装成请求式接口 | 需 service/action wrapper |
| KFS 视觉检测 | `/kfs_tracker/detection` `kfs_tracker/msg/KFSDetection` | 已有连续检测 topic | 需封装成方块级单次复核 service |

## 全局 topic

| topic | 类型建议 | 发布方 | BT 用途 |
|---|---|---|---|
| `/r2/safety/emergency_stop_state` | `EmergencyStopState.msg` | 安全节点 | 根节点急停分支 |
| `/r2/retry/request` | `RetryRequest.msg` | 人工/监控节点 | 区域恢复分支 |
| `/r2/match/elapsed_time` | `builtin_interfaces/Duration` 或自定义 msg | 比赛计时节点 | 判断 120 秒入梅林条件 |
| `/r2/perception/r1_aruco_state` | `R1ArUcoState.msg` | ArUco 识别节点 | 继续入梅林、对齐、层数指令 |
| `/r2/perception/kfs_map` | `KFSMap.msg` | KFS map manager | 梅林规划和规则检查 |
| `/r2/forest/map_state` | `ForestMapState.msg` | 地图维护状态机 | 记录 `removed_r2_blocks`、步骤进度 |
| `/r2/perception/payload_state` | `PayloadState.msg` | 吸盘/视觉融合节点 | 确认是否携带 R2 KFS |
| `/r2/chassis/wheel_height_state` | `WheelHeightState.msg` | 底盘节点 | 判断丝杠/红外高度 |
| `/r2/chassis/travel_mode_state` | `TravelModeState.msg` | 底盘节点 | 判断 `GROUND/FOREST/RECOVERY` 到位 |
| `/r2/chassis/motion_state` | `MotionState.msg` | 底盘/导航节点 | 判断卡住、抖动、移动是否发生 |
| `/r2/manipulation/suction_state` | `SuctionState.msg` | 吸盘节点 | 判断吸取是否成功 |
| `/r2/perception/grid_state` | `GridState.msg` | 九宫格视觉节点 | 对抗区放置前确认层/格状态 |

## 全局 service

### `/r2/health/check_start_ready`

| 项 | 内容 |
|---|---|
| 建议形式 | service `CheckStartReady.srv` |
| 输入 | `mode`、`required_modules[]`、`max_state_age_sec` |
| 读取 | TF、导航 action、相机、雷达、底盘、吸盘、急停、地图文件 |
| 输出 | `ready`、`blocking_errors[]`、`warnings[]`、`diagnostic_text` |
| BT 节点 | `PreMatchSelfCheckPlaceholder` 的替代 |

### `/r2/rule_guard/check`

| 项 | 内容 |
|---|---|
| 建议形式 | service `CheckRuleGuard.srv` |
| 输入 | `proposed_action`、`zone`、`current_block`、`target_block`、`payload_state`、`kfs_map_version` |
| 读取 | 当前 KFS map、payload、比赛规则、地图状态机 |
| 输出 | `allowed`、`reason_code`、`reason_text`、`suggested_recovery` |
| BT 节点 | 放在进入梅林、抓取、临时放置、离开树林、爬上 R1、放置 KFS 前 |

## 赛前与入梅林接口

### `/r2/perception/read_aruco`

| 项 | 内容 |
|---|---|
| 建议形式 | action，可直接复用 `ReadArUco.action` |
| 输入 | `expected_id`、`timeout` |
| 读取 | R1 ArUco 图像、识别 id、指令内容、位姿 |
| 输出 | `success`、`code_id`、`instruction`、`message` |
| feedback | `status`、`progress` |
| BT 用途 | 卡住后继续进梅林、R1 手操对齐完成、二/三层放置指令 |
| 注意 | 当前不按非 ArUco 码设计，topic 名称和文档统一使用 `aruco` |

### `/r2/match/elapsed_time`

| 项 | 内容 |
|---|---|
| 建议形式 | topic 或 BT timing wrapper |
| 输入 | 无，或启动时间戳 |
| 输出 | 比赛开始后的 elapsed seconds |
| BT 用途 | `MatchElapsedReached120SecPlaceholder`，达到 `120.0` 秒后允许进入梅林 |
| 注意 | 如果比赛计时只在 BT 内实现，也应在 rosbag 中输出计时状态便于复盘 |

## 网页 KFS map 与梅林规划

### `/r2/setup/validate_manual_kfs_map`

| 项 | 内容 |
|---|---|
| 建议形式 | service |
| 输入 | 来自 `/kfs_locator/state` 的 `ready`、`color`、`kfs_pos[12]`、`paths[]` |
| 读取 | KFS 数量规则、方块编号规则、入口限制 |
| 输出 | `valid`、`normalized_map`、`map_version`、`error_code`、`message` |
| BT 用途 | `WaitForManualKFSMapPlaceholder`，确认人工 map 可用 |
| 已知来源 | `web_spoiler/msg/WebSpoiler`，`kfs_pos` 中 0/1/2/3 分别表示空、R1、R2、假KFS |

### `/r2/forest/request_plan`

| 项 | 内容 |
|---|---|
| 建议形式 | service；若规划节点启动慢或需要多轮反馈，可做 action |
| 输入 | `current_block`、`entry_block=2`、`kfs_map`、`payload_state`、`removed_r2_blocks[]` |
| 读取 | `meilin_router/router_planner`、方块邻接图、KFS 分布 |
| 输出 | `success`、`ForestSteps`、`route_version`、`requires_r1_removed_blocks[]`、`message` |
| BT 用途 | `RequestForestPlanPlaceholder` |
| 当前缺口 | 现有 planner 使用 `topic1 -> topic2`，需要 request id 或 service wrapper，避免旧结果串扰 |

### `/r2/forest/update_map_state`

| 项 | 内容 |
|---|---|
| 建议形式 | service，同时发布 `/r2/forest/map_state` |
| 输入 | `event`、`block_id`、`step_index`、`payload_state`、`observation` |
| 输出 | `accepted`、`new_map_version`、`new_state`、`diagnostic_text` |
| 必存字段 | `initial_kfs_map`、`current_kfs_map`、`removed_r2_blocks`、`r1_wait_blocks`、`conflict_blocks`、`current_block`、`current_step_index` |
| BT 用途 | 每个 ForestSteps step 后更新地图状态，供重试和 rosbag 分析 |

## 梅林执行接口

### `/r2/perception/validate_kfs_on_block`

| 项 | 内容 |
|---|---|
| 建议形式 | service `ValidateKFSOnBlock.srv` |
| 输入 | `block_id`、`expected_type`、`expected_occupancy`、`camera=ARM_DEPTH`、`max_age_sec` |
| 读取 | 机械臂深度相机、`/kfs_tracker/detection`、方块几何标定 |
| 输出 | `match`、`observed_type`、`confidence`、`pose`、`error_code`、`message` |
| BT 用途 | ForestSteps 每个高风险动作前做一次复核 |
| 失败语义 | 不吻合直接返回 failure，不在 BT 内反复检测，交给重试/重规划处理 |

### `/r2/forest/check_r1_kfs_removed`

| 项 | 内容 |
|---|---|
| 建议形式 | service 或带超时 action |
| 输入 | `block_id`、`expected_removed=true`、`confirm_frames`、`timeout_sec` |
| 读取 | 相机检测、KFS map、路径规划标记 |
| 输出 | `removed`、`confidence`、`last_seen_stamp`、`message` |
| BT 用途 | 路径必须经过 R1 KFS 阻碍位置时，先等 R1 移除；移除后短暂停顿再动 |

### `/r2/chassis/set_travel_mode`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `mode=GROUND/FOREST/TRANSITION/RECOVERY`、`target_height[]`、`timeout_sec` |
| 读取 | 四轮丝杠圈数/电流、四轮红外高度、IMU/底盘状态 |
| 输出执行 | 丝杠目标、底盘限速、保护状态 |
| result | `mode_reached`、`wheel_height[]`、`confidence`、`failure_reason` |
| BT 用途 | 进入树林前、离开树林后、恢复流程 |

### `/r2/forest/execute_forest_plan`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `ForestSteps`、`route_version`、`initial_map_version`、`current_block` |
| 内部调用 | `ValidateKFSOnBlock`、`CheckR1KFSRemoved`、`UpStairs`、`DownStairs`、`RotateOnStair`、`PickAdjacentKFS`、`PlacePayloadTemporarily`、`UpdateForestMapState` |
| feedback | `step_index`、`step_type`、`target_block_id`、`current_block`、`payload_state`、`map_version`、`state` |
| result | `success`、`final_block`、`payload_state`、`removed_r2_blocks[]`、`failure_reason` |
| BT 用途 | 替换 `ExecuteForestPlanPlaceholder` |
| 注意 | `ForestSteps` 当前只有 `MOVE/PICK/PLACE` 和 `target_block_id`，上下台阶、转向和高度差处理需在执行 action 内补全 |

### `/r2/manipulation/pick_adjacent_kfs`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `current_block`、`target_block`、`expected_type=R2_KFS`、`timeout_sec` |
| 读取 | 机械臂深度相机、吸盘真空、机械臂关节状态、payload |
| result | `picked`、`payload_state`、`target_block_now`、`failure_reason` |
| BT 用途 | 由 `ExecuteForestPlan` 内部调用 |

### `/r2/forest/place_payload_temporarily`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `current_block`、`target_block`、`payload_type=R2_KFS`、`reason` |
| 读取 | 机械臂相机、吸盘状态、方块占用 |
| result | `placed`、`new_payload_state`、`new_block_state`、`failure_reason` |
| BT 用途 | 遇到路径阻碍需要换取/让路时临时放下 R2 KFS |

### `/r2/forest/exit_via_block`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `exit_candidates=[10,11,12]`、`current_block`、`payload_required=R2_KFS` |
| 读取 | 地图状态机、规划器、底盘状态 |
| result | `exited`、`exit_block`、`final_pose`、`failure_reason` |
| BT 用途 | `ExitForestViaBlock10_11_12Placeholder` |

## 对抗区接口

### `/r2/motion/align_to_r1`

| 项 | 内容 |
|---|---|
| 建议形式 | action；若已有 `AlignToTarget.action` 可复用，应优先复用 |
| 输入 | `target=R1_CLIMB_POSE`、`aruco_hint`、`timeout_sec` |
| 读取 | R1 ArUco 位姿、前方/机械臂深度相机、雷达、TF |
| result | `aligned`、`pose_error`、`failure_reason` |
| BT 用途 | 爬上 R1 前对齐 |
| 当前缺口 | README 曾提到 `AlignToR1`，但 action 文件暂未发现 |

### `/r2/motion/place_kfs_on_grid`

| 项 | 内容 |
|---|---|
| 建议形式 | action |
| 输入 | `layer=2/3`、`cell_id`、`payload_required=R2_KFS`、`timeout_sec` |
| 读取 | R1 ArUco 层数指令、九宫格状态、机械臂深度相机、吸盘状态 |
| result | `placed`、`layer`、`cell_id`、`payload_after`、`failure_reason` |
| BT 用途 | `PlaceKFSOnGridLayer2Placeholder` / `PlaceKFSOnGridLayer3Placeholder` |
| 当前缺口 | `PlaceKFS.action` 只有 `PLACE_ON_MERLIN=0` 和 `PLACE_ON_GRID_LAYER2=1`，三层需扩展 |

## 建议新增/统一消息

| 名称 | 类型 | 用途 |
|---|---|---|
| `PayloadState.msg` | msg | `NONE/R2_KFS/UNKNOWN`、吸盘状态、置信度 |
| `R1ArUcoState.msg` | msg | ArUco 可见性、id、instruction、位姿、时间戳 |
| `KFSBlockState.msg` | msg | 单个方块的 KFS 类型、占用、来源、置信度 |
| `KFSMap.msg` | msg | 1-12 号方块初始/当前地图 |
| `ForestMapState.msg` | msg | `removed_r2_blocks`、步骤进度、冲突和等待记录 |
| `WheelHeightState.msg` | msg | 四轮红外、丝杠圈数/电流、目标高度 |
| `TravelModeState.msg` | msg | `GROUND/FOREST/TRANSITION/RECOVERY` |
| `ValidateKFSOnBlock.srv` | srv | 机械臂相机单次复核方块 KFS |
| `CheckR1KFSRemoved.srv` | srv | 相机确认 R1 KFS 是否已移除 |
| `RequestForestPlan.srv` | srv | 请求梅林规划器输出 ForestSteps |
| `ExecuteForestPlan.action` | action | 执行完整梅林动作序列 |
| `PlaceKFSOnGrid.action` | action | 对抗区二层/三层 KFS 放置 |

## 当前不接入的接口

| 接口方向 | 当前处理 |
|---|---|
| 端头夹取 | 当前主树不执行 |
| 兵器组装 | 当前主树不执行 |
| 非 ArUco 码 | 当前按 R1 ArUco 设计 |
| 行为树内部实现视觉/路径规划 | 不做，保持外部 action/service/topic |

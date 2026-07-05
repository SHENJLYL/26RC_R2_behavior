# R2 Action Interface Catalog

本文档是 R2 后续任务级 action/service/topic 的准备清单。命名为建议稿，不代表已经全部实现。

## 接口分层

| 层级 | 形式 | 作用 |
|---|---|---|
| 行为树 BT | XML + 小型 wrapper 节点 | 编排任务顺序、分支、重试和保护 |
| 任务级 action | ROS 2 action | 执行耗时动作，如导航、夹取、放置、穿越树林 |
| 状态 topic | ROS 2 topic | 持续发布载荷、二维码、KFS 地图、底盘高度等状态 |
| 查询/检查 service | ROS 2 service | 做瞬时规则检查、健康检查、策略选择 |
| 底层控制 topic/service | ROS 2 topic/service | 由底盘、机械臂、吸盘、视觉节点内部使用，BT 不直接细控 |

## 已有或当前优先复用接口

| 接口 | 类型 | 当前用途 | 后续建议 |
|---|---|---|---|
| `/manual_start` | topic `std_msgs/msg/Int32` | 行为树启动 | 保持现状 |
| `/r2/navigation/navigate_to_named_pose` | action | 命名航点导航 | 继续作为武馆、梅林入口、对抗区预备位导航入口 |
| `goal_pose` | topic `geometry_msgs/msg/PoseStamped` | 直接发 Nav2 目标 | 保留为调试/简化接口 |
| `cmd_vel` | topic `geometry_msgs/msg/Twist` | 停车或底盘速度 | 仅用于停车或底盘调试，不建议任务树长期直接控速度 |
| `/cmd_suction_suck` | topic `std_msgs/msg/Bool` | 吸盘开关候选接口 | 建议外面包一层任务级 action 并补反馈 |

## 全局状态与规则服务

### `/r2/health/check_start_ready`

建议类型：service 或短 action。

| 项 | 内容 |
|---|---|
| 目的 | 比赛开始前确认 R2 能否进入主流程 |
| 输入 | 期望模式 `competition/simulation/test`，必要传感器列表 |
| 读取 | TF 是否连通，导航 action 是否可用，雷达/相机/机械臂/吸盘/底盘状态时间戳，急停状态 |
| 输出 | `ready`、`blocking_errors[]`、`warnings[]` |
| BT 用法 | 替换 `PreMatchSelfCheckPlaceholder` |

### `/r2/safety/emergency_stop_state`

建议类型：topic。

| 项 | 内容 |
|---|---|
| 目的 | 向行为树提供急停或软停状态 |
| 读取 | 硬件急停、遥控急停、底盘保护、机械臂保护 |
| 输出 | `is_stop_requested`、`source`、`stamp` |
| BT 用法 | 替换 `IsEmergencyStopRequestedPlaceholder`，触发 `StopAllMotion` |

### `/r2/rule_guard/check`

建议类型：service。

| 项 | 内容 |
|---|---|
| 目的 | 在动作执行前检查是否违反规则 |
| 输入 | `proposed_action`、`current_zone`、`current_block`、`target_block`、`payload_state`、`kfs_map` |
| 读取 | 规则约束、KFS 类型、树林方块占用、载荷状态、R1 状态 |
| 输出 | `allowed`、`reason_code`、`reason_text`、`suggested_recovery` |
| BT 用法 | 放在抓取、进入树林、离开树林、进入对抗区、放置 KFS 前 |
| 关键规则 | 不触碰 R1 KFS/假KFS；不完全进入有 KFS 的树林方块；离开树林前携带至少一个 R2 KFS；经 10/11/12 号方块离开树林；R2 携带 R2 KFS 才进入对抗区 |

### `/r2/perception/payload_state`

建议类型：topic。

| 项 | 内容 |
|---|---|
| 目的 | 记录 R2 当前是否携带 KFS 或端头 |
| 读取 | 吸盘真空压力、吸盘开关状态、机械臂相机确认、夹爪状态 |
| 输出 | `payload_type` = `NONE/R2_KFS/R1_KFS/FAKE_KFS/TIP/UNKNOWN`，`confidence`，`held_by` = `SUCTION/GRIPPER/NONE`，`stamp` |
| BT 用法 | 抓取前确认空载，离开树林前确认携带 R2 KFS，对抗区放置后确认空载 |

## 武馆 action

### `/r2/manipulation/pick_tip`

建议类型：action `PickTip.action`。

| 项 | 内容 |
|---|---|
| 目的 | R2 从端头架夹取一个端头 |
| 输入 | `rack_pose_hint`、`target_slot_policy`、`team_side`、`timeout_sec` |
| 读取 | 机械臂深度相机、夹爪开合/电流/力反馈、端头架位姿、机械臂关节状态 |
| 输出执行 | 机械臂运动、夹爪闭合、必要时微调对位 |
| 结果 | `success`、`held_tip`、`tip_pose`、`slot_id`、`failure_reason` |
| BT 用法 | 替换 `DetectAndPickSpearheadPlaceholder` |
| 规则注意 | 每次只能接触、拿起和移动一个端头；不能影响对方抓取其它端头 |

### `/r2/manipulation/assemble_tip_to_pole`

建议类型：action `AssembleTipToPole.action`。

| 项 | 内容 |
|---|---|
| 目的 | R2 持端头，与 R1 持长杆完成兵器组装 |
| 输入 | `assembly_pose_hint`、`r1_state_hint`、`timeout_sec` |
| 读取 | R1 二维码状态、机械臂深度相机、夹爪状态、端头姿态、长杆/快速接头视觉目标 |
| 输出执行 | 导航微调或底盘对齐、机械臂插接动作、夹爪释放 |
| 结果 | `assembled`、`weapon_id`、`tip_released`、`failure_reason` |
| BT 用法 | 替换 `AssembleWeaponPlaceholder` |
| 规则注意 | R2 不得接触 R1 抓住的长杆；R1/R2 不得发生直接肢体接触 |

### `/r2/perception/read_r1_qr_state`

建议类型：action 或 service `ReadR1StateQRCode`。

| 项 | 内容 |
|---|---|
| 目的 | 在能看到 R1 二维码时读取 R1 状态 |
| 输入 | `expected_states[]`、`max_age_sec`、`timeout_sec` |
| 读取 | 前方深度相机或机械臂深度相机图像、二维码检测、二维码位姿估计 |
| 输出 | `visible`、`state`、`confidence`、`qr_pose`、`stamp` |
| BT 用法 | 判断 R1 是否离开武馆、是否进入梅林、对抗区是否可对齐九宫格 |
| 状态建议 | `UNKNOWN`、`R1_LEFT_MC`、`R1_ENTERED_MF`、`R1_READY_FOR_ASSEMBLY`、`R1_AT_GRID_ALIGN_POSE`、`NEXT_TASK_HINT` |
| 关键约束 | 二维码只作为机会性感知来源，必须允许不可见和过期 |

## 梅林与树林 action

### `/r2/chassis/set_travel_mode`

建议类型：action `SetTravelMode.action`。

| 项 | 内容 |
|---|---|
| 目的 | 切换普通地面、树林、过渡等底盘抬升模式 |
| 输入 | `mode` = `GROUND/FOREST/TRANSITION/RECOVERY`，`target_height[]`，`timeout_sec` |
| 读取 | 四轮丝杠圈数/电流、四轮红外高度、底盘姿态、轮端状态 |
| 输出执行 | 丝杠电机目标、底盘速度限制、抬升过程保护 |
| 结果 | `mode_reached`、`wheel_height[]`、`confidence`、`failure_reason` |
| BT 用法 | 进入树林前、退出树林后、异常恢复时调用 |

### `/r2/chassis/level_on_blocks`

建议类型：action `LevelOnBlocks.action`。

| 项 | 内容 |
|---|---|
| 目的 | 在高低不同的树林方块上保持车体可移动姿态 |
| 输入 | `current_block`、`support_blocks[]`、`level_tolerance` |
| 读取 | 四轮红外相对高度、丝杠圈数/电流、IMU/底盘姿态、轮速 |
| 输出执行 | 四轮丝杠微调、底盘速度上限 |
| 结果 | `leveled`、`height_error[]`、`blocked_wheel[]`、`failure_reason` |
| BT 用法 | `StepToAdjacentBlock` 内部调用，或作为 BT 前置保护动作 |

### `/r2/perception/update_kfs_map`

建议类型：action 或 service。

| 项 | 内容 |
|---|---|
| 目的 | 将视觉检测转换为 1-12 号树林方块的 KFS 占用图 |
| 输入 | `zone`、`team_side`、`known_block_layout`、`timeout_sec` |
| 读取 | 机械臂深度相机、前方深度相机、激光雷达、方块几何标定、KFS 分类模型 |
| 输出 | `kfs_map[1..12]`，每格含 `occupancy`、`kfs_type`、`pose`、`confidence`、`last_seen` |
| BT 用法 | 进入树林前、每次移动/抓取后刷新 |
| 分类 | `EMPTY/R1_KFS/R2_KFS/FAKE_KFS/UNKNOWN/OCCLUDED` |

### `/r2/forest/plan_single_kfs_task`

建议类型：action 或 service。

| 项 | 内容 |
|---|---|
| 目的 | 在一次只能携带一个 KFS 的约束下规划 R2 收集与离开树林 |
| 输入 | `current_block`、`entry_pose`、`kfs_map`、`payload_state`、`exit_candidates=[10,11,12]` |
| 读取 | 树林方块邻接图、KFS 类型、规则检查结果、底盘可达性 |
| 输出 | `plan_steps[]`，包含 `MOVE_TO_BLOCK/PICK_ADJACENT_KFS/PLACE_PAYLOAD/EXIT_FOREST/REPLAN` |
| BT 用法 | 替换 `CollectSingleR2KFSPlaceholder` 和 `ExitForestViaBlock10_11_12Placeholder` 的规划部分 |
| 规则注意 | 若 1/2/3 号方块有 R2 KFS，应从 R2 入口处优先收集第一个 KFS；不得选择 R1 KFS/假KFS/UNKNOWN 为抓取目标 |

### `/r2/forest/step_to_adjacent_block`

建议类型：action `StepToAdjacentBlock.action`。

| 项 | 内容 |
|---|---|
| 目的 | 在树林方块间移动到相邻方块或指定边界位置 |
| 输入 | `from_block`、`to_block`、`target_pose_on_block`、`avoid_occupied_blocks` |
| 读取 | 四轮红外、丝杠圈数/电流、激光雷达、前方深度相机、里程计、TF、KFS map |
| 输出执行 | 底盘 `cmd_vel`、丝杠抬升、必要时暂停/找平/重规划 |
| 结果 | `arrived`、`current_block`、`pose_error`、`contact_risk`、`failure_reason` |
| BT 用法 | 梅林内部移动原语，建议由森林 action server 内部调用 |

### `/r2/manipulation/pick_adjacent_kfs`

建议类型：action `PickAdjacentKFS.action`。

| 项 | 内容 |
|---|---|
| 目的 | 从当前所在方块的相邻方块抓取一个 R2 KFS |
| 输入 | `current_block`、`target_block`、`expected_type=R2_KFS`、`timeout_sec` |
| 读取 | 机械臂深度相机、KFS pose、KFS 分类、吸盘真空反馈、机械臂关节状态、payload_state |
| 输出执行 | 机械臂对位、吸盘开启、抬起确认 |
| 结果 | `picked`、`payload_state`、`target_block_now`、`failure_reason` |
| BT 用法 | 替换 `PickAdjacentKFSMock` |
| 规则注意 | 只能取相邻方块上的 R2 KFS；如果 payload 不为 `NONE`，必须先拒绝或执行放置/换取流程 |

### `/r2/forest/place_payload_temporarily`

建议类型：action `PlacePayloadTemporarily.action`。

| 项 | 内容 |
|---|---|
| 目的 | 当已携带 KFS 且必须处理路径阻碍时，将当前 KFS 临时放到允许位置 |
| 输入 | `preferred_block_or_pose`、`reason`、`timeout_sec` |
| 读取 | payload_state、机械臂深度相机、放置区域可用性、规则检查 |
| 输出执行 | 机械臂放置、吸盘释放、payload_state 更新 |
| 结果 | `placed`、`placed_pose`、`placed_block`、`payload_state_after`、`failure_reason` |
| BT 用法 | 仅在 `resolve_blocking_kfs` 或异常流程中使用 |
| 风险 | 需要规则与裁判口径确认，尤其是临时放置位置是否会影响后续计分或重试 |

### `/r2/forest/resolve_blocking_kfs`

建议类型：action `ResolveBlockingKFS.action`。

| 项 | 内容 |
|---|---|
| 目的 | 处理必经路径上存在 R2 必须自行解决的 KFS 阻碍 |
| 输入 | `blocked_edge_or_block`、`current_plan`、`payload_state`、`kfs_map` |
| 读取 | KFS 类型、路径阻塞原因、payload_state、规则检查、机械臂相机 |
| 输出执行 | 必要时临时放下已携带 KFS，再抓取/转移阻碍的 R2 KFS，然后重规划 |
| 结果 | `resolved`、`new_kfs_map`、`new_payload_state`、`next_plan_hint`、`failure_reason` |
| BT 用法 | 高风险后续能力，建议先做离线仿真和规则审查，不作为第一批实车动作 |
| 硬约束 | 不得触碰 R1 KFS、假KFS 或分类不可信目标 |

### `/r2/forest/exit_via_block`

建议类型：action `ExitForestViaBlock.action`。

| 项 | 内容 |
|---|---|
| 目的 | 从 10/11/12 号方块之一离开树林 |
| 输入 | `exit_block`、`payload_required=true`、`timeout_sec` |
| 读取 | payload_state、current_block、方块邻接图、底盘高度状态 |
| 输出执行 | 方块路径跟随、底盘模式切换 |
| 结果 | `exited`、`exit_block`、`payload_state`、`failure_reason` |
| BT 用法 | 替换 `ExitForestViaBlock10_11_12Placeholder` |

## 对抗区 action

### `/r2/navigation/navigate_to_battlefield_standoff`

建议类型：复用 `/r2/navigation/navigate_to_named_pose`。

| 项 | 内容 |
|---|---|
| 目的 | 从梅林出口移动到对抗区九宫格预备位 |
| 输入 | 命名航点，如 `battlefield_grid_standoff` |
| 读取 | Nav2 定位、地图、雷达、里程计 |
| 输出执行 | 导航 action 内部控制底盘 |
| 结果 | `success`、`final_pose`、`failure_reason` |
| BT 用法 | 放在 `Battlefield_PlaceMiddleLayer` 开始处 |

### `/r2/perception/detect_grid_state`

建议类型：action 或 service。

| 项 | 内容 |
|---|---|
| 目的 | 识别九宫格位姿、空格和已占用格 |
| 输入 | `layer_filter=MIDDLE`、`team_side`、`timeout_sec` |
| 读取 | 前方深度相机、机械臂深度相机、R1 二维码对齐提示、九宫格几何模型 |
| 输出 | `grid_pose`、`cells[3x3]`，每格含 `layer`、`occupied`、`confidence`、`cell_pose` |
| BT 用法 | 替换 `ReadGridStateMock` |

### `/r2/strategy/select_grid_cell`

建议类型：service。

| 项 | 内容 |
|---|---|
| 目的 | 选择 R2 中层放置目标格 |
| 输入 | `grid_state`、`payload_state`、`r1_state_hint`、`score_policy` |
| 读取 | 九宫格占用、本队策略、R1 二维码状态、规则检查 |
| 输出 | `target_cell`、`target_layer=MIDDLE`、`reason` |
| BT 用法 | 替换 `SelectMiddleLayerCellMock` |

### `/r2/manipulation/place_kfs_middle`

建议类型：action `PlaceKFSMiddle.action`。

| 项 | 内容 |
|---|---|
| 目的 | 将当前携带的 KFS 放入九宫格中层空格 |
| 输入 | `target_cell`、`grid_pose`、`payload_expected=R2_KFS`、`timeout_sec` |
| 读取 | payload_state、机械臂深度相机、九宫格 cell pose、吸盘真空反馈、机械臂关节状态 |
| 输出执行 | 机械臂放置轨迹、吸盘释放、撤出格子虚拟表面 |
| 结果 | `placed`、`cell_occupied`、`payload_state_after=NONE`、`failure_reason` |
| BT 用法 | 替换 `PlaceMiddleLayerPlaceholder` |
| 规则注意 | 只能放入中层空格，每次只能放一个，不能向非空格放置 |

## 重试与恢复 action

### `/r2/retry/execute_area_aware_retry`

建议类型：action。

| 项 | 内容 |
|---|---|
| 目的 | 根据当前区域执行安全重试或请求人工恢复 |
| 输入 | `current_zone`、`reason_code`、`payload_state`、`last_known_pose` |
| 读取 | 裁判/操作员重试请求、定位、payload_state、规则状态 |
| 输出执行 | 停车、释放或保持机构、安全姿态、导航到重试区或等待人工 |
| 结果 | `retry_ready`、`target_retry_area`、`state_after_retry`、`failure_reason` |
| BT 用法 | 替换 `ExecuteAreaAwareRetryPlaceholder` |

## 建议优先实现的自定义 message/action

| 名称 | 形式 | 用途 |
|---|---|---|
| `PayloadState.msg` | msg | 全局载荷状态 |
| `KFSBlockState.msg` | msg | 单个树林方块的 KFS 分类与位姿 |
| `KFSMap.msg` | msg | 1-12 号方块占用图 |
| `R1QRCodeState.msg` | msg | R1 二维码状态、可见性和时间戳 |
| `WheelHeightState.msg` | msg | 四轮红外高度、丝杠圈数/电流 |
| `RuleGuardCheck.srv` | srv | 规则检查 |
| `PickTip.action` | action | 夹取端头 |
| `PickAdjacentKFS.action` | action | 夹取相邻 R2 KFS |
| `PlaceKFSMiddle.action` | action | 中层放置 KFS |
| `StepToAdjacentBlock.action` | action | 树林方块移动 |
| `SetTravelMode.action` | action | 切换底盘抬升模式 |


# R2 Behavior Action Sequence

本文档把当前行为树流程映射到后续 action/service/topic 接入顺序。目标是让行为树保持简洁：XML 负责任务流程，外部 action server 负责具体执行。

## 当前主树骨架

```text
R2_Competition_Main
├── EmergencyStopBranch
├── RetryRecoveryBranch
└── CompetitionMission
    ├── PreMatchAndStart
    ├── MC_AssembleWeapon
    ├── WaitForR1BeforeR2LeavesMC
    ├── MF_CollectSingleR2KFS
    ├── MF_ExitForest
    ├── Battlefield_PlaceMiddleLayer
    └── StopAllMotion
```

## 0. 全局保护分支

### EmergencyStopBranch

| 当前节点 | 后续接口 | 进入条件 | 成功/输出 |
|---|---|---|---|
| `IsEmergencyStopRequestedPlaceholder` | `/r2/safety/emergency_stop_state` | 急停、软停或严重安全故障 | 返回 true 后执行 `StopAllMotion` |
| `StopAllMotion` | `PublishTwist cmd_vel=0`，同时底层急停链路 | 需要停车 | 底盘停，机械臂进入安全策略 |

### RetryRecoveryBranch

| 当前节点 | 后续接口 | 进入条件 | 成功/输出 |
|---|---|---|---|
| `IsRetryRequestedPlaceholder` | `/r2/retry/request` | 裁判/操作员/规则守卫请求重试 | 返回 true |
| `ExecuteAreaAwareRetryPlaceholder` | `/r2/retry/execute_area_aware_retry` | 已确认重试 | 根据当前区域回到允许重试状态 |

## 1. PreMatchAndStart

```text
PreMatchSelfCheck
-> WaitForKFSMapConfirmation
-> IsManualStart
```

| 当前节点 | 后续接口 | 读取信息 | 成功条件 |
|---|---|---|---|
| `PreMatchSelfCheckPlaceholder` | `/r2/health/check_start_ready` | TF、导航 action、雷达、相机、机械臂、吸盘、底盘、急停 | 所有关键模块 ready，或只存在允许 warning |
| 新增建议节点 `WaitForKFSMapConfirmation` | `/r2/setup/wait_for_kfs_map_confirmation` | `/r2/perception/kfs_map`、网页人工确认状态、map 校验结果 | 赛前 KFS map 已确认且锁定 |
| `IsManualStart` | `/manual_start` | `std_msgs/msg/Int32` | 收到 `>= 1` |

行为树策略：

- `IsManualStart` 保持现状。
- 健康检查失败时不进入后续任务，输出清晰的 `blocking_errors[]`。
- 如果使用网页人工确认 KFS 位置，则必须在比赛开始前完成确认并锁定 map。
- 行为树不直接依赖网页页面，只等待 ROS 层的 `/r2/perception/kfs_map confirmed=true locked=true`。

### 1.1 赛前网页 KFS map 确认

```text
OpponentPlacesKFS
-> OperatorConfirmsInWebPage
-> SubmitManualKFSMap
-> ValidateAndLockMap
-> PublishUnifiedKFSMap
```

| 能力 | 后续接口 | 进入条件 | 输出 |
|---|---|---|---|
| 网页提交人工 map | `/r2/setup/submit_manual_kfs_map` | 对方完成梅林 KFS 放置，操作员完成 1-12 号方块录入 | `accepted`、`map_version`、`validation_errors[]` |
| 等待 map 确认 | `/r2/setup/wait_for_kfs_map_confirmation` | 行为树启动前或进入梅林前 | `confirmed=true`、`locked=true`、`source=MANUAL_SETUP` |
| 发布统一 map | `/r2/perception/kfs_map` | 人工 map 校验通过 | 供 rule_guard、forest planner、pick action 读取 |

校验规则：

- 1-12 号方块编号不可重复。
- 开局应有 3 个 `R1_KFS`、4 个 `R2_KFS`、1 个 `FAKE_KFS`。
- 假KFS 不应位于 1/2/3 号入口方块。
- 未确认位置必须显式标记为 `UNKNOWN`，不能默认为 `EMPTY`。
- 锁定后的人工 map 作为 `initial_kfs_map` 保存，比赛中另用 `current_kfs_map` 随动作结果更新。

## 2. MC_AssembleWeapon

```text
NavigateToNamedPose(spearhead_rack_standoff)
-> PickTip
-> Navigate/AlignToAssemblyPose
-> AssembleTipToPole
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `NavigateToNamedPose target=spearhead_rack_standoff` | 已接入 `/r2/navigation/navigate_to_named_pose` | map/odom/TF、雷达、导航航点 | 底盘导航 | 到达端头架前停靠点 |
| `DetectAndPickSpearheadPlaceholder` | `/r2/manipulation/pick_tip` | 机械臂深度相机、端头位姿、夹爪反馈 | 机械臂夹取端头 | `payload_state=TIP` 或 `held_tip=true` |
| `PubNav2Goal goal=0.0;0.0;0.0` | 建议替换为命名航点 `/r2/navigation/navigate_to_named_pose` | 装配位航点 | 底盘导航/对齐 | 到达装配位 |
| `AssembleWeaponPlaceholder` | `/r2/manipulation/assemble_tip_to_pole` | R1 二维码、长杆/端头视觉、夹爪状态 | 插接、释放端头 | `assembled=true`，`payload_state=NONE` |

规则约束：

- R2 每次只能处理一个端头。
- R2 不应影响对方端头抓取。
- 组装时 R2 不得接触 R1 抓住的长杆，R1/R2 不得直接肢体接触。

## 3. WaitForR1BeforeR2LeavesMC

```text
StopAllMotion
-> WaitUntilR1FullyEnteredMF
```

| 当前节点 | 后续接口 | 读取信息 | 成功条件 |
|---|---|---|---|
| `StopAllMotion` | `PublishTwist cmd_vel=0` | 无 | 底盘停车 |
| `WaitUntilR1FullyEnteredMFPlaceholder` | `/r2/perception/read_r1_qr_state` + 区域/视觉判断 | R1 二维码、R1 位姿、场地区域判断 | R1 已满足离开武馆/进入梅林所需条件 |

设计建议：

- 可见二维码时直接更新 `r1_qr_state`。
- 二维码不可见时允许读取最近可信状态，但必须检查时间戳。
- 如果无法确认规则条件，行为树应等待或进入人工/重试，不应盲目离开武馆。

## 4. MF_CollectSingleR2KFS

```text
NavigateToForestEntry
-> SetTravelMode(FOREST)
-> WaitOrUpdateKFSMap
-> PlanSingleKFSTask
-> Step/Align
-> PickAdjacentKFS
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `PubNav2Goal goal=0.0;0.0;0.0` | 建议替换为命名航点 `forest_entry_standoff` | 导航定位、雷达 | 底盘导航 | 到达 R2 入口附近 |
| `CollectSingleR2KFSPlaceholder` | `/r2/chassis/set_travel_mode` | 四轮红外、丝杠圈数/电流 | 抬升/模式切换 | 进入 `FOREST` 模式 |
| 同上 | `/r2/setup/wait_for_kfs_map_confirmation` 或 `/r2/perception/update_kfs_map` | 网页人工确认 map、机械臂深度相机、前方深度相机、KFS 分类 | 生成或校验方块占用图 | `kfs_map confirmed=true` 且规划可用 |
| 同上 | `/r2/forest/plan_single_kfs_task` | KFS map、方块邻接图、payload_state、rule_guard | 输出步骤计划 | 有合法目标和路线 |
| 同上 | `/r2/forest/step_to_adjacent_block` | 红外、丝杠、雷达、深度、odom | 底盘移动/找平 | 到达合法方块或抓取位 |
| 同上 | `/r2/manipulation/pick_adjacent_kfs` | KFS 位姿、分类、吸盘反馈 | 机械臂吸取 | `payload_state=R2_KFS` |

规则约束：

- R2 只应收集 R2 KFS。
- 不得接触 R1 KFS 或假KFS。
- 只能从当前位置拿取相邻方块上的 R2 KFS。
- 如果 1/2/3 号方块上有 R2 KFS，应从 R2 入口处优先收集第一个 KFS。
- R2 不应完全进入有 KFS 的树林方块。
- 如果采用网页人工 map，规划器可以先以人工 map 为初始依据；抓取/移动后必须根据 action 结果更新 `current_kfs_map`。

## 5. 路径阻碍与单载荷处理

这是后续高风险能力，建议晚于基础取放闭环实现。

```text
DetectBlockingKFS
-> RuleGuardCheck
-> PlacePayloadTemporarily(optional)
-> ResolveBlockingKFS
-> UpdateKFSMap
-> Replan
```

| 能力 | 后续接口 | 进入条件 | 输出 |
|---|---|---|---|
| 阻碍判断 | `/r2/forest/plan_single_kfs_task` | 必经路径被 R2 可处理 KFS 阻碍 | `BLOCKED_BY_R2_KFS` |
| 临时放下当前 KFS | `/r2/forest/place_payload_temporarily` | 当前已携带一个 KFS，且必须空载处理阻碍 | `payload_state=NONE`，记录放置位置 |
| 处理阻碍 | `/r2/forest/resolve_blocking_kfs` | 阻碍目标确认是 R2 KFS | 新 KFS map、新 payload_state |
| 重新规划 | `/r2/forest/plan_single_kfs_task` | 地图变化后 | 新 plan |

硬约束：

- 车辆只能携带一个吸盘 KFS。
- 分类为 `UNKNOWN` 的目标不能抓。
- 假KFS和 R1 KFS 不能作为阻碍处理目标。
- 临时放置策略需要单独做规则审核和实车验证。

## 6. MF_ExitForest

```text
CheckPayload
-> PlanExitViaBlock10_11_12
-> FollowForestExitPath
-> SetTravelMode(GROUND/TRANSITION)
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `ExitForestViaBlock10_11_12Placeholder` | `/r2/perception/payload_state` | 吸盘、视觉确认 | 无 | `payload_state=R2_KFS` |
| 同上 | `/r2/forest/plan_single_kfs_task` 或 `/r2/forest/exit_via_block` | current_block、exit candidates | 出口路径 | 选中 10/11/12 |
| 同上 | `/r2/forest/step_to_adjacent_block` | 红外、丝杠、雷达、深度、odom | 方块移动 | 到达出口方块 |
| 同上 | `/r2/chassis/set_travel_mode` | 红外、丝杠 | 底盘模式切换 | 安全离开树林 |

规则约束：

- 离开树林前必须携带至少一个 R2 KFS。
- 必须经过 10、11 或 12 号方块之一离开树林。

## 7. Battlefield_PlaceMiddleLayer

```text
NavigateToBattlefield
-> ReadR1QRCodeOrGridHint
-> DetectGridState
-> SelectMiddleLayerCell
-> PlaceKFSMiddle
```

| 当前节点 | 后续接口 | 读取信息 | 输出执行 | 成功条件 |
|---|---|---|---|---|
| `PubNav2Goal goal=0.0;0.0;0.0` | 建议替换为命名航点 `battlefield_grid_standoff` | 导航定位、雷达、地图 | 底盘导航 | 到达九宫格预备位 |
| `PlaceMiddleLayerPlaceholder` | `/r2/perception/read_r1_qr_state` | R1 二维码、位姿提示 | 更新对齐参考 | 可见则用于辅助对齐，不可见则降级 |
| 同上 | `/r2/perception/detect_grid_state` | 前方/机械臂深度相机、九宫格几何 | 九宫格位姿和占用 | 中层格状态可信 |
| 同上 | `/r2/strategy/select_grid_cell` | grid_state、payload_state、R1 状态 | 目标 cell | 选中空的中层格 |
| 同上 | `/r2/manipulation/place_kfs_middle` | cell pose、吸盘、机械臂相机 | 机械臂放置、吸盘释放 | KFS 在目标中层格，`payload_state=NONE` |

规则约束：

- R2 携带一个或多个 R2 KFS 才能进入对抗区。
- R2 当前目标是中层放置。
- 每次只能放一个 KFS。
- 不得向非空格放置。

## 8. 建议接入节奏

### 第一阶段：树和接口可观测

- 接入 `/r2/health/check_start_ready`。
- 接入网页人工 KFS map 提交与 `/r2/setup/wait_for_kfs_map_confirmation`。
- 接入 `/r2/perception/payload_state`。
- 接入 `/r2/rule_guard/check`。
- rosbag 记录所有关键状态。

### 第二阶段：武馆闭环

- 标定 `spearhead_rack_standoff` 和装配位。
- 接入 `/r2/manipulation/pick_tip`。
- 接入 `/r2/manipulation/assemble_tip_to_pole`。
- 接入 R1 二维码读取或本地区域判断。

### 第三阶段：梅林基础闭环

- 接入底盘抬升模式和轮端高度状态。
- 接入 `kfs_map`，优先支持赛前网页人工确认作为初始 map。
- 只做一个最简单合法目标：识别一个相邻 R2 KFS 并吸取。
- 暂不做复杂阻碍重排。

### 第四阶段：梅林完整路径

- 接入方块图规划。
- 接入 10/11/12 出口。
- 接入路径阻碍与单载荷处理。

### 第五阶段：对抗区放置

- 接入九宫格识别。
- 接入 R1 二维码辅助对齐。
- 接入中层放置。

## 9. 每个 action 的最小验收记录

每个 action 接入后，都应至少能在 rosbag 中回答：

1. action 什么时候收到 goal？
2. goal 输入是什么？
3. action 执行期间读取了哪些关键状态？
4. action 输出了哪些底盘/机械臂/吸盘命令？
5. 最终 success 还是 failure？
6. 如果失败，`failure_reason` 是什么？
7. 行为树是否根据结果进入正确下一步或恢复分支？

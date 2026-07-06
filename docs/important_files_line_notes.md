# 重要文件注释

日期：2026-07-06

本文档是当前 no-tip 行为树的阅读辅助，不参与编译。旧版端头夹取、兵器组装、非 ArUco 码和单纯中层放置说明已移除；当前说明以“武馆/梅林交界等待 -> 梅林树林 KFS -> 对抗区爬上 R1 后二/三层放置”为准。

表格中的“行”使用当前文件行号；多行 XML 标签按起止行合并解释，避免把同一个节点拆散后反而难读。

## 1. `behavior_trees/r2_competition_main.xml`

当前正式主树安全骨架。已注册节点为 `IsManualStart`、`NavigateToNamedPose`、`PubNav2Goal`、`PublishTwist`；其它实车能力以 `AlwaysFailure` 占位，避免误认为完整实车流程已经接通。

| 行 | 内容 | 注释 |
|---:|---|---|
| 1-2 | XML 声明与 `<root BTCPP_format="4">` | BehaviorTree.CPP XML 根节点，格式版本为 4。 |
| 4-12 | `StopAllMotion` | 停车子树，通过 `PublishTwist` 向 `cmd_vel` 发布零速度。 |
| 14-23 | `PreMatchAndStart` | 赛前检查、等待 `/manual_start`、声明当前策略跳过端头与兵器组装。 |
| 16-17 | `PreMatchSelfCheckPlaceholder` | 未来替换为 `/r2/health/check_start_ready`。 |
| 18-19 | `IsManualStart` | 已接入真实节点，`/manual_start >= 1` 时成功。 |
| 20-21 | `SkipTipPickupAndWeaponAssemblyByStrategy` | 语义标记：当前主流程不执行端头任务。 |
| 25-43 | `MC_WaitAtMFBoundary` | R2 先到武馆/梅林交界等待，再满足 120 秒或 ArUco 条件后进入梅林。 |
| 27-31 | `NavigateToNamedPose target="mc_mf_boundary_standoff"` | 命名航点导航到交界等待位，当前需要标定真实坐标。 |
| 32 | `SubTree StopAllMotion` | 到达等待位后先停车。 |
| 34-41 | `Wait120SecOrArUcoContinue` | 未来接入比赛计时或 R1 ArUco 继续指令。 |
| 45-69 | `MF_EnterForestEntry` | 进入梅林树林入口，正常分支失败时使用 ArUco 恢复分支。 |
| 47-55 | `NormalEnterMF` | 导航到 2 号平台面前，并切换树林行驶模式。 |
| 51-54 | `SetTravelModeForest` / `ConfirmChassisLift...` 占位 | 未来接入丝杠、红外、激光雷达确认。 |
| 57-67 | `ArucoContinueRecovery` | R2 卡住或未能进入梅林时，读 R1 ArUco 后再次尝试到树林入口。 |
| 71-96 | `MF_ExecuteForestPlan` | 梅林树林核心阶段：KFS map、规划、地图状态机、执行 ForestSteps。 |
| 73-74 | `WaitForManualKFSMapPlaceholder` | 未来读取 `web_spoiler/msg/WebSpoiler` 的 `/kfs_locator/state` 并生成 `/r2/perception/kfs_map`。 |
| 76-77 | `RequestForestPlanPlaceholder` | 未来封装 `meilin_router/router_planner`；当前 planner 仍是 `topic1 -> topic2`。 |
| 79-80 | `InitializeForestMapStatePlaceholder` | 未来初始化 `initial_kfs_map/current_kfs_map/removed_r2_blocks`。 |
| 82-90 | `ExecuteForestPlanPlaceholder` | 未来按 ForestSteps 执行移动、`ValidateKFSOnBlock` 单次复核、等待 R1 KFS 移除、抓取、临时放置。 |
| 92-94 | payload、出口、地面模式占位 | 离开树林前必须携带 R2 KFS，经 10/11/12 出口并切回普通地面。 |
| 98-125 | `Battlefield_ClimbAndPlaceKFS` | 对抗区流程：到九宫格附近、等 R1 对齐、爬上 R1、按 ArUco 放二层或三层。 |
| 100-103 | `NavigateToNamedPose battlefield_grid_standoff` | 导航到对抗区九宫格预备点并停车。 |
| 105-110 | ArUco 对齐、R1 对齐、上台阶占位 | 未来接入 `ReadArUco`、`AlignToR1/AlignToTarget`、`UpStairs`。 |
| 112-121 | `PlaceByArUcoLayerCode` | 根据 R1 ArUco 分支选择二层或三层放置。 |
| 123 | `RequirePayloadNonePlaceholder` | 放置后确认吸盘不再携带 KFS。 |
| 127-150 | `R2_Competition_Main` | 根树：急停优先，其次区域重试，最后执行 no-tip 主任务。 |
| 129-133 | `EmergencyStopBranch` | 未来接入 `/r2/safety/emergency_stop_state`。 |
| 135-139 | `RetryRecoveryBranch` | 未来接入区域感知重试恢复。 |
| 141-148 | `CompetitionMissionNoTip` | 当前完整任务顺序。 |

## 2. `behavior_trees/r2_full_flow_verification.xml`

完整流程验证树。它使用 `AlwaysSuccess` mock 验证 no-tip 主流程顺序，不验证底层动作是否真实完成。

| 行 | 内容 | 注释 |
|---:|---|---|
| 1-2 | XML 声明与根节点 | BT.CPP XML 入口。 |
| 4-12 | `Flow_StopAllMotion` | 验证用停车子树，仍使用真实 `PublishTwist`。 |
| 14-21 | `Flow_PreMatchAndStart_NoTip` | 模拟赛前检查通过，真实等待 `/manual_start`，然后跳过端头任务。 |
| 23-29 | `Flow_MC_WaitAtMFBoundary` | 模拟到交界处并满足 120 秒或 ArUco 继续条件。 |
| 31-37 | `Flow_MF_EnterForestEntry` | 模拟到 2 号平台面前、切换树林模式并确认抬升。 |
| 39-51 | `Flow_MF_ExecuteForestPlan` | 模拟 KFS map、规划、机械臂相机复核、等待 R1 KFS 移除、执行 ForestSteps、记录移除方块、离开树林。 |
| 53-64 | `Flow_Battlefield_ClimbAndPlaceKFS` | 模拟对抗区导航、R1 对齐、爬上 R1、按 ArUco 放二/三层、确认空载。 |
| 66-88 | `R2_Full_Flow_Verification` | 根树保持急停和重试结构，但默认不触发，最终顺序跑完整个 no-tip mock 流程。 |

## 3. `behavior_trees/r2_no_tip_competition_draft.xml`

详细 no-tip 草稿树。它比 `r2_competition_main.xml` 有更多注释，用于后续封装接口时对照语义。

| 行 | 内容 | 注释 |
|---:|---|---|
| 4-20 | 文件头注释 | 说明该树跳过端头任务，未实现接口用占位节点表示。 |
| 22-30 | `StopAllMotion` | 停车子树。 |
| 32-44 | `NoTip_PreMatchAndStart` | 健康检查占位、手动启动、跳过端头策略。 |
| 46-68 | `NoTip_MoveToMFBoundaryAndWait` | 导航到交界等待位，等待 120 秒或 R1 ArUco。 |
| 70-92 | `NoTip_EnterMFWithRecovery` | 正常进梅林或通过 ArUco 恢复进梅林。 |
| 94-132 | `NoTip_ForestPlanAndExecute` | 详细描述网页 KFS map、规划、地图状态机、机械臂相机复核、R1 KFS 移除判断、移动/抓取/临时放置。 |
| 134-170 | `NoTip_BattlefieldGridPlacement` | 到对抗区、等 R1 对齐、爬上 R1、按 ArUco 放二/三层。 |
| 172-179 | `NoTip_RetryRecovery` | 区域感知重试恢复入口。 |
| 181-201 | `R2_NoTip_Competition_Main` | no-tip 草稿主树入口。 |

## 4. `src/r2_behavior_server.cpp`

行为树运行器。它负责加载 XML、注册节点、订阅 `/manual_start` 并周期 tick 行为树。

| 行 | 内容 | 注释 |
|---:|---|---|
| 5-15 | include | 引入 server 头文件和当前已实现的 BT 插件节点。 |
| 17 | `using namespace std::chrono_literals` | 允许使用 `500ms` 等 chrono 字面量。 |
| 22-47 | 构造函数 | 声明参数、订阅 `/manual_start`、注册节点、加载树、创建 tick 定时器。 |
| 25-29 | 参数声明 | `behavior_tree_file`、`behavior_tree_directory`、`target_tree`、`tick_frequency`。 |
| 36-38 | `manual_start_sub_` | 订阅 `manual_start`，把整型值保存到共享原子变量。 |
| 49-76 | `registerNodes()` | 注册 `IsManualStart`、`PubNav2Goal`、`PublishTwist`、`NavigateToNamedPose`。 |
| 78-94 | `loadTree()` | 从单文件或目录加载 XML，并创建 `target_tree_` 指定的树。 |
| 96-107 | `tickTree()` | 周期 tick 根节点，并在状态变化时输出日志。 |
| 111-117 | `main()` | 初始化 ROS、启动 server、退出时 shutdown。 |

## 5. `plugins/action/navigate_to_named_pose.cpp`

命名航点导航 BT action。它不自己规划路径，只调用外部 `/r2/navigation/navigate_to_named_pose` action server。

| 行 | 内容 | 注释 |
|---:|---|---|
| 5-7 | include 与 chrono | 引入头文件并启用 `500ms/0ms` 写法。 |
| 12-20 | 构造函数 | 保存 ROS 节点指针，读取 `action_name` 端口并创建 action client。 |
| 22-33 | `providedPorts()` | 声明 XML 可传入的 `action_name`、`target`、`team_side`、`timeout_sec`，以及输出 `error_code/message`。 |
| 35-62 | `onStart()` | 读取端口、检查 target、等待 action server、发送 goal。 |
| 43-47 | 空 target 保护 | 没有目标航点时返回 `FAILURE`，输出 `UNKNOWN_WAYPOINT`。 |
| 49-53 | server 可用性检查 | 500ms 内找不到 action server，则返回 `NAV2_UNAVAILABLE`。 |
| 55-61 | 发送 goal | 记录开始时间，异步发送 goal，返回 `RUNNING`。 |
| 64-104 | `onRunning()` | 检查超时、等待 goal accepted、等待 result，并把 action result 转成 BT SUCCESS/FAILURE。 |
| 66-76 | 超时保护 | 超时后取消 goal，返回 `TIMEOUT`。 |
| 79-93 | goal accepted 处理 | 等待 send_goal future，若被拒绝返回 `NAV2_ABORTED`。 |
| 95-103 | result 处理 | 拿到结果后输出 `error_code/message`，按 `result->success` 返回状态。 |
| 106-112 | `onHalted()` | BT 中断节点时取消正在执行的导航 goal。 |

## 6. `plugins/condition/is_manual_start.cpp`

手动启动条件节点。它只判断共享的 `/manual_start` 值是否达到阈值。

| 行 | 内容 | 注释 |
|---:|---|---|
| 1 | include | 引入条件节点头文件。 |
| 6-11 | 构造函数 | 保存共享的 `manual_start` 原子变量。 |
| 13-19 | `providedPorts()` | 声明兼容端口 `key_port` 和启动阈值 `start_value`。 |
| 21-26 | `tick()` | 读取 `start_value`，若 `/manual_start` 当前值达到阈值则返回 `SUCCESS`，否则 `FAILURE`。 |

## 7. 文档与准备文件

| 文件 | 注释 |
|---|---|
| `README.md` | GitHub 首页说明，包含结构、当前 no-tip 主线、已接入节点、待接入 topic/action/service。 |
| `docs/current_progress.md` | 当前进度、文件状态、已接入与未接入能力。 |
| `docs/integration_judgement.md` | 判断当前包按照 PB2025 结构制作，并说明新增代码准则。 |
| `docs/r2_no_tip_behavior_tree_design.md` | no-tip 行为树设计、状态机、重试流程和仍需确认事项。 |
| `action preparation/r2_action_interface_catalog.md` | topic/action/service 接口清单。 |
| `action preparation/r2_sensor_observation_requirements.md` | 传感器观测、状态输出和 rosbag 录制需求。 |
| `action preparation/r2_behavior_action_sequence.md` | 从行为树阶段映射到后续接口接入顺序。 |

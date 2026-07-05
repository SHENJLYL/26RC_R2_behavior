# 重要文件逐行注释

本文档是阅读辅助，不参与编译。格式约定：

| 列 | 含义 |
| --- | --- |
| 行 | 源文件行号 |
| 源码 | 原始代码或 XML/YAML 内容 |
| 注释 | 这一行在行为树系统中的作用 |

## 1. behavior_trees/r2_competition_main.xml

正式比赛行为树骨架。它保留真实任务顺序，未接入的实车接口使用 `AlwaysFailure` 占位，所以它不是“全流程通过验证树”。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `<?xml version="1.0" encoding="UTF-8"?>` | 声明这是 UTF-8 编码的 XML 文件。 |
| 2 | `<root BTCPP_format="4">` | BehaviorTree.CPP 根节点，格式版本为 4。 |
| 3 |  | 空行，用于分隔根声明和第一棵子树。 |
| 4 | `<BehaviorTree ID="StopAllMotion">` | 定义停车子树，名字是 `StopAllMotion`。 |
| 5 | `<Sequence>` | 顺序节点，内部动作从上到下执行。 |
| 6 | `<PublishTwist topic_name="cmd_vel"` | 调用 `PublishTwist`，准备向 `cmd_vel` 发布速度。 |
| 7 | `duration="0"` | 兼容参数，目前表示本节点 tick 时发布一次。 |
| 8 | `v_x="0.0"` | 线速度 x 设为 0。 |
| 9 | `v_y="0.0"` | 线速度 y 设为 0。 |
| 10 | `v_yaw="0.0"/>` | 角速度 yaw 设为 0，并结束 `PublishTwist` 标签。 |
| 11 | `</Sequence>` | 结束停车顺序节点。 |
| 12 | `</BehaviorTree>` | 结束 `StopAllMotion` 子树。 |
| 13 |  | 空行，分隔不同子树。 |
| 14 | `<BehaviorTree ID="PreMatchAndStart">` | 定义赛前检查和启动等待子树。 |
| 15 | `<Sequence>` | 赛前流程必须按顺序通过。 |
| 16 | `<!-- TODO interface: /r2/health/status + TF + sensor freshness -->` | 未来接入健康状态、TF、传感器新鲜度检查。 |
| 17 | `<AlwaysSuccess name="PreMatchSelfCheckPlaceholder"/>` | 当前赛前检查占位，始终成功。 |
| 18 | `<IsManualStart key_port="{@manual_start}"` | 调用手动启动条件节点，读取启动状态。 |
| 19 | `start_value="1"/>` | `/manual_start >= 1` 时认为比赛启动。 |
| 20 | `</Sequence>` | 结束赛前启动顺序节点。 |
| 21 | `</BehaviorTree>` | 结束 `PreMatchAndStart` 子树。 |
| 22 |  | 空行，分隔子树。 |
| 23 | `<BehaviorTree ID="MC_AssembleWeapon">` | 定义武馆区装配武器流程。 |
| 24 | `<Sequence>` | 武器装配各步骤按顺序执行。 |
| 25 | `<NavigateToNamedPose target="spearhead_rack_standoff"` | 调用命名航点导航，目标是矛头架前停靠点。 |
| 26 | `team_side="red"` | 使用红方航点表。 |
| 27 | `timeout_sec="45.0"/>` | 导航超时时间 45 秒。 |
| 28 |  | 空行，分隔导航和夹取动作。 |
| 29 | `<!-- TODO interface: /r2/manipulation/pick_tip action -->` | 未来接入夹取矛头的机械臂 action。 |
| 30 | `<AlwaysFailure name="DetectAndPickSpearheadPlaceholder"/>` | 当前未接入夹取，故意失败以阻止误认为完整可用。 |
| 31 |  | 空行，分隔夹取和下一段导航。 |
| 32 | `<PubNav2Goal topic_name="goal_pose"` | 发布一个 Nav2 goal 话题。 |
| 33 | `goal="0.0;0.0;0.0"/>` | 占位坐标，格式是 `x;y;yaw`。 |
| 34 |  | 空行，分隔导航目标和装配动作。 |
| 35 | `<!-- TODO interfaces:` | 开始列出装配武器需要的未来接口。 |
| 36 | `/r2/perception/r1_assembly_ready` | 未来判断 R1 是否已经准备好配合装配。 |
| 37 | `/r2/rule_guard/no_body_contact` | 未来规则保护，避免违规接触。 |
| 38 | `/r2/manipulation/assemble_tip_to_pole action` | 未来机械臂装配矛头到杆上的 action。 |
| 39 | `/r2/perception/weapon_assembled` | 未来感知确认武器是否装配完成。 |
| 40 | `-->` | 结束 TODO 注释。 |
| 41 | `<AlwaysFailure name="AssembleWeaponPlaceholder"/>` | 当前装配未接入，故意失败。 |
| 42 | `</Sequence>` | 结束武馆装配顺序节点。 |
| 43 | `</BehaviorTree>` | 结束 `MC_AssembleWeapon` 子树。 |
| 44 |  | 空行，分隔子树。 |
| 45 | `<BehaviorTree ID="WaitForR1BeforeR2LeavesMC">` | 定义 R2 离开武馆前等待 R1 的流程。 |
| 46 | `<Sequence>` | 等待流程按顺序执行。 |
| 47 | `<SubTree ID="StopAllMotion"/>` | 先调用停车子树，保证等待时不动。 |
| 48 | `<!-- TODO interface: /r2/perception/r1_entered_mf, avoid wireless R1-R2 communication -->` | 未来通过感知确认 R1 已进入密林，避免无线通信依赖。 |
| 49 | `<AlwaysFailure name="WaitUntilR1FullyEnteredMFPlaceholder"/>` | 当前等待感知未接入，故意失败。 |
| 50 | `</Sequence>` | 结束等待顺序节点。 |
| 51 | `</BehaviorTree>` | 结束等待 R1 子树。 |
| 52 |  | 空行，分隔子树。 |
| 53 | `<BehaviorTree ID="MF_CollectSingleR2KFS">` | 定义密林区收集一个 R2 可操作 KFS 的流程。 |
| 54 | `<Sequence>` | 密林收集流程按顺序执行。 |
| 55 | `<PubNav2Goal topic_name="goal_pose"` | 先发布一个占位 Nav2 目标。 |
| 56 | `goal="0.0;0.0;0.0"/>` | 占位坐标，后续应替换成密林入口或相关点。 |
| 57 |  | 空行，分隔导航和密林任务接口列表。 |
| 58 | `<!-- TODO interfaces:` | 开始列出密林收集需要的未来接口。 |
| 59 | `/r2/forest/enter action` | 未来进入密林的动作接口。 |
| 60 | `/kfs_tracker/detection -> /r2/perception/kfs_map` | 未来 KFS 检测转为 R2 可用地图。 |
| 61 | `/r2/rule_guard/check_forest_plan` | 未来检查密林规划是否符合规则。 |
| 62 | `/r2/forest/planner` | 未来密林路径规划模块。 |
| 63 | `/r2/nav/go_to_woods_block action` | 未来导航到指定木桩/块位的 action。 |
| 64 | `/r2/manipulation/pick_adjacent_kfs action` | 未来夹取相邻 KFS 的机械臂 action。 |
| 65 | `/r2/perception/payload_state` | 未来确认载荷状态。 |
| 66 | `-->` | 结束 TODO 注释。 |
| 67 | `<AlwaysFailure name="CollectSingleR2KFSPlaceholder"/>` | 当前密林收集未接入，故意失败。 |
| 68 | `</Sequence>` | 结束密林收集顺序节点。 |
| 69 | `</BehaviorTree>` | 结束 `MF_CollectSingleR2KFS` 子树。 |
| 70 |  | 空行，分隔子树。 |
| 71 | `<BehaviorTree ID="MF_ExitForest">` | 定义退出密林流程。 |
| 72 | `<Sequence>` | 退出密林动作按顺序执行。 |
| 73 | `<!-- TODO interfaces:` | 开始列出退出密林需要的接口。 |
| 74 | `/r2/perception/payload_state` | 未来检查是否带着目标载荷。 |
| 75 | `/r2/forest/planner` | 未来密林内规划器。 |
| 76 | `/r2/nav/follow_woods_graph_path action` | 未来沿木桩图路径离开密林。 |
| 77 | `/r2/forest/exit action` | 未来执行退出密林的动作接口。 |
| 78 | `-->` | 结束 TODO 注释。 |
| 79 | `<AlwaysFailure name="ExitForestViaBlock10_11_12Placeholder"/>` | 当前退出密林未接入，故意失败。 |
| 80 | `</Sequence>` | 结束退出密林顺序节点。 |
| 81 | `</BehaviorTree>` | 结束 `MF_ExitForest` 子树。 |
| 82 |  | 空行，分隔子树。 |
| 83 | `<BehaviorTree ID="Battlefield_PlaceMiddleLayer">` | 定义战场区放置中层 KFS 的流程。 |
| 84 | `<Sequence>` | 战场放置流程按顺序执行。 |
| 85 | `<PubNav2Goal topic_name="goal_pose"` | 发布战场目标点，占位用。 |
| 86 | `goal="0.0;0.0;0.0"/>` | 占位坐标，后续替换成战场放置点。 |
| 87 |  | 空行，分隔导航和放置接口。 |
| 88 | `<!-- TODO interfaces:` | 开始列出战场放置需要的接口。 |
| 89 | `/r2/perception/grid_state` | 未来读取九宫格/战场格子状态。 |
| 90 | `/r2/strategy/select_grid_cell` | 未来策略选择放置格。 |
| 91 | `/r2/manipulation/place_kfs_middle action` | 未来机械臂执行中层放置。 |
| 92 | `/r2/perception/payload_state` | 未来确认放置后载荷状态。 |
| 93 | `-->` | 结束 TODO 注释。 |
| 94 | `<AlwaysFailure name="PlaceMiddleLayerPlaceholder"/>` | 当前战场放置未接入，故意失败。 |
| 95 | `</Sequence>` | 结束战场放置顺序节点。 |
| 96 | `</BehaviorTree>` | 结束 `Battlefield_PlaceMiddleLayer` 子树。 |
| 97 |  | 空行，分隔子树和主树。 |
| 98 | `<BehaviorTree ID="R2_Competition_Main">` | 定义正式主树入口。 |
| 99 | `<ReactiveFallback name="R2Root">` | 根节点，优先检查急停、重试，否则执行主任务。 |
| 100 | `<Sequence name="EmergencyStopBranch">` | 急停分支，条件成功时执行停车。 |
| 101 | `<!-- TODO interface: /r2/emergency_stop -->` | 未来接入急停输入。 |
| 102 | `<AlwaysFailure name="IsEmergencyStopRequestedPlaceholder"/>` | 当前急停条件始终失败，表示不触发。 |
| 103 | `<SubTree ID="StopAllMotion"/>` | 若急停触发则停车。当前不会执行到这里。 |
| 104 | `</Sequence>` | 结束急停分支。 |
| 105 |  | 空行，分隔急停和重试分支。 |
| 106 | `<Sequence name="RetryRecoveryBranch">` | 重试/恢复分支。 |
| 107 | `<!-- TODO interfaces: /r2/retry/request, /r2/retry/execute action -->` | 未来接入重试请求和区域恢复 action。 |
| 108 | `<AlwaysFailure name="IsRetryRequestedPlaceholder"/>` | 当前重试请求始终失败，表示不触发。 |
| 109 | `<AlwaysFailure name="ExecuteAreaAwareRetryPlaceholder"/>` | 当前恢复动作未接入，保留失败占位。 |
| 110 | `</Sequence>` | 结束重试分支。 |
| 111 |  | 空行，分隔重试和主任务。 |
| 112 | `<Sequence name="CompetitionMission">` | 比赛主任务顺序。 |
| 113 | `<SubTree ID="PreMatchAndStart"/>` | 第一步：赛前检查并等待启动。 |
| 114 | `<SubTree ID="MC_AssembleWeapon"/>` | 第二步：武馆区装配武器。 |
| 115 | `<SubTree ID="WaitForR1BeforeR2LeavesMC"/>` | 第三步：等待 R1 进入密林后 R2 再离开。 |
| 116 | `<SubTree ID="MF_CollectSingleR2KFS"/>` | 第四步：密林区收集一个 KFS。 |
| 117 | `<SubTree ID="MF_ExitForest"/>` | 第五步：退出密林。 |
| 118 | `<SubTree ID="Battlefield_PlaceMiddleLayer"/>` | 第六步：战场区中层放置。 |
| 119 | `<SubTree ID="StopAllMotion"/>` | 最后停车。 |
| 120 | `</Sequence>` | 结束比赛主任务顺序。 |
| 121 | `</ReactiveFallback>` | 结束根 fallback。 |
| 122 | `</BehaviorTree>` | 结束正式主树。 |
| 123 | `</root>` | 结束 XML 根节点。 |

## 2. behavior_trees/r2_full_flow_verification.xml

完整流程验证树。它把未接入接口替换为 `AlwaysSuccess` mock，用来确认主流程能够顺序跑完。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `<?xml version="1.0" encoding="UTF-8"?>` | 声明 XML 文件编码。 |
| 2 | `<root BTCPP_format="4">` | BehaviorTree.CPP 根节点。 |
| 3 |  | 空行，分隔根节点和子树。 |
| 4 | `<BehaviorTree ID="Flow_StopAllMotion">` | 定义验证用停车子树。 |
| 5 | `<Sequence>` | 顺序执行停车动作。 |
| 6 | `<PublishTwist topic_name="cmd_vel"` | 调用现有速度发布节点。 |
| 7 | `duration="0"` | tick 时发布一次。 |
| 8 | `v_x="0.0"` | x 方向速度为 0。 |
| 9 | `v_y="0.0"` | y 方向速度为 0。 |
| 10 | `v_yaw="0.0"/>` | yaw 角速度为 0，并结束标签。 |
| 11 | `</Sequence>` | 结束停车顺序。 |
| 12 | `</BehaviorTree>` | 结束停车子树。 |
| 13 |  | 空行。 |
| 14 | `<BehaviorTree ID="Flow_PreMatchAndStart">` | 定义验证用赛前启动子树。 |
| 15 | `<Sequence>` | 赛前检查和启动按顺序执行。 |
| 16 | `<AlwaysSuccess name="PreMatchSelfCheckMock"/>` | 模拟赛前检查通过。 |
| 17 | `<IsManualStart key_port="{@manual_start}"` | 仍使用真实启动条件节点。 |
| 18 | `start_value="1"/>` | `/manual_start >= 1` 才继续。 |
| 19 | `</Sequence>` | 结束启动顺序。 |
| 20 | `</BehaviorTree>` | 结束启动子树。 |
| 21 |  | 空行。 |
| 22 | `<BehaviorTree ID="Flow_MC_AssembleWeapon">` | 定义验证用武馆装配流程。 |
| 23 | `<Sequence>` | 武馆装配步骤顺序执行。 |
| 24 | `<AlwaysSuccess name="NavigateToSpearheadRackMock"/>` | 模拟导航到矛头架成功。 |
| 25 | `<AlwaysSuccess name="DetectAndPickSpearheadMock"/>` | 模拟识别并夹取矛头成功。 |
| 26 | `<AlwaysSuccess name="NavigateToAssemblyPoseMock"/>` | 模拟导航到装配位成功。 |
| 27 | `<AlwaysSuccess name="AssembleWeaponMock"/>` | 模拟武器装配成功。 |
| 28 | `</Sequence>` | 结束武馆装配顺序。 |
| 29 | `</BehaviorTree>` | 结束武馆验证子树。 |
| 30 |  | 空行。 |
| 31 | `<BehaviorTree ID="Flow_WaitForR1BeforeR2LeavesMC">` | 定义验证用等待 R1 流程。 |
| 32 | `<Sequence>` | 等待流程顺序执行。 |
| 33 | `<SubTree ID="Flow_StopAllMotion"/>` | 等待前先停车。 |
| 34 | `<AlwaysSuccess name="WaitUntilR1FullyEnteredMFMock"/>` | 模拟 R1 已完全进入密林。 |
| 35 | `</Sequence>` | 结束等待顺序。 |
| 36 | `</BehaviorTree>` | 结束等待子树。 |
| 37 |  | 空行。 |
| 38 | `<BehaviorTree ID="Flow_MF_CollectSingleR2KFS">` | 定义验证用密林取 KFS 流程。 |
| 39 | `<Sequence>` | 密林任务顺序执行。 |
| 40 | `<AlwaysSuccess name="NavigateToForestEntryMock"/>` | 模拟导航到密林入口成功。 |
| 41 | `<AlwaysSuccess name="EnterForestMock"/>` | 模拟进入密林成功。 |
| 42 | `<AlwaysSuccess name="TrackAndSelectKFSMock"/>` | 模拟识别并选择 KFS 成功。 |
| 43 | `<AlwaysSuccess name="NavigateToWoodsBlockMock"/>` | 模拟导航到木桩/块位成功。 |
| 44 | `<AlwaysSuccess name="PickAdjacentKFSMock"/>` | 模拟夹取相邻 KFS 成功。 |
| 45 | `</Sequence>` | 结束密林取 KFS 顺序。 |
| 46 | `</BehaviorTree>` | 结束密林取 KFS 子树。 |
| 47 |  | 空行。 |
| 48 | `<BehaviorTree ID="Flow_MF_ExitForest">` | 定义验证用退出密林流程。 |
| 49 | `<Sequence>` | 退出密林步骤顺序执行。 |
| 50 | `<AlwaysSuccess name="PlanExitViaBlock10_11_12Mock"/>` | 模拟规划经 10/11/12 号块退出成功。 |
| 51 | `<AlwaysSuccess name="FollowForestExitPathMock"/>` | 模拟沿规划路径退出成功。 |
| 52 | `<AlwaysSuccess name="ExitForestMock"/>` | 模拟完成离开密林动作。 |
| 53 | `</Sequence>` | 结束退出密林顺序。 |
| 54 | `</BehaviorTree>` | 结束退出密林子树。 |
| 55 |  | 空行。 |
| 56 | `<BehaviorTree ID="Flow_Battlefield_PlaceMiddleLayer">` | 定义验证用战场放置流程。 |
| 57 | `<Sequence>` | 战场放置步骤顺序执行。 |
| 58 | `<AlwaysSuccess name="NavigateToBattlefieldMock"/>` | 模拟导航到战场成功。 |
| 59 | `<AlwaysSuccess name="ReadGridStateMock"/>` | 模拟读取格子状态成功。 |
| 60 | `<AlwaysSuccess name="SelectMiddleLayerCellMock"/>` | 模拟选择中层放置格成功。 |
| 61 | `<AlwaysSuccess name="PlaceKFSMiddleLayerMock"/>` | 模拟放置 KFS 到中层成功。 |
| 62 | `</Sequence>` | 结束战场放置顺序。 |
| 63 | `</BehaviorTree>` | 结束战场放置子树。 |
| 64 |  | 空行。 |
| 65 | `<BehaviorTree ID="R2_Full_Flow_Verification">` | 定义完整流程验证主树。 |
| 66 | `<ReactiveFallback name="R2FlowVerificationRoot">` | 根节点，保持与正式主树相似的优先级结构。 |
| 67 | `<Sequence name="EmergencyStopBranchMockedInactive">` | 验证用急停分支，默认不触发。 |
| 68 | `<AlwaysFailure name="IsEmergencyStopRequestedMock"/>` | 模拟没有急停请求。 |
| 69 | `<SubTree ID="Flow_StopAllMotion"/>` | 急停触发时会停车，但本 mock 不会执行到这里。 |
| 70 | `</Sequence>` | 结束急停 mock 分支。 |
| 71 |  | 空行。 |
| 72 | `<Sequence name="RetryRecoveryBranchMockedInactive">` | 验证用重试分支，默认不触发。 |
| 73 | `<AlwaysFailure name="IsRetryRequestedMock"/>` | 模拟没有重试请求。 |
| 74 | `<AlwaysSuccess name="ExecuteAreaAwareRetryMock"/>` | 若重试触发则模拟恢复成功，目前不会执行到。 |
| 75 | `</Sequence>` | 结束重试 mock 分支。 |
| 76 |  | 空行。 |
| 77 | `<Sequence name="CompetitionMissionFlow">` | 真正验证的比赛任务顺序。 |
| 78 | `<SubTree ID="Flow_PreMatchAndStart"/>` | 第一步：赛前检查和启动。 |
| 79 | `<SubTree ID="Flow_MC_AssembleWeapon"/>` | 第二步：武馆装配。 |
| 80 | `<SubTree ID="Flow_WaitForR1BeforeR2LeavesMC"/>` | 第三步：等待 R1。 |
| 81 | `<SubTree ID="Flow_MF_CollectSingleR2KFS"/>` | 第四步：密林取 KFS。 |
| 82 | `<SubTree ID="Flow_MF_ExitForest"/>` | 第五步：退出密林。 |
| 83 | `<SubTree ID="Flow_Battlefield_PlaceMiddleLayer"/>` | 第六步：战场放置。 |
| 84 | `<SubTree ID="Flow_StopAllMotion"/>` | 最后停车。 |
| 85 | `</Sequence>` | 结束完整比赛流程。 |
| 86 | `</ReactiveFallback>` | 结束根 fallback。 |
| 87 | `</BehaviorTree>` | 结束完整流程验证主树。 |
| 88 | `</root>` | 结束 XML 文件。 |

## 3. params/r2_behavior_full_flow_verification.yaml

流程验证专用参数，决定启动哪棵树、tick 频率，以及是否自动发布启动信号。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `r2_behavior_server:` | 给 `r2_behavior_server` 节点配置参数。 |
| 2 | `ros__parameters:` | ROS 2 参数命名固定层级。 |
| 3 | `use_sim_time: false` | 默认不用仿真时间，适合不启动 Gazebo 的流程验证。 |
| 4 | `target_tree: R2_Full_Flow_Verification` | 指定加载完整流程验证树。 |
| 5 | `tick_frequency: 2.0` | 行为树每秒 tick 2 次。 |
| 6 |  | 空行，分隔 server 和 client 参数。 |
| 7 | `r2_behavior_client:` | 给 `r2_behavior_client` 节点配置参数。 |
| 8 | `ros__parameters:` | ROS 2 参数命名固定层级。 |
| 9 | `use_sim_time: false` | client 也不用仿真时间。 |
| 10 | `auto_start: true` | 启动后自动发布 `/manual_start`。 |
| 11 | `start_value: 1` | 发布的启动值为 1。 |
| 12 | `auto_start_delay_ms: 500` | 当前源码未实际读取该参数，保留作配置意图说明。 |
| 13 | `auto_start_repeat_count: 20` | 重复发布 20 次，避免 server 尚未订阅时错过启动信号。 |

## 4. launch/r2_behavior_launch.py

统一启动入口。它启动 server 和 client，并把 YAML 参数传进去。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `import os` | 引入路径拼接工具。 |
| 2 |  | 空行，分隔标准库和 ROS launch 导入。 |
| 3 | `from ament_index_python.packages import get_package_share_directory` | 获取 ROS 包安装后的 share 目录。 |
| 4 | `from launch import LaunchDescription` | launch 文件必须返回 `LaunchDescription`。 |
| 5 | `from launch.actions import DeclareLaunchArgument` | 用于声明可从命令行传入的参数。 |
| 6 | `from launch.actions import GroupAction` | 将多个 launch 动作组织成一组。 |
| 7 | `from launch.actions import SetEnvironmentVariable` | 设置运行时环境变量。 |
| 8 | `from launch.substitutions import LaunchConfiguration` | 读取 launch 参数的占位对象。 |
| 9 | `from launch_ros.actions import Node` | 启动 ROS 2 节点的 action。 |
| 10 | `from launch_ros.actions import PushRosNamespace` | 给组内节点加命名空间。 |
| 11 | `from launch_ros.actions import SetRemap` | 设置 topic remap。 |
| 12 | `from launch_ros.descriptions import ParameterFile` | 声明参数文件对象。 |
| 13 | `from nav2_common.launch import RewrittenYaml` | 用于动态改写 YAML 参数，例如 `use_sim_time`。 |
| 14 |  | 空行，分隔导入和函数定义。 |
| 15 |  | 空行。 |
| 16 | `def generate_launch_description():` | ROS 2 launch 入口函数。 |
| 17 | `bringup_dir = get_package_share_directory("r2_behavior")` | 找到 `r2_behavior` 安装目录。 |
| 18 |  | 空行，分隔目录和 launch 参数。 |
| 19 | `namespace = LaunchConfiguration("namespace")` | 获取命名空间参数。 |
| 20 | `use_sim_time = LaunchConfiguration("use_sim_time")` | 获取是否使用仿真时间的参数。 |
| 21 | `params_file = LaunchConfiguration("params_file")` | 获取参数文件路径。 |
| 22 | `log_level = LaunchConfiguration("log_level")` | 获取日志等级。 |
| 23 |  | 空行，分隔参数和配置文件改写。 |
| 24 | `configured_params = ParameterFile(` | 创建最终传给节点的参数文件对象。 |
| 25 | `RewrittenYaml(` | 开始配置 YAML 改写规则。 |
| 26 | `source_file=params_file,` | 原始 YAML 来自命令行或默认参数。 |
| 27 | `root_key=namespace,` | 支持按命名空间包裹参数。 |
| 28 | `param_rewrites={"use_sim_time": use_sim_time},` | 用 launch 参数覆盖 YAML 里的 `use_sim_time`。 |
| 29 | `convert_types=True,` | 自动把字符串转为 bool/int/float 等类型。 |
| 30 | `),` | 结束 `RewrittenYaml`。 |
| 31 | `allow_substs=True,` | 允许 launch substitution。 |
| 32 | `)` | 结束 `ParameterFile`。 |
| 33 |  | 空行，分隔参数处理和返回对象。 |
| 34 | `return LaunchDescription(` | 返回 launch 描述。 |
| 35 | `[` | 开始 launch action 列表。 |
| 36 | `SetEnvironmentVariable("RCUTILS_LOGGING_BUFFERED_STREAM", "1"),` | 让日志缓冲输出更稳定。 |
| 37 | `SetEnvironmentVariable("RCUTILS_COLORIZED_OUTPUT", "1"),` | 开启彩色日志。 |
| 38 | `DeclareLaunchArgument("namespace", default_value="", description="Top-level namespace"),` | 声明命名空间参数，默认无命名空间。 |
| 39 | `DeclareLaunchArgument(` | 开始声明 `use_sim_time`。 |
| 40 | `"use_sim_time",` | 参数名是 `use_sim_time`。 |
| 41 | `default_value="false",` | 默认不用仿真时间。 |
| 42 | `description="Use simulation clock if true",` | 参数说明。 |
| 43 | `),` | 结束 `use_sim_time` 参数声明。 |
| 44 | `DeclareLaunchArgument(` | 开始声明 `params_file`。 |
| 45 | `"params_file",` | 参数名是 `params_file`。 |
| 46 | `default_value=os.path.join(bringup_dir, "params", "r2_behavior.yaml"),` | 默认使用安装目录里的 `r2_behavior.yaml`。 |
| 47 | `description="Full path to the R2 behavior parameter file",` | 参数说明。 |
| 48 | `),` | 结束 `params_file` 参数声明。 |
| 49 | `DeclareLaunchArgument("log_level", default_value="info", description="ROS log level"),` | 声明日志等级，默认 info。 |
| 50 | `GroupAction(` | 开始一组共同使用 namespace/remap 的动作。 |
| 51 | `[` | 开始组内 action 列表。 |
| 52 | `PushRosNamespace(namespace=namespace),` | 对组内节点应用命名空间。 |
| 53 | `SetRemap("/tf", "tf"),` | 将绝对 `/tf` remap 成相对 `tf`，便于命名空间隔离。 |
| 54 | `SetRemap("/tf_static", "tf_static"),` | 同理处理 `/tf_static`。 |
| 55 | `Node(` | 开始启动 server 节点。 |
| 56 | `package="r2_behavior",` | server 来自 `r2_behavior` 包。 |
| 57 | `executable="r2_behavior_server",` | 可执行文件名。 |
| 58 | `name="r2_behavior_server",` | ROS 节点名。 |
| 59 | `output="screen",` | 日志输出到终端。 |
| 60 | `parameters=[configured_params],` | 将改写后的 YAML 参数传给 server。 |
| 61 | `arguments=["--ros-args", "--log-level", log_level],` | 设置 ROS 日志等级。 |
| 62 | `),` | 结束 server 节点定义。 |
| 63 | `Node(` | 开始启动 client 节点。 |
| 64 | `package="r2_behavior",` | client 也来自 `r2_behavior` 包。 |
| 65 | `executable="r2_behavior_client",` | 可执行文件名。 |
| 66 | `name="r2_behavior_client",` | ROS 节点名。 |
| 67 | `output="screen",` | 日志输出到终端。 |
| 68 | `parameters=[configured_params],` | client 使用同一份 YAML 参数。 |
| 69 | `arguments=["--ros-args", "--log-level", log_level],` | 设置 client 日志等级。 |
| 70 | `),` | 结束 client 节点定义。 |
| 71 | `]` | 结束组内 action 列表。 |
| 72 | `)` | 结束 `GroupAction`。 |
| 73 | `]` | 结束 launch action 列表。 |
| 74 | `)` | 结束并返回 `LaunchDescription`。 |

## 5. src/r2_behavior_server.cpp

行为树运行器。它负责加载 XML、注册 C++ 节点、订阅启动信号并周期性 tick 行为树。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `// Copyright 2026 AIM Robotics` | 版权声明。 |
| 2 | `//` | 注释空行。 |
| 3 | `// Licensed under the Apache License, Version 2.0.` | 许可证声明。 |
| 4 |  | 空行，分隔文件头和 include。 |
| 5 | `#include "r2_behavior/r2_behavior_server.hpp"` | 引入 server 类声明。 |
| 6 |  | 空行。 |
| 7 | `#include <chrono>` | 使用时间周期类型。 |
| 8 | `#include <filesystem>` | 遍历行为树 XML 目录。 |
| 9 | `#include <memory>` | 使用智能指针。 |
| 10 |  | 空行。 |
| 11 | `#include "ament_index_cpp/get_package_share_directory.hpp"` | 获取 ROS 包 share 目录。 |
| 12 | `#include "r2_behavior/plugins/action/navigate_to_named_pose.hpp"` | 引入命名航点导航 BT 节点。 |
| 13 | `#include "r2_behavior/plugins/action/pub_nav2_goal.hpp"` | 引入 Nav2 goal 发布 BT 节点。 |
| 14 | `#include "r2_behavior/plugins/action/pub_twist.hpp"` | 引入速度发布 BT 节点。 |
| 15 | `#include "r2_behavior/plugins/condition/is_manual_start.hpp"` | 引入手动启动条件节点。 |
| 16 |  | 空行。 |
| 17 | `using namespace std::chrono_literals;` | 允许使用 `500ms` 等时间字面量。 |
| 18 |  | 空行。 |
| 19 | `namespace r2_behavior` | 进入项目命名空间。 |
| 20 | `{` | 命名空间开始。 |
| 21 |  | 空行。 |
| 22 | `R2BehaviorServer::R2BehaviorServer(const rclcpp::NodeOptions & options)` | server 构造函数定义。 |
| 23 | `: Node("r2_behavior_server", options), manual_start_(std::make_shared<std::atomic<int>>(0))` | 初始化 ROS 节点名和启动状态，默认 0。 |
| 24 | `{` | 构造函数体开始。 |
| 25 | `const auto share_dir = ament_index_cpp::get_package_share_directory("r2_behavior");` | 找到安装后的 `r2_behavior/share` 路径。 |
| 26 | `declare_parameter<std::string>("behavior_tree_file", "");` | 声明可指定单个 XML 文件。 |
| 27 | `declare_parameter<std::string>("behavior_tree_directory", share_dir + "/behavior_trees");` | 声明 XML 目录，默认安装目录下的 `behavior_trees`。 |
| 28 | `declare_parameter<std::string>("target_tree", "R2_Competition_Main");` | 声明目标树 ID，默认正式主树。 |
| 29 | `declare_parameter<double>("tick_frequency", 5.0);` | 声明 tick 频率，默认 5Hz。 |
| 30 |  | 空行。 |
| 31 | `get_parameter("behavior_tree_file", behavior_tree_file_);` | 读取单文件 XML 参数。 |
| 32 | `get_parameter("behavior_tree_directory", behavior_tree_directory_);` | 读取 XML 目录参数。 |
| 33 | `get_parameter("target_tree", target_tree_);` | 读取目标树 ID。 |
| 34 | `const double tick_frequency = get_parameter("tick_frequency").as_double();` | 读取 tick 频率。 |
| 35 |  | 空行。 |
| 36 | `manual_start_sub_ = create_subscription<std_msgs::msg::Int32>(` | 创建 `/manual_start` 订阅者。 |
| 37 | `"manual_start", rclcpp::QoS(10),` | topic 名为 `manual_start`，队列深度 10。 |
| 38 | `[this](const std_msgs::msg::Int32::SharedPtr msg) { manual_start_->store(msg->data); });` | 收到消息后写入原子变量。 |
| 39 |  | 空行。 |
| 40 | `registerNodes();` | 注册 XML 可使用的 C++ BT 节点。 |
| 41 | `loadTree();` | 加载 XML 并创建目标树。 |
| 42 |  | 空行。 |
| 43 | `const auto period = std::chrono::duration<double>(1.0 / std::max(tick_frequency, 0.1));` | 根据频率计算 tick 周期，并限制最低 0.1Hz 防止除零。 |
| 44 | `tick_timer_ = create_wall_timer(` | 创建周期定时器。 |
| 45 | `std::chrono::duration_cast<std::chrono::milliseconds>(period),` | 将周期转换成毫秒。 |
| 46 | `std::bind(&R2BehaviorServer::tickTree, this));` | 定时调用 `tickTree()`。 |
| 47 | `}` | 构造函数结束。 |
| 48 |  | 空行。 |
| 49 | `void R2BehaviorServer::registerNodes()` | 定义注册 BT 节点的函数。 |
| 50 | `{` | 函数体开始。 |
| 51 | `auto node = std::shared_ptr<rclcpp::Node>(this, [](rclcpp::Node *) {});` | 包装当前 ROS 节点指针，供插件节点使用，空 deleter 避免重复释放。 |
| 52 |  | 空行。 |
| 53 | `factory_.registerBuilder<IsManualStartCondition>(` | 向 BT 工厂注册 `IsManualStart`。 |
| 54 | `"IsManualStart",` | XML 中使用的节点名。 |
| 55 | `[this](const std::string & name, const BT::NodeConfiguration & config) {` | 定义创建该节点的 builder。 |
| 56 | `return std::make_unique<IsManualStartCondition>(name, config, manual_start_);` | 创建条件节点，并传入共享的启动状态。 |
| 57 | `});` | 结束 `IsManualStart` 注册。 |
| 58 |  | 空行。 |
| 59 | `factory_.registerBuilder<PubNav2GoalAction>(` | 注册 `PubNav2Goal`。 |
| 60 | `"PubNav2Goal",` | XML 中使用的节点名。 |
| 61 | `[node](const std::string & name, const BT::NodeConfiguration & config) {` | 定义创建发布 goal 节点的 builder。 |
| 62 | `return std::make_unique<PubNav2GoalAction>(name, config, node);` | 创建节点，并传入 ROS 节点用于发布 topic。 |
| 63 | `});` | 结束 `PubNav2Goal` 注册。 |
| 64 |  | 空行。 |
| 65 | `factory_.registerBuilder<PublishTwistAction>(` | 注册 `PublishTwist`。 |
| 66 | `"PublishTwist",` | XML 中使用的节点名。 |
| 67 | `[node](const std::string & name, const BT::NodeConfiguration & config) {` | 定义创建速度发布节点的 builder。 |
| 68 | `return std::make_unique<PublishTwistAction>(name, config, node);` | 创建节点，并传入 ROS 节点用于发布 `/cmd_vel`。 |
| 69 | `});` | 结束 `PublishTwist` 注册。 |
| 70 |  | 空行。 |
| 71 | `factory_.registerBuilder<NavigateToNamedPoseAction>(` | 注册 `NavigateToNamedPose`。 |
| 72 | `"NavigateToNamedPose",` | XML 中使用的节点名。 |
| 73 | `[node](const std::string & name, const BT::NodeConfiguration & config) {` | 定义创建导航 action 节点的 builder。 |
| 74 | `return std::make_unique<NavigateToNamedPoseAction>(name, config, node);` | 创建导航节点，并传入 ROS 节点用于创建 action client。 |
| 75 | `});` | 结束导航节点注册。 |
| 76 | `}` | `registerNodes()` 结束。 |
| 77 |  | 空行。 |
| 78 | `void R2BehaviorServer::loadTree()` | 定义加载 XML 行为树的函数。 |
| 79 | `{` | 函数体开始。 |
| 80 | `if (!behavior_tree_file_.empty()) {` | 如果指定了单个 XML 文件。 |
| 81 | `factory_.registerBehaviorTreeFromFile(behavior_tree_file_);` | 只注册该 XML 文件。 |
| 82 | `} else {` | 否则使用目录模式。 |
| 83 | `for (const auto & entry : std::filesystem::directory_iterator(behavior_tree_directory_)) {` | 遍历行为树目录。 |
| 84 | `if (entry.is_regular_file() && entry.path().extension() == ".xml") {` | 只处理普通 `.xml` 文件。 |
| 85 | `factory_.registerBehaviorTreeFromFile(entry.path().string());` | 注册该 XML 文件中的所有 BehaviorTree。 |
| 86 | `}` | 结束 XML 文件判断。 |
| 87 | `}` | 结束目录遍历。 |
| 88 | `}` | 结束单文件/目录分支。 |
| 89 |  | 空行。 |
| 90 | `tree_ = std::make_unique<BT::Tree>(factory_.createTree(target_tree_));` | 按 `target_tree` 创建实际运行的树。 |
| 91 | `RCLCPP_INFO(` | 开始打印加载日志。 |
| 92 | `get_logger(), "Loaded R2 behavior tree '%s' from '%s'", target_tree_.c_str(),` | 日志内容：目标树 ID 和来源。 |
| 93 | `behavior_tree_file_.empty() ? behavior_tree_directory_.c_str() : behavior_tree_file_.c_str());` | 根据模式显示目录或单文件路径。 |
| 94 | `}` | `loadTree()` 结束。 |
| 95 |  | 空行。 |
| 96 | `void R2BehaviorServer::tickTree()` | 定义周期 tick 函数。 |
| 97 | `{` | 函数体开始。 |
| 98 | `if (!tree_) {` | 如果树还没创建。 |
| 99 | `return;` | 直接返回，避免空指针。 |
| 100 | `}` | 结束空树保护。 |
| 101 |  | 空行。 |
| 102 | `const auto status = tree_->tickRoot();` | tick 根节点，驱动整棵行为树运行一步。 |
| 103 | `if (status != last_status_) {` | 只有状态变化时才打印日志。 |
| 104 | `RCLCPP_INFO(get_logger(), "R2 behavior tree status changed to %d", static_cast<int>(status));` | 输出状态码，SUCCESS 通常显示为 2。 |
| 105 | `last_status_ = status;` | 更新上一次状态。 |
| 106 | `}` | 结束状态变化判断。 |
| 107 | `}` | `tickTree()` 结束。 |
| 108 |  | 空行。 |
| 109 | `}  // namespace r2_behavior` | 退出项目命名空间。 |
| 110 |  | 空行。 |
| 111 | `int main(int argc, char * argv[])` | server 可执行程序入口。 |
| 112 | `{` | main 函数开始。 |
| 113 | `rclcpp::init(argc, argv);` | 初始化 ROS 2。 |
| 114 | `rclcpp::spin(std::make_shared<r2_behavior::R2BehaviorServer>(rclcpp::NodeOptions()));` | 创建并运行 server 节点，直到进程退出。 |
| 115 | `rclcpp::shutdown();` | 关闭 ROS 2。 |
| 116 | `return 0;` | 正常退出。 |
| 117 | `}` | main 函数结束。 |

## 6. src/r2_behavior_client.cpp

启动辅助节点。它可以自动重复发布 `/manual_start`，用于流程验证或比赛自动启动调试。

| 行 | 源码 | 注释 |
| ---: | --- | --- |
| 1 | `// Copyright 2026 AIM Robotics` | 版权声明。 |
| 2 | `//` | 注释空行。 |
| 3 | `// Licensed under the Apache License, Version 2.0.` | 许可证声明。 |
| 4 |  | 空行。 |
| 5 | `#include "r2_behavior/r2_behavior_client.hpp"` | 引入 client 类声明。 |
| 6 |  | 空行。 |
| 7 | `#include <chrono>` | 使用时间字面量和定时器周期。 |
| 8 |  | 空行。 |
| 9 | `using namespace std::chrono_literals;` | 允许写 `500ms`。 |
| 10 |  | 空行。 |
| 11 | `namespace r2_behavior` | 进入项目命名空间。 |
| 12 | `{` | 命名空间开始。 |
| 13 |  | 空行。 |
| 14 | `R2BehaviorClient::R2BehaviorClient(const rclcpp::NodeOptions & options)` | client 构造函数定义。 |
| 15 | `: Node("r2_behavior_client", options)` | 初始化 ROS 节点名。 |
| 16 | `{` | 构造函数体开始。 |
| 17 | `declare_parameter<bool>("auto_start", false);` | 声明是否自动发布启动信号。 |
| 18 | `declare_parameter<int>("start_value", 1);` | 声明启动信号数值。 |
| 19 | `declare_parameter<int>("auto_start_repeat_count", 1);` | 声明自动启动重复发布次数。 |
| 20 | `get_parameter("auto_start", auto_start_);` | 读取是否自动启动。 |
| 21 | `get_parameter("start_value", start_value_);` | 读取启动值。 |
| 22 | `get_parameter("auto_start_repeat_count", auto_start_repeat_count_);` | 读取重复次数。 |
| 23 |  | 空行。 |
| 24 | `manual_start_pub_ = create_publisher<std_msgs::msg::Int32>("manual_start", rclcpp::QoS(10));` | 创建 `/manual_start` 发布者。 |
| 25 |  | 空行。 |
| 26 | `if (auto_start_) {` | 如果参数要求自动启动。 |
| 27 | `timer_ = create_wall_timer(500ms, std::bind(&R2BehaviorClient::publishStart, this));` | 每 500ms 调用一次 `publishStart()`。 |
| 28 | `} else {` | 否则进入空闲模式。 |
| 29 | `RCLCPP_INFO(` | 开始打印提示日志。 |
| 30 | `get_logger(),` | 使用本节点 logger。 |
| 31 | `"R2 behavior client is idle. Publish /manual_start or set auto_start:=true to start.");` | 告诉用户需要手动发布或打开自动启动。 |
| 32 | `}` | 结束自动/空闲分支。 |
| 33 | `}` | 构造函数结束。 |
| 34 |  | 空行。 |
| 35 | `void R2BehaviorClient::publishStart()` | 定义发布启动信号的函数。 |
| 36 | `{` | 函数体开始。 |
| 37 | `std_msgs::msg::Int32 msg;` | 创建 Int32 消息。 |
| 38 | `msg.data = start_value_;` | 将消息值设为配置的启动值。 |
| 39 | `manual_start_pub_->publish(msg);` | 发布到 `/manual_start`。 |
| 40 | `RCLCPP_INFO(get_logger(), "Published manual_start=%d", start_value_);` | 打印发布日志。 |
| 41 | `++auto_start_publish_count_;` | 已发布次数加一。 |
| 42 | `if (auto_start_publish_count_ >= std::max(auto_start_repeat_count_, 1)) {` | 如果达到重复次数，至少发布 1 次。 |
| 43 | `timer_->cancel();` | 停止定时器，不再继续发布。 |
| 44 | `}` | 结束次数判断。 |
| 45 | `}` | `publishStart()` 结束。 |
| 46 |  | 空行。 |
| 47 | `}  // namespace r2_behavior` | 退出项目命名空间。 |
| 48 |  | 空行。 |
| 49 | `int main(int argc, char * argv[])` | client 可执行程序入口。 |
| 50 | `{` | main 函数开始。 |
| 51 | `rclcpp::init(argc, argv);` | 初始化 ROS 2。 |
| 52 | `rclcpp::spin(std::make_shared<r2_behavior::R2BehaviorClient>(rclcpp::NodeOptions()));` | 创建并运行 client 节点。 |
| 53 | `rclcpp::shutdown();` | 关闭 ROS 2。 |
| 54 | `return 0;` | 正常退出。 |
| 55 | `}` | main 函数结束。 |

## 7. 插件节点速查

这些文件是 XML 节点背后的真实执行代码：

| XML 节点名 | 主要文件 | 作用 |
| --- | --- | --- |
| `IsManualStart` | `plugins/condition/is_manual_start.cpp` | 判断 `/manual_start` 是否达到启动值。 |
| `NavigateToNamedPose` | `plugins/action/navigate_to_named_pose.cpp` | 调用 `/r2/navigation/navigate_to_named_pose` action。 |
| `PubNav2Goal` | `plugins/action/pub_nav2_goal.cpp` | 发布 `geometry_msgs/PoseStamped` 到 `goal_pose`。 |
| `PublishTwist` | `plugins/action/pub_twist.cpp` | 发布 `geometry_msgs/Twist` 到 `cmd_vel`，用于停车或简单速度输出。 |

后续如果只是改流程，优先改 XML；如果要接入新的 topic/service/action，才改或新增这些插件节点。

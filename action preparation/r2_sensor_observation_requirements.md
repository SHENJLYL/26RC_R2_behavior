# R2 Sensor Observation Requirements

本文档描述当前 no-tip 行为树需要从底层读取哪些信息、如何判断动作是否成功、以及哪些状态必须输出给行为树和 rosbag 调研使用。

## 总体要求

所有关键状态都应包含：

| 字段 | 要求 |
|---|---|
| `stamp` | 状态生成时间，必须能判断是否过期 |
| `frame_id` | 位姿、点云、检测框所属坐标系 |
| `confidence` | 识别或估计置信度 |
| `source` | 来源，例如 `front_lidar`、`front_depth_camera`、`arm_depth_camera`、`screw_feedback` |
| `error_code` / `diagnostic_text` | 失败原因，供 rosbag 回放定位 |

行为树不直接订阅原始图像或点云；视觉、底盘和机械臂节点应先汇总成任务级 topic/service/action。

## 底盘、丝杠和轮端红外

### 需要读取

| 信息 | 来源 | 用途 |
|---|---|---|
| 四轮红外相对对地高度 | 四个轮端红外 | 判断普通地面、树林台面高度、轮端悬空/顶死 |
| 丝杠圈数 | 丝杠编码或估算 | 记录开环抬升目标与执行量 |
| 丝杠电流 | 驱动反馈 | 判断抬升受阻、过载、触底 |
| 底盘里程计 | `/odom` 或底盘驱动 | 判断移动是否发生、是否卡住 |
| 速度命令 | `/cmd_vel` | 判断 BT/导航是否输出运动指令 |
| TF | `/tf`、`/tf_static` | 判断 `map/odom/base_link` 是否连通 |
| 前方障碍/边缘 | 激光雷达、前方深度相机 | 判断树林入口、台面边缘、对抗区靠近风险 |

### 建议 topic

| topic | 内容 |
|---|---|
| `/r2/chassis/wheel_height_state` | 四轮红外高度、丝杠圈数、电流、目标高度、状态 |
| `/r2/chassis/travel_mode_state` | `GROUND/FOREST/TRANSITION/RECOVERY`，模式是否到位 |
| `/r2/chassis/motion_state` | 当前速度、目标速度、是否卡住、是否抖动 |

### 判断成功条件

| 判断 | 成功条件 | 异常处理 |
|---|---|---|
| 已进入树林模式 | 四轮目标高度到位，红外高度可信，丝杠无过流 | 停车，重试 `SetTravelMode(FOREST)` |
| 方块间移动完成 | 里程计/TF 到位，红外高度稳定，激光未见危险障碍 | 标记 step failed，触发 `ExecuteForestPlan` 重试 |
| 已离开树林 | 通过 10/11/12 号出口，切回普通地面模式 | 若 payload 丢失或位置不明，进入恢复 |

## 前方激光雷达与前方深度相机

| 信息 | 用途 |
|---|---|
| 障碍距离和方向 | 导航避障、树林入口对齐、对抗区靠近 |
| 台面边缘/高度突变 | 森林方块移动安全判断 |
| R1/九宫格粗位姿 | 对抗区对齐前的初始定位 |
| 动态障碍 | 避免和 R1 或现场物体发生干涉 |

建议 topic：

| topic | 内容 |
|---|---|
| `/r2/perception/front_obstacle_state` | 最近障碍距离、方向、置信度 |
| `/r2/perception/forest_edge_state` | 树林方块边界、可通行方向 |
| `/r2/perception/grid_pose_hint` | 九宫格粗位姿和置信度 |

## 机械臂深度相机

当前 no-tip 主线中，机械臂深度相机的核心作用是 KFS 复核、KFS 抓取和九宫格放置确认。

| 信息 | 用途 |
|---|---|
| KFS 位姿 | 吸盘抓取相邻方块上的 R2 KFS |
| KFS 类型 | 判断 `R2_KFS/R1_KFS/FAKE_KFS/UNKNOWN` |
| 方块内 KFS 是否存在 | `ValidateKFSOnBlock` 单次 service |
| R1 KFS 是否已移除 | 路径被 R1 KFS 阻碍时的相机判断 |
| 九宫格 cell 位姿 | 二层/三层放置时对位 |
| 放置后状态 | 确认 payload 释放、目标格占用 |

建议 topic/service：

| 接口 | 内容 |
|---|---|
| `/kfs_tracker/detection` | 连续 KFS 视觉检测源 |
| `/r2/perception/validate_kfs_on_block` | 单次复核某方块 KFS 是否符合预期 |
| `/r2/forest/check_r1_kfs_removed` | 单次或限时判断 R1 KFS 是否已被移除 |
| `/r2/perception/grid_state` | 九宫格位姿、层数、格子占用 |

KFS 分类规则：

| 类型 | 行为树处理 |
|---|---|
| `R2_KFS` | 可作为抓取目标 |
| `R1_KFS` | R2 不抓取；若阻碍路径则等待 R1 移除 |
| `FAKE_KFS` | 硬禁止，不抓取、不推动 |
| `UNKNOWN` | 不继续该 step，进入复核/重规划 |
| `EMPTY` | 可作为通行或放置候选 |

## 吸盘、机械臂和载荷

当前 R2 只能携带一个由机械臂吸盘吸取的 R2 KFS。

| 信息 | 来源 | 用途 |
|---|---|---|
| 吸盘开关 | 吸盘控制 | 判断命令状态 |
| 真空压力 | 吸盘传感器 | 判断 KFS 是否吸附 |
| 机械臂关节状态 | 机械臂驱动 | 判断动作是否完成、安全姿态是否到位 |
| 视觉确认 | 机械臂深度相机 | 复核吸取/放置后的 KFS 状态 |

建议 topic：

| topic | 内容 |
|---|---|
| `/r2/manipulation/suction_state` | 吸盘开关、真空压力、吸附可信度 |
| `/r2/manipulation/arm_state` | 机械臂动作状态、关节状态、安全姿态 |
| `/r2/perception/payload_state` | 当前携带物类型、携带方式、置信度 |

载荷规则：

| 状态 | 行为约束 |
|---|---|
| `NONE` | 可抓取一个 R2 KFS |
| `R2_KFS` | 可离开树林、进入对抗区、执行二层/三层放置 |
| `R1_KFS` | 严重异常，立即停车并进入恢复 |
| `FAKE_KFS` | 严重异常，立即停车并进入恢复 |
| `UNKNOWN` | 禁止继续抓取或放置，先重新识别 |

## R1 ArUco

R1 指令源按 ArUco 设计。

| 信息 | 用途 |
|---|---|
| ArUco 是否可见 | 判断当前是否能读取 R1 指令 |
| ArUco id | 区分继续入梅林、对齐完成、放置层数等指令 |
| instruction | 解析后的任务语义 |
| ArUco 位姿 | 对抗区靠近和对齐 R1 时作为参考 |
| 最后可信时间 | 防止使用过期指令 |

建议 topic：

| topic | 内容 |
|---|---|
| `/r2/perception/r1_aruco_state` | `visible`、`code_id`、`instruction`、`pose`、`confidence`、`last_seen_stamp` |

建议 instruction 枚举：

| 指令 | 用途 |
|---|---|
| `CONTINUE_TO_MF` | 卡住或等待后继续进入梅林 |
| `R1_GRID_ALIGN_READY` | R1 手操已完成对齐，R2 可以准备爬上 R1 |
| `PLACE_GRID_LAYER2` | R2 放置二层 |
| `PLACE_GRID_LAYER3` | R2 放置三层 |
| `NEXT_TASK_HINT` | 预留给后续策略提示 |

处理原则：

1. ArUco 不可见时，不让整棵树在非必要阶段永久等待。
2. 可见时更新 `/r2/perception/r1_aruco_state`；不可见时只允许使用未过期的最近可信状态。
3. 高风险动作，例如进入梅林恢复、爬上 R1、放置 KFS，必须检查 id、instruction 和时间戳。

## 网页 KFS map 与地图状态机

当前待定方案是：对方完成 KFS 放置后，操作员通过网页手动确认 1-12 号树林方块上的 KFS 类型。网页只作为初始输入，不直接成为 BT 的业务逻辑。

建议链路：

```text
网页人工录入
-> /kfs_locator/state
-> KFS map manager 校验
-> /r2/perception/kfs_map
-> /r2/forest/request_plan
-> /r2/forest/map_state
```

人工 map 必须包含：

| 字段 | 含义 |
|---|---|
| `source` | `MANUAL_SETUP` |
| `confirmed` | 操作员已确认 |
| `locked` | 比赛开始后不允许随意修改 |
| `map_version` | 每次提交或动作更新递增 |
| `validation_errors[]` | 不满足规则时列出错误 |

比赛中的更新策略：

| 事件 | map 更新 |
|---|---|
| 成功吸取某方块 R2 KFS | 目标方块改为 `EMPTY`，加入 `removed_r2_blocks` |
| 临时放下 R2 KFS | 目标方块记录 `TEMP_R2_KFS` 或等效状态 |
| 视觉与人工 map 不一致 | 标记 `CONFLICT/UNKNOWN`，触发重规划或人工恢复 |
| R1 KFS 被移除 | 对应方块加入 `r1_wait_blocks` 历史并更新可通行性 |
| payload 丢失 | `payload_state=UNKNOWN/NONE`，暂停继续执行 |

## rosbag 必录 topic

为后续调研和离线回放，建议录制：

```bash
/manual_start
/cmd_vel
/odom
/tf
/tf_static
/kfs_locator/state
/kfs_tracker/detection
/r2/safety/emergency_stop_state
/r2/retry/request
/r2/match/elapsed_time
/r2/perception/r1_aruco_state
/r2/perception/kfs_map
/r2/forest/map_state
/r2/perception/payload_state
/r2/chassis/wheel_height_state
/r2/chassis/travel_mode_state
/r2/chassis/motion_state
/r2/manipulation/suction_state
/r2/manipulation/arm_state
/r2/perception/grid_state
/r2/navigation/navigate_to_named_pose/_action/status
/r2/navigation/navigate_to_named_pose/_action/feedback
/r2/navigation/navigate_to_named_pose/_action/result
/r2/forest/execute_forest_plan/_action/status
/r2/forest/execute_forest_plan/_action/feedback
/r2/forest/execute_forest_plan/_action/result
/r2/motion/place_kfs_on_grid/_action/status
/r2/motion/place_kfs_on_grid/_action/feedback
/r2/motion/place_kfs_on_grid/_action/result
```

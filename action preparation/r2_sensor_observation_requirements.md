# R2 Sensor Observation Requirements

本文档描述后续 action 需要读取哪些信息、怎样判断当前状态、以及应输出哪些状态供行为树和 rosbag 调研使用。

## 总体要求

所有关键状态都应带：

- `stamp`：时间戳。
- `frame_id`：坐标系，视觉/点云/位姿必须明确。
- `confidence`：识别或估计置信度。
- `source`：状态来源，例如 `front_depth_camera`、`arm_depth_camera`、`qr_detector`、`suction_pressure`。
- `failure_reason` 或 `diagnostic_text`：失败原因或诊断文本。

行为树不应直接订阅一堆原始图像/点云来做判断，而应读取经过外部节点汇总后的任务级状态。

## 底盘与抬升

### 需要读取的信息

| 信息 | 来源 | 用途 |
|---|---|---|
| 四轮红外相对对地高度 | 四个轮端红外 | 判断普通地面、树林高低方块、轮端悬空/顶死 |
| 丝杠圈数 | 丝杠电机编码或估算 | 开环记录目标高度与实际动作量 |
| 丝杠电流 | 电机驱动反馈 | 判断抬升受阻、触底、过载 |
| 底盘里程计 | `/odom` 或底盘驱动 | 判断移动是否发生、闭环控制 |
| 速度命令 | `/cmd_vel` | rosbag 中判断 BT/导航是否输出运动指令 |
| TF | `/tf`、`/tf_static` | 判断 `map/odom/base_link` 是否连通 |
| 前方障碍 | 激光雷达、前方深度相机 | 树林入口、方块边缘、障碍物检测 |

### 建议状态 topic

| topic | 内容 |
|---|---|
| `/r2/chassis/wheel_height_state` | 四轮红外高度、丝杠圈数、电流、目标高度、状态 |
| `/r2/chassis/travel_mode_state` | `GROUND/FOREST/TRANSITION/RECOVERY`，模式是否到位 |
| `/r2/chassis/motion_state` | 当前速度、目标速度、是否卡住、是否抖动 |

### 典型判断

| 判断 | 输入 | 成功条件 | 异常 |
|---|---|---|---|
| 普通地面可行驶 | 四轮红外、丝杠高度 | 四轮高度在普通地面阈值内 | 某轮高度异常或丝杠过流 |
| 已进入树林模式 | 丝杠圈数、电流、红外高度 | 四轮目标高度到位且车体可移动 | 开环圈数到位但红外高度不可信 |
| 方块间移动完成 | 里程计、红外、深度相机 | 车体到目标方块安全区域 | 完全进入有 KFS 方块风险 |

## 前方激光雷达与前方深度相机

### 需要读取的信息

| 信息 | 用途 |
|---|---|
| 障碍距离 | 导航避障、树林入口对齐、对抗区靠近九宫格 |
| 方块边缘/高度变化 | 树林方块入口、方块间移动安全判断 |
| 九宫格粗位姿 | 对抗区对齐和视觉初始定位 |
| 动态障碍 | 防止与 R1 或其他物体发生碰撞 |

### 建议状态 topic

| topic | 内容 |
|---|---|
| `/r2/perception/front_obstacle_state` | 最近障碍距离、方向、置信度 |
| `/r2/perception/forest_edge_state` | 树林方块边界、可通行方向 |
| `/r2/perception/grid_pose_hint` | 九宫格粗位姿和置信度 |

## 机械臂深度相机

### 需要读取的信息

| 信息 | 用途 |
|---|---|
| 端头位姿 | 端头架抓取 |
| 长杆/快速接头位姿 | 兵器组装 |
| KFS 位姿 | 相邻方块 KFS 抓取 |
| KFS 类型 | R2 KFS / R1 KFS / 假KFS / UNKNOWN 分类 |
| 九宫格 cell 位姿 | 中层放置 |
| 放置后状态 | 确认 KFS 是否留在目标格 |

### 建议状态 topic

| topic | 内容 |
|---|---|
| `/r2/perception/tip_detections` | 端头检测框、位姿、slot、置信度 |
| `/r2/perception/kfs_detections` | KFS 类型、位姿、所属方块、置信度 |
| `/r2/perception/grid_state` | 九宫格位姿、空格/占用、cell pose |

### KFS 分类要求

| 类型 | 规则意义 | 行为 |
|---|---|---|
| `R2_KFS` | R2 在梅林应收集的目标 | 可规划抓取 |
| `R1_KFS` | R2 在梅林不得接触 | 标为障碍/禁止目标 |
| `FAKE_KFS` | R1/R2 都不得接触和移动 | 标为硬禁止目标 |
| `UNKNOWN` | 分类不可靠 | 不抓取，只可重新识别或请求人工确认 |
| `EMPTY` | 方块空 | 可用于通行规划 |

## 夹爪与吸盘

### 需要读取的信息

| 信息 | 来源 | 用途 |
|---|---|---|
| 夹爪开合状态 | 夹爪驱动 | 端头是否夹住 |
| 夹爪电流/力 | 夹爪驱动 | 端头接触确认、夹取失败诊断 |
| 吸盘开关 | 吸盘控制 | 判断当前命令 |
| 真空压力 | 吸盘传感器 | 判断 KFS 是否吸附 |
| 机械臂关节状态 | 机械臂驱动 | 动作是否完成、是否进入安全姿态 |
| 视觉确认 | 机械臂深度相机 | 二次确认载荷类型和位置 |

### 建议状态 topic

| topic | 内容 |
|---|---|
| `/r2/manipulation/gripper_state` | 夹爪开合、力/电流、是否夹持端头 |
| `/r2/manipulation/suction_state` | 吸盘开关、真空压力、吸附可信度 |
| `/r2/perception/payload_state` | 当前携带物类型、夹持方式、置信度 |

### 载荷状态规则

| 状态 | 行为约束 |
|---|---|
| `NONE` | 可抓取端头或 KFS |
| `TIP` | 只能进入兵器组装相关动作 |
| `R2_KFS` | 可离开树林、进入对抗区、中层放置 |
| `R1_KFS` | 需要立即进入异常处理，梅林中不应由 R2 抓取 |
| `FAKE_KFS` | 严重异常，必须停车并进入重试/人工处理 |
| `UNKNOWN` | 禁止继续抓取或放置，先重新识别 |

## R1 二维码

### 需要读取的信息

| 信息 | 用途 |
|---|---|
| 二维码是否可见 | 判断是否能直接读取 R1 状态 |
| 二维码内容 | R1 状态、下一步任务、对齐提示 |
| 二维码位姿 | 对抗区九宫格附近辅助对齐 |
| 最后可信时间 | 判断状态是否过期 |

### 建议状态 topic

| topic | 内容 |
|---|---|
| `/r2/perception/r1_qr_state` | `visible`、`state`、`pose`、`confidence`、`last_seen_stamp` |

### 状态枚举建议

| 状态 | 用途 |
|---|---|
| `UNKNOWN` | 没看到或读不可信 |
| `R1_READY_FOR_ASSEMBLY` | 武馆内配合组装 |
| `R1_LEFT_MC` | R1 已离开武馆，R2 可进入下一判断 |
| `R1_ENTERED_MF` | R1 已进入梅林，R2 可按规则进入梅林 |
| `R1_AT_GRID_ALIGN_POSE` | 对抗区可用二维码辅助对齐九宫格 |
| `NEXT_TASK_HINT` | R1 明示下一阶段任务提示 |

### 处理原则

1. 二维码不可见时，不应让整棵树永久等待，除非当前阶段规则必须确认。
2. 可见时更新状态缓存；不可见时使用最近可信状态，但必须检查 `max_age_sec`。
3. 对高风险动作，例如 R2 离开武馆、进入梅林、对齐九宫格，应优先用二维码加本地视觉/区域判断双重确认。

## 树林 KFS map

### 建议数据结构

每个方块记录：

| 字段 | 含义 |
|---|---|
| `block_id` | 1-12 |
| `occupancy` | `EMPTY/OCCUPIED/UNKNOWN/OCCLUDED` |
| `kfs_type` | `R1_KFS/R2_KFS/FAKE_KFS/UNKNOWN/NONE` |
| `kfs_pose` | KFS 在 map 或 forest frame 下的位姿 |
| `reachable_from_current` | 当前方块是否可合法触达 |
| `adjacent_to_current` | 是否与当前方块相邻 |
| `confidence` | 视觉分类置信度 |
| `last_seen_stamp` | 最后观测时间 |

### rosbag 必录 topic

为后续调研和离线回放，建议录制：

```bash
/manual_start
/cmd_vel
/odom
/tf
/tf_static
/r2/chassis/wheel_height_state
/r2/chassis/travel_mode_state
/r2/perception/payload_state
/r2/perception/kfs_map
/r2/perception/r1_qr_state
/r2/perception/grid_state
/r2/manipulation/gripper_state
/r2/manipulation/suction_state
/r2/navigation/navigate_to_named_pose/_action/status
/r2/navigation/navigate_to_named_pose/_action/feedback
/r2/navigation/navigate_to_named_pose/_action/result
```


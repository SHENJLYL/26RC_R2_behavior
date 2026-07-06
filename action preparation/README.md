# R2 Action Preparation

本目录用于归档 R2 行为树后续接入 topic、action、service 时的接口准备内容。

这些文档只做接口语义、任务边界和调试记录设计，不把视觉算法、梅林路径搜索、机械臂轨迹和底盘闭环控制写进行为树节点。后续实现时优先复用现有 R2 接口；只有确认没有可复用接口时，才新增最小 wrapper。

## 当前任务线

当前 R2 主线为 no-tip 流程：

```text
启动
-> 到武馆/梅林交界处等待
-> 比赛开始后 120 秒或收到 R1 ArUco 继续指令
-> 到梅林树林入口，也就是 2 号平台面前
-> 读取网页人工录入 KFS 分布
-> 调用梅林路径规划逻辑得到 ForestSteps
-> 按步骤移动、复核、拿取、临时放置、更新地图状态
-> 经 10/11/12 号方块离开树林
-> 到对抗区九宫格附近
-> 等 R1 手操对齐 ArUco
-> 爬上 R1
-> 按 R1 ArUco 指令放置二层或三层 KFS
```

当前不执行端头夹取和兵器组装任务。端头相关接口可以保留在其它包中，但本目录的当前优先级不围绕该任务展开。

## 来源

本文档综合以下本地材料整理：

| 来源 | 当前用途 |
|---|---|
| `26RC_R2_behavior` 行为树 XML 与 md | 行为树主流程、占位节点、接入记录 |
| `26RC_R2_interfaces` / `rc26_r2_interfaces` action 与 msg | 底层成员已提供或拟提供的任务接口 |
| `web_spoiler` | 网页人工录入对手 KFS 摆放，发布 `/kfs_locator/state` |
| `meilin_router/router_planner` | 梅林路径判断与 ForestSteps 生成逻辑，目前仍需封装 |
| `KFS-Tracker` | KFS 视觉识别输出，后续用于机械臂相机单次复核 |
| 赛事 PDF / KFS 图像资料 | 统一术语、确认 KFS 识别对象 |

## 设计原则

1. 行为树只决定“什么时候做什么、失败后走哪条恢复分支”。
2. 耗时动作优先做成 action，例如导航、上/下台阶、执行 ForestSteps、爬上 R1、放置 KFS。
3. 单次检查优先做成 service，例如校验 KFS map、复核某个方块 KFS、检查 R1 KFS 是否已移除。
4. 持续状态优先做成 topic，例如 ArUco 状态、载荷状态、底盘高度、地图状态机。
5. R2 只能携带一个由吸盘吸取的 R2 KFS，`payload_state` 必须作为全局保护条件。
6. R1 指令源按 ArUco 设计，不再按非 ArUco 码设计。
7. 网页人工 KFS map 是初始信息；比赛中仍要由动作结果和视觉复核更新 `current_kfs_map`。
8. `removed_r2_blocks` 必须进入地图维护状态机并录入 rosbag，方便后续调研。

## 目录内容

| 文件 | 内容 |
|---|---|
| `r2_action_interface_catalog.md` | 完整 topic/action/service 清单，包含输入、读取信息、输出和 BT 用法 |
| `r2_sensor_observation_requirements.md` | 传感器、检测量、状态 topic 和 rosbag 需求 |
| `r2_behavior_action_sequence.md` | 从 no-tip 行为树阶段映射到未来 action/service/topic 接入顺序 |

## 当前接入优先级

1. 健康检查、急停、软停、重试请求和 rosbag 诊断状态。
2. `ReadArUco` wrapper：继续进梅林、R1 手操对齐、二层/三层放置指令。
3. 网页 KFS map 桥接：`/kfs_locator/state` -> `/r2/perception/kfs_map`。
4. 梅林规划封装：把 `meilin_router/router_planner` 的 `topic1 -> topic2` 包装成可请求、可追踪的 service/action。
5. 机械臂深度相机单次复核：`ValidateKFSOnBlock`。
6. 相机判断路径阻碍中的 R1 KFS 是否已被 R1 移除。
7. 梅林地图维护状态机：`initial_kfs_map`、`current_kfs_map`、`removed_r2_blocks`、`current_block`、`payload_state`。
8. `ExecuteForestPlan`：按 ForestSteps 调用上/下台阶、旋转、相邻移动、吸盘抓取、临时放置。
9. 对抗区：R1 对齐、爬上 R1、按 ArUco 放二层/三层 KFS。

## 统一术语

| 术语 | 说明 |
|---|---|
| `武馆(MC)` | 比赛一区，R2 启动和进入梅林前等待的位置来源 |
| `梅林(MF)` | 比赛二区，包含树林、R1 通道、R2 入口区和 R2 出口区 |
| `树林` | 梅林内 1-12 号高低方块区域；口头“梅花林”在文档中统一写作树林 |
| `KFS` | 武术秘籍，分为 `R1 KFS`、`R2 KFS`、`假KFS` |
| `R1 ArUco` | R1 上显示的 ArUco 指令码 |
| `九宫格` | 对抗区 3x3 架子，当前需要按 R1 ArUco 指令完成二层或三层放置 |
| `地图维护状态机` | 记录开局 KFS 分布、当前 KFS 分布、R2 已移除方块和执行进度的状态机 |

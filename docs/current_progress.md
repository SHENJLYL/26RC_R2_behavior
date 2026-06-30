# RC R2 Behavior Tree Current Progress

Date: 2026-06-30

## Goal

Create an independent ROS 2 / BehaviorTree.CPP package in `26RC_R2_behavior` for ROBOCON 2026 R2. The package should be runnable on its own, while PB2025 is only used as a source-layout and node-interface reference.

## Source Material Included

Sakiko/R2 material read for requirements:

- `sakiko/md/r2_behavior_tree_design.md`
- `sakiko/md/r2_target_behavior_tree_plan.md`
- `sakiko/md/r2_future_behavior_tree_node_reference_zh.md`
- `sakiko/rc26_nav_test-main/R2_WORKFLOW.md`
- `sakiko/rc26_nav_test-main/R2_NAVIGATION_DESIGN.md`
- `sakiko/rc26_nav_test-main/src/rc2026_navigation/README.md`
- `sakiko/rc26_nav_test-main/src/rc2026_navigation/rc2026_navigation/outside_nav_controller.py`
- `sakiko/rc26_nav_test-main/src/r2_suction_control/src/r2_suction_control.cpp`

PB2025 files used only as implementation references:

- `pb2025_sentry_behavior/package.xml`
- `pb2025_sentry_behavior/CMakeLists.txt`
- `pb2025_sentry_behavior/src/pb2025_sentry_behavior_server.cpp`
- `pb2025_sentry_behavior/src/pb2025_sentry_behavior_client.cpp`
- `pb2025_sentry_behavior/launch/pb2025_sentry_behavior_launch.py`
- `pb2025_sentry_behavior/params/sentry_behavior.yaml`
- `pb2025_sentry_behavior/plugins/action/pub_nav2_goal.cpp`
- `pb2025_sentry_behavior/plugins/action/pub_twist.cpp`
- `pb2025_sentry_behavior/plugins/condition/is_manual_start.cpp`

## Source Material Not Included Yet

- `sakiko/md/r2_bt_and_env_grill_plan.md`
- `sakiko/md/r2_bt_and_env_grill_plan_zh.md`
- `sakiko/md/robocon_wulin_rules_v6.pdf`
- `sakiko/rc26_nav_test-main/Appendix V1.1.pdf`
- `sakiko/rc26_nav_test-main/第二十五届全国大学生机器人大赛ROBOCON_u201C武林探秘_u201D竞技赛规则V6.pdf`
- `sakiko/rc26_nav_test-main/R2_GENERAL_NAVIGATION_PACKAGE_PROMPT.md`
- `sakiko/rc26_nav_test-main/R2_OUTSIDE_NAVIGATION_DESIGN.md`
- `sakiko/rc26_nav_test-main/README.md`

Reason: the markdown and source files above were enough for the first independent runtime and BT skeleton. The PDFs and remaining plans should be audited before real-match deployment.

## Created Package

Package name: `r2_behavior`

Current strategy: independent R2 behavior package. It contains its own:

- `r2_behavior_server`
- `r2_behavior_client`
- `IsManualStart`
- `PubNav2Goal`
- `PublishTwist`
- R2 XML behavior trees
- launch, params, tests, and documentation

It no longer depends on `pb2025_sentry_behavior` at runtime.

## Behavior Trees

### `R2_Competition_Main`

```text
PreMatchAndStart
-> MC_AssembleWeapon
-> WaitForR1BeforeR2LeavesMC
-> MF_CollectSingleR2KFS
-> MF_ExitForest
-> Battlefield_PlaceMiddleLayer
-> StopAllMotion
```

Emergency stop and retry branches are present but still use placeholders until the real wired/safety/retry interfaces are defined.

### `R2_DryRun_WithMocks`

Smoke-test tree that exercises:

- `PubNav2Goal` on `goal_pose`
- `PublishTwist` on `cmd_vel`

Use this to test tree loading and topic publishing without claiming the full robot task is connected.

## Current Nodes

| Node | Implemented in | Interface |
|---|---|---|
| `IsManualStart` | `plugins/condition/is_manual_start.cpp` | subscribes `manual_start` `std_msgs/msg/Int32` through `r2_behavior_server` |
| `PubNav2Goal` | `plugins/action/pub_nav2_goal.cpp` | publishes `geometry_msgs/msg/PoseStamped`, default topic `goal_pose`, fixed frame `map` |
| `PublishTwist` | `plugins/action/pub_twist.cpp` | publishes `geometry_msgs/msg/Twist`, default topic `cmd_vel` |

BehaviorTree.CPP built-ins still used:

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

## Real Interfaces Connected Now

| Capability | Interface | Status |
|---|---|---|
| Manual start | `manual_start` `std_msgs/msg/Int32` | Connected |
| Outside navigation goal topic | `goal_pose` `geometry_msgs/msg/PoseStamped` | Connected with placeholder coordinates |
| Stop chassis | `cmd_vel` `geometry_msgs/msg/Twist` | Connected for zero-speed publish |

## Placeholders Kept For Real-Car Debugging

| Capability | Placeholder node/interface |
|---|---|
| Health monitor | `/r2/health/status` |
| Emergency stop input | `/r2/emergency_stop` |
| Retry handling | `/r2/retry/request`, `/r2/retry/execute` |
| Spearhead detection/pick | `/r2/manipulation/pick_tip` |
| R1 assembly readiness | `/r2/perception/r1_assembly_ready` |
| R1 entered MF | `/r2/perception/r1_entered_mf` |
| Forest entry/exit primitives | `/r2/forest/enter`, `/r2/forest/exit` |
| KFS block map | `/kfs_tracker/detection -> /r2/perception/kfs_map` |
| Forest graph planner | `/r2/forest/planner` |
| Woods graph navigation | `/r2/nav/go_to_woods_block`, `/r2/nav/follow_woods_graph_path` |
| Adjacent KFS pick | `/r2/manipulation/pick_adjacent_kfs` |
| Suction command and payload state | `/cmd_suction_suck`, `/r2/perception/payload_state` |
| Grid state and target cell | `/r2/perception/grid_state`, `/r2/strategy/select_grid_cell` |
| Middle-layer place | `/r2/manipulation/place_kfs_middle` |

## Known Gaps

- Waypoints are still `0.0;0.0;0.0`.
- `PubNav2Goal` currently uses `frame_id="map"`; verify against real navigation.
- Suction has command topic but no feedback loop.
- KFS tracker output is not yet converted into a block-indexed map.
- MoveIt2 task-level actions are not wrapped yet.
- Forest graph traversal and rule guard are not implemented.
- Rule PDFs need a later audit before real competition use.

## Verified

- Independent runtime layout test passes: `python test/test_independent_runtime_layout.py`.
- XML parse checks pass for both behavior trees.
- `colcon build` passes with build/install/log kept under `26RC_R2_behavior`.
- Dry-run server loads `R2_DryRun_WithMocks`; the test run was stopped by `timeout` after confirming the tree loaded and ticked.

## Minimal Next Debug Steps

1. Source ROS and build this package.
2. Launch `r2_behavior` with `r2_behavior_dry_run.yaml`.
3. Confirm `goal_pose` and `cmd_vel` with `ros2 topic echo`.
4. Replace waypoint placeholders with calibrated poses.
5. Implement one real interface at a time, starting with health check, payload feedback, KFS map, and forest rule guard.

## Follow-up Judgement Record

See `docs/integration_judgement.md` for the current integration judgement, reuse policy, and future research checklist.

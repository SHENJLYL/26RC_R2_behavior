# RC R2 Behavior Tree Current Progress

Date: 2026-06-30

## Goal

Create an RC R2 behavior-tree package in `26RC_R2_behavior` by following the
`pb2025_sentry_behavior` source layout and reading the R2 materials in `sakiko`.
The result is executable ROS 2/BehaviorTree.CPP package content, not only a plan.

## Source Material Included

These files were read and used as behavior-tree requirements:

- `sakiko/md/r2_behavior_tree_design.md`
- `sakiko/md/r2_target_behavior_tree_plan.md`
- `sakiko/md/r2_future_behavior_tree_node_reference_zh.md`
- `sakiko/rc26_nav_test-main/R2_WORKFLOW.md`
- `sakiko/rc26_nav_test-main/R2_NAVIGATION_DESIGN.md`
- `sakiko/rc26_nav_test-main/src/rc2026_navigation/README.md`
- `sakiko/rc26_nav_test-main/src/rc2026_navigation/rc2026_navigation/outside_nav_controller.py`
- `sakiko/rc26_nav_test-main/src/r2_suction_control/src/r2_suction_control.cpp`

These PB2025 files were read and used as implementation pattern references:

- `pb2025_sentry_behavior/package.xml`
- `pb2025_sentry_behavior/CMakeLists.txt`
- `pb2025_sentry_behavior/src/pb2025_sentry_behavior_server.cpp`
- `pb2025_sentry_behavior/src/pb2025_sentry_behavior_client.cpp`
- `pb2025_sentry_behavior/launch/pb2025_sentry_behavior_launch.py`
- `pb2025_sentry_behavior/params/sentry_behavior.yaml`
- `pb2025_sentry_behavior/behavior_trees/rmul_2025.xml`
- `pb2025_sentry_behavior/behavior_trees/rmul_2025_reality.xml`
- `pb2025_sentry_behavior/plugins/action/pub_nav2_goal.cpp`
- `pb2025_sentry_behavior/plugins/action/send_nav2_goal.cpp`
- `pb2025_sentry_behavior/plugins/action/pub_twist.cpp`

## Source Material Not Included Yet

These files exist but were not directly converted into BT code in this pass:

- `sakiko/md/r2_bt_and_env_grill_plan.md`
- `sakiko/md/r2_bt_and_env_grill_plan_zh.md`
- `sakiko/md/robocon_wulin_rules_v6.pdf`
- `sakiko/rc26_nav_test-main/Appendix V1.1.pdf`
- `sakiko/rc26_nav_test-main/第二十五届全国大学生机器人大赛ROBOCON_u201C武林探秘_u201D竞技赛规则V6.pdf`
- `sakiko/rc26_nav_test-main/R2_GENERAL_NAVIGATION_PACKAGE_PROMPT.md`
- `sakiko/rc26_nav_test-main/R2_OUTSIDE_NAVIGATION_DESIGN.md`
- `sakiko/rc26_nav_test-main/README.md`

Reason: the markdown files already captured the actionable R2 BT constraints
needed for the first implementation pass. The PDFs and remaining planning docs
should be used for later rule audit before real-match deployment.

## Created Package

Package name: `r2_behavior`

Current strategy: R2 behavior-tree asset package only. It reuses
`pb2025_sentry_behavior` runtime and plugins, and does not add a custom BT
runner or custom C++ BT plugins.

Created structure:

- `CMakeLists.txt`
- `package.xml`
- `behavior_trees/r2_competition_main.xml`
- `behavior_trees/r2_dry_run_with_mocks.xml`
- `params/r2_behavior.yaml`
- `params/r2_behavior_dry_run.yaml`
- `launch/r2_behavior_launch.py`
- `docs/current_progress.md`
- `README.md`

## Behavior Trees

### `R2_Competition_Main`

Complete mission chain:

```text
PreMatchAndStart
-> MC_AssembleWeapon
-> WaitForR1BeforeR2LeavesMC
-> MF_CollectSingleR2KFS
-> MF_ExitForest
-> Battlefield_PlaceMiddleLayer
-> StopAllMotion
```

Emergency stop and retry branches are present at the top level. Their inputs are
placeholders until the real wired/safety/retry interfaces are defined.

### `R2_DryRun_WithMocks`

Smoke-test tree that allows placeholder conditions to succeed and exercises:

- `goal_pose`
- `cmd_vel`

Use this to test the BT loader and basic topic output without claiming the full
robot task is connected.

## Reused BT Nodes

Runtime and plugins are reused from `pb2025_sentry_behavior`.

Reused PB2025 nodes:

- `IsManualStart`
- `PubNav2Goal`
- `PublishTwist`

Reused BehaviorTree.CPP built-ins:

- `AlwaysSuccess`
- `AlwaysFailure`
- `Sequence`
- `ReactiveFallback`
- `SubTree`

Missing robot interfaces are preserved as XML comments plus `AlwaysFailure`
placeholders. No custom placeholder C++ plugin is added.

## Real Interfaces Connected Now

| Capability | Interface | Status |
|---|---|---|
| Manual start | `manual_start` `std_msgs/msg/Int32` | Reused `IsManualStart` |
| Outside navigation goal topic | `goal_pose` `geometry_msgs/msg/PoseStamped` | Reused `PubNav2Goal` |
| Stop chassis | `cmd_vel` `geometry_msgs/msg/Twist` | Reused `PublishTwist` |

## Placeholders Kept For Real-Car Debugging

| Capability | Placeholder node/interface |
|---|---|
| Health monitor | `PreMatchSelfCheck`, `/r2/health/status` |
| Emergency stop input | `IsEmergencyStopRequested`, `/r2/emergency_stop` |
| Retry handling | `IsRetryRequested`, `/r2/retry/request`, `/r2/retry/execute` |
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

- Waypoint coordinates are left as `0.0;0.0;0.0` placeholders. Replace them with
  calibrated red/blue task waypoints before driving the real robot.
- The suction controller has a command topic but no feedback topic, so the BT
  cannot yet prove successful pickup or release.
- `KFS-Tracker` exists, but its detection output is not yet converted into a
  block-indexed `kfs_map` with R2/R1/fake classification and confidence.
- MoveIt2 config exists, but no task-level BT action wraps tip pick, KFS pick,
  or grid placement yet.
- Forest graph traversal and rule guard are not implemented. Until they are,
  the real competition tree intentionally blocks at placeholder nodes.
- The PDFs should be re-audited before real competition use to catch any rule
  updates or interpretations not already reflected in the markdown notes.

## Minimal Next Debug Steps

1. Build `r2_behavior` with build products kept under `26RC_R2_behavior`:
   `colcon --log-base 26RC_R2_behavior/log build --base-paths 26RC_R2_behavior --packages-select r2_behavior --build-base 26RC_R2_behavior/build --install-base 26RC_R2_behavior/install --cmake-clean-cache --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3`.
2. Ensure `pb2025_sentry_behavior` and its dependencies are built and sourced in the same ROS 2 environment.
3. Launch `rc2026_navigation`.
4. Launch `r2_behavior` with `r2_behavior_dry_run.yaml` and verify `goal_pose` and `cmd_vel`.
5. Replace waypoint placeholders with calibrated task poses.
6. Implement one placeholder interface at a time, starting with health check,
   payload feedback, KFS block map, and forest rule guard.

## Follow-up Judgement Record

See `docs/integration_judgement.md` for the current integration judgement,
reuse-first policy, and future research checklist.

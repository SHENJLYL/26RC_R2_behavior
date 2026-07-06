# RC R2 Behavior Tree Current Progress

Date: 2026-07-06

## Goal

Create the ROBOCON 2026 R2 behavior tree content in `26RC_R2_behavior`, including the ROS 2 / BehaviorTree.CPP runner, XML task flow, launch files, params, plugin nodes, and integration records.

The package follows the PB2025 behavior package pattern: server/client executables, launch files, params, XML behavior trees, and small plugin nodes.

## Current Strategy

The current R2 mission is a no-tip flow:

```text
Start
-> move to MC/MF boundary
-> wait 120 seconds or R1 ArUco continue instruction
-> move to forest entry in front of block 2
-> use web KFS map and forest planner
-> execute forest steps with arm-camera validation and camera-based R1 KFS removal checks
-> record removed R2 KFS blocks in map_state
-> exit MF via block 10/11/12
-> navigate to battlefield grid standoff
-> wait for R1 manual alignment ArUco
-> climb onto R1
-> place KFS on grid layer 2 or layer 3 according to ArUco code
```

The tip pickup and weapon assembly task is intentionally out of the current main flow.

## Competition Terminology

| Term | Meaning |
|---|---|
| `武馆(MC)` | Zone 1; R2 starts and waits before MF entry |
| `梅林(MF)` | Zone 2; includes the forest, R1 passage, R2 entrance, and R2 exit |
| `树林` | The 12 numbered blocks inside MF; R2 operates here |
| `对抗区` | Zone 3; includes ramp, nine-grid rack, and retry zone |
| `端头` | Official term for the weapon tip; not executed in the current no-tip flow |
| `KFS` | Kung Fu Scroll; includes R1 KFS, R2 KFS, and fake KFS |
| `R1 ArUco` | R1-displayed ArUco instruction code |
| `九宫格` | 3x3 rack in the battlefield area, with bottom/middle/top layers |

## Current File Structure

```text
26RC_R2_behavior/
├── behavior_trees/
│   ├── r2_competition_main.xml
│   ├── r2_no_tip_competition_draft.xml
│   ├── r2_dry_run_with_mocks.xml
│   └── r2_full_flow_verification.xml
├── include/r2_behavior/
├── plugins/
├── src/
├── launch/
├── params/
├── docs/
│   ├── current_progress.md
│   ├── important_files_line_notes.md
│   ├── integration_judgement.md
│   └── r2_no_tip_behavior_tree_design.md
├── action preparation/
│   ├── README.md
│   ├── r2_action_interface_catalog.md
│   ├── r2_sensor_observation_requirements.md
│   └── r2_behavior_action_sequence.md
└── test/
```

## Behavior Trees

| Tree | Status |
|---|---|
| `R2_Competition_Main` | Current no-tip safe main skeleton. Unknown nodes use `AlwaysFailure` placeholders. |
| `R2_NoTip_Competition_Main` | Detailed no-tip draft with interface comments. Do not run as mission tree until wrappers exist. |
| `R2_Full_Flow_Verification` | Mocked no-tip flow verification using `AlwaysSuccess` placeholders. |
| `R2_DryRun_WithMocks` | Minimal server/topic smoke test. |

## Current Nodes

| Node | Implemented in | Interface |
|---|---|---|
| `IsManualStart` | `plugins/condition/is_manual_start.cpp` | subscribes `/manual_start` `std_msgs/msg/Int32` |
| `NavigateToNamedPose` | `plugins/action/navigate_to_named_pose.cpp` | calls `/r2/navigation/navigate_to_named_pose` action |
| `PubNav2Goal` | `plugins/action/pub_nav2_goal.cpp` | publishes `geometry_msgs/msg/PoseStamped`, default topic `goal_pose` |
| `PublishTwist` | `plugins/action/pub_twist.cpp` | publishes `geometry_msgs/msg/Twist`, default topic `cmd_vel` |

## Real Interfaces Connected Now

| Capability | Interface | Status |
|---|---|---|
| Manual start | `/manual_start` | Connected |
| Named-pose navigation | `/r2/navigation/navigate_to_named_pose` | BT action client connected; external server and waypoints must be validated separately |
| Outside navigation goal topic | `goal_pose` | Connected for debug/simple goals |
| Stop chassis | `cmd_vel` | Connected for zero-speed publish |

## Not Connected / Waiting For Integration

| Capability | Required interface direction |
|---|---|
| Health check | `/r2/health/check_start_ready` |
| Emergency stop | `/r2/safety/emergency_stop_state` |
| Retry recovery | `/r2/retry/request`, `/r2/retry/execute_area_aware_retry` |
| 120-second match wait | `/r2/match/elapsed_time` or BT timing wrapper |
| R1 ArUco instruction | `r2/perception/read_aruco`, `/r2/perception/r1_aruco_state` |
| Manual KFS map | `/kfs_locator/state` -> `/r2/perception/kfs_map` |
| Forest route planning | `router_planner` -> `/r2/forest/request_plan` |
| Forest map state | `/r2/forest/map_state`, `/r2/forest/update_map_state` |
| KFS block validation | `/r2/perception/validate_kfs_on_block`, using arm depth camera |
| R1 KFS removal check | `/r2/forest/check_r1_kfs_removed`, using camera |
| Travel mode and lift confirmation | `/r2/chassis/set_travel_mode`, wheel IR, screw feedback, LiDAR |
| Forest step execution | `/r2/forest/execute_forest_plan`, `/r2/forest/step_to_adjacent_block` |
| Adjacent KFS pick | `/r2/manipulation/pick_adjacent_kfs` |
| Payload feedback | `/r2/perception/payload_state`, `/r2/manipulation/suction_state` |
| Grid align and placement | `/r2/motion/align_to_r1`, `/r2/motion/place_kfs_on_grid` |

## Known Gaps

- Main no-tip tree is a safe skeleton; most task nodes are placeholders.
- Waypoints `mc_mf_boundary_standoff`, `forest_entry_block_2_standoff`, and `battlefield_grid_standoff` must be calibrated.
- `router_planner` currently uses `topic1 -> topic2`; a service/action wrapper is still needed.
- `KFS-Tracker` publishes detections but is not yet wrapped into block-indexed validation service.
- `PlaceKFS.action` currently lacks an explicit layer-3 mode.
- `AlignToR1` is referenced by documentation but the action file is not present yet.

## Verified

- Runtime layout test: `python3 test/test_independent_runtime_layout.py`.
- XML parse checks have been run for edited behavior trees.
- Dry-run and full-flow verification remain intended as safe test paths; real task nodes still require external wrappers.

## Next Steps

1. Confirm and implement the topic/action/service contracts listed in `action preparation/r2_behavior_action_sequence.md`.
2. Calibrate the three named waypoints.
3. Wrap `web_spoiler` and `router_planner` into behavior-tree friendly service/action interfaces.
4. Implement arm-camera KFS validation and camera-based R1 KFS removal checks.
5. Implement `/r2/forest/map_state` so `removed_r2_blocks` is recorded in rosbag.
6. Implement R1 ArUco-driven battlefield align, climb, and layer 2/3 placement.

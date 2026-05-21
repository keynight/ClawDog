# ClawDog Master Implementation Plan

## Overview

This plan synthesizes all project documentation into a unified, actionable roadmap for building the ClawDog autonomous quadruped robot. The project converts a cheap toy robot dog into a fully autonomous system using ESP32-S3 for real-time control and a companion computer (RPi5/Jetson) running ROS2 for high-level navigation and AI integration.

**Key Success Criteria:**
- Robot walks stably in trot gait
- Autonomous navigation with obstacle avoidance
- Safety systems functional (emergency stop, human detection, joint limits)
- 30-40 minute runtime on 2S LiPo
- Total weight under 750g

---

## Master Timeline

### Phase A: Hardware Assembly (Weeks 1-3)
*Based on README phases 0-2 and DESC hardware steps*

| Week | Milestone | Deliverable |
|------|-----------|-------------|
| 1 | Hardware Intelligence | PCB traced, original signals mapped, components spec'd |
| 2 | Power & Motors | Power system redesigned, SA8301 drivers mounted, wiring harness built |
| 3 | Sensor Integration | BNO055, LD2420, WitMotion D6, OLED mounted and wired |

### Phase B: Firmware Foundation (Weeks 4-6)
*Based on CLAUDE phases 1-3 and DESC calibration*

| Week | Milestone | Deliverable |
|------|-----------|-------------|
| 4 | Joint Control | ADC reads working, PWM output working, joints move under basic control |
| 5 | PID Tuning | All 4 joints hold position, smooth motion, joint limits enforced |
| 6 | Communication | UDP bridge working at 50Hz, ROS2 topics publishing/subscribing |

### Phase C: Software & Simulation (Weeks 7-9)
*Based on CLAUDE phases 4-5 and README phase 3*

| Week | Milestone | Deliverable |
|------|-----------|-------------|
| 7 | OpenClaw Integration | Gait generation working in simulation, trajectory publishing |
| 8 | URDF & Visualization | Robot model in RViz, joint states visualized, TF tree correct |
| 9 | First Steps | Static gait on floor, robot stands and walks with tether |

### Phase D: Autonomy (Weeks 10-13)
*Based on CLAUDE phases 6-8 and README phases 4-5*

| Week | Milestone | Deliverable |
|------|-----------|-------------|
| 10 | IMU Feedback | Tilt compensation working, adaptive gait on slopes |
| 11 | SLAM | LiDAR map building, robot localizes while teleoperated |
| 12 | Nav2 | Autonomous navigation to waypoints, obstacle avoidance |
| 13 | Safety Polish | Emergency stop tested, human detection active, documentation complete |

**Total Duration:** ~13 weeks part-time (5-10 hours/week)

---

## Phase A: Hardware Assembly

### Task A1: PCB Analysis and Signal Mapping

**Description:** Trace the original toy robot PCB to understand motor wiring, sensor connections, and power routing. Document without destroying original functionality.

**Acceptance criteria:**
- [ ] Original PCB traced with all motor driver ICs identified
- [ ] Motor winding resistances measured and recorded
- [ ] Original power rails identified (battery input, motor voltage, logic voltage)
- [ ] All test points documented with oscilloscope captures
- [ ] Photos of PCB front/back with annotations

**Verification:**
- [ ] Original robot still powers on and moves after analysis
- [ ] All signals documented in `docs/hardware/original_pcb.md`
- [ ] Motor specs recorded (voltage, current, resistance, gear ratio)

**Dependencies:** None

**Files likely touched:**
- `docs/hardware/original_pcb.md` (new)
- `docs/hardware/motor_specs.md` (new)

**Estimated scope:** Medium (3-5 files of documentation)

### Task A2: Power System Design

**Description:** Design and build the new power distribution system. Replace original battery with 2S LiPo, add dedicated 5V buck converter for LiDAR, add battery monitoring circuit.

**Acceptance criteria:**
- [ ] 2S LiPo connector mounted with polarity protection
- [ ] 5V buck converter (3A+) wired with dedicated LiDAR connector
- [ ] Battery voltage divider connected to ESP32 ADC for monitoring
- [ ] Power switch accessible on chassis
- [ ] Emergency stop button wired to cut motor power
- [ ] Current budget validated: ~2.7A typical, ~5.1A peak

**Verification:**
- [ ] All voltage rails measure correctly under load
- [ ] Battery monitor reads accurately (±0.1V)
- [ ] Emergency stop cuts all motor power within 100ms
- [ ] Power distribution documented in `docs/hardware/power.md`

**Dependencies:** Task A1

**Files likely touched:**
- `docs/hardware/power.md` (new)
- `docs/hardware/power_schematic.png` (new)

**Estimated scope:** Medium

### Task A3: Motor Driver Installation

**Description:** Install 4x SA8301 H-bridge drivers for independent motor control. Mount on perfboard or custom PCB, wire to ESP32 GPIOs.

**Acceptance criteria:**
- [ ] 4x SA8301 mounted on driver board with heatsinks
- [ ] Each driver wired per `docs/hardware/pinout.md`:
  - PWM pins: GPIO 1, 4, 7, 10
  - DIR pins: GPIO 2, 5, 8, 11
- [ ] Flyback diodes installed on all motor outputs
- [ ] Decoupling capacitors (100nF + 10uF) on each driver
- [ ] Motor connectors labeled 0-3

**Verification:**
- [ ] Each motor spins bidirectionally under manual test
- [ ] No overheating after 5 minutes continuous operation
- [ ] PWM control responsive from 0-80% duty cycle
- [ ] Wiring matches `docs/hardware/pinout.md` exactly

**Dependencies:** Task A2

**Files likely touched:**
- `docs/hardware/driver_board.md` (new)
- `docs/hardware/pinout.md` (update with measured values)

**Estimated scope:** Medium

### Task A4: Sensor Wiring Harness

**Description:** Build wiring harness for all sensors: potentiometers (4x ADC), BNO055 (I2C), WitMotion D6 (UART2), LD2420 (UART1), OLED (I2C).

**Acceptance criteria:**
- [ ] 4x potentiometer voltage dividers wired to ADC pins (GPIO 13, 6, 9, 12)
- [ ] I2C bus wired with 4.7kΩ pull-ups (SDA: GPIO 40, SCL: GPIO 41)
  - BNO055 on I2C addr 0x28
  - SSD1306 OLED on I2C addr 0x3C
- [ ] UART2 wired for WitMotion D6 (RX: GPIO 17, TX: GPIO 18, 230400 baud)
- [ ] UART1 wired for LD2420 (RX: GPIO 15, TX: GPIO 16, 115200 baud)
- [ ] All connectors keyed/labelled to prevent misconnection

**Verification:**
- [ ] All ADC pins read stable values when potentiometers moved
- [ ] I2C scan detects both BNO055 (0x28) and OLED (0x3C)
- [ ] UART1 receives data from LD2420 (human presence reported)
- [ ] UART2 receives data from WitMotion D6 (scan data visible)
- [ ] Continuity test passes for all connections

**Dependencies:** Task A3

**Files likely touched:**
- `docs/hardware/wiring_harness.md` (new)
- `docs/hardware/pinout.md` (update with actual wiring)

**Estimated scope:** Large (break into subtasks if needed)

### Checkpoint: Hardware Complete
- [ ] All components mounted securely (vibration-resistant)
- [ ] Power system validated under full load (all motors + sensors)
- [ ] Wiring matches documentation exactly
- [ ] Weight under 750g
- [ ] No shorts or loose connections
- [ ] **HUMAN REVIEW REQUIRED** before proceeding to firmware

---

## Phase B: Firmware Foundation

### Task B1: Project Bootstrap

**Description:** Initialize PlatformIO project for ESP32-S3-N16R8 with Arduino framework. Set up build configuration, debugging, and initial project structure.

**Acceptance criteria:**
- [ ] `firmware/platformio.ini` configured for ESP32-S3-N16R8 (16MB flash, 8MB PSRAM)
- [ ] Build succeeds with `-DARDUINO_USB_CDC_ON_BOOT=1`
- [ ] Serial monitor working at 115200 baud
- [ ] FreeRTOS task structure template created (Core 0: comms, Core 1: control)
- [ ] Project builds without warnings

**Verification:**
- [ ] `pio run` completes successfully
- [ ] `pio test` runs (even if no tests yet)
- [ ] Serial output visible: "ClawDog firmware booting..."
- [ ] Git commit with conventional commit format

**Dependencies:** Checkpoint: Hardware Complete

**Files likely touched:**
- `firmware/platformio.ini` (new)
- `firmware/src/main.cpp` (new)
- `firmware/include/config.h` (new)

**Estimated scope:** Small

### Task B2: Motor Control Abstraction

**Description:** Create motor driver class wrapping SA8301 H-bridge control. Implement PWM output, direction control, and duty cycle limiting.

**Acceptance criteria:**
- [ ] `MotorDriver` class with `setSpeed(float duty)` API (-1.0 to 1.0)
- [ ] PWM frequency set to 1kHz (adjustable via config)
- [ ] Duty cycle clamped to ±80% maximum (safety)
- [ ] Direction pin handled automatically based on sign
- [ ] All 4 motors independently controllable

**Verification:**
- [ ] Unit test: `MotorDriver.setSpeed(0.5)` outputs 50% PWM forward
- [ ] Unit test: `MotorDriver.setSpeed(-0.5)` outputs 50% PWM reverse
- [ ] Unit test: `MotorDriver.setSpeed(1.0)` clamped to 80%
- [ ] Manual test: Each motor responds to serial commands
- [ ] No motor movement on boot (all initialized to 0)

**Dependencies:** Task B1

**Files likely touched:**
- `firmware/src/motor_driver.cpp`
- `firmware/include/motor_driver.h`
- `firmware/test/test_motor_driver.cpp`

**Estimated scope:** Medium

### Task B3: ADC and Joint Position Reading

**Description:** Implement ADC reading for 4 joint potentiometers with moving average filtering. Map raw ADC to joint angles using calibration values.

**Acceptance criteria:**
- [ ] `JointSensor` class reads 12-bit ADC with 10-sample moving average
- [ ] ADC-to-angle mapping using calibrated min/max values
- [ ] Angle output in degrees or radians (configurable)
- [ ] Reading rate: 200Hz minimum
- [ ] `JointState` struct with position, velocity (computed), timestamp

**Verification:**
- [ ] Unit test: Known ADC input produces expected angle
- [ ] Unit test: Filtering reduces noise by >50%
- [ ] Manual test: Moving joint shows smooth angle output on serial plotter
- [ ] All 4 joints read simultaneously without crosstalk
- [ ] ADC values logged for calibration (move to extremes, record min/max)

**Dependencies:** Task B2

**Files likely touched:**
- `firmware/src/joint_sensor.cpp`
- `firmware/include/joint_sensor.h`
- `firmware/include/joint_state.h`
- `firmware/test/test_joint_sensor.cpp`

**Estimated scope:** Medium

### Task B4: Safety Subsystem

**Description:** Implement safety-critical systems: watchdog timer, joint limit enforcement, emergency stop, low battery cutoff.

**Acceptance criteria:**
- [ ] Watchdog timer: 100ms timeout, petted by ROS2 heartbeat
- [ ] Joint limits: Motion commands clamped to calibrated ADC min/max
- [ ] Emergency stop: Hardware button immediately sets all motor PWM to 0
- [ ] Low battery cutoff: Force sit pose when battery <6.0V
- [ ] All safety events logged with timestamp
- [ ] Safety state machine: INIT → ACTIVE → E_STOP → RECOVERY

**Verification:**
- [ ] Test: Missing heartbeat for 100ms triggers motor stop
- [ ] Test: Command beyond joint limit is clamped, event logged
- [ ] Test: E-stop button pressed → all motors stop within 50ms
- [ ] Test: Battery voltage 5.9V → robot forced to sit pose
- [ ] Test: Safety event log persisted to flash (optional)

**Dependencies:** Task B3

**Files likely touched:**
- `firmware/src/safety.cpp`
- `firmware/include/safety.h`
- `firmware/src/watchdog.cpp`
- `firmware/include/watchdog.h`

**Estimated scope:** Medium

### Task B5: PID Joint Position Control

**Description:** Implement PID controller for each joint to hold commanded position. Run at 200Hz on Core 1.

**Acceptance criteria:**
- [ ] `PIDController` class with configurable Kp, Ki, Kd
- [ ] Anti-windup on integral term
- [ ] Output rate-limited for smooth motion
- [ ] PID runs at 200Hz with consistent timing
- [ ] One leg at a time: Joint holds position against light disturbance

**Verification:**
- [ ] Unit test: Step response overshoot <20%, settling time <500ms
- [ ] Manual test: Joint holds position when pushed gently
- [ ] Manual test: All 4 joints can hold position simultaneously
- [ ] PID gains logged for each joint (start conservative: Kp=1.0, Ki=0, Kd=0)
- [ ] **CRITICAL:** Robot suspended during all PID tests (no ground contact)

**Dependencies:** Task B4

**Files likely touched:**
- `firmware/src/pid_controller.cpp`
- `firmware/include/pid_controller.h`
- `firmware/src/joint_controller.cpp`
- `firmware/include/joint_controller.h`

**Estimated scope:** Large

### Task B6: Sensor Integration (IMU, Radar, LiDAR)

**Description:** Interface with BNO055 (I2C), LD2420 (UART1), and WitMotion D6 (UART2). Publish sensor data at appropriate rates.

**Acceptance criteria:**
- [ ] BNO055: Read orientation (quaternion or Euler) at 100Hz
- [ ] BNO055: Calibration status reported (sys, gyro, accel, mag)
- [ ] LD2420: Human presence detected, distance reported
- [ ] LD2420: Emergency stop triggered when human in unsafe proximity
- [ ] WitMotion D6: Parse scan data, publish LaserScan-compatible format
- [ ] OLED: Display battery, gait mode, errors (10Hz refresh)

**Verification:**
- [ ] BNO055: Orientation data stable when IMU stationary
- [ ] LD2420: Detects human presence within 6m, reports distance
- [ ] WitMotion D6: Point cloud visible in serial output
- [ ] OLED: All status items visible and updating
- [ ] All sensors work simultaneously without bus conflicts

**Dependencies:** Task B5

**Files likely touched:**
- `firmware/src/imu_bno055.cpp`
- `firmware/include/imu_bno055.h`
- `firmware/src/radar_ld2420.cpp`
- `firmware/include/radar_ld2420.h`
- `firmware/src/lidar_witmotion.cpp`
- `firmware/include/lidar_witmotion.h`
- `firmware/src/display_oled.cpp`
- `firmware/include/display_oled.h`

**Estimated scope:** Large

### Task B7: UDP Communication Bridge

**Description:** Implement binary UDP protocol for communication with companion computer. TX: joint states + IMU at 50Hz. RX: target joint positions at 50Hz.

**Acceptance criteria:**
- [ ] UDP socket bound to port (configurable, default 8888)
- [ ] TX packet: `[joint0, joint1, joint2, joint3, roll, pitch, yaw]` (7 floats, 28 bytes)
- [ ] RX packet: `[target0, target1, target2, target3]` (4 floats, 16 bytes)
- [ ] Sequence numbers on packets to detect loss
- [ ] Watchdog petted on valid RX packet (100ms timeout)
- [ ] WiFi connection to companion computer AP or STA mode

**Verification:**
- [ ] Loopback test: TX packets received locally, contents correct
- [ ] Integration test: Companion computer receives 50Hz joint states
- [ ] Integration test: Commands from companion computer move joints
- [ ] Test: Packet loss >10% triggers safety stop
- [ ] Latency test: Round-trip <20ms

**Dependencies:** Task B6

**Files likely touched:**
- `firmware/src/udp_bridge.cpp`
- `firmware/include/udp_bridge.h`
- `firmware/include/protocol.h`
- `scripts/udp_test_client.py`

**Estimated scope:** Medium

### Checkpoint: Firmware Foundation Complete
- [ ] All joints move smoothly under PID control
- [ ] UDP communication working at 50Hz with <20ms latency
- [ ] All sensors publishing data
- [ ] Safety systems tested and functional
- [ ] Build passes: `pio run`
- [ ] Tests pass: `pio test`
- [ ] **HUMAN REVIEW REQUIRED** before proceeding to software

---

## Phase C: Software & Simulation

### Task C1: ROS2 Workspace Bootstrap

**Description:** Initialize ROS2 workspace with package structure. Set up build system, dependencies, and initial launch files.

**Acceptance criteria:**
- [ ] `ros2_ws/` initialized with `colcon` build
- [ ] Packages created: `clawdog_control`, `clawdog_description`, `clawdog_bringup`
- [ ] Package dependencies declared: `ros2_control`, `xacro`, `joint_state_publisher`
- [ ] Build succeeds: `colcon build --packages-select clawdog_*`
- [ ] Launch file skeleton: `ros2 launch clawdog_bringup robot.launch.py`

**Verification:**
- [ ] `colcon build` completes without errors
- [ ] `ros2 pkg list | grep clawdog` shows all packages
- [ ] Launch file starts without errors (minimal nodes)
- [ ] Git commit with conventional commit format

**Dependencies:** Checkpoint: Firmware Foundation Complete

**Files likely touched:**
- `ros2_ws/src/clawdog_control/package.xml`
- `ros2_ws/src/clawdog_description/package.xml`
- `ros2_ws/src/clawdog_bringup/package.xml`
- `ros2_ws/src/clawdog_bringup/launch/robot.launch.py`

**Estimated scope:** Small

### Task C2: URDF Robot Description

**Description:** Create URDF/Xacro model of ClawDog with all joints, links, and visual meshes.

**Acceptance criteria:**
- [ ] URDF with 4 active joints (hip, shoulder, elbow, wrist or equivalent)
- [ ] Link masses and inertias estimated from CAD or measured
- [ ] Visual meshes simplified (bounding boxes acceptable initially)
- [ ] Joint limits match calibrated ADC ranges
- [ ] Gazebo-compatible (if simulation desired)

**Verification:**
- [ ] `check_urdf` passes without errors
- [ ] RViz shows robot model with TF tree
- [ ] Moving joints in RViz matches physical joint movement
- [ ] Joint limits enforced in RViz

**Dependencies:** Task C1

**Files likely touched:**
- `ros2_ws/src/clawdog_description/urdf/clawdog.urdf.xacro`
- `ros2_ws/src/clawdog_description/urdf/materials.xacro`
- `ros2_ws/src/clawdog_description/meshes/` (placeholder meshes)

**Estimated scope:** Medium

### Task C3: ros2_control Hardware Interface

**Description:** Implement `hardware_interface::SystemInterface` to bridge ROS2 control with ESP32 UDP protocol.

**Acceptance criteria:**
- [ ] `ClawDogHardware` class implements `SystemInterface`
- [ ] `read()`: Receives UDP packet, updates joint states
- [ ] `write()`: Sends target positions via UDP
- [ ] Handles connection loss gracefully (stop joints)
- [ ] Configurable via ROS2 parameters (IP, port, rates)

**Verification:**
- [ ] `ros2_control` loads hardware interface without errors
- [ ] Joint states published to `/joint_states` at 50Hz
- [ ] Commands from `/joint_trajectory` sent to ESP32
- [ ] Disconnect test: Joints stop when UDP lost

**Dependencies:** Task C2, Task B7

**Files likely touched:**
- `ros2_ws/src/clawdog_control/src/clawdog_hardware.cpp`
- `ros2_ws/src/clawdog_control/include/clawdog_hardware.hpp`
- `ros2_ws/src/clawdog_control/clawdog_control.xml`

**Estimated scope:** Large

### Task C4: OpenClaw Integration

**Description:** Integrate OpenClaw quadruped gait generation framework. Configure gaits (stand, walk, trot) and publish trajectories.

**Acceptance criteria:**
- [ ] OpenClaw dependency added (git submodule or package)
- [ ] Gait generator node publishes `JointTrajectory` messages
- [ ] Configurable gaits: stand, walk, trot (later: bound, pace)
- [ ] Gait parameters: speed, step height, duty factor
- [ ] Smooth transitions between gaits

**Verification:**
- [ ] Simulation test: Gait generates smooth joint trajectories
- [ ] Trajectory points within joint limits
- [ ] Frequency: 50Hz trajectory updates
- [ ] No discontinuities during gait transitions
- [ ] Stand gait: All joints at neutral position

**Dependencies:** Task C3

**Files likely touched:**
- `ros2_ws/src/clawdog_control/src/gait_generator.cpp`
- `ros2_ws/src/clawdog_control/include/gait_generator.hpp`
- `ros2_ws/src/clawdog_control/config/gait_params.yaml`

**Estimated scope:** Large

### Task C5: Teleoperation Interface

**Description:** Create teleoperation node for manual control via gamepad or keyboard. Publish velocity commands and gait switches.

**Acceptance criteria:**
- [ ] `teleop_node` subscribes to `/cmd_vel` (Twist)
- [ ] Gamepad support ( Xbox/PS4 controller via `joy` package)
- [ ] Keyboard fallback (WASD + QE for gait)
- [ ] Deadband on analog sticks (±10%)
- [ ] Emergency stop button mapped ( Xbox: BACK, PS4: SHARE)

**Verification:**
- [ ] Gamepad input produces correct `/cmd_vel` output
- [ ] Keyboard input works without gamepad
- [ ] E-stop button triggers immediate halt
- [ ] Teleop launch file: `ros2 launch clawdog_bringup teleop.launch.py`

**Dependencies:** Task C4

**Files likely touched:**
- `ros2_ws/src/clawdog_control/src/teleop_node.cpp`
- `ros2_ws/src/clawdog_control/src/teleop_keyboard.cpp`
- `ros2_ws/src/clawdog_bringup/launch/teleop.launch.py`

**Estimated scope:** Medium

### Task C6: First On-Floor Steps

**Description:** Execute first walking tests on floor. Start with static gait, progress to dynamic trot. Robot tethered for safety.

**Acceptance criteria:**
- [ ] Robot stands stably (all feet on ground, balanced)
- [ ] Static gait: Lifts and places one foot at a time
- [ ] Dynamic trot: Diagonal pairs move together
- [ ] No oscillation or instability at rest
- [ ] Gait speed adjustable 0-0.5 m/s

**Verification:**
- [ ] Video: Robot stands for 30 seconds without falling
- [ ] Video: Robot walks 2 meters in straight line
- [ ] Video: Robot turns in place
- [ ] IMU data shows pitch/roll within ±5° during walk
- [ ] **CRITICAL:** Robot tethered or in harness for all tests

**Dependencies:** Task C5, Checkpoint: Firmware Foundation Complete

**Files likely touched:**
- `ros2_ws/src/clawdog_control/config/walking_params.yaml`
- Test logs and videos

**Estimated scope:** Large (integration test, not code)

### Checkpoint: Software Complete
- [ ] URDF accurate and visualized
- [ ] ros2_control bridge working end-to-end
- [ ] OpenClaw gaits generating trajectories
- [ ] Teleop working with gamepad and keyboard
- [ ] Robot walks stably in trot gait
- [ ] All ROS2 nodes documented

---

## Phase D: Autonomy

### Task D1: IMU-Based Balance Control

**Description:** Add IMU feedback for adaptive gait. Compensate for slopes and external disturbances using pitch/roll data.

**Acceptance criteria:**
- [ ] Body orientation estimated from BNO055 (sensor fusion)
- [ ] Gait height adjusted based on pitch (slope compensation)
- [ ] Foot placement adjusted based on roll (lateral balance)
- [ ] Balance PID: Keep body level within ±3°
- [ ] Recovery from light pushes

**Verification:**
- [ ] Test: Robot walks on 5° slope without drifting
- [ ] Test: Robot recovers from 10° push and returns to level
- [ ] IMU data logged during all tests
- [ ] No oscillation or resonance in balance loop

**Dependencies:** Checkpoint: Software Complete

**Files likely touched:**
- `ros2_ws/src/clawdog_control/src/balance_controller.cpp`
- `ros2_ws/src/clawdog_control/include/balance_controller.hpp`

**Estimated scope:** Large

### Task D2: SLAM with LiDAR

**Description:** Integrate WitMotion D6 LiDAR for mapping and localization. Use `slam_toolbox` or `gmapping`.

**Acceptance criteria:**
- [ ] LiDAR driver node publishes `sensor_msgs/LaserScan`
- [ ] SLAM node produces `nav_msgs/OccupancyGrid`
- [ ] Robot pose estimated in map frame
- [ ] Map saved to file and reloadable
- [ ] Loop closure working (if supported by SLAM package)

**Verification:**
- [ ] Test: Build map while teleoperating around room
- [ ] Test: Saved map loads correctly
- [ ] Test: Robot localizes in known map within 10cm
- [ ] Map quality: Walls distinct, no ghost obstacles

**Dependencies:** Task D1

**Files likely touched:**
- `ros2_ws/src/witmotion_driver/` (new package)
- `ros2_ws/src/clawdog_bringup/launch/slam.launch.py`
- `ros2_ws/src/clawdog_bringup/config/slam_params.yaml`

**Estimated scope:** Large

### Task D3: Navigation Stack (Nav2)

**Description:** Configure Nav2 for autonomous navigation. Global planner, local planner, costmaps, behavior tree.

**Acceptance criteria:**
- [ ] Nav2 launch file: `ros2 launch clawdog_bringup navigation.launch.py`
- [ ] Global planner: A* or Dijkstra on occupancy grid
- [ ] Local planner: DWB or TEB with kinematic constraints
- [ ] Costmap layers: static map, obstacles, inflation
- [ ] Behavior tree: NavigateToPose with recovery actions
- [ ] Action server: `/navigate_to_pose` accepts goals

**Verification:**
- [ ] Test: Robot navigates 5m to goal in known map
- [ ] Test: Robot avoids unexpected obstacle
- [ ] Test: Recovery behavior triggers when path blocked
- [ ] Test: Navigation stops on emergency stop
- [ ] Navigation metrics: success rate, time to goal

**Dependencies:** Task D2

**Files likely touched:**
- `ros2_ws/src/clawdog_bringup/launch/navigation.launch.py`
- `ros2_ws/src/clawdog_bringup/config/nav2_params.yaml`
- `ros2_ws/src/clawdog_bringup/behavior_trees/`

**Estimated scope:** Large

### Task D4: AI Integration (RosClaw)

**Description:** Integrate RosClaw AI framework for high-level behavior. Voice commands, object detection, decision making.

**Acceptance criteria:**
- [ ] RosClaw node subscribes to camera/audio topics
- [ ] Voice command parser: "walk", "stop", "turn left", "follow me"
- [ ] Object detection: Identify human, ball, stairs (optional)
- [ ] Behavior state machine: IDLE → WALKING → FOLLOWING → STOPPED
- [ ] AI decisions published as `/cmd_vel` or action goals

**Verification:**
- [ ] Test: Voice command "walk" starts forward gait
- [ ] Test: Voice command "stop" triggers halt
- [ ] Test: Robot follows human using radar + vision (optional)
- [ ] Latency: AI decision to action <500ms

**Dependencies:** Task D3

**Files likely touched:**
- `ros2_ws/src/clawdog_ai/` (new package)
- `ros2_ws/src/clawdog_ai/src/voice_command_node.cpp`
- `ros2_ws/src/clawdog_ai/src/behavior_tree.cpp`

**Estimated scope:** Large (optional MVP feature)

### Task D5: Safety and Reliability Framework

**Description:** Comprehensive safety testing and documentation. Validate all safety systems under various failure modes.

**Acceptance criteria:**
- [ ] Emergency stop tested: Robot halts within 100ms
- [ ] Human detection tested: LD2420 triggers stop at <1m
- [ ] Battery monitoring: Low battery warning at 6.5V, cutoff at 6.0V
- [ ] Joint limits: Software limits + hardware endstops (if any)
- [ ] Watchdog: All scenarios where watchdog should trigger, verified
- [ ] Fault injection: Test behavior when sensors fail
- [ ] Documentation: Safety procedures for all test scenarios

**Verification:**
- [ ] Safety test report with all scenarios and results
- [ ] Video evidence of emergency stop tests
- [ ] Video evidence of human detection tests
- [ ] Checklist: All safety features signed off by human operator
- [ ] **HUMAN REVIEW REQUIRED** before unsupervised operation

**Dependencies:** Task D4 (or Task D3 if AI skipped)

**Files likely touched:**
- `docs/safety/test_report.md` (new)
- `docs/safety/procedures.md` (new)
- `scripts/safety_test_suite.py`

**Estimated scope:** Large (mostly testing, not coding)

### Checkpoint: Project Complete
- [ ] Robot autonomously navigates to goals
- [ ] All safety systems tested and documented
- [ ] AI integration functional (if implemented)
- [ ] Documentation complete and accurate
- [ ] Code committed and pushed
- [ ] Demo video recorded

---

## Parallelization Opportunities

### Safe to Parallelize (Independent Work)

1. **Hardware A1 (PCB analysis)** and **Documentation (CLAUDE.md updates)** — No dependencies
2. **Hardware A2 (Power)** and **Hardware A3 (Motors)** — Can be designed simultaneously
3. **Firmware B2 (Motor)** and **Firmware B3 (ADC)** — Independent drivers
4. **Firmware B6 (Sensors)** — IMU, radar, LiDAR, OLED can be developed in parallel
5. **ROS2 C2 (URDF)** and **Firmware B5 (PID)** — URDF doesn't need working PID
6. **ROS2 C4 (OpenClaw)** and **ROS2 C5 (Teleop)** — Independent nodes

### Must Be Sequential (Dependencies)

1. Hardware assembly → Firmware (can't test firmware without hardware)
2. Motor driver → ADC → PID (need to read position before controlling it)
3. PID → UDP → ros2_control (need local control before remote control)
4. ros2_control → OpenClaw → Walking (need trajectory interface before gaits)
5. Walking → SLAM → Nav2 (need basic locomotion before navigation)
6. Nav2 → AI (need navigation before high-level behaviors)

### Needs Coordination (Shared Interfaces)

1. **UDP Protocol** — Firmware B7 and ROS2 C3 must agree on packet format
   - Define protocol in `firmware/include/protocol.h` and `ros2_ws/src/clawdog_control/include/protocol.hpp`
   - **Recommendation:** Define protocol document first, then implement both sides

2. **Joint Limits** — Firmware B4 and ROS2 C2 must use same limits
   - Calibrate and document in single source of truth: `docs/hardware/calibration.md`

3. **Gait Parameters** — OpenClaw C4 and Balance D1 share gait config
   - Use YAML parameter files loaded by both nodes

---

## Risks and Mitigations

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| ESP32-S3 GPIO conflicts (strapping pins) | High | Medium | Pinout validated against ESP32-S3 datasheet; GPIO 3 moved to GPIO 13 |
| Motor drivers overheat | High | Medium | Heatsinks, 80% PWM limit, thermal monitoring in firmware |
| WitMotion D6 protocol undocumented | Medium | High | Reverse engineer from SDK; contact WitMotion; implement parser incrementally |
| OpenClaw ROS2 integration incompatible | Medium | Medium | Test OpenClaw simulation early (Task C4); fork/modify if needed |
| IMU drift causes balance issues | Medium | Medium | BNO055 sensor fusion; magnetic calibration; fallback to gyro-only |
| UDP packet loss over WiFi | Medium | Medium | Sequence numbers; 100ms watchdog; graceful degradation |
| Weight exceeds 750g | Medium | Low | Weigh each component; use lighter battery if needed; optimize wiring |
| 2S LiPo runtime <30 min | Medium | Low | Measure actual current draw; reduce motor holding current; sleep modes |
| LD2420 false positives/negatives | Medium | Medium | Configurable detection threshold; fuse with camera (if available); manual override |
| PCB modification damages original | Low | Low | Document before modifying; desolder carefully; have spare toy on hand |

---

## Open Questions

1. **Is the original toy's motor gearbox suitable for PID position control?**
   - High gear ratio may cause backlash; test with encoder/potentiometer
   - May need to modify gearbox or accept position tolerance

2. **What is the WitMotion D6 communication protocol?**
   - Vendor SDK may be required
   - Plan: Contact WitMotion; if no response, reverse engineer from binary data

3. **What is the exact form factor for the chassis modifications?**
   - 3D printed mounts? Laser cut acrylic?
   - Need CAD model or physical measurements

4. **What ROS2 distribution? Humble or Jazzy?**
   - Humble (Ubuntu 22.04) has longer support
   - Jazzy (Ubuntu 24.04) is newer but may have compatibility issues
   - **Recommendation:** Humble for stability

5. **What companion computer? RPi5 or Jetson?**
   - RPi5: Cheaper, lower power, good for Nav2
   - Jetson: GPU for AI, more expensive, higher power
   - **Recommendation:** Start with RPi5, upgrade to Jetson for AI features

6. **How will the robot be charged?**
   - Balance charger with external connector?
   - Remove battery for charging?
   - Add BMS for in-robot charging?

7. **What is the fallback if OpenClaw doesn't work out?**
   - Custom gait generator (significant effort)
   - Simpler static gait (acceptable MVP)
   - **Mitigation:** Test OpenClaw simulation in Task C4 before committing

8. **How will calibration values be stored and loaded?**
   - Hardcoded in firmware config header?
   - EEPROM/flash storage on ESP32?
   - YAML file on companion computer?
   - **Recommendation:** Flash storage with factory defaults, override via ROS2 parameter

---

## Definition of Done

The ClawDog project is complete when:

1. **Hardware:** All components mounted, wired, and tested. Weight <750g. Power system safe.
2. **Firmware:** All joints controlled by PID. UDP communication at 50Hz. All sensors reading. Safety systems functional.
3. **ROS2:** URDF accurate. ros2_control bridge working. OpenClaw gaits generating trajectories. Teleop functional.
4. **Walking:** Robot walks stably in trot gait for 10+ meters. Can turn, stop, and start.
5. **SLAM:** Builds and saves maps. Localizes within known map.
6. **Navigation:** Autonomously navigates to goals with obstacle avoidance.
7. **AI:** Responds to voice commands (if implemented).
8. **Safety:** All safety features tested and documented. Emergency stop <100ms. Human detection active.
9. **Documentation:** All docs accurate and complete. ADRs updated if decisions changed.
10. **Demo:** Video of robot walking and navigating autonomously.

---

## Appendix: File Inventory

### Existing Documentation (Maintain These)

| File | Purpose | Last Updated |
|------|---------|--------------|
| `CLAUDE.md` | Agent context, safety rules, conventions | Now |
| `DESC.md` | Full project description | Now |
| `README.md` | User-facing overview | Original |
| `docs/ARCHITECTURE.md` | System architecture | Now |
| `docs/decisions/ADR-001` | UDP vs micro-ROS | Now |
| `docs/decisions/ADR-002` | ROS2 on companion computer | Now |
| `docs/decisions/ADR-003` | OpenClaw vs custom gaits | Now |
| `docs/hardware/pinout.md` | Pin assignments | Now |
| `docs/hardware/overview.md` | Component specs | Now |

### New Documentation (Create These)

| File | Purpose | Created By |
|------|---------|------------|
| `docs/PLAN.md` | This document | Now |
| `docs/hardware/original_pcb.md` | PCB tracing results | Task A1 |
| `docs/hardware/motor_specs.md` | Motor measurements | Task A1 |
| `docs/hardware/power.md` | Power system design | Task A2 |
| `docs/hardware/driver_board.md` | Motor driver assembly | Task A3 |
| `docs/hardware/wiring_harness.md` | Sensor wiring | Task A4 |
| `docs/hardware/calibration.md` | Calibration values | Task B3 |
| `docs/safety/test_report.md` | Safety test results | Task D5 |
| `docs/safety/procedures.md` | Safety procedures | Task D5 |

### Code (Create These)

| Directory | Purpose | First Created |
|-----------|---------|---------------|
| `firmware/` | ESP32 firmware | Task B1 |
| `ros2_ws/src/clawdog_control/` | ros2_control, gaits, teleop | Task C1 |
| `ros2_ws/src/clawdog_description/` | URDF, meshes | Task C1 |
| `ros2_ws/src/clawdog_bringup/` | Launch files, configs | Task C1 |
| `ros2_ws/src/witmotion_driver/` | LiDAR driver | Task D2 |
| `ros2_ws/src/clawdog_ai/` | AI behavior (optional) | Task D4 |
| `scripts/` | Test scripts, utilities | Task B7 |

---

*This plan is a living document. Update it as the project evolves, decisions change, and new information becomes available.*

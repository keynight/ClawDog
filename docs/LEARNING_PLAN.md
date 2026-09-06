# ClawDog Learning Plan (for a Linux Admin new to ROS/Embedded/Robotics)

## Why this plan exists

The project plan (`docs/PLAN.md`) assumes robotics skills. This document is the **learning companion**: it tells you *what* to learn, *why each topic exists*, *how deep you must go* (no math-heavy detours — usage-level depth is enough for ClawDog), and *when* to learn it so knowledge lands exactly when a project stage needs it.

**Good news up front:** as a Linux administrator you already hold most of the systems foundation:
shell, `systemd`, processes, package management, networking/UDP/IP/firewalls, git, and scripting (bash/python). Those map 1:1 onto robotics infrastructure (see the transfer map below). You only need to add **three new domains**, and two of them you can learn on a $30 lab before touching the robot.

---

## The three domains (and the minimum depth you need)

| Domain | Why ClawDog needs it | Minimum depth (no more) |
|--------|----------------------|-------------------------|
| **A. ROS2** | The "operating system" on the companion PC; every node, topic, launch file | Operate it confidently: nodes, topics, services, actions, params, `colcon`, `launch`, `rviz2`, TF, and *using* `slam_toolbox` + Nav2 configs. Not writing a new SLAM algorithm. |
| **B. Embedded C++ / ESP32** | You will read *and modify* the firmware: PWM, ADC, I2C, UART, FreeRTOS, UDP | Arduino-framework C++ on PlatformIO, pin-level I/O, tasks on 2 cores, watchdog patterns. Not advanced C++ templates or ESP-IDF internals. |
| **C. Robotics concepts** | PID (why motors "hold position"), quadruped gaits, coordinates/TF, mapping intuition | Intuitive understanding + parameter-level tuning and configuration. Not control-theory math or SLAM internals. |

Rule of thumb for every topic: **if you can explain it to another Linux admin in one analogy and demo it once, that's enough.**

---

## Transfer map: what you already know → robotics equivalent

| Linux/sysadmin skill | Robotics equivalent | It means… |
|----------------------|---------------------|-----------|
| Processes/daemons | **ROS2 nodes** | One program per concern, communicating over the network |
| Log streams / `journalctl -f` | **Topics** (`ros2 topic echo`) | Named pub/sub streams; anyone can subscribe |
| `ps`, `/proc` | `ros2 node list`, `ros2 topic info` | Introspection of the live system |
| systemd units / docker-compose | **`ros2 launch`** | Declaratively start a group of nodes with params |
| `apt`/package repos | **`colcon` + ROS packages + `rosdep`** | Build/install ROS workspaces |
| `/etc` config files | **ROS parameters + YAML** | Runtime config for nodes |
| DNS/mDNS/zeroconf discovery | **DDS discovery (multicast)** | Nodes find each other on the LAN — and you already know how to debug this with `tcpdump`/`tshark`! |
| Kernel device driver | **`ros2_control` hardware interface** | The glue that lets ROS talk to the ESP32 "device" |
| Hardware watchdog / systemd watchdog | **Firmware watchdog (100 ms)** | Kill motors if the "parent" goes silent |
| Cron jobs with progress/cancel | **Actions** | Long tasks (navigate to point) with feedback + cancel |
| GPIO/sysfs, `i2cdetect`, serial via `/dev/ttyUSB0` | Same concepts at MCU level | Pin I/O, I2C bus scan, UART — only smaller and real-time |
| NTP/time sync | **TF (coordinate transforms)** | Keeping every part's *position* consistent instead of its clock |

---

## Domain A — ROS2 curriculum (for the companion PC)

Do these as the **pre-learning track** in parallel with project Stage 0–3. Ubuntu 22.04 + **ROS2 Humble** (recommended in the project plan). If you don't have a second machine yet, a VM or **Docker** (`osrf/ros:humble-desktop`) works for everything until Stage 3.

| Module | Learn | Demo / proof of learning | Time |
|--------|-------|--------------------------|------|
| A1 | Core mental model: graph of nodes, topics, services, actions, params | Run `turtlesim`; drive the turtle with `ros2 topic pub` + `teleop_twist_keyboard` | 3–5 h |
| A2 | CLI fluency (your friend): `ros2 node/topic/service/action/param/interface`, `rqt_graph`, `ros2 bag` | Inspect a running turtlesim the way you'd inspect a server; record + replay a bag | 3 h |
| A3 | Workspaces & packages: `colcon build`, `package.xml`, `ament_cmake`/`ament_python`, `ros2 pkg create`, `rosdep` | Create a tiny talker/listener package pair; run with `ros2 run` | 4–6 h |
| A4 | Launch files & params: `launch.py`, YAML param files, namespaces/remapping | Write a launch file that starts your talker/listener with params | 3–4 h |
| A5 | TF2 & rviz2: frames, `static_transform_publisher`, visualizing | Publish 2–3 static frames; watch them in RViz; read a transform with `tf2_echo` | 3–4 h |
| A6 | URDF/Xacro: link/joint model → what `clawdog_description` is | Build the official "urdf tutorial" robot; view in RViz | 4–6 h |
| A7 | User-level SLAM + Nav2 *usage*: `slam_toolbox`, costmaps, `navigate_to_pose` | Run Nav2 + SLAM on the official **TB3 simulation** in Gazebo; drive it, map a room, send a navigation goal | 6–10 h |
| A8 | ros2_control concepts (defer until Stage 3) | Read how a hardware interface + controller manager works on the TB3 example | 3–4 h |
| A9 | **Optional lab — OmniSim** ([github.com/omnilink-tech/omnisim](https://github.com/omnilink-tech/omnisim)): an agent-native simulator — HTTP/JSON + MCP control, Newton physics (CPU default, **no GPU needed**), wgpu rendering, Apache-2.0, with a ROS 2 sidecar, URDF import, and shipped **quadruped** models/demos | Run the Go2 / OmniQuad legged demos and watch a quadruped walk, stumble and recover; import a URDF into a scene; if ambitious, drive a robot from the ROS 2 sidecar topics | 3–6 h |

**Primary resources (all free):** Official ROS2 Humble tutorials (docs.ros.org/en/humble/Tutorials.html) · Articulated Robotics (articulatedrobotics.xyz + YouTube) — best for beginners · Robotics Back-End (YouTube) · Nav2 docs (navigation.ros.org). If videos help you, start with Articulated Robotics' ROS2 series; it uses the same toy-robot flavor as ClawDog.

**OmniSim details (module A9):** open-source (Apache-2.0, same as ClawDog); the ROS 2 sidecar (`packages/omnisim-ros2`) needs **Humble or newer** (verified on Humble) and exposes `simulation_interfaces` plus the same style of topics the project uses (`joint_states`, `/odom`, `/cmd_vel`, `/scan`, `/imu/data`). It is deliberately agent-friendly — a coding agent can load a world, run it, and hot-reload over plain HTTP — which fits how this project is being developed. **Platform reality:** Linux is a *source build* on Ubuntu 22.04/24.04 (`scripts/install/linux_bootstrap.sh`, ~25–45 min of compile; on 22.04 it installs Python 3.12 via deadsnakes); Windows ships a prebuilt public-beta package; macOS is unsupported. It's a young project in public beta — expect rough edges, and use `python -m omnisim doctor` to verify an install actually has physics.

> **Two-simulator strategy.** Keep **A7 (Gazebo/TurtleBot3) mandatory**: it is the only path where ROS2 + SLAM + Nav2 work end-to-end on Humble today, and it matches the project stack exactly. Use **OmniSim as an extra lab**: it is the closest thing in pure software to ClawDog's actual problem — a *legged* quadruped — plus URDF-import practice and an agent-driven workflow. Honest caveats: OmniSim's own Nav2 bring-up is **Jazzy-only and very new**, its `ros2_control` `SystemInterface` is verified for **velocity-commanded wheeled bases** (Husky) and not yet a quadruped joint controller, MoveIt is out of reach, and **sim-to-real is unproven** — so treat it as learning/experimentation, never as a substitute for the suspended-hardware tests in the project plan.

---

## Domain B — ESP32 / embedded C++ curriculum

Lab order (~$30): ESP32-S3 dev board, breadboard + jumper wires, a potentiometer, a small DC motor + a cheap H-bridge module (or reuse the toy's leg later), an I2C sensor (e.g., MPU6050/BNO055-class), a 2S LiPo or bench supply **only for motor experiments**.

| Module | Learn | Demo / proof of learning | Time |
|--------|-------|--------------------------|------|
| B1 | PlatformIO + Arduino framework: project layout, `platformio.ini`, `pio run/upload/monitor`, serial prints | Blink an LED; print "hello" over serial; read a button | 3–4 h |
| B2 | C++ you actually need: classes, methods, pointers/refs, globals, `struct`, timers vs `delay()` | Refactor the blink into a small `Led` class | 3–5 h |
| B3 | Analog I/O: ADC (12-bit, moving average), **PWM** duty cycle, why potentiometer → angle | Read a pot's ADC with a 5–10 sample filter; fade an LED with PWM | 3–4 h |
| B4 | I2C & UART: bus scan (`Wire`), reading a sensor register; serial RX parsing | `i2c scan` finds your IMU; print parsed sensor values; parse an NMEA-like text stream | 4–6 h |
| B5 | FreeRTOS tasks + cores: `xTaskCreatePinnedToCore`, queues, task timing; Arduino `loop()` on one core | Two tasks: one blinks, one prints sensor data at a fixed rate | 4–5 h |
| B6 | Networking: WiFi STA/AP, **UDP** send/recv, packet format + sequence numbers | Send 50 Hz UDP "joint frames" to a python receiver on your PC; verify loss with sequence numbers | 4–5 h |
| B7 | Safety patterns (do NOT skip): watchdog semantics, PWM=0 on boot, duty clamps, fail-safe defaults | Code a "no heartbeat 100 ms → motors off" demo with a fake motor output | 3 h |

**Resources:** PlatformIO docs (docs.platformio.org) · Random Nerd Tutorials ESP32 (randomnerdtutorials.com) — excellent step-by-step · Arduino language reference · ESP32-S3 datasheet/pinout (skim). C++ refresher: learncpp.com (first ~6 chapters only).

---

## Domain C — robotics concepts (usage level)

| Module | Learn | Why (ClawDog connection) | Time |
|--------|-------|--------------------------|------|
| C1 | **PID intuition**: P=how hard to push toward target, I=push harder if error persists, D=brake against overshoot; "tune by feel" — Kp up until oscillation, then Ki, then Kd | Every firmware joint controller (Stage 2) is a PID you must tune with real motors | 3–4 h |
| C2 | Quadruped anatomy: 1 DOF per leg here, joints read by potentiometer (position feedback), H-bridge = direction+power | Understands why the toy needs PID *at all* (DC motors don't hold position) | 2 h |
| C3 | Gaits conceptually: stand → static walk (3 legs down) → trot (diagonal pairs); what OpenClaw provides (trajectories, not magic) | Stage 4 milestones; you'll judge gait quality by eye | 2–3 h |
| C4 | Coordinates & frames: `base_link`, odometry, why transforms must match physical reality | Debugging RViz "robot in wrong place" = a sysadmin-style fault hunt | 2 h |
| C5 | Mapping & localization *usage*: occupancy grid, LiDAR spins → `/scan` → map; localization = "am I on the map?"; why you teleop-build the map first | Stage 5–6; you configure YAML, you don't write SLAM | 3 h |
| C6 | Safety architecture of an autonomous system: E-stop layers (hardware → firmware watchdog → software cancel), fault injection mindset | Stage 6 sign-off; you already think in failure modes — this is the same discipline | 2 h |

**Resources:** PID — Brian Douglas' PID control YouTube series or Brett Beauregard's "Improving the Beginner's PID" (brettbeauregard.com/blog); gaits — any "quadruped gait explained" primer + OpenClaw docs when you reach Stage 4; Nav2/SLAM docs for C5.

---

## Just-in-time matrix: learn X while doing Stage Y

Learn each module in the *same window* the project stage needs it — this is what makes the plan stick.

| Project stage | Learning to do in parallel | You'll be able to… |
|---------------|----------------------------|--------------------|
| **Stage 0** (now, docs/contracts) | B1, B2, C1; start A1–A2 | Read firmware headers; build a PlatformIO project; already drive a turtle |
| **Stage 1** (single leg) | B3, B4(part), C2, B7(safety-first) | Wire/verify a pot + PWM; run the test-leg sketch with *understanding* |
| **Stage 2** (firmware) | B4–B6, C1 (tune the real PID), A1–A3 | Modify every firmware module; debug UDP like a network engineer |
| **Stage 3** (ROS2 ws) | A3–A6, A8; C4 | Build the workspace, model the robot, understand the hardware-interface glue |
| **Stage 4** (gaits/teleop) | A5(revisit), C3; **A9 (OmniSim quadrupeds, optional)**; OpenClaw docs | Watch real quadruped gaits in sim *before* judging your own; diagnose "why is it limping" = trace topics/frames |
| **Stage 5** (SLAM/sensors) | A7 (TB3 simulation!), C5 | Run mapping/localization; configure the LiDAR driver |
| **Stage 6** (Nav2/sign-off) | A7 deepen; C6; safety docs | Configure Nav2 params; run fault-injection tests like an incident drill |

> The TB3 simulation in A7 is the single highest-value hour-per-hour investment: it teaches Stage 5+6 *concepts* (SLAM, Nav2, costmaps) in pure software, weeks before the robot can walk.
>
> Runner-up (optional): **OmniSim's quadruped demos** (A9) — the only software in this plan where you watch a *legged* robot walk, stumble and recover, which is exactly the problem ClawDog faces at Stage 4.

---

## Suggested schedule (part-time)

Two pace options; the column order is what matters more than the calendar.

| Week | Relaxed (~5–7 h/wk) | Focused (~10 h/wk) |
|------|----------------------|---------------------|
| 1–2 | B1, B2, start A1 | B1–B3, A1–A2 |
| 3–4 | C1, B3, A2 | B4–B5, A3 |
| 5–6 | B4, A3, C2 | B6–B7, A4–A5 (now ready to *do* Stage 1–2 work) |
| 7–8 | A4–A5, B5 | A6, C3, TB3 install |
| 9–10 | A6, B6, C3 | A7 (TB3 SLAM/Nav2 lab) |
| 11–12 | A7 (TB3 lab), B7, start A9 (OmniSim) | A7 polish, C4–C5, OpenClaw intro (+A9 OmniSim optional) |
| 13–14 | C4–C6, OpenClaw intro | Buffer/overrun → back into project stages |

Realistic target: **~10–12 weeks of part-time study** puts you at the *operating* level every Stage 1–4 task needs; Stages 5–6 remain configuration-level learning as you reach them.

---

## Learning hardware/software shopping list (optional but recommended)

| Item | Approx. cost | Used for |
|------|--------------|----------|
| ESP32-S3 devkit (same family as the project MCU) | $8–15 | All of Domain B |
| Breadboard + jumper wire set | $5–10 | Domain B |
| Potentiometer (10 kΩ) + push buttons + LEDs | $3–5 | B3 demos |
| Small DC motor + L9110S/L298N H-bridge module | $5–8 | PWM + PID feel (or reuse the toy's spare leg later) |
| I2C sensor breakout (MPU6050 IMU is cheapest) | $3–6 | B4 I2C/UART parsing |
| Multimeter | $15–30 (if you lack one) | Every hardware stage — you will thank yourself |
| Ubuntu 22.04 machine **or** Docker on your current box | $0 | ROS2 Humble (Docker: `osrf/ros:humble-desktop`) until Stage 3 |
| *(optional)* OmniSim simulator (module A9) | $0 | Source-build on the same Ubuntu 22.04/24.04 box (~25–45 min); CPU physics is fine — no GPU needed |

Total lab: **≈$40–70**, versus a robotics course that costs far more and teaches less of what ClawDog specifically needs.

---

## ROS2 quick cheat-sheet (sysadmin-flavored)

```bash
source /opt/ros/humble/setup.bash          # "enter the environment"
ros2 node list                             # ps aux for ROS
ros2 topic list                            # available log streams
ros2 topic echo /joint_states              # tail -f a stream
ros2 topic hz /joint_states                # measure rate (like iostat)
ros2 topic info /joint_states -v           # who publishes/subscribes
ros2 param list /node                      # /etc for a node
ros2 interface show sensor_msgs/msg/LaserScan   # read the packet format
ros2 launch clawdog_bringup robot.launch.py     # systemd start a stack
ros2 run teleop_twist_keyboard teleop_twist_keyboard
colcon build --packages-select clawdog_*   # build only my packages
ros2 bag record -a                         # tcpdump for topics
rviz2                                      # the "dashboard" for robot state
```

**Debugging mindset:** when a node doesn't connect, think discovery/firewall/multicast — you already debug this daily. `tcpdump -i any udp port 8888` works on ROS DDS and on the ESP32 UDP bridge alike.

---

## Glossary (minimal, admin-flavored)

| Term | Plain meaning |
|------|---------------|
| Node | A process that talks over ROS |
| Topic | Named pub/sub stream (one-to-many, no reply) |
| Service | Request/reply (like an RPC) |
| Action | Long-running task with feedback and cancel (like a monitored job) |
| Parameter | Per-node runtime config (like `/etc` entries) |
| Launch file | Starts a whole stack with config (like a systemd target/compose file) |
| DDS | The middleware doing discovery + transport over the LAN |
| URDF/Xacro | XML describing the robot's links/joints (a "device tree" for robots) |
| TF | Coordinate transforms between parts of the robot |
| Hardware interface | ROS driver that talks to real hardware (like a kernel module) |
| Controller manager | Loads/unloads controllers (like managing services) |
| PWM | Turning power on/off fast; duty % ≈ brightness/speed |
| ADC | Converts a voltage (e.g., pot position) to a number |
| I2C / UART | Serial buses for sensors; UART = plain serial like a console cable |
| PID | Feedback controller: corrects error to hold a target position |
| Gait | The repeating footfall pattern of a walk |
| Watchdog | "Parent went silent → shut motors down" (systemd watchdog idea) |
| Occupancy grid / map | 2D grid of free/occupied cells a robot builds with SLAM |
| `/scan` | One LiDAR spin, as a LaserScan message of ranges/angles |
| E-stop | Emergency stop — hardware cut, independent of software |

---

## Definition of "you're ready"

You are ready to start project **Stage 1** when you can:
1. Explain to a colleague why a DC-motor leg needs PID but a servo doesn't.
2. Build, upload, and debug a PlatformIO project that reads a potentiometer and dims an LED via PWM.
3. Run two ROS2 nodes and prove they talk (`rqt_graph` + `ros2 topic echo`).
4. Parse a binary UDP frame in python from your ESP32 and detect packet loss by sequence number.

You are ready for **Stage 3+** when you can additionally:
5. Build a small colcon workspace and launch a multi-node stack from a launch file.
6. Run the TB3 simulation, map a room with SLAM, and send a Nav2 goal.

**Final reminder:** this project is 80% systems engineering (your home turf) and 20% new robotics content — and the 20% is exactly what this plan covers. Learn just-in-time, never more deeply than the stage requires, and keep the robot suspended until the plan says otherwise.

---

*Companion to `docs/PLAN.md`. Update this file when you finish modules (tick them off) or discover better resources.*

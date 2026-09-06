# ClawDog Master Implementation Plan (Stage-Gated)

## Overview

ClawDog converts a cheap toy robot dog into an autonomous quadruped: an **ESP32-S3** handles real-time joint control and sensor reading, while a companion computer (RPi5/Jetson) runs **ROS2** for gait generation (OpenClaw), SLAM, and autonomous navigation (Nav2). The two halves talk over a simple binary **UDP** link (ADR-001), not micro-ROS.

This plan replaces the earlier week-based phase list with a **stage-gate roadmap**. Work is organized into **Stage 0 → Stage 6**; each stage ends with an explicit **Gate** — a checklist that must fully pass before the next stage starts. Stages 1, 2, 4, and 6 contain **mandatory human review gates** because they involve powered hardware and moving motors.

**Key success criteria:**
- Robot walks stably in trot gait (stand → walk → trot progression)
- Autonomous navigation with obstacle avoidance (Nav2)
- Safety systems functional and *demonstrated*: hardware E-stop, LD2420 human detection, 100 ms watchdog, joint limits, low-battery cutoff
- 30–40 minute runtime on 2S LiPo; total weight under 750 g
- All safety testing done with the robot **suspended** until on-floor work is explicitly gated

---

## Current State (Baseline — as of this rewrite)

| Area | Status | Notes |
|------|--------|-------|
| Documentation | Substantial | `CLAUDE.md`, `DESC.md`, `README.md`, `docs/ARCHITECTURE.md`, 3 ADRs, `docs/hardware/{pinout,overview}.md` exist |
| `docs/PLAN.md` | This file | Former phase list replaced by stage gates |
| Firmware | Prototype scaffold, **untracked** | `firmware/` contains a **micro-ROS** prototype (`main.cpp`) that **contradicts ADR-001** (UDP binary), plus a sound single-leg test rig doc (`test-leg/README.md`) and `motor_control.{h,cpp}` |
| ROS2 workspace | Not created | `ros2_ws/` absent |
| OpenClaw | Not integrated | No `openclaw/` tree |
| Scripts / tests | Not created | Top-level `scripts/`, `tests/` absent |
| Calibration | All TBD | ADC min/max per joint unmeasured (blocks joint-angle mapping & URDF limits) |
| Hardware | Toy teardown photos only | `hardware/toy/` images; PCB surgery not started |

**Known contradictions to resolve in Stage 0** (single source of truth must win):
- MCU part number: `N16R8` (README, ARCHITECTURE, code header) vs `H16R8` (CLAUDE.md, DESC.md).
- License: `LICENSE` file + README say **Apache-2.0**; DESC.md §8 says **MIT**. → Apache-2.0 (existing LICENSE file) unless an owner decision changes it (record as ADR).
- PWM safety cap: test code clamps at 30%; CLAUDE.md/README mandate **80% sustained max**. → 80% is the driver-protection limit; 30% remains a *default startup* value for bring-up.
- README contains an **unresolved Git merge-conflict block** (`<<<<<<< HEAD … >>>>>>>`) that must be cleaned.
- `firmware/` is untracked — nothing is committed; the micro-ROS prototype must be marked historical or moved, then real work committed.

---

## The Stage-Gate Model

- A **Stage** is a coherent slice of work with a defined goal.
- A **Gate** is a hard checklist at the end of a stage. No task in the next stage starts until every gate box is checked.
- **Human review gates** are points where a person must physically confirm wiring/power/safety before code is run on hardware — an agent must not auto-advance past them.
- Test-first ordering is mandatory: **suspended** (no ground contact) before floor, **one leg** before four, **local control** before remote, **teleop** before autonomy.
- This document is a living artifact. Update task tables when scope changes; record stage results (date, who, evidence links) in the Gate checklist notes.

---

# Stage 0 — Foundation & Contracts

**Goal:** Turn the current documentation-heavy repo into a clean, internally consistent baseline with frozen interface contracts, so every later stage builds against one source of truth.

**Duration:** ~1 week (or one focused working session).

## 0.1 Repo hygiene & structure
- Commit or archive all untracked content; remove `.omc/` sessions or add to `.gitignore`.
- Resolve the README merge-conflict block; fix internal doc links (README references `docs/PLAN.md` "25 tasks / 13 weeks" text — update wording).
- Create the missing top-level directories with placeholder READMEs: `ros2_ws/src/`, `openclaw/`, `scripts/`, `tests/`.
- Standardize `.gitignore` (PlatformIO build, `.pio/`, colcon `build|install|log`, `.omc/`).

## 0.2 Resolve documentation conflicts (single source of truth)
Record every resolution as a short ADR or an amendment note so history is preserved:
1. MCU part (N16R8 vs H16R8) — pick actual purchased silicon, update all docs + `platformio.ini` board/flash/PSRAM flags.
2. License (Apache-2.0 vs MIT) — default: Apache-2.0 per existing `LICENSE`.
3. PWM cap policy — define: absolute hardware clamp (80%) vs startup default (30%) vs bring-up cap, and where each lives in code (`config.h`).
4. Firmware transport — reaffirm **UDP binary (ADR-001)**; mark the micro-ROS `main.cpp` prototype as historical (move to `firmware/experiments/microros_legacy/` or prefix with `LEGACY_` + header note) so it cannot be mistaken for current design.
5. UART2 LiDAR baud rate: README table says 921600, DESC.md says 921600 in §3.1 but 230400 in PLAN task A4 wiring — verify against WitMotion D6 docs and record in `docs/hardware/overview.md`.

## 0.3 Freeze interface contracts (each becomes a single source of truth)
- **UDP protocol v1** → new `docs/protocol.md`:
  - TX (ESP32→PC) `[joint0..3, roll, pitch, yaw]` — 7 × float32, plus header/sequence; 50 Hz
  - RX (PC→ESP32) `[target0..3]` — 4 × float32 + header/sequence; 50 Hz
  - Wire endianness, magic/version byte, sequence numbering, port (default 8888)
  - Firmware header `firmware/include/protocol.h` and ROS2 mirror header must both derive from this doc.
- **Topic & frame naming** (from ARCHITECTURE.md): `/joint_states`, `/joint_trajectory`, `/scan`, `/cmd_vel`, `/odom`, `/imu/data`, `/human_presence`; TF frames `base_link` etc.
- **Pinout**: `docs/hardware/pinout.md` is authoritative; strapping-pin warnings (GPIO 0/3/46, PSRAM-reserved 26–32) enforced by review.
- **Power tree & battery budget**: 2S LiPo → SA8301 direct; MP1584 5 V/3 A (LiDAR); AMS1117 3.3 V (ESP32, BNO055, OLED, 3.3 V LD2420 variant). Common ground mandatory.

## 0.4 Calibration & toolchain foundations
- Define calibration procedure (sweep joint by hand, record raw ADC at physical extremes, store per-robot in flash with factory defaults + ROS2 override) and create `docs/hardware/calibration.md` template.
- Verify toolchains and document versions: PlatformIO/ESP-IDF env build of the (cleaned) firmware scaffold; ROS2 distro chosen (recommend **Humble** on Ubuntu 22.04 for stability; revisit if new hardware demands Jazzy); `colcon` + `rosdep` sanity run with an empty workspace.

## Gate 0 — Baseline Ready
- [ ] Repo clean: no untracked clutter, merge markers gone, `.gitignore` in place, all real work committed (conventional commits).
- [ ] All conflicts in §0.2 resolved and recorded (docs consistent; no two docs contradict).
- [ ] `docs/protocol.md` frozen; firmware + ROS2 headers mirror it.
- [ ] `docs/hardware/calibration.md` template exists.
- [ ] PlatformIO builds the scaffold firmware; ROS2 distro chosen and recorded.
- **Human note:** confirm part numbers/purchase reality before §0.2 resolutions are locked.

---

# Stage 1 — Single-Leg Bring-Up (Hardware Proof)

**Goal:** Prove the core hardware loop — ESP32 → SA8301 → one motor, potentiometer → ADC — with safe, slow, suspended movement, and capture the first real calibration data.

**Duration:** 1–2 weeks.

## 1.1 Wiring (one leg)
- Per `firmware/test-leg/README.md`: connect one SA8301's IN1/IN2 to ESP32 GPIO (PWM/DIR), potentiometer wiper to an ADC pin, **common ground first**; 3.3 V to pot VCC only if the mainboard does not power it.
- Verify with multimeter *before* power: pot sweep reads ~0–3.3 V (never 0–7 V into the ADC), continuity on all joints.
- Motor power from bench supply/2S LiPo; hardware E-stop button placed in the motor power line.

## 1.2 Firmware (test-rig scope)
- `test-leg` sketch: 5-second pre-move pause, slow sweep, serial commands `s` (stop) / `r` (resume).
- ADC: 12-bit reads with moving-average filter (5–10 samples); log raw ADC during a hand sweep to find per-joint min/max.
- PWM: 1 kHz, duty clamped to bring-up cap (start ≤30%), motor outputs forced 0 on boot.
- First E-stop check: physical button removes motor power while ESP32 stays alive.

## 1.3 Calibration capture
- Record ADC min/max and observed motion direction for joint 0 into `docs/hardware/calibration.md`; note gearbox dead zones / backlash observations (feeds Open Questions).

## Gate 1 — Single Leg Proven
- [ ] Leg sweeps smoothly in both directions under serial command; direction reversible.
- [ ] ADC tracks position smoothly; raw min/max captured and stored.
- [ ] `s` stops instantly; E-stop button cuts motor power independently of software.
- [ ] No overheating after 5 min continuous slow sweep.
- [ ] Test log/video stored under `docs/logs/` (new) or `hardware/`.
- **HUMAN REVIEW GATE** — person confirms wiring quality and safety before scaling to four legs.

---

# Stage 2 — Full-Robot Firmware: 4 Joints, Sensors, UDP Bridge

**Goal:** Production firmware per ADR-001 (binary UDP — **not** micro-ROS): 4-joint PID position control at 200 Hz on core 1, sensor interfaces, safety subsystem, and a 50 Hz UDP bridge to the companion PC. All motor testing suspended.

**Duration:** 3–4 weeks.

## 2.1 Code architecture (rewrite the legacy prototype)
- `config.h` — pins, limits, tuning defaults, WiFi, UDP endpoints (single source derived from Gate 0 docs).
- Core 0: WiFi + UDP comms, heartbeat. Core 1: control ISR/task — ADC, PID, PWM at 200 Hz.
- Modules (mirroring ARCHITECTURE.md "Key Files"): `motor_driver` (SA8301 PWM+DIR, duty clamp), `joint_sensor` (ADC filter + calibrated angle map), `pid_controller` (per-joint, anti-windup, rate-limited output), `joint_controller` (coordinator), `safety`, `watchdog`, `udp_bridge`, `protocol`.
- Retire/relocate the micro-ROS `main.cpp` (done in Stage 0); new `main.cpp` implements the safety state machine.

## 2.2 Safety subsystem (build before any multi-joint motion)
- Safety state machine: `INIT → ACTIVE → E_STOP → RECOVERY`; log every transition with timestamp.
- Watchdog: ROS2/UDP heartbeat > 100 ms missing → all PWM = 0, sit pose.
- Joint limits: every motion command clamped to calibrated ADC min/max; violations logged.
- Low battery: warn at ~6.5 V, force sit pose < 6.0 V (2S).
- 80% absolute duty clamp enforced in the driver layer.
- Unit tests for each clamp/transition (see 2.5).

## 2.3 Sensors & peripherals
- BNO055 IMU on I2C (4.7 kΩ pull-ups): orientation (quaternion/Euler) at ~100 Hz + calibration status.
- SSD1306 OLED (I2C, 0x3C): battery, gait mode, error/state, ~10 Hz refresh.
- LD2420 mmWave (UART1, 115200): human presence + distance; presence in the *unsafe proximity zone* during autonomous operation → E-stop trigger.
- WitMotion D6 LiDAR (UART2): parse vendor protocol incrementally (see Open Question Q2); forward frames for the ROS2 driver.
- All four buses verified working simultaneously (no I2C/UART contention).

## 2.4 UDP bridge
- Binary protocol v1 exactly per `docs/protocol.md`; TX joint states + IMU attitude at 50 Hz; RX target positions at 50 Hz.
- Sequence numbers; >10% loss or stale RX → safety stop. Round-trip latency target <20 ms.
- Companion-side loopback test client in `scripts/udp_test_client.py`.

## 2.5 Verification
- `pio run` clean; `pio test` covers: duty clamping, joint-limit clamping, PID step response (overshoot <20%, settle <500 ms), watchdog timeout, E-stop latency (<50 ms to PWM=0), battery-cutoff transition.
- Suspended manual tests: each joint holds position under gentle push; all four hold simultaneously; gains start conservative (Kp≈1, Ki=0, Kd=0) and tune one joint at a time.

## Gate 2 — Firmware Foundation Complete
- [ ] All 4 joints move smoothly under PID (suspended), limits enforced.
- [ ] Watchdog, E-stop, battery cutoff, human-presence stop all demonstrated + logged.
- [ ] UDP 50 Hz end-to-end with companion PC; loopback + loss tests pass; <20 ms RTT.
- [ ] IMU, OLED, radar, LiDAR streaming; no bus conflicts.
- [ ] `pio run` + `pio test` green; firmware committed.
- **HUMAN REVIEW GATE** — before any floor motion is ever attempted.

---

# Stage 3 — ROS2 Workspace, URDF, Hardware Interface

**Goal:** Stand up the companion-computer half: ROS2 workspace, accurate robot model, and a `ros2_control` hardware interface that speaks the frozen UDP protocol to the ESP32.

**Duration:** 2–3 weeks (partially parallel with Stage 2 — see Parallelization).

## 3.1 Workspace bootstrap
- `ros2_ws/` with `colcon`; packages: `clawdog_control`, `clawdog_description`, `clawdog_bringup`.
- Dependencies declared (`ros2_control`, `xacro`, `joint_state_publisher`, `controller_manager`); ROS2 distro per Gate 0.
- Skeleton launch `ros2 launch clawdog_bringup robot.launch.py` starts minimal nodes.

## 3.2 URDF / Xacro model
- 4 actuated joints (one per leg DOF) with names `joint_0..3` matching firmware; links/masses estimated; visual meshes may start as boxes.
- Joint limits sourced from calibration doc; TF tree (`base_link` → … ) per ARCHITECTURE.
- `check_urdf` clean; RViz renders model; moving joints in RViz matches physical motion direction (verify with the real leg suspended).

## 3.3 ros2_control hardware interface
- `ClawDogHardware` implementing `SystemInterface`: `read()` parses ESP32 TX UDP frames → `/joint_states`; `write()` sends RX frames; connection-loss handling stops joints.
- Configurable via parameters (IP, port, rates); exposes `/joint_trajectory` command interface.
- End-to-end test: ROS2 sends target → ESP32 moves joints (suspended) → joint states stream back at 50 Hz.

## Gate 3 — Bridge Works End-to-End
- [ ] `colcon build --packages-select clawdog_*` clean; packages listed by `ros2 pkg list`.
- [ ] URDF passes `check_urdf`; RViz/TF correct; joint directions match hardware.
- [ ] Hardware interface loads; joints commanded from ROS2 (suspended); disconnect → joints stop.
- [ ] Live `/joint_states` at 50 Hz verified with `ros2 topic hz`.

---

# Stage 4 — Locomotion: OpenClaw Gaits, Teleop, First Steps

**Goal:** From local joint control to walking: OpenClaw gait generation, teleop input, and the first on-floor motion (tethered).

**Duration:** 3–4 weeks.

## 4.1 OpenClaw integration
- Add OpenClaw as git submodule/package under `openclaw/`; confirm ROS2 compatibility early (fork/modify if the published integration is stale — risk R5).
- Gait generator node publishes `/joint_trajectory` at 50 Hz: stand, walk, trot (bound/pace later); parameters speed, step height, duty factor; smooth gait transitions; all trajectory points within joint limits.

## 4.2 Teleoperation
- Teleop node: gamepad (Xbox/PS via `joy`) + keyboard fallback (WASD + gait switches); deadband ±10%; E-stop button mapped (Xbox BACK / PS4 SHARE).
- Launch: `ros2 launch clawdog_bringup teleop.launch.py`.
- Teleop drives Nav2-style `/cmd_vel`; gait command maps velocity → trajectory (see Open Questions Q9).

## 4.3 On-floor walking (tether mandatory)
- Progression: stand (30 s, no oscillation) → static gait (one foot at a time) → trot (diagonal pairs). Speed 0–0.5 m/s adjustable.
- Record video at each milestone: stand 30 s; walk 2 m straight; turn in place; IMU pitch/roll within ±5° during walk.
- All sessions tethered or in a harness; E-stop within arm's reach; no autonomous operation.

## Gate 4 — First Stable Walking
- [ ] Stand stable 30 s; static gait lifts/places feet cleanly.
- [ ] Trot walks 2 m straight + turns in place; gait transitions smooth.
- [ ] Teleop works with gamepad and keyboard; E-stop binding halts immediately.
- [ ] Videos + logs stored; gait/balance observations recorded.
- **HUMAN REVIEW GATE** — gait quality, timing, and safety reviewed before autonomy work.

---

# Stage 5 — Sensing & State: IMU Balance, LiDAR Driver, SLAM, Radar Safety

**Goal:** Give the robot situational awareness: IMU-based tilt compensation, LiDAR → `/scan` → map, and human-presence safety wired into the control loop.

**Duration:** 2–3 weeks.

## 5.1 IMU balance feedback
- Body attitude from BNO055 (fused); slope compensation adjusts gait height from pitch; lateral foot placement from roll; balance PID keeps body within ±3°; recovery from light pushes.
- Tests: 5° slope walk without drift; recovery from a gentle 10° push; log IMU throughout; no resonance/oscillation.

## 5.2 LiDAR driver & SLAM
- `witmotion_driver` ROS2 package parses ESP32-forwarded D6 frames → `sensor_msgs/LaserScan` on `/scan`.
- `slam_toolbox` (or `gmapping`) mapping while teleoperating; save map; reload and localize (within ~10 cm); inspect map quality (no ghost obstacles).

## 5.3 Radar safety integration
- `human_presence` topic from LD2420 data; safety node: presence inside unsafe zone during **autonomous** operation → cancel goals + trigger E-stop; log event; configurable threshold to tame false positives (risk R9).

## Gate 5 — Situational Awareness
- [ ] Slope/push-balance tests pass with logs.
- [ ] Map built while teleoperating, saved, reloaded; localization within ~10 cm.
- [ ] Radar stop fires during simulated autonomous run; threshold tunable; no nuisance trips during teleop.
- [ ] `/scan` at expected rate; TF + laser frame consistent.

---

# Stage 6 — Autonomy & Sign-Off

**Goal:** Autonomous navigation with Nav2, optional AI integration (RosClaw), full safety validation, documentation completion, and the final demo.

**Duration:** 2–3 weeks.

## 6.1 Nav2 autonomy
- Navigation launch (`navigation.launch.py`): global planner on occupancy grid, local planner (DWB/TEB) with quadruped kinematic constraints, layered costmaps, behavior-tree recovery.
- `/navigate_to_pose` action server; tests: navigate 5 m in known map; avoid unexpected obstacle; recovery on blocked path; **navigation stops on E-stop**.

## 6.2 Optional — AI integration (RosClaw) — *explicitly optional for MVP*
- Bridge between OpenClaw/ROS2 and AI agents (messaging apps or direct LLM); capability discovery, safety pre-checks (battery, joint limits, no collision), audit logging; voice/text command parsing ("walk", "stop", "follow me").
- Only proceed if Stage 6.1 is stable; otherwise defer and mark as post-MVP.

## 6.3 Safety validation & documentation (mandatory)
- Fault-injection matrix: sensor failure, UDP loss, battery sag, radar false trigger, E-stop at speed, watchdog scenarios — each with expected vs observed behavior.
- Produce `docs/safety/test_report.md` and `docs/safety/procedures.md`; video evidence for E-stop and human-detection tests.
- Update all docs/ADRs; consolidate calibration values; record remaining gotchas in CLAUDE.md.

## Gate 6 / Project Done — Definition of Done
- [ ] Autonomously navigates to goals with obstacle avoidance (5 m test passes; recovery works; E-stop halts Nav2).
- [ ] Trots 10+ m; turns/stops/starts on command.
- [ ] All safety features tested with evidence; emergency stop <100 ms; human detection active.
- [ ] (Optional) RosClaw responds to voice/text commands with safety pre-checks.
- [ ] Docs accurate; ADRs current; code committed/pushed; demo video recorded.
- **HUMAN REVIEW GATE** — final sign-off before any unsupervised operation.

---

## Parallelization Opportunities

### Safe to parallelize (independent work)
1. Stage 0 doc/contract cleanup ‖ Stage 1 wiring prep (bench power, E-stop).
2. Within Stage 2: `motor_driver`/`joint_sensor`/`pid_controller` (independent modules); IMU, radar, LiDAR, OLED drivers (independent buses).
3. Stage 3 ROS2/URDF work can start alongside Stage 2 (URDF does not need finished PID).
4. Stage 5 LiDAR driver + SLAM scaffolding can be prototyped on the companion PC with recorded D6 data before the robot walks.

### Must be sequential (dependencies)
1. Hardware wiring (Stage 1) → 4-joint firmware (Stage 2).
2. Local PID control → UDP bridge → ros2_control (local before remote).
3. Hardware interface (Stage 3) → OpenClaw gaits → floor walking (Stage 4).
4. Walking → SLAM → Nav2 (locomotion before mapping before planning).
5. Radar safety wiring precedes autonomous-mode operation (Stage 5 → 6).
6. Nav2 → optional AI layer.

### Needs coordination (shared interfaces — define first, implement both sides)
| Interface | Sides | Action |
|-----------|-------|--------|
| UDP protocol | Firmware (2.x) ↔ `clawdog_control` (3.x) | Freeze in `docs/protocol.md` (Stage 0); headers generated/mirrored |
| Joint limits | Firmware safety ↔ URDF ↔ gait params | Single source: `docs/hardware/calibration.md` |
| Topic/TF names | All ROS2 nodes + firmware | Single source: ARCHITECTURE.md §topic table (Stage 0) |
| Gait parameters | OpenClaw gait node ↔ balance controller | Shared YAML params |
| Joint names | Firmware, URDF, controllers | `joint_0..3`, no aliases |

---

## Risks & Mitigations

| # | Risk | Impact | Likelihood | Mitigation |
|---|------|--------|------------|------------|
| R1 | ESP32-S3 GPIO conflicts (strapping/PSRAM) | High | Med | Pinout validated vs datasheet; GPIO 0/3/46 and 26–32 avoided; review at Gate 0 |
| R2 | Motor drivers overheat | High | Med | Heatsinks, 80% clamp, thermal watch in firmware, current budget ~2.7 A typ / 5.1 A peak |
| R3 | WitMotion D6 protocol undocumented | Med | High | Incremental parser from SDK/contact vendor; fall back to recording raw frames first |
| R4 | Toy gearbox unsuited to position control | Med | Med | Backlash/dead-zone check at Stage 1; software deadband; accept tolerance or modify gearbox |
| R5 | OpenClaw ROS2 integration incompatible | Med | Med | Validate in simulation early (Stage 3/4); fork/modify; fallback static-gait MVP |
| R6 | IMU drift / balance instability | Med | Med | BNO055 fusion + calibration; gyro-only fallback; balance gains tuned suspended |
| R7 | UDP loss over WiFi | Med | Med | Sequence numbers; 100 ms watchdog; loss-triggered stop; Fast DDS/network tuning |
| R8 | Weight/runtime budgets missed | Med | Low | Weigh components; measure current; lighter battery; sleep modes |
| R9 | LD2420 false positives/negatives | Med | Med | Configurable zone/threshold; fuse with other cues; manual override; staged enablement |
| R10 | PCB surgery damages toy | Low | Med | Document before cutting; spare toy on hand; SA8301 inputs verified 3.3 V-logic |
| R11 | Scope creep into AI before locomotion | High | Med | AI integration explicitly optional (Stage 6.2); gate order enforced |

---

## Open Questions (carried forward + new)

| # | Question | Status / Default |
|---|----------|------------------|
| Q1 | Original gearbox suitable for PID position control? | Test at Stage 1; may need backlash/gearhead changes |
| Q2 | WitMotion D6 protocol & baud? | Contact vendor; reverse-engineer; record in overview.md at Stage 0 |
| Q3 | Chassis form factor (3D print vs acrylic)? | CAD/measure after legs verified |
| Q4 | ROS2 distro (Humble vs Jazzy)? | **Default Humble** (Ubuntu 22.04) — decide at Gate 0 |
| Q5 | Companion computer (RPi5 vs Jetson)? | **Default RPi5**; Jetson only if AI stage requires GPU |
| Q6 | Charging method (external, removable, or in-robot BMS)? | Decide at Stage 0 hardware notes |
| Q7 | Fallback if OpenClaw fails? | Custom gait generator is large; static-gait MVP acceptable — test OpenClaw early |
| Q8 | Where do calibration values live? | **Flash w/ factory defaults + ROS2 parameter override** (ADR in Stage 0) |
| Q9 | How do `/cmd_vel` velocity commands map to gait parameters (speed/step/duty)? | OpenClaw config research in Stage 4.1; define mapping table in gait YAML |

---

## Appendix A — Calibration Matrix (fill per joint during Stages 1–2)

| Joint | ADC Min | ADC Max | Angle Min (deg) | Angle Max (deg) | Direction (+PWM = ?) | Deadband (ADC) |
|-------|---------|---------|-----------------|-----------------|----------------------|----------------|
| 0 | TBD | TBD | TBD | TBD | TBD | TBD |
| 1 | TBD | TBD | TBD | TBD | TBD | TBD |
| 2 | TBD | TBD | TBD | TBD | TBD | TBD |
| 3 | TBD | TBD | TBD | TBD | TBD | TBD |

Single source of truth: `docs/hardware/calibration.md` (created Stage 0). Every stage that consumes angles (PID, URDF, gait generator) reads from here.

---

## Appendix B — File Inventory (target state)

### Documentation (maintain/update)
| File | Purpose | Updated in |
|------|---------|------------|
| `CLAUDE.md` | Agent context, safety rules, conventions | Ongoing |
| `DESC.md` | Full project description | Stage 0 (conflicts) |
| `README.md` | User-facing overview | Stage 0 (merge markers, links) |
| `docs/PLAN.md` | This stage-gated roadmap | Ongoing |
| `docs/ARCHITECTURE.md` | Layered architecture, topics, flows | Stage 0 sync |
| `docs/protocol.md` | **UDP protocol v1 (frozen)** | Stage 0 |
| `docs/decisions/ADR-001…003` | Architecture decisions | Stage 0 additions |
| `docs/hardware/pinout.md` | Pin assignments (authoritative) | Stage 1 verified |
| `docs/hardware/overview.md` | Component specs, power budget | Stage 0 sync |
| `docs/hardware/calibration.md` | Calibration single source | Stage 1–2 |
| `docs/safety/test_report.md`, `procedures.md` | Safety evidence | Stage 6 |
| `docs/logs/` | Test logs, videos, evidence | Stages 1–6 |

### Code / structure (create per stage)
| Directory | Purpose | Created |
|-----------|---------|---------|
| `firmware/` (src, include, lib, test, test-leg) | ESP32 firmware (UDP, not micro-ROS) | Reworked Stage 1–2; `experiments/microros_legacy/` in Stage 0 |
| `ros2_ws/src/clawdog_control/` | ros2_control interface, gait node, teleop, safety | Stage 3 |
| `ros2_ws/src/clawdog_description/` | URDF/Xacro, meshes | Stage 3 |
| `ros2_ws/src/clawdog_bringup/` | Launch files, params | Stage 3 |
| `ros2_ws/src/witmotion_driver/` | LiDAR → `/scan` | Stage 5 |
| `ros2_ws/src/clawdog_ai/` (optional) | RosClaw bridge | Stage 6.2 |
| `openclaw/` | OpenClaw submodule/integration | Stage 4 |
| `scripts/` | UDP test client, calibration sweep helper, safety test suite | Stage 1–6 |
| `tests/` | Integration tests (host/companion side) | Stage 3+ |

---

## How Agents Should Use This Plan
1. Read CLAUDE.md (safety rules) and the ADRs before any work; CLAUDE.md overrides this file on **safety** questions.
2. Work within the current stage only; never start next-stage tasks before the current Gate passes.
3. Never skip a **HUMAN REVIEW GATE** — flag it and stop; do not auto-advance powered-hardware stages.
4. On completing a Gate, tick the checklist and record date/evidence in the section notes, then commit.
5. Update this document when scope, decisions, or gotchas change; record architectural changes as ADRs.

---

*Living document. Stage boundaries are gates, not dates — quality gates gate, estimates guide.*

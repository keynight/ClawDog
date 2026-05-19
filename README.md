# ClawDog
The ClawDog project aims to transform your toy robot dog into an intelligent, autonomous quadruped robot using ESP32-S3-N16R8, ROS2, and WitRobot LiDAR, with eventual integration into the OpenClaw framework 
![alt text](https://github.com/keynight/ClawDog/blob/main/ClawDog.png)

# **ClawDog Project - Comprehensive Development Plan**

## **📋 Project Overview**

The **ClawDog** project aims to transform your toy robot dog into an intelligent, autonomous quadruped robot using ESP32-S3-N16R8, ROS2, and WitRobot LiDAR, with eventual integration into the OpenClaw framework [[22]][[27]].

---

## **🛠️ Hardware Architecture**

### **Components Needed:**
1. **ESP32-S3-N16R8** - Main controller
2. **WitRobot LiDAR** - For autonomous navigation
3. **Motor Driver** - Keep from original toy
4. **Motors/Servos** - Keep from original toy (12 DOF recommended)
5. **IMU Sensor** (MPU6050/MPU9250) - For balance
6. **Battery** - 7.4V LiPo recommended
7. **Motor Driver Board** - TB6612FNG or similar
8. **UART to USB Converter** - For LiDAR connection
9. **Power Distribution Board**

---

## **💻 Software Architecture**

### **System Layers:**
```
┌─────────────────────────────────────┐
│      OpenClaw Framework             │ ← High-level AI control
├─────────────────────────────────────┤
│      ROS2 Navigation Stack (Nav2)   │ ← Autonomous navigation
├─────────────────────────────────────┤
│   micro-ROS Agent (PC/Raspberry Pi) │ ← Bridge
├─────────────────────────────────────┤
│   ESP32-S3 with micro-ROS           │ ← Low-level motor control
├─────────────────────────────────────┤
│   Motors | LiDAR | IMU | Sensors    │ ← Hardware layer
└─────────────────────────────────────┘
```

---

## **📅 Implementation Plan**

### **Phase 1: Foundation Setup (Weeks 1-2)**

#### **1.1 Install ROS2 Humble on PC**
```bash
# Install ROS2 Humble (Ubuntu 22.04)
sudo apt update && sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo apt update
sudo apt install -y ros-humble-desktop
source /opt/ros/humble/setup.bash
```

#### **1.2 Set up micro-ROS for ESP32-S3**
Follow the micro-ROS installation guide [[40]][[43]]:

```bash
# Install micro-ROS dependencies
sudo apt install -y python3-dev python3-pip
pip3 install catkin_pkg lark-parser empy==3.3.44

# Clone micro-ROS firmware
git clone -b humble https://github.com/micro-ROS/micro_ros_espidf_component.git
cd micro_ros_espidf_component
```

#### **1.3 Install micro-ROS Agent**
```bash
# On your PC/Raspberry Pi
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 -b 115200
```
The micro-ROS Agent acts as a bridge between ESP32 and ROS2 [[45]].

---

### **Phase 2: ESP32 Firmware Development (Weeks 3-4)**

#### **2.1 Create micro-ROS Firmware**
```cpp
// main.cpp - Basic micro-ROS setup for ESP32-S3
#include <micro_ros_espidf_component.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <geometry_msgs/msg/twist.h>

// Motor control subscriber
rcl_subscription_t cmd_vel_sub;
geometry_msgs_msg_Twist cmd_vel_msg;

void cmd_vel_callback(const void *msgin) {
    const geometry_msgs_msg_Twist *msg = (const geometry_msgs_msg_Twist *)msgin;
    // Process velocity commands for quadruped gait
    control_motors(msg->linear.x, msg->angular.z);
}

void setup() {
    // Initialize micro-ROS
    micro_ros_espidf_component_init();
    
    // Create node
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_ops = rcl_node_get_default_options();
    rcl_node_init(&node, "clawdog_esp32", "", &node_ops);
    
    // Create subscriber for motor control
    cmd_vel_sub = rcl_get_zero_initialized_subscription();
    rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
    rcl_subscription_init(&cmd_vel_sub, &node, 
                         ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
                         "cmd_vel", &sub_ops);
}

void loop() {
    // Spin micro-ROS
    rcl_spin(&node);
    vTaskDelay(pdMS_TO_TICKS(10));
}
```

#### **2.2 Motor Control Implementation**
Implement inverse kinematics for quadruped locomotion [[48]][[54]]:

```cpp
// inverse_kinematics.cpp
struct LegIK {
    float calculate_joint_angles(float x, float y, float z) {
        // Calculate hip, knee, ankle angles
        float hip = atan2(y, x);
        float knee = acos((x*x + z*z - L1*L1 - L2*L2) / (2*L1*L2));
        float ankle = /* calculate ankle */;
        return {hip, knee, ankle};
    }
};

// Gait patterns (Trot, Walk, Bound)
enum GaitPattern { TROT, WALK, BOUND };

void execute_gait(GaitPattern pattern, float velocity, float turn) {
    // Implement quadruped gait sequences
    // Control 4 legs with proper timing
}
```

---

### **Phase 3: LiDAR Integration (Weeks 5-6)**

#### **3.1 Connect WitRobot LiDAR**
Connect LiDAR to ESP32 or directly to PC via UART/USB [[56]][[64]]:

```bash
# Install LiDAR driver for ROS2
cd ~/ros2_ws/src
git clone https://github.com/Myzhar/ldrobot-lidar-ros2.git  # Example for LD19
cd ..
colcon build --packages-select ldrobot_lidar_ros2
source install/setup.bash
```

#### **3.2 Create LiDAR ROS2 Node**
```python
# lidar_driver.py
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan
import serial

class WitRobotLiDAR(Node):
    def __init__(self):
        super().__init__('witrobot_lidar')
        self.publisher = self.create_publisher(LaserScan, 'scan', 10)
        self.serial_port = serial.Serial('/dev/ttyUSB0', 115200)
        self.timer = self.create_timer(0.1, self.publish_scan)
    
    def publish_scan(self):
        scan_msg = LaserScan()
        scan_msg.header.stamp = self.get_clock().now().to_msg()
        scan_msg.header.frame_id = 'laser_frame'
        scan_msg.angle_min = -3.14159
        scan_msg.angle_max = 3.14159
        scan_msg.angle_increment = 0.01745  # 1 degree
        scan_msg.range_min = 0.1
        scan_msg.range_max = 12.0
        
        # Read data from LiDAR
        data = self.read_lidar_data()
        scan_msg.ranges = data
        
        self.publisher.publish(scan_msg)
```

---

### **Phase 4: ROS2 Navigation Stack (Weeks 7-9)**

#### **4.1 Install Nav2**
```bash
sudo apt install ros-humble-navigation2
sudo apt install ros-humble-nav2-bringup
```

#### **4.2 Create Robot URDF**
```xml
<!-- clawdog.urdf -->
<robot name="clawdog">
    <!-- Base link -->
    <link name="base_link">
        <visual>
            <geometry>
                <box size="0.3 0.2 0.15"/>
            </geometry>
        </visual>
    </link>
    
    <!-- LiDAR link -->
    <link name="laser_frame">
        <visual>
            <geometry>
                <cylinder radius="0.05" length="0.05"/>
            </geometry>
        </visual>
    </link>
    
    <joint name="laser_joint" type="fixed">
        <parent link="base_link"/>
        <child link="laser_frame"/>
        <origin xyz="0 0 0.1" rpy="0 0 0"/>
    </joint>
    
    <!-- 4 Legs with 3 DOF each -->
    <!-- Implement leg kinematics -->
</robot>
```

#### **4.3 Configure Nav2 for Quadruped**
Create navigation parameters [[65]][[66]]:

```yaml
# nav2_params.yaml
nav2_params:
  controller_server:
    ros__parameters:
      controller_frequency: 20.0
      min_x_velocity_threshold: 0.001
      min_y_velocity_threshold: 0.001
      min_theta_velocity_threshold: 0.001
      
  planner_server:
    ros__parameters:
      planner_plugins: ["GridBased"]
      GridBased:
        plugin: "nav2_navfn_planner/NavfnPlanner"
        tolerance: 0.5
        use_astar: false
        allow_unknown: true
        
  recovery_server:
    ros__parameters:
      recovery_plugins: ["spin", "backup", "wait"]
```

---

### **Phase 5: Integration & Testing (Weeks 10-12)**

#### **5.1 Create Launch Files**
```python
# clawdog_launch.py
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # micro-ROS Agent
        Node(
            package='micro_ros_agent',
            executable='micro_ros_agent',
            arguments=['serial', '--dev', '/dev/ttyACM0', '-b', '115200']
        ),
        
        # LiDAR Driver
        Node(
            package='witrobot_lidar',
            executable='lidar_node',
            name='witrobot_lidar'
        ),
        
        # Robot State Publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': open('clawdog.urdf').read()}]
        ),
        
        # Nav2
        Node(
            package='nav2_bringup',
            executable='bringup_launch.py',
            parameters=['nav2_params.yaml']
        ),
    ])
```

#### **5.2 Test Autonomous Navigation**
```bash
# Launch everything
ros2 launch clawdog_bringup clawdog_launch.py

# Set initial pose in RViz
# Send goal pose
ros2 action send_goal /navigate_to_pose nav2_msgs/action/NavigateToPose \
"{pose: {header: {frame_id: map}, pose: {position: {x: 1.0, y: 2.0}}}}"
```

---

### **Phase 6: OpenClaw Integration (Weeks 13-16)**

#### **6.1 Install OpenClaw**
```bash
# Clone OpenClaw Robotics
git clone https://github.com/LooperRobotics/OpenClaw-Robotics.git
cd OpenClaw-Robotics
pip install -e .
```

#### **6.2 Create ROSClaw Bridge**
Integrate with ROSClaw (OpenClaw ROS2 framework) [[22]][[27]]:

```python
# openc law_bridge.py
import rclpy
from openc law.agent import Agent
from openc law.robot import RobotCapabilities

class ClawDogAgent(Agent):
    def __init__(self):
        super().__init__("ClawDog")
        
        # Define robot capabilities
        self.capabilities = RobotCapabilities(
            locomotion="quadruped",
            sensors=["lidar", "imu"],
            manipulation=False
        )
        
    async def execute_task(self, task: str):
        # High-level task execution via OpenClaw
        if "navigate" in task:
            await self.navigate_to_goal(task)
        elif "explore" in task:
            await self.explore_environment()
```

---

## **📁 GitHub Repository Structure**

```
ClawDog/
├── README.md
├── LICENSE
├── docs/
│   ├── hardware_setup.md
│   ├── software_architecture.md
│   └── tutorials/
├── firmware/
│   ├── esp32_micro_ros/
│   │   ├── main/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── main.cpp
│   │   │   ├── motor_control.cpp
│   │   │   └── inverse_kinematics.cpp
│   │   └── README.md
├── ros2_packages/
│   ├── clawdog_bringup/
│   │   ├── launch/
│   │   │   └── clawdog_launch.py
│   │   ├── params/
│   │   │   └── nav2_params.yaml
│   │   └── CMakeLists.txt
│   ├── clawdog_description/
│   │   ├── urdf/
│   │   │   └── clawdog.urdf
│   │   └── CMakeLists.txt
│   ├── witrobot_lidar_driver/
│   │   ├── src/
│   │   │   └── lidar_node.py
│   │   └── CMakeLists.txt
│   └── clawdog_navigation/
│       ├── config/
│       ├── maps/
│       └── CMakeLists.txt
├── openc law_integration/
│   ├── openc law_bridge.py
│   ├── capabilities.py
│   └── README.md
├── hardware/
│   ├── schematics/
│   ├── 3d_models/
│   └── wiring_diagram.pdf
├── scripts/
│   ├── setup.sh
│   ├── flash_esp32.sh
│   └── calibration.py
└── tests/
    ├── unit_tests/
    └── integration_tests/
```

---

## **🔧 Development Workflow**

### **Step-by-Step Guide:**

1. **Week 1-2: Environment Setup**
   - Install Ubuntu 22.04 or use WSL2
   - Install ROS2 Humble
   - Install ESP-IDF for ESP32-S3
   - Set up micro-ROS [[40]][[43]]

2. **Week 3-4: Basic Motor Control**
   - Flash micro-ROS firmware to ESP32-S3
   - Test motor control via ROS2 topics
   - Implement basic gait patterns [[48]]

3. **Week 5-6: Sensor Integration**
   - Connect and test WitRobot LiDAR
   - Publish LaserScan messages
   - Add IMU for balance

4. **Week 7-9: Navigation**
   - Configure Nav2 for quadruped [[65]][[66]]
   - Create URDF model
   - Test autonomous navigation in simulation

5. **Week 10-12: Real Robot Testing**
   - Deploy to physical robot
   - Tune PID controllers
   - Test obstacle avoidance

6. **Week 13-16: OpenClaw Integration**
   - Install OpenClaw framework [[27]]
   - Create capability interfaces
   - Test AI-driven control

---

## **🚀 Quick Start Commands**

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/ClawDog.git
cd ClawDog

# Build ROS2 packages
colcon build --symlink-install
source install/setup.bash

# Flash ESP32
cd firmware/esp32_micro_ros
idf.py set-target esp32s3
idf.py flash monitor

# Run on PC
ros2 launch clawdog_bringup clawdog_launch.py
```

---

## **📚 Key Resources**

1. **micro-ROS Documentation**: https://micro.ros.org
2. **ROS2 Navigation**: https://docs.nav2.org
3. **OpenClaw**: https://github.com/LooperRobotics/OpenClaw-Robotics [[27]]
4. **ESP32 Quadruped Examples**: [[50]][[55]]
5. **Nav2 for Quadrupeds**: [[65]][[66]]

---

## **⚠️ Important Notes**

- **Power Management**: Ensure adequate battery capacity (minimum 5000mAh)
- **Safety**: Implement emergency stop functionality
- **Testing**: Test each component individually before integration
- **Documentation**: Document all changes and calibrations
- **Version Control**: Use Git branches for feature development

---

## **🎯 Milestones**

✅ **M1**: ESP32-S3 communicates with ROS2 via micro-ROS  
✅ **M2**: Motors respond to ROS2 commands  
✅ **M3**: LiDAR data published to ROS2  
✅ **M4**: Basic gait implemented (walking)  
✅ **M5**: Nav2 navigation working  
✅ **M6**: Autonomous obstacle avoidance  
✅ **M7**: OpenClaw integration complete  
✅ **M8**: AI-driven task execution

---

## **🤝 Next Steps**

1. Create GitHub repository: `git init && git remote add origin https://github.com/keynight/ClawDog.git`
2. Start with Phase 1 setup
3. Join ROS2 and OpenClaw communities for support
4. Document your progress with photos/videos

**Good luck with your ClawDog project! I'll guide you through each step.** 🐕

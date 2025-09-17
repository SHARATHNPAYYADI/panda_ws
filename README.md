# Panda Workspace (panda_ws)

A ROS 2 workspace for the **Franka Emika Panda robot**, providing simulation, motion planning, camera based pose estimation (colour based) and integration tests with Gazebo and MoveIt 2.

---

## 📦 Packages

- **`panda_bringup`** – Launch files to bring up the robot in simulation and execute demo scenarios.  
- **`panda_integration_test`** - Integration test cased for verifying object detection, motion planning and grasping.
- **`panda_motion_planning`** – Nodes for joint control, motion planning, and pick-and-place scenarios.  
- **`panda_msgs`** – Custom service and message definitions for Panda robot control.  
- **`panda_object_detection`** – Nodes for OpenCV based object pose estimation.  
- **`IFRA_LinkAttcher`** - Third Party module which helps the robot link to attach to the object link

---

## 🔧 Requirements

- ROS 2 Humble (recommended)  
- Gazebo (Classic)  
- MoveIt2  
- Python 3.10+  
- Dependencies from `rosdep`  


## ⚙️ Build Instructions

- **`Clone and build:`**
```bash
mkdir -p colcon_ws/src
cd colcon_ws/src
git clone git@github.com:SHARATHNPAYYADI/panda_ws.git
cd ../
git submodule init && git submodule update
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install

```

- **`Source the workspace:`**
```bash
source install/setup.bash
```

## 🚀 Demo
- **`Launch Simulation`**
    - Simple pick and place scenario

    ```bash
    ros2 launch panda_bringup simple_pick_place_demo.launch.py
    ```
    - Camera based pick and place scenario

    ```bash
    ros2 launch panda_bringup camera_based_pick_place_demo.launch.py
    ```

## 🧪 Testing

- Unit test
    ```bash
    colcon test --packages-select panda_object_detection --event-handlers console_direct+
    ```
    as for the motion planning verification we need robot_description so we need to start the launch file which publihes the robot_description
    ```bash
    ros2 launch panda_bringup simple_pick_place_demo.launch.py
    #in another terminal execute
    colcon test --packages-select panda_motion_planning --event-handlers console_direct+

    ```
- Integration test
    ```bash
    colcon test --packages-select panda_integration_test --event-handlers console_direct+
    ```

We can also run the entire package test using:
 ```bash
colcon test
colcon test-result --verbose
```
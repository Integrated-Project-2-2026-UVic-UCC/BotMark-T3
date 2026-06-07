<h1 align="center">🌱🤖Automated Line-Painting Robot🤖🌱</h1>

<p align="center"> A mechatronics engineering project focused on the precise marking of grass fields using an autonomous robot </p>
<div align="center">
   <img width=100% src=https://capsule-render.vercel.app/api?type=waving&height=100&color=gradient&reversal=true/>
</div>

<a href="https://youtu.be/6cvCalASXHU" target="_blank"><img align="center" alt="difermon09 | GitHub" width="100%" src="https://github.com/Integrated-Project-2-2026-UVic-UCC/BotMark-T3/blob/main/assets/Botmark_video_image.png"/> </a>

## 📌Introduction:
Field marking in football pitches is a time-consuming and precision-critical task. Traditional methods require manual measurement, steady handling, and several hours of labor.

Our bussiness, BOTMARK, proposes the design and development of a reduced-scale autonomous robot capable of navigating a football field and painting regulation-compliant lines using integrated positioning, sensing, and control systems.<br>
<img align="right" width="216" height="188" alt="image" src="https://github.com/user-attachments/assets/b5e36d97-bb9c-4655-bead-495ff6bfb911"/>

The goal is to combine:
- Mechanical robustness  
- Accurate dispensing mechanisms  
- Autonomous path execution  


## 🏅Objectives:
The main objective is to design and build an autonomous robotic platform capable of marking the lines of a grass field with high precision and minimal human intervention.

### Specific goals
To achieve this, we will fulfill the following requirements:
| Feature | Description |
|---------|-------------|
| 🔍 Localization | Integration of reliable positioning systems (Odometry + Lidar) |
| 🛞 Mobility | Chassis suitable for outdoor use |
| 🎯 Precision | Precise paint application mechanism |
| 🤖 Autonomy | Automated painting sequences |
| 📱 Control | Mobile application for supervision |


## 📁 Repository Structure

<details>
  <summary>🖼️ <b>assets</b></summary>
  <ul>
    <li>Contains all the <b>media</b> and <b>images</b> for the this README.</li>
  </ul>
</details>

<details>
  <summary>💡 <b>electronics</b></summary>
  <ul>
    <li>Contains all the electronic circuit designs, <b>PCB layouts</b>, <b>Schematics</b>, and <b>Gerber files</b> necessary for board manufacturing.</li>
  </ul>
</details>

<details>
  <summary>🔩 <b>mechanics</b></summary>
  <ul>
    <li>Contains the mechanical plans, source <b>CAD files</b>, and ready-to-print <b>STL files</b> for the chassis, wheels, and sand funnel mechanisms.</li>
  </ul>
</details>

<details>
  <summary>📱 <b>web_app</b></summary>
  <ul>
    <li>Contains the application code for the <b>user interface</b> and remote supervision of the robot.</li>
  </ul>
</details>

<details>
  <summary>🤖 <b>robot_coding</b></summary>
  <ul>
    <li>Central repository containing the complete codebase for the autonomous robot, from low-level microcontroller firmware to high-level ROS 2 navigation and networking.</li>
  </ul>
  
  <blockquote>
  <details>
    <summary>👾 <b>firmware</b> <i>(ESP32)</i></summary>
    <ul>
      <li>ESP32 PlatformIO project containing the low-level C/C++ programming.</li>
      <ul>
      <li><b>lib/:</b> Custom libraries for hardware control (Encoders, IMU, Kinematics, Motors, PID, Zenoh).</li>
      <li><b>src/:</b> Main execution loop (<code>main.cpp</code>).</li>
      </ul>
    </ul>
  </details>

  <details>
    <summary>🧠 <b>ros2_ws</b> <i>(High-Level Logic & ROS 2)</i></summary>
    <ul>
      <li>ROS 2 workspace responsible for high-level processing, autonomous navigation, sensor data handling, and DDS communication.</li>
      <ul>
        <li><b>mission_manager/:</b> Python package handling navigation sequences. Includes a <code>config/</code> folder with the route YAML.</li>
        <li><b>controller_pkg:</b> C++ package for managing the robot's control loop and odometry.</li>
        <li><b>ldlidar_stl_ros2/:</b> Node driver for the Lidar sensor (<a href="https://github.com/ldrobotSensorTeam/ldlidar_stl_ros2.git" target="_blank">provided by manufacturer</a>).</li>
        <li><b>obstacle_detector/:</b> C++ package for Lidar data processing and collision avoidance.</li>
        <li><b>zenoh_bridge/:</b> C++ package for DDS communication with the microcontroller.</li>
        <li><b>robot_bringup/:</b> Python package containing the launch file (<code>launch/robot_core.launch.py</code>) to start the entire system.</li>
      </ul>
    </ul>
  </details>

  <details>
    <summary>🔌 <b>udev & zenoh_router</b></summary>
    <ul>
      <li><b>udev/:</b> Linux rules (<code>99-robot.rules</code>) for stable USB sensor and motor connections.</li>
      <li><b>zenoh_router_v1.8.0/:</b> Router setup and configuration for Zenoh communications.</li>
    </ul>
  </details>
  </blockquote>
</details>

<details>
  <summary>⚙️ <b>Environment Setup</b></summary>
  <ul>
    <li><b>setup_env.sh:</b> Bash script to quickly configure the development environment and dependencies.
</details>


## <picture><img src = "https://github.com/7oSkaaa/7oSkaaa/blob/main/Images/about_me.gif?raw=true" width = 30px></picture> Development Status
**Current Phase:** 🚩Milestone 3🚩
- Design sand funnel
- Solder components on the test PCB
- Programm Lidar triangulation
- Milestone 3 - Mid term presentation


## <img src='https://raw.githubusercontent.com/ShahriarShafin/ShahriarShafin/main/Assets/handshake.gif' width="30px"> Team

- Project Manager – Oriol Arbonies  
- Mechanical Lead – Àlex López  
- Electronics Lead – Joan Marc Tur  
- Software Lead – Dídac Fernández -> <a href="https://github.com/difermon09" target="_blank"><img align="center" alt="difermon09 | GitHub" width="26px" src="https://raw.githubusercontent.com/rahulbanerjee26/githubAboutMeGenerator/main/icons/github.svg"/> </a> 


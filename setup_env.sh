#!/bin/bash

''' Professional environment configuration script to set up hardware permissions 
    and ROS 2 dependencies for the robot coding project.'''

# Determine the absolute directory path where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Starting environment setup in $SCRIPT_DIR"

# ---------------------------------------------------------
# 1. Hardware Permissions (UDEV Rules)
# ---------------------------------------------------------
echo "Copying UDEV rules..."
UDEV_SOURCE="$SCRIPT_DIR/robot_coding/udev/99-robot.rules"

if [ -f "$UDEV_SOURCE" ]; then
    sudo cp "$UDEV_SOURCE" /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    echo "UDEV rules successfully installed."
else
    echo "Error: Could not find UDEV rules file at $UDEV_SOURCE"
fi

# ---------------------------------------------------------
# 2. ROS 2 Dependency Installation (rosdep)
# ---------------------------------------------------------
echo "Installing ROS 2 dependencies..."
WS_DIR="$SCRIPT_DIR/robot_coding/ros2_ws"

if [ -d "$WS_DIR" ]; then
    # Safely navigate to the workspace directory
    cd "$WS_DIR" || { echo "Failed to enter directory $WS_DIR"; exit 1; }
    
    # Update local rosdep database
    rosdep update
    
    # Scan the 'src' folder and install all missing system dependencies automatically
    rosdep install --from-paths src --ignore-src -r -y
    
    echo "ROS 2 dependencies successfully installed."
else
    echo "Error: Could not find ROS 2 workspace at $WS_DIR"
fi

echo "Environment setup completed successfully."
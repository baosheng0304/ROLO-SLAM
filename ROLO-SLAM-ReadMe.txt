

Data/
├── apt-lists-backup/          # Backup of APT lists
├── deb-packages/              # Required .deb package files
├── gtsam/                     # GTSAM source code
└── rolo_ws/                   # ROLO-SLAM ROS workspace
 !!! Important
 In order to reduce file size uploding/downloading, we removed duplicated files in the apt-lists-backup and deb-packages
 1. Add all files downloaded from download https://github.com/baosheng0304/PointCloudLib/tree/main/var-lib-apt-lists to the apt-lists-backup folder
 2. Append all deb packages which we already sent previously to the deb-packages

Step 1: Move Required Folders
    Please move the deb-packages and apt-lists-backup folders into  Downloads directory. 
    Then, please move the 'gtsam' and 'rolo_ws' folders into home directory.

Step 2: Restore APT Cache and Install ROS Packages
    # Restore deb packages and APT list
    sudo mv ~/Downloads/deb-packages/*.deb /var/cache/apt/archives/
    sudo mv ~/Downloads/apt-lists-backup/* /var/lib/apt/lists/

Step 3: Install ROS Noetic and Dependencies

    # Add ROS package source
    sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'

    # Install ROS Noetic (assumes .deb packages are already cached)
    sudo apt install ros-noetic-desktop-full
    sudo apt install ros-noetic-slam-gmapping

    # Setup ROS environment
    source /opt/ros/noetic/setup.bash
    echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
    source ~/.bashrc

    # Install additional dependencies
    sudo apt install python3-rosdep python3-rosinstall python3-rosinstall-generator python3-wstool build-essential
    sudo apt install python3-catkin-tools

    # Check ROS version
    rosversion -d

Step 4: Build GTSAM
    
    cd ~/gtsam
    mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DGTSAM_USE_SYSTEM_EIGEN=ON
    make -j$(nproc) VERBOSE=1
    sudo make install

Step 5: Build ROLO-SLAM Workspace
    cd ~/rolo_ws
    catkin_make

    # Add library path
    echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
    source ~/.bashrc

    # Source workspace
    source devel/setup.bash


Step 6: Run ROLO-SLAM
    roslaunch rolo rolo_run.launch


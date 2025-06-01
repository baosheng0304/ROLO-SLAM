Data
 -apt-lists-backup
 -deb-packages
 -gtsam
 -rolo_ws

put 'deb-packages' and 'apt-lists-backup' folders into Downloads folder 
put 'gtsam' and 'rolo_ws' folders into Home folder 

sudo mv /home/<username>/Downloads/deb-packages/*.deb /var/cache/apt/archives/
sudo mv /home/<username>/Downloads/apt-lists-backup/* /var/lib/apt/lists/

#open the terminal in Home

sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt install ros-noetic-desktop-full
sudo apt install ros-noetic-slam-gmapping
source /opt/ros/noetic/setup.bash
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
sudo apt install python3-rosdep python3-rosinstall python3-rosinstall-generator python3-wstool build-essential
rosversion -d // ros checking
sudo apt install python3-catkin-tools


cd ~/gtsam
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DGTSAM_USE_SYSTEM_EIGEN=ON
make -j$(nproc) VERBOSE=1
sudo make install


cd ~/rolo_ws
catkin_make
echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
source devel/setup.bash

roslaunch rolo rolo_run.launch   //ROLO-SLAM checking
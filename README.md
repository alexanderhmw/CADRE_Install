# CADRE_Install
One-click Installation of CADRE

## Install and Configure Qt5
CADRE_Install does not provide Qt5 installation since it requires user account and password to access Qt5 repo. Please install Qt5 under **/opt/Qt** and install OpenSSL for ArcGIS Qt.

![img.png](img/img.png)

### Configuration
1. Add Qt5.x.x library path to ldconfig
   ```
   sudo bash -c 'echo "/opt/Qt/5.x.x/gcc_64/lib" > /etc/ld.so.conf.d/Qt.conf'
   sudo ldconfig
   ```
2. Set default Qt5 to Qt5.x.x
   ```
   sudo apt-get install qt5-default -y
   qtchooser -install qt5-opt /opt/Qt/5.x.x/gcc_64/bin/qmake
   echo "export QT_SELECT=qt5-opt" > ~/.bashrc
   ```
3. Check qmake version
   ```
   qmake --version
   ```

## Install CADRE Packages
1. Clone CADRE_Install 
   ```
   git clone https://github.com/alexanderhmw/CADRE_Install.git
   ```
2. Install Prerequisite Packages
   ```
   cd CADRE_Install
   sudo apt-get install python3 python3-dev python3-pip
   sudo pip install -r requirements.txt
   ```
3. Install CADRE 3rd-Party Packages (It may take a long time)
   ```
   python3 cadre_install.py <cadre_tartanracing_path>
   ```
4. Check CADRE Package Installation
   ```
   vim ./MakeIt/makeit_status.json
   ```

## Compile CADRE
```
scons -j8
```

## Add CADRE Package

## Plan
1. Tools to generate tasks and interfaces with templates
2. Tools to add 3rd-party packages

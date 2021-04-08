# SmartTV Alpha X
 
## Setup ## 

After clone repo, need to install pistache and nlohmann.

### Install pistache
```
sudo add-apt-repository ppa:pistache+team/unstable
sudo apt update
sudo apt install libpistache-dev
```

### Install nlohmann
```
sudo apt-get install nlohmann-json3-dev
```

### Create CMakeFiles
```
cd <project>
cmake .
```

### Build
```
cmake --build .
```

### Run
```
./smart-tv
```

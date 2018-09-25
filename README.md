# kio_recentfolders
This is the [KIO](http://en.wikipedia.org/wiki/KIO) slave for displaying recently used folders (only folders).

This KIO slave only works when Baloo service is enabled.
![](screenshot.png)

## Build
```bash
mkdir build && cd build
cmake ..
make
```

## Installation
```
sudo make install
```

## Configuration
The KIO slave can be configured via configuration file `~/.config/kio_recentfolders`.
The file will be created after first run of the KIO slave (you need to open URL ```recentfolders:/``` in Dolphin or Konqueror).

Those options can be configured:
* BackDays — how many days to look for changed folders back in history.

Default configuration:
```ini
[General]
BackDays=7
```

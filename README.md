# Installation

``` sh
wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.28-98749-g6643ecee5-gcc-linux.tar.gz -O ~/pin.tar.gz
mkdir ~/pin && tar xf ~/pin.tar.gz -C ~/pin --strip-components 1
git clone https://github.com/Jaeyong-Cho/pftracer.git ~/pin/source/tools/PFTrace
cd ~/pin/source/tools/PFTrace/ && make
```

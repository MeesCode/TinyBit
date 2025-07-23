### on a fresh ubuntu install:

#### make sure the submodules are initializad and pulled:
```
git submodule init
git submodule update --recursive
```

#### install SDL2
```
sudo apt-get install libsdl2-dev libsdl2-image-dev
```

#### compile 
```
make
```

#### run
```
./bin/tinybit
```
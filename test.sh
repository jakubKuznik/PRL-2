mpic++ --prefix /usr/local/share/OpenMPI -o life life.cpp
mpirun --prefix /usr/local/share/OpenMPI -n 4 life $1 $2
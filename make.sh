rm output.h5md
hdf5home=/disk/jtyang/hdf5-1.14.6
hdf5lib=$hdf5home/lib
hdf5inc=$hdf5home/include
g++ -g ./src/test.cpp ./src/h5md_io.cpp ./src/h5md_io.h \
 -I$hdf5inc \
 -L$hdf5lib \
 $hdf5lib/libhdf5_cpp.a $hdf5lib/libhdf5.a -lz -ldl -lm\
 -o ./build/run

./build/run
h5dump output.h5md
# g++ ./src/hello_hdf5.cpp \
#  -I$hdf5inc \
#  -L$hdf5lib \
#  -lhdf5_cpp -lhdf5 \
#  -Wl,-rpath,$hdf5lib \
#  -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 \
#  -o ./build/hello_hdf5

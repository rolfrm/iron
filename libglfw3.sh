rm -r glfw/build
cmake -S glfw -B glfw/build -DCMAKE_BUILD_TYPE=Release
make -C glfw/build glfw
cp glfw/build/src/libglfw3.a .
rm -r libglfw3_o
mkdir libglfw3_o
cd libglfw3_o
ar -x ../libglfw3.a
cd .. 
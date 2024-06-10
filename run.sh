set -ex

rm -rf build;
mkdir -p build;
cd build;
cmake .. && make

cd ../

source ./venv/bin/activate
python3 plot_drawer.py
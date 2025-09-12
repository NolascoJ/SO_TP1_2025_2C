pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze
pvs-studio-analyzer analyze
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
cd "/Users/jnolascodecarles/ITBA/SO_TP1" && make pvs-analysis | cat
# 1) Ensure license is visible to PVS
mkdir -p ~/.config/PVS-Studio && cp -f .config/PVS-Studio/PVS-Studio.lic ~/.config/PVS-Studio/
# 2) Force rebuild under trace, then analyze and convert
make clean
pvs-studio-analyzer trace -o strace_out -- make -B all
pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
plog-converter -a GA:1,2 -t tasklist -o report.tasks PVS-Studio.log
clear
mkdir -p ~/.config/PVS-Studio && cp -f .config/PVS-Studio/PVS-Studio.lic ~/.config/PVS-Studio/
make clean
make clean
pvs-studio-analyzer trace -o strace_out -- make -B all
pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
make clean   # or equivalent
make clean
pvs-studio-analyzer trace -- make
make clean
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze -f strace_out -o PVS-Studio.log
pvs-studio-analyzer trace -- -C gcc make
pvs-studio-analyzer trace -- -C gcc make
make clean
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
which gcc
pvs-studio-analyzer analyze -f strace_out -C /usr/bin/gcc -o PVS-Studio.log
pvs-studio-analyzer analyze -f strace_out -C /usr/bin/gcc -o PVS-Studio.log
grep execve strace_out | grep gcc | head -n 10
grep execve strace_out | grep gcc | head -n 10
ls Makefile
ls
make clean
pvs-studio-analyzer trace -o strace_out -- make -B all
test -s strace_out || { echo "trace is empty"; exit 1; }
pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
apt-get update && apt-get install -y build-essential make gcc libncurses5-dev
apt-get update && apt-get install -y build-essential make gcc libncurses5-dev
pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
apt-get update && apt-get install -y build-essential make gcc libncurses5-dev
pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"
ls Makefile 
make clean
pvs-studio-analyzer trace -o strace_out -- make -B all
pvs-studio-analyzer trace -o strace_out -- make -B all
pvs-studio-analyzer trace -o strace_out -- make -B all
# inside the project dir
make clean
pvs-studio-analyzer trace -o /tmp/strace_out -- make -B all
pvs-studio-analyzer analyze -f /tmp/strace_out -C gcc -o PVS-Studio.log
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
make clean
pvs-studio-analyzer trace -o /tmp/strace_out -- make -B all
test -s /tmp/strace_out || { echo "trace is empty (no compiles captured)"; exit 1; }
pvs-studio-analyzer analyze -f /tmp/strace_out -C gcc -o PVS-Studio.log
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
clear
make clean
pvs-studio-analyzer trace -o /tmp/strace_out -- make -B all
test -s /tmp/strace_out || { echo "trace is empty (no compiles captured)"; exit 1; }
pvs-studio-analyzer analyze -f /tmp/strace_out -C gcc -o PVS-Studio.log
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
make -nB all
make clean
pvs-studio-analyzer trace -o /tmp/strace_out -- make -B all
ls
ls -al
ls
make clean
pvs-studio-analyzer trace -o ./strace_out -- make -B all
test -s ./strace_out || { echo "trace is empty (no compiles captured)"; exit 1; }
apt-get update && apt-get install -y bear
make clean
bear -- make -B all
pvs-studio-analyzer analyze -l compile_commands.json -C gcc -o PVS-Studio.log
echo "$HOME"
ls -l ~/.config/PVS-Studio/
# Ensure license is installed where the analyzer looks
pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"
ls -l ~/.config/PVS-Studio/PVS-Studio.lic
export PVS_STUDIO_ANALYZER_LICENSE_FILE="$HOME/.config/PVS-Studio/PVS-Studio.lic"
# Use compile database (no ptrace needed)
apt-get update && apt-get install -y bear
make clean
bear -- make -B all
# Analyze and convert
pvs-studio-analyzer analyze -l compile_commands.json -C gcc -o PVS-Studio.log
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
make clean
pvs-studio-analyzer trace -o ./strace_out -- make -B all
make clean
pvs-studio-analyzer analyze -l compile_commands.json -C gcc -o PVS-Studio.log
pvs-studio-analyzer trace --make
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
cd ..
ls
cd ./home/
ls
cd ~
ls
make clean
pvs-studio-analyzer trace -o "$(pwd)/strace_out" -- make -B all
strace -o /dev/null true   # should succeed if ptrace is permitted
strace -o /dev/null true   # should succeed if ptrace is permitted
strace -o /dev/null true   # should succeed if ptrace is permitted
ls -l ./strace_out         # should now exist in your cwd
make clean
clear
pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"
find . -name "*.c"
pvs-studio-analyzer --help
pvs-studio-analyzer trace -- make
exit
make clean all
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player
exit
git branch
git pull
ls
exit
ps -all
pkill master
pkill player
pkill view
pkill gdb
ps -all
pkill gdb
ps -all
pkill view
ps -all
# -9 significa "matar a la fuerza" (SIGKILL)
kill -9 2813 2814 2815 2816 2820 2890 2891 2899
ps -all
ps -all
ps -all
ps -all
exit
make clean all
./bin/master -v ./bin/view -p ./bin/player
make clean
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player
make clean
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player
make clean all
make clean all
make clean all
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player 
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
make clean all
make clean all
make clean all
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player 
./bin/master -v ./bin/view -p ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player -d 10
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -d 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -d 10 -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -d 10 -w 30 -h 30
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
make clean all
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player   -d 10 
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player   -d 10 
./bin/master -v ./bin/view -p ./bin/player -d 10 
make clean all
./bin/master -v ./bin/view -p ./bin/player -d 10 
make clean all
./bin/master -v ./bin/view -p ./bin/player -d 10 
./bin/master -v ./bin/view -p ./bin/player -d 10 
./bin/master -v ./bin/view -p ./bin/player -d 10 
./bin/master -v ./bin/view -p ./bin/player -d 10 
./bin/master -v ./bin/view -p ./bin/player -d 10 
./bin/master -v ./bin/view -p ./bin/player ./bin/player  -d 10 
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 10 -w 20 -h 20
clear
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 1 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
make clean all
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
make clean
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 5 -w 20 -h 20
clear
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 20  -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  -d 20  -w 20 -h 20
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 20  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 20  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 10 -h 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 10 -h 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 10 -h 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1 -w 10 -h 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 5  -w 10 -h 10
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 100 -h 100
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 50 -h 50
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player -d 1  -w 50 -h 50
exit
type keyword
dcstop
exit
ls
dcrun
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 100 -h 100
make clean all
make clean all
make clean all
make clean all
make pvs-test
ls
nvim report.tasks 
vim report.tasks 
make pvs-test
vim report.tasks 
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 100 -h 100
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player -d 1  -w 30 -h 30
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
exit
make clean all
exit
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes -q --log-file="valgrind_no_view_%p.log" ./bin/master -p ./bin/player
make valgrind-test
grep -H 'LEAK SUMMARY' valgrind_*.log
make valgrind-extensive
grep -H 'LEAK SUMMARY' valgrind_*.log
make valgrind-extensive
make clean 
exit
make clean all
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player ./bin/player ./bin/player2 -d 1  -w 30 -h 30 -t 3
./bin/master -v ./bin/view -p ./bin/player ./bin/player  -d 400  -w 10 -h 10 
./bin/master -v ./bin/view -p ./bin/player ./bin/player  ./bin/player -d 400  -w 10 -h 10 
./bin/master -v ./bin/view -p ./bin/player ./bin/player  ./bin/player -d 400  -w 10 -h 10 
make clean all
./bin/master -v ./bin/view -p ./bin/player ./bin/player  ./bin/player -d 400  -w 10 -h 10 
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player  ./bin/player -d 400  -w 10 -h 10 
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player  ./bin/player2  -d 40  -w 10 -h 10 
make clean all
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player2  -d 40  -w 10 -h 10 
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player2  -d 40  -w 10 -h 10 
make clean all
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player2  -d 40  -w 10 -h 10 
./master/ChompChamps -v ./bin/view -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player  ./bin/player2  -d 40  -w 10 -h 10 
exit
make clean
make all
./bin/master -v ./bin/view -w 20 -h 20 -d 300 -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 100 -p ./bin/player ./bin/player ./bin/player2
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 20 -p ./bin/player ./bin/player ./bin/player2
make run
make clean all
make run d=20
make run d=20
make clean all
make run d=20
make run d=20
make run d=20
make run d=20
make run d=20
make run d=20
make run d=100
make run d=300
make run d=300
make run d=300
make run d=300
make run d=300
make clean all
echo $TERM
exit
make clean all
make run d=3
make run d=3
make run d=3
make clean all
make run d=3
make run d=3
make clean all
make run d=3
make run d=3
make clean all
make run d=3
make run d=3
make run d=3
make run d=3
make run d=3
make run d=3
make run d=3
make clean all
make run d=3
make clean all
make run d=3
echo $TERM
exit
make clean all
make run d=3
make clean all

./bin/master -v ./bin/view -w 20 -h 20 -d 20 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
make clean all
make clean all
make clean all
./bin/master -v ./bin/view -w 20 -h 20 -d 70 -p ./bin/player ./bin/player ./bin/player2 ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player2
make clean
exit

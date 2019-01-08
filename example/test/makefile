test:test.cpp
	g++ test.cpp -o test -lnotifex



# 将生成的动态库文件复制到/usr/local/lib/
cp:libnotifex header

libnotifex:
	sudo cp ../../cmake-build-debug/libnotifex.so /usr/local/lib/

header:
	cp ../../notifex/*.h ./

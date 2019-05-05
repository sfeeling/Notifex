test:test.cpp
	g++ test.cpp -o test -lnotifex -pthread -lglog -I/usr/local/include/notifex


# 将生成的动态库文件复制到/usr/local/lib/
cp:libnotifex header

libnotifex:
	sudo cp ../../cmake-build-debug/libnotifex.so /usr/local/lib/

header:
	sudo rm -r /usr/local/include/notifex
	sudo mkdir /usr/local/include/notifex
	sudo cp ../../notifex/*.h /usr/local/include/notifex

# 运行test
run:test test_file

test_file:
	./test

http:http.cpp poem.html
	g++ http.cpp -o http -lnotifex -pthread -lglog
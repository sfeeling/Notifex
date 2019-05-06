echo:echo/echo

echo/echo:echo/echo.cpp
	g++ echo/echo.cpp -o echo/echo -lnotifex -pthread -lglog -I/usr/local/include


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

http:http/http

http/http:http/http.cpp http/poem.html
	g++ http/http.cpp -o http/http -lnotifex -pthread -lglog -I/usr/local/include
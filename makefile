main:
	g++ -o main Main.cpp TCP_Server.cpp c_conf.cpp server_string.cpp -lpthread -g
clean:
	rm -f main
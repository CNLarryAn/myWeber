main:
	g++ -o main Main.cpp TCP_Server.cpp c_conf.cpp -lpthread
clean:
	rm -f main
all:
	g++ -O3 VideoStream.cpp Socket.cpp DebugUtility.cpp server.cpp -o server `pkg-config --cflags --libs opencv` -lGL -lGLU -lglui -L/usr/lib/x86_64-linux-gnu -lglut -lboost_thread -lboost_system -lboost_date_time -lboost_regex

#g++ dnp3master.cpp -o blah -std=c++11 /usr/local/lib/libopendnp3.so /usr/local/lib/libopenpal.so /usr/local/lib/libasiodnp3.so /usr/local/lib/libasiopal.so -pthread

CC=g++
CFLAGS=-pthread -std=c++11 
TARGET = dnp3gen
LIBS= /usr/local/lib/libopendnp3.so /usr/local/lib/libopenpal.so /usr/local/lib/libasiodnp3.so /usr/local/lib/libasiopal.so

all: $(TARGET)


$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp  $(CFLAGS) -o $(TARGET) $(LIBS) 

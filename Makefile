all: 
	g++ tracker1.cpp -o tracker1
	g++ client.cpp -o client -lcrypto -pthread
connectionpool:test.cc ConnectionPool.cc Connection.cc
	g++ -o $@ $^ -std=c++11 -L /usr/lib64/mysql -lmysqlclient -lpthread

.PHONY:clean
clean:
	rm -rf connectionpool
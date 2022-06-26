CXX = g++
CXXFLAGS = -Wall -Wno-unknown-pragmas -std=c++14 -s -Ofast -pthread -Ijwt-include
LIBS = -lodb-mysql -lodb -lcrypto
TARGET = server

$(TARGET): main.cpp persistence-odb.cxx persistence-odb.hxx
	$(CXX) $< persistence-odb.cxx $(CXXFLAGS) $(LIBS) -o $@

persistence-odb.cxx: persistence.hpp
	odb -d mysql --generate-query --generate-schema --generate-session --default-pointer std::shared_ptr --std c++11 $<

.PHONY: clean

clean:
	rm -rf $(TARGET)
	rm -rf persistence-odb.cxx
	rm -rf persistence.sql
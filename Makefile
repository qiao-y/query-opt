all: query.cpp query.h
	g++ -Wall query.cpp -o query

clean:
	rm -f query 
	

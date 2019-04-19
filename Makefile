TSP:
	mpicxx -DMPICH_SKIP_MPICXX -o ptsp tsp.cpp
test:
	mpicxx -DMPICH_SKIP_MPICXX -o test test.cpp
clean:
	rm -f ptsp
	rm -f test
	rm -f output
	rm -f error

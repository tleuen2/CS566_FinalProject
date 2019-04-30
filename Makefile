PTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp parallel_tsp.cpp
TSP:
	mpicxx -DMPICH_SKIP_MPICXX -o ptsp tsp.cpp
test:
	mpicxx -DMPICH_SKIP_MPICXX -o test test.cpp
clean:
	rm -f ptsp
	rm -f test
	rm -f output
	rm -f error
	rm parallel_tsp
	rm test_1.out
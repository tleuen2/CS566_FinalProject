PTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp_6 parallel_tsp_6_Input6.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp_8 parallel_tsp_8_Input8.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp_15 parallel_tsp_15_Input15.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp_16 parallel_tsp_16_Input16.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o parallel_tsp_28 parallel_tsp_28_Input28.cpp
TSP:
	mpicxx -DMPICH_SKIP_MPICXX -o ptsp tsp.cpp
test:
	mpicxx -DMPICH_SKIP_MPICXX -o test working_parallel.cpp
clean:
	rm -f ptsp
	rm -f test
	rm -f output
	rm -f error
	rm parallel_tsp
	rm test_1.out
	rm parallel_tsp_6 parallel_tsp_8 parallel_tsp_4 parallel_tsp_15 parallel_tsp 16
	rm parallel_tsp_28
QEPTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_6 QE_ParallelTSP_6.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_8 QE_ParallelTSP_8.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_15 QE_ParallelTSP_15.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_16 QE_ParallelTSP_16.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_28 QE_ParallelTSP_28.cpp
TSP:
	mpicxx -DMPICH_SKIP_MPICXX -o ptsp tsp.cpp
test:
	mpicxx -DMPICH_SKIP_MPICXX -o test working_parallel.cpp
clean:
	rm -f ptsp
	rm -f test
	rm -f output
	rm -f error
	rm test*
	rm Homework3*
	rm QE_Parallel_TSP_6 QE_Parallel_TSP_8 QE_Parallel_TSP_15 QE_Parallel_TSP_16
	rm QE_Parallel_TSP_28
	rm Node_Output*
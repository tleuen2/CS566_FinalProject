QEPTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_6 QE_ParallelTSP_6.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_8 QE_ParallelTSP_8.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_15 QE_ParallelTSP_15.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_16 QE_ParallelTSP_16.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_28 QE_ParallelTSP_28.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o QE_Parallel_TSP_50 QE_ParallelTSP_50.cpp
	
IQEPTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o Improved_QE_28 Improved_QE_28.cpp
	
FinalProject:
	mpicxx -DMPICH_SKIP_MPICXX -o FinalProject final_project_28Input.cpp
	
PTSP:
	mpicxx -DMPICH_SKIP_MPICXX -o Parallel_TSP_6 ParallelTSP_6.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o Parallel_TSP_8 ParallelTSP_8.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o Parallel_TSP_15 ParallelTSP_15.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o Parallel_TSP_16 ParallelTSP_16.cpp
	mpicxx -DMPICH_SKIP_MPICXX -o Parallel_TSP_28 ParallelTSP_28.cpp
	
TSP:
	mpicxx -DMPICH_SKIP_MPICXX -o ptsp tsp.cpp
test:
	mpicxx -DMPICH_SKIP_MPICXX -o test working_parallel.cpp
clean:
	rm -f ptsp
	rm -f test
	rm -f output
	rm -f error
	rm -f test*
	rm -f Homework3*
	rm -f QE_Parallel_TSP_6 QE_Parallel_TSP_8 QE_Parallel_TSP_15 QE_Parallel_TSP_16
	rm -f QE_Parallel_TSP_28
	rm -f Node_Output* Improved_QE_28
#include <iostream>
#include <algorithm>
#include <vector>
#include <bits/stdc++.h>
#include <functional>
#include <random>
#include <parmetis.h>
#include <mpi.h>

void adjtocsr(int** xadj,
              int** adjncy,
              int** vtxdist,
              real_t** xyz,
              int* nvtxs,
              int* nedges,
              const std::string& graph_file_path,
              const std::string& coord_file_path,
              MPI_Comm* comm);

int main(int argc, char* argv[]) {
    MPI_Comm comm = MPI_COMM_WORLD;
    int pid;
    MPI_Init(&argc, &argv);
    idx_t* adjncy;
    idx_t* xadj;
    idx_t* vtxdist;
    idx_t* adjwgt;
    real_t* xyz;
    idx_t nvtxs;
    idx_t nedges;
    idx_t wgtflag=0, numflag =0 , ncon = 1, nparts = 2;
    idx_t options[] = {0};
    idx_t edgecut=0;
    idx_t* part;
    idx_t ndims=3;
    auto tpwgts = new real_t[ncon*nparts];
    for(int i=0; i<ncon*nparts; ++i){
        tpwgts[i] = 1/((float_t )nparts);
    }
    auto ubvec = new real_t[ncon];
    for(int i=0; i<ncon; ++i){
        ubvec[i] = 1.05;
    }
    const std::string adj_file_path(argv[1]);
    const std::string coord_file_path(argv[2]);
    adjtocsr(&xadj, &adjncy, &vtxdist, &xyz, &nvtxs, &nedges, adj_file_path, coord_file_path , &comm);
    MPI_Comm_rank(comm, &pid);
    std::cout<<"Processor: "<<pid<<std::endl;
    std::cout<<"Vertices: "<<nvtxs<<std::endl;
    std::cout<<"Edges: "<<nedges<<std::endl;
    std::cout<<std::endl;
    MPI_Barrier(comm);
    //std::random_device r;
    //std::seed_seq      seed{r(), r(), r(), r(), r(), r(), r(), r()};
    //std::mt19937       eng(seed); // a source of random data
    //std::uniform_int_distribution<int> dist(1,10);
    //std::vector<int> v(nedges);
    //generate(v.begin(), v.end(), bind(dist, eng));
    //adjwgt = new int[nedges];
    //std::copy(v.begin(), v.end(), adjwgt);
    //for (int i=0; i<nedges; ++i){
      //adjwgt[i] = 1;
    //}
    part = new idx_t[nvtxs];
    int rval = ParMETIS_V3_PartGeomKway(vtxdist, xadj, adjncy, NULL, NULL, &wgtflag, &numflag, &ndims, xyz, &ncon, &nparts, tpwgts, ubvec,
                                options, &edgecut, part, &comm);
    std::cout<<rval<<std::endl;
    if(rval==METIS_OK){
        std::cout<<edgecut<<std::endl;
        std::cout<<"OK!"<<std::endl;
    }
    MPI_Finalize();
    return 0;
}
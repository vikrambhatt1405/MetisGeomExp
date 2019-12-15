#include <string>
#include <fstream>
#include <parmetis.h>
#include <iostream>
#include <vector>
#include <mpi.h>

void adjtocsr(int** xadj,
              int** adjncy,
              int** vtxdist,
              real_t** xyz,
              int* nvtxs,
              int* nedges,
              const std::string& graph_file_path,
              const std::string& coord_file_path,
              MPI_Comm *comm){
    int id;
    int n_proc;
    int adjncy_size, xadj_size;
    int total_vtxs, total_edges;
    std::vector<int> _xadj = {0};
    std::vector<int> _adjncy;
    std::vector<real_t> _xyz;
    MPI_Comm_rank(*comm, &id);
    MPI_Comm_size(*comm, &n_proc);
    if(id ==  n_proc-1) {
        std::ifstream adj_file, coord_file;
        adj_file.open(graph_file_path, std::ios::in);
        coord_file.open(coord_file_path, std::ios::in);
        if (!adj_file || !coord_file)  {
            std::cout << "Input graph or coordinate file not found" << std::endl;
            MPI_Abort(*comm, 1);
        }
        std::string s, p;
        while(getline(adj_file, s)){
            if(s.rfind("%")!=0) break;
        }
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            total_vtxs = stoi(token);
            s.erase(0, pos + delimiter.length());
        }
        total_edges = stoi(s);
        getchar();
        MPI_Bcast(&total_vtxs, 1, MPI_INT, n_proc - 1, *comm);
        MPI_Bcast(&total_edges, 1, MPI_INT, n_proc - 1, *comm);
        for (int process = 0; process < n_proc - 1; ++process) {
            int vtxs_read=0;
            _xadj = {0};
                while(vtxs_read<total_vtxs/n_proc){
                    getline(adj_file, s);
                    if(s.empty()){
                        //
                        // std::cout<<"Fucked"<<std::endl;
                        while(s.empty()){
                         getline(adj_file, s);
                        }
                    }
                    getline(coord_file, p);
                    std::string delim = " ";
                    auto start = 0U;
                    auto end = s.find(delim);
                    while (end != std::string::npos)
                    {
                        _adjncy.push_back(std::stoi(s.substr(start, end-start))-1);
                        start = end + delim.length();
                        end = s.find(delim, start);
                    }
                    //std::cout<<s.substr(start, end-start)<<std::endl;
                    _adjncy.push_back(std::stoi(s.substr(start, end-start))-1);
                    start = 0U;
                    end = p.find(delim);
                    while (end != std::string::npos)
                    {
                        //std::cout<< std::stof(p.substr(start, end-start)) <<std::endl;
                        _xyz.push_back(std::stof(p.substr(start, end-start)));
                        start = end + delim.length();
                        end = p.find(delim, start);
                    }
                    _xyz.push_back(std::stof(p.substr(start, end-start)));
                    vtxs_read+=1;
                    _xadj.push_back(_adjncy.size());
                }
                adjncy_size = _adjncy.size();
                xadj_size = _xadj.size();
                MPI_Send(&adjncy_size, 1, MPI_INT, process, process+8, *comm);
                MPI_Send(&xadj_size, 1, MPI_INT, process, process+16, *comm);
                MPI_Send(_adjncy.data(), _adjncy.size(), MPI_INT, process, process+32, *comm);
                MPI_Send(_xadj.data(), _xadj.size(), MPI_INT, process, process+64, *comm);
                std::cout<<_xyz.size()<<std::endl;
                MPI_Send(_xyz.data(), _xyz.size(), MPI_FLOAT, process, process+128, *comm);
                _xadj.clear();
                _adjncy.clear();
                _xyz.clear();
        } // Data distributed for all other process at this point
        // Now assign the rest of data to this process n_proc-1
        _xadj = {0};
        _adjncy.clear();
        _xyz.clear();
        while (getline(adj_file, s) && getline(coord_file, p)){
            if(s.empty()){
                while(s.empty()){
                    getline(adj_file,s);
                }
            }

                std::string delim = " ";
                auto start = 0U;
                auto end = s.find(delim);
                while (end != std::string::npos)
                {
                    _adjncy.push_back(std::stoi(s.substr(start, end-start))-1);
                    start = end + delim.length();
                    end = s.find(delim, start);
                }

            _adjncy.push_back(std::stoi(s.substr(start, end-start))-1);
                start = 0U;
                end = p.find(delim);
                while (end != std::string::npos){
                    _xyz.push_back(std::stof(p.substr(start, end-start)));
                    start = end + delim.length();
                    end = p.find(delim, start);
                }
                _xyz.push_back(std::stof(p.substr(start, end-start)));
            _xadj.push_back(_adjncy.size());
            }
        *nvtxs = _xadj.size()-1;
        *nedges = _adjncy.size();
        *xadj = new int[_xadj.size()];
        *adjncy = new int[_adjncy.size()];
        *xyz = new real_t[3*(*nvtxs)];
        std::copy(_xadj.begin(), _xadj.end(), *xadj);
        std::copy(_adjncy.begin(), _adjncy.end(), *adjncy);
        std::cout<<_xyz.size()<<std::endl;
        std::copy(_xyz.begin(), _xyz.end(), *xyz);
        adj_file.close();
        coord_file.close();
    }else{
        MPI_Status status[5];
        MPI_Bcast(&total_vtxs, 1, MPI_INT, n_proc-1, *comm);
        MPI_Bcast(&total_edges, 1, MPI_INT, n_proc - 1, *comm);
        MPI_Recv(&adjncy_size, 1, MPI_INT, n_proc-1, id+8, *comm, &status[0]);
        MPI_Recv(&xadj_size, 1, MPI_INT, n_proc-1, id+16, *comm, &status[1]);
        *nvtxs = xadj_size-1;
        *nedges = adjncy_size;
        *adjncy = new int[adjncy_size];
        *xadj = new int[xadj_size];
        *xyz = new real_t[3*(*nvtxs)];
        MPI_Recv(*adjncy, adjncy_size, MPI_INT, n_proc-1, id+32, *comm, &status[2]);
        MPI_Recv(*xadj, xadj_size, MPI_INT, n_proc-1, id+64, *comm, &status[3]);
        MPI_Recv(*xyz, 3*(*nvtxs), MPI_FLOAT, n_proc-1, id+128, *comm, &status[4]);
    }
    //Define vtxdist for all processes
    *vtxdist = new int[n_proc+1]();
    for(int i=1; i<n_proc; ++i) {
        (*vtxdist)[i] = i*(total_vtxs/n_proc);
    }
    (*vtxdist)[n_proc] =  total_vtxs;
}

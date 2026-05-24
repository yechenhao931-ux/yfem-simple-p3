// Serial-only compatibility shim. No MPI implementation is included.
#ifndef MFEM_COMMUNICATION
#define MFEM_COMMUNICATION

namespace mfem
{

class Mpi
{
public:
   static void Init(int * = nullptr, char *** = nullptr, int = 0,
                    int * = nullptr) { }
   static void Finalize() { }
   static bool IsInitialized() { return false; }
   static bool IsFinalized() { return true; }
   static int WorldRank() { return 0; }
   static int WorldSize() { return 1; }
   static bool Root() { return true; }
};

class MPI_Session
{
public:
   MPI_Session() { }
   MPI_Session(int &, char **&) { }
   int WorldRank() const { return 0; }
   int WorldSize() const { return 1; }
   bool Root() const { return true; }
};

} // namespace mfem

#endif

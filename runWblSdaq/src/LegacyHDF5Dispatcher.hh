// ejc

#ifndef LegacyHDF5Dispatcher_hh
#define LegacyHDF5Dispatcher_hh

#include <iostream>
#include <string>
#include <time.h>
#include <H5Cpp.h>
#include <Digitizer.hh>
#include <Dispatcher.hh>
using namespace std;
using namespace H5;

class LegacyHDF5Dispatcher: public Dispatcher{
  public:
    LegacyHDF5Dispatcher();
    LegacyHDF5Dispatcher(size_t _nEvents,
                         string _basename,
                         vector<Decoder*> _decoders);
   ~LegacyHDF5Dispatcher();

    string NextPath();
    size_t Digest(vector<Buffer*>& buffers);
    void Dispatch(vector<Buffer*>& buffers);
  protected:
    string basename;
    vector<Decoder*> decoders;
    void Initialize(H5File& file);
};

#endif

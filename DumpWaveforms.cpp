#include "DumpWaveforms.h"

#include <iterator>
#include <sstream>
#include <iterator> // distance
#include "ADCTrace.h"

DumpWaveforms::DumpWaveforms():Tool(){}


bool DumpWaveforms::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  savedwaves = 0;
  m_variables.Get("OutputFile",outputfile);
  outf.open(outputfile.c_str(), std::fstream::out | std::fstream::binary | std::fstream::trunc );
  if (!m_variables.Get("MaxWaves",maxwaves)) maxwaves = 10000;
  if (!m_variables.Get("WaveLength",wavelength)) wavelength = 1000;
  if (!m_variables.Get("BlackHole",blackhole)) blackhole = 0;
  traces = new std::multimap<uint64_t,ADCTrace>();
  m_data->CStore.Get("ADCTraces",traces); 
  Log("DumpWaveforms loaded");
  return true;
}


bool DumpWaveforms::Execute(){
  if (traces) {
    if (!blackhole) {traces->clear(); return true;}
    Log("Dumping traces");
    cout << "dt: Traces now holds: " << traces->size() << endl;
    auto start = traces->begin();
    auto end = traces->end();
    if (std::distance(start,end) > (maxwaves-savedwaves)) {
      end = traces->begin();
      advance(end,maxwaves-savedwaves);
    }
    savedwaves += std::distance(start,end);
    cout << "  Dumping " << savedwaves << " traces" << endl;
    for (auto waveit = start ; waveit != end ; ++waveit) {
      cout << waveit->second.Crate  << " " << waveit->second.Card << " " << waveit->second.Channel << " " << waveit->second.Start <<endl;
      outf.write((char*)&(waveit->second.Crate),sizeof(int));
      outf.write((char*)&(waveit->second.Card),sizeof(int));
      outf.write((char*)&(waveit->second.Channel),sizeof(int));
      outf.write((char*)&(waveit->second.Start),sizeof(uint64_t));
      std::vector<uint16_t> tempvec(waveit->second.Samples);
      int errpos = 0;
      for (auto sit = tempvec.begin(); sit!=tempvec.end(); ++sit) {
        if (*sit > 0xF000) {
          errpos = std::distance(tempvec.begin(),sit);
          break;
        }
      }
      outf.write((char*)&errpos,sizeof(int));
      if (tempvec.size() != wavelength) tempvec.resize(wavelength);
      outf.write((char*)tempvec.data(),tempvec.size()*sizeof(uint16_t));
    }
    cout << endl;
    traces->erase(start,end);
    if (savedwaves >= maxwaves) m_data->vars.Set("StopLoop",1);
  }
  return true;
}


bool DumpWaveforms::Finalise(){
  outf.close();
  return true;
}

#include "DumpWaveforms.h"

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
  traces = new std::multimap<uint64_t,ADCTrace>();
  m_data->CStore.Get("ADCTraces",traces); 
  Log("DumpWaveforms loaded");
  return true;
}


bool DumpWaveforms::Execute(){
  if (traces) {
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
      outf << waveit->second.Crate << waveit->second.Card << waveit->second.Channel << waveit->second.Start;
      std::vector<uint16_t> tempvec(waveit->second.Samples);
      if (tempvec.size() != wavelength) tempvec.resize(wavelength);
      for (const auto &s : tempvec) outf << s;
    }
    traces->erase(start,end);
    if (savedwaves >= maxwaves) m_data->vars.Set("StopLoop",1);
  }
  return true;
}


bool DumpWaveforms::Finalise(){
  outf.close();
  return true;
}

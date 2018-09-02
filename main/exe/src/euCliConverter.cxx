#include "eudaq/OptionParser.hh"
#include "eudaq/DataConverter.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/FileReader.hh"
#include "eudaq/StdEventConverter.hh"

#include <iostream>

int main(int /*argc*/, const char **argv) {
  eudaq::OptionParser op("EUDAQ Command Line DataConverter", "2.0", "The Data Converter launcher of EUDAQ");
  eudaq::Option<std::string> file_input(op, "i", "input", "", "string",
					"input file");
  eudaq::Option<std::string> file_output(op, "o", "output", "", "string",
					 "output file");
  eudaq::OptionFlag iprint(op, "ip", "iprint", "enable print of input Event");

  try{
    op.Parse(argv);
  }
  catch (...) {
    return op.HandleMainException();
  }
  
  std::string infile_path = file_input.Value();
  if(infile_path.empty()){
    std::cout<<"option --help to get help"<<std::endl;
    return 1;
  }
  
  std::string outfile_path = file_output.Value();
  std::string type_in = infile_path.substr(infile_path.find_last_of(".")+1);
  std::string type_out = outfile_path.substr(outfile_path.find_last_of(".")+1);
  bool print_ev_in = iprint.Value();
  
  if(type_in=="raw")
    type_in = "native";
  if(type_out=="raw")
    type_out = "native";
  
  eudaq::FileReaderUP reader;
  eudaq::FileWriterUP writer;
  reader = eudaq::Factory<eudaq::FileReader>::MakeUnique(eudaq::str2hash(type_in), infile_path);
  if(!type_out.empty())
    writer = eudaq::Factory<eudaq::FileWriter>::MakeUnique(eudaq::str2hash(type_out), outfile_path);
  while(1){
    auto ev = reader->GetNextEvent();
    if(!ev)
      break;

    auto stdev = eudaq::StandardEvent::MakeShared();
    eudaq::StdEventConverter::Convert(ev, stdev, nullptr); //no conf
    size_t hits_num_jade = 0;
    size_t hits_num_m26 = 0;
    size_t num = stdev->NumPlanes();
    bool nohit = false;
    for (unsigned int i = 0; i < num;i++){
      const eudaq::StandardPlane & plane = stdev->GetPlane(i);
      if(plane.Sensor() == "Jade"){
	hits_num_jade += plane.HitPixels();
      }
      if(plane.Sensor() == "MIMOSA26"){
	hits_num_m26 =plane.HitPixels();
        if(hits_num_m26 == 0) nohit=true;
        //std::cout<<"hit m26 is "<<hits_num_m26<<std::endl;
      }
    }

    if(!hits_num_jade){
      continue;
    }

    if(nohit) continue;

    if(ev->GetEventN()%10000==0)
      ev->Print(std::cout);

    if(print_ev_in)
      ev->Print(std::cout);
    if(writer)
      writer->WriteEvent(ev);
  }
  return 0;
}

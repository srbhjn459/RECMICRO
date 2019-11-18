#include "reconfiguration.hpp"


extern std::string skl_graph_file;
extern std::string skl_graph_node_file;
extern std::string reg_to_be_bypassed_file;
extern std::string register_level_file;
extern std::string dummy_nodes_file;
extern vector <graph_node*>dummy_nodes;
extern vector<std::string> input_ports;
extern vector<std::string> output_ports;
extern vector<std::string> flipflop_array;
extern bool from_netlist;
extern std::string ip_port_file;
extern std::string op_port_file;
extern std::string register_file;
extern std::string nl_graph_file;
extern nl_graph_iter nl_graph;

int main()
{
    if (!from_netlist)
    {
      std::ifstream ifp(skl_graph_file.c_str());
      std::ifstream ifp_1(skl_graph_node_file.c_str());
      std::ifstream ifp_2(dummy_nodes_file.c_str());
      cout << "Rebuilding the skeleton graph...."<<endl;
      rebuild_skl_graph(ifp,ifp_1);
      cout << "Rebuilding the dummy node vector...."<<endl;
      rebuild_dummy_node_vector(ifp_2);
      //print_skl_graph_node_vector(dummy_nodes);
      //print_skl_graph(0);
      graph_node* node = get_skl_graph_node_v2("dummy","","dummy",0);
      cout << "level labelling of all the nodes in design...."<<endl;
      ////print_skl_graph_node(node);
      ////cout << endl;
      int start = getMilliCount();
      Traverse_to_create_reg_level_linear(node,0);
      int milliSecondsElapsed = getMilliSpan(start);
      cout << "Total time for lavel labelling of all nodes: " << (milliSecondsElapsed/1000.0) << "s" << endl;
      cout << "printing bypassable registers to file: "<<reg_to_be_bypassed_file<<endl;
      ofstream fout(reg_to_be_bypassed_file.c_str(), ios::binary);
      write_bypaassable_reg_to_file(fout,true);
      cout << "printing levels to a file: "<<register_level_file<<endl; 
      ofstream fout1(register_level_file.c_str(), ios::binary);
      print_reg_level_hash_to_file(fout1);
    }
    else
    {
        std::string line;
        std::ifstream ifp(ip_port_file.c_str());
        //input_ports vector
        while(std::getline(ifp,line))
        {
          input_ports.push_back(line);
        }
        ifp.close();
        std::ifstream ifp_1(op_port_file.c_str());
        //output_ports vector
        while(std::getline(ifp_1,line))
        {
           output_ports.push_back(line);
        }
        ifp_1.close();
        std::ifstream ifp_2(register_file.c_str());
        //all flops vector
        while(std::getline(ifp_2,line))
        {
           flipflop_array.push_back(line);
        }
        ifp_2.close();
        //netlist graph  
        std::ifstream ifp_3(nl_graph_file.c_str());
        cout << "Rebuilding the nl graph...."<<endl;
        rebuild_nl_graph(ifp_3);
        for (int i = 0; i<input_ports.size();i++)
        {
          vector <std::string> temp_vector;
          temp_vector.push_back("D");
          temp_vector.push_back(input_ports[i]);
          temp_vector.push_back("DUMMY_FF");
          temp_vector.push_back("Q");
          temp_vector.push_back("nv");
          nl_graph["dummy"][input_ports[i]] = temp_vector; 
        }
        Traverse_to_create_reg_level_linear_nl_graph("dummy",0);
        ofstream fout(reg_to_be_bypassed_file.c_str(), ios::binary);
        cout << "printing bypassable registers to file: "<<reg_to_be_bypassed_file<<endl;
        write_bypaassable_reg_to_file(fout,true);
        cout << "printing levels to a file: "<<register_level_file<<endl; 
        ofstream fout1(register_level_file.c_str(), ios::binary);
        print_reg_level_hash_to_file(fout1);


        //ofstream fout1(register_level_file.c_str(), ios::binary);
        //print_reg_level_hash_to_file(fout1);
    }
}

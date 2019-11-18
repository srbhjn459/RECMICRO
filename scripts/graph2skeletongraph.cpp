#include "reconfiguration.hpp"

///input files/parameters


///output files/parameters
extern std::string ip_port_file;
extern std::string op_port_file;
extern std::string register_file;
extern std::string nl_graph_file;
extern std::string skl_graph_file;
extern std::string skl_graph_node_file;
extern std::string dummy_nodes_file;
extern std::string register_node_file;
extern vector<std::string> input_ports;
extern vector<std::string> output_ports;
extern vector<std::string> flipflop_array;
extern nl_graph_iter nl_graph;
extern reg_graph skl_graph;
extern vector <graph_node*>skl_graph_nodes;
extern vector <graph_node*>register_graph_nodes;

int main ()
{

    std::string line;
    std::ifstream ifp(ip_port_file.c_str());
    //input_ports vector
    cout << "preparing database...."<<endl;
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
    ////add dummy flop connecting to all inputs with mreg_input_w = '': mreg = 'dummy' :mreg_output_w = 'dummy';
    vector <std::string> temp_vector;
    temp_vector.push_back("D");
    temp_vector.push_back("dummy");
    temp_vector.push_back("DUMMY_FF");
    temp_vector.push_back("Q");
    temp_vector.push_back("nv");
    nl_graph[""]["dummy"] = temp_vector; 
    //print_nl_graph();
    //creating all the nodes (consisting of flip-flops) including output/input and dummy nodes
    cout << "creating node database...."<<endl;
    int start = getMilliCount();
    create_skl_graph_node_vector();
    int milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for creating skl node database: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    cout << "creating skl graph...."<<endl;
    start = getMilliCount();
    for (int i = 0 ; i < skl_graph_nodes.size() ;i++)
    {
      //cout << "Finding connectivity of node: ";
      //print_skl_graph_node(skl_graph_nodes[i]);
      cout <<"completed (" << (i+1)*100/double(skl_graph_nodes.size()) << "%)"<<endl;
      Traverse_to_create_skl_graph(skl_graph_nodes[i]->get_reg_output_w(),skl_graph_nodes[i]);
      mark_all_edges_in_nl_graph();
      //print_skl_graph(0);
    }
    //print_skl_graph(0);
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for creating skl graph: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    //print_skl_graph_nodes(0);
    //graph_node* node = new graph_node();
    //Traverse_to_create_skl_graph("",node);
    //////add dummy flop connected from all outputs with mreg_input_w = '': mreg = "dummy_out" :mreg_output_w = "dummy_out";
    put_dummy_output_node();
    //print_skl_graph(0);
    ofstream fout((skl_graph_file).c_str(), ios::binary);
    cout << "printed skl graph to file: "<<skl_graph_file<<endl;
    print_skl_graph_to_file(fout);
    cout << "printing all the skl graph nodes to file: "<<skl_graph_node_file<<endl;
    ofstream fout_v2((skl_graph_node_file).c_str(), ios::binary);
    print_graph_nodes_in_file(fout_v2);
    cout << "printing all the dummy skl graph nodes to file: "<<dummy_nodes_file<<endl;
    ofstream fout_v3((dummy_nodes_file).c_str(), ios::binary);
    print_dummy_graph_nodes_in_file(fout_v3);
    cout << "printing all the register graph nodes to file: "<<register_node_file<< endl;
    ofstream fout_v4((register_node_file).c_str(), ios::binary);
    print_register_nodes_in_file(fout_v4);
}



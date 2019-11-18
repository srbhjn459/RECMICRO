#include "reconfiguration.hpp"


extern std::string skl_graph_file;
extern std::string skl_graph_node_file;
extern std::string set_of_cutset_file;
extern std::string loops_file;
//extern std::string upstream_node_file;
//extern std::string downstream_node_file;
//extern std::string ip_node_reach_file;
//extern std::string ip_port_file;
extern std::string dummy_nodes_file;
extern std::string register_node_file;
extern std::string reg_to_be_bypassed_file;
extern std::string reg_not_to_be_bypassed_file;
extern reg_graph rev_graph;
extern vector <reg_graph_edge*> edges_in_loop;
extern vector <graph_node*> nodes_in_loop;
extern vector <vector <graph_node*> > loop_groups;
extern vector <graph_node*> curr_loop_group;
extern vector <graph_node*> skl_graph_nodes;
extern reachable_graph reachability_hash;
extern map <std::string,graph_node*>skl_graph_nodes_v2;
extern type_of_cutsets cut_type;
//extern int no_of_edges = 0;

int main()
{
    std::ifstream ifp(skl_graph_file.c_str());
    std::ifstream ifp_1(skl_graph_node_file.c_str());
    std::ifstream ifp_dummy(dummy_nodes_file.c_str());
    std::ifstream ifp_reg(register_node_file.c_str());
    //vector<graph_node*>path_graph_node;
    //reconstruct the skl_graph from text file, after weighing edges
    cout << "Rebuilding the skeleton graph...."<<endl;
    rebuild_skl_graph(ifp,ifp_1);
    cout << "Rebuilding the dummy node vector...."<<endl;
    rebuild_dummy_node_vector(ifp_dummy);
    cout << "Rebuilding the register graph node vector...."<<endl;
    rebuild_register_graph_nodes(ifp_reg);
    //print_skl_graph(0);
    mark_all_skl_graph_nodes(false,0);
    mark_all_skl_graph_edges(false,0);
    ifp.close();
    ifp_1.close();
    graph_node* node;
    //node = get_skl_graph_node("dummy","","dummy",0);
    node = skl_graph_nodes_v2[convert_graph_node_el_2_str("","dummy","dummy")];
    mark_all_skl_graph_nodes(false,0);
    mark_all_skl_graph_edges(false,0);
    stack<graph_node*>*path_graph_node = new stack<graph_node*>;
    //find loops in design
    cout << "Finding loops in design...."<<endl;
    path_graph_node->push(node);
    int start = getMilliCount();
    find_loops_in_graph(node,path_graph_node);
    int milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for finding loops in design: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    find_nodes_in_loop();
    ofstream fout_loop(loops_file.c_str(), ios::binary);
    print_loops_to_file(fout_loop);
    fout_loop.close();
    cout << "Printing all the loops in design to file "<<loops_file <<endl;
    mark_all_skl_graph_nodes(false,0);
    mark_all_skl_graph_edges(false,0);
    int count = 0;
    //categorize loops in design into different groups
    cout << "Categorizing loop groups in design...."<<endl;
    start = getMilliCount();
    while(count < nodes_in_loop.size())
    {
        if(!nodes_in_loop[count]->get_visited())
        {
            empty_a_vector(&curr_loop_group);
            distribute_loop_nodes_to_grp(nodes_in_loop[count]);
            loop_groups.push_back(curr_loop_group);
        }
        count++;
    }
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for categorizing loops in design: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    cout << "Found "<<loop_groups.size()<<" loops groups in design...."<<endl;
    mark_all_skl_graph_nodes(false,0);
    mark_all_skl_graph_edges(false,0);
    //reduce each loop group as a coglomerated node and create modified skl graph
    create_modified_skl_graph_node_vector();
    create_modified_skl_graph();
    //cout << "Old nodes are:" <<endl;
    //print_skl_graph_nodes(0);
    //cout << "New nodes are:" <<endl;
    //print_skl_graph_nodes(1);
    //cout << "Old graph is:" <<endl;
    //print_skl_graph(0);
    //cout << "New graph is:" <<endl;
    //print_skl_graph(1);
    mark_all_skl_graph_nodes(false,1);
    mark_all_skl_graph_edges(false,1);
    //create a reachability hash for every node
    cout << "Creating reachability hash for design...."<<endl;
    start = getMilliCount();
    form_reachability_graph(node,1);
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for creating reachability hash in design: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    mark_all_skl_graph_nodes(false,1);
    mark_all_skl_graph_edges(false,1);
    cout << "level labelling of all the nodes in design...."<<endl;
    start = getMilliCount();
    Traverse_to_create_reg_level(node,0);
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for lavel labelling of all nodes: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    mark_all_skl_graph_nodes(false,1);
    mark_all_skl_graph_edges(false,1);
    reg_graph_edge* null_edge = new reg_graph_edge();
    null_edge = NULL;
    vector<vector<reg_graph_edge*> > set_of_cutset;
    vector<vector<reg_graph_edge*> > modified_set_of_cutset;
    cout << "Finding all the cutsets in design...."<<endl;
    start = getMilliCount();
    set_of_cutset = set_of_cut_set(node,null_edge,cut_type,1,true);
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for finding all cutsets in design: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    cout << "Converting loop grp nodes to original nodes in cutset...."<<endl;
    start = getMilliCount();
    modified_set_of_cutset = modify_set_of_cut_set(&set_of_cutset);
    milliSecondsElapsed = getMilliSpan(start);
    cout << "Total time for modifying all cutsets in design: " << (milliSecondsElapsed/1000.0) << "s" << endl;
    ofstream fout(set_of_cutset_file.c_str(), ios::binary);
    cout << "Printing all the cutsets to file "<<set_of_cutset_file <<endl;
    print_set_of_cutsets_to_file(fout,modified_set_of_cutset);
    if (cut_type == bypassable)
    {
        vector<int> reward_arr;
        vector<vector<graph_node*> > pipelines;
        map<int,vector<int> > forbidden_pipelines;
        vector<int> pipelines_idx;
        for (int i = 0 ; i < modified_set_of_cutset.size();i++)
        {
            vector<graph_node*> curr_reg = cut_2_reg(modified_set_of_cutset[i]);
            reward_arr.push_back(curr_reg.size());
            pipelines_idx.push_back(i);
            pipelines.push_back(curr_reg);
        }
        pipelines = filter_dummy_pipelines(pipelines);
        for (int i = 0 ; i < pipelines.size() ; i++)
        {
            vector<graph_node*> curr_pipeline = pipelines[i];
            vector<int> temp= get_intersection_index(i,pipelines);
            forbidden_pipelines[i] = temp;
        }
        vector<int> chosen_pipelines_idx = choose_pipelines(pipelines,forbidden_pipelines,reward_arr,0.01,1);
        ofstream fout_reg(reg_to_be_bypassed_file.c_str(), ios::binary);
        cout << "Printing all bypassable registers to file: "<<reg_to_be_bypassed_file <<endl;
        print_bypassable_reg_to_file(fout_reg,pipelines,chosen_pipelines_idx);
        ofstream fout_nb_reg(reg_not_to_be_bypassed_file.c_str(), ios::binary);
        cout << "Printing all non bypasable registers to file: "<<reg_not_to_be_bypassed_file <<endl;
        print_non_bypassable_reg_to_file(fout_nb_reg,pipelines,chosen_pipelines_idx);
    }
}

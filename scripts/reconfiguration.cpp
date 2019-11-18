#include "reconfiguration.hpp"


//input_file to graph2skeletongraph.cpp

std::string nl_graph_file = "./output_dir/graph_netlist.txt";
std::string op_port_file = "./output_dir/op_ports_netlist.txt";
std::string register_file = "./output_dir/registers_netlist.txt";

//output_files of graph2skeletongraph.cpp, input files to identify_pipeline.cpp

//std::string skl_graph_node_file = "/home/a0117963/sandbox19082015/intel_work/journal_data_tvlsi/syn/netlist/skl_nodes_fmult_seq_v1.txt";
std::string skl_graph_node_file = "./output_dir/nodes_netlist.txt";
//std::string skl_graph_file = "/home/a0117963/sandbox19082015/intel_work/journal_data_tvlsi/syn/netlist/skl_graph_fmult_seq_v1.txt";
std::string skl_graph_file = "./output_dir/skl_graph_netlist.txt";
//std::string dummy_nodes_file = "/home/a0117963/sandbox19082015/intel_work/journal_data_tvlsi/syn/netlist/dummy_skl_nodes_fmult_seq_v1.txt";
std::string dummy_nodes_file = "./output_dir/dummy_nodes_netlist.txt";
std::string register_node_file = "./output_dir/register_nodes_netlist.txt";


//input_files of graph2skeletongraph.cpp, input files to identify_pipeline.cpp
//std::string ip_port_file = "/home/a0117963/sandbox19082015/intel_work/journal_data_tvlsi/syn/netlist/ip_ports_fmult_seq_v1.txt";
std::string ip_port_file = "./output_dir/ip_ports_netlist.txt";

//output_files of identify_pipeline.cpp
std::string loops_file = "./output_dir/loops_netlist.txt";
//std::string loops_file = "./random_graphs/loops_4001.txt";
//std::string set_of_cutset_file = "/home/a0117963/sandbox19082015/intel_work/journal_data_tvlsi/syn/netlist/set_of_cuts_fmult_seq_v1.txt";
std::string set_of_cutset_file = "./output_dir/set_of_cuts_netlist.txt";

//output_files of identify_pipeline_linear.cpp
std::string reg_to_be_bypassed_file = "./output_dir/bypassable_registers_netlist.txt";
std::string reg_not_to_be_bypassed_file = "./output_dir/nbypassable_registers_netlist.txt";
std::string register_level_file = "./output_dir/registers_level_netlist.txt";


//net_list_graph nl_graph;
//vector<nl_graph_edge*> nl_graph;
nl_graph_iter nl_graph;
reg_graph skl_graph;
reinstated_reg_graph skl_graph_v2;
reg_graph modified_skl_graph;
reinstated_reg_graph modified_skl_graph_v2;
vector <std::string> flipflop_array;
vector <std::string> input_ports;
vector <std::string> output_ports;
vector <graph_node*>skl_graph_nodes;
vector <graph_node*>register_graph_nodes;
map <std::string,graph_node*>skl_graph_nodes_v2;
vector <graph_node*>modified_skl_graph_nodes;
vector <graph_node*>dummy_nodes;
map <std::string,bool>dummy_nodes_v2;
map <std::string,graph_node*>modified_skl_graph_nodes_v2;
reachable_graph reachability_hash;
cutset_graph cutset_hash;
vector <reg_graph_edge*> edges_in_loop;
reinstated_reg_graph edges_in_loop_v2;
vector <graph_node*> nodes_in_loop;
map <std::string,graph_node*> nodes_in_loop_v2;
vector <vector<graph_node*> > loop_groups;
vector <graph_node*> curr_loop_group;
reg_level_graph level_hash;
reg_graph rev_graph;
combinations map10;
int no_of_edges = 0;
int no_of_nodes = 0;
double max_tolerable_level_diff = 100;
double max_tolerable_penalty = 100; //(in percent)
bool from_netlist = true;  //linear pipeline level identification from netlist or from skl graph
type_of_cutsets cut_type = bypassable;


int getMilliCount(){
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

int getMilliSpan(int nTimeStart){
	int nSpan = getMilliCount() - nTimeStart;
	if(nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}

std::string trim(string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}
bool if_output(std::string instance_name)
{
    bool ret_value = false;
    for (int i = 0 ; i < output_ports.size();i++)
    {
        if(instance_name == output_ports[i])
        {
            ret_value = true;
            break;
        }
    }
    return ret_value;
}

void empty_a_vector(vector<graph_node*>* vec)
{
    int size = vec->size();
    for(int i = 0 ; i < size; i++)
    {
        (*vec).pop_back();
    }
}

//nl_graph functions

vector<std::string> get_children_nl_graph(std::string parent)
{
  vector<std::string> children;
  startnode_iter:: iterator q;
  startnode_iter curr_map;
  curr_map=nl_graph[parent];
  for ( q = curr_map.begin() ; q!= curr_map.end();q++)
  {
      //if (q->second[4] == "nv")
      //{
        children.push_back(q->first);
      //}
  }
  return children;
}
vector<std::string> get_children_nl_graph_nv(std::string parent)
{
  vector<std::string> children;
  startnode_iter:: iterator q;
  startnode_iter curr_map;
  curr_map=nl_graph[parent];
  for ( q = curr_map.begin() ; q!= curr_map.end();q++)
  {
      if (q->second[4] == "nv")
      {
        children.push_back(q->first);
      }
  }
  return children;
}

bool if_flop(std::string instance)
{
    for (int i = 0 ; i < flipflop_array.size() ;i++)
    {
        if (instance == flipflop_array[i])
            return true;
    }
    return false;
}

void Traverse_to_create_reg_level_linear_nl_graph(std::string node,int level)
{
    vector<std::string> children = get_children_nl_graph_nv(node);
    for (int i = 0 ; i < children.size() ;i++)
    {
        vector<std::string> prop = nl_graph[node][children[i]];
        std::string instance_name = prop[1];
        nl_graph[node][children[i]][4] = "v";
        if(if_flop(instance_name))
        {
          reg_level_graph::iterator p = level_hash.find(convert_graph_node_el_2_str(node,instance_name,children[i]));
          if (p != level_hash.end())
          {
              //p->second = (p->second+level)/2;
              double curr_min = p->second[0];
              double curr_max = p->second[1];
              if (level< curr_min)
              {
                  p->second[0] = level;
              }
              else if (level>curr_max)
              {
                  p->second[1] = level;
              }
              p->second[2] = (p->second[2]*p->second[3]+level)/(p->second[3]+1);
              p->second[3] = p->second[3]+1;
              if (curr_min!=curr_max)
              {
                  cout << "FATAL:: different reg level on same register in linear arch"<<endl;
                  exit(0);
              }
          }
          else
          {
              double* temp = new double[4];
              *(temp) = level;
              *(temp+1) = level;
              *(temp+2) = level;
              *(temp+3) = 1;
              level_hash[convert_graph_node_el_2_str(node,instance_name,children[i])] = temp;
          }
          Traverse_to_create_reg_level_linear_nl_graph(children[i],level+1);
        }
        else
        {
          Traverse_to_create_reg_level_linear_nl_graph(children[i],level);
        }
    }

}


void mark_all_edges_in_nl_graph()
{
    nl_graph_iter:: iterator p;
    nl_graph_iter brand_new_graph;
    for(p = nl_graph.begin();p != nl_graph.end(); p++)
    {
        std::string start_wire = p->first;
        startnode_iter curr_map = p->second;
        startnode_iter:: iterator q;
        for (q = curr_map.begin(); q!=curr_map.end();q++)
        {
            std::string end_wire = q->first;
            vector <std::string> old_vec = q->second; 
            if (old_vec[4] == "v")
            {
                //cout << "saurabh"<<endl;
                old_vec[4] = "nv";
            }
            brand_new_graph[start_wire][end_wire]=old_vec;
        }
    }
    nl_graph = brand_new_graph;
}

void print_children_of_nl_graph_node(queue<child_and_prop>Q)
{
    while(!Q.empty())
    {
        cout << Q.front().get_node()<<endl;
        Q.pop();
    }
}


void print_nl_graph()
{
    nl_graph_iter:: iterator p;
    for(p = nl_graph.begin();p != nl_graph.end(); p++)
    {
        std::string start_wire = p->first;
        startnode_iter curr_map = p->second;
        startnode_iter:: iterator q;
        for (q = curr_map.begin(); q!=curr_map.end();q++)
        {
            std::string end_wire = q->first;
            vector<std::string> curr_prop = q->second;
            std::string ip_port = curr_prop[0];
            std::string instance_name = curr_prop[1];
            std::string module_name = curr_prop[2];
            std::string op_port = curr_prop[3];
            std::string visited = curr_prop[4];
            cout << start_wire << ","<<end_wire<<"="<<ip_port<<","<<instance_name<<","<<module_name<<","<<op_port<< ","<< visited<<endl;
        }
    }
}


vector<std::string> get_edge_from_instance(std::string instance_name)
{
    nl_graph_iter:: iterator p;
    vector<std::string> ret_vec;
    for(p = nl_graph.begin();p != nl_graph.end(); p++)
    {
        std::string start_wire = p->first;
        startnode_iter curr_map = p->second;
        startnode_iter:: iterator q;
        for (q = curr_map.begin(); q!=curr_map.end();q++)
        {
            std::string end_wire = q->first;
            vector<std::string> curr_prop = q->second;
            if (curr_prop[1] == instance_name)
            {
                ret_vec.push_back(start_wire);
                ret_vec.push_back(end_wire);
                return ret_vec;
            }
        }
    }
    return ret_vec;
}

vector<std::string> get_parent(std::string wire)
{
    nl_graph_iter:: iterator p;
    vector<std::string> ret_vec;
    for(p = nl_graph.begin();p != nl_graph.end(); p++)
    {
        std::string start_wire = p->first;
        startnode_iter curr_map = p->second;
        startnode_iter:: iterator q;
        for (q = curr_map.begin(); q!=curr_map.end();q++)
        {
            std::string end_wire = q->first;
            if (end_wire == wire)
              ret_vec.push_back(start_wire);
        }
    }
    return ret_vec;
}

//nl_graph functions




///skeleton graph functions

std::string convert_graph_node_2_str(graph_node* node)
{
    std::string ip_wire = node->get_reg_input_w(); 
    std::string reg = node->get_reg_value(); 
    std::string op_wire = node->get_reg_output_w();
    stringstream ret_value;
    ret_value << ip_wire << ":" << reg << ":" << op_wire;
    return ret_value.str();
}

std::string convert_graph_node_el_2_str(std::string ip_wire, std::string reg, std::string op_wire)
{
    stringstream ret_value;
    ret_value << ip_wire << ":" << reg << ":" << op_wire;
    return ret_value.str();
}

std::vector<graph_node*> get_parents_of_given_skl_graph_node(reinstated_reg_graph* graph, graph_node* node)
{
    vector<graph_node*>ret_vec;
    reinstated_reg_graph::iterator p;
    for( p = graph->begin(); p!=graph->end();p++)
    {
        map<std::string,reg_graph_edge*>::iterator q;
        map<std::string,reg_graph_edge*> curr_map = p->second;
        for(q = curr_map.begin(); q!=curr_map.end(); q++)
        {
            reg_graph_edge* curr_edge = curr_map[q->first];
            graph_node* to_be_comp_node = curr_edge->get_end_graph_node();
            graph_node* to_be_pushed_node = curr_edge->get_start_graph_node();
            if((*(to_be_comp_node)) == (*(node)))
            {
                ret_vec.push_back(to_be_pushed_node);
            }
        }
    }
    return ret_vec;
}
//
std::vector<graph_node*> get_children_of_given_skl_graph_node(reinstated_reg_graph* graph, graph_node* node)
{
    vector<graph_node*>ret_vec;
    map<std::string,reg_graph_edge*> relevant_map = (*graph)[convert_graph_node_2_str(node)];
    map<std::string,reg_graph_edge*>::iterator q;
    for (q=relevant_map.begin();q!=relevant_map.end();q++)
    {
        reg_graph_edge* curr_edge = relevant_map[q->first];
        graph_node* end_node = curr_edge->get_end_graph_node();
        ret_vec.push_back(end_node);
    }
    return ret_vec;
}


void modify_graph(reinstated_reg_graph* curr_graph, graph_node* parent, graph_node* node, graph_node* child,double mux_overhead)
{
    reinstated_reg_graph::iterator p;
    map<std::string,reg_graph_edge*>::iterator q;
    p = curr_graph->find(convert_graph_node_2_str(parent));
    if (p!=curr_graph->end())
    {
        q = (p->second).find(convert_graph_node_2_str(child));
        reg_graph_edge* p_n_edge = (*curr_graph)[convert_graph_node_2_str(parent)][convert_graph_node_2_str(node)];
        reg_graph_edge* n_c_edge = (*curr_graph)[convert_graph_node_2_str(node)][convert_graph_node_2_str(child)];
        //make the new edge and update the weight of this edge
        if (q == (p->second).end())
        {
            //edge not found amek a new edge
            reg_graph_edge* new_edge = new reg_graph_edge(parent,child);
            //evaluate the weight of new edge
            new_edge->set_weight(p_n_edge->get_weight()+n_c_edge->get_weight()+mux_overhead);
            (*curr_graph)[convert_graph_node_2_str(parent)][convert_graph_node_2_str(child)] = new_edge;
        }
        else
        {
            //edge found
            reg_graph_edge* curr_edge = q->second;
            double curr_weight = curr_edge->get_weight();
            double new_weight = p_n_edge->get_weight()+n_c_edge->get_weight()+mux_overhead;
            //update the weight of already existing edge
            if(new_weight > curr_weight)
            {
                curr_edge->set_weight(new_weight);
            }

        }
        //delete connection parent->node
        //q = (p->second).find(convert_graph_node_2_str(node));
        //if (q!=(p->second).end())
        //{
        //    //map<std::string,reg_graph_edge*> curr_map = p->second;
        //    //cout<<(q->second)->get_weight()<<endl;
        //    //cout << convert_graph_node_2_str(q->second->get_start_graph_node()) << ":"<<convert_graph_node_2_str(q->second->get_end_graph_node()) <<endl;
        //    (p->second).erase(q);
        //    //
        //}
    }
    else 
    {
        cout<<"Node: "<<convert_graph_node_2_str(parent)<<" not found in the graph"<<endl;
    }
    
}

void new_graph(vector<graph_node*>curr_pipe,reinstated_reg_graph* curr_graph,double mux_overhead)
{
  vector<graph_node*> children;
  vector<graph_node*> parent;
  for (int i = 0 ; i < curr_pipe.size() ; i++ )
  {
      children = get_children_of_given_skl_graph_node(curr_graph,curr_pipe[i]);
      parent = get_parents_of_given_skl_graph_node(curr_graph,curr_pipe[i]);
      ///insert new edges
      for (int j = 0 ; j < parent.size() ; j++)
      {
          for (int k = 0 ; k < children.size() ; k++)
          {
              modify_graph(curr_graph,parent[j],curr_pipe[i],children[k],mux_overhead);
          }
          //delete connection parent[j]->curr_pipe[i]
          reinstated_reg_graph :: iterator q = curr_graph->find(convert_graph_node_2_str(parent[j]));
          if (q!=curr_graph->end())
          {
              map<std::string,reg_graph_edge*>::iterator r = (q->second).find(convert_graph_node_2_str(curr_pipe[i]));
              if(r!=(q->second).end())
              {
                  (q->second).erase(r);
              }

          }
      }
      //delete all the connection from curr_pipe[i]->children
      reinstated_reg_graph :: iterator p = curr_graph->find(convert_graph_node_2_str(curr_pipe[i]));
      curr_graph->erase(p);

  }
}

vector<int> subtract_group_int(vector<int> bigger_grp,vector<int>smaller_grp)
{
    for (int i = 0 ; i < smaller_grp.size(); i++)
    {
        vector<int>::iterator p = find(bigger_grp.begin(),bigger_grp.end(),smaller_grp[i]);
        if (p != bigger_grp.end())
        {
            bigger_grp.erase(p);
        }

    }
    return bigger_grp;
}

vector<vector<graph_node*> > filter_dummy_pipelines(vector<vector<graph_node*> > pipelines)
{
    vector<vector<graph_node*> > ret_vec;
    for (int i = 0 ; i < pipelines.size() ; i++)
    {
        vector<graph_node*> curr_pipeline = pipelines[i];
        if(!intersection(curr_pipeline,dummy_nodes))
        {
           ret_vec.push_back(curr_pipeline);
        }
    }
  return ret_vec;
}

vector<int> choose_pipelines(vector<vector<graph_node*> >pipelines, map<int,vector<int> > forbidden_pipelines, vector<int> reward_arr, double mux_overhead,int no_of_iterations)
{
    int count = 0;
    vector<int> ret_set;
    int max_reward = 0;
    double univ_crit_path = get_crit_path(&skl_graph_v2);
    //cout << univ_crit_path<<endl;
    while (count < no_of_iterations)
    {
        vector<int> curr_set;
        int total_reward=0;
        double total_penalty = 0; //(in percentage of critical path degradation)
        vector<int> pipeline_pool;
        //initialize current pool
        for (int i = 0 ; i < pipelines.size() ; i++)
        {
            pipeline_pool.push_back(i);
        }
        //insert pipelines index at random position in current pool
        random_shuffle(pipeline_pool.begin(),pipeline_pool.end());
        //for(int i = 0 ; i < pipeline_pool.size() ; i++)
        //{
        //    cout<<pipeline_pool[i]<<endl;
        //}
        //cout << endl<<endl;
        reinstated_reg_graph curr_graph = skl_graph_v2;
        while(pipeline_pool.size()>0 && total_penalty < max_tolerable_penalty )
        {
            int curr_idx = pipeline_pool.back();
            //cout << curr_idx<<endl;
            vector<graph_node*> curr_pipe = pipelines[curr_idx];
            //print_skl_graph_node_vector(curr_pipe);
            //cout<<endl;
        //    //print_skl_graph_node_vector(curr_pipe);
            //cout<<"BEFORE MODIFICATION: "<<endl;
            //print_given_skl_graph(&curr_graph);
            new_graph(curr_pipe,&curr_graph,mux_overhead);
            //break;
            //cout<<"AFTER MODIFICATION: "<<endl;
            //print_given_skl_graph(&curr_graph);
            double by_pipe_crit_path  = get_crit_path(&curr_graph);
            total_penalty = (by_pipe_crit_path-univ_crit_path)*100/univ_crit_path;
            total_reward = total_reward+reward_arr[curr_idx];
            pipeline_pool.pop_back();
            pipeline_pool = subtract_group_int(pipeline_pool,forbidden_pipelines[curr_idx]);
            curr_set.push_back(curr_idx);
        }
        if (total_penalty > max_tolerable_penalty)
        {
          curr_set.pop_back();
        }
        if (total_reward > max_reward)
        {
            max_reward = total_reward;
            ret_set = curr_set;
        }
        count++;
    }
    return ret_set;
}



double get_crit_path(reinstated_reg_graph* curr_graph)
{
  reinstated_reg_graph::iterator p;
  double crit_path = 0;
  for (p = curr_graph->begin() ; p!=curr_graph->end(); p++)
  {
     map<std::string,reg_graph_edge*>::iterator q;
     map<std::string,reg_graph_edge*> curr_map = p->second;
     for (q = curr_map.begin() ; q!=curr_map.end() ;q++)
     {
         reg_graph_edge* curr_edge = q->second;
         double curr_path = curr_edge->get_weight();
         if (curr_path > crit_path)
         {
             crit_path = curr_path;
         }
     }

  }
  return crit_path;
}

vector<int> get_intersection_index(int pipeline_idx,vector<vector<graph_node*> >pipelines)
{
    vector<int> ret_vec;
    for(int i = 0 ; i < pipelines.size(); i++)
    {
        if (i != pipeline_idx)
        {
            vector<graph_node*> curr_pipeline = pipelines[i];
            vector<graph_node*> arg_pipeline = pipelines[pipeline_idx];
            if(intersection(curr_pipeline,arg_pipeline))
            {
                ret_vec.push_back(i);
            }
        }

    }
    return ret_vec;
}


vector<graph_node*> cut_2_reg(vector<reg_graph_edge*> curr_cut)
{
    vector<graph_node*> ret_vec;
    for(int i = 0 ; i < curr_cut.size(); i++)
    {
        reg_graph_edge* curr_edge = curr_cut[i];
        graph_node* start_node = curr_edge->get_start_graph_node();
        if (!check_if_node_in_vector(start_node,ret_vec))
        {
            ret_vec.push_back(start_node);
        }
    }
    return ret_vec;
}

void print_graph_nodes_in_file(ofstream& file)
{
   for(int  i = 0 ; i < skl_graph_nodes.size();i++)
   {
       file << skl_graph_nodes[i]->get_reg_input_w() <<":"<<skl_graph_nodes[i]->get_reg_value()<< ":"<<skl_graph_nodes[i]->get_reg_output_w()<<endl;
   }
}
void print_dummy_graph_nodes_in_file(ofstream& file)
{
   for(int  i = 0 ; i < dummy_nodes.size();i++)
   {
       file << dummy_nodes[i]->get_reg_input_w() <<":"<<dummy_nodes[i]->get_reg_value()<< ":"<<dummy_nodes[i]->get_reg_output_w()<<endl;
   }
}
void print_register_nodes_in_file(ofstream& file)
{
   for(int  i = 0 ; i < register_graph_nodes.size();i++)
   {
       file << register_graph_nodes[i]->get_reg_input_w() <<":"<<register_graph_nodes[i]->get_reg_value()<< ":"<<register_graph_nodes[i]->get_reg_output_w()<<endl;
   }
}

void print_bypassable_reg_to_file(ofstream& file,vector<vector<graph_node*> > pipelines,vector<int>index)
{
   for(int  i = 0 ; i < index.size();i++)
   {
       vector<graph_node*> curr_pipe = pipelines[index[i]];
       for (int j = 0 ; j < curr_pipe.size();j++)
       {
          file << curr_pipe[j]->get_reg_input_w() <<":"<<curr_pipe[j]->get_reg_value()<< ":"<<curr_pipe[j]->get_reg_output_w()<<endl;
       }
   }
}

void print_non_bypassable_reg_to_file(ofstream& file,vector<vector<graph_node*> > pipelines,vector<int>index)
{
   vector<graph_node*> to_be_bypassed_reg;
   for(int  i = 0 ; i < index.size();i++)
   {
       vector<graph_node*> curr_pipe = pipelines[index[i]];
       for (int j = 0 ; j < curr_pipe.size();j++)
       {
           to_be_bypassed_reg.push_back(curr_pipe[j]);
       }
   }
   vector<graph_node*> not_to_be_bypassed_reg = subtract_group(register_graph_nodes,to_be_bypassed_reg);
  for (int i = 0 ; i < not_to_be_bypassed_reg.size() ;i++)
  {
      file << not_to_be_bypassed_reg[i]->get_reg_input_w() <<":"<<not_to_be_bypassed_reg[i]->get_reg_value()<< ":"<<not_to_be_bypassed_reg[i]->get_reg_output_w()<<endl;
  }
}


vector<graph_node* >get_children_of_skl_graph_node_v2(graph_node* node, bool mode, int grp) //mode=false -> all_children //mode=true -> non-visited children
{
    vector <graph_node* > return_vector;
    reinstated_reg_graph* curr_graph = (grp == 0)?(&skl_graph_v2):(&modified_skl_graph_v2);
    map<std::string, reg_graph_edge* > children_map = (*curr_graph)[convert_graph_node_2_str(node)];
    map<std::string, reg_graph_edge* >::iterator p;
    for(p = children_map.begin(); p!=children_map.end() ; p++)
    {
        reg_graph_edge* curr_edge = p->second;
        graph_node* end_node = curr_edge->get_end_graph_node();
        if (mode == false)
        {
           return_vector.push_back(end_node);
        }
        else
        {
            if (!(curr_edge->get_visited()))
            {
              return_vector.push_back(end_node);
            }
        }
    }
return return_vector;
}

vector<graph_node* > get_children_of_skl_graph_node(graph_node* node,bool mode, int grp)
{
    vector<graph_node* >return_vector;
    reg_graph* curr_graph = (grp == 0)?(&skl_graph):(&modified_skl_graph);
    for(int i = 0 ; i < curr_graph->size(); i++)
    {
        reg_graph_edge* graph_edge = (*curr_graph)[i];
        graph_node* start_node = graph_edge->get_start_graph_node();
        graph_node* end_node = graph_edge->get_end_graph_node();
        if( (*(node)) == (*(start_node)))
        {
           if(mode == false)
              return_vector.push_back(end_node);
           else
           {
               if(!(graph_edge->get_visited()))
                   return_vector.push_back(end_node);
           }
        }
    }
    return return_vector;
}

vector<graph_node* > get_children_of_loop_graph_node_v2(graph_node* node)
{
    vector<graph_node* >return_vector;
    map<std::string, reg_graph_edge*> children_map = edges_in_loop_v2[convert_graph_node_2_str(node)];
    map<std::string, reg_graph_edge* >:: iterator p;
    for ( p = children_map.begin(); p!=children_map.end() ; p++)
    {
        reg_graph_edge* curr_edge = p->second;
        graph_node* child = curr_edge->get_end_graph_node();
        if(!(curr_edge->get_visited()))
          return_vector.push_back(child);
    }
    return return_vector;
}
vector<graph_node* > get_children_of_loop_graph_node(graph_node* node)
{
    vector<graph_node* >return_vector;
    for(int i = 0 ; i < edges_in_loop.size(); i++)
    {
        reg_graph_edge* graph_edge = edges_in_loop[i];
        graph_node* start_node = graph_edge->get_start_graph_node();
        graph_node* end_node = graph_edge->get_end_graph_node();
        if( (*(node)) == (*(start_node)))
        {
             if(!(graph_edge->get_visited()))
                 return_vector.push_back(end_node);
        }
    }
    return return_vector;
}

void put_dummy_output_node()
{
    graph_node* dummy_output = get_skl_graph_node("dummy_out","","dummy_out",0);
    for (int i = 0 ; i < skl_graph_nodes.size() ;i++)
    {
        if (if_output(skl_graph_nodes[i]->get_reg_output_w()))
        {
            //cout << skl_graph_nodes[i]->get_reg_output_w() <<endl;
            reg_graph_edge* new_edge = new reg_graph_edge(skl_graph_nodes[i],dummy_output);
            skl_graph.push_back(new_edge);
        }
    }
}



void print_skl_graph_nodes(int grp)
{
  vector <graph_node*>* vec = (grp == 0)?(&skl_graph_nodes) : (&modified_skl_graph_nodes);  
  for(int i = 0 ; i < vec->size(); i++)
  {
      if ((*vec)[i]->get_visited())
      {
        cout<< (*vec)[i]->get_reg_input_w()<<":"<<(*vec)[i]->get_reg_value()<<":"<<(*vec)[i]->get_reg_output_w()<<":"<<"(visited)"<<endl;
      }
      else
      {
        cout<< (*vec)[i]->get_reg_input_w()<<":"<<(*vec)[i]->get_reg_value()<<":"<<(*vec)[i]->get_reg_output_w()<<":"<<"(not-visited)"<<endl;
      }
  }
}

void print_skl_graph_to_file(ofstream& file)
{
    for(int i = 0 ; i < skl_graph.size() ; i++)
    {
        reg_graph_edge* edge = skl_graph[i];
        file << edge->get_start_graph_node()->get_reg_input_w()<<":"<<edge->get_start_graph_node()->get_reg_value() << ":"<< edge->get_start_graph_node()->get_reg_output_w()<<"->"<< edge->get_end_graph_node()->get_reg_input_w()<<":"<<edge->get_end_graph_node()->get_reg_value()<<":"<< edge->get_end_graph_node()->get_reg_output_w() <<"="<<edge->get_weight()<<endl;
    }
}


void print_skl_graph_node(graph_node* node)
{
    if (node != NULL)
    {
      cout<< node->get_reg_input_w()<<":"<<node->get_reg_value()<<":"<<node->get_reg_output_w();
    }

}

bool if_one_of_the_skl_graph_nodes(graph_node node,int grp) //grp = 0 -> skl_graph_nodes // grp = 1 -> modified_skl_graph_nodes
{
    vector <graph_node*>* vec = (grp == 0)?(&skl_graph_nodes):(&modified_skl_graph_nodes);
    for(int  i = 0 ; i < vec->size();i++)
    {
        if( ((*vec)[i]->get_reg_input_w() == node.get_reg_input_w()) && ((*vec)[i]->get_reg_value()== node.get_reg_value()) && ((*vec)[i]->get_reg_output_w() == node.get_reg_output_w()) )
        {
            return true;
        }
    }
    return false;
}

bool if_one_of_the_skl_graph_nodes_v2(graph_node* node, int grp)
{
    map<std::string, graph_node*>* curr_vec = (grp == 0) ? (&skl_graph_nodes_v2):(&modified_skl_graph_nodes_v2);
    map<std::string,graph_node*>::iterator p0 = curr_vec->find(convert_graph_node_2_str(node));
    if (p0!=curr_vec->end())
    {
        return true;
    }
    else
    {
        return false;
    }
}


graph_node* get_skl_graph_node_v2(std::string reg,std::string start_wire,std::string end_wire, int grp)
{
    std::string node_str = convert_graph_node_el_2_str(start_wire,reg,end_wire);
    map<std::string,graph_node*>*vec = (grp == 0)?(&skl_graph_nodes_v2):(&modified_skl_graph_nodes_v2);
    map<std::string,graph_node*>::iterator p0 = (*vec).find(node_str);
    if (p0 != vec->end())
    {
        return p0->second;
    }
    else
    {
        return NULL;
    }

}

graph_node* get_skl_graph_node(std::string reg,std::string start_wire,std::string end_wire,int grp)
{
    graph_node* node;
    graph_node* return_node = new graph_node();
    vector <graph_node*>* vec = (grp == 0)?(&skl_graph_nodes):(&modified_skl_graph_nodes);
    for(int i = 0 ; i < vec->size() ; i++)
    {
        node = (*vec)[i];
        if(node->get_reg_value() == reg && node->get_reg_input_w() == start_wire && node->get_reg_output_w() == end_wire)
        {
            return (*vec)[i]; 
        }
    }
    return return_node;
}

bool if_a_flop(std::string instance_name)
{
    for (int k = 0 ; k < flipflop_array.size();k++)
    {
        if(instance_name == flipflop_array[k])
        {
            return true;
        }
    }
    return false;
}

void create_skl_graph_node_vector()
{
    graph_node* node = new graph_node("dummy","","dummy");
    skl_graph_nodes.push_back(node);
    dummy_nodes.push_back(node);
    for (int i = 0 ; i < input_ports.size(); i++)
    {
        std::string input = input_ports[i];
        graph_node* node = new graph_node(input,"dummy",input);
        skl_graph_nodes.push_back(node);
        dummy_nodes.push_back(node);
    }
    for (int i = 0; i < flipflop_array.size(); i++)
    {
        vector<std::string> edge = get_edge_from_instance(flipflop_array[i]);
        if (edge.size() > 0)
        {
          if (!if_output(edge[1]))
          {
            graph_node* node = new graph_node(flipflop_array[i],edge[0],edge[1]);
            skl_graph_nodes.push_back(node);
            register_graph_nodes.push_back(node);
          }
        }
        else
        {
            cout << "WARNING:: flip-flop : "<<flipflop_array[i]<<" not found in netlist graph"<<endl;
        }
    }
    for (int i = 0 ; i < output_ports.size() ; i++)
    {
        vector<std::string> parent_arr = get_parent(output_ports[i]);
        for (int j = 0 ; j < parent_arr.size(); j++)
        {
            graph_node* node = new graph_node(nl_graph[parent_arr[j]][output_ports[i]][1],parent_arr[j],output_ports[i]);
            skl_graph_nodes.push_back(node);
            std::string instance_name = nl_graph[parent_arr[j]][output_ports[i]][1];
            bool a_flop = if_a_flop(instance_name);
            if (!a_flop)
            {
                dummy_nodes.push_back(node);
            }
            else
            {
                register_graph_nodes.push_back(node);
            }
        }
    }
    graph_node* dummy_output = new graph_node("dummy_out","","dummy_out");
    skl_graph_nodes.push_back(dummy_output);
    dummy_nodes.push_back(dummy_output);
}


void create_modified_skl_graph_node_vector()
{
    for(int i = 0 ; i < skl_graph_nodes.size(); i++)
    {
        int grp = get_group(skl_graph_nodes[i]);
        if(grp != -1)
        {
            stringstream reg_name; 
            reg_name <<grp;
            stringstream ip_w; 
            ip_w << "loop_grp_win_" << grp;
            stringstream op_w; 
            op_w << "loop_grp_wout_" << grp;
            graph_node* group_node = new graph_node(reg_name.str(),ip_w.str(),op_w.str());
            if(!if_one_of_the_skl_graph_nodes_v2(group_node,1))
            {
                modified_skl_graph_nodes.push_back(group_node);
                modified_skl_graph_nodes_v2[convert_graph_node_2_str(group_node)] = group_node;
            }
            else
            {
                delete group_node;
            }
        }
        else
        {
            modified_skl_graph_nodes.push_back(skl_graph_nodes[i]);
            modified_skl_graph_nodes_v2[convert_graph_node_2_str(skl_graph_nodes[i])] = skl_graph_nodes[i];
        }
    }
}

void create_modified_skl_graph()
{
   for(int i = 0 ; i < skl_graph.size(); i++)
   {
      reg_graph_edge* curr_edge = skl_graph[i];
      graph_node* start_node = curr_edge->get_start_graph_node();
      int start_grp_no = get_group(start_node);
      graph_node* end_node = curr_edge->get_end_graph_node();
      int end_grp_no = get_group(end_node);
      if(start_grp_no!=-1 && end_grp_no!=-1)
      {
          if (start_grp_no != end_grp_no)
          {
             stringstream reg_name; 
             reg_name <<start_grp_no;
             stringstream ip_w; 
             ip_w << "loop_grp_win_" << start_grp_no;
             stringstream op_w; 
             op_w << "loop_grp_wout_" << start_grp_no;
             graph_node* new_start_node = get_skl_graph_node_v2(reg_name.str(),ip_w.str(),op_w.str(),1);
             stringstream reg_name_2; 
             reg_name_2 <<end_grp_no;
             stringstream ip_w_2; 
             ip_w_2 << "loop_grp_win_" << end_grp_no;
             stringstream op_w_2; 
             op_w_2 << "loop_grp_wout_" << end_grp_no;
             graph_node* new_end_node = get_skl_graph_node_v2(reg_name_2.str(),ip_w_2.str(),op_w_2.str(),1);
             reg_graph_edge* new_edge;
             if (new_start_node != NULL && new_end_node != NULL)
             {
               new_edge = get_edge_from_skl_graph_v2(new_start_node,new_end_node,1);
             }
             else
             {
                 cout << "WARNING:: node1: ";
                 print_skl_graph_node(new_start_node);
                 cout << " and node2: ";
                 print_skl_graph_node(new_end_node);
                 cout<<" not found in skl_node database " <<endl;
             }
             if(new_edge == NULL)
             {
                 new_edge = new reg_graph_edge(new_start_node,new_end_node);
                 modified_skl_graph.push_back(new_edge);
                 modified_skl_graph_v2[convert_graph_node_2_str(new_start_node)][convert_graph_node_2_str(new_end_node)] = new_edge;
             }
          }
      }
      else if(start_grp_no==-1 && end_grp_no==-1)
      {
          if (!if_edge_in_skl_graph_v2(skl_graph[i],1))
          {
            modified_skl_graph.push_back(skl_graph[i]);
            modified_skl_graph_v2[convert_graph_node_2_str(start_node)][convert_graph_node_2_str(end_node)] = skl_graph[i];
          }
      }
      else if(start_grp_no==-1 && end_grp_no!=-1)
      {
          //start node nahi mila
          stringstream reg_name_2; 
          reg_name_2 <<end_grp_no;
          stringstream ip_w_2; 
          ip_w_2 << "loop_grp_win_" << end_grp_no;
          stringstream op_w_2; 
          op_w_2 << "loop_grp_wout_" << end_grp_no;
          graph_node* new_end_node = get_skl_graph_node_v2(reg_name_2.str(),ip_w_2.str(),op_w_2.str(),1);
          reg_graph_edge* new_edge = get_edge_from_skl_graph_v2(start_node,new_end_node,1);
          if(new_edge == NULL)
          {
              new_edge = new reg_graph_edge(start_node,new_end_node);
              modified_skl_graph.push_back(new_edge);
              modified_skl_graph_v2[convert_graph_node_2_str(start_node)][convert_graph_node_2_str(new_end_node)] = new_edge;
          }
      }
      else if(start_grp_no!=-1 && end_grp_no==-1)
      {
          stringstream reg_name; 
          reg_name <<start_grp_no;
          stringstream ip_w; 
          ip_w << "loop_grp_win_" << start_grp_no;
          stringstream op_w; 
          op_w << "loop_grp_wout_" << start_grp_no;
          graph_node* new_start_node = get_skl_graph_node_v2(reg_name.str(),ip_w.str(),op_w.str(),1);
          reg_graph_edge* new_edge = get_edge_from_skl_graph_v2(new_start_node,end_node,1);
          if(new_edge == NULL)
          {
              new_edge = new reg_graph_edge(new_start_node,end_node);
              modified_skl_graph.push_back(new_edge);
              modified_skl_graph_v2[convert_graph_node_2_str(new_start_node)][convert_graph_node_2_str(end_node)] = new_edge;
          }
      }
   }
}

bool null_skl_graph_node(graph_node* node)
{
    if(node->get_reg_output_w() == "" && node->get_reg_value() == "" &&  node->get_reg_input_w() == "")
        return true;
    else
        return false;
}


bool if_edge_in_skl_graph(reg_graph_edge* edge,int grp)
{
    reg_graph* curr_graph = (grp == 0)?(&skl_graph):(&modified_skl_graph);
    for(int i = 0 ; i < curr_graph->size() ; i++)
    {
        if((*edge) == (*((*curr_graph)[i])))
            return true;
    }
    return false;
}


bool if_edge_in_skl_graph_v2(reg_graph_edge* edge, int grp)
{
    reinstated_reg_graph* curr_graph = (grp == 0) ? (&skl_graph_v2) : (&modified_skl_graph_v2);
    reinstated_reg_graph::iterator p0 = (*curr_graph).find(convert_graph_node_2_str(edge->get_start_graph_node()));
    if (p0 == curr_graph->end())
    {
        map<std::string, reg_graph_edge*>::iterator p1 = p0->second.find(convert_graph_node_2_str(edge->get_end_graph_node()));
        if (p1 != p0->second.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

}


void write_bypaassable_reg_to_file(ofstream& file, bool even)
{
    reg_level_graph::iterator p;
    int rem = even ? 0 : 1;
    for (p=level_hash.begin();p!=level_hash.end();p++)
    {
        double min_level = p->second[0];
        double max_level = p->second[1];
        double avg_level = p->second[2];
        std::string reg_name = p->first;
        if (!if_dummy_str(reg_name))
        {
          if (min_level == max_level && avg_level == max_level)
          {
              if (int(min_level)%2 == rem)
              {
                  file << reg_name<<endl;
              }
          }
          else
          {
              cout << "register: "<<reg_name<<"found in multiple levels"<<endl;
              exit(0);
          }
        }
    }
}

void Traverse_to_create_reg_level(graph_node* node, double level)
{
   vector<graph_node*> children_pruned = get_children_of_skl_graph_node_v2(node,true,1);
   vector<graph_node*> children_actual = get_children_of_skl_graph_node_v2(node,false,1);
   reg_level_graph::iterator p = level_hash.find(convert_graph_node_2_str(node));
   if (p != level_hash.end())
   {
       //p->second = (p->second+level)/2;
       double curr_min = p->second[0];
       double curr_max = p->second[1];
       if (level< curr_min)
       {
           p->second[0] = level;
       }
       else if (level>curr_max)
       {
           p->second[1] = level;
       }
       p->second[2] = (p->second[2]*p->second[3]+level)/(p->second[3]+1);
       p->second[3] = p->second[3]+1;
   }
   else
   {
       double* temp = new double[4];
       *(temp) = level;
       *(temp+1) = level;
       *(temp+2) = level;
       *(temp+3) = 1;
       level_hash[convert_graph_node_2_str(node)] = temp;
   }
   for (int i = 0 ; i < children_pruned.size() ; i++)
   {
       Traverse_to_create_reg_level(children_pruned[i],level+1);
   }
}



void Traverse_to_create_reg_level_linear(graph_node* node, double level)
{
   vector<graph_node*> children_pruned = get_children_of_skl_graph_node_v2(node,true,0);
   //vector<graph_node*> children_actual = get_children_of_skl_graph_node_v2(node,false,0);
   reg_level_graph::iterator p = level_hash.find(convert_graph_node_2_str(node));
   if (p != level_hash.end())
   {
       //p->second = (p->second+level)/2;
       double curr_min = p->second[0];
       double curr_max = p->second[1];
       if (level< curr_min)
       {
           p->second[0] = level;
       }
       else if (level>curr_max)
       {
           p->second[1] = level;
       }
       p->second[2] = (p->second[2]*p->second[3]+level)/(p->second[3]+1);
       p->second[3] = p->second[3]+1;
   }
   else
   {
       double* temp = new double[4];
       *(temp) = level;
       *(temp+1) = level;
       *(temp+2) = level;
       *(temp+3) = 1;
       level_hash[convert_graph_node_2_str(node)] = temp;
   }
   for (int i = 0 ; i < children_pruned.size() ; i++)
   {
       Traverse_to_create_reg_level_linear(children_pruned[i],level+1);
   }
}


void rebuild_nl_graph(ifstream& fin)
{
    std::string line;
    while(std::getline(fin,line))
    {
        //cout << line <<endl;
        
        std::string delimiter = "=";
        size_t pos = 0;
        std::string token;
        std::string start_wire = "";
        std::string end_wire = "";
        std::string in_port = "";
        std::string instance_name = "";
        std::string module_name = "";
        std::string out_port= "";
        int count = 0;
        //processing graph edge
        pos = line.find(delimiter);
        token = line.substr(0, pos);
        std::string delimiter_comma = ",";
        size_t pos1 = 0;
        std::string token2;
        while((pos1 = token.find(delimiter_comma)) != std::string::npos)
        {
            token2 = token.substr(0,pos1);
            start_wire = token2;
            token.erase(0, pos1 + delimiter_comma.length());
            end_wire = token;
        }
        line.erase(0, pos + delimiter.length());
        token = line;
        pos1 = 0;
        //processing graph edge properties
        while((pos1 = token.find(delimiter_comma)) != std::string::npos)
        {
            token2 = token.substr(0,pos1);
            token.erase(0, pos1 + delimiter_comma.length());
            if(count == 0)
              in_port = token2;
            else if (count == 1)
              instance_name = token2;
            else if (count == 2)
              module_name = token2;
            count++;
        }
        token.erase(0, pos1 + delimiter_comma.length());
        out_port = token;
        vector<std::string> temp_vector;
        temp_vector.push_back(in_port);
        temp_vector.push_back(instance_name);
        temp_vector.push_back(module_name);
        temp_vector.push_back(out_port);
        temp_vector.push_back("nv");
        if (nl_graph[start_wire][end_wire].size() == 0)
        {
          nl_graph[start_wire][end_wire]=temp_vector;
        }
        else
        {
           cout << "INFO:: repeated edge: "<< start_wire<<","<<end_wire<<endl;
        }
    }
    //print_nl_graph();
    fin.close();

}



void print_reg_level_hash_to_file(ofstream& file)
{
    reg_level_graph::iterator p;
    for (p = level_hash.begin() ; p!= level_hash.end() ; p++)
    {
        file << p->first << "="<<p->second[0]<<","<<p->second[1]<<","<<p->second[2]<<","<<p->second[3]<<endl;
    }
}

void print_reg_level_hash()
{
    reg_level_graph::iterator p;
    for (p = level_hash.begin() ; p!= level_hash.end() ; p++)
    {
        cout << p->first << "="<<p->second[0]<<","<<p->second[1]<<","<<p->second[2]<<","<<p->second[3]<<endl;
    }
}

void Traverse_to_create_skl_graph(std::string current_nd,graph_node* node)
{

  vector<std::string> children;
  startnode_iter:: iterator q;
  startnode_iter curr_map;
  curr_map=nl_graph[current_nd];
  for ( q = curr_map.begin() ; q!= curr_map.end();q++)
  {
      if (q->second[4] == "nv")
      {
        children.push_back(q->first);
      }
  }
  int count;
  for (count = 0 ; count < children.size(); count++)
  {
      nl_graph[current_nd][children[count]][4] = "v";
      std::string instance_name = nl_graph[current_nd][children[count]][1];
      graph_node* possible_node = new graph_node(instance_name,current_nd,children[count]);
      if (if_one_of_the_skl_graph_nodes(*possible_node,0))
      {
          graph_node* second_node = get_skl_graph_node(instance_name,current_nd,children[count],0);
          reg_graph_edge* possible_edge = get_edge_from_skl_graph_v2(node,second_node,0);
          //reg_graph_edge* possible_edge = skl_graph_v2[convert_graph_node_2_str(node)][convert_graph_node_2_str(second_node)];
          if (possible_edge == NULL)
          {
              possible_edge = new reg_graph_edge(node,second_node);
              skl_graph.push_back(possible_edge);
              skl_graph_v2[convert_graph_node_2_str(node)][convert_graph_node_2_str(second_node)] = possible_edge;
          }
      }
      else
      {
          Traverse_to_create_skl_graph(children[count],node);
      }
      delete possible_node;
  }
}

void print_given_skl_graph(reinstated_reg_graph* curr_graph)
{
    reinstated_reg_graph::iterator p;
    map<std::string,reg_graph_edge*>::iterator q;
    for(p = curr_graph->begin(); p!=curr_graph->end();p++)
    {
         map<std::string,reg_graph_edge*> curr_map = p->second;
         for(q = curr_map.begin();q!=curr_map.end();q++)
         {
              reg_graph_edge* edge = q->second;
              if (edge->get_visited())
              {
                cout << edge->get_start_graph_node()->get_reg_input_w()<<":"<<edge->get_start_graph_node()->get_reg_value() << ":"<< edge->get_start_graph_node()->get_reg_output_w()<<"->"<< edge->get_end_graph_node()->get_reg_input_w()<<":"<<edge->get_end_graph_node()->get_reg_value()<<":"<< edge->get_end_graph_node()->get_reg_output_w() <<"="<<edge->get_weight()<<"(visited)"<<endl;
              }
              else
              {
                cout << edge->get_start_graph_node()->get_reg_input_w()<<":"<<edge->get_start_graph_node()->get_reg_value() << ":"<< edge->get_start_graph_node()->get_reg_output_w()<<"->"<< edge->get_end_graph_node()->get_reg_input_w()<<":"<<edge->get_end_graph_node()->get_reg_value()<<":"<< edge->get_end_graph_node()->get_reg_output_w() <<"="<<edge->get_weight()<<"(not-visited)"<<endl;
              }
         }
   
    }
}

void print_skl_graph(int grp)
{
    reg_graph* curr_graph = (grp == 0) ? (&skl_graph):(&modified_skl_graph);
    for(int i = 0 ; i < curr_graph->size() ; i++)
    {
        reg_graph_edge* edge = (*curr_graph)[i];
        if (edge->get_visited())
        {
          cout << edge->get_start_graph_node()->get_reg_input_w()<<":"<<edge->get_start_graph_node()->get_reg_value() << ":"<< edge->get_start_graph_node()->get_reg_output_w()<<"->"<< edge->get_end_graph_node()->get_reg_input_w()<<":"<<edge->get_end_graph_node()->get_reg_value()<<":"<< edge->get_end_graph_node()->get_reg_output_w() <<"="<<edge->get_weight()<<"(visited)"<<endl;
        }
        else
        {
          cout << edge->get_start_graph_node()->get_reg_input_w()<<":"<<edge->get_start_graph_node()->get_reg_value() << ":"<< edge->get_start_graph_node()->get_reg_output_w()<<"->"<< edge->get_end_graph_node()->get_reg_input_w()<<":"<<edge->get_end_graph_node()->get_reg_value()<<":"<< edge->get_end_graph_node()->get_reg_output_w() <<"="<<edge->get_weight()<<"(not-visited)"<<endl;
        }
    }
}

reachable_graph::iterator get_reachability_vector(graph_node* node)
{
    std::string node_str = convert_graph_node_2_str(node);
    reachable_graph::iterator p = reachability_hash.find(node_str);
    return p;
}


reg_graph_edge* get_edge_from_skl_graph_v2(graph_node* start_node, graph_node* end_node, int grp)
{
    reinstated_reg_graph* curr_graph = (grp == 0) ? (&skl_graph_v2):(&modified_skl_graph_v2);
    std::string start_node_str = convert_graph_node_2_str(start_node);
    std::string end_node_str = convert_graph_node_2_str(end_node);
    reinstated_reg_graph::iterator p0 = (*curr_graph).find(start_node_str);
    if (p0 != (*curr_graph).end())
    {
        map<std::string,reg_graph_edge*>::iterator p1 = (p0->second).find(end_node_str);
        if (p1!=(p0->second).end())
        {
            return p1->second;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

reg_graph_edge* get_edge_from_edges_in_loop_v2(graph_node* start_node, graph_node* end_node)
{
   std::string start_node_str = convert_graph_node_2_str(start_node);
   std::string end_node_str = convert_graph_node_2_str(end_node);
   reinstated_reg_graph::iterator p0 = edges_in_loop_v2.find(start_node_str);
   if (p0 != edges_in_loop_v2.end())
   {
       map<std::string,reg_graph_edge*>::iterator p1 = (p0->second).find(end_node_str);
       if (p1!=(p0->second).end())
       {
           return p1->second;
       }
       else
       {
           return NULL;
       }
   }
   else
   {
       return NULL;
   }
    
}


bool if_edge_in_loop_data_base_v2(reg_graph_edge* edge)
{
    if (edge != NULL)
    {
      graph_node* start_node = edge->get_start_graph_node();
      graph_node* end_node = edge->get_end_graph_node();
      std::string start_node_str = convert_graph_node_2_str(start_node);
      std::string end_node_str = convert_graph_node_2_str(end_node);
      reinstated_reg_graph::iterator p0 = edges_in_loop_v2.find(start_node_str);
      if (p0 != edges_in_loop_v2.end())
      {
          map<std::string,reg_graph_edge*>::iterator p1 = (p0->second).find(end_node_str);
          if (p1!=(p0->second).end())
          {
              return true;
          }
          else
          {
              return false;
          }
      }
      else
      {
          return false;
      }
    }
    else
    {
        return false;
    }

}

reg_graph_edge* get_edge_from_skl_graph(graph_node* start_node,graph_node* end_node,bool supress_warning,int grp)
{
    reg_graph* curr_graph = (grp == 0)?(&skl_graph):(&modified_skl_graph);
    reg_graph_edge edge(start_node,end_node);
    int i;
    for(i = 0 ; i < curr_graph->size() ;i++)
    {
        if(edge == (*((*curr_graph)[i])))
            return (*curr_graph)[i];
    }
    if(i == curr_graph->size())
    {
        if(!supress_warning)
        {
            cout << "WARNING::Edge not found in the graph null edge will be returned"<<endl;
        }
        reg_graph_edge* null_edge = new reg_graph_edge();
        null_edge = NULL;
        return null_edge;
    }
}

vector<graph_node* >merge_skl_graph_node_vectors(vector<graph_node* >vec1,vector<graph_node* >vec2)
{
    vector<graph_node* >ret_vec;
    ret_vec = vec1;
    if(vec2.size() == 0)
    {
        return vec1;
    }
    else
    {
        for(int i = 0 ; i < vec2.size();i++)
        {
            int k;
            for(k = 0 ; k < vec1.size();k++)
            {
                if((*(vec2[i]))==(*(vec1[k])))
                {
                    break;
                }
            }
            if(k == vec1.size())
            {
               ret_vec.push_back(vec2[i]); 
            }
        }
    }
    return ret_vec;
}

void print_reachability_hash()
{
    reachable_graph::iterator p;
    for(p = reachability_hash.begin() ; p != reachability_hash.end();p++)
    {
        std::string node_str = p->first;
        vector<graph_node* >vec = p->second;
        cout <<endl;
        cout<< "NODE: " << node_str << endl;  
        cout<< "REACHABLE NODES: " << endl;  
        for(int i = 0 ; i < vec.size() ;i++)
        {
            cout << vec[i]->get_reg_input_w() << ":" <<vec[i]->get_reg_value() << ":" << vec[i]->get_reg_output_w() << endl; 
        }
    }
}


bool check_if_node_in_vector(graph_node* node, vector<graph_node*>vec)
{
    for (int i = 0 ; i < vec.size() ; i++)
    {
        if((*vec[i]) == (*node))
          return true;
    }
    return false;
}

vector <graph_node*> get_downstream_of_cut(vector<reg_graph_edge*>ip_cut)
{
    vector<graph_node*> end_node_vec;
    for(int i = 0 ; i < ip_cut.size(); i++)
    {
        reg_graph_edge* curr_edge = ip_cut[i];
        graph_node* end_node = curr_edge->get_end_graph_node();
        if (!check_if_node_in_vector(end_node,end_node_vec))
        {
            end_node_vec.push_back(end_node);
        }
    }
    vector<graph_node*> ret_vec = get_reachability_vector_of_grp(end_node_vec);
    return ret_vec;
}

vector<graph_node*> flatten_grp(vector<graph_node*> ip_vec)
{
    vector<graph_node*>ret_vec;
    for (int i = 0 ; i < ip_vec.size();i++)
    {
        int grp_no = if_node_a_coglomerate_node(ip_vec[i]);
        if (grp_no == -1)
        {
            if (!check_if_node_in_vector(ip_vec[i],ret_vec))
            {
              ret_vec.push_back(ip_vec[i]);
            }
        }
        else
        {
            vector <graph_node*> island_nodes = loop_groups[grp_no];
            for (int j = 0 ; j < island_nodes.size(); j++)
            {
                if (!check_if_node_in_vector(island_nodes[j],ret_vec))
                {
                  ret_vec.push_back(island_nodes[j]);
                }
            }
        }
    }
    return ret_vec;
}


vector<graph_node* > form_reachability_graph(graph_node* node,int grp)
{
    vector<graph_node* >children_pruned = get_children_of_skl_graph_node_v2(node,true,grp);
    vector<graph_node* >children_actual = get_children_of_skl_graph_node_v2(node,false,grp);
    vector<graph_node* >reachability_vector;
    reachable_graph::iterator p;
    p = get_reachability_vector(node);
    int count = 0;
    if (p == reachability_hash.end())
    {
        if(children_pruned.size() == 0)
        {
           if (children_actual.size() == 0)
           {
              //output node
              if(!check_if_node_in_vector(node,reachability_vector))
                reachability_vector.push_back(node);
              reachability_hash[convert_graph_node_2_str(node)]=reachability_vector;
           }
           else
           {
              //loop might be present
              if (grp == 1)
                cout << "WARNING:: A reachability vector of a node in reachability hash has to be present in non loop graph with size(actual_children(node)) !=0 and size(children_pruned(children)) == 0";
              if(!check_if_node_in_vector(node,reachability_vector))
                reachability_vector.push_back(node);
           }
        }
        else
        {
            if(children_actual.size() == children_pruned.size())
            {
                  while(count < children_actual.size())
                   {
                       reg_graph_edge* curr_edge = get_edge_from_skl_graph_v2(node,children_actual[count],grp);
                       if(curr_edge != NULL)
                       {
                           curr_edge->set_visited(true);
                       }
                       vector<graph_node* >temp_vector = form_reachability_graph(children_actual[count],grp);
                       reachability_vector = merge_skl_graph_node_vectors(temp_vector,reachability_vector);
                       count++;
                   }
                  if(!check_if_node_in_vector(node,reachability_vector))
                    reachability_vector.push_back(node);
                  reachability_hash[convert_graph_node_2_str(node)]=reachability_vector;
            }
            else
            {
                //loop present
                if (grp == 1)
                  cout << "WARNING:: A non-loop graph cannot have  a node with size(pruned_children(node)) != size(actual_children(node)) && size(pruned_children) != 0 for a node";
                if(!check_if_node_in_vector(node,reachability_vector))
                  reachability_vector.push_back(node);
            }
        }
    }
    else
    {
        reachability_vector = p->second;
    }
    return reachability_vector;
}



void distribute_loop_nodes_to_grp(graph_node* node)
{
    vector<graph_node*> children = get_children_of_loop_graph_node_v2(node);
    int count = 0;
    while (count<children.size())
    {
        if(!check_if_node_in_vector(node,curr_loop_group))
          curr_loop_group.push_back(node);
        reg_graph_edge* curr_edge = get_edge_from_edges_in_loop_v2(node,children[count]);
        //reg_graph_edge* curr_edge = edges_in_loop_v2[convert_graph_node_2_str(node)][convert_graph_node_2_str(children[count])];
        if (curr_edge != NULL)
        {
          curr_edge->set_visited(true);
        }
        else
        {
            cout << "WARNING:: edge: ";
            print_skl_graph_node(node);
            cout << "->";
            print_skl_graph_node(children[count]);
            cout << " not found in loop edge database"<<endl;
        }
        node->set_visited(true);
        distribute_loop_nodes_to_grp(children[count]);
        count++;
    }
}

int get_group(graph_node* node)
{
    for (int i = 0 ; i < loop_groups.size(); i++)
    {
        for (int j = 0 ; j < loop_groups[i].size();j++)
        {
            if((*node) == (*loop_groups[i][j]))
            {
                return i;
            }
        }
    }
    return (-1);
}


vector<graph_node* > get_reachability_vector_of_grp(vector<graph_node* >vec)
{
    vector <graph_node*> union_vec;
    for(int i = 0 ; i < vec.size();i++)
    {
        vector<graph_node*> curr_vec = get_reachability_vector(vec[i])->second;
        union_vec = merge_skl_graph_node_vectors(union_vec,curr_vec);
    }
    return union_vec;

}


void print_skl_graph_node_vector(vector<graph_node*> vec)
{
    for (int i = 0 ; i < vec.size(); i++)
    {
        if(i<(vec.size()-1))
        {
            graph_node* curr = vec[i];
            print_skl_graph_node(curr);
            cout<<',';
        }
        else
        {
            graph_node* curr = vec[i];
            print_skl_graph_node(curr);
        }
    }
}

bool if_dummy(graph_node* node)
{
    std::string node_str = convert_graph_node_2_str(node);
    map<std::string,bool>::iterator p = dummy_nodes_v2.find(node_str);
    if (p != dummy_nodes_v2.end())
    {
        return true;
    }
    return false;
}

bool if_dummy_str(std::string node_str)
{
    map<std::string,bool>::iterator p = dummy_nodes_v2.find(node_str);
    if (p != dummy_nodes_v2.end())
    {
        return true;
    }
    return false;
}

void rebuild_dummy_node_vector(ifstream& fin)
{
    std::string line;
    while(std::getline(fin,line))
    {
        std::string token;
        std::string input_wire;
        std::string reg_value;
        std::string output_wire;
        std::string delimiter_node = ":";
        token = line;
        //cout<<token<<endl;
        token = trim(token);
        size_t pos_arrow = 0;
        std::string token_arrow;
        int count = 0;
        while((pos_arrow = token.find(delimiter_node)) != std::string::npos)
        {
            token_arrow = token.substr(0,pos_arrow);
            token.erase(0, pos_arrow + delimiter_node.length());
            if (count == 0)
            {
                input_wire = token_arrow;
            }
            else if (count == 1)
            {
                reg_value = token_arrow;
            }
            count++;
        }
        output_wire = token;
        graph_node* curr_dummy_node = get_skl_graph_node(reg_value,input_wire,output_wire,0);
        if(null_skl_graph_node(curr_dummy_node))
        {
            cout << "FATAL:: graph_node: " << convert_graph_node_2_str(curr_dummy_node) << " not found in graph"<<endl;
            exit(0);
        }
        dummy_nodes.push_back(curr_dummy_node);
        dummy_nodes_v2[convert_graph_node_2_str(curr_dummy_node)] = true;
    }
}

void rebuild_register_graph_nodes(ifstream& fin)
{
    std::string line;
    while(std::getline(fin,line))
    {
        std::string token;
        std::string input_wire;
        std::string reg_value;
        std::string output_wire;
        std::string delimiter_node = ":";
        token = line;
        //cout<<token<<endl;
        token = trim(token);
        size_t pos_arrow = 0;
        std::string token_arrow;
        int count = 0;
        while((pos_arrow = token.find(delimiter_node)) != std::string::npos)
        {
            token_arrow = token.substr(0,pos_arrow);
            token.erase(0, pos_arrow + delimiter_node.length());
            if (count == 0)
            {
                input_wire = token_arrow;
            }
            else if (count == 1)
            {
                reg_value = token_arrow;
            }
            count++;
        }
        output_wire = token;
        graph_node* curr_node = get_skl_graph_node(reg_value,input_wire,output_wire,0);
        if(null_skl_graph_node(curr_node))
        {
            cout << "FATAL:: graph_node: " << convert_graph_node_2_str(curr_node) << " not found in graph"<<endl;
            exit(0);
        }
        register_graph_nodes.push_back(curr_node);
    }
}




void rebuild_skl_graph(ifstream& fin1,ifstream& fin2)
{
    std::string line;
    //cout << "saurabh"<<endl;
    while(std::getline(fin1,line))
    {
        
        std::string delimiter = "=";
        size_t pos = 0;
        std::string token;
        std::string start_node = "";
        std::string start_node_input_wire="";
        std::string start_node_reg="";
        std::string start_node_output_wire="";
        std::string end_node = "";
        std::string end_node_input_wire="";
        std::string end_node_reg="";
        std::string end_node_output_wire="";
        std::string weight = "";
        
        pos = line.find(delimiter);
        token = line.substr(0, pos);
        token = trim(token);
        std::string delimiter_edge = "->";
        size_t pos1 = 0;
        std::string token2;
        while((pos1 = token.find(delimiter_edge)) != std::string::npos)
        {
            token2 = token.substr(0,pos1);
            start_node = token2;
            token.erase(0, pos1 + delimiter_edge.length());
            end_node = token;
        }
        //cout << start_node <<endl;
        line.erase(0, pos + delimiter.length());
        weight = line;


        std::string delimiter_node = ":";
        token = start_node;
        size_t pos_arrow = 0;
        std::string token_arrow;
        int count = 0;
        while((pos_arrow = token.find(delimiter_node)) != std::string::npos)
        {
            token_arrow = token.substr(0,pos_arrow);
            token.erase(0, pos_arrow + delimiter_node.length());
            if (count == 0)
            {
                start_node_input_wire = token_arrow;
            }
            else if (count == 1)
            {
                start_node_reg = token_arrow;
            }
            count++;
        }
        start_node_output_wire = token;
        count = 0;

        token = end_node;
        while((pos_arrow = token.find(delimiter_node)) != std::string::npos)
        {
            token_arrow = token.substr(0,pos_arrow);
            token.erase(0, pos_arrow + delimiter_node.length());
            if (count == 0)
            {
                end_node_input_wire = token_arrow;
            }
            else if (count == 1)
            {
                end_node_reg = token_arrow;
            }
            count++;
        }
        std::string::size_type sz;
        double doub_weight = atof(weight.c_str());

        end_node_output_wire = token;
        graph_node* start_obj = get_skl_graph_node(start_node_reg,start_node_input_wire,start_node_output_wire,0);
        if(null_skl_graph_node(start_obj))
        {
            delete start_obj;
            start_obj = new graph_node(start_node_reg,start_node_input_wire,start_node_output_wire);
            skl_graph_nodes.push_back(start_obj);
            skl_graph_nodes_v2[convert_graph_node_2_str(start_obj)] = start_obj;
        }

        graph_node* end_obj = get_skl_graph_node(end_node_reg,end_node_input_wire,end_node_output_wire,0);
        if(null_skl_graph_node(end_obj))
        {
            delete end_obj;
            end_obj = new graph_node(end_node_reg,end_node_input_wire,end_node_output_wire);
            skl_graph_nodes.push_back(end_obj);
            skl_graph_nodes_v2[convert_graph_node_2_str(end_obj)] = end_obj;
        }
        reg_graph_edge* curr_edge = new reg_graph_edge(start_obj,end_obj);
        curr_edge->set_weight(doub_weight);
        skl_graph.push_back(curr_edge);
        skl_graph_v2[convert_graph_node_2_str(start_obj)][convert_graph_node_2_str(end_obj)] = curr_edge;
    }

    while(std::getline(fin2,line))
    {
        std::string token;
        std::string input_wire;
        std::string reg_value;
        std::string output_wire;
        std::string delimiter_node = ":";
        token = line;
        token = trim(token);
        size_t pos_arrow = 0;
        std::string token_arrow;
        int count = 0;
        while((pos_arrow = token.find(delimiter_node)) != std::string::npos)
        {
            token_arrow = token.substr(0,pos_arrow);
            token.erase(0, pos_arrow + delimiter_node.length());
            if (count == 0)
            {
                input_wire = token_arrow;
            }
            else if (count == 1)
            {
                reg_value = token_arrow;
            }
            count++;
        }
        output_wire = token;
        //cout << input_wire<<"->"<<reg_value<<"->"<<output_wire<<endl;
        graph_node* curr_node = get_skl_graph_node(reg_value,input_wire,output_wire,0);
        if(null_skl_graph_node(curr_node))
        {
            curr_node = new graph_node(reg_value,input_wire,output_wire);
            cout << "WARNING::graph_node: " << convert_graph_node_2_str(curr_node) << " not found in graph"<<endl; 
            skl_graph_nodes.push_back(curr_node);
            skl_graph_nodes_v2[convert_graph_node_2_str(curr_node)] = curr_node;
        }
    }
}



void mark_all_skl_graph_nodes(bool value,int grp)
{
    vector<graph_node*>* vec = (grp == 0)?(&skl_graph_nodes):(&modified_skl_graph_nodes);
    for(int i = 0 ; i < vec->size();i++)
    {
        (*vec)[i]->set_visited(value);
    }
}


void mark_all_skl_graph_edges(bool value,int grp)
{
    reg_graph* curr_graph = (grp == 0)?(&skl_graph):(&modified_skl_graph);
    for(int i = 0 ; i < curr_graph->size() ; i++)
    {
        (*curr_graph)[i]->set_visited(value);
    }
}

void mark_all_edges_in_loop(bool value)
{
    for(int i = 0 ; i < edges_in_loop.size(); i++)
    {
        edges_in_loop[i]->set_visited(value);
    }
}


bool found_in_stack(graph_node* element,stack<graph_node*>* univ_stack)
{
    stack<graph_node*> brand_new_stack;
    bool found = false;
    while(!univ_stack->empty() && !found)
    {
        graph_node* top_el = univ_stack->top();
        if ((*element) == (*top_el))
        {
            found = true;
        }
        else
        {
            univ_stack->pop();
            brand_new_stack.push(top_el);
        }
    }
    while(!brand_new_stack.empty())
    {
        graph_node* top_el = brand_new_stack.top();
        if (found)
        {
            graph_node* top_el_univ = univ_stack->top();
            //reg_graph_edge* curr_edge = get_edge_from_skl_graph(top_el_univ,top_el,true,0);
            reg_graph_edge* curr_edge = get_edge_from_skl_graph_v2(top_el_univ,top_el,0);
            if (curr_edge != NULL && !if_edge_in_loop_data_base_v2(curr_edge))
            {
                edges_in_loop.push_back(curr_edge);
                edges_in_loop_v2[convert_graph_node_2_str(top_el_univ)][convert_graph_node_2_str(top_el)] = curr_edge;
            }
        }
        univ_stack->push(top_el);
        brand_new_stack.pop();
    }
    if(found)
    {
        graph_node* top_el_univ = univ_stack->top();
        //reg_graph_edge* curr_edge = get_edge_from_skl_graph(top_el_univ,element,true,0);
        reg_graph_edge* curr_edge = get_edge_from_skl_graph_v2(top_el_univ,element,0);
        if (curr_edge != NULL && !if_edge_in_loop_data_base_v2(curr_edge))
        {
            edges_in_loop.push_back(curr_edge);
            edges_in_loop_v2[convert_graph_node_2_str(top_el_univ)][convert_graph_node_2_str(element)] = curr_edge;
        }
    }
    return found;
}

void find_loops_in_graph(graph_node* node,stack<graph_node*>* univ_stack)
{
    vector<graph_node*> children = get_children_of_skl_graph_node_v2(node,false,0);
    for (int i = 0 ; i < children.size(); i++)
    {
        if(!found_in_stack(children[i],univ_stack))
        {
            univ_stack->push(children[i]);
            find_loops_in_graph(children[i],univ_stack);
            univ_stack->pop();
        }
    }
    
}


void find_nodes_in_loop()
{
    for(int i = 0 ; i < edges_in_loop.size();i++)
    {
       if(!check_if_node_in_vector(edges_in_loop[i]->get_start_graph_node(),nodes_in_loop))
        nodes_in_loop.push_back(edges_in_loop[i]->get_start_graph_node());
        nodes_in_loop_v2[convert_graph_node_2_str(edges_in_loop[i]->get_start_graph_node())] = edges_in_loop[i]->get_start_graph_node();
       if(!check_if_node_in_vector(edges_in_loop[i]->get_end_graph_node(),nodes_in_loop))
        nodes_in_loop.push_back(edges_in_loop[i]->get_end_graph_node());
        nodes_in_loop_v2[convert_graph_node_2_str(edges_in_loop[i]->get_end_graph_node())] = edges_in_loop[i]->get_end_graph_node();
    }
}

void print_nodes_in_loop()
{
    print_skl_graph_node_vector(nodes_in_loop);
}

bool if_edge_in_loop_data_base(reg_graph_edge* edge)
{
    for(int i = 0 ; i < edges_in_loop.size();i++)
    {
        if((*(edges_in_loop[i])) == (*edge))
        {
            return true;
        }
    }
    return false;
}



void print_edges_in_loops()
{
    for(int i = 0 ; i < edges_in_loop.size() ;i++)
    {
        reg_graph_edge* curr_edge = edges_in_loop[i];
        graph_node* st_node = curr_edge->get_start_graph_node();
        graph_node* end_node = curr_edge->get_end_graph_node();
        cout << st_node->get_reg_input_w()<<":"<<st_node->get_reg_value()<<":"<<st_node->get_reg_output_w()<<"->"<< end_node->get_reg_input_w()<<":"<<end_node->get_reg_value()<<":"<<end_node->get_reg_output_w()<<endl;
    }
}


cutset_graph::iterator get_set_of_cutset_for_skl_graph_node(graph_node* node)
{
    std::string node_str = convert_graph_node_2_str(node);
    cutset_graph::iterator p = cutset_hash.find(node_str);
    return p;
}

void print_set_of_cutset_for_node(graph_node* node)
{
    cutset_graph::iterator p;
    p = get_set_of_cutset_for_skl_graph_node(node);
    vector<vector<reg_graph_edge*> > return_set_of_cutset = p->second;
    print_set_of_cutsets(return_set_of_cutset);
}

vector<vector<reg_graph_edge*> >set_of_cut_set(graph_node* node,reg_graph_edge* edge,type_of_cutsets type, int grp, bool print_percent)
{    

      
      vector<vector<reg_graph_edge*> > return_set_of_cutset;
      vector<graph_node* >merged_children;
      cutset_graph::iterator p;
      p = get_set_of_cutset_for_skl_graph_node(node);
      int count = 0;
      if(p == cutset_hash.end())
      {   
          vector<graph_node* >children_pruned = get_children_of_skl_graph_node_v2(node,true,grp);
          vector<graph_node* >children_actual = get_children_of_skl_graph_node_v2(node,false,grp);
          //cout<<"Did not found in hash table"<<endl;
          vector<vector<reg_graph_edge*> > current_set_of_cutset;
          vector<reg_graph_edge*> cutset_with_incoming_edge;
          if (edge!= NULL)
          {
            cutset_with_incoming_edge.push_back(edge);
          }
          if(children_actual.size()> children_pruned.size() && children_pruned.size() == 0)
          {
              //loop:Let # of children = 2 and # of actual children = 3 && 
              //set of cutset also not in cutset hash
              //implies that the same node has been visited again
              // # of children == 0 and not present in the cutset hash not possible!!!!!
              // dont even insert cutset set in the hash because it will be taken care off later
              //graph_node* try_node = get_skl_graph_node("1","win_1","wout_1",0);
              //if ((*try_node) == (*node))
              //{
              //    print_set_of_cutsets(return_set_of_cutset);
              //}
              if (grp == 1)
              {
                  cout << "Warning:: impossible condition: children_actual.size()> children_pruned.size() && children_pruned.size() == 0 && set of cutset for node not present in hash, in non-loop graph";
              }
              return return_set_of_cutset;     
          }
          else if (children_actual.size()==children_pruned.size() && children_pruned.size()==0)
          {
              //output node
              //do nothing
              //this condition implies that node is a output node and needs to be placed
              //in hash table of cutset
              //only incoming edge in the hash of this node
          }
          else if (children_actual.size() == children_pruned.size() && children_pruned.size()>0 ) 
          {
              vector<graph_node*>already_merged_children;
              while(count<children_actual.size())
              {
                  //find set of cutset recursively
                  //graph_node* try_node = get_skl_graph_node("0","win_0","wout_0",0);
                  //if ((*try_node) == (*node))
                  //{
                  //    cout << "For node:";
                  //    print_skl_graph_node(children_actual[count]);
                  //}
                  reg_graph_edge* curr_edge= get_edge_from_skl_graph_v2(node,children_actual[count],grp);
                  if (curr_edge != NULL)
                  {
                    curr_edge->set_visited(true);
                  }
                  else
                  {
                      cout << "WARNING:: edge ";
                      print_skl_graph_node(node);
                      cout << "->";
                      print_skl_graph_node(children_actual[count]);
                      cout << " not found in skl_graph"<<endl;
                  }
                  //if(print_percent)
                  //{
                  //    cout<< "("<<(double)no_of_edges*100/modified_skl_graph_v2.size()<<"%) complete" <<endl;
                  //}
                  //no_of_edges = no_of_edges+1;
                  current_set_of_cutset = set_of_cut_set(children_actual[count],curr_edge,type,grp,print_percent);
                  //if ((*try_node) == (*node))
                  //{
                  //    cout << "current set of cutset is"<<endl;;
                  //    print_set_of_cutsets(current_set_of_cutset);
                  //}
                  return_set_of_cutset = merge_cutsets(current_set_of_cutset,children_actual[count],return_set_of_cutset,already_merged_children,type,grp);
                  //if ((*try_node) == (*node))
                  //{
                  //    cout << "merged set of cutset is"<<endl;;
                  //    print_set_of_cutsets(return_set_of_cutset);
                  //}
                  already_merged_children.push_back(children_actual[count]);
                  count++;
              }
          }
          no_of_nodes = no_of_nodes+1;
          if(print_percent)
          {
              cout<< "("<<(double)no_of_nodes*100/modified_skl_graph_nodes_v2.size()<<"%) nodes in cutset hash" <<endl;
          }
          cutset_hash[convert_graph_node_2_str(node)] = return_set_of_cutset;
          if(cutset_with_incoming_edge.size()>0)
            return_set_of_cutset.push_back(cutset_with_incoming_edge);
          //graph_node* try_node = get_skl_graph_node("1","win_1","wout_1",0);
          //if ((*try_node) == (*node))
          //{
          //    print_set_of_cutsets(return_set_of_cutset);
          //}
      }
      else
      {
          //cout<<"found in hash table"<<endl;
          vector<reg_graph_edge*> cutset_with_incoming_edge;
          if (edge!= NULL)
          {
            cutset_with_incoming_edge.push_back(edge);
          }
          return_set_of_cutset = p->second;
          if(cutset_with_incoming_edge.size()>0)
            return_set_of_cutset.push_back(cutset_with_incoming_edge);
          //if(children_pruned.size()> 0)
          //{
          //    cout << "Warning:: set_of_cut_set::  a node not covered completely, but still in cutset hash"<<endl;
          //}
      }
     return return_set_of_cutset;
}

int if_node_a_coglomerate_node(graph_node* node)
{
    std::string reg = node->get_reg_value();
    std::string ip_w = node->get_reg_input_w();
    std::string op_w = node->get_reg_input_w();
    std::string string_match = "loop_grp";
    std::size_t found_1 = ip_w.find(string_match);
    std::size_t found_2 = op_w.find(string_match);
    if (found_1 == std::string::npos)
    {
        return -1;
    }
    else
    {
        if(if_one_of_the_skl_graph_nodes_v2(node,1))
        {
          int return_value;
          istringstream (node->get_reg_value()) >> return_value;
          return return_value;
        }
        else
        {
            return -1;
        }
    }

}

vector<reg_graph_edge*> get_edges_btwn_2_grps(vector<graph_node*> start_grp, vector<graph_node*> end_grp)
{
    vector <reg_graph_edge*> return_vec;
    for (int i = 0 ; i < start_grp.size() ; i++)
    {
        vector<graph_node*> children = get_children_of_skl_graph_node_v2(start_grp[i],false,0);
        for (int j = 0 ; j < end_grp.size() ; j++)
        {
            reg_graph_edge* hypothetical_edge = get_edge_from_skl_graph_v2(start_grp[i],end_grp[j],0);
            if (hypothetical_edge!=NULL)
            {
                return_vec.push_back(hypothetical_edge);
            }

        }
    }
    return return_vec;
}


vector<vector<reg_graph_edge*> > modify_set_of_cut_set(vector<vector<reg_graph_edge*> >* set_of_cutset)
{
    
    vector<vector<reg_graph_edge*> > return_set_of_cutset;
    for(int i = 0; i < set_of_cutset->size();i++)
    {
        vector<reg_graph_edge*>* curr_cut = &(*set_of_cutset)[i];
        vector<reg_graph_edge*> result_cut;
        for(int j =0; j < curr_cut->size() ; j++)
        {
            reg_graph_edge* curr_edge = (*curr_cut)[j];
            graph_node* start_node = curr_edge->get_start_graph_node();
            int start_grp_no = if_node_a_coglomerate_node(start_node);
            graph_node* end_node = curr_edge->get_end_graph_node();
            int end_grp_no = if_node_a_coglomerate_node(end_node);
            if(start_grp_no == -1 && end_grp_no == -1)
            {
                  //normal edge
                  result_cut.push_back(curr_edge);
            }
            else if (start_grp_no == -1 && end_grp_no != -1)
            {
                vector<graph_node*> start_grp;
                vector<graph_node*> end_grp;
                start_grp.push_back(start_node);
                end_grp = loop_groups[end_grp_no];
                vector<reg_graph_edge*> edges_btwn_2_grps = get_edges_btwn_2_grps(start_grp,end_grp);
                for(int k = 0 ; k < edges_btwn_2_grps.size();k++)
                {
                  result_cut.push_back(edges_btwn_2_grps[k]);
                }
            }
            else if (start_grp_no != -1 && end_grp_no == -1)
            {
                //
                vector<graph_node*> start_grp;
                vector<graph_node*> end_grp;
                end_grp.push_back(end_node);
                start_grp = loop_groups[start_grp_no];
                vector<reg_graph_edge*> edges_btwn_2_grps = get_edges_btwn_2_grps(start_grp,end_grp);
                for(int k = 0 ; k < edges_btwn_2_grps.size();k++)
                {
                  result_cut.push_back(edges_btwn_2_grps[k]);
                }
            }
            else if (start_grp_no != -1 && end_grp_no != -1)
            {
                vector<graph_node*> start_grp;
                vector<graph_node*> end_grp;
                end_grp = loop_groups[end_grp_no];
                start_grp = loop_groups[start_grp_no];
                vector<reg_graph_edge*> edges_btwn_2_grps = get_edges_btwn_2_grps(start_grp,end_grp);
                for(int k = 0 ; k < edges_btwn_2_grps.size();k++)
                {
                  result_cut.push_back(edges_btwn_2_grps[k]);
                }
            }
        }
        return_set_of_cutset.push_back(result_cut);
    }
    return return_set_of_cutset;
}


bool filter_out(vector<reg_graph_edge*> cut)
{
    double max_level = 100; // max of max level of cut end node
    double min_level = 100; //min of min level of cut end node
    for (int i = 0 ; i < cut.size(); i++)
    {
        reg_graph_edge* curr_edge = cut[i];
        graph_node* start_node = curr_edge->get_start_graph_node();
        //filter1 = if any node belonging to S(c) is a coglomerate node discard that cutset
        if(if_node_a_coglomerate_node(start_node) > -1)
          return true;
        //filter2 = if max (accross all start max(level)) - min (accross all the endnodes min(level)) > max_tolerable_level_diff, discard that cutset
        else
        {
            //graph_node* end_node = curr_edge->get_end_graph_node();
            double curr_level_max = level_hash[convert_graph_node_2_str(start_node)][1];
            double curr_level_min = level_hash[convert_graph_node_2_str(start_node)][0];
            if (i == 0)
            {
                max_level = curr_level_max;
                min_level = curr_level_min;
            }
            else 
            {
                if (curr_level_min < min_level)
                  min_level = curr_level_min;
                if (curr_level_max > max_level)
                  max_level = curr_level_max;
                if ((max_level-min_level) > max_tolerable_level_diff) 
                  return true;
            }
        }
    }

}
//to filter out the cuts containing edges that belong to dummy->input or output->dummy
bool filter_out2(vector<reg_graph_edge*>cut)
{
    for(int i = 0; i < cut.size() ; i++)
    {
        reg_graph_edge* curr_edge = cut[i];
        graph_node* start_node = curr_edge->get_start_graph_node();
        if (if_dummy(start_node))
        {
            return true;
        }
    }
    return false;
}


vector<vector<reg_graph_edge*> >merge_cutsets( vector<vector<reg_graph_edge*> >set_of_cutset,graph_node* of_child,vector<vector<reg_graph_edge*> >merged_set_of_cutset,vector<graph_node* >of_children,type_of_cutsets type,int grp)
{
 
    vector<vector<reg_graph_edge*> >return_set_of_cutset;   
    if(merged_set_of_cutset.size() == 0)
    {
        return set_of_cutset;
    }
    else
    {
        if (set_of_cutset.size()>0)
        {
            vector<graph_node* >of_children_reachability;
            vector<graph_node* >of_child_reachability;
            reachable_graph::iterator p;
            p = get_reachability_vector(of_child);
            if(p!= reachability_hash.end())
                of_child_reachability = p->second;
            else
                cout << "WARNING::merge_cutsets::node not found in reachability hash"<<endl;
            vector<graph_node* > group_1_of_child;
            vector<graph_node* > group_2_of_child;
            vector<graph_node* > group_1_of_children;
            vector<graph_node* > group_2_of_children;
            for (int i = 0 ; i < of_children.size() ;i++)
            {
                vector<graph_node* >curr_reachability;
                p=get_reachability_vector(of_children[i]);
                if(p!= reachability_hash.end())
                    curr_reachability = p->second;
                else
                    cout << "WARNING::merge_cutsets::node not found in reachability hash"<<endl;
                of_children_reachability = merge(curr_reachability,of_children_reachability);
            }
            for(int i = 0 ; i < set_of_cutset.size();i++)
            {
                vector<reg_graph_edge* >cut_set_merged;
                group_2_of_child = get_group2(set_of_cutset[i]);
                group_1_of_child = subtract_group(of_child_reachability,group_2_of_child);
                for (int k = 0 ; k < merged_set_of_cutset.size();k++)
                {
                    group_2_of_children = get_group2(merged_set_of_cutset[k]);
                    group_1_of_children = subtract_group(of_children_reachability,group_2_of_children);
                    if(!intersection(group_1_of_child,group_2_of_children) && !intersection(group_2_of_child,group_1_of_children))
                    {
                          cut_set_merged = merge_2_cutsets(merged_set_of_cutset[k],set_of_cutset[i]);
                          ///add a filter(s) here for cutsets
                          //filter 1: to filter out the cuts containing edge with starting node belonging to loop or the difference between max(level)-min(level) of cut<max_tolerable_level_diff
                          bool remove = filter_out(cut_set_merged);
                          //filter 2: to remove the cuts that contain S(c) intersection DN(G) = non-empty set
                          if (!remove)
                          {
                              remove = filter_out2(cut_set_merged);
                          }
                          //filter3: only bypassable cuts
                          if(!remove)
                          {
                              if(type == bypassable)
                              {
                                  vector<graph_node*>merged_group = merge(group_1_of_children,group_1_of_child);
                                  remove = !bypassable_cutset(cut_set_merged,merged_group,grp);
                              }
                          }

                          //if(type == feed_fwd)
                          //{
                              if (!remove)
                                return_set_of_cutset.push_back(cut_set_merged);
                          //}
                          //else
                          //{
                          //    vector<graph_node*>merged_group = merge(group_1_of_children,group_1_of_child);
                          //    if(bypassable_cutset(cut_set_merged,merged_group,grp) && !remove)
                          //    {
                          //        return_set_of_cutset.push_back(cut_set_merged);
                          //    }
                          //}
                    }
                }
            }
        }
        else
        {
            return merged_set_of_cutset;
        }
    }
    return return_set_of_cutset;
}

bool bypassable_cutset(vector<reg_graph_edge* >c1,vector<graph_node* >group1,int grp)
{
    for(int i  = 0 ; i < c1.size() ;i++ )
    {
        graph_node* start_node = c1[i]->get_start_graph_node();
        vector<graph_node* >children = get_children_of_skl_graph_node_v2(start_node,false,grp);
        if(intersection(children,group1))
        {
            return false;
        }
    }
   return true;
}

vector<reg_graph_edge* >merge_2_cutsets(vector<reg_graph_edge* >c1,vector<reg_graph_edge*>c2)
{
    vector<reg_graph_edge* >c_return;
    c_return = c2;
    for(int i = 0 ; i < c1.size() ;i++)
    {
        int k;
        for (k = 0 ; k < c2.size() ; k++)
        {
            if((*(c2[k])) == (*(c1[i])))
            {
                break;
            }
        }
        if( k == c2.size())
            c_return.push_back(c1[i]);
    }
    return c_return;
}


bool intersection (vector<graph_node* >group1,vector<graph_node* >group2)
{
    for(int i = 0 ; i < group1.size();i++)
    {
        for(int k =0 ; k < group2.size();k++)
        {
            if((*(group1[i])) == (*(group2[k])))
                return true;
        }
    }
    return false;
}

vector<graph_node* >get_group2(vector<reg_graph_edge*>c1)
{
    vector<graph_node* >group2;
    reachable_graph::iterator p;
    vector<graph_node* >curr_group;
    for(int i = 0 ; i < c1.size();i++)
    {
        graph_node* end_node = c1[i]->get_end_graph_node();
        p = get_reachability_vector(end_node);
        if(p!=reachability_hash.end())
            curr_group = p->second;
        else
            cout<<"WARNING::get_group2::node not found in map7"<<endl;
        group2 = merge(curr_group,group2);
    }
    return group2;
}

vector<graph_node* >merge(vector<graph_node* >vec1,vector<graph_node* >vec2)
{
    vector<graph_node* >ret_vec;
    ret_vec = vec1;
    if(vec2.size() == 0)
    {
        return vec1;
    }
    else
    {
        for(int i = 0 ; i < vec2.size();i++)
        {
            int k;
            for(k = 0 ; k < vec1.size();k++)
            {
                if((*(vec2[i]))==(*(vec1[k])))
                {
                    break;
                }
            }
            if(k == vec1.size())
            {
               ret_vec.push_back(vec2[i]); 
            }
        }
    }
    return ret_vec;
}

vector<graph_node* >subtract_group(vector<graph_node* >bigger_group,vector<graph_node* >smaller_group)
{
    for(int i = 0 ; i < smaller_group.size();i++)
    {
        int k;
        for(k = 0 ; k < bigger_group.size() ; k++)
        {
            if((*(smaller_group[i])) == (*(bigger_group[k])))
                break;
        }
        if(k<bigger_group.size())
          bigger_group.erase(bigger_group.begin()+k);
        else
            cout << "WARNING::subtract_group::bigger group is missing some of the nodes of smaller group !!!!!"<<endl;
    }
    return bigger_group;
}


void print_set_of_cutsets(vector<vector<reg_graph_edge*> >set_of_cutset)
{
    for(int i = 0 ; i < set_of_cutset.size();i++)
    {
        //cout << set_of_cutset[i].size() << endl;
        for (int k = 0 ; k < set_of_cutset[i].size();k++)
        {

            if (k < (set_of_cutset[i].size()-1))
            {
              print_skl_graph_node((set_of_cutset[i][k]->get_start_graph_node()));
              cout << "->";
              print_skl_graph_node(set_of_cutset[i][k]->get_end_graph_node());
              cout<< "  ,  ";
            }
            else
            {
              print_skl_graph_node((set_of_cutset[i][k]->get_start_graph_node()));
              cout << "->";
              print_skl_graph_node(set_of_cutset[i][k]->get_end_graph_node());
            }
        }
        cout <<endl;
    }
}


//void print_skl_graph_to_file(ofstream& file)
void print_2D_vec_to_file(ofstream& file,vector<vector<graph_node*> >vec)
{
    for(int i = 0 ; i < vec.size(); i++)
    {
        for (int j = 0 ; j < vec[i].size(); j++)
        {
           if (j < (vec[i].size()-1))
           {
              file << vec[i][j]->get_reg_input_w()<<':'<<vec[i][j]->get_reg_value()<<':'<<vec[i][j]->get_reg_output_w()<<',' ;
           }
           else
           {
              file << vec[i][j]->get_reg_input_w()<<':'<<vec[i][j]->get_reg_value()<<':'<<vec[i][j]->get_reg_output_w();
           }
        }
        file<<endl;
    }
}

void print_loops_to_file(ofstream& fout)
{
    for (int i = 0 ; i < edges_in_loop.size();i++)
    {
        graph_node* start_node = edges_in_loop[i]->get_start_graph_node();
        graph_node* end_node = edges_in_loop[i]->get_end_graph_node();
        double weight  = edges_in_loop[i]->get_weight();
        fout << start_node->get_reg_input_w() << ":" << start_node->get_reg_value() << ":" << start_node->get_reg_output_w()<<"->"<<end_node->get_reg_input_w() << ":" << end_node->get_reg_value() << ":" << end_node->get_reg_output_w()<<"="<<weight<<endl;
    }
}

void print_set_of_cutsets_to_file(ofstream& file,vector<vector<reg_graph_edge*> >set_of_cutset)
{
    for(int i = 0 ; i < set_of_cutset.size(); i++)
    {
        for(int j = 0 ; j < set_of_cutset[i].size(); j++)
        {
            graph_node* start_node = set_of_cutset[i][j]->get_start_graph_node();
            graph_node* end_node = set_of_cutset[i][j]->get_end_graph_node();
            if (j < (set_of_cutset[i].size()-1))
            {
              file << start_node->get_reg_input_w() << ':' << start_node->get_reg_value() << ':' << start_node->get_reg_output_w()<<"->"<<end_node->get_reg_input_w() << ':' << end_node->get_reg_value() << ':' << end_node->get_reg_output_w()<<','; 
            }
            else
            {
              file << start_node->get_reg_input_w() << ':' << start_node->get_reg_value() << ':' << start_node->get_reg_output_w()<<"->"<<end_node->get_reg_input_w() << ':' << end_node->get_reg_value() << ':' << end_node->get_reg_output_w(); 
            }
        }
        file<<endl;
    }
}


vector<vector<reg_graph_edge*> >exhaustive_set_of_cutsets()
{
   vector<vector<reg_graph_edge*> > possible_set_of_cutsets;
   for(int i = 1 ; i <= skl_graph_nodes.size()-1 ;i++)
   {
        vector<vector<graph_node*> > group1_possibilities; 
        vector<vector<graph_node*> > group2_possibilities;
       if(i <= skl_graph_nodes.size()/2)
       {
          group1_possibilities = get_all_combinations(i,false);
       }
       else
       {
          group2_possibilities = get_all_combinations((skl_graph_nodes.size()-i),true);
       }
       vector<vector<graph_node*> > group_computed = (group1_possibilities.size() > 0)?group1_possibilities:group2_possibilities;
       for(int k = 0 ; k < group_computed.size() ; k++)
       {
           vector<graph_node*> group1; 
           vector<graph_node*> group2;
           if(group1_possibilities.size()>0)
           {
                group1 = group_computed[k]; 
                group2 = subtract_group(skl_graph_nodes,group1);
           }
           else
           {
                group2 = group_computed[k]; 
                group1 = subtract_group(skl_graph_nodes,group2);
           }
           vector<reg_graph_edge* > possible_cutset = get_cut_set_for_known_group(group1,group2);
           bool validate = validate_cutset(group1,group2,rev_graph);
           if(validate && possible_cutset.size()!=0 && !duplicate_cutset(possible_set_of_cutsets,possible_cutset))
               possible_set_of_cutsets.push_back(possible_cutset);
       }
   }
   return possible_set_of_cutsets;
}

vector<reg_graph_edge* > get_cut_set_for_known_group(vector<graph_node*>group1,vector<graph_node*>group2)
{
    vector<graph_node*>children;
    vector<reg_graph_edge* > return_cutset;
    for (int i = 0 ; i < group1.size() ; i++)
    {
        children = get_children_of_skl_graph_node(group1[i],false,0);
        children = get_intersecting_children(children,group2);
        for(int j = 0 ; j < children.size();j++)
        {
          reg_graph_edge* edge = get_edge_from_skl_graph(group1[i],children[j],false,0);
          return_cutset.push_back(edge);
        }
    }
    return return_cutset;
}

vector<graph_node*> get_intersecting_children (vector<graph_node* >group1,vector<graph_node* >group2)
{
    vector<graph_node*>intersect;
    for(int i = 0 ; i < group1.size();i++)
    {
        for(int k =0 ; k < group2.size();k++)
        {
            if((*(group1[i])) == (*(group2[k])))
            {
                intersect.push_back(group1[i]);
                break;
            }
        }
    }
    return intersect;
}

vector<vector<graph_node*> > get_all_combinations(int no_of_nodes,bool get_from_table)
{
    vector<vector<graph_node*> > return_vec;
    if(!get_from_table)
    {
      combinations::iterator p_map10;
      p_map10 = map10.find(no_of_nodes-1);
      if(p_map10!=map10.end())
      {
          vector<vector<graph_node*> > one_level_less_vec = p_map10->second;
          for (int i = 0 ; i < one_level_less_vec.size();i++)
          {
             vector<graph_node*> subtract = subtract_group(skl_graph_nodes,one_level_less_vec[i]);
             for (int j = 0 ; j < subtract.size(); j++)
             {
                 vector<graph_node*> current_vec_concatanate = one_level_less_vec[i];
                 current_vec_concatanate.push_back(subtract[j]);
                 if(!duplicate(return_vec,current_vec_concatanate))
                      return_vec.push_back(current_vec_concatanate);
             }
          }
      }
      else
      {
          if(no_of_nodes == 1)
          {
              for(int i = 0 ; i < skl_graph_nodes.size() ; i++)
              {
                  vector<graph_node*> current_vec_concatanate;
                  current_vec_concatanate.push_back(skl_graph_nodes[i]);
                  return_vec.push_back(current_vec_concatanate);
              }
              map10.insert(std::pair<int,vector<vector<graph_node*> > >(no_of_nodes,return_vec));
              return return_vec;
          }
          else
          {
              cout << "WARNING1::get_all_combinations:: Couldn't find in map10" << endl;
              return return_vec;
          }
      }
      map10.insert(std::pair<int,vector<vector<graph_node*> > >(no_of_nodes,return_vec));
      return return_vec;
    }
    else
    {
      combinations::iterator p_map10;
      p_map10 = map10.find(no_of_nodes);
      if(p_map10!=map10.end())
      {
          return p_map10->second;
      }
      else
      {
          //cout << no_of_nodes << endl;
          cout << "WARNING2::get_all_combinations:: Couldn't find in map10" << endl;
          return return_vec;
      }
    }
}

bool validate_cutset(vector<graph_node*>group1,vector<graph_node*>group2,reg_graph in_graph)
{
      for(int i = 0 ; i < group1.size() ; i++)
      {
          graph_node* curr_group1_node = group1[i];
          int j;
          for(j = 0 ; j < group2.size();j++)
          {
              graph_node* curr_group2_node = group2[j];
              reg_graph_edge* curr_edge = get_edge_from_gen_graph(curr_group1_node,curr_group2_node,in_graph);
              if(curr_edge!=NULL)
                  return false;
          }

      }
return true;
}

bool duplicate_cutset(vector<vector<reg_graph_edge*> >vec_of_vec,vector<reg_graph_edge*>vec)
{
    for (int i = 0 ; i < vec_of_vec.size();i++)
    {
        vector<reg_graph_edge*> current_vec = vec_of_vec[i];
        bool match = match_2_vec_of_edge(current_vec,vec);
        if (match)
            return true;
    }
}

bool duplicate(vector<vector<graph_node*> >vec_of_vec,vector<graph_node*>vec)
{
    for (int i = 0 ; i < vec_of_vec.size();i++)
    {
        vector<graph_node*> current_vec = vec_of_vec[i];
        bool match = match_2_vec(current_vec,vec);
        if (match)
            return true;
    }
}


bool match_2_vec_of_edge(vector<reg_graph_edge*>vec1, vector<reg_graph_edge*>vec2)
{
    if(vec1.size()!=vec2.size())
        return false;
    else
    {
        for(int i = 0 ; i<vec1.size();i++)
        {
            reg_graph_edge* to_be_matched_node = vec1[i];
            int j;
            for( j = 0 ; j < vec2.size();j++)
            {
                reg_graph_edge* to_be_matched_with_node = vec2[j];
                if((*to_be_matched_with_node) == (*to_be_matched_node))
                    break;
            }
            if(j == vec2.size())
                return false;
        }
    }
    return true;
}

bool match_2_vec(vector<graph_node*>vec1, vector<graph_node*>vec2)
{
    if(vec1.size()!=vec2.size())
        return false;
    else
    {
        for(int i = 0 ; i<vec1.size();i++)
        {
            graph_node* to_be_matched_node = vec1[i];
            int j;
            for(j = 0 ; j < vec2.size();j++)
            {
                graph_node* to_be_matched_with_node = vec2[j];
                if((*to_be_matched_with_node) == (*to_be_matched_node))
                    break;
            }
            if(j == vec2.size())
                return false;
        }
    }
    return true;
}


reg_graph_edge* get_edge_from_gen_graph(graph_node* start_node,graph_node* end_node,reg_graph vec)
{
    reg_graph_edge edge(start_node,end_node);
    int i;
    for(i = 0 ; i < vec.size() ;i++)
    {
        if(edge == (*(vec[i])))
            return vec[i];
    }
    if(i == vec.size())
    {
        reg_graph_edge* null_edge = new reg_graph_edge();
        null_edge = NULL;
        return null_edge;
    }
}
reg_graph reverse_graph()
{
    reg_graph rev_graph;
    for(int i=0 ;i < skl_graph.size();i++)
    {
        reg_graph_edge* curr_edge = skl_graph[i];
        reg_graph_edge* new_rev_edge = new reg_graph_edge(curr_edge->get_end_graph_node(),curr_edge->get_start_graph_node());
        rev_graph.push_back(new_rev_edge);
    }
    return rev_graph;
}


int equivalance_of_set_of_cutsets(vector<vector<reg_graph_edge*> >set_of_cutset1,vector<vector<reg_graph_edge*> >set_of_cutset2)
{
    
    if(set_of_cutset1.size()  >= set_of_cutset2.size())
    {
        for(int i = 0 ; i < set_of_cutset2.size() ; i++)
        {
            vector<reg_graph_edge*> cutset_from_2 = set_of_cutset2[i];
            bool equal = false;
            int j;
            for(j = 0 ; j < set_of_cutset1.size() ; j++)
            {
                vector<reg_graph_edge*> cutset_from_1 = set_of_cutset1[j];
                equal = match_2_vec_of_edge(cutset_from_1,cutset_from_2);
                if(equal)
                    break;
            }
            if(!equal)
            {
                return -1;
            }
            //if (j ==set_of_cutset2.size())
            //{
            //    return false;
            //}
        }
        if (set_of_cutset2.size() == set_of_cutset1.size())
        {
          return 1;
        }
        else
        {
          return 0;
        }
    }
    else
    {
        return -1;
    }
}
///skeleton graph functions

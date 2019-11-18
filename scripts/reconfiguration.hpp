#include <fstream>
#include <iostream>
#include <cstring>
#include <memory>
#include <cstdio>
#include <map>
#include <queue>
#include <stack>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <time.h>
#include <string.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <string>
#include <algorithm>
#include <sys/timeb.h>

///technology dependent ports defined
#define clk_port "CP"                         ////clock port of flip-flops in a given technology
#define rstn_port "CDN"                       ////active-low reset port of flip-flops in a given technology
#define stn_port "SDN"                        ///active-high reset port of flip-flop in a given technology
#define neg_enable "EN"                         ///Enable pin in active low latch
#define enable "E"                          ///Enable pin active high latch
#define reg_out_port1 "Q"                     /// 1st output port of flip-flop in given technology
#define reg_out_port2 "QN"                    /// 2nd output port of flip-flop in a given tecnology
#define mux_sel_pin "S"                       /// select port of mux in given technology
#define mux_in_1 "I0"                         ///1st input port of mux in given technology
#define mux_in_2 "I1"                         ///2nd input port of mux in given technology
#define mux_out_pin "Z"                       ///output port of mux in given technology
#define clk_gate_in1 "CPN"                    ///clock input port in clock gate in given technology
#define clk_gate_in2 "E"                      ///Enable pin in clock gate in given technology
#define clk_gate_out "Q"                      ///Output pin of clock gate
#define clk_gate_in3 "TE"                     ///Extra input in clock gate

///technology dependent ports defined


//Implementation parameters of algorithm
#define progress_percentage true              ///tells the progress percentage of Tree formation and tree traversal for topological_analysis  
#define debugging false                       /// for invoking dugging prints in the code
#define defination false                      ///----------------- defination -------------------------/////
                                              /// true: feed-fwd registers can originate from different inputs as well
                                              /// false: feed-fwd registers can originat from same input only (use this defination0) 
                                              //----------------end defination-------------------------/////
                           

#define operation topological_analysis              ///-----------------------------------------operations---------------------------////
                                             /// Does following operations:
                                             ///1. input_insertion: Insert specified number of flip-flops from all primary inputs
                                             ///2. output_insertion: Insert specified number of flip-flops from all primary outputs
                                             ///3. in_out_insertion: Insert specified number of flip-flops from primary inputs and outputs
                                             ///4. n_slowing : Converts an architecture into n-slowing architecture 
                                             ///5. right_cut_insertion : Only for simple topology inserts a pipeline on right side of already existing pipeline and sets already existing pipeline as dont touch 
                                             ///6. cut_set_based_insertion: Identifies the best cut in the design and insert
                                             ///registers along that cut
                                             ///7. topological_analysis: Identifies the feed-fwd, linear and feed-back kind of flip-flops 
                                             ///in design, it also tells the information about the number of loops present in the design,
                                             ///and the number of feed-fwd paths in the design 
                                             ///8. bypass_arch: Inserts bypass circuitary in a linear architecture or non-linear architecture 
                                             ///9. weigh_edges: weighs the edges in graph (i.e. the critical path delay between any two given
                                             //flip-flops
                                             ///----------------------------------------end operations---------------------------////


#define report_no_of_paths true             ///reports the number of feed-fwd path 
#define topology_of_arch linear_top         /// can be of two types a.) non-linear arch b.) linear arch 
#define user_input false                    /// user input can be enabled for inputs line no of pipelines to be inserted, n slowing number etc
#define pipe_stage 1                        /// number of pipe stages to be inserted from input and output in "in_out_insertion"
#define pipe_stage_input 1                  /// number of pipe stages to be inserted from input in "input_insertion"
#define pipe_stage_output 2                 /// number of pipe stages to be inserted from output in "output_insertion"
#define n_slow_number 2                     /// number of n-slowing in "n_slowing"
#define bypass_no 3                         /// number of stages to be bypassed   
#define no_of_ff_per_clock_gate 20          /// number of flip-flops per clock gate   
#define comb_insertion true                 /// if input or output or in-out reg insertion is in a completely combination circuit
#define clk_wire_input "clk"                /// clock wire in the design (used only if comb_insertion is true)
#define debug_bypassing true               /// to debugging the algorithm for feed-fwd cutset finding with random graphs
#define put_clk_gating true                 /// whether to do clockgating or not   
#define no_of_pipe_stages 1                 ///no of pipestages in linear kind of topology2
#define ff_or_latch "flip-flop"                    ///insertion type from input
#define insert_separate_module true               
#define separate_module_file_path "/home/a0117963/sandbox19082015/intel_work/fft_bypassable_6_pipelined/pr_fft_files/rec_ff.sv"               

using namespace std;
const int by_pattern[] = {1};          ///1-to put bypass circuitary 0-not to put bypass circuitary 
typedef enum {linear,feedfwd,loopbk} reg_prop;
typedef enum {input_insertion,output_insertion,in_out_insertion,n_slowing,right_cut_insertion,cut_set_based_insertion,topological_analysis,bypass_arch,weigh_edges,none} type_of_operation; // type of operation enums
typedef enum {input,output} direction; 
typedef enum {linear_top,non_linear_top}topology; //gives the prelimenary idea about if design contains feed-fwd paths/loops or is completely linear
typedef enum {odd,even,neither}odd_even; 
typedef enum {feed_fwd,bypassable}type_of_cutsets; //type of cutsets
                                                   //1.feed_fwd: used for register insertion
                                                   //2. bypassable: Used for bypassing registers




class reg_mark
{
    private:
        bool manalyzed;
        bool mbypassed;
        bool mbypassable;
    public:
        reg_mark()
        {
           manalyzed = false;
           mbypassed = false;
           mbypassable = false;
        }

        void set_analyzed(bool val)
        {
            manalyzed = val;
        }

        void set_bypassed(bool val)
        {
            mbypassed = val;
        }

        void set_bypassability(bool val)
        {
            mbypassed = val;
        }

        bool get_analyzed() const
        {
            return manalyzed;
        }
        bool get_bypassed() const
        {
            return mbypassed;
        }
        bool get_bypassability() const
        {
            return mbypassable;
        }
};



class graph_node
{
    private:
        std::string mreg;
        std::string mreg_input_w;
        std::string mreg_output_w;
        bool mvisited;
        //bool mdirty;
        //bool maffected;

    public:
        graph_node()
        {
            mreg = "";
            mreg_input_w = "";
            mreg_output_w = "";
            mvisited = false;
        }
        graph_node(std::string reg, std::string input_w, std::string output_w)
        {
            mreg = reg;
            mreg_input_w = input_w;
            mreg_output_w = output_w;
            mvisited = false;
        }
        std::string get_reg_value() const
        {
            return mreg; 
        }
        std::string get_reg_output_w() const
        {
            return mreg_output_w;
        }
        std::string get_reg_input_w() const
        {
            return mreg_input_w;
        }

        bool get_visited() const
        {
            return mvisited;
        }
        
        void set_visited(bool value)
        {
            mvisited = value;
        }
        //void set_dirty(bool value)
        //{
        //    mdirty = value;
        //}
        //void set_affected(bool value)
        //{
        //    maffected = value;
        //}
        //bool get_dirty()
        //{
        //    return mdirty;
        //}
        //bool get_affected()
        //{
        //    return maffected;
        //}
        bool operator == (const graph_node& ein)
        {
            if(
                 (ein.get_reg_value() == this->mreg) &&
                 (ein.get_reg_input_w() == this->mreg_input_w) &&
                 (ein.get_reg_output_w() == this->mreg_output_w)
              )
                return true;
            else
                return false;
        }
        
};




class graph_node_prop
{
    private:
        graph_node* mnode;
        double mcost_of_bypassing;
    public:
        graph_node_prop()
        {
            mnode = new graph_node();
            mcost_of_bypassing = -1000;
        }

        graph_node_prop(graph_node* node)
        {
            mnode = node;
            mcost_of_bypassing = -1000;
        }
        
        graph_node* get_node() const
        {
            return mnode;
        }

        double get_cost() const
        {
            return mcost_of_bypassing;
        }
        void set_cost(double value)
        {
            mcost_of_bypassing = value;
        }
        bool operator == (const graph_node_prop& ein)
        {
            if ((*(ein.get_node())) == (*(this->mnode)))
                return true;
            else
                return false;
        }
};



class abs_tree_node
{
    private:
        std::string mreg;
        std::string mreg_output_w;
    public:
        abs_tree_node()
        {
            mreg = "";
            mreg_output_w = "";
        }
        abs_tree_node(std::string reg,std::string output_w)
        {
            mreg = reg;
            mreg_output_w = output_w;
        }
        std::string get_reg_value() const
        {
            return mreg;
        }

        std::string get_reg_output_w() const
        {
            return mreg_output_w;
        }

        bool operator == (const abs_tree_node& ein)
        {
            if(
                 (ein.get_reg_value() == this->mreg) &&
                 (ein.get_reg_output_w() == this->mreg_output_w)
              )
                return true;
            else
                return false;
        }

};

class tree_node
{
    private:
        std::string mreg;
        vector<tree_node*>* mchildren;
        tree_node* mparent;
        std::string mreg_output_w;
        std::string mreg_input_w;
        int mlevel;
    public:
        tree_node()
        {
        }
        tree_node(std::string reg, vector<tree_node*>* children, tree_node* parent,std::string reg_output_w,std::string reg_input_w)
        {
            mreg = reg;
            mchildren = children;
            mparent = parent;
            mreg_output_w = reg_output_w;
            mreg_input_w = reg_input_w;
            mlevel = -1000;
        }

        std::string get_reg_value() const
        {
            return mreg;
        }

        vector<tree_node*>* get_children() const
        {
            return mchildren;
        }
        tree_node* get_parent() const
        {
            return mparent;
        }

        int get_no_of_children()
        {
            return mchildren->size();
        }

       std::string get_reg_output_w() const
       {
           return mreg_output_w;
       }

       std::string get_reg_input_w() const
       {
           return mreg_input_w;
       }
       void set_children(vector<tree_node*>* children)
       {
           mchildren = children;
       }

       void set_parent(tree_node* node)
       {
           mparent = node;
       }
        
       bool operator == (const tree_node& ein)
       {
           if(
                (ein.get_reg_value() == this->mreg) &&
                (ein.get_reg_output_w() == this->mreg_output_w)
             )
               return true;
           else
               return false;
       }
       void set_level(int level)
       {
          mlevel = level;
       }

       int get_level() const
       {
           return mlevel;
       }
};


class reg_graph_edge
{
    private:
      graph_node* mreg_start;
      graph_node* mreg_end;
      double mweight;
      bool mvisited;
    public: 
      reg_graph_edge()
      {
          mreg_start = new graph_node();
          mreg_end = new graph_node();
          mweight = 0;
          mvisited = false;
      }
      
      reg_graph_edge(graph_node* start,graph_node* end)
      {
          mreg_start = start;
          mreg_end  = end;
          mweight = 0;
          mvisited = false;
      }

      void set_weight (double value)
      {
          mweight = value;
      }
      
      void set_visited (bool value)
      {
          mvisited = value;
      }

      graph_node* get_start_graph_node() const
      {
          return mreg_start;
      }

      graph_node* get_end_graph_node() const
      {
          return mreg_end;
      }

      double get_weight() const 
      {
          return mweight;
      }
      double get_visited() const 
      {
          return mvisited;
      }

      bool operator == (const reg_graph_edge& ein)
      {
          if(
                  ((*(ein.get_start_graph_node())) == (*(this->mreg_start))) &&
                  ((*(ein.get_end_graph_node())) == (*(this -> mreg_end)))
            )
              return true;
          else
              return false;

      }
};



class graph_edge
{
    private:
        std::string m_start_wire;
        std::string m_end_wire;
        bool m_visited;
    public:
        graph_edge (std::string start_wire , std::string end_wire)
        {
            m_start_wire = start_wire;
            m_end_wire = end_wire;
            m_visited = false;
        }
        std::string get_start_wire() const
        {
            return m_start_wire;
        }
        std::string get_end_wire() const
        {
            return m_end_wire;
        }
        bool get_visited() const
        {
            return m_visited;
        }

        void set_visited(bool visit)
        {
            m_visited = visit;
        }
        bool operator == ( const graph_edge& ein)
        {
            if((strcmp(ein.get_start_wire().c_str(),(this->m_start_wire).c_str()) == 0) && (strcmp(ein.get_end_wire().c_str(),(this->m_end_wire).c_str()) == 0) )
                return true;
            else
                return false;
        }
        
        graph_edge& operator =(graph_edge& e1)
        {
            this->m_start_wire = e1.get_start_wire();
            this->m_end_wire = e1.get_end_wire();
            return *this;
        }


};




class graph_edge_prop 
{
    private:
        std::string m_in_port;
        std::string m_instance_name;
        std::string m_module_name;
        std::string m_out_port;
        bool m_visited;
    public:
        graph_edge_prop(std::string in_port,std::string instance_name,std::string module_name,std::string out_port/*, bool visited*/)
        {
            m_in_port=in_port;
            m_instance_name=instance_name;
            m_module_name=module_name;
            m_out_port=out_port;
            //m_visited = visited;
        };

        std::string get_in_port() const
        {
            return m_in_port;
        }
        std::string get_instance_name() const
        {
            //cout<< "Entered get instance name: " << m_instance_name << endl; 
            return m_instance_name;
        }
        std::string get_module_name() const
        {
            return m_module_name;
        }
        std::string get_out_port() const
        {
            return m_out_port;
        }

        bool get_visited() const
        {
            return m_visited;
        }                    
        void set_visited(bool visited)
        {                    
            m_visited=visited;
        }
};




class compare_graph_edge
{
    public:
        bool operator()(graph_edge e1 , graph_edge e2)
        {
            return (e1.get_start_wire() < e2.get_start_wire()) || (e1.get_end_wire() < e2.get_end_wire());
        }
};


class compare_graph_node
{
    public:
        bool operator()(graph_node* n1,graph_node* n2)
        {
            return ((n1->get_reg_value() < n2->get_reg_value()) || (n1->get_reg_input_w() < n2->get_reg_input_w()) || (n1->get_reg_output_w() < n2->get_reg_output_w()));
        }
};

class compare_reg
{
    public:
        bool operator()(std::string reg1 , std::string reg2)
        {
            return (reg1>reg2);
        }
};

class compare_abs_tree_node
{
    public:
        bool operator()(abs_tree_node* n1, abs_tree_node* n2)
        {
            return ((n1->get_reg_value() < n2->get_reg_value()) || (n1->get_reg_output_w() < n2->get_reg_output_w()));
        }
};

typedef map<graph_edge,graph_edge_prop,compare_graph_edge> net_list_graph;

class compare_reg_level_graph
{
    public:
        bool operator()(net_list_graph::iterator p1,net_list_graph::iterator p2)
        {
            return (((p1->second).get_instance_name()) > ((p2->second).get_instance_name()));
        }
};


class child_and_prop
{
    private:
        std::string mnode;
        bool mactive;
    public:
        child_and_prop(std::string node)
        {
            mnode = node;
            mactive = true;
        }
        void set_activity(bool activity)
        {
            mactive = activity;
        }
        bool get_activity() const
        {
            return mactive;
        }
        std::string get_node() const
        {
            return mnode;
        }
};



typedef vector<reg_graph_edge*> reg_graph;
typedef vector<vector<tree_node*> >counted_paths;
typedef map<std::string,reg_prop,compare_reg> register_prop_graph;
typedef map<std::string,vector<std::string> > in_out_port_info;
typedef map<std::string,tree_node* > graph_node_to_tree_map;
typedef map<std::string,int> register_max_level;
typedef map<std::string,reg_mark> reg_mark_graph;
typedef map<std::string ,double*> reg_level_graph;
typedef map<std::string,bool>visited_graph;
typedef map<std::string,vector<graph_node*> >reachable_graph;
typedef map<std::string,vector<vector<reg_graph_edge*> > >cutset_graph;
typedef map<abs_tree_node*,vector<tree_node*>,compare_abs_tree_node > abs_tree_node_2_tree_node;
typedef map<int,vector<vector<graph_node*> > > combinations;
typedef map<std::string,vector<std::string> > startnode_iter;
typedef map<std::string, startnode_iter > nl_graph_iter;
typedef map<std::string, map<std::string,reg_graph_edge*> > reinstated_reg_graph;



///function declarations
bool if_output(std::string instance_name);
void print_children_of_nl_graph_node(queue<child_and_prop>Q);
queue<child_and_prop>get_children_of_nl_graph_node(std::string current_nd);
void print_nl_graph();
void print_graph_nodes_in_file(ofstream& file);
vector<graph_node* > get_children_of_skl_graph_node(graph_node* node,bool mode, int grp);
void put_dummy_output_node();
void print_skl_graph_nodes(int grp);
void print_skl_graph_to_file(ofstream& file);
void print_skl_graph_node(graph_node* node);
bool if_one_of_the_skl_graph_nodes(graph_node node,int grp); //grp = 0 -> skl_graph_nodes // grp = 1 -> modified_skl_graph_nodes
graph_node* get_skl_graph_node(std::string reg,std::string start_wire,std::string end_wire,int grp);
reg_graph_edge* get_edge_from_skl_graph(graph_node* start_node,graph_node* end_node,bool supress_warning,int grp);
void create_skl_graph_node_vector();
bool null_skl_graph_node(graph_node* node);
bool if_edge_in_skl_graph(reg_graph_edge* edge,int grp);
void Traverse_to_create_skl_graph(std::string current_nd,graph_node* node);
void print_skl_graph(int grp);
std::string trim(string& str);
void rebuild_skl_graph(ifstream& fin1,ifstream& fin2);
vector<graph_node* > form_reachability_graph(graph_node* node,int grp);
reachable_graph::iterator get_reachability_vector(graph_node* node);
vector<graph_node* >merge_skl_graph_node_vectors(vector<graph_node* >vec1,vector<graph_node* >vec2);
void print_reachability_hash();
void mark_all_skl_graph_nodes(bool value,int grp);
//void find_loops_in_graph(graph_node* node, vector<graph_node* > vec);
void find_loops_in_graph(graph_node* node,stack<graph_node*>* univ_stack);
bool if_edge_in_loop_data_base(reg_graph_edge* edge);
void print_edges_in_loops();
void mark_all_skl_graph_edges(bool value,int grp);
cutset_graph::iterator get_value_for_a_key_map8(graph_node* node);
vector<vector<reg_graph_edge*> >merge_cutsets( vector<vector<reg_graph_edge*> >set_of_cutset,graph_node* of_child,vector<vector<reg_graph_edge*> >merged_set_of_cutset,vector<graph_node* >of_children,type_of_cutsets type,int grp);
vector<graph_node* >merge(vector<graph_node* >vec1,vector<graph_node* >vec2);
vector<graph_node* >get_group2(vector<reg_graph_edge*>c1);
vector<graph_node* >subtract_group(vector<graph_node* >bigger_group,vector<graph_node* >smaller_group);
bool intersection (vector<graph_node* >group1,vector<graph_node* >group2);
vector<reg_graph_edge* >merge_2_cutsets(vector<reg_graph_edge* >c1,vector<reg_graph_edge*>c2);
bool bypassable_cutset(vector<reg_graph_edge* >c1,vector<graph_node* >group1,int grp);
void print_set_of_cutsets(vector<vector<reg_graph_edge*> >set_of_cutset);
vector<vector<reg_graph_edge*> >exhaustive_set_of_cutsets();
vector<vector<reg_graph_edge*> >set_of_cut_set(graph_node* node,reg_graph_edge* edge,type_of_cutsets type, int grp, bool print_percent);
vector<reg_graph_edge* > get_cut_set_for_known_group(vector<graph_node*>group1,vector<graph_node*>group2);
vector<graph_node*> get_intersecting_children (vector<graph_node* >group1,vector<graph_node* >group2);
bool validate_cutset(vector<graph_node*>group1,vector<graph_node*>group2,reg_graph in_graph);
vector<vector<graph_node*> > get_all_combinations(int no_of_nodes,bool get_from_table);
bool duplicate_cutset(vector<vector<reg_graph_edge*> >vec_of_vec,vector<reg_graph_edge*>vec);
bool duplicate(vector<vector<graph_node*> >vec_of_vec,vector<graph_node*>vec);
bool match_2_vec_of_edge(vector<reg_graph_edge*>vec1, vector<reg_graph_edge*>vec2);
bool match_2_vec(vector<graph_node*>vec1, vector<graph_node*>vec2);
reg_graph_edge* get_edge_from_gen_graph(graph_node* start_node,graph_node* end_node,reg_graph vec);
reg_graph reverse_graph();
void print_set_of_cutset_for_node(graph_node* node);
bool check_if_node_in_vector(graph_node* node, vector<graph_node*>vec);
void find_nodes_in_loop();
void distribute_loop_nodes_to_grp(graph_node* node);
void print_nodes_in_loop();
void print_skl_graph_node_vector(vector<graph_node*> vec);
vector<graph_node* > get_children_of_loop_graph_node(graph_node* node);
void empty_a_vector(vector<graph_node*>* vec);
void mark_all_skl_graph_edges(bool value);
int get_group(graph_node* node);
void mark_all_edges_in_loop(bool value);
vector<graph_node* > get_reachability_vector_of_grp(vector<graph_node* >vec);
int equivalance_of_set_of_cutsets(vector<vector<reg_graph_edge*> >set_of_cutset1,vector<vector<reg_graph_edge*> >set_of_cutset2);
void create_modified_skl_graph_node_vector();
void create_modified_skl_graph();
int if_node_a_coglomerate_node(graph_node* node);
vector<vector<reg_graph_edge*> > modify_set_of_cut_set(vector<vector<reg_graph_edge*> >* set_of_cutset);
vector<reg_graph_edge*> get_edges_btwn_2_grps(vector<graph_node*> start_grp, vector<graph_node*> end_grp);
void print_set_of_cutsets_to_file(ofstream& file,vector<vector<reg_graph_edge*> >set_of_cutset);
vector <graph_node*> get_downstream_of_cut(vector<reg_graph_edge*>ip_cut);
vector<graph_node*> flatten_grp(vector<graph_node*> ip_vec);
void print_2D_vec_to_file(ofstream& file,vector<vector<graph_node*> >vec);
void mark_all_edges_in_nl_graph();
int getMilliCount();
int getMilliSpan(int nTimeStart);
bool found_in_stack(graph_node* element,stack<graph_node*>* univ_stack);
std::string convert_graph_node_2_str(graph_node* node);
std::string convert_graph_node_el_2_str(std::string ip_wire, std::string reg, std::string op_wire);
vector<graph_node* > get_children_of_loop_graph_node_v2(graph_node* node);
bool if_edge_in_loop_data_base_v2(reg_graph_edge* edge);
reg_graph_edge* get_edge_from_skl_graph_v2(graph_node* start_node, graph_node* end_node, int grp);
void print_loops_to_file(ofstream& fout);
bool if_edge_in_skl_graph_v2(reg_graph_edge* edge, int grp);
graph_node* get_skl_graph_node_v2(std::string reg,std::string start_wire,std::string end_wire, int grp);
void Traverse_to_create_reg_level(graph_node* node, double level);
void print_reg_level_hash();
void print_reg_level_hash_to_file(ofstream& file);
void write_bypaassable_reg_to_file(ofstream& file, bool even);
void Traverse_to_create_reg_level_linear(graph_node* node, double level);
bool if_a_flop(std::string instance_name);
void print_dummy_graph_nodes_in_file(ofstream& file);
void print_register_nodes_in_file(ofstream& file);
void rebuild_dummy_node_vector(ifstream& fin);
bool if_dummy(graph_node* node);
bool if_dummy_str(std::string node_str);
vector<graph_node*> cut_2_reg(vector<reg_graph_edge*> curr_cut);
vector<int> get_intersection_index(int pipeline_idx,vector<vector<graph_node*> >pipelines);
vector<int> choose_pipelines(vector<vector<graph_node*> >pipelines, map<int,vector<int> > forbidden_pipelines, vector<int> reward_arr, double mux_ovehead,int no_of_iterations);
void rebuild_nl_graph(ifstream& fin);
vector<std::string> get_children_nl_graph(std::string parent);
vector<std::string> get_children_nl_graph_nv(std::string parent);
void Traverse_to_create_reg_level_linear_nl_graph(std::string node,int level);
double get_crit_path(reinstated_reg_graph* );
void new_graph(vector<graph_node*>,reinstated_reg_graph* ,double );
std::vector<graph_node*> get_children_of_given_skl_graph_node(reinstated_reg_graph*, graph_node*);
std::vector<graph_node*> get_parents_of_given_skl_graph_node(reinstated_reg_graph*, graph_node*);
void modify_graph(reinstated_reg_graph* curr_graph, graph_node* parent, graph_node* node, graph_node* child,double mux_overhead);
void print_given_skl_graph(reinstated_reg_graph*);
vector<int> subtract_group_int(vector<int>,vector<int>);
vector<vector<graph_node*> > filter_dummy_pipelines(vector<vector<graph_node*> > );
void print_bypassable_reg_to_file(ofstream& ,vector<vector<graph_node*> > ,vector<int>);
void rebuild_register_graph_nodes(ifstream&);
void print_non_bypassable_reg_to_file(ofstream& ,vector<vector<graph_node*> > ,vector<int>);

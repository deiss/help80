/*
    
************************************************************************************************
 
LICENSE

    Ce programme est distribué sous la license GPL.
 
************************************************************************************************

FILE

    The classes defined in this file can be used for parameters parsing and display, while
    making sure the help menu looks nice on terminals.

    This file defines classes Parameters, ParamHolder and Param. Param is a template function
    that inherits from ParamHolder. Parameters owns a std::vector of ParamHolder, and has access
    to the derived object Param through dynamic_cast.
    
    To store parameters of multiple types, the std::string version of types is available with
    typeid(T).name(). This type is stored in ParamHolder. When arguments are parsed, each type is
    tried, and the right function is called for the right type (among stoi, stod, stold, and
    nothing for std::string parameters).
    
    
                       ---------------                 ----------------
                       | ParamHolder |---------------<>|  Parameters  |
                       ---------------                 ----------------
                              ^
                              |
                              |    ------------
                      -------------| Template |
                      |     Param  --+---------
                      ----------------
 
                         
    ParamHolder and Param are private nested classes inside class Parameters.
    
 
    How to build the menu:
        
        First you can specify a description of your program, and how to use it with functions
        set_program_description and set_usage.
 
        To build the menu, use the functions:
            - define_param :        to define a parameter with no value
            - define_num_str_param: to define a numeric or std::string parameter
            - define_choice_param:  to define a multiple choice parameter
 
        You can add structure by adding subsections with function insert_subsection.
        When the menu is ready, you can print it with print_help().
        
 
    How to use the parameters:
 
        First, call parse_params to analyse the command line arguments. Then,
        use the functions:
            - is_def  :  to know if a simple parameter is specified
            - num_val : to get a numeric value
            - str_val : to get a std::string value
            - cho_val : to get a multiple choice value
 
************************************************************************************************

*/

#ifndef Parameters_hpp
#define Parameters_hpp

#include <iostream>
#include <map>
#include <set>
#include <vector>

class Parameters {

    class ParamHolder;

    public:
    
        typedef std::vector<std::pair<std::string, std::string>> vec_choices;
    
        enum LANG {lang_fr, lang_us};
    
        struct config {
            const int   terminal_width;                                                   // the width of the terminal
            const int   param_to_desc_len;                                                // nb of spaces between longest param list and descripton
            const int   desc_indent_len;                                                  // nb of characters form the left to print description
            const int   params_indent_len;                                                // nb of characters from the left to print param+values
            const int   choice_indent_len;                                                // indentation for choices descriptions
            LANG        lang;                                                             // language to print the menu in
        };
    
        Parameters(const int, char const* const* const, config);
        ~Parameters();
    
        void set_program_description(const std::string&);                                 // sets program description
        void set_usage(const std::string&);                                               // sets usage
    
        /* help menu */
        template<typename T>
        void define_num_str_param(const std::string&, const std::vector<std::string>&,
            const std::vector<T>&, const std::string&, const bool=false);                 // add a parameter with values
        void define_choice_param(const std::string&, const std::string&,
            const std::string&, vec_choices, const std::string&, const bool=false);       // add a parameter with choices
        void define_param(const std::string&, const std::string&);                        // add a parameter with no values
        void insert_subsection(const std::string&);                                       // prints subsection when printing help menu
        void print_help(const bool=true, const bool=true) const;                          // print help menu
    
        /* use of parameters */
        template<typename T>
        const T           num_val(const std::string&, const int=1) const;                 // return n-th value for parameter. nb starts at 1
        const std::string str_val(const std::string&, const int=1) const;                 // return n-th value for parameter. nb starts at 1
        const std::string cho_val(const std::string&)              const;                 // returns choice value
        const bool        is_def(const std::string&)               const;                 // tells if parameters is defined
        void              parse_params();                                                 // reads cmd line and store args
    
    
    private:
    
        typedef std::map<std::string, ParamHolder* const> map_params;
        typedef std::map<std::string, vec_choices>        map_choices;
        typedef std::map<size_t, std::string>             map_order;
    
        void                     print_description() const;                               // print program description
        void                     print_usage()       const;                               // print usage
        void                     print_parameters()  const;                               // print list of parameters
    
        Parameters(const Parameters&);
        Parameters& operator=(const Parameters&);
    
        /* cmd line */
        const int                argc;                                                    // command line args number
        char const* const* const argv;                                                    // command line args values
    
        /* display parameters */
        const int                terminal_width;                                          // the width of the terminal
        const int                param_to_desc_len;                                       // nb of spaces between longest param list and descripton
        const int                desc_indent_len;                                         // nb of characters form the left to print desccription
        const int                params_indent_len;                                       // nb of characters from the left to print param+values
        const int                choice_indent_len;                                       // indentation for choices descriptions
        std::string              desc_indent;                                             // spaces for indentation of big description
        std::string              choice_indent;                                           // indentation for choices descriptions
        std::string              params_indent;                                           // string of 'params_indent_len' spaces
    
        /* internal vars */
        const LANG               lang;                                                    // language to print the menu in
        std::string              description;                                             // description of the program
        bool                     description_is_set;                                      // true if set_description() is called
        std::string              usage;                                                   // usage of the program
        bool                     usage_is_set;                                            // true if set_usage() is called
        std::vector<std::string> subsections;                                             // sub sections titles in the help menu
        std::vector<std::size_t> subs_indexes;                                            // indexes of the subsections (where to print them)
        map_params               params;                                                  // data structure that stores all the parameters
        map_order                order;                                                   // data structure to store order of parameters
        map_choices              choices;                                                 // stores choices associated to choice-parameters
        std::set<std::string>    choices_params;                                          // stores all the params that are multiple choice
    
    
    private:
    
        class ParamHolder {

            public:
            
                ParamHolder(const std::string& p_name, const std::string& p_description, const std::string& p_type_name, const std::vector<std::string>& p_values_names={}, const bool p_display_default_value=false):
                    name(p_name),
                    description(p_description + " "),
                    nb_values(p_values_names.size()),
                    values_names(p_values_names),
                    type_name(p_type_name),
                    display_default_value(p_display_default_value),
                    is_defined(false) {}
                virtual ~ParamHolder() {}
            
                const std::string              name;                                      // param name with added suffix '--'
                const std::string              description;                               // long description paragraph, can't start or end with ' '
                const std::size_t              nb_values;                                 // nb of values expected for the parameters
                const std::vector<std::string> values_names;                              // name of these values, without '<' and '>'
                const std::string              type_name;                                 // to distinguish the type of the values
                const bool                     display_default_value;                     // if default value has to be displayed in help menu
                bool                           is_defined;                                // if the arg was specified by the user
            
            
            private:
            
                ParamHolder(const ParamHolder&);
                ParamHolder& operator=(const ParamHolder&);

        };

        template<typename T>
        class Param: public ParamHolder {

            public:
            
                Param(const std::string& p_name, const std::string& p_description, const std::vector<std::string>& p_values_names, const std::vector<T>& p_default_values, const bool p_display_default_value):
                    ParamHolder(p_name, p_description, typeid(T).name(), p_values_names, p_display_default_value),
                    values(p_default_values),
                    def_values(p_default_values) {}
                Param(const std::string& p_name, const std::string& p_description):
                    ParamHolder(p_name, p_description, typeid(T).name()) {}
                virtual ~Param() {}
                
                std::vector<T>       values;                                              // parameter values
                const std::vector<T> def_values;                                          // parameter default values

        };

};




/*** template functions definition ***/

template<typename T>
const T Parameters::num_val(const std::string& param_name, const int value_number) const {
    if(params.count("--" + param_name)) {
        Parameters::ParamHolder* const p = params.at("--" + param_name);
        if(value_number>p->nb_values) {
            throw std::string("parameter \"--" + param_name + "\" only has " + std::to_string(p->nb_values) + " values");
        }
        else {
            if(p->type_name==typeid(int).name()) {
                /* reinterpret with the good type */
                Param<int>* const p_reint = dynamic_cast<Param<int>* const>(p);
                /* return value */
                return p_reint->values[value_number-1];
            }
            else if(p->type_name==typeid(double).name()) {
                /* reinterpret with the good type */
                Param<double>* const p_reint = dynamic_cast<Param<double>* const>(p);
                /* return value */
                return p_reint->values[value_number-1];
            }
            else if(p->type_name==typeid(long double).name()) {
                /* reinterpret with the good type */
                Param<long double>* const p_reint = dynamic_cast<Param<long double>* const>(p);
                /* return value */
                return p_reint->values[value_number-1];
            }
            else {
                throw std::string("type not supported yet");
            }
        }
    }
    else {
        throw std::string("error: unknown parameter \"--" + param_name + "\"");
    }
}

template<typename T>
void Parameters::define_num_str_param(const std::string& param_name, const std::vector<std::string>& values_names, const std::vector<T>& default_param_values, const std::string& param_desc, const bool display_default_value) {
    /* check if already exist */
    if(params.count("--" + param_name)) {
        if(lang==lang_fr) throw std::string("erreur : un paramètre de même nom existe déjà");
        else              throw std::string("error: a parameter with the same name already exists");
    }
    /* get type name */
    const std::string type_name = typeid(T).name();
    /* create param */
    Param<T>* const p = new Param<T>("--" + param_name, param_desc, values_names, default_param_values, display_default_value);
    /* store param */
    order.insert(std::make_pair(params.size(), "--" + param_name));
    params.insert(std::make_pair("--" + param_name, p));
}

#endif
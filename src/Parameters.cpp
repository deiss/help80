#include "Parameters.hpp"

Parameters::Parameters(const int p_argc, char const* const* const p_argv, config p_c):
    argc(p_argc),
    argv(p_argv),

    terminal_width(p_c.terminal_width),
    param_to_desc_len(p_c.param_to_desc_len),
    desc_indent_len(p_c.desc_indent_len),
    params_indent_len(p_c.params_indent_len),
    choice_indent_len(p_c.choice_indent_len),
    desc_indent(""),
    choice_indent(""),
    params_indent(""),

    lang(p_c.lang),
    description_is_set(false) {
    for(int i=0 ; i<params_indent_len ; i++) params_indent += " ";
    for(int i=0 ; i<choice_indent_len ; i++) choice_indent += " ";
    for(int i=0 ; i<desc_indent_len ; i++)   desc_indent += " ";
}

Parameters::~Parameters() {
    for(std::pair<std::string, ParamHolder*> p: params) delete p.second;
}

void Parameters::set_program_description(const std::string &p_description) {
    description        = p_description + " ";
    description_is_set = true;
}

void Parameters::set_usage(const std::string &p_usage) {
    usage        = p_usage;
    usage_is_set = true;
}

/*** build help menu ***/

void Parameters::insert_subsection(const std::string& subsection_title) {
    subsections.push_back(subsection_title);
    subs_indexes.push_back(params.size());
}

void Parameters::define_param(const std::string& param_name, const std::string& param_desc) {
    /* check if already exist */
    if(params.count("--" + param_name)) {
        if(lang==lang_fr) throw std::string("erreur : un paramètre de même nom existe déjà");
        else              throw std::string("error: a parameter with the same name already exists");
    }
    /* get type name */
    const std::string type_name = typeid(bool).name();
    /* create param */
    Param<bool>* const p = new Param<bool>("--" + param_name, param_desc);
    /* store param */
    order.insert(std::make_pair(params.size(), "--" + param_name));
    params.insert(std::make_pair("--" + param_name, p));
}

void Parameters::define_choice_param(const std::string& param_name, const std::string& value_name, const std::string& default_choice, vec_choices p_choices, const std::string& param_desc, const bool display_default_value) {
    /* check if already exist */
    if(params.count("--" + param_name)) {
        if(lang==lang_fr) throw std::string("erreur : un paramètre de même nom existe déjà");
        else              throw std::string("error: a parameter with the same name already exists");
    }
    /* get type name */
    const std::string type_name = typeid(std::string).name();
    /* append space in descriptions */
    for(std::pair<std::string, std::string>& p:p_choices) p.second += " ";
    /* create param */
    Param<std::string>* const p = new Param<std::string>("--" + param_name, param_desc, {value_name}, {default_choice}, display_default_value);
    /* store param */
    order.insert(std::make_pair(params.size(), "--" + param_name));
    params.insert(std::make_pair("--" + param_name, p));
    choices.insert(std::make_pair("--" + param_name, p_choices));
    choices_params.insert("--" + param_name);
}

/*** display help menu ***/

void Parameters::print_help(const bool p_print_usage, const bool p_print_description) const {
    if(description_is_set && p_print_description) { print_description(); }
    if(usage_is_set       && p_print_usage)       { std::cout << std::endl; print_usage(); }
    print_parameters();
}

void Parameters::print_description() const {
    std::cout << std::endl;
    std::string line       = "";
    std::string word       = "";
    bool        first_word = true;
    for(std::size_t i=0 ; i<description.length() ; i++) {
        char c = description.at(i);
        if(c!=' ') {
            word += c;
        }
        else {
            if(line.length()+word.length()+1<=terminal_width) {
                if(first_word) { line = word; first_word = false; }
                else           { line += " " + word; }
                word = "";
            }
            else {
                /* line would be too long, print it */
                std::cout << line << std::endl;
                line = word;
                word = "";
            }
        }
    }
    /* print last line */
    std::cout << line << std::endl;
}

void Parameters::print_usage() const {
    if(lang==lang_fr) std::cout << "UTILISATION :" << std::endl << std::endl;
    else              std::cout << "USAGE:" << std::endl << std::endl;
    std::cout << params_indent << usage << std::endl << std::endl;
}

void Parameters::print_parameters() const {
    for(std::size_t i=0 ; i<params.size() ; i++) {
        /* print subsection if needed */
        for(std::size_t j=0 ; j<subs_indexes.size() ; j++) {
            if(subs_indexes[j]==i) {
                if(lang==lang_fr) std::cout << std::endl << subsections[j] << " :" << std::endl << std::endl;
                else              std::cout << std::endl << subsections[j] << ":" << std::endl << std::endl;
            }
        }
        
        /* retrieve param */
        ParamHolder* p = params.at(order.at(i));
        /* build use string */
        std::string use = params_indent + p->name;
        for(std::string value_name: p->values_names) use += " <" + value_name + ">";
        
        /* print param and values to take */
        bool desc_on_new_line = false;
        if(use.length()+param_to_desc_len>desc_indent_len) {
            /* print param list now if too long */
            std::cout << use << std::endl;
            desc_on_new_line = true;
        }
        else {
            /* print list and spaces if it fits */
            /* minus one because one space will be added by first description line */
            std::string spaces = "";
            for(int j=0 ; j<desc_indent_len-use.length() ; j++) spaces += " ";
            std::cout << use << spaces;
        }
        
        /* print description */
        std::string line       = "";
        std::string word       = "";
        bool        first_l    = true;
        bool        first_word = true;
        for(std::size_t j=0 ; j<p->description.length() ; j++) {
            char c = p->description.at(j);
            if(c!=' ' && desc_indent_len+word.length()<terminal_width) {
                word += c;
            }
            else {
                if(desc_indent_len+line.length()+word.length()+1<=terminal_width) {
                    if(first_word) { line = word; first_word = false; }
                    else           { line += " " + word; }
                    word = "";
                }
                else {
                    /* line would be too long, print it */
                    if(!first_l || desc_on_new_line) std::cout << desc_indent;
                    if(first_l)                      first_l = false;
                    if(desc_indent_len+word.length()<terminal_width) {
                        /* prints line and take a new line */
                        std::cout << line << std::endl;
                        line = word;
                        word = "";
                    }
                    else {
                        /* no need to take another line, the word will be split anyways */
                        std::size_t line_len;
                        if(line!="") { std::cout << line << " "; line_len = line.length() + 1; }
                        else         { line_len = 0; }
                        std::cout << word.substr(0, terminal_width-(desc_indent_len+line_len)) << std::endl;
                        word = word.substr(terminal_width-(desc_indent_len+line_len));
                        word.push_back(c);
                        line       = "";
                        first_word = true;
                    }
                }
            }
        }
        /* print last line */
        if(!first_l || desc_on_new_line) std::cout << desc_indent;
        std::cout << line << std::endl;
        
        /* print choices */
        if(choices_params.count(p->name)) {
            for(const std::pair<std::string, std::string>& p: choices.at(p->name)) {
                /* print choice */
                if(lang==lang_fr) std::cout << desc_indent << choice_indent << "\"" << p.first << "\" : ";
                else              std::cout << desc_indent << choice_indent << "\"" << p.first << "\": ";
                /* print choice description */
                /* print description */
                const std::size_t other_len  = (lang==lang_fr) ? 5+p.first.length() : 4+p.first.length();
                std::string       spaces     = desc_indent + choice_indent;
                std::string       line       = "";
                std::string       word       = "";
                bool              first_l    = true;
                bool              first_word = true;
                for(int j=0 ; j<other_len ; j++) spaces += " ";
                for(std::size_t j=0 ; j<p.second.length() ; j++) {
                    char c = p.second.at(j);
                    if(c!=' ' && desc_indent_len+choice_indent_len+other_len+word.length()<terminal_width) {
                        word += c;
                    }
                    else {
                        if(desc_indent_len+choice_indent_len+other_len+line.length()+word.length()+1<=terminal_width) {
                            if(first_word) { line = word; first_word = false; }
                            else           { line += " " + word; }
                            word = "";
                        }
                        else {
                            /* line would be too long, print it */
                            if(!first_l || desc_on_new_line) std::cout << spaces;
                            if(first_l)                      first_l = false;
                            if(desc_indent_len+choice_indent_len+other_len+word.length()<terminal_width) {
                                /* prints line and take a new line */
                                std::cout << line << std::endl;
                                line = word;
                                word = "";
                            }
                            else {
                                /* no need to take another line, the word will be split anyways */
                                std::size_t line_len;
                                if(line!="") { std::cout << line << " "; line_len = line.length() + 1; }
                                else         { line_len = 0; }
                                std::cout << word.substr(0, terminal_width-(desc_indent_len+choice_indent_len+other_len+line_len)) << std::endl;
                                word = word.substr(terminal_width-(desc_indent_len+choice_indent_len+other_len+line_len));
                                word.push_back(c);
                                line       = "";
                                first_word = true;
                            }
                        }
                    }
                }
                /* print last line */
                if(!first_l || desc_on_new_line) std::cout << spaces;
                std::cout << line << std::endl;
            }
        }
        
        /* print default value */
        if(p->display_default_value) {
            if(p->type_name==typeid(int).name()) {
                /* reinterpret with the good type */
                const Param<int>* const p_reint = dynamic_cast<Param<int>* const>(p);
                if(lang==lang_fr) std::cout << desc_indent << "Défaut :";
                else              std::cout << desc_indent << "Default:";
                for(std::size_t j=0 ; j<p->nb_values ; j++) { std::cout << " " << p_reint->def_values[j]; if(j<p->nb_values-1) std::cout << ","; }
                std::cout << std::endl;
            }
            else if(p->type_name==typeid(double).name()) {
                /* reinterpret with the good type */
                const Param<double>* const p_reint = dynamic_cast<Param<double>* const>(p);
                if(lang==lang_fr) std::cout << desc_indent << "Défaut :";
                else              std::cout << desc_indent << "Default:";
                for(std::size_t j=0 ; j<p->nb_values ; j++) { std::cout << " " << p_reint->def_values[j]; if(j<p->nb_values-1) std::cout << ","; }
                std::cout << std::endl;
            }
            else if(p->type_name==typeid(long double).name()) {
                /* reinterpret with the good type */
                const Param<long double>* const p_reint = dynamic_cast<Param<long double>* const>(p);
                if(lang==lang_fr) std::cout << desc_indent << "Défaut :";
                else              std::cout << desc_indent << "Default:";
                for(std::size_t j=0 ; j<p->nb_values ; j++) { std::cout << " " << p_reint->def_values[j]; if(j<p->nb_values-1) std::cout << ","; }
                std::cout << std::endl;
            }
            else if(p->type_name==typeid(std::string).name()) {
                const Param<std::string>* const p_reint = dynamic_cast<Param<std::string>* const>(p);
                if(lang==lang_fr) std::cout << desc_indent << "Défaut :";
                else              std::cout << desc_indent << "Default:";
                for(std::size_t j=0 ; j<p->nb_values ; j++) { std::cout << " \"" << p_reint->def_values[j] << "\""; if(j<p->nb_values-1) std::cout << "\","; }
                std::cout << std::endl;
            }
            else if(p->type_name==typeid(bool).name()) {
                /* parameters with no value */
            }
            else {
                /* unknown type */
            }
        }
        
        /* skip line */
        std::cout << std::endl;
    }
}

/*** use parameters ***/

void Parameters::parse_params() {
    for(int i=1 ; i<argc ; i++) {
        /* get arg name */
        const std::string line_param(argv[i]);
        if(params.count(line_param)) {
            /* retrieve param */
            ParamHolder* const p = params[line_param];
            /* read param values */
            for(int j=0 ; j<p->nb_values ; j++) {
                if(++i<=argc) {
                    std::string arg_value(argv[i]);
                    if(p->type_name==typeid(int).name()) {
                        /* reinterpret with the good type */
                        Param<int>* const p_reint = dynamic_cast<Param<int>* const>(p);
                        /* update value */
                        try { p_reint->values[j] = std::stoi(arg_value); }
                        catch(const std::exception& e) {
                            if(lang==lang_fr) std::cerr << "le paramètre \"" << line_param << "\" attend une valeur entière, et a reçu \"" << arg_value << "\"";
                            else              std::cerr << "parameter \"" << line_param << "\" expects an integer value, received \"" << arg_value << "\"";
                        }
                    }
                    else if(p->type_name==typeid(double).name()) {
                        /* reinterpret with the good type */
                        Param<double>* const p_reint = dynamic_cast<Param<double>* const>(p);
                        /* update value */
                        try { p_reint->values[j] = std::stod(arg_value); }
                        catch(const std::exception& e) {
                            if(lang==lang_fr) std::cerr << "le paramètre \"" << line_param << "\" attend une valeur entière, et a reçu \"" << arg_value << "\"";
                            else              std::cerr << "parameter \"" << line_param << "\" expects an integer value, received \"" << arg_value << "\"";
                        }
                    }
                    else if(p->type_name==typeid(long double).name()) {
                        /* reinterpret with the good type */
                        Param<long double>* const p_reint = dynamic_cast<Param<long double>* const>(p);
                        /* update value */
                        try { p_reint->values[j] = std::stold(arg_value); }
                        catch(const std::exception& e) {
                            if(lang==lang_fr) std::cerr << "le paramètre \"" << line_param << "\" attend une valeur entière, et a reçu \"" << arg_value << "\"";
                            else              std::cerr << "parameter \"" << line_param << "\" expects an integer value, received \"" << arg_value << "\"";
                        }
                    }
                    else if(p->type_name==typeid(std::string).name()) {
                        /* reinterpret with the good type */
                        Param<std::string>* const p_reint = dynamic_cast<Param<std::string>* const>(p);
                        /* update value */
                        p_reint->values[j] = arg_value;
                    }
                }
                else {
                    if(lang==lang_fr) std::cerr << "erreur : le paramètre \"" << line_param << "\" attend " << p->nb_values << " valeurs" << std::endl;
                    else              std::cerr << "error: parameter \"" << line_param << "\" expects " << p->nb_values << " values" << std::endl;
                }
            }
            /* arg is defined */
            p->is_defined = true;
        }
        else {
            if(lang==lang_fr) std::cerr << "erreur : paramètre \"--" << line_param << "\" inconnu" << std::endl;
            else              std::cerr << "unknown parameter \"" << line_param << "\"" << std::endl;
        }
    }
}

const bool Parameters::is_def(const std::string& param_name) const {
    if(params.count("--" + param_name)) {
        ParamHolder* const p = params.at("--" + param_name);
        return p->is_defined;
    }
    else {
        if(lang==lang_fr) throw std::string("erreur : paramètre \"--" + param_name + "\" inconnu");
        else              throw std::string("error: unknown parameter \"--" + param_name + "\"");
    }
}

const std::string Parameters::str_val(const std::string& param_name, const int value_number) const {
    if(params.count("--" + param_name)) {
        Parameters::ParamHolder* const p = params.at("--" + param_name);
        if(value_number>p->nb_values) {
            throw std::string("parameter \"--" + param_name + "\" only has " + std::to_string(p->nb_values) + " values");
        }
        else {
            /* reinterpret with the good type */
            Param<std::string>* const p_reint = dynamic_cast<Param<std::string>* const>(p);
            /* return value */
            return p_reint->values[value_number-1];
        }
    }
    else {
        throw std::string("error: unknown parameter \"--" + param_name + "\"");
    }
}

const std::string Parameters::cho_val(const std::string& param_name) const {
    if(params.count("--" + param_name)) {
        Parameters::ParamHolder* const p = params.at("--" + param_name);
        /* reinterpret with the good type */
        Param<std::string>* const p_reint = dynamic_cast<Param<std::string>* const>(p);
        /* return value */
        return p_reint->values[0];
    }
    else {
        throw std::string("error: unknown parameter \"--" + param_name + "\"");
    }
}
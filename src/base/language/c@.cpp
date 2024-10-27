#include <base/language/c@.hpp>
#include <boost/tokenizer.hpp>
#include <variant>

using namespace art;

namespace language_parsers {
    struct file_parser {
        enum class tokens {
            tok_fn,
            tok_gen,
            tok_async,
            tok_annotation,
            tok_const,
            tok_final,
            tok_signed,
            tok_unsigned,
            tok_static,
            tok_checked,
            tok_auto,
            tok_any,
            tok_char,
            tok_double,
            tok_float,
            tok_short,
            tok_int,
            tok_long,
            tok_byte,
            tok_string,
            tok_arr,
            tok_map,
            tok_void,
            tok_return,
            tok_yield,
            tok_consume,
            tok_if,
            tok_else,
            tok_goto,
            tok_while,
            tok_do,
            tok_for,
            tok_loop,
            tok_switch,
            tok_case,
            tok_break,
            tok_match,
            tok_continue,
            tok_default,
            tok_struct,
            tok_class,
            tok_union,
            tok_enum,
            tok_interface,
            tok_flags,
            tok_extern,
            tok_sizeof,
            tok_typedef,
            tok_using,
            tok_namespace,
            tok_typename,
            tok_decltype,
            tok_static_assert,
            tok_constexpr,
            tok_import,
            tok_export,
            tok_this,
            tok_base,
            tok_public,
            tok_protected,
            tok_private,
            tok_internal,
            tok_implicit,
            tok_explicit,
            tok_sealed,
            tok_virtual,
            tok_override,
            tok_operator,
            tok_constructor,
            tok_destructor,
            tok_try,
            tok_catch,
            tok_throw,
            tok_finally,
            tok_filter,
            tok_throw,
            tok_exception,
            tok_in,
            tok_out,
            tok_ref,
            tok_inline,
            tok_noexcept,
            tok_null,
            tok_true,
            tok_false,

            tok__assign,
            tok__plus,
            tok__minus,
            tok__multiply,
            tok__divide,
            tok__rest,
            tok__inc,
            tok__dec,
            tok__assign_plus,
            tok__assign_minus,
            tok__assign_multiply,
            tok__assign_divide,
            tok__assign_rest,
            tok__equal,
            tok__not_equal,
            tok__bigger,
            tok__lower,
            tok__bigger_or_equal,
            tok__lower_or_equal,
            tok__logical_and,
            tok__logical_or,
            tok__logical_not,
            tok__binary_and,
            tok__binary_or,
            tok__binary_xor,
            tok__binary_invert,
            tok__binary_left_shift,
            tok__binary_right_shift,
            tok__binary_assign_left_shift,
            tok__binary_assign_right_shift,
            tok__binary_assign_and,
            tok__binary_assign_or,
            tok__binary_assign_xor,
            tok__binary_assign_invert,

            tok_block_begin_operator, //(
            tok_block_end_operator,   //)
            tok_block_begin_index,    //[
            tok_block_end_index,      //]
            tok_block_begin_scope,    //{
            tok_block_end_scope,      //}
            tok_block_begin_template, //<
            tok_block_end_template,   //>

            tok_delimiter_question,
            tok_delimiter_exclamation,
            tok_delimiter_left_slash,
            tok_delimiter_apostrophe,       //'
            tok_delimiter_lying_apostrophe, //`
            tok_delimiter_quotation,        //"
            tok_delimiter_coma,             //,
            tok_delimiter_dot,              //dot
            tok_delimiter_colon,            //:
            tok_delimiter_semicolon,        //;
            tok_delimiter_hash,             //#
            tok_delimiter_dollar,           //$
            tok_delimiter_annotation,       //@

            identifier,
            value_double,
            value_hex,
            value_long,
            value_string,
            //conditional tokens:
            //value,
            //argument,
            //return_value,
            //file,
            //utf8_file,
            //utf16_file,
            //utf32_file,
            //ascii_file,


            //internal token
            vir__pre_call,   //if parser got vir__pre_call then it checks inner childs, if found compatible then it calls vir__pre_call callback and continues from child
            vir__after_call, //if parser got vir__after_call then it checks inner childs, if found compatible then its proceeds child and then calls vir__after_call callback
        };


        inline static std::unordered_map<std::string, tokens> keywords = {
            {"fn", tokens::tok_fn},
            //usage
            //fn `name`(`type` `name`, ...) > `type` {...};
            //fn `name`(`type` `name`, ...) > `type1`/ `type2`/`type3` {...};
            //fn(`type` `name`,...) {...}; //anonymous, no return type, same as below
            //fn(`type` `name`,...) > void {...}; //anonymous, no return type
            //fn(`type` `name`,...) `name`; // function reference, same as below
            //fn(`type`,...) `name`;
            //function can be overloaded by return type, selects by declaration order first function that conforms by return type
            {"gen", tokens::tok_gen},
            //same as fn but can use yield keyword(returns Generator)
            {"async", tokens::tok_async},
            //same as fn but can use yield keyword, runs as task(returns Task)

            {"annotation", tokens::tok_annotation},
            //syntax:
            //  @`name`
            //  @`name`(...args)
            //  @[`name`]
            //  @[`name`(...args)]
            //  @[`name`,`name`,...]
            //usage
            //  ** annotation is interceptor like callback that can modify annotated function, arguments, or value
            //  ** annotation data for containers can be accessed through internal:view_structure
            //  **      but for global functions, generators or async functions and their arguments or return values are not visible in runtime, because they're not stored for raw functions but methods
            //  ** annotation types: fn, gen, async, struct, class, union, enum, interface, flags, value, argument, return_value
            //  **      types fn, gen and async can be combined to one `fn/gen/async` | `gen/async` |`fn/async` | `fn/gen` | `fn/async`
            //  **      types argument, return_value and value can be combined to one like fn, gen and async
            //  **
            //  ** can be overloaded by types
            //  ** after annotation completes without exception will be called annotated function
            //  ** implicit `func` argument can be changed at runtime then will be called overriden `func`, this does not changes function overriding
            //   ** if function has multiple annotations then annotation will be called by declaration order
            //   ** annotation for functions works as interceptor, it can throw exception, modify arguments and do anything what need to do
            //
            //annotation `name` `type`(`type` `name`, ... ){`varies`}
            //
            //or without arguments
            //
            //annotation `name` `type`{`varies`}
            //
            //also allowed:
            //constexpr annotation `name` `type` ...
            //  ** constexpr annotation allows to statically create interception, modify vtable, but does noting for value, argument and return_value types
            //      in runtime there just empty annotation with name
            //
            //  ** implicit arguments: func, args, annotation_args
            //annotation log_debug fn/gen/async{
            //    console:printf(
            //        "Function [], has been called from [] with arguments: \[[]\]",
            //        some_lib:name(func)
            //        internal:stack:clean_trace(1, true, 1)[0].fun_name
            //        args
            //    );
            //}
            //
            //  ** implicit arguments: value, args, annotation_args, value_name
            //      value_name is const
            //annotation max argument/value/return_value(any max_value){
            //    if(value > annotation_args.max_value)
            //      throw out_of_range(value_name + " is bigger than " + annotation_args.max_value);
            //}
            //

            {"const", tokens::tok_const},
            //declares value or argument as constant, internally can be ignored
            {"final", tokens::tok_final},
            //declares value as once assignable, sets const flag after assignation also applicable for functions to prevent redefinition for childs
            {"signed", tokens::tok_signed},
            //used with integers
            {"unsigned", tokens::tok_unsigned},
            //used wth integers
            {"static", tokens::tok_static},
            //declares function or value in class/union/enum e.t.c.... as singleton(same as static in c++) but not in structure
            {"checked", tokens::tok_checked},
            //every operation with this integer will be checked for overflow and underflow, not checked by runtime(manual checking)

            {"auto", tokens::tok_auto},
            //same as C++, type will be automatically selected from return types
            {"any", tokens::tok_any},
            //can hold any value, int double string or even whole class
            {"char", tokens::tok_char},
            {"double", tokens::tok_double},
            {"float", tokens::tok_float},
            {"short", tokens::tok_short},
            {"int", tokens::tok_int},
            {"long", tokens::tok_long},
            {"byte", tokens::tok_byte},
            {"string", tokens::tok_string},
            {"arr", tokens::tok_arr}, //usage: arr or arr<`type`>, arr<any> is same as arr //TODO create templates
            {"map", tokens::tok_map}, //usage: map or map<`key type`> or map<`key type`,`value type`>, key type can be anything//TODO create templates
            {"void", tokens::tok_void},


            {"return", tokens::tok_return}, //return not required when return type is void
            {"yield", tokens::tok_yield},
            //yields result, can be used only in gen and async functions
            {"consume", tokens::tok_consume},
            //collects every result in gen and async functions to array

            {"if", tokens::tok_if}, //usage:
            //if(condition)...
            //if(condition) {...}
            //if(declaration; condition)//declared values accessible in whole condition tree if(...){...}else if(...){...} else{...}
            //if(declaration;declaration;declaration; condition) //there allowed multiple declarations, but at the end must be condition
            {"else", tokens::tok_else},     // can build condition tree
            {"goto", tokens::tok_goto},     //explicit jump
            {"while", tokens::tok_while},   //same as c++
            {"do", tokens::tok_do},         //same as c++
            {"for", tokens::tok_for},       //same as c++ but iterator can be c++ or c# style
            {"loop", tokens::tok_loop},     //same as while(true)
            {"switch", tokens::tok_switch}, //same as c++ but allows strings
            {"case", tokens::tok_case},
            {"break", tokens::tok_break},
            {"match", tokens::tok_match}, //usage:
            //match(name) {
            //  size() < 4: throw InvalidArgument("Name too short");
            //  size() > 20: throw InvalidArgument("Name too long");
            //  bad_names_array.contains(name): throw InvalidArgument("Illegal name");
            //  "Bob":  throw InvalidArgument("Nice name, but nope");
            //  some_custom_func(creator, name): {
            //      ...
            //  }
            //  _ : createUser()
            //}
            // or
            // allowed_to_drink = match(years) { < 18: false; > 130: false; _: true};
            {"continue", tokens::tok_continue},
            {"default", tokens::tok_default},


            {"struct", tokens::tok_struct},
            //struct is simple type with automaticaly created available operators, every type is public, can be annotated with annotations that adds custom functions, but declaring is not allowed
            //usage:
            // struct user; // no op construction, can be used to calm down linters, does noting
            // struct user(int id, string name);//actual declaration of user struct
            // struct user_ballance(int, string, decimal);//accessed as value[0]
            // struct user_balance(decimal balance) : user;//copys user declaration and add new field
            // struct user_twin : user;//creates another struct that copys user declaration
            // struct user_balance {// C style
            //   ind id;
            //   string name;
            //   decimal balance;
            // };
            // struct user_balance : user {//copy declaration also allowed here
            //   decimal balance;
            // };
            {"class", tokens::tok_class},
            //class is same as C++ classes
            {"union", tokens::tok_union},
            //union is same as C++ unions
            {"enum", tokens::tok_enum},
            //enum is same as C++ `enum class` but allows declaring functions and to_string and from_string methods automaticaly created(case sensitive)
            //usage:
            //enum type(_struct, _class, _enum, _interface);
            //enum type(_struct, _class, _enum, _interface) : byte; // can follow basic type, but then to_string, from_string functions not declared and type becomes just alias to basic type and annotating value not allowed
            //enum type( _struct, _class, @hide _enum, @hide _interface);//enum values can be annotated
            //enum type {
            //  _struct,
            //  _class,
            //  _enum,
            //  @hide _interface,//enum values can be annotated
            //  ;
            //
            //  fn construct(arr<byte> bytecode) > any {
            //      ...
            //  }
            //
            //  fn get_type(any) > type {
            //      ...
            //  }
            //};
            {"interface", tokens::tok_interface},
            //Allows to not fully implementing functions
            {"flags", tokens::tok_flags},
            //like enum but stores flags in packed format, allows annotations and functions
            //usage:
            //flags mode {
            //  is_async;
            //  skip_invalid;
            //  is_lazy;
            //
            //  fn get_parser() > json_parser/xml_parser/html_parser;
            //}


            {"extern", tokens::tok_extern}, //used to call function that already declared in runtime
            //usage
            //extern `type` `name`;
            //extern `fn/gen/async` `name`(`type` `name`, ...);
            //extern(`any registered language`) `type` `name`;  //applies various conversions for this declaration
            //extern(`any registered language`)  `fn/gen/async` `name`(`type` `name`, ...);//applies various conversions for this declaration

            {"sizeof", tokens::tok_sizeof},   //returns type/value/class/function size, for void is 0
            {"typedef", tokens::tok_typedef}, //defines type in c++ style

            {"using", tokens::tok_using}, //usage:
            //using alias `name` = `identifier`;// creates alias for this identifier, also allowed namespaces in identifier, like: `namespace`.`namespace`.`identifier`
            //using namespace `namespace`;//use namespace in this scope, also allowed specifying namespaces to target namespace, like: `namespace`.`namespace`.....`namespace`
            //using enum `enum`;
            //using flags `enum`;

            //using file, utf8_file, utf16_file, utf32_file, ascii_file
            //  using file byte[] `name` = "`path`";//reads file on load, binary file
            //  using file `type` `name` = "`path`";
            //      ** can be any other class/interface/union/enum/flags
            //         that implements function "from_using_file" that accepts file_handle, blocking_file_handle or byte array,
            //         return ignored, must be non static
            //  using *_file `type` `name` = "`path`";
            //      ** can be any other class/interface/union/enum/flags
            //         that implements function "from_using_file" that accepts text_file, string or string array,
            //         return ignored, must be non static
            //
            //  using utf8_file string[] `name` = "`path`";//reads file on load, utf-8 encoded file, splits by \n and \0
            //  using utf16_file string `name` = "`path`";//reads file on load, utf-16 encoded file, stops on \0
            //
            //  using utf32_file string[] `name` = "`path`";//reads file on load, utf-32 encoded file, splits by \n and \0


            {"namespace", tokens::tok_namespace}, //usage:
            //namespace `name`{...}
            //namespace `name`.`name`{...}
            //namespace `name`:
            //namespace `name`.`name`:
            //namespace . {} //anonymous declarations
            {"typename", tokens::tok_typename},
            {"decltype", tokens::tok_decltype}, //extract type from(value), usage:
            //decltype(`value`)
            {"static_assert", tokens::tok_static_assert}, //static_assert(`condition`,`optional reason`);//stops build if condition not met
            {"constexpr", tokens::tok_constexpr},         //used to declare values or functions that will be calculated in build time
            {"import", tokens::tok_import},               //gives hint, which module will be used
            {"export", tokens::tok_export},               //registers module in this namespace, usage: export `namespace`.`name`; export `name`; export `namespace`.namespace`.`name`;


            {"this", tokens::tok_this},               //refers to this class, to use this class defintions, usage: this.`...`
            {"base", tokens::tok_base},               //refers to base class, to use base class defintions, usage: base.`...`
            {"public", tokens::tok_public},           //can be used as modifier to function or type or like c++ style: public fn... or public: ....
            {"protected", tokens::tok_protected},     //same as above
            {"private", tokens::tok_private},         //same as above
            {"internal", tokens::tok_internal},       //same as above
            {"implicit", tokens::tok_implicit},       //used as modifier to manage language constructor or operator selection behavior
            {"explicit", tokens::tok_explicit},       //same as above
            {"sealed", tokens::tok_sealed},           //seals function, or class works only for this language
            {"virtual", tokens::tok_virtual},         //declares if function will be calculated statically by language or by runtime, if declared as virtual then function will be selected by runtime
            {"override", tokens::tok_override},       //declares function overloaded, if not then function will be called only if function selected by optimizer in language
            {"operator", tokens::tok_operator},       //declares custom operator
            {"constructor", tokens::tok_constructor}, //declares constructor
            {"destructor", tokens::tok_destructor},   //declares destructor


            {"try", tokens::tok_try}, //
            {"catch", tokens::tok_catch},
            {"finally", tokens::tok_finally},
            {"filter", tokens::tok_filter},
            {"throw", tokens::tok_throw},         //throws exception, constructs type with this arguments and falls runtime to exception state
            {"exception", tokens::tok_exception}, //used to access exception runtime state and data, valid only in filter, and partially in finally(only read)


            {"in", tokens::tok_in},   //modifier in arguments to signal pass value by reference, and value must be constructed, guarantees that the value will not be changed
            {"out", tokens::tok_out}, //modifier in arguments to signal pass value by reference, and value can be not constructed
            {"ref", tokens::tok_ref}, //modifier in arguments to signal pass value by reference, and value must be constructed, full access


            {"inline", tokens::tok_inline},     //modifier for functions to inline function to caller
            {"noexcept", tokens::tok_noexcept}, //modifier for functions to signal that function will not throw any exception


            {"null", tokens::tok_null},
            {"true", tokens::tok_true},
            {"false", tokens::tok_false},


        };

        inline static std::unordered_map<std::string, tokens> operators = {
            {"+", tokens::tok__plus},
            {"-", tokens::tok__minus},
            {"*", tokens::tok__multiply},
            {"/", tokens::tok__divide},
            {"%", tokens::tok__rest},
            {"++", tokens::tok__inc},
            {"--", tokens::tok__dec},
            {"+=", tokens::tok__assign_plus},
            {"-=", tokens::tok__assign_minus},
            {"*=", tokens::tok__assign_multiply},
            {"/=", tokens::tok__assign_divide},
            {"%=", tokens::tok__assign_rest},
            {"==", tokens::tok__equal},
            {"!=", tokens::tok__not_equal},
            {">", tokens::tok__bigger},
            {"<", tokens::tok__lower},
            {">=", tokens::tok__bigger_or_equal},
            {"<=", tokens::tok__lower_or_equal},
            {"&&", tokens::tok__logical_and},
            {"||", tokens::tok__logical_or},
            {"!", tokens::tok__logical_not},
            {"&", tokens::tok__binary_and},
            {"|", tokens::tok__binary_or},
            {"^", tokens::tok__binary_xor},
            {"~", tokens::tok__binary_invert},
            {"<<", tokens::tok__binary_left_shift},
            {">>", tokens::tok__binary_right_shift},
            {">>=", tokens::tok__binary_assign_right_shift},
            {"<<=", tokens::tok__binary_assign_left_shift},
            {"&=", tokens::tok__binary_assign_and},
            {"|=", tokens::tok__binary_assign_or},
            {"^=", tokens::tok__binary_assign_xor},
            {"~=", tokens::tok__binary_assign_invert},
        };

        inline static std::unordered_set<char> keep_delimiters_set = {'+', '-', '*', '/', '%', '(', ')', '[', ']', '{', '}', ';', ':', ',', '.', '?', '!', '=', '<', '>', '&', '|', '^', '~', '\\', '\'', '\"', '`', '#', '@', '$'};

        inline static std::string keep_delimiters = {
            '+',
            '-',
            '*',
            '/',
            '%',
            '(',
            ')',
            '[',
            ']',
            '{',
            '}',
            ';',
            ':',
            ',',
            '.',
            '?',
            '!',
            '=',
            '<',
            '>',
            '&',
            '|',
            '^',
            '~',
            '\\',
            '\'',
            '\"',
            '`',
            '#',
            '@',
            '$',
            '\n',
            ' ',
            '\t',
        };

        struct identifier_data {
            list_array<std::string> namespaces;
            std::string identifier;
        };

        struct value_data {
            std::optional<std::variant<ValueItem, identifier_data>> data; //ValueItem(integer|string) | identifier_data(global_identifier)
            std::string value_name;                                       //used in maps

            list_array<value_data> initializer_data;
            list_array<std::variant<value_data, tokens>> expression_data;

            struct function_call_t {
                list_array<value_data> arguments;
                identifier_data symbol;
            };

            function_call_t function_call;

            enum class type_t {
                default_,
                map_,
                array_,
                expression_,
                function_call,
            } type = type_t::default_;
        };

        struct annotation_data {
            std::string name;
            list_array<std::pair<std::string, value_data>> values;
        };

        struct type_data {
            list_array<annotation_data> annotations;
            list_array<std::string> namespaces;
            std::variant<tokens, std::string> type;
            bool is_static = false;
            bool is_checked = false;
            bool is_signed = false;
            bool is_const = false;
            bool is_final = false;
            bool is_pointer = false;
            bool is_gc = false;

            enum class refrence_type_t {
                none,
                ref,
                out,
                in,
            } refrence_type = refrence_type_t::none;
        };

        struct global_function_defintion {
            list_array<annotation_data> annotations;
            std::unordered_map<std::string, std::variant<tokens, std::string>> named_args;
            enum class type_ {
                normal,
                async,
                generator,
            } type;
            bool is_constexpr;
        };

        struct struct_defintion {
            list_array<std::variant<tokens, std::string>> array_values;
            std::unordered_map<std::string, std::variant<tokens, std::string>> named_values;

            list_array<std::string> follows;
        };

        struct token_data {
            tokens token;
            std::variant<std::string, int64_t, double, uint64_t> data;
        };

        struct excepted_state {
            tokens current_token;
            list_array<typed_lgr<excepted_state>> next_states;
            std::function<void()> callback;
            std::function<list_array<typed_lgr<excepted_state>>()> conditional_token; //returns next states, can throw exceptions
        };

        token_data current_token;
        type_data current_type;
        std::optional<value_data> current_value;
        annotation_data current_annotation;
        list_array<annotation_data> current_annotations;
        list_array<std::string> current_namespaces;


        list_array<identifier_data> cached_identifiers;

        list_array<value_data> cached_values;
        list_array<size_t> cached_values_usage;

        list_array<std::string> cached_any;


        list_array<tokens> block_closers;

        struct assignable_tree_data_t {
            enum class state {
                _none,
                _construct,
                _array,
                _map,
            };
            list_array<state> history;
            state current;
        } assignable_tree_data;

        list_array<typed_lgr<excepted_state>> building_tree;


        list_array<std::pair<std::string, std::vector<uint8_t>>> initializer_functions;
        list_array<std::pair<std::string, std::vector<uint8_t>>> cached_functions;
        list_array<std::pair<std::string, std::vector<uint8_t>>> global_functions;

        typed_lgr<excepted_state> build_type(list_array<typed_lgr<excepted_state>> then_states) {
            typed_lgr<excepted_state> type_namespace_next = new excepted_state{
                .current_token = tokens::tok_delimiter_dot,
                .callback = [&] {
                    current_type.namespaces.push_back(std::get<std::string>(std::move(current_type.type)));
                }
            };
            typed_lgr<excepted_state> ind_next = new excepted_state{
                .current_token = tokens::identifier,
                .next_states = {
                    then_states,
                    type_namespace_next
                },
                .callback = [&] {
                    current_type.type = std::get<std::string>(std::move(current_token.data));
                },
            };
            type_namespace_next->next_states = {ind_next};
            return ind_next;
        }

        typed_lgr<excepted_state> build_identifier(list_array<typed_lgr<excepted_state>> then_states) {
            typed_lgr<excepted_state> type_namespace_next = new excepted_state{
                .current_token = tokens::tok_delimiter_dot,
                .callback = [&] {
                    auto& ind = std::get<identifier_data>(*current_value->data);
                    ind.namespaces.push_back(std::move(ind.identifier));
                }
            };
            typed_lgr<excepted_state> ind_next = new excepted_state{
                .current_token = tokens::identifier,
                .next_states = {
                    then_states,
                    type_namespace_next
                },
                .callback = [&] {
                    if (!current_value)
                        current_value = value_data();
                    auto& ind = std::get<identifier_data>(*current_value->data);
                    ind.identifier = std::get<std::string>(std::move(current_token.data));
                },
            };
            type_namespace_next->next_states = {ind_next};
            return ind_next;
        }

        list_array<typed_lgr<excepted_state>> build_assignable_expression_value(list_array<typed_lgr<excepted_state>> then_states) {
            auto value_callback = [&] {
                current_value = value_data();
                switch (current_token.token) {
                case tokens::tok_true:
                    current_value->data = ValueItem(true);
                    break;
                case tokens::tok_false:
                    current_value->data = ValueItem(false);
                    break;
                case tokens::tok_null:
                    current_value->data = ValueItem(nullptr);
                    break;
                case tokens::value_string:
                case tokens::value_hex:
                case tokens::value_long:
                case tokens::value_double: {
                    std::visit(
                        [&](auto& value) {
                            using T = std::decay_t<decltype(value)>;
                            if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double>) {
                                current_value->data = ValueItem(value);
                            } else
                                current_value->data = ValueItem((ustring)std::get<std::string>(std::move(current_token.data)));
                        },
                        current_token.data
                    );
                    break;
                }
                default:
                    break;
                }
            };

            //typed_lgr<excepted_state> constructor_begin = new excepted_state{
            //    .current_token = tokens::tok_block_begin_operator,
            //    .callback = [&] {
            //        cached_identifiers.push_back(std::get<identifier_data>(current_value.data));
            //        assignable_tree_data.history.push_back(assignable_tree_data.current);
            //        assignable_tree_data.current = assignable_tree_data_t::state::_construct;
            //        block_closers.push_back(tokens::tok_block_end_operator);
            //    }
            //};

            typed_lgr<excepted_state> array_in_place = new excepted_state{
                .current_token = tokens::tok_block_begin_index,
                .callback = [&] {
                    cached_values_usage.push_back(0);
                    block_closers.push_back(tokens::tok_block_end_index);
                }
            };

            typed_lgr<excepted_state> map_in_place = new excepted_state{
                .current_token = tokens::tok_block_begin_scope,
                .callback = [&] {
                    cached_values_usage.push_back(0);
                    block_closers.push_back(tokens::tok_block_end_scope);
                }
            };

            typed_lgr<excepted_state> array_in_place_end = new excepted_state{
                .current_token = tokens::tok_block_end_index,
                .callback =
                    [&] {
                        if (current_value) {
                            cached_values.push_back(std::move(*current_value));
                            ++cached_values_usage.back();
                        }
                        size_t used_values = cached_values_usage.take_back();
                        current_value = value_data();
                        current_value->initializer_data = cached_values.take(cached_values.size() - used_values, used_values);
                        current_value->type = value_data::type_t::array_;
                    },
            };


            typed_lgr<excepted_state> map_item = new excepted_state{
                .current_token = tokens::value_string,
                .callback = [&] {
                    cached_any.push_back(std::get<std::string>(std::move(current_token.data)));
                }
            };

            typed_lgr<excepted_state> map_in_place_end = new excepted_state{
                .current_token = tokens::tok_block_end_index,
                .callback = [&] {
                    if (current_value) {
                        cached_values.push_back(std::move(*current_value));
                        ++cached_values_usage.back();
                    }
                    size_t used_values = cached_values_usage.take_back();
                    current_value = value_data();
                    current_value->initializer_data = cached_values.take(cached_values.size() - used_values, used_values);
                    current_value->type = value_data::type_t::map_;
                }
            };


            typed_lgr<excepted_state> unified_delimiter = new excepted_state{
                .current_token = tokens::tok_delimiter_coma,
                .callback =
                    [&] {
                        if (!current_value || cached_values_usage.empty() || block_closers.empty())
                            throw InvalidInput("Symbol `,` is unexpected here.");
                        if (block_closers.back() == tokens::tok_block_end_scope)
                            current_value->value_name = cached_any.take_back();
                        ++cached_values_usage.back();
                        cached_values.push_back(std::move(*current_value));
                    },
            };

            list_array<typed_lgr<excepted_state>> result{
                build_identifier({then_states, /*constructor_begin*/}),
                new excepted_state{.current_token = tokens::value_double, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_hex, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_long, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_string, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_null, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_true, .next_states = then_states, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_false, .next_states = then_states, .callback = value_callback},
                array_in_place,
                map_in_place
            };

            list_array<typed_lgr<excepted_state>> inner_next{
                build_identifier({unified_delimiter, /*constructor_begin*/}),
                new excepted_state{.current_token = tokens::value_double, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_hex, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_long, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::value_string, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_null, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_true, .next_states = {unified_delimiter}, .callback = value_callback},
                new excepted_state{.current_token = tokens::tok_false, .next_states = {unified_delimiter}, .callback = value_callback},
                array_in_place,
                map_in_place,
                unified_delimiter
            };

            unified_delimiter->conditional_token =
                [&, inner_next = inner_next, map_in_place_end = map_in_place_end, map_item = map_item, array_in_place_end = array_in_place_end]() -> list_array<typed_lgr<excepted_state>> {
                if (block_closers.empty())
                    throw InvalidInput("Symbol `,` is unexpected here.");

                switch (block_closers.back()) {
                case tokens::tok_block_end_index:
                    return {
                        inner_next,
                        array_in_place_end
                    };
                case tokens::tok_block_end_scope:
                    return {
                        map_item,
                        map_in_place_end
                    };
                default:
                    throw InvalidInput("Symbol `,` is unexpected here.");
                }
            };

            array_in_place_end->conditional_token =
                [&, then_states = then_states, array_in_place_end = array_in_place_end, unified_delimiter = unified_delimiter]() -> list_array<typed_lgr<excepted_state>> {
                if (block_closers.empty())
                    throw InvalidInput("Symbol `]` is unexpected here.");
                if (block_closers.back() == tokens::tok_block_end_index) {
                    block_closers.pop_back();
                    if (block_closers.empty())
                        return then_states;
                    else {
                        return {
                            unified_delimiter,
                            array_in_place_end
                        };
                    }
                } else
                    throw InvalidInput("Symbol `]` is unexpected here.");
            };

            map_in_place_end->conditional_token =
                [&, then_states = then_states, map_in_place_end = map_in_place_end, unified_delimiter = unified_delimiter]() -> list_array<typed_lgr<excepted_state>> {
                if (block_closers.empty())
                    throw InvalidInput("Symbol `}` is unexpected here.");
                if (block_closers.back() == tokens::tok_block_end_scope) {
                    block_closers.pop_back();
                    if (block_closers.empty())
                        return then_states;
                    else {
                        return {
                            unified_delimiter,
                            map_in_place_end
                        };
                    }
                } else
                    throw InvalidInput("Symbol `}` is unexpected here.");
            };

            map_item->next_states = {
                new excepted_state{
                    .current_token = tokens::tok_delimiter_colon,
                    .next_states = {inner_next}
                }
            };
            array_in_place->next_states = {
                inner_next,
                unified_delimiter
            };
            map_in_place->next_states = {
                map_item,
                unified_delimiter
            };
            return result;
        }

        list_array<typed_lgr<excepted_state>> build_assignable_expression(list_array<typed_lgr<excepted_state>> then_states) {
        }

        typed_lgr<excepted_state> build_annotation_acceptor_ind(list_array<typed_lgr<excepted_state>> then_states) {
            typed_lgr<excepted_state> argument_next = new excepted_state{
                .current_token = tokens::tok_delimiter_coma,
                .callback = [&] { current_annotation.values.push_back({cached_any.take_back(), std::move(*current_value)}); }
            };
            typed_lgr<excepted_state> argument_end = new excepted_state{
                .current_token = tokens::tok_block_end_operator,
                .next_states = then_states,
                .callback = [&] {
                    current_annotation.values.push_back({cached_any.take_back(), std::move(*current_value)});
                    current_annotations.push_back(std::move(current_annotation));
                }
            };

            typed_lgr<excepted_state> argument_begin = new excepted_state{
                .current_token = tokens::tok_delimiter_dot,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::identifier,
                        .next_states{
                            new excepted_state{
                                .current_token = tokens::tok__assign,
                                .next_states = build_assignable_expression({argument_next, argument_end}),
                            }

                        },
                        .callback = [&] { cached_any.push_back(std::get<std::string>(std::move(current_token.data))); }
                    }
                }
            };
            argument_next->next_states = {argument_begin};

            return new excepted_state{
                .current_token = tokens::identifier,
                .next_states = {
                    new excepted_state{
                        .current_token = tokens::tok_block_begin_operator,
                        .next_states{
                            argument_begin,
                        }
                    },
                    new excepted_state{
                        .current_token = tokens::vir__pre_call,
                        .next_states = then_states,
                        .callback = [&] {
                            current_annotations.push_back(std::move(current_annotation));
                        }
                    }
                },
                .callback = [&] {
                    current_annotation.name = std::get<std::string>(std::move(current_token.data));
                }
            };
        }

        typed_lgr<excepted_state> build_annotation_acceptor(list_array<typed_lgr<excepted_state>> then_states) {
            typed_lgr<excepted_state> next_annotation = new excepted_state{
                .current_token = tokens::tok_delimiter_coma,
            };
            next_annotation->next_states = {build_annotation_acceptor_ind({next_annotation, new excepted_state{.current_token = tokens::tok_block_end_index, .next_states = then_states}})};

            typed_lgr<excepted_state> annotation_begin = new excepted_state{
                .current_token = tokens::tok_delimiter_annotation,
                .next_states{
                    build_annotation_acceptor_ind(then_states),
                    new excepted_state{
                        .current_token = tokens::tok_block_begin_index,
                        .next_states = next_annotation->next_states
                    }
                }
            };
        }

        list_array<typed_lgr<excepted_state>> build_type_id(list_array<typed_lgr<excepted_state>> then_states, bool in_argument_context = true) {
            then_states.push_back(new excepted_state{
                .current_token = tokens::tok__multiply,
                .next_states = then_states,
                .callback = [&] {
                    current_type.is_pointer = true;
                },
            });

            then_states.push_back(new excepted_state{
                //gc pointer, yes gc pointer to regular pointer is allowed
                .current_token = tokens::tok__binary_xor,
                .next_states = then_states,
                .callback = [&] {
                    current_type.is_gc = true;
                },
            });


            auto built_in_type_set = [&] {
                current_type.type = current_token.token;
            };

            list_array<typed_lgr<excepted_state>> res;
            typed_lgr<excepted_state> any_type_def = new excepted_state{
                .current_token = tokens::tok_any,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> char_type_def = new excepted_state{
                .current_token = tokens::tok_char,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> double_type_def = new excepted_state{
                .current_token = tokens::tok_double,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> float_type_def = new excepted_state{
                .current_token = tokens::tok_float,
                .next_states = then_states,
                .callback = built_in_type_set,
            };
            typed_lgr<excepted_state> short_type_def = new excepted_state{
                .current_token = tokens::tok_short,
                .next_states = then_states,
                .callback = built_in_type_set,
            };
            typed_lgr<excepted_state> int_type_def = new excepted_state{
                .current_token = tokens::tok_int,
                .next_states = then_states,
                .callback = built_in_type_set,
            };
            typed_lgr<excepted_state> long_type_def = new excepted_state{
                .current_token = tokens::tok_long,
                .next_states = then_states,
                .callback = built_in_type_set,
            };
            typed_lgr<excepted_state> byte_type_def = new excepted_state{
                .current_token = tokens::tok_byte,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> string_type_def = new excepted_state{
                .current_token = tokens::tok_string,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> arr_type_def = new excepted_state{
                .current_token = tokens::tok_arr,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> map_type_def = new excepted_state{
                .current_token = tokens::tok_map,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> map_type_def = new excepted_state{
                .current_token = tokens::tok_void,
                .next_states = then_states,
                .callback = built_in_type_set,
            };

            typed_lgr<excepted_state> custom_type_def = build_type(then_states);


            typed_lgr<excepted_state> signed_type_def = new excepted_state{
                .current_token = tokens::tok_signed,
                .next_states = {
                    char_type_def,
                    short_type_def,
                    int_type_def,
                    long_type_def,
                    byte_type_def
                },
                .callback = [&] {
                    current_type.is_signed = true;
                },
            };

            typed_lgr<excepted_state> unsigned_type_def = new excepted_state{
                .current_token = tokens::tok_unsigned,
                .next_states = {
                    char_type_def,
                    short_type_def,
                    int_type_def,
                    long_type_def,
                    byte_type_def
                },
                .callback = [&] {
                    current_type.is_signed = false;
                },
            };


            res.reserve(19);
            res = {
                any_type_def,
                char_type_def,
                double_type_def,
                float_type_def,
                short_type_def,
                int_type_def,
                long_type_def,
                byte_type_def,
                string_type_def,
                arr_type_def,
                map_type_def,
                custom_type_def,
                signed_type_def,
                unsigned_type_def,
                new excepted_state{
                    .current_token = tokens::tok_checked,
                    .next_states = {
                        double_type_def,
                        float_type_def,
                        short_type_def,
                        int_type_def,
                        long_type_def,
                        byte_type_def
                    },
                    .callback = [&] {
                        current_type.is_checked = true;
                    },
                }
            };

            typed_lgr<excepted_state> _const = new excepted_state{
                .current_token = tokens::tok_const,
                .next_states = res,
                .callback = [&] {
                    current_type.is_const = true;
                },
            };


            if (in_argument_context) {
                res.push_back(new excepted_state{
                    .current_token = tokens::tok_final,
                    .next_states = res,
                    .callback = [&] {
                        current_type.is_final = true;
                    },
                });
                res.push_back(_const);
                res.push_back(new excepted_state{
                    .current_token = tokens::tok_static,
                    .next_states = res,
                    .callback = [&] {
                        current_type.is_static = true;
                    },
                });
            } else {
                res.push_back(_const);
                typed_lgr<excepted_state> _ref = new excepted_state{
                    .current_token = tokens::tok_ref,
                    .next_states = res,
                    .callback = [&] {
                        current_type.refrence_type = type_data::refrence_type_t::ref;
                    },
                };
                typed_lgr<excepted_state> _out = new excepted_state{
                    .current_token = tokens::tok_out,
                    .next_states = res,
                    .callback = [&] {
                        current_type.refrence_type = type_data::refrence_type_t::out;
                    },
                };
                typed_lgr<excepted_state> _in = new excepted_state{
                    .current_token = tokens::tok_in,
                    .next_states = res,
                    .callback = [&] {
                        current_type.refrence_type = type_data::refrence_type_t::in;
                    },
                };
                res.push_back(_ref);
                res.push_back(_out);
                res.push_back(_in);
            }
        }

        list_array<typed_lgr<excepted_state>> build_annotated_type_id(list_array<typed_lgr<excepted_state>> then_states, bool in_argument_context = true) {

            auto res = build_type_id(then_states, in_argument_context);
            res.push_back(build_annotation_acceptor(res));
            return res;
        }

        typed_lgr<excepted_state> build_arguments(list_array<typed_lgr<excepted_state>> then_states) {
            typed_lgr<excepted_state> next_argument = new excepted_state{
                .current_token = tokens::tok_delimiter_coma
            };

            typed_lgr<excepted_state> after_arguments = new excepted_state{
                .current_token = tokens::tok_block_end_operator,
                .next_states{then_states}
            };

            next_argument->next_states = build_annotated_type_id({
                new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{
                        next_argument,
                        after_arguments
                    }
                },
            });
            return new excepted_state{
                .current_token = tokens::tok_block_begin_operator,
                .next_states = next_argument->next_states
            };
        }

        typed_lgr<excepted_state> build_excepted_tree_fn(tokens fun_token, typed_lgr<excepted_state> function_state) {
            typed_lgr<excepted_state> next_type = new excepted_state{
                .current_token = tokens::tok__divide
            };
            next_type->next_states = build_annotated_type_id({
                new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{
                        next_type,
                        function_state
                    }
                },
            });

            auto arguments = build_arguments({

                new excepted_state{
                    .current_token = tokens::tok__bigger,
                    .next_states = build_annotated_type_id({
                        new excepted_state{.current_token = tokens::tok__bigger, .next_states = next_type->next_states},
                    }),
                },
                function_state,
                new excepted_state{.current_token = tokens::tok_delimiter_semicolon}
            });

            return new excepted_state{
                .current_token = fun_token,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::identifier,
                        .next_states{arguments}
                    },
                }
            };
        }

        typed_lgr<excepted_state> build_excepted_anonymous_tree_fn(tokens fun_token, typed_lgr<excepted_state> function_state) {
            typed_lgr<excepted_state> next_type = new excepted_state{
                .current_token = tokens::tok__divide
            };
            next_type->next_states = build_annotated_type_id({
                new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{
                        next_type,
                        function_state
                    }
                },
            });
            auto arguments = build_arguments({

                new excepted_state{
                    .current_token = tokens::tok__bigger,
                    .next_states = build_annotated_type_id({
                        new excepted_state{.current_token = tokens::tok__bigger, .next_states = next_type->next_states},
                    }),
                },
                function_state,
                new excepted_state{.current_token = tokens::tok_delimiter_semicolon}
            });

            return new excepted_state{
                .current_token = fun_token,
                .next_states{arguments}
            };
        }

        typed_lgr<excepted_state> build_excepted_special_tree(tokens special, typed_lgr<excepted_state> next_state) {
            auto arguments = build_arguments({next_state, new excepted_state{.current_token = tokens::tok_delimiter_semicolon}});

            return new excepted_state{
                .current_token = special,
                .next_states{arguments}
            };
        }

        typed_lgr<excepted_state> build_excepted_operator_tree(typed_lgr<excepted_state> next_state) {
            auto arguments = build_arguments({next_state, new excepted_state{.current_token = tokens::tok_delimiter_semicolon}});

            return new excepted_state{
                .current_token = tokens::tok_operator,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::tok__plus,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__minus,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__multiply,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__divide,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__rest,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__inc,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__dec,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__assign_plus,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__assign_minus,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__assign_multiply,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__assign_divide,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__assign_rest,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__equal,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__not_equal,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__bigger,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__lower,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__bigger_or_equal,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__lower_or_equal,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__logical_and,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__logical_or,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__logical_not,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_and,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_or,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_xor,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_invert,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_left_shift,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_right_shift,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_right_shift,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_left_shift,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_and,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_or,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_xor,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::tok__binary_assign_invert,
                        .next_states{arguments}
                    },
                    new excepted_state{
                        .current_token = tokens::identifier, //cast
                        .next_states{arguments}
                    },
                }
            };
        }

        typed_lgr<excepted_state> build_excepted_tree_annotation(typed_lgr<excepted_state> annotation_state) {

            typed_lgr<excepted_state> next_type = new excepted_state{
                .current_token = tokens::tok__divide
            };

            auto arguments = build_arguments({annotation_state});

            next_type->next_states = {
                new excepted_state{
                    .current_token = tokens::tok_fn,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_gen,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_async,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_struct,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_class,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_union,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_enum,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_interface,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::tok_flags,
                    .next_states{annotation_state, next_type, arguments}
                },
                new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{annotation_state, next_type, arguments}
                },
            };


            return new excepted_state{
                .current_token = tokens::tok_annotation,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::identifier,
                        .next_states = next_type->next_states
                    },
                }
            };
        }

        typed_lgr<excepted_state> build_excepted_struct() {
            typed_lgr<excepted_state> short_def_next = new excepted_state{
                .current_token = tokens::tok_delimiter_coma
            };
            typed_lgr<excepted_state> end_short_def = new excepted_state{
                .current_token = tokens::tok_block_end_operator,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::tok_delimiter_semicolon,
                        //.callback = callback
                    },
                }
            };
            short_def_next->next_states = build_annotated_type_id({
                new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{
                        short_def_next,
                        end_short_def,
                    }
                },
                short_def_next,
                end_short_def,
            });


            typed_lgr<excepted_state> def_next = new excepted_state{
                .current_token = tokens::tok_delimiter_semicolon
            };
            typed_lgr<excepted_state> end_def = new excepted_state{
                .current_token = tokens::tok_block_end_scope,
                .next_states{
                    new excepted_state{
                        .current_token = tokens::tok_delimiter_semicolon,
                        //.callback = callback
                    },
                }
            };

            def_next->next_states = build_annotated_type_id({new excepted_state{
                .current_token = tokens::identifier,
                .next_states{def_next, end_def}
            }});

            typed_lgr<excepted_state> short_begin = new excepted_state{
                .current_token = tokens::tok_block_begin_operator,
                .next_states = short_def_next->next_states
            };
            typed_lgr<excepted_state> long_begin = new excepted_state{
                .current_token = tokens::tok_block_begin_scope,
                .next_states = def_next->next_states
            };

            typed_lgr<excepted_state> _follow_def_next = new excepted_state{
                .current_token = tokens::tok_delimiter_coma,
            };
            typed_lgr<excepted_state> _follow_def = new excepted_state{
                .current_token = tokens::identifier,
                .next_states = {
                    short_begin,
                    long_begin,
                    _follow_def_next,
                    new excepted_state{
                        .current_token = tokens::tok_delimiter_semicolon,
                        //.callback = callback
                    }
                }
            };
            _follow_def_next->next_states = {_follow_def};

            typed_lgr<excepted_state> _follow_begin = new excepted_state{
                .current_token = tokens::tok_delimiter_colon,
                .next_states = {_follow_def}
            };

            typed_lgr<excepted_state> nop_begin = new excepted_state{.current_token = tokens::tok_delimiter_semicolon};

            return new excepted_state{
                .current_token = tokens::tok_struct,
                .next_states{new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{_follow_begin, short_begin, long_begin, nop_begin}
                }}
            };
        }

        typed_lgr<excepted_state> build_excepted_class() {
            typed_lgr<excepted_state> def_next = new excepted_state{
                .current_token = tokens::tok_delimiter_semicolon,
            };
            auto def_types = build_annotated_type_id({def_next}, false);
            auto def_fun = build_excepted_tree_fn(tokens::tok_fn, {def_next});
            auto def_gen = build_excepted_tree_fn(tokens::tok_gen, {def_next});
            auto def_async = build_excepted_tree_fn(tokens::tok_gen, {def_next});
            auto constructor_ = build_excepted_special_tree(tokens::tok_constructor, {def_next});
            auto destructor_ = build_excepted_special_tree(tokens::tok_destructor, {def_next});
            auto operator_ = build_excepted_operator_tree({def_next});
            auto _implicit = new excepted_state{
                .current_token = tokens::tok_implicit,
                .next_states{
                    constructor_,
                    operator_,
                }
            };
            auto _explicit = new excepted_state{
                .current_token = tokens::tok_explicit,
                .next_states{
                    constructor_,
                    operator_,
                }
            };


            list_array<typed_lgr<excepted_state>> def_specified_excepted{
                new excepted_state{
                    .current_token = tokens::tok_sealed,
                    .next_states{
                        def_fun,
                        def_gen,
                        def_async,
                        destructor_,
                        operator_,
                        _implicit,
                        _explicit,
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_virtual,
                    .next_states{
                        def_fun,
                        def_gen,
                        def_async,
                        destructor_,
                        operator_,
                        _implicit,
                        _explicit,
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_override,
                    .next_states{
                        def_fun,
                        def_gen,
                        def_async,
                        destructor_,
                        operator_,
                        _implicit,
                        _explicit,
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_final,
                    .next_states{
                        def_fun,
                        def_gen,
                        def_async,
                        destructor_,
                        operator_,
                        _implicit,
                        _explicit,
                    }
                },
                _implicit,
                _explicit,
                def_types,
                def_fun,
                def_gen,
                def_async,

            };


            def_next->next_states = {
                new excepted_state{
                    .current_token = tokens::tok_public,
                    .next_states{
                        new excepted_state{
                            .current_token = tokens::tok_delimiter_colon,
                        },
                        def_specified_excepted
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_protected,
                    .next_states{
                        new excepted_state{
                            .current_token = tokens::tok_delimiter_colon,
                        },
                        def_specified_excepted,
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_private,
                    .next_states{
                        new excepted_state{
                            .current_token = tokens::tok_delimiter_colon,
                        },
                        def_specified_excepted,
                    }
                },
                new excepted_state{
                    .current_token = tokens::tok_internal,
                    .next_states{
                        new excepted_state{
                            .current_token = tokens::tok_delimiter_colon,
                        },
                        def_specified_excepted,
                    }
                },
                def_specified_excepted
            };
            def_next->next_states.push_back(build_annotation_acceptor(def_next->next_states));


            typed_lgr<excepted_state> long_begin = new excepted_state{
                .current_token = tokens::tok_block_begin_scope,
                .next_states = def_next->next_states
            };


            typed_lgr<excepted_state> _follow_def_next = new excepted_state{
                .current_token = tokens::tok_delimiter_coma,
            };
            typed_lgr<excepted_state> _follow_def = new excepted_state{
                .current_token = tokens::identifier,
                .next_states = {
                    long_begin,
                    _follow_def_next,
                    new excepted_state{
                        .current_token = tokens::tok_delimiter_semicolon,
                        //.callback = callback
                    }
                }
            };
            _follow_def_next->next_states = {_follow_def};

            typed_lgr<excepted_state> _follow_begin = new excepted_state{
                .current_token = tokens::tok_delimiter_colon,
                .next_states = {_follow_def}
            };
            typed_lgr<excepted_state> nop_begin = new excepted_state{.current_token = tokens::tok_delimiter_semicolon};

            typed_lgr<excepted_state> _class_def = new excepted_state{
                .current_token = tokens::tok_class,
                .next_states{new excepted_state{
                    .current_token = tokens::identifier,
                    .next_states{_follow_begin, long_begin, nop_begin}
                }}
            };
            return _class_def;
        }

        typed_lgr<excepted_state> build_excepted_union() {
        }

        typed_lgr<excepted_state> build_excepted_enum() {
        }

        typed_lgr<excepted_state> build_excepted_interface() {
        }

        typed_lgr<excepted_state> build_excepted_flags() {
        }

        list_array<typed_lgr<excepted_state>> build_global_start() {
            return {
                build_excepted_tree_fn(tokens::tok_fn, {}),
                build_excepted_tree_fn(tokens::tok_async, {}),
                build_excepted_tree_fn(tokens::tok_gen, {}),
                build_excepted_tree_annotation({}),
                build_excepted_struct(),
                build_excepted_class(),
                build_excepted_union(),
                build_excepted_enum(),
                build_excepted_interface(),
                build_excepted_flags(),
                new excepted_state{
                    .current_token = tokens::tok_namespace
                },
                new excepted_state{
                    .current_token = tokens::tok_typename
                },
                new excepted_state{
                    .current_token = tokens::tok_decltype
                },
                new excepted_state{
                    .current_token = tokens::tok_static_assert
                },
                new excepted_state{
                    .current_token = tokens::tok_constexpr
                },
                new excepted_state{
                    .current_token = tokens::tok_import
                },
                new excepted_state{
                    .current_token = tokens::tok_export
                },
                new excepted_state{
                    .current_token = tokens::tok_using
                },
            };
        };

        void parse(const std::string& data) {
            boost::char_separator<char> sep("\r\f\v", keep_delimiters.c_str(), boost::keep_empty_tokens);
            boost::tokenizer<boost::char_separator<char>> tok(data, sep);


            std::string current_namespace;
            std::string current_struct_type;
            std::string current_struct_name;

            std::string current_type;
            std::string current_identifier;
            std::string current_value;

            std::string current_function;
            list_array<std::pair<std::string, std::string>> current_function_args;

            bool now_one_line_comment;
            bool now_greedy_comment;
            bool now_in_struct_declaration;

            enum class parse_context {
                global,
                _template,
                _generic,
                _struct,
                _class,
                _abstract,
                _interface,

                one_line_comment,
                greedy_comment,
                in_struct_declaration,

            };


            std::string current_operator;
            for (const std::string& token : tok) {
                if (token == "\n") {
                    now_one_line_comment = false;
                    continue;
                }
                if (keywords.contains(token)) {

                } else if (token.size() == 1 ? keep_delimiters_set.contains(token[0]) : false) {
                } else {
                }
            }
        }
    };

    void prepare_file(files::FileHandle& file, std::unordered_map<ustring, uint64_t, hash<ustring>>& file_data) {
    }

    void handle_file(files::FileHandle& file, std::unordered_map<ustring, uint64_t, hash<ustring>>& file_data) {
    }

    void handle_delete(std::unordered_map<ustring, uint64_t, hash<ustring>>& file_data) {
    }

    art::patch_list c_async::handle_init(files::FileHandle& file) {
        lock_guard guard(mutex);
        prepare_file(file, declared_functions[file.get_path()]);
    }

    art::patch_list c_async::handle_init_complete() {
        lock_guard guard(mutex);
        for (auto& [path, file_data] : declared_functions) {
            files::FileHandle file(path.c_str(), path.size(), files::open_mode::read, files::on_open_action::open_exists, files::_async_flags{});
            handle_file(file, file_data);
        }
    }

    art::patch_list c_async::handle_create(files::FileHandle& file) {
        lock_guard guard(mutex);
        handle_file(file, declared_functions[file.get_path()]);
    }

    art::patch_list c_async::handle_renamed(const ustring& old, files::FileHandle& file) {
        lock_guard guard(mutex);
        auto it = declared_functions.find(old);
        if (it != declared_functions.end()) {
            declared_functions[file.get_path()] = std::move(it->second);
            declared_functions.erase(it);
        } else
            handle_file(file, declared_functions[file.get_path()]);
    }

    art::patch_list c_async::handle_changed(files::FileHandle& file) {
        lock_guard guard(mutex);
        handle_file(file, declared_functions[file.get_path()]);
    }

    art::patch_list c_async::handle_removed(const ustring& removed) {
        lock_guard guard(mutex);
        auto it = declared_functions.find(removed);
        if (it != declared_functions.end()) {
            handle_delete(it->second);
            declared_functions.erase(it);
        }
    }
}
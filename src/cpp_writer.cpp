#include <iostream>
#include <cstring>
#include <cassert>
#include <vl.h>
#include <TypeResolver.h>
#include <utils/log.h>
#include "cpp_writer.h"
#ifdef LOG_ON
	LOG_TITLE("cpp_writer")
	SET_LOCAL_LOG_DEBUG(true)
#endif

namespace
{
	#define PRINT_INDENT(file, level) \
		for (int i = 0; i < level; i++) \
			file << "\t"

	#define PRINT(file, data) file << data

	#define INIT_PRINT(file, indent_level) \
		auto& pr_file = file; \
		auto pr_ind_level = indent_level \
	
	#define BEGIN_PRINT_LINE PRINT_INDENT(pr_file, pr_ind_level)

	#define END_PRINT_LINE PRINT(pr_file, "\n")

	#define PRINT_LINE(data) { \
		BEGIN_PRINT_LINE; \
		PRINT(pr_file, data); \
		END_PRINT_LINE; }

	#define PRINT_LINE_BREAK PRINT_LINE("")

	#define PRINT_INDENT_DECREASE \
		pr_ind_level--

	#define PRINT_INDENT_INCREASE \
		pr_ind_level++

	#define INDENT_LEVEL pr_ind_level
	
	#define PRINT_SCOPE_BEGIN \
		PRINT_LINE("{"); \
		PRINT_INDENT_INCREASE;

	#define PRINT_SCOPE_END \
		PRINT_INDENT_DECREASE; \
		PRINT_LINE("}");
	
	#define PRINT_SCOPE_END_SEMI \
		PRINT_INDENT_DECREASE; \
		PRINT_LINE("};");

	#define CLASS_DATA_INITIALIZER (has_proto() ? (get_proto_class_name(ctx.writer.get_type_resolver()) + "(data)") : "m_data(data)")
	#define CLASS_DATA_INITIALIZER_2 (has_proto() ? (get_proto_class_name(ctx.writer.get_type_resolver()) + "(vl::MakePtr(data))") : "m_data(vl::MakePtr(data))")

	#define M_DATA \
		(pdata.proto_id.empty() ? "m_data" : "_data_()")

	// Data checks
	#define PRINT_DATA_CHECK(return_ex) \
		PRINT_LINE("if (!" << M_DATA << ")"); \
		PRINT_INDENT_INCREASE; \
		PRINT_LINE("return" << return_ex << ";"); \
		PRINT_INDENT_DECREASE;

	#define PRINT_DATA_IS_OBJECT_CHECK(return_ex) \
		PRINT_LINE("if (!" << M_DATA << "->IsObject())"); \
		PRINT_INDENT_INCREASE; \
		PRINT_LINE("return" << return_ex << ";"); \
		PRINT_INDENT_DECREASE;

	// Variables declarations
	#define PRINT_DATA_OBJ \
		PRINT_LINE("auto& data_obj = " << M_DATA << "->AsObject();");

	#define VARIABLE_DECLARATION(t, n, v) \
		t << " " << n << " = " << v << ";";

	#define STATIC_VARIABLE_DECLARATION(t, n, v) \
		"static " << VARIABLE_DECLARATION(t, n, v)

	#define PRINT_DATA_OBJECT_WITH_CHECKS(return_ex) \
		PRINT_DATA_CHECK(return_ex); \
		PRINT_DATA_IS_OBJECT_CHECK(return_ex); \
		PRINT_DATA_OBJ;

	#define DATA_VAR_NAME(fn) \
		"data_" << fn

	#define PRINT_DATA_GET(fn) \
		PRINT_LINE("auto& " << DATA_VAR_NAME(fn) << " = data_obj.Get(\"" << fn << "\");");
	

	#define PRINT_RETURN_DATA_METHOD_UNQUOTED(fn, m, suffix) \
		PRINT_LINE("return data_obj." << m << "(" << fn << ")" << suffix << ";");

	// Typed data
	#define PRINT_DATA_IS_TYPE_CHECK(t, fn, return_ex); \
		PRINT_LINE("if (!" << DATA_VAR_NAME(fn) << ".Is" << t << "())"); \
		PRINT_INDENT_INCREASE; \
		PRINT_LINE("return" << return_ex << ";"); \
		PRINT_INDENT_DECREASE;

	#define PRINT_DATA_TYPE(t, fn); \
		PRINT_LINE("auto& " << DATA_VAR_NAME(fn) << "_" << t " = " << DATA_VAR_NAME(fn) << ".As" << t << "()"; );

	#define PRINT_DATA_RETURN_AS_TYPE(t, fn, suffix); \
		PRINT_LINE("return " << DATA_VAR_NAME(fn) << ".As" << t << "()" << suffix << ";");

	#define PRINT_DATA_TYPE_WITH_CHECKS(t, fn) \
		PRINT_DATA_OBJECT_WITH_CHECKS(" empty_val"); \
		PRINT_DATA_GET(fn); \
		PRINT_DATA_IS_TYPE_CHECK(t, fn, " empty_val"); \
		PRINT_DATA_TYPE(t, fn);

	#define PRINT_DATA_TYPE_RETURN_WITH_CHECKS(t, fn, suffix) \
		PRINT_DATA_OBJECT_WITH_CHECKS(" empty_val"); \
		PRINT_DATA_GET(fn); \
		PRINT_DATA_IS_TYPE_CHECK(t, fn, " empty_val"); \
		PRINT_DATA_RETURN_AS_TYPE(t, fn, suffix);

	#define PRINT_DATA_RETURN_WITH_CHECKS_UNQUOTED(fn, m, return_ex) \
		PRINT_DATA_OBJECT_WITH_CHECKS(return_ex); \
		PRINT_RETURN_DATA_METHOD_UNQUOTED(fn, m, "");
}

namespace
{
	#define CLASS_CPP_SCOPE (ctx.cpp_scope())
	#define CLASS_DEF_CONSTRUCTOR_SIGNATURE get_name() << "()"
	#define CLASS_DEF_CONSTRUCTOR_DECL CLASS_DEF_CONSTRUCTOR_SIGNATURE << " = default;"
	#define CLASS_CONSTRUCTOR_SIGNATURE get_name() << "(const vl::VarPtr& data)"
	#define CLASS_CONSTRUCTOR_2_SIGNATURE get_name() << "(const vl::Var& data)"

	#define METHOD_SIGNATURE(type, name, args, scope, suffix) \
		type << " " << scope << name << "(" << args << ")" << suffix
	#define MEHTOD_DECLARATION(type, name, args, suffix) METHOD_SIGNATURE(type, name, args, "", suffix) << ";"
	#define METHOD_DEFINITION(type, name, args, suffix) METHOD_SIGNATURE(type, name, args, CLASS_CPP_SCOPE, suffix)

	#define CLASS_GETTER_SIGNATURE(prefix, fn, c, scope, suffix) \
		METHOD_SIGNATURE(prefix << c->as_class()->get_name() << "& ", "" << fn, "", scope, suffix)
	#define CLASS_GETTER_DECLARATION(prefix, fn, c, suffix) CLASS_GETTER_SIGNATURE(prefix, fn, c, "", suffix) << ";"
	#define CLASS_GETTER_DEFINITION(prefix, fn, c, suffix) CLASS_GETTER_SIGNATURE(prefix, fn, c, CLASS_CPP_SCOPE, suffix)

	#define PRINT_SETTER_DEFINITION(fn, t) \
		PRINT_LINE(METHOD_DEFINITION("void", "set_" + fn, t << " value", "")); \
		PRINT_SCOPE_BEGIN; \
		PRINT_DATA_OBJECT_WITH_CHECKS(""); \
		PRINT_LINE("data_obj.Set(\"" << fn << "\", value);"); \
		PRINT_SCOPE_END;
}

namespace
{
	const std::set<std::string> restricted_names = { "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "reflexpr", "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq" };
	std::string process_class_name(std::string class_name) {
		if (restricted_names.find(class_name) != restricted_names.end())
			return class_name + "_t";
		std::string::size_type n = 0;
		const std::string s = ".";
		const std::string t = "::";
		while ((n = class_name.find(s, n)) != std::string::npos)
		{
			class_name.replace(n, s.size(), t);
			n += t.size();
		}
		return class_name;
	}

	bool is_path(const std::string& class_name) {
		return class_name.find_first_of(":.") != std::string::npos;
	}
	// recursion: < 0 - no recursion, otherwise it is a recursion level incrementing every iteration
	void foreach_field (
		const vl::var_desc& v
		, const std::function<void(const std::string&, const vl::var_desc_ptr&, int)>& pred
		, int recursion = 0
	)
	{
		if (v.is_class())
		{
			auto view = v.as_class()->get_fields_map();
			std::for_each(view.begin(), view.end(), [&](auto f) {
				if (recursion >= 0)
					foreach_field(*f.second, pred, recursion + 1);
				pred(f.first, f.second, recursion);
			});
		}
		else if (v.is_list())
			for (auto& f : v.as_list()->get_fields())
			{
				if (recursion >= 0)
					foreach_field(*f, pred, recursion + 1);
				pred("", f, recursion);
			}
	}
}

namespace vl
{
	// Begin of cpp_writer
	cpp_writer::cpp_writer(const TypeResolver& type_resolver, const cppw_params& params)
		: m_params(params)
		, m_type_resolver(type_resolver)
		, m_root(std::make_shared<class_desc>())
	{}

	bool cpp_writer::add_member(const var_desc_ptr& val, const char* name)
	{
		bool is_root = false;
		if (!get_current_container())
		{
			if (m_root->is_empty())
			{
				if (val->is_class() && val->as_class()->is_root())
					m_root = val->as_class();
				else
				{
					LOCAL_WARNING("Skipping field '" << name << "' on the root level");
					return false;
				}
			}
			is_root = true;
			push_current_container(m_root);
		}
		auto parent = get_current_container();

		// Ignore specified branch
		if (!m_params.ignore.empty())
			if (m_type_resolver.GetTypeId(parent->get_data()->AsObject()) == m_params.ignore)
				return false;

		// Process containers
		if (parent->is_class())
		{
			auto parent_obj = parent->as_class();
			
			if (m_params.cppgen_params.ignore_overloadings)
				if (auto proto = parent_obj->get_data()->AsObject().GetPrototype())
				{
					// Ignore overloadings
					// TODO: support overloadings cpp generation
					// 		Now the problem is that we should convern field names according to C++ syntax
					//		but still should use the original name for data access
					if (name)
						if (proto.Has(name))
							if (name != std::string("proto"))
								if (proto.Get(name).GetType() == parent_obj->get_data()->AsObject().Get(name).GetType())
									return false;
				}
			if ((val->is_class() && !is_root) || !val->is_class())
			{
				assert(name); // Any property if an object should have it's name
				auto name_str = (name ? name : "");
				var_desc_ptr new_val(nullptr);
				if (auto r = parent_obj->find_field(name_str))
				{
					if (name_str != std::string("proto")) // Ignore the proto
						parent_obj->set(name_str, *r);
					new_val = *r;
				}
				else
				{
					if (name_str != std::string("proto")) // Ignore the proto
						new_val = parent_obj->add_field(name_str, val);
					else
						new_val = val;
				}
				if (new_val->is_class() || new_val->is_list())
					push_current_container(new_val);
			}
		}
		else if (parent->is_list())
		{
			auto current_val_list = parent->as_list();
			auto& new_val = current_val_list->add_field(val);
			if (new_val->is_class() || new_val->is_list())
				push_current_container(new_val);
		}
		else
			throw "cpp_writer: unsupported container type for adding elements";
		
		return true;
	}

	bool cpp_writer::add_proto(const ObjectVar& value)
	{
		auto current_val = get_current_container();
		if (m_params.use_proto_refs)
		{
			auto type_id = m_type_resolver.GetTypeId(value);
			auto val = std::make_shared<class_desc>(vl::MakePtr(value), process_class_name(type_id), true);
			add_member(val, "proto");
			return false; // Don't visit nested 'proto' object
		}
		else if (m_params.merge_with_proto)
		{
			// Put everything from 'proto' into the current container
			push_current_container(current_val);
			if (m_params.store_type_id)
			{
				auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(value));
				add_member(val, "typeid");
			}
			return true;
		}

		return false;
	}

	var_desc_ptr cpp_writer::get_current_container()
	{
		return m_stack.empty() ? nullptr : m_stack.back();
	}

	void cpp_writer::push_current_container(const var_desc_ptr& val)
	{
		m_stack.push_back(val);
	}

	bool cpp_writer::pop_current_container()
	{
		if (m_stack.empty())
			return false;
		m_stack.pop_back();
		return true;
	}

	bool cpp_writer::VisitNull(const NullVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		return add_member(val, name);
	}

	bool cpp_writer::VisitBool(const BoolVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		return add_member(val, name);
	}

	bool cpp_writer::VisitNumber(const NumberVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		return add_member(val, name);
	}

	bool cpp_writer::VisitString(const StringVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		return add_member(val, name);
	}

	bool cpp_writer::VisitPointer(const PointerVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		return add_member(val, name);
	}

	bool cpp_writer::VisitObject(const ObjectVar& var, const char* name)
	{
		// Write CPP
		//PRINT_CPP("class " << name);
		//PRINT_CPP("{");
		//m_level++;
		// ==============
		// Create class structure
		if (!var.IsNull())
		{	// Add a new class if it is not a null object
			if (name && std::strcmp(name, "proto") == 0)
				add_proto(var);
			else
			{
				bool is_root = name == nullptr;
				if (is_root)
					begin_writing_fwd();
				if (is_root && m_params.root_name.empty())
					return true; // Skip the root
				bool is_logic_root = !get_current_container() && m_root->is_empty();
				bool is_type = m_type_resolver.IsType(var);
				std::string class_name(
					is_logic_root && is_root ? m_params.root_name
					: (is_type ? m_type_resolver.GetTypeId(var) : (name ? name : ""))
				);
				auto val = std::make_shared<class_desc>(vl::MakePtr(var), process_class_name(class_name), is_type);
				if (!add_member(val, name))
 					return false;
			}
		}
		else
		{	// Create a null object and put it to the current container
			auto current_val = get_current_container();
			if (!current_val)
				return false;
			
			auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(vl::Null()));
			if (current_val->is_list())
				current_val->as_list()->add_field(val);
			else if (current_val->is_class())
				current_val->as_class()->add_field(name, val);
		}
		return true;
	}

	bool cpp_writer::EndVisitObject(const ObjectVar& var, const char* name)
	{
		bool is_root = name == nullptr;
		if (is_root)
			end_writing_fwd();
		if (is_root && m_params.root_name.empty())
			return true; // Skip the root
		auto abstract_container = get_current_container();
		assert(abstract_container && "A current container should always exist when ending visiting an object");
		if (!abstract_container)
		{
			LOCAL_ERROR("Empty container stack when ending visiting object '" << name << "'");
			return false;
		}
		assert(abstract_container->is_class() && "Ending visiting an object should work with a class container");
		auto container = abstract_container->as_class();
		pop_current_container();
		class_desc::class_print_context ctx = { *this, *container };
		if (m_stack.empty()) // is root
		{
			container->print(ctx);
			m_root = std::make_shared<class_desc>();
		}
		else
			if (auto c = container->as_class())
				if (c->is_type())
					c->print(ctx);
		return true;
	}

	bool cpp_writer::VisitList(const ListVar& var, const char* name)
	{
		if (!var.IsNull())
		{
			auto val = std::make_shared<list_desc>(vl::MakePtr(vl::List()));
			if (!add_member(val, name))
				return false;
		}
		else
		{
			auto val = std::make_shared<list_desc>(vl::MakePtr(vl::Null()));
			if (!add_member(val, name))
				return false;
		}
		return true;
	}

	bool cpp_writer::EndVisitList(const ListVar& var, const char* name)
	{
		pop_current_container();
		return true;
	}

	void cpp_writer::print_fwd(const std::string& class_name)
	{
		INIT_PRINT(m_print_context.fwd_f, 1);
		PRINT_LINE("class " << class_name << ";");
	}

	void cpp_writer::begin_writing_fwd()
	{
		const std::string fpath = m_params.cppgen_params.out_dir_path + "/fwd.h";
		m_print_context.fwd_f.open(fpath);
		INIT_PRINT(m_print_context.fwd_f, 0);
		PRINT_LINE("#pragma once");
		PRINT_LINE_BREAK;
		PRINT_LINE("namespace " << m_params.cpp_namespace);
		PRINT_SCOPE_BEGIN;
	}

	void cpp_writer::end_writing_fwd()
	{
		m_print_context.fwd_f << "}";
		m_print_context.fwd_f.close();
	}
	// End of cpp_writer
	
	// Begin of class_desc
	bool class_desc::is_empty()
	{
		return m_class_name.empty() && m_fields.empty();
	}

	const var_desc_ptr& class_desc::add_field(const std::string& name, const var_desc_ptr& field)
	{
		auto& res = m_fields.add(name, field);
		field->set_parent(shared());
		return res;
	}
	
	const var_desc_ptr* class_desc::find_field(const std::string& field_name) const
	{
		static var_desc_ptr null_var_desc(nullptr);
		auto it = m_fields.find(field_name);
		if (it == m_fields.end())
			return nullptr;
		return &(*it).second;
	}

	void class_desc::set(const std::string& field_name, const var_desc_ptr& field)
	{
		m_fields.set(field_name, field);
	}

	int class_desc::print(class_print_context& ctx) const
	{
		std::ofstream local_file;
		std::ofstream& f = ctx.file ? *ctx.file : local_file;
		
		bool has_own_file = is_type() || is_root();
		if (!ctx.file && !has_own_file)
		{
			LOCAL_ERROR("Can't provide an output file for class '" << m_class_name << "' as it is not a type and there is no file provided in the print context");
			return -1;
		}

		// Open the file
		if (has_own_file)
		{
			auto& wctx = ctx.writer.print_context();
			if (wctx.printed_classes.find(get_name()) != wctx.printed_classes.end())
				return 0; // Already printed or printing
			else
			{
				wctx.printed_classes.emplace(get_name());
				ctx.writer.print_fwd(get_name());
			}
			std::string fpath = ctx.writer.get_params().cppgen_params.out_dir_path + "/" + m_class_name + ".h";
			f.open(fpath);
			if (!f.is_open())
			{
				LOCAL_ERROR("Can't write to a file '" << fpath << "'");
				return -2;
			}
		}

		print_data pdata;
		int collect_data_result = collect_data(pdata, ctx);
		if (collect_data_result < 0)
			return collect_data_result;
		
		auto& proto_id = pdata.proto_id;

		INIT_PRINT(f, ctx.indent_level);

		// === Start writing ===
		// Predefines section
		auto& subclasses = pdata.subclasses;
		auto& classes = pdata.classes;
		auto& primitives = pdata.primitives;
		auto& includes = pdata.includes;
		auto& types = pdata.types;
		
		if (has_own_file)
		{
			PRINT_LINE("#pragma once");
			PRINT_LINE_BREAK;
			// Include VL
			PRINT_LINE("#include <vl_fwd.h>");

			if (!includes.empty())
			{
				PRINT_LINE_BREAK;
				for (auto& i : includes)
					PRINT_LINE("#include \"" << i << ".h\"");
			}
			
			PRINT_LINE_BREAK;

			// Write namespace
			PRINT_LINE("namespace " << ctx.writer.get_params().cpp_namespace);
			PRINT_LINE("{");
			PRINT_INDENT_INCREASE;
		}
		// Print class declaration header
		BEGIN_PRINT_LINE;
		PRINT(f, "class " << m_class_name);
		if (!proto_id.empty())
			PRINT(f, " : public " << proto_id);
		END_PRINT_LINE;

		PRINT_LINE("{");
		PRINT_LINE("public:");
		PRINT_INDENT_INCREASE;
		// Declare constructor
		PRINT_LINE("// Initializers");
		PRINT_LINE(CLASS_DEF_CONSTRUCTOR_DECL);
		PRINT_LINE(CLASS_CONSTRUCTOR_SIGNATURE << ";");
		PRINT_LINE(CLASS_CONSTRUCTOR_2_SIGNATURE << ";");
		
		PRINT_LINE("// Data validation checker through the bool operator");
		
		// Declare operator bool()
		if (pdata.proto_id.empty())
			PRINT_LINE("operator bool() const;");
		
		
		if (pdata.proto_id.empty())
		{
			// Declare get_data(string) method
			PRINT_LINE("const vl::Var& get_data(const std::string& field_name) const;");
			// Declare has_data(string) method
			PRINT_LINE("bool has_data(const std::string& field_name) const;");
			// Declare has_data_own(string) method
			PRINT_LINE("bool has_data_own(const std::string& field_name) const;");
			// Declare get_data() method
			PRINT_LINE("// Data getter for internal use");
			PRINT_LINE("const vl::VarPtr& get_data() const {");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("return m_data;");
			PRINT_SCOPE_END;
			PRINT_LINE_BREAK;
		}
		if (!m_fields.empty())
		{
			// Print nested classes
			if (!subclasses.empty())
			{
				PRINT_LINE("// Nested object fields classes")
				PRINT_INDENT_DECREASE;
				class_print_context subctx = { ctx.writer, *this , ctx.file, ctx.indent_level };
				subctx.file = &f;
				subctx.indent_level = INDENT_LEVEL + 1;
				subctx.parent_context = &ctx;
				std::for_each(subclasses.begin(), subclasses.end(), [&](auto f) {
					f.second->as_class()->print(subctx);
				});
				PRINT_INDENT_INCREASE;
			}
			
			PRINT_LINE("// Field access");
			
			foreach_field(*this, [&](const std::string& n, const var_desc_ptr& f, int recursion_level) {
				PRINT_LINE("// \"" << n << "\" field");
				if (auto c = f->as_class())
				{
					PRINT_LINE(CLASS_GETTER_DECLARATION("", "get_" << n, c, ""));
					PRINT_LINE(CLASS_GETTER_DECLARATION("const ", "get_" << n, c, " const"));
				}
				else if (auto l = f->as_list())
				{
					PRINT_LINE(MEHTOD_DECLARATION("vl::List&", n, "", ""));
					PRINT_LINE(MEHTOD_DECLARATION("const vl::List&", "get_" + n, "", " const"));
				}
				else if (auto p = f->as_primitive_type())
				{
					if (p->is_bool()) {
						PRINT_LINE(MEHTOD_DECLARATION("bool", n, "", " const"));
						if (ctx.writer.get_params().cppgen_params.generate_setters)
							PRINT_LINE(MEHTOD_DECLARATION("void", "set_" << n, "bool value", ""));
					} else if (p->is_number()) {
						PRINT_LINE(MEHTOD_DECLARATION("float", n, "", " const"));
						if (ctx.writer.get_params().cppgen_params.generate_setters)
							PRINT_LINE(MEHTOD_DECLARATION("void", "set_" << n, "float value", ""));
					} else if (p->is_string()) {
						PRINT_LINE(MEHTOD_DECLARATION("const std::string&", n, "", " const"));
						if (ctx.writer.get_params().cppgen_params.generate_setters)
							PRINT_LINE(MEHTOD_DECLARATION("void", "set_" << n, "const std::string& value", ""));
					} else if (p->is_pointer()) {
						PRINT_LINE(MEHTOD_DECLARATION("void*", n, "", " const"));
						if (ctx.writer.get_params().cppgen_params.generate_setters)
							PRINT_LINE(MEHTOD_DECLARATION("void", "set_" << n, "const void* value", ""));
					}
				}
				PRINT_LINE_BREAK;
			}, -1);
			
			PRINT_INDENT_DECREASE;
		}
		else
			PRINT_INDENT_DECREASE;
		// Print members
		
		if (pdata.proto_id.empty())
		{
			// Declare _data_() method
			PRINT_LINE("protected:");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("// Data getter for internal use");
			PRINT_LINE("const vl::VarPtr& _data_() const {");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("return m_data;");
			PRINT_SCOPE_END;
			PRINT_INDENT_DECREASE;
			PRINT_LINE_BREAK;
		}
		if (!classes.empty() || !has_proto())
		{
			PRINT_LINE("private:");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("// Data members");
			std::for_each(classes.begin(), classes.end(), [&](auto f) {
				auto& fn = f.first;
				auto c = f.second->as_class();
				PRINT_LINE("class " << c->get_name() << " m_" << fn << ";");
			});
			if (!has_proto())
				PRINT_LINE("vl::VarPtr m_data;");
			PRINT_INDENT_DECREASE;
		}
		// Close class declaration
		PRINT_LINE("};");
		if (has_own_file)
		{
			// Close namespace
			PRINT_SCOPE_END;
			
			// Print .cpp
			std::ofstream fcpp;
			std::string fcpp_path = ctx.writer.get_params().cppgen_params.out_dir_path + "/" + m_class_name + ".cpp";
			fcpp.open(fcpp_path);
			if (!fcpp.is_open())
			{
				LOCAL_ERROR("Can't write to a file '" << fcpp_path << "'");
				return -2;
			}
			
			INIT_PRINT(fcpp, 0);
			// Includes
			// Include VL
			PRINT_LINE("#include <vl.h>");
			PRINT_LINE("#include \"" << get_name() << ".h\"");
			PRINT_LINE_BREAK;
			PRINT_LINE("namespace " << ctx.writer.get_params().cpp_namespace);
			PRINT_SCOPE_BEGIN;
			class_print_context defs_ctx = { ctx.writer, *this, &fcpp, ctx.indent_level };
			defs_ctx.indent_level = INDENT_LEVEL;
			print_definitions_context print_defs_ctx = {defs_ctx, &pdata};
			print_definitions(print_defs_ctx);
			PRINT_SCOPE_END;
		}
		return 0;
	}

	void class_desc::print_definitions(print_definitions_context& pctx) const
	{
		auto& ctx = pctx.ctx;
		print_data pdata;
		if (!pctx.pdata)
			collect_data(pdata, ctx);
		auto& classes = pctx.pdata ? pctx.pdata->classes : pdata.classes;
		auto& subclasses = pctx.pdata ? pctx.pdata->subclasses : pdata.subclasses;
		auto& types = pctx.pdata ? pctx.pdata->types : pdata.types;
		auto& primitives = pctx.pdata ? pctx.pdata->primitives : pdata.primitives;
		// Define constructors
		INIT_PRINT(*pctx.ctx.file, pctx.ctx.indent_level);
		
		auto print_constructor_body = [&]() {
			PRINT_SCOPE_BEGIN;
			
			// Check m_data
			if (!classes.empty())
			{
				PRINT_DATA_OBJECT_WITH_CHECKS("")
				// Print class members initialisers
				std::for_each(classes.begin(), classes.end(), [&](auto f) {
					auto& fn = f.first;
					auto& c = f.second;
					auto& cn = c->as_class()->get_name();
					// TODO: add checks for the field existance
					// TODO: add logs when the data is not found or has a wrong type
					PRINT_LINE("m_" << fn << " = "
							   << "{vl::MakePtr(data_obj.Get(\"" << cn << "\"))};");
				});
			}
			PRINT_SCOPE_END;
		};
		
		PRINT_LINE("// Initializers");
		
		PRINT_LINE_BREAK;
		
		// Define constructor from vl::VarPtr
		PRINT_LINE(CLASS_CPP_SCOPE << CLASS_CONSTRUCTOR_SIGNATURE);
		PRINT_LINE("\t" << ": " << CLASS_DATA_INITIALIZER);
		print_constructor_body();
		PRINT_LINE_BREAK;
		
		// Define constructor from vl::Var
		PRINT_LINE(CLASS_CPP_SCOPE << CLASS_CONSTRUCTOR_2_SIGNATURE);
		PRINT_LINE("\t" << ": " << CLASS_DATA_INITIALIZER_2);
		print_constructor_body();
		PRINT_LINE_BREAK;
		
		// Define operator bool()
		if (pdata.proto_id.empty())
		{
			PRINT_LINE(CLASS_CPP_SCOPE << "operator bool() const");
			PRINT_SCOPE_BEGIN;
			// Data availability check
			PRINT_LINE("if (!m_data)");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("return false;");
			PRINT_INDENT_DECREASE;
			// Data consistency check
			PRINT_LINE("if (!m_data->IsObject())");
			PRINT_INDENT_INCREASE;
			PRINT_LINE("return false;");
			PRINT_INDENT_DECREASE;
			PRINT_LINE("return true;");
			PRINT_SCOPE_END;
			PRINT_LINE_BREAK;
		}
		
		// Define get_data(string) method
		if (pdata.proto_id.empty())
		{
			PRINT_LINE("const vl::Var& " << CLASS_CPP_SCOPE << "get_data(const std::string& field_name) const");
			PRINT_SCOPE_BEGIN;
			PRINT_DATA_RETURN_WITH_CHECKS_UNQUOTED("field_name", "Get", " vl::emptyVar");
			PRINT_SCOPE_END;
			PRINT_LINE_BREAK;
		}

		// Define has_data(string) method
		if (pdata.proto_id.empty())
		{
			PRINT_LINE("bool " << CLASS_CPP_SCOPE << "has_data(const std::string& field_name) const");
			PRINT_SCOPE_BEGIN;
			PRINT_DATA_RETURN_WITH_CHECKS_UNQUOTED("field_name", "Has", " vl::emptyVar");
			PRINT_SCOPE_END;
			PRINT_LINE_BREAK;
		}

		// Define has_data_own(string) method
		if (pdata.proto_id.empty())
		{
			PRINT_LINE("bool " << CLASS_CPP_SCOPE << "has_data_own(const std::string& field_name) const");
			PRINT_SCOPE_BEGIN;
			PRINT_DATA_RETURN_WITH_CHECKS_UNQUOTED("field_name", "HasOwn", " vl::emptyVar");
			PRINT_SCOPE_END;
			PRINT_LINE_BREAK;
		}
		// Getters and setters definitions
		if (!m_fields.empty())
		{
			PRINT_LINE("// Field access");
			foreach_field(*this, [&](const std::string& fn, const var_desc_ptr& v, int recursion_level) {
				PRINT_LINE("// \"" << fn << "\" field");
				if (auto c = v->as_class())
				{
					// Define non-const class getter
					PRINT_LINE((c && !c->as_class()->is_type() ? CLASS_CPP_SCOPE : "") << CLASS_GETTER_DEFINITION("", "get_" << fn, c, ""));
					PRINT_SCOPE_BEGIN;
					PRINT_LINE("return m_" << fn << ";");
					PRINT_SCOPE_END;
					PRINT_LINE_BREAK;
					
					// Define const class getter
					PRINT_LINE(CLASS_GETTER_DEFINITION("const " << (c && !c->as_class()->is_type() ? CLASS_CPP_SCOPE : ""), "get_" << fn, c, " const"));
					PRINT_SCOPE_BEGIN;
					PRINT_LINE("return m_" << fn << ";");
					PRINT_SCOPE_END;
				}
				else
				{
					if (auto l = v->as_list())
					{
						// Define non-const list getter
						PRINT_LINE(METHOD_DEFINITION("vl::List&", fn, "", ""));
						PRINT_SCOPE_BEGIN;
						PRINT_LINE("return const_cast<vl::List&>(get_" << fn << "()); ");
						PRINT_SCOPE_END;
						PRINT_LINE_BREAK;
						
						// Define const list getter
						PRINT_LINE(METHOD_DEFINITION("const vl::List&", "get_" + fn, "", " const"));
						PRINT_SCOPE_BEGIN;
						PRINT_LINE(STATIC_VARIABLE_DECLARATION("vl::List", "empty_val", "vl::emptyList"));
						PRINT_DATA_TYPE_RETURN_WITH_CHECKS("List", fn, "");
						PRINT_SCOPE_END;
					}
					else if (auto p = v->as_primitive_type())
					{
						if (p->is_bool()) {
							// Define bool getter
							PRINT_LINE(METHOD_DEFINITION("bool", fn, "", " const"));
							PRINT_SCOPE_BEGIN;
							PRINT_LINE(VARIABLE_DECLARATION("bool", "empty_val", "false"));
							PRINT_DATA_TYPE_RETURN_WITH_CHECKS("Bool", fn, ".Val()");
							PRINT_SCOPE_END;
							PRINT_LINE_BREAK;
							// Define bool setter
							if (ctx.writer.get_params().cppgen_params.generate_setters)
								PRINT_SETTER_DEFINITION(fn, "bool");
						} else if (p->is_number()) {
							// Define number getter
							PRINT_LINE(METHOD_DEFINITION("float", fn, "", " const"));
							PRINT_SCOPE_BEGIN;
							PRINT_LINE(VARIABLE_DECLARATION("float", "empty_val", "0.0f"));
							PRINT_DATA_TYPE_RETURN_WITH_CHECKS("Number", fn, ".Val()");
							PRINT_SCOPE_END;
							PRINT_LINE_BREAK;
							// Define number setter
							if (ctx.writer.get_params().cppgen_params.generate_setters)
								PRINT_SETTER_DEFINITION(fn, "float");
						} else if (p->is_string()) {
							// Define string getter
							PRINT_LINE(METHOD_DEFINITION("const std::string&", fn, "", " const"));
							PRINT_SCOPE_BEGIN;
							PRINT_LINE(STATIC_VARIABLE_DECLARATION("std::string", "empty_val", "\"\""));
							PRINT_DATA_TYPE_RETURN_WITH_CHECKS("String", fn, ".Val()");
							PRINT_SCOPE_END;
							PRINT_LINE_BREAK;
							// Define string setter
							if (ctx.writer.get_params().cppgen_params.generate_setters)
								PRINT_SETTER_DEFINITION(fn, "const std::string&");
						} else if (p->is_pointer()) {
							// Define pointer getter
							PRINT_LINE(METHOD_DEFINITION("void*", fn, "", " const"));
							PRINT_SCOPE_BEGIN;
							PRINT_LINE(VARIABLE_DECLARATION("void*", "empty_val", "nullptr"));
							PRINT_DATA_TYPE_RETURN_WITH_CHECKS("Pointer", fn, ".Val()");
							PRINT_SCOPE_END;
							PRINT_LINE_BREAK;
							// Define pointer setter
							if (ctx.writer.get_params().cppgen_params.generate_setters)
								PRINT_SETTER_DEFINITION(fn, "const void*");
						}
					}
				}
				PRINT_LINE_BREAK
			}, -1);
		}
		
		// Subclasses defenitions
		if (!subclasses.empty())
		{
			PRINT_LINE("// Subclasses definitions begin")
			std::for_each(subclasses.begin(), subclasses.end(), [&](auto f) {
				auto& n = f.first;
				auto& c = f.second;
				class_print_context subctx = { ctx.writer, *c->as_class() , ctx.file, INDENT_LEVEL, &ctx};
				print_definitions_context print_defs_subctx = { subctx };
				c->as_class()->print_definitions(print_defs_subctx);
			});
		}
	}

	int class_desc::collect_data(print_data& pdata, class_print_context& ctx) const
	{
		auto& data = get_data();
		if (!data)
		{
			LOCAL_ERROR("Attemption to print an unitialized class '" << m_class_name << "'");
			return -3;
		}

		if (!data->IsObject())
		{
			LOCAL_ERROR("Attemption to print a wrongly itialized class '" << m_class_name << "'. Data should be of 'Object' type");
			return -4;
		}
		
		auto& data_obj = data->AsObject();
		
		// Get proto id
		if (auto proto = data_obj.GetPrototype())
			pdata.proto_id = process_class_name(ctx.writer.get_type_resolver().GetTypeId(proto));
		
		if (!pdata.proto_id.empty())
			if (!is_path(pdata.proto_id))
				pdata.includes.emplace(pdata.proto_id);

		foreach_field(*this, [&](const std::string& __n, const var_desc_ptr& __f, int) {
			collect_data_recursion(ctx, pdata, __f, __n, 0);
		}, -1);
		
		return 0;
	}

	void class_desc::collect_data_recursion(
		class_print_context& ctx
		, print_data& pdata
		, const vl::var_desc_ptr& f
		, const std::string& n
		, int recursion_level
	) const
	{
		// Works only on the first recursion level
		if (recursion_level == 0)
		{
			if (auto c = f->as_class())
			{
				pdata.classes.add(n, c);
				if (c->is_type())
					pdata.types.emplace(c->get_name());
				else
					pdata.subclasses.add(n, f);
			}
			else if (auto p = f->as_primitive_type())
			{
				pdata.primitives.add(n, f);
			}
		}
		// Works recursively
		if (auto c = f->as_class())
		{
			if (c->is_type())
			{
				pdata.includes.emplace(c->get_name());
				// Stop recursion after a type is found
			}
			else
			{
				if (c->has_proto())
				{
					auto include = c->get_proto_class_name(ctx.writer.get_type_resolver());
					if (!is_path(include))
						if (pdata.includes.find(include) == pdata.includes.end())
							pdata.includes.emplace(include);
				}
				// Go down recursively for every nested class
				foreach_field(*f, [&](const std::string& __n, const var_desc_ptr& __f, int) {
					collect_data_recursion(ctx, pdata, __f, __n, recursion_level + 1);
				}, -1);
			}
		}
	}

	bool class_desc::has_proto() const
	{
		if (!m_data)
			return false;
		if (!m_data->IsObject())
			return false;
		return m_data->AsObject().GetPrototype();
	}

	std::string class_desc::get_proto_class_name(const TypeResolver& type_resolver) const
	{
		if (!m_data)
			return "";
		if (!m_data->IsObject())
			return "";
		if (auto proto = m_data->AsObject().GetPrototype())
			return process_class_name(type_resolver.GetTypeId(proto));
		return "";
	}
	// End of class_desc

	// Begin of list_desc
	bool list_desc::is_empty()
	{
		return m_fields.empty();
	}

	const var_desc_ptr& list_desc::add_field(const var_desc_ptr& field)
	{
		m_fields.push_back(field);
		return m_fields.back();
	}
	// End of list_desc
}


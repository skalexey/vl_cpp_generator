#include <set>
#include <iostream>
#include <cstring>
#include <cassert>
#include <vl.h>
#include <TypeResolver.h>
#include "cpp_writer.h"
#include "Log.h"
#ifdef LOG_ON
	LOG_TITLE("cpp_writer")
	LOG_STREAM([]() -> std::ostream& { return std::cout; })
	SET_LOG_DEBUG(true)
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

	#define PRINT_INDENT_DECREASE \
		pr_ind_level--

	#define PRINT_INDENT_INCREASE \
		pr_ind_level++

	#define INDENT_LEVEL pr_ind_level
}

namespace vl
{
	cpp_writer::cpp_writer(const TypeResolver& type_resolver, const cppw_params& params)
		: m_params(params)
		, m_type_resolver(type_resolver)
		, m_root(std::make_shared<class_desc>())
	{}

	void cpp_writer::add_member(const var_desc_ptr& val, const char* name)
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
					return;
				}
			}
			is_root = true;
			push_current_container(m_root);
		}
		auto current_val = get_current_container();
		if (current_val->is_class())
		{
			auto current_val_obj = current_val->as_class();
			if ((val->is_class() && !is_root) || !val->is_class())
			{
				assert(name); // Any property if an object should have it's name
				auto name_str = (name ? name : "");
				var_desc_ptr new_val(nullptr);
				if ((new_val = current_val_obj->find_field(name_str)))
					new_val = val;
				else
					new_val = current_val_obj->add_field(name_str, val);
				if (new_val->is_class() || new_val->is_list())
					push_current_container(new_val);
			}
		}
		else if (current_val->is_list())
		{
			auto current_val_list = current_val->as_list();
			auto& new_val = current_val_list->add_field(val);
			if (new_val->is_class() || new_val->is_list())
				push_current_container(new_val);
		}
		else
			throw "cpp_writer: unsupported container type for adding elements";
	}

	bool cpp_writer::add_proto(const ObjectVar& value)
	{
		auto current_val = get_current_container();
		if (m_params.use_proto_refs)
		{
			auto type_id = m_type_resolver.GetTypeId(value);
			auto val = std::make_shared<class_desc>(vl::MakePtr(value), type_id, true);
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
		add_member(val, name);
		return true;
	}

	bool cpp_writer::VisitBool(const BoolVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		add_member(val, name);
		return true;
	}

	bool cpp_writer::VisitNumber(const NumberVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		add_member(val, name);
		return true;
	}

	bool cpp_writer::VisitString(const StringVar& var, const char* name)
	{
		auto val = std::make_shared<primitive_type_desc>(vl::MakePtr(var));
		add_member(val, name);
		return true;
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
				bool is_root = !get_current_container() && m_root->is_empty();
				bool is_type = m_type_resolver.IsType(var);
				std::string class_name(
					is_root ? "root" 
					: (is_type ? m_type_resolver.GetTypeId(var) : name)
				);
				auto val = std::make_shared<class_desc>(vl::MakePtr(var), class_name, is_type);
				add_member(val, name);
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
		//m_level--;
		//PRINT_CPP("};");
		auto container = get_current_container();
		assert(container && "A current container should always exist when ending visiting an object");
		if (!container)
		{
			LOCAL_ERROR("Empty container stack when ending visiting object '" << name << "'");
			return false;
		}
		assert(container->is_class() && "Ending visiting an object should work with a class container");
		pop_current_container();
		print_context ctx = { *this };
		if (m_stack.empty()) // is root
			container->print(ctx);
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
			add_member(val, name);
		}
		else
		{
			auto val = std::make_shared<list_desc>(vl::MakePtr(vl::Null()));
			add_member(val, name);
		}
		return true;
	}

	bool cpp_writer::EndVisitList(const ListVar& var, const char* name)
	{
		pop_current_container();
		return true;
	}

	bool class_desc::is_empty()
	{
		return m_class_name.empty() && m_fields.empty();
	}

	const var_desc_ptr& class_desc::add_field(const std::string& name, const var_desc_ptr& field)
	{
		m_name_to_index[name] = m_fields.size();
		m_fields.push_back(field);
		field->set_parent(shared_from_this());
		return m_fields.back();
	}
	
	const var_desc_ptr& class_desc::find_field(const std::string& field_name) const
	{
		static var_desc_ptr null_var_desc(nullptr);
		auto it = m_name_to_index.find(field_name);
		if (it == m_name_to_index.end())
			return null_var_desc;
		return m_fields[it->second];
	}

	// recursion: < 0 - no recursion, otherwise it is a recursion level incrementing every iteration
	void foreach_field (
		const vl::var_desc& v
		, const std::function<void(const var_desc_ptr& f, int)>& pred
		, int recursion = 0
	)
	{
		if (v.is_class())
			for (auto& f : v.as_class()->get_fields())
			{
				foreach_field(*f, pred, recursion < 0 ? recursion : recursion + 1);
				pred(f, recursion);
			}
		else if (v.is_list())
			for (auto& f : v.as_list()->get_fields())
			{
				foreach_field(*f, pred, recursion < 0 ? recursion : recursion + 1);
				pred(f, recursion);
			}
	}

	int class_desc::print(const print_context& ctx) const
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
			std::string fpath = ctx.writer.get_params().out_dir_path + "/" + m_class_name + ".h";
			f.open(fpath);
			if (!f.is_open())
			{
				LOCAL_ERROR("Can't write to a file '" << fpath << "'");
				return -2;
			}
		}

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
		
		INIT_PRINT(f, ctx.indent_level);
		auto& data_obj = data->AsObject();

		// Get proto id
		std::string proto_id;
		if (auto proto = data_obj.GetPrototype())
			proto_id = ctx.writer.get_type_resolver().GetTypeId(proto);

		// === Start writing ===
		// Predefines section
		std::vector<class_desc_ptr> subclasses;
		std::set<std::string> includes;
		if (has_own_file)
		{
			if (!proto_id.empty())
			{
				PRINT_LINE("#include \"" << proto_id << ".h\"");
				includes.emplace(proto_id);
			}

			foreach_field(*this, [&](const var_desc_ptr& f, int recursion_level) {
				if (auto c = f->as_class())
				{
					if (c->is_type())
					{
						auto include = c->get_name();
						if (includes.find(include) == includes.end())
						{
							PRINT_LINE("#include \"" << include << ".h\"");
						}
						includes.emplace(include);
					}
					else
					{
						if (c->has_proto())
						{
							auto include = c->get_proto_id(ctx.writer.get_type_resolver());
							if (includes.find(include) == includes.end())
							{
								PRINT_LINE(
									"#include \""
									<< include
									<< ".h\""
								);
								includes.emplace(include);
							}
						}
						if (recursion_level == 0)
							subclasses.push_back(c);
					}
				}
			}, 0);
		}

		PRINT_LINE("");

		// Write namespace
		if (has_own_file)
		{
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

		// Print nested classes
		print_context subctx = ctx;
		subctx.file = &f;
		subctx.indent_level = INDENT_LEVEL + 1;
		for (auto& c : subclasses)
			c->print(subctx);
		// Close class declaration
		PRINT_LINE("};");
		// Close namespace
		if (has_own_file)
		{
			PRINT_INDENT_DECREASE;
			PRINT_LINE("}");
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

	std::string class_desc::get_proto_id(const TypeResolver& type_resolver) const
	{
		if (!m_data)
			return "";
		if (!m_data->IsObject())
			return "";
		if (auto proto = m_data->AsObject().GetPrototype())
			return type_resolver.GetTypeId(proto);
		return "";
	}

	bool list_desc::is_empty()
	{
		return m_fields.empty();
	}

	int vl::list_desc::print(const print_context& ctx) const
	{
		return 0;
	}

	const var_desc_ptr& list_desc::add_field(const var_desc_ptr& field)
	{
		m_fields.push_back(field);
		return m_fields.back();
	}
}


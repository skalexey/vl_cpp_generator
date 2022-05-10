#include <iterator>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <vl_visitor.h>
#include <utils/ordered_map.h>
#include "cppgen_fwd.h"

namespace vl
{
	class cpp_writer;

	// var_desc
	class var_desc;
	using var_desc_ptr = std::shared_ptr<var_desc>;
	class class_desc;
	using class_desc_ptr = std::shared_ptr<class_desc>;
	using class_desc_ptr_const = std::shared_ptr<const class_desc>;
	class list_desc;
	using list_desc_ptr = std::shared_ptr<list_desc>;
	using list_desc_ptr_const = std::shared_ptr<const list_desc>;
	class primitive_type_desc;

	class var_desc : public std::enable_shared_from_this<var_desc>
	{
		friend class class_desc;
	public:
		var_desc() = default;
		var_desc(const vl::VarPtr& data)
			: m_data(data)
		{}
		vl::VarPtr& get_data() { return m_data; }
		inline virtual bool is_class() const { return false; }
		inline virtual bool is_list() const { return false; }
		inline virtual bool is_primitive() const { return false; }
		inline virtual class_desc_ptr as_class() { return nullptr; }
		inline virtual list_desc_ptr as_list() { return nullptr; }
		inline virtual list_desc_ptr_const as_list() const { return nullptr; }
		inline virtual primitive_type_desc* as_primitive_type() { return nullptr; }
		inline virtual class_desc_ptr_const as_class() const { return nullptr; }
		inline virtual const primitive_type_desc* as_primitive_type() const { return nullptr; }
		inline const var_desc_ptr& parent() const { return m_parent; }

	protected:
		vl::VarPtr& data() { return m_data; }
		const vl::VarPtr& get_data() const { return m_data; }
		inline void set_parent(const var_desc_ptr& parent) { m_parent = parent; }

	private:
		vl::VarPtr m_data;
		var_desc_ptr m_parent = nullptr;
	};
	
	// primitive_type_desc
	class primitive_type_desc : public var_desc
	{
		using base = var_desc;

	public:
		primitive_type_desc() = default;
		primitive_type_desc(const vl::VarPtr& data)
			: base(data)
		{}
		inline bool is_primitive() const override { return true; }
		primitive_type_desc* as_primitive_type() override { return this; }
		const primitive_type_desc* as_primitive_type() const override { return this; }
		inline bool is_bool() const { return get_data() && get_data()->IsBool(); };
		inline bool is_number() const { return get_data() && get_data()->IsNumber(); };
		inline bool is_string() const { return get_data() && get_data()->IsString(); };
	};
	using primitive_type_ptr = std::shared_ptr<primitive_type_desc>;

	// class_desc
	
	
	class class_desc : public var_desc
	{
		friend class cpp_writer;
		using base = var_desc;

	public:
		using fields_list = std::vector<var_desc_ptr>;
		using fields_map_view = utils::ordered_map_view<std::string, var_desc_ptr>;
		using fields_map = utils::ordered_map<std::string, var_desc_ptr>;
		using field_name_list = std::vector<std::string>;
		
	protected:
		struct print_data
		{
			std::string proto_id;
			fields_map subclasses;
			fields_map primitives;
			fields_map classes;
			std::set<std::string> includes;
			std::set<std::string> types;
		};
		struct class_print_context
		{
			cpp_writer& writer;
			std::ofstream* file = nullptr;
			int indent_level = 0;
			std::string parent_class;
		};
	public:
		class_desc() = default;
		class_desc(
			const vl::VarPtr& data
			, const std::string& class_name
			, bool is_type
		)
			: base(data)
			, m_class_name(class_name)
			, m_is_type(is_type)
		{}
		bool is_empty();
		inline bool is_class() const override { return true; }
		class_desc_ptr as_class() override { return shared(); }
		class_desc_ptr_const as_class() const override { return shared(); }
		const var_desc_ptr& add_field(const std::string& name, const var_desc_ptr& field);
		const var_desc_ptr* find_field(const std::string& field) const;
		void set(const std::string& field_name, const var_desc_ptr& field);
		inline bool is_type() const { return m_is_type; }
		inline bool is_root() const { return parent() == nullptr; }
		inline const std::string& get_name() const { return m_class_name; }
		int print(class_print_context& ctx) const;
		int print_fwd(class_print_context& ctx) const;
		struct print_definitions_context
		{
			class_print_context& ctx;
			const print_data* pdata = nullptr;
		};
		void print_definitions(print_definitions_context& ctx) const;
		int collect_data(print_data& data, class_print_context& ctx) const;
		inline class_desc_ptr shared() {
			return std::dynamic_pointer_cast<class_desc>(shared_from_this());
		}
		inline class_desc_ptr_const shared() const {
			return std::dynamic_pointer_cast<const class_desc>(shared_from_this());
		}
		
		inline fields_map_view get_fields_map() const {
			return fields_map_view(m_fields);
		}
		bool has_proto() const;
		std::string get_proto_id(const TypeResolver& type_resolver) const;

	private:
		std::string m_class_name;
		utils::ordered_map<std::string, var_desc_ptr> m_fields;
		bool m_is_type = false;
		
	protected:
		void collect_data_recursion(
			class_print_context& ctx
			, print_data& pdata
			, const vl::var_desc_ptr& f
			, const std::string& n
			, int recursion_level
		) const;
	};

	// list_desc
	class list_desc : public var_desc
	{
		using base = var_desc;

	public:
		using fields_list = std::vector<var_desc_ptr>;

		list_desc() = default;
		list_desc(const vl::VarPtr& data)
			: base(data)
		{}
		bool is_empty();
		inline bool is_list() const override { return true; }
		list_desc_ptr as_list() override { return shared(); }
		list_desc_ptr_const as_list() const override { return shared(); };
		inline list_desc_ptr shared() {
			return std::dynamic_pointer_cast<list_desc>(shared_from_this());
		}
		inline list_desc_ptr_const shared() const {
			return std::dynamic_pointer_cast<const list_desc>(shared_from_this());
		}
		const var_desc_ptr& add_field(const var_desc_ptr& field);
		inline const fields_list& get_fields() const { return m_fields; }

	private:
		fields_list m_fields;
	};
	using class_desc_ptr = std::shared_ptr<class_desc>;

	// cpp_writer
	class cpp_writer : public Visitor
	{
		// These classes use incapsulated print context of the writer
		friend class var_desc;
		friend class class_desc;
		friend class list_desc;
		friend class primitive_type_desc;
		
	protected:
		struct global_print_context
		{
			std::ofstream fwd_f;
			std::set<std::string> printed_classes;
		};
		
	public:
		cpp_writer(const TypeResolver& type_resolver, const cppw_params& params = cppw_params());
		// Visitor interface
		bool VisitNull(const NullVar& var, const char* name) override;
		bool VisitBool(const BoolVar& var, const char* name) override;
		bool VisitNumber(const NumberVar& var, const char* name) override;
		bool VisitString(const StringVar& var, const char* name) override;
		bool VisitObject(const ObjectVar& var, const char* name) override;
		bool EndVisitObject(const ObjectVar& var, const char* name) override;
		bool VisitList(const ListVar& var, const char* name) override;
		bool EndVisitList(const ListVar& var, const char* name) override;

	public:
		const cppw_params& get_params() const { return m_params; }
		const TypeResolver& get_type_resolver() const { return m_type_resolver; }

	protected:
		bool add_proto(const ObjectVar& value);
		bool add_member(const var_desc_ptr& val, const char* name);
		var_desc_ptr get_current_container();
		void push_current_container(const var_desc_ptr& val);
		bool pop_current_container();
		inline global_print_context& print_context() {
			return m_print_context;
		}
	
	protected:
		void begin_writing_fwd();
		void end_writing_fwd();
		void print_fwd(const std::string& class_name);
		
	private:
		std::vector<var_desc_ptr> m_stack;
		class_desc_ptr m_root;
		cppw_params m_params;
		TypeResolver m_type_resolver;
		global_print_context m_print_context;
	};
}

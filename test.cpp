#include <iostream>
#include <vl/TypeResolver.h>
#include "JSONConverter.h"

void WriteTest()
{
	std::cout << "Write Test\n";
	auto pear = vl::Object();
	pear.Set("isFruit", true);
	pear.Set("color", "Yellow");
	pear.Set("radius", 0.3f);
	pear.Set("branchCount", 1);
	std::cout << "Val: " << pear.Get("color").as<vl::String>().Val() << "\n";
	auto bush = vl::Object();
	bush.Set("leafColor", "Green");
	bush.Set("isTree", true);
	bush.Set("leafPerBranch", 6);
	bush.Set("branches", vl::List());
	bush.Set("x", 0);
	bush.Set("y", 0);
	auto branch = vl::Object();
	branch.Set("leafCount", 10);
	branch.Set("fruit", vl::Object());
	branch.Set("branches", vl::List());
	auto branch1 = branch.Copy();
	branch1.Set("leafCount", 9);
	auto branch2 = branch.Copy();
	branch2.Set("leafCount", 3);
	branch2.Set("fruit", pear);
	branch1.Get("branches").as<vl::List>().Add(branch2);
	bush.Get("branches").as<vl::List>().Add(branch1);
	{
		auto var_ptr = vl::VarPtr::Make<vl::Object>();
		auto& obj = var_ptr.as<vl::Object>();
		obj.Set("test", 1);
		auto ptr = obj.Ptr();
		assert(ptr.as<vl::Object>()["test"].as<vl::Number>().Val() == 1);
	}
	{
		auto ptr = bush.Ptr();
		ptr.as<vl::Object>().Set("leafPerBranch", 2);
		assert(bush["leafPerBranch"].as<vl::Number>().Val() == 2);
		bush["leafPerBranch"] = 6;
		assert(ptr.as<vl::Object>()["leafPerBranch"].as<vl::Number>().Val() == 6);
	}
	{
		auto& original_list = bush["branches"].as<vl::List>();
		auto list_copy = original_list.Copy();
		assert(list_copy.Size() == 1);
		auto& original_first_branch = original_list[0].as<vl::Object>();
		auto original_leaf_count = original_first_branch["leafCount"].as<vl::Number>().Val();
		auto branch_copy = original_first_branch.Copy();
		branch_copy.Set("leafCount", 5);
		assert(original_first_branch["leafCount"].as<vl::Number>().Val() == original_leaf_count);
		list_copy.Add(branch_copy);
		assert(list_copy.Size() == 2);
		assert(original_list.Size() == 1);
	}
	vl::JSONConverter converter;
	const char* fName = "write_test.json";
	if (converter.Store(bush, vl::TypeResolver(), fName, {true}))
	{
		std::cout << "Stored to '" << fName << "':\n";
		std::cout << converter.JSONStr(bush, vl::TypeResolver(), { true });
	}
	else
		std::cout << "Store error\n";
}

void LoadTest()
{
	std::cout << "Load test\n";
	auto converter = vl::JSONConverter();
	vl::Object object;
	const char* fName = "write_test.json";
	if (converter.Load(object, fName))
		std::cout << "Loaded from '" << fName << "':\n";
	std::cout << converter.JSONStr(object, vl::TypeResolver(), { true });
}

int main(int argc, const char* argv[])
{
	std::cout << "JSONConverter Project Test Unit\n";
	WriteTest();
	LoadTest();
	return 0;
}

#include <iostream>
#include "JSONConverter.h"

void WriteTest()
{
	std::cout << "Write Test\n";
	auto pear = vl::Object();
	pear.Set("isFruit", true);
	pear.Set("color", "Yellow");
	pear.Set("radius", 0.3f);
	pear.Set("branchCount", 1);
	std::cout << "Val: " << pear.Get("color").AsString().Val() << "\n";
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
	branch1.Get("branches").AsList().Add(branch2);
	bush.Get("branches").AsList().Add(branch1);
	vl::JSONConverter converter;
	const char* fName = "write_test.json";
	if (converter.Store(bush, TypeResolver(), fName, {true}))
	{
		std::cout << "Stored to '" << fName << "':\n";
		std::cout << converter.JSONStr(bush, TypeResolver(), { true });
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
	std::cout << converter.JSONStr(object, TypeResolver(), { true });
}

int main(int argc, const char* argv[])
{
	std::cout << "JSONConverter Project Test Unit\n";
	WriteTest();
	LoadTest();
	return 0;
}

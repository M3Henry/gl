#include <iostream>
#include "vulkan/vulkan.hpp"

template<typename T>
bool hasRequiredStrings(std::vector<char const*> need, std::vector<T> able, vk::ArrayWrapper1D<char, 256>(T::* name))
{
	std::vector<char const*> matched;
	for (auto n = need.begin(); n != need.end();)
	{
		bool found;
		for (auto a = able.begin(); a != able.end(); ++a)
		{
			found = not strcmp((*a).*name, *n);
			if (found)
			{
				able.erase(a);
				break;
			}
		}
		if (found)
		{
			matched.push_back(*n);
			n = need.erase(n);
		} else
		{
			++n;
		}
	}
	using std::cout; using std::endl;
	bool success = not need.size();
	if (not success)
	{
		cout << "\x1B[31;1m";
		for (auto x : need) cout << "  " << x << '\n';
	}
	if (matched.size())
	{
		cout << "\x1B[32;1m";
		for (auto x : matched) cout << "  " << x << '\n';
	}
	cout << "\x1B[m";
	//for (auto x : able) cout << "  " << x.*name << '\n';
	return success;
}

bool hasRequiredExtensions(std::vector<char const*> need, std::vector<vk::ExtensionProperties> able)
{
	return hasRequiredStrings(need, able, &vk::ExtensionProperties::extensionName);
}

bool hasRequiredLayers(std::vector<char const*> need, std::vector<vk::LayerProperties> able)
{
	return hasRequiredStrings(need, able, &vk::LayerProperties::layerName);
}

template <typename T>
auto make_file(T& start, T& end)
{
	 return std::make_pair((&end - &start) * sizeof(T), &start);
}

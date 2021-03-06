#pragma once

#include "core/string/string_name.h"
#include "core/templates/local_vector.h"

class World;

typedef bool (*func_temporary_system_execute)(World *p_world);

typedef void (*func_system_execute)(World *p_world);

struct SystemExeInfo {
	bool valid = true;
	Set<uint32_t> mutable_components;
	Set<uint32_t> immutable_components;
	Set<uint32_t> mutable_components_storage;
	Set<uint32_t> mutable_databags;
	Set<uint32_t> immutable_databags;
	Set<uint32_t> need_changed;
	func_system_execute system_func = nullptr;

	void clear() {
		valid = true;
		mutable_components.clear();
		immutable_components.clear();
		mutable_components_storage.clear();
		mutable_databags.clear();
		immutable_databags.clear();
		need_changed.clear();
		system_func = nullptr;
	}
};

typedef void (*func_get_system_exe_info)(SystemExeInfo &);

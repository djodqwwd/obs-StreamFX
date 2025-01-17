// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2017-2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// AUTOGENERATED COPYRIGHT HEADER END

#include "plugin.hpp"
#include "gfx/gfx-opengl.hpp"
#include "obs/gs/gs-helper.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"

#include "configuration.hpp"
#include "ui/ui.hpp"
#include "updater.hpp"

#include "warning-disable.hpp"
#include <fstream>
#include <list>
#include <map>
#include <stdexcept>
#include "warning-enable.hpp"

static std::shared_ptr<streamfx::gfx::opengl> _streamfx_gfx_opengl;

namespace streamfx {
	struct component_info_t {
		std::string           name;
		std::set<std::string> dependencies;
		loader_priority_t     priority;
		loader_function_t     initializer;
		loader_function_t     finalizer;

		typedef std::list<component_info_t> component_list_t;

		static component_list_t& get()
		{
			static component_list_t list;
			return list;
		}
	};

	component::component(std::string_view name, loader_function_t initializer, loader_function_t finalizer, std::set<std::string> dependencies, loader_priority_t priority /*= loader_priority::DEFAULT*/)
	{
		component_info_t ld;
		ld.name         = name;
		ld.dependencies = dependencies;
		ld.priority     = priority;
		ld.initializer  = initializer;
		ld.finalizer    = finalizer;

		component_info_t::get().push_back(ld);
	}

} // namespace streamfx

MODULE_EXPORT bool obs_module_load(void)
{
	try {
		DLOG_INFO("Loading Version %s", STREAMFX_VERSION_STRING);

		// Initialize GLAD (OpenGL)
		{
			streamfx::obs::gs::context gctx{};
			if (gs_get_device_type() == GS_DEVICE_OPENGL) {
				_streamfx_gfx_opengl = streamfx::gfx::opengl::get();
			}
		}

		{ // Run component initializers.
			// ToDo: This can be optimized

			// Retrieve the list
			auto& components = streamfx::component_info_t::get();

			// Sort by priority.
			components.sort([](const streamfx::component_info_t::component_list_t::value_type& a, const streamfx::component_info_t::component_list_t::value_type& b) { return (a.priority < b.priority); });

			// Create a copy of the list and clear the original.
			auto unsorted_components = components;
			components.clear();

			std::set<std::string> resolved;
			while (resolved.size() < unsorted_components.size()) {
				bool has_loaded_anything = false;

				for (auto ld : unsorted_components) {
					// Skip entries that have already been loaded.
					if (resolved.contains(ld.name)) {
						continue;
					}

					// Check if all dependencies are met.
					bool have_dependencies = true;
					for (auto dep : ld.dependencies) {
						if (!resolved.contains(dep)) {
							have_dependencies = false;
							break;
						}
					}
					if (!have_dependencies) {
						continue;
					}

					// Call the initializer
					try {
						ld.initializer();
						has_loaded_anything = true;

						// Add back into the original list.
						components.push_back(ld);

						// And mark as loaded.
						resolved.emplace(ld.name);

						DLOG_INFO("Component %s loaded.", ld.name.c_str());
					} catch (const std::exception& ex) {
						DLOG_ERROR("Initializer threw exception: %s", ex.what());
					} catch (...) {
						DLOG_ERROR("Initializer threw unknown exception.");
					}
				}

				// Fatal error if loading stalled.
				if (!has_loaded_anything) {
					throw std::runtime_error("Loading components stalled, this is a fatal error.");
				}
			}
		}

		DLOG_INFO("Loaded Version %s", STREAMFX_VERSION_STRING);
		return true;
	} catch (std::exception const& ex) {
		DLOG_ERROR("Unexpected exception in function '%s': %s", __FUNCTION_NAME__, ex.what());
		return false;
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		return false;
	}
}

MODULE_EXPORT void obs_module_post_load(void) {}

MODULE_EXPORT void obs_module_unload(void)
{
	try {
		DLOG_INFO("Unloading Version %s", STREAMFX_VERSION_STRING);

		// Run component finalizers.
		auto& components = streamfx::component_info_t::get();
		for (auto itr = components.rbegin(); itr != components.crend(); ++itr) {
			try {
				itr->finalizer();
			} catch (const std::exception& ex) {
				DLOG_ERROR("Finalizer threw exception: %s", ex.what());
			} catch (...) {
				DLOG_ERROR("Finalizer threw unknown exception.");
			}
		}

		// Finalize GLAD (OpenGL)
		{
			streamfx::obs::gs::context gctx{};
			_streamfx_gfx_opengl.reset();
		}

		DLOG_INFO("Unloaded Version %s", STREAMFX_VERSION_STRING);
	} catch (std::exception const& ex) {
		DLOG_ERROR("Unexpected exception in function '%s': %s", __FUNCTION_NAME__, ex.what());
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	}
}

std::filesystem::path streamfx::data_file_path(std::string_view file)
{
	const char8_t* root_path = reinterpret_cast<const char8_t*>(obs_get_module_data_path(obs_current_module()));
	if (root_path) {
		auto ret = std::filesystem::path(root_path);
		ret.append(file.data());
		return ret;
	} else {
		throw std::runtime_error("obs_get_module_data_path returned nullptr");
	}
}

std::filesystem::path streamfx::config_file_path(std::string_view file)
{
	char8_t* root_path = reinterpret_cast<char8_t*>(obs_module_get_config_path(obs_current_module(), file.data()));
	if (root_path) {
		auto ret = std::filesystem::path(root_path);
		bfree(root_path);
		return ret;
	} else {
		throw std::runtime_error("obs_module_get_config_path returned nullptr");
	}
}

bool streamfx::open_url(std::string_view url)
{
	QUrl qurl = QString::fromUtf8(url.data());
	return QDesktopServices::openUrl(qurl);
}

const char* streamfx::translate(const char* key, const char* fallback)
{
	const char* out = nullptr;
	if (obs_module_get_string(key, &out))
		return out;
	return fallback;
}

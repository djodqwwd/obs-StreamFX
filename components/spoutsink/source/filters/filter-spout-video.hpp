// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// AUTOGENERATED COPYRIGHT HEADER END

#pragma once
#include "common.hpp"
#include "obs/obs-source-factory.hpp"

#include "warning-disable.hpp"
#include <utility>
#include <vector>
#include "warning-enable.hpp"

namespace streamfx::filter::spout {
	class video_instance : public obs::source_instance {
		std::pair<size_t, size_t> _resolution;
		gs_color_space            _color_space;
		gs_color_format           _color_format;

		public:
		video_instance(obs_data_t*, obs_source_t*);
		~video_instance() override;

		void load(obs_data_t* settings) override;
		void migrate(obs_data_t* data, uint64_t version) override;
		void update(obs_data_t*) override;

		struct obs_audio_data* filter_audio(struct obs_audio_data* audio) override;

		void video_tick(float) override;
		void video_render(gs_effect_t*) override;
	};

	class video_factory : public obs::source_factory<filter::spout::video_factory, filter::spout::video_instance> {
		public:
		video_factory();
		~video_factory() override;

		const char* get_name() override;

		void get_defaults2(obs_data_t* data) override;

		obs_properties_t* get_properties2(filter::spout::video_instance* data) override;

		public: // Singleton
		static std::shared_ptr<video_factory> instance();
	};
} // namespace streamfx::filter::spout
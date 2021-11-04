/*
 * AnalogSynthModultatorView.cpp
 *
 *  Created on: Jan 24, 2021
 *      Author: jojo
 */

#include "../view/AnalogSynthModulatorView.h"

#include "../view/AnalogSynthOscilatorView.h"
#include "../view/AnalogSynthView.h"
#include "../../resources.h"

AnalogSynthModulatorView::AnalogSynthModulatorView(AdvancedSynth &s, size_t part) : synth(s), binder{s.get_lock(), [&s, part]() {
			return new AnalogSynthModulatorView(s, part);
		}, main_font} {
	this->part = part;
}

Scene AnalogSynthModulatorView::create(ViewHost &frame) {
	std::vector<Control*> controls;
	std::vector<Control*> show_amount;
	std::vector<Control*> show_source;

	ActionHandler& handler = frame.get_master_host().get_action_handler();
	ModEnvelopeEntity& env = synth.preset.mod_envs.at(this->part);
	LFOEntity& lfo = synth.preset.lfos.at(this->part);

	//Background
	Pane* pane = new Pane(sf::Color(120, 120, 120), 5, 5, frame.get_width() - 10, frame.get_height() - 5);
	controls.push_back(pane);

	int tmp_x = 10;
	int tmp_y = 10;

	//Col 1 - Mod Env
	//Title
	{
		Label* title = new Label("Mod Envelope", main_font, 24, tmp_x, tmp_y);
		controls.push_back(title);
	}
	tmp_y += 30;
	//Envelope
	adsr_controls(&controls, tmp_x, tmp_y, env.env, handler);
	tmp_y += 225;
	//Volume
	property_mod_controls(&controls, tmp_x, tmp_y, env.volume, handler, "Volume", &show_amount, &show_source);
	tmp_y += 75;

	//Col 2 - LFO
	tmp_x = 500;
	tmp_y = 10;
	//Title
	{
		Label* title = new Label("LFO", main_font, 24, tmp_x, tmp_y);
		controls.push_back(title);
	}
	tmp_y += 30;
	//Frequency
	{
		Label* title = new Label("Frequency", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, 0, 50, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->drag_mul *= 0.25;
		value->property.bind(lfo.freq, handler);
		controls.push_back(value);
	}
	tmp_x += 90;
	tmp_y += 15;
	//Wave Form
	{
		std::vector<std::string> waveforms = {"Sine", "Saw Down", "Saw Up", "Square", "Triangle", "Noise"};

		ComboBox* waveform = new ComboBox(1, waveforms, main_font, 16, 0, tmp_x , tmp_y, 150, 40);
		waveform->property.bind_cast(lfo.waveform, handler);
		controls.push_back(waveform);
	}
	tmp_x = 500;
	tmp_y += 60;
	//Volume
	property_mod_controls(&controls, tmp_x, tmp_y, lfo.volume, handler, "Volume", &show_amount, &show_source);
	tmp_y += 75;

	//Master Sync
	{
		CheckBox* value = new CheckBox(false, "Master Sync", main_font, 16, tmp_x, tmp_y, 40, 40);
		value->property.bind(lfo.sync_master, handler);
		controls.push_back(value);
	}
	tmp_x += 160;
	//Clock Value
	{
		Label* title = new Label("Clock Value", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<int>* value = new DragBox<int>(0, -32, 32, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->drag_mul *= 0.5;
		value->property.bind(lfo.clock_value, handler);
		controls.push_back(value);
	}
	tmp_x += 90;
	//Sync Phase
	{
		Label* title = new Label("Sync Phase", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, 0, 1, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(lfo.sync_phase, handler);
		controls.push_back(value);
	}
	tmp_x += 90;
	//Motion Sequencer
	{
		Label* title = new Label("Motion Sequencer", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<int>* value = new DragBox<int>(0, -32, 32, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->drag_mul *= 0.5;
		value->property.bind(lfo.motion_sequencer, handler);
		controls.push_back(value);
	}
	tmp_x += 90;

	//Edit Sources
	Button* edit = new Button("Edit Sources", main_font, 18, 75, frame.get_height() - 40, 120, 40);
	edit->rect.setFillColor(sf::Color::Yellow);
	edit->set_on_click([&frame, show_amount, show_source, this]() {
		edit_source = !edit_source;
		for (Control* c : show_amount) {
			c->set_visible(!edit_source);
		}
		for (Control* c : show_source) {
			c->set_visible(edit_source);
		}
	});
	controls.push_back(edit);

	controls.push_back(binder.create_button(75 + 120, frame.get_height() - 40, &frame));
	//Back Button
	Button* back = new Button("Back", main_font, 18, 5, frame.get_height() - 40, 70, 40);
	back->rect.setFillColor(sf::Color::Yellow);
	back->set_on_click([&frame, this]() {
		frame.change_view(new AnalogSynthView(synth));
	});
	controls.push_back(back);

	for (Control* c : show_amount) {
		c->set_visible(!edit_source);
	}
	for (Control* c : show_source) {
		c->set_visible(edit_source);
	}

	return {controls};
}

AnalogSynthModulatorView::~AnalogSynthModulatorView() {

}

bool AnalogSynthModulatorView::on_action(Control *control) {
	return binder.on_action(control);
}
